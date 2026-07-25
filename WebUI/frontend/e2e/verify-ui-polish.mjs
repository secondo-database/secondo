// Verifies: (1) the map does not move when editing layer style / removing a
// layer, (2) the database list is not duplicated in the console header and the
// open-db shown there matches the catalog, (3) panels resize by dragging the
// splitters, (4) the console can be docked to the bottom.
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
         "--enable-unsafe-swiftshader", "--window-size=1400,850"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1400, height: 850 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));
  await page.evaluateOnNewDocument(() => localStorage.clear());

  async function runCmd(cmd) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 800));
  }
  // A fingerprint of what's on the map: total drawn pixels + their centroid.
  const frame = () =>
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
  const rect = (sel) => page.$eval(sel, (el) => {
    const r = el.getBoundingClientRect();
    return { w: Math.round(r.width), h: Math.round(r.height), x: Math.round(r.x), y: Math.round(r.y) };
  });
  const drag = async (sel, dx, dy) => {
    const r = await rect(sel);
    await page.mouse.move(r.x + r.w / 2, r.y + r.h / 2);
    await page.mouse.down();
    await page.mouse.move(r.x + r.w / 2 + dx, r.y + r.h / 2 + dy, { steps: 8 });
    await page.mouse.up();
    await new Promise((r2) => setTimeout(r2, 250));
  };

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query Flaechen feed head[5] consume");
  await runCmd("query BGrenzenLine");
  await new Promise((r) => setTimeout(r, 600));

  // --- 1) Map must not re-fit on style change / layer removal ---
  // Measuring a centroid over *coloured* pixels is the wrong instrument here: a
  // recolour alone shifts it, because a brighter hue pushes more anti-aliased
  // edge pixels past the brightness threshold. Instead, zoom out so the view is
  // clearly not the auto-fit framing, then assert the framing survives: a re-fit
  // would snap back and multiply the drawn-pixel count (viewfit measures ~10x).
  const zoomOut = async (n) => {
    const btn = await page.evaluateHandle(() =>
      [...document.querySelectorAll(".zoom-ctl button")].find((b) => b.title === "Zoom out"));
    for (let i = 0; i < n; i++) {
      await btn.asElement().click();
      await new Promise((r) => setTimeout(r, 200));
    }
    await new Promise((r) => setTimeout(r, 300));
  };
  await zoomOut(3);
  const zoomed = await frame();
  check(zoomed.n > 0, `zoomed-out baseline drawn (${zoomed.n} px)`);

  await page.click(".lp-name"); // expand style editor of the top layer
  await page.waitForSelector(".lp-style", { timeout: 3000 });
  await page.$eval(".lp-style input[type=color]", (el) => {
    const set = Object.getOwnPropertyDescriptor(
      window.HTMLInputElement.prototype, "value").set;
    set.call(el, "#ff0000");
    el.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await new Promise((r) => setTimeout(r, 400));
  const afterStyle = await frame();
  check(afterStyle.n < zoomed.n * 3,
        `map keeps its framing on style change (${zoomed.n} -> ${afterStyle.n} px; a re-fit would ~10x)`);

  // Remove the top layer: the view must still not snap back to a fit.
  await page.click(".lp-item .lp-x");
  await new Promise((r) => setTimeout(r, 500));
  const afterRemove = await frame();
  check(afterRemove.n < zoomed.n * 3,
        `map keeps its framing on layer removal (${zoomed.n} -> ${afterRemove.n} px)`);

  // --- 2) No duplicated database list; console shows the open db ---
  const headerText = await page.$eval(".console header", (el) => el.textContent);
  check(!/BERLINTEST\s*·\s*OPT/.test(headerText ?? ""),
        `console header no longer duplicates the database list`);
  check(/db:\s*berlintest/.test(headerText ?? ""),
        `console header shows the open database (${headerText?.trim()})`);
  const catalogDbs = await page.$$eval(".cat-db", (els) => els.length);
  check(catalogDbs >= 3, `catalog is the single place listing databases (${catalogDbs})`);

  // --- 3) The layout gives the map the full width (console under it) ---
  const con0 = await rect(".console-pane");
  const map0 = await rect(".map-pane");
  check(con0.y > map0.y, `console is docked under the map (map on top)`);
  check(Math.abs(con0.w - map0.w) < 2,
        `console spans the map's width (${con0.w} vs ${map0.w})`);
  const dockBtns = await page.$$eval(".dock-btn", (els) =>
    els.map((b) => b.textContent.trim()));
  check(!dockBtns.some((t) => /left|bottom/.test(t)),
        `no dock-to-the-left button any more (${dockBtns.join(" | ")})`);

  // --- 4) Resizable panels ---
  const catBefore = (await rect(".catalog-pane")).w;
  await drag(".split-a", 80, 0);
  const catAfter = (await rect(".catalog-pane")).w;
  check(catAfter > catBefore + 50, `catalog resizes by drag (${catBefore} -> ${catAfter})`);

  const hBefore = (await rect(".console-pane")).h;
  await drag(".split-b", 0, -70);
  const hAfter = (await rect(".console-pane")).h;
  check(hAfter > hBefore + 40, `bottom console resizes vertically (${hBefore} -> ${hAfter})`);

  // --- 5) Collapsing panels frees space for the map ---
  const mapBefore = await rect(".map-pane");
  await page.click(".cat-collapse"); // catalog -> rail
  await new Promise((r) => setTimeout(r, 300));
  const mapWider = await rect(".map-pane");
  check(mapWider.w > mapBefore.w + 150,
        `collapsing the catalog widens the map (${mapBefore.w} -> ${mapWider.w})`);

  // Collapse the query history -> console shrinks to an input bar.
  const conFull = (await rect(".console-pane")).h;
  const histBtn = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".dock-btn")].find((b) => /history/.test(b.textContent)));
  await histBtn.asElement().click();
  await new Promise((r) => setTimeout(r, 300));
  const conSlim = await rect(".console-pane");
  const mapTaller = await rect(".map-pane");
  check(conSlim.h < conFull - 100,
        `hiding history shrinks the console to an input bar (${conFull} -> ${conSlim.h})`);
  check(mapTaller.h > mapWider.h + 100,
        `map gains the freed height (${mapWider.h} -> ${mapTaller.h})`);
  // The command input must remain usable while history is hidden.
  check(!!(await page.$(".input textarea")), `query input still available when collapsed`);
  await page.screenshot({ path: `${OUT}/layout-max-map.png` });

  // Restore the catalog from the rail.
  await page.click(".rail-btn");
  await new Promise((r) => setTimeout(r, 300));
  const catBack = await rect(".catalog-pane");
  check(catBack.w > 100, `catalog restores from the rail (${catBack.w})`);

  // --- 6) Showing the history again gives the console its height back ---
  await histBtn.asElement().click();
  await new Promise((r) => setTimeout(r, 300));
  const conBack = await rect(".console-pane");
  check(conBack.h > conSlim.h + 100,
        `history comes back (${conSlim.h} -> ${conBack.h})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
