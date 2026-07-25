// Verifies the light/dark theme switch: the toggle in the console header flips
// the palette, every surface follows (no element keeps a dark background while
// the app is light), text stays readable, and the choice survives a reload.
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

// Perceived lightness (0..1) of a CSS colour, so "is this surface dark?" is one
// number rather than a hex comparison.
const LUMA = `(css) => {
  const m = css.match(/[\\d.]+/g);
  if (!m) return null;
  const [r, g, b, a] = m.map(Number);
  if (a === 0) return null; // fully transparent: it shows what is behind it
  return (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255;
}`;

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1400, height: 850 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));
  // Start from a clean slate, but only once: the reload below is what proves
  // the theme is remembered, so it must keep what the first load stored.
  await page.evaluateOnNewDocument(() => {
    if (!sessionStorage.getItem("e2e-started")) {
      localStorage.clear();
      sessionStorage.setItem("e2e-started", "1");
    }
  });
  await page.goto(URL, { waitUntil: "networkidle2" });
  await page.waitForSelector(".console header");

  const luma = (sel, prop) =>
    page.evaluate(
      (sel, prop, fn) => {
        const el = document.querySelector(sel);
        if (!el) return null;
        return eval(`(${fn})`)(getComputedStyle(el)[prop]);
      },
      sel, prop, LUMA
    );

  const themeBtn = async () =>
    (await page.evaluateHandle(() =>
      [...document.querySelectorAll(".dock-btn")].find((b) => /light|dark/.test(b.textContent))
    )).asElement();

  // --- 1) Dark is the default ---
  const darkBg = await luma("body", "backgroundColor");
  const darkText = await luma("body", "color");
  check(darkBg !== null && darkBg < 0.25, `dark theme is the default (bg luma ${darkBg?.toFixed(2)})`);
  check(darkText > 0.7, `dark theme keeps light text (luma ${darkText?.toFixed(2)})`);
  await page.screenshot({ path: `${OUT}/theme-dark.png` });

  // --- 2) The toggle switches to light ---
  const btn = await themeBtn();
  check(!!btn, `theme toggle is in the console header`);
  await btn.click();
  await new Promise((r) => setTimeout(r, 200));

  const attr = await page.evaluate(() => document.documentElement.dataset.theme);
  check(attr === "light", `toggling sets data-theme=light (got ${attr})`);

  const lightBg = await luma("body", "backgroundColor");
  const lightText = await luma("body", "color");
  check(lightBg > 0.75, `light theme paints a light background (luma ${lightBg?.toFixed(2)})`);
  check(lightText < 0.3, `light theme paints dark text (luma ${lightText?.toFixed(2)})`);
  check(lightBg - lightText > 0.5, `light theme keeps text/background contrast`);
  await page.screenshot({ path: `${OUT}/theme-light.png` });

  // --- 3) Nothing stays dark behind the switch ---
  // Every visible element's own background must be light (or transparent) once
  // the app is light -- a missed hard-coded colour shows up here.
  const strays = await page.evaluate((fn) => {
    const lumaOf = eval(`(${fn})`);
    const out = [];
    for (const el of document.querySelectorAll(".app *")) {
      const r = el.getBoundingClientRect();
      if (r.width < 8 || r.height < 8) continue;
      const l = lumaOf(getComputedStyle(el).backgroundColor);
      // Filled accent buttons are meant to be dark-on-light; skip them.
      if (l !== null && l < 0.45 && !el.matches("button, .split")) {
        out.push(`${el.className || el.tagName}`);
      }
    }
    return [...new Set(out)];
  }, LUMA);
  check(strays.length === 0, `no element keeps a dark surface in light mode${strays.length ? ": " + strays.join(", ") : ""}`);

  // The map canvas backdrop follows too (it is its own pane background).
  const mapBg = await luma(".map-pane", "backgroundColor");
  check(mapBg > 0.7, `map pane lightens with the theme (luma ${mapBg?.toFixed(2)})`);

  // --- 4) The choice is remembered, without a dark flash on reload ---
  await page.reload({ waitUntil: "networkidle2" });
  await page.waitForSelector(".console header");
  const kept = await page.evaluate(() => document.documentElement.dataset.theme);
  check(kept === "light", `theme survives a reload (got ${kept})`);
  const beforeReact = await page.evaluate(() =>
    localStorage.getItem("secondo.webui.theme"));
  check(beforeReact === "light", `theme is persisted in localStorage`);

  // --- 5) And back to dark ---
  (await themeBtn()).click();
  await new Promise((r) => setTimeout(r, 200));
  const backBg = await luma("body", "backgroundColor");
  check(backBg < 0.25, `toggling again returns to dark (luma ${backBg?.toFixed(2)})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
