// Verifies mreal/mint value plots: they render as small multiples (one y-scale
// each, never a dual axis), the readout tracks the timeline cursor, and a
// plot-only object (no geometry) still works.
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

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1280,850"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1280, height: 850 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  async function runCmd(cmd) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 900));
  }
  const seek = async (frac) => {
    await page.$eval(".tl-range", (el, f) => {
      const set = Object.getOwnPropertyDescriptor(
        window.HTMLInputElement.prototype, "value").set;
      set.call(el, String(Number(el.max) * f));
      el.dispatchEvent(new Event("input", { bubbles: true }));
    }, frac);
    await new Promise((r) => setTimeout(r, 400));
  };
  const values = () => page.$$eval(".plot-value", (els) => els.map((e) => e.textContent));

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");

  // A plot-only object: no geometry at all, just a value over time.
  await runCmd("query mreal5000");
  await page.waitForSelector(".plot", { timeout: 8000 });
  check(true, "plot-only object (mreal, no geometry) renders a plot");
  const constVals = await values();
  check(constVals[0] === "5000", `mreal5000 reads 5000 (${constVals[0]})`);

  // A varying mreal + a mint on different scales -> small multiples.
  await runCmd("query distance(train7, mehringdamm)");
  await runCmd("query noAtCenter");
  await new Promise((r) => setTimeout(r, 500));
  const plots = await page.$$eval(".plot", (els) => els.length);
  check(plots === 3, `three measures -> three small multiples, not one axis (${plots})`);
  const svgs = await page.$$eval(".plot-svg", (els) => els.length);
  check(svgs === plots, `each measure has its own plot/y-scale (${svgs})`);

  // Each plot is labelled and swatched (identity never colour-alone).
  const labels = await page.$$eval(".plot-label", (els) => els.map((e) => e.textContent));
  const swatches = await page.$$eval(".plot-swatch", (els) => els.length);
  check(labels.every((l) => l && l.length > 0) && swatches === plots,
        `every plot is directly labelled + swatched (${labels.join(", ")})`);

  // The readout must follow the timeline cursor.
  await page.click(".tl-play"); // pause
  await new Promise((r) => setTimeout(r, 300));
  await seek(0.05);
  const early = await values();
  await seek(0.55);
  const mid = await values();
  check(early.join("|") !== mid.join("|"),
        `readouts track the timeline cursor (${early.join(",")} -> ${mid.join(",")})`);
  // The constant series must NOT change, the varying one must.
  check(early[0] === "5000" && mid[0] === "5000", `constant series stays constant`);

  await page.screenshot({ path: `${OUT}/plots.png` });

  // Collapsing keeps the map clear.
  await page.click(".plots-toggle");
  await new Promise((r) => setTimeout(r, 250));
  check((await page.$$(".plot")).length === 0, `plots collapse away`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
