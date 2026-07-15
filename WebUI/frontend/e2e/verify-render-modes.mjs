// Verifies the render-mode + style polish: moving-object "positions" mode draws
// discrete dots (fewer pixels than the trail), pointRadius affects them, and
// internal _attr/_layer keys are hidden from the details panel.
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
         "--enable-unsafe-swiftshader", "--window-size=1280,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1280, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  async function runCmd(cmd) {
    await page.click(".input input");
    await page.$eval(".input input", (el) => (el.value = ""));
    await page.type(".input input", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 700));
  }
  const drawn = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const px = new Uint8Array(c.width * c.height * 4);
      gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, px);
      let n = 0;
      for (let i = 0; i < px.length; i += 4)
        if (px[i + 3] > 10 && Math.max(px[i], px[i + 1], px[i + 2]) > 70) n++;
      return n;
    });
  async function setMode(mode) {
    await page.select(".lp-style select", mode);
    await new Promise((r) => setTimeout(r, 500));
  }

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query Trains feed head[10] project[Id, Trip] consume");
  await page.waitForSelector(".timeline", { timeout: 12000 });

  // Pause and seek to mid-domain so measurements are stable.
  await page.click(".tl-play");
  await new Promise((r) => setTimeout(r, 200));
  await page.$eval(".tl-range", (el) => {
    const set = Object.getOwnPropertyDescriptor(
      window.HTMLInputElement.prototype, "value").set;
    set.call(el, String(Number(el.max) * 0.5));
    el.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await new Promise((r) => setTimeout(r, 400));

  // Expand the layer style; the moving-mode <select> must be present.
  await page.click(".lp-name");
  await page.waitForSelector(".lp-style select", { timeout: 3000 });
  check(true, "moving-mode selector shown for temporal layer");

  await setMode("trail");
  const trailPx = await drawn();
  await page.screenshot({ path: `${OUT}/mode-trail.png` });

  await setMode("points");
  const pointsPx = await drawn();
  await page.screenshot({ path: `${OUT}/mode-points.png` });
  check(pointsPx > 0 && pointsPx < trailPx,
        `positions mode draws discrete dots, fewer px than trail (${pointsPx} < ${trailPx})`);

  // pointRadius affects the moving-position dots (item 3).
  await page.$$eval(".lp-style input[type=range]", (els) => {
    // order: opacity, point, line  -> index 1 is point radius
    const el = els[1];
    const set = Object.getOwnPropertyDescriptor(
      window.HTMLInputElement.prototype, "value").set;
    set.call(el, "12");
    el.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await new Promise((r) => setTimeout(r, 400));
  const biggerDots = await drawn();
  check(biggerDots > pointsPx,
        `increasing pointRadius enlarges position dots (${biggerDots} > ${pointsPx})`);

  // Details panel hides internal _attr / _layer (item 2 parity).
  const clearBtn = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".lp-clear")].find((b) => b.textContent === "clear"));
  await clearBtn.asElement().click(); // remove the Trains layer first
  await new Promise((r) => setTimeout(r, 200));
  await runCmd("query Flaechen feed head[5] consume");
  await new Promise((r) => setTimeout(r, 500));
  const target = await page.evaluate(() => {
    const c = document.querySelector(".mapview canvas");
    const rect = c.getBoundingClientRect();
    const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
               c.getContext("webgl", { preserveDrawingBuffer: true });
    const w = c.width, h = c.height;
    const px = new Uint8Array(w * h * 4);
    gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, px);
    for (let i = 0; i < px.length; i += 4) {
      const r = px[i], g = px[i + 1], b = px[i + 2], a = px[i + 3];
      if (a > 60 && b > 90 && r < 150) {
        const p = i / 4, x = p % w, y = Math.floor(p / w);
        if (x > w * 0.3 && x < w * 0.7 && y > h * 0.3 && y < h * 0.7)
          return { x: rect.left + (x / w) * rect.width,
                   y: rect.top + ((h - y) / h) * rect.height };
      }
    }
    return null;
  });
  if (target) {
    await page.mouse.click(target.x, target.y);
    await new Promise((r) => setTimeout(r, 300));
  }
  const keys = await page.$$eval(".details .dk", (els) => els.map((e) => e.textContent))
    .catch(() => []);
  check(keys.includes("Name") && !keys.includes("_attr") && !keys.includes("_layer"),
        `details hides _attr/_layer, keeps Name (keys: ${keys.join(",")})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
