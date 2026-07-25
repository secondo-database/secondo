// Verifies moving regions (mregion): the polygon is rebuilt at the current
// instant, so it visibly moves/changes as the timeline advances.
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
  // Drawn-pixel count + centroid of the rendered region.
  const shape = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const w = c.width, h = c.height;
      const px = new Uint8Array(w * h * 4);
      gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, px);
      let n = 0, sx = 0, sy = 0;
      for (let i = 0; i < px.length; i += 4) {
        if (px[i + 3] > 10 && Math.max(px[i], px[i + 1], px[i + 2]) > 40) {
          const p = i / 4; sx += p % w; sy += Math.floor(p / w); n++;
        }
      }
      return { n, cx: n ? sx / n : 0, cy: n ? sy / n : 0 };
    });
  const seek = async (frac) => {
    await page.$eval(".tl-range", (el, f) => {
      const set = Object.getOwnPropertyDescriptor(
        window.HTMLInputElement.prototype, "value").set;
      set.call(el, String(Number(el.max) * f));
      el.dispatchEvent(new Event("input", { bubbles: true }));
    }, frac);
    await new Promise((r) => setTimeout(r, 600));
  };

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query mrain"); // a moving region (rain cloud drifting NE)

  await page.waitForSelector(".timeline", { timeout: 10000 });
  check(true, "mregion produces a timeline (temporal layer)");
  await page.click(".tl-play"); // pause for deterministic sampling
  await new Promise((r) => setTimeout(r, 300));

  await seek(0.1);
  const a = await shape();
  await page.screenshot({ path: `${OUT}/mregion-t10.png` });
  check(a.n > 500, `moving region is drawn (${a.n} px)`);

  await seek(0.9);
  const b = await shape();
  await page.screenshot({ path: `${OUT}/mregion-t90.png` });
  check(b.n > 500, `still drawn later in the domain (${b.n} px)`);

  const moved = Math.hypot(a.cx - b.cx, a.cy - b.cy);
  check(moved > 20,
        `region moves as time advances (centroid drift ${moved.toFixed(1)}px)`);

  // It must also work under the BerlinMOD projection (vertices projected).
  await page.select(".projection-ctl select", "berlinmod");
  await page.waitForSelector(".maplibregl-map", { timeout: 8000 });
  await new Promise((r) => setTimeout(r, 2500));
  const geo = await shape();
  check(geo.n > 500, `moving region renders under BerlinMOD projection (${geo.n} px)`);
  await page.screenshot({ path: `${OUT}/mregion-projected.png` });
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
