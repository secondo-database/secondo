// Verifies that the hover tooltip appears at the pointer.
//
// deck.gl 9.4 places it relative to its React wrapper's widget root, which sits
// below the full-height canvas unless styles.css pins it -- and the tooltip
// then came out one map height above the pointer, mostly cut off at the top of
// the map. deck puts the tooltip's top-left corner on the pointer, so that is
// what is compared.
//
// Needs only berlintest; reads Kinos and creates nothing.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

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
const wait = (ms) => new Promise((r) => setTimeout(r, ms));

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  // See verify-table.mjs, which this is copied from.
  async function runCmd(cmd) {
    const before = await page.$$eval(".log .entry", (els) => els.length);
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await page.waitForFunction(
      (n) =>
        document.querySelectorAll(".log .entry").length > n &&
        !document.querySelector(".log .entry.pending"),
      { timeout: 30000 },
      before
    );
    await wait(200);
  }

  /** A filled pixel in the interior of a drawn shape, in page coordinates; see
   *  verify-select-link.mjs, which this is copied from. */
  const findTarget = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      if (!c) return null;
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
      const R = 4;
      for (let y = R; y < h - R; y++) {
        for (let x = R; x < w - R; x++) {
          if (filled(x, y) && filled(x + R, y) && filled(x - R, y) &&
              filled(x, y + R) && filled(x, y - R)) {
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

  await page.goto(URL, { waitUntil: "networkidle2" });
  await page.waitForSelector(".cat-db-select", { timeout: 15000 });
  await runCmd("open database berlintest");
  await runCmd("query Kinos");
  await wait(600);

  let tip = null, at = null;
  for (let attempt = 0; attempt < 6 && tip === null; attempt++) {
    at = await findTarget();
    if (!at) { await wait(300); continue; }
    // Arrive from a step away: deck only picks on a pointer *move*.
    await page.mouse.move(at.x - 5, at.y - 5);
    await wait(100);
    await page.mouse.move(at.x, at.y);
    await wait(400);
    tip = await page.evaluate(() => {
      const el = document.querySelector(".deck-tooltip");
      if (!el || el.style.display === "none") return null;
      const r = el.getBoundingClientRect();
      return { left: r.left, top: r.top, text: el.innerText };
    });
  }
  check(tip !== null, `hovering a feature shows a tooltip${tip ? ` (${tip.text.split("\n")[0]})` : ""}`);
  if (tip) {
    const dx = Math.abs(tip.left - at.x), dy = Math.abs(tip.top - at.y);
    check(dx <= 2 && dy <= 2,
          `the tooltip sits at the pointer (${Math.round(tip.left)},${Math.round(tip.top)} for ` +
          `${Math.round(at.x)},${Math.round(at.y)})`);
  }

  // The root now covers the map, so it must not take the pointer: a drag on the
  // map still has to reach deck's canvas.
  const through = await page.evaluate(({ x, y }) =>
    document.elementFromPoint(x, y)?.id, at ?? { x: 400, y: 300 });
  check(through === "deckgl-overlay", `the pointer still reaches the map canvas (${through})`);
} finally {
  await browser.close();
}
process.exit(fails === 0 ? 0 : 1);
