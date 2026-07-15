// Regression: a moving-object (mpoint) layer must fit correctly under the
// BerlinMOD projection. The temporal bbox used to stay in raw BBBike
// coordinates while the trips were projected, so the view fell back to a
// whole-world Mercator view (reported for `train7`).
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

const tileLon = (x, z) => (x / 2 ** z) * 360 - 180;
const tileLat = (y, z) => {
  const n = Math.PI - (2 * Math.PI * y) / 2 ** z;
  return (180 / Math.PI) * Math.atan(0.5 * (Math.exp(n) - Math.exp(-n)));
};

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

  const tiles = [];
  page.on("response", (r) => {
    const m = r.url().match(/tile\.openstreetmap\.org\/(\d+)\/(\d+)\/(\d+)\.png/);
    if (m && r.status() === 200) {
      const z = +m[1], x = +m[2], y = +m[3];
      tiles.push({ z, lon: tileLon(x + 0.5, z), lat: tileLat(y + 0.5, z) });
    }
  });

  async function runCmd(cmd) {
    await page.click(".input input");
    await page.$eval(".input input", (el) => (el.value = ""));
    await page.type(".input input", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 800));
  }
  // A single mpoint draws only a faint context path, a thin trail and one dot,
  // so use a lower brightness threshold than the multi-train tests.
  const drawn = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const px = new Uint8Array(c.width * c.height * 4);
      gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, px);
      let n = 0;
      for (let i = 0; i < px.length; i += 4)
        if (px[i + 3] > 10 && Math.max(px[i], px[i + 1], px[i + 2]) > 40) n++;
      return n;
    });

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query train7"); // a bare mpoint: temporal-only layer
  // Let the animation advance so the trail has actually been drawn.
  await new Promise((r) => setTimeout(r, 2000));

  // Flat first.
  const flatPx = await drawn();
  check(flatPx > 50, `mpoint visible in flat view (${flatPx})`);

  // Now the projection: must fit Berlin, not the globe.
  await page.select(".projection-ctl select", "berlinmod");
  await page.waitForSelector(".maplibregl-map", { timeout: 8000 });
  await new Promise((r) => setTimeout(r, 3500));

  check(tiles.length > 0, `OSM tiles loaded (${tiles.length})`);
  const nearBerlin = tiles.some(
    (t) => Math.abs(t.lon - 13.4) < 1.0 && Math.abs(t.lat - 52.5) < 1.0
  );
  check(nearBerlin, `mpoint basemap centered on Berlin, not the world`);
  // A whole-world view would be zoom 0-3; a fitted city view is much closer in.
  const maxZoom = Math.max(...tiles.map((t) => t.z));
  check(maxZoom >= 8, `zoomed to city level, not world (max tile z=${maxZoom})`);

  const geoPx = await drawn();
  check(geoPx > 50, `mpoint drawn under projection (${geoPx})`);

  await page.screenshot({ path: `${OUT}/mpoint-projected.png` });
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
