// Verifies per-layer point icons: the default stays a circle, choosing a Maki
// icon changes what is drawn for both static points and moving-object
// positions, the picker previews the glyph, and clearing it returns to the
// circle. Also asserts the atlas plumbing never falls back to deck's
// auto-packing, which is the one way this can degrade silently.
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

  // Any icon-atlas trouble surfaces here rather than as a blank map: deck
  // throws "Icon url is missing." the moment it falls back to auto-packing,
  // and logs a load failure if the data URL is rejected.
  const iconErrors = [];
  const watch = (text) => {
    if (/icon|iconAtlas/i.test(text) && /missing|error|failed|invalid/i.test(text))
      iconErrors.push(text);
  };
  page.on("pageerror", (e) => { console.log("[pageerror]", e.message); watch(e.message); });
  page.on("console", (m) => { if (m.type() === "error") watch(m.text()); });

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
      for (let i = 0; i < px.length; i += 4)
        if (px[i + 3] > 10 && Math.max(px[i], px[i + 1], px[i + 2]) > 70) n++;
      return n;
    });
  // The height of the drawn symbol, in device pixels. This is the measurement
  // that actually distinguishes a circle from an icon: pixel *counts* do not,
  // because a thin glyph in a 16px box lights about as many pixels as a filled
  // 8px disc. Only meaningful with a single symbol on screen.
  const symbolHeight = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const w = c.width, h = c.height;
      const px = new Uint8Array(w * h * 4);
      gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, px);
      let top = Infinity, bottom = -Infinity;
      for (let y = 0; y < h; y++)
        for (let x = 0; x < w; x++) {
          const i = (y * w + x) * 4;
          if (px[i + 3] > 10 && Math.max(px[i], px[i + 1], px[i + 2]) > 70) {
            if (y < top) top = y;
            if (y > bottom) bottom = y;
          }
        }
      return bottom < top ? 0 : bottom - top + 1;
    });
  // Deck redraws asynchronously (and the icon atlas arrives via a data-URL
  // fetch), so read until two consecutive frames agree rather than guessing a
  // sleep -- an early read once measured a trail that was already switched off.
  async function settled(measure) {
    let prev = -1;
    for (let i = 0; i < 12; i++) {
      await new Promise((r) => setTimeout(r, 350));
      const v = await measure();
      if (v === prev) return v;
      prev = v;
    }
    return prev;
  }
  const setIcon = (name) => page.select(".lp-style .lp-icon", name);
  const clearLayers = async () => {
    const btn = await page.evaluateHandle(() =>
      [...document.querySelectorAll(".lp-clear")].find((b) => b.textContent.trim() === "clear"));
    await btn.asElement().click();
    await page.waitForFunction(() => document.querySelectorAll(".lp-item").length === 0,
                               { timeout: 3000 });
  };

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");

  // --- moving objects: the ScatterplotLayer swaps for an IconLayer ---------
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

  await page.click(".lp-name");
  await page.waitForSelector(".lp-style .lp-icon", { timeout: 3000 });
  check(true, "icon picker shown in the style editor");
  const defaultIcon = await page.$eval(".lp-style .lp-icon", (el) => el.value);
  check(defaultIcon === "",
        `a fresh layer defaults to the circle, not an icon (value "${defaultIcon}")`);
  check((await page.$$(".lp-icon-preview")).length === 0,
        "no glyph preview while the layer draws circles");

  // Show only the position symbols, so the measurement is about them alone.
  await page.select(".lp-style .lp-moving", "points");
  const dotsPx = await settled(drawn);

  await setIcon("rail");
  const railPx = await settled(drawn);
  await page.screenshot({ path: `${OUT}/icon-rail.png` });
  check(railPx > 0 && Math.abs(railPx - dotsPx) > dotsPx * 0.1,
        `rail icons replace the position dots (${dotsPx} px -> ${railPx} px)`);
  const previewPath = await page.$eval(".lp-icon-preview path", (el) => el.getAttribute("d"))
    .catch(() => null);
  check(previewPath && previewPath.length > 20 && !previewPath.includes("&"),
        `the picker previews the glyph with decoded path data (${(previewPath ?? "").slice(0, 24)}…)`);

  await setIcon("");
  const backPx = await settled(drawn);
  check(Math.abs(backPx - dotsPx) < Math.max(dotsPx * 0.15, 30),
        `clearing the icon returns to the circle (${backPx} ~ ${dotsPx})`);

  // --- static points: GeoJsonLayer pointType circle -> icon ----------------
  // One isolated point, so the symbol's own height can be measured: that is
  // the assertion that really separates a circle from an icon. "restaurant" is
  // one of the 77 Maki icons whose path data carries XML character references,
  // so a naive regex parse would draw a truncated fragment -- this doubles as
  // the decoding check.
  await clearLayers();
  await runCmd("query mehringdamm");
  await page.waitForSelector(".lp-item", { timeout: 8000 });
  await page.click(".lp-name");
  await page.waitForSelector(".lp-style .lp-icon", { timeout: 3000 });
  const circleH = await settled(symbolHeight);
  await setIcon("restaurant");
  const iconH = await settled(symbolHeight);
  await page.screenshot({ path: `${OUT}/icon-restaurant.png` });
  check(iconH > circleH * 1.4,
        `a static point draws as a taller icon, not a disc (${circleH}px -> ${iconH}px)`);
  await setIcon("");
  check(Math.abs((await settled(symbolHeight)) - circleH) <= 2,
        `and returns to the disc when the icon is cleared`);

  check(iconErrors.length === 0,
        `no icon-atlas errors logged (${iconErrors.join(" | ") || "none"})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
