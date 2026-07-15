// Verifies moving-object animation: queries Trains, pauses the timeline, seeks
// to two different instants, and asserts (a) the TripsLayer draws and (b) the
// moving head's position changes between the two instants (i.e. it animates).
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
    await page.click(".input input");
    await page.$eval(".input input", (el) => (el.value = ""));
    await page.type(".input input", command);
    await page.keyboard.press("Enter");
  }

  // Move the timeline slider to a fraction of the domain (0..1) and let React
  // apply it. The animation is paused first so the value sticks.
  async function seekFraction(frac) {
    await page.$eval(
      ".tl-range",
      (el, f) => {
        const max = Number(el.max);
        const nativeSetter = Object.getOwnPropertyDescriptor(
          window.HTMLInputElement.prototype,
          "value"
        ).set;
        nativeSetter.call(el, String(max * f));
        el.dispatchEvent(new Event("input", { bubbles: true }));
      },
      frac
    );
    // A single thin trail needs a moment to be drawn; too short a wait here
    // sampled an empty frame and made this test flaky.
    await new Promise((r) => setTimeout(r, 1500));
  }

  // Centroid + count of the trail pixels. The trail is drawn in the *layer's*
  // colour, so read that from the layer swatch rather than hard-coding a hue
  // (the palette is a design decision and must not break this test). The faint
  // full-trajectory context path blends far darker than the trail, so a tight
  // tolerance around the full-intensity colour isolates the moving trail.
  async function trailStats() {
    return page.evaluate(() => {
      const swatch = document.querySelector(".lp-swatch");
      const m = getComputedStyle(swatch).backgroundColor.match(/\d+/g);
      const [tr, tg, tb] = m.map(Number);
      const c = document.querySelector(".mapview canvas");
      const gl =
        c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
        c.getContext("webgl", { preserveDrawingBuffer: true });
      const w = c.width,
        h = c.height;
      const px = new Uint8Array(w * h * 4);
      gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, px);
      let n = 0,
        sx = 0,
        sy = 0;
      for (let i = 0; i < px.length; i += 4) {
        const a = px[i + 3];
        if (
          a > 10 &&
          Math.abs(px[i] - tr) < 60 &&
          Math.abs(px[i + 1] - tg) < 60 &&
          Math.abs(px[i + 2] - tb) < 60
        ) {
          const p = i / 4;
          sx += p % w;
          sy += Math.floor(p / w);
          n++;
        }
      }
      return n ? { n, cx: sx / n, cy: sy / n } : { n: 0, cx: 0, cy: 0 };
    });
  }

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await page.waitForFunction(
    () => document.querySelectorAll(".entry").length >= 1,
    { timeout: 8000 }
  );

  await runCmd("query Trains feed head[3] project[Id, Trip] consume");
  await page.waitForSelector(".timeline", { timeout: 12000 });
  // Pause so seeks are stable.
  await page.click(".tl-play");
  await new Promise((r) => setTimeout(r, 300));

  await seekFraction(0.2);
  const a = await trailStats();
  await page.screenshot({ path: `${OUT}/anim-t20.png` });

  await seekFraction(0.8);
  const b = await trailStats();
  await page.screenshot({ path: `${OUT}/anim-t80.png` });

  const moved = Math.hypot(a.cx - b.cx, a.cy - b.cy);
  console.log("t=20%:", JSON.stringify(a));
  console.log("t=80%:", JSON.stringify(b));
  console.log("head moved (px):", moved.toFixed(1));

  const ok = a.n > 20 && b.n > 20 && moved > 15;
  console.log(ok ? "PASS animation" : "FAIL animation");
  process.exitCode = ok ? 0 : 1;
} finally {
  await browser.close();
}
