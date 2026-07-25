// Verifies the layers/styling/selection milestone: multiple layers accumulate,
// per-layer recolor updates the swatch, visibility toggling changes what's
// drawn, reordering swaps draw order, and clicking a feature shows its details.
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
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});

let fails = 0;
const check = (cond, msg) => { console.log(`${cond ? "PASS" : "FAIL"} ${msg}`); if (!cond) fails++; };

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  async function runCmd(cmd) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
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
      for (let i = 0; i < px.length; i += 4) if (px[i + 3] > 10) n++;
      return n;
    });

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query Flaechen feed head[5] consume"); // regions with Name
  await runCmd("query BGrenzenLine"); // a line

  // 1) Two layers listed.
  const items = await page.$$eval(".lp-item", (els) => els.length);
  check(items === 2, `two layers listed (got ${items})`);

  // Panel shows topmost first; the line (added last) should be on top.
  const firstName = await page.$eval(".lp-name", (el) => el.textContent);
  check(/BGrenzenLine/.test(firstName ?? ""), `top layer is the line (got "${firstName}")`);

  // 2) Expand the top layer and recolor it -> swatch updates.
  await page.click(".lp-name");
  await page.waitForSelector(".lp-style", { timeout: 3000 });
  const before = await page.$eval(".lp-swatch", (el) => el.style.background);
  await page.$eval(".lp-style input[type=color]", (el) => {
    const set = Object.getOwnPropertyDescriptor(
      window.HTMLInputElement.prototype, "value").set;
    set.call(el, "#ff0000");
    el.dispatchEvent(new Event("input", { bubbles: true }));
    el.dispatchEvent(new Event("change", { bubbles: true }));
  });
  await new Promise((r) => setTimeout(r, 300));
  const after = await page.$eval(".lp-swatch", (el) => el.style.background);
  check(before !== after && /255|#ff0000|rgb\(255/.test(after),
        `recolor updates swatch ("${before}" -> "${after}")`);

  // 3) Visibility toggle changes drawn pixels (hide the big regions layer).
  const withAll = await drawn();
  // second checkbox in list = the regions layer (bottom item)
  const boxes = await page.$$(".lp-item input[type=checkbox]");
  await boxes[boxes.length - 1].click(); // hide bottom (regions)
  await new Promise((r) => setTimeout(r, 400));
  const withoutRegions = await drawn();
  check(withoutRegions < withAll - 500,
        `hiding regions reduces drawn pixels (${withAll} -> ${withoutRegions})`);
  await boxes[boxes.length - 1].click(); // show again
  await new Promise((r) => setTimeout(r, 400));

  // 4) Reorder: send the top (line) backward; a region name becomes first.
  await page.click(".lp-name"); // collapse style so rows are simple
  await new Promise((r) => setTimeout(r, 150));
  const sendBack = await page.$('.lp-item button[title="Send backward"]');
  await sendBack.click();
  await new Promise((r) => setTimeout(r, 250));
  const newFirst = await page.$eval(".lp-name", (el) => el.textContent);
  check(!/BGrenzenLine/.test(newFirst ?? ""),
        `reorder moved line down (top now "${newFirst}")`);

  // 5) Collapse: the panel folds to its header, freeing the map corner, and
  // restores the same rows. Collapsing must not drop or hide any layer.
  const panelH = () => page.$eval(".layers-panel", (el) => Math.round(el.getBoundingClientRect().height));
  // The header must not shift: hiding the export/clear buttons used to shrink
  // it and bounce the title by a few px.
  const titleY = () => page.$eval(".lp-collapse", (el) => el.getBoundingClientRect().top);
  const headH = () => page.$eval(".lp-head", (el) => el.getBoundingClientRect().height);
  const openH = await panelH();
  const openTitleY = await titleY();
  const openHeadH = await headH();
  const drawnOpen = await drawn();
  await page.click(".lp-collapse");
  await new Promise((r) => setTimeout(r, 250));
  check(Math.abs((await titleY()) - openTitleY) < 0.5,
        `"Layers" title does not move when collapsing (${openTitleY.toFixed(1)} -> ${(await titleY()).toFixed(1)})`);
  check(Math.abs((await headH()) - openHeadH) < 0.5,
        `header keeps its height when collapsed (${openHeadH.toFixed(1)} -> ${(await headH()).toFixed(1)})`);
  const shutH = await panelH();
  check(shutH < openH - 40, `collapse folds the panel to its header (${openH} -> ${shutH}px)`);
  check((await page.$$(".lp-item")).length === 0, `collapsed panel hides the layer rows`);
  // The map is untouched: collapsing is a panel affordance, not a layer edit.
  check(Math.abs((await drawn()) - drawnOpen) < drawnOpen * 0.02,
        `collapsing does not change what is drawn`);
  await page.click(".lp-collapse");
  await new Promise((r) => setTimeout(r, 250));
  check((await page.$$(".lp-item")).length === 2,
        `expanding restores both layer rows`);

  // 6) Selection: isolate the regions layer so it fills the viewport, then
  // click a filled region -> details panel shows its Name property.
  // NB: several buttons share `.lp-clear`, so target "clear" by its text --
  // a bare page.click(".lp-clear") hits "export" and silently leaves the
  // layers in place, which then keeps the view fit to another layer's bounds.
  const clearBtn = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".lp-clear")].find((b) => b.textContent.trim() === "clear"));
  await clearBtn.asElement().click();
  await page.waitForFunction(() => document.querySelectorAll(".lp-item").length === 0,
                             { timeout: 3000 });
  await runCmd("query Flaechen feed head[5] consume");
  await new Promise((r) => setTimeout(r, 600));

  // Locate a filled pixel in the *interior* of a polygon. Requiring the
  // neighbourhood to be filled too avoids landing on a thin line or an
  // anti-aliased edge, where picking legitimately misses. Colour-agnostic: the
  // layer palette is a design choice and must not break this test.
  const findTarget = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      const rect = c.getBoundingClientRect();
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const w = c.width, h = c.height;
      const px = new Uint8Array(w * h * 4);
      gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, px);
      const filled = (x, y) => {
        const i = (y * w + x) * 4;
        return px[i + 3] > 60 && Math.max(px[i], px[i + 1], px[i + 2]) > 60;
      };
      const R = 4; // interior margin
      for (let y = R; y < h - R; y++) {
        for (let x = R; x < w - R; x++) {
          if (
            filled(x, y) && filled(x + R, y) && filled(x - R, y) &&
            filled(x, y + R) && filled(x, y - R)
          ) {
            // readPixels is bottom-up; CSS is top-down.
            return {
              x: rect.left + (x / w) * rect.width,
              y: rect.top + ((h - y) / h) * rect.height,
            };
          }
        }
      }
      return null;
    });

  // Snapshot -> click -> verify, and retry. Under full-suite load the regions
  // aren't always drawn (and re-fitted) within the fixed wait above, so the
  // first snapshot can find no interior pixel or an off frame and the pick
  // misses. Re-snapshotting between attempts waits for a settled frame and a
  // correct coordinate rather than flaking on a single early look.
  let hasName = false;
  for (let attempt = 0; attempt < 5 && !hasName; attempt++) {
    const target = await findTarget();
    if (!target) { await new Promise((r) => setTimeout(r, 300)); continue; }
    await page.mouse.click(target.x, target.y);
    await new Promise((r) => setTimeout(r, 300));
    if (await page.$(".details")) {
      hasName = await page.$$eval(".details .dk", (els) =>
        els.some((e) => e.textContent === "Name"));
    }
  }
  check(hasName, `clicking a region shows its Name in details`);

  await page.screenshot({ path: `${OUT}/layers.png` });
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
