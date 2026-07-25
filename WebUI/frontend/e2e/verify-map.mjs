// End-to-end map verification: drives the running app with a headless browser,
// runs spatial queries, and asserts the deck.gl WebGL canvas actually draws.
//
// Prereqs (all three must be up):
//   - SecondoMonitor on :1234 with berlintest
//   - FastAPI bridge on :8000
//   - Vite dev server on :5173
//
// Usage:  CHROMIUM=/usr/bin/chromium node e2e/verify-map.mjs
// Writes screenshots to e2e/out/.
import { createRequire } from "module";
import { mkdirSync } from "fs";
import { dirname } from "path";
import { fileURLToPath } from "url";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const HERE = dirname(fileURLToPath(import.meta.url));
const OUT = `${HERE}/out`;
mkdirSync(OUT, { recursive: true });

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

const CASES = [
  { name: "region", command: "query thecenter" },
  { name: "line", command: "query BGrenzenLine" },
  { name: "point", command: "query mehringdamm" },
];

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: [
    "--no-sandbox",
    "--use-gl=angle",
    "--use-angle=swiftshader",
    "--enable-unsafe-swiftshader",
    "--enable-webgl",
    "--window-size=1200,800",
  ],
});

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("  [pageerror]", e.message));

  async function runCmd(command) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", command);
    await page.keyboard.press("Enter");
  }

  async function drawnPixels() {
    return page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      if (!c) return -1;
      const gl =
        c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
        c.getContext("webgl", { preserveDrawingBuffer: true });
      if (!gl) return -1;
      const px = new Uint8Array(c.width * c.height * 4);
      gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, px);
      let drawn = 0;
      for (let i = 0; i < px.length; i += 4)
        if (px[i + 3] > 10 && px[i + 2] > 40) drawn++;
      return drawn;
    });
  }

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await page.waitForFunction(
    () => document.querySelectorAll(".entry").length >= 1,
    { timeout: 8000 }
  );

  let failures = 0;
  let expected = 0;
  for (const { name, command } of CASES) {
    expected += 1; // each spatial query adds one ".geohint" marker
    await runCmd(command);
    await page.waitForFunction(
      (n) => document.querySelectorAll(".geohint").length >= n,
      { timeout: 10000 },
      expected
    );
    await new Promise((r) => setTimeout(r, 1200));
    const drawn = await drawnPixels();
    await page.screenshot({ path: `${OUT}/map-${name}.png` });
    // A lone point is a ~4px dot (~80px); regions/lines are far larger.
    const ok = drawn >= 30;
    console.log(`${ok ? "PASS" : "FAIL"} ${name}: drawn=${drawn}`);
    if (!ok) failures++;
  }

  process.exitCode = failures === 0 ? 0 : 1;
} finally {
  await browser.close();
}
