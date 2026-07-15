// Verifies the BerlinMOD projection: querying berlintest data and selecting the
// "BerlinMOD -> OSM" projection renders it on the OpenStreetMap basemap at the
// real Berlin location (checked via the tile coordinates actually requested).
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

// OSM tile -> lon/lat of tile center.
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

  const tileCenters = [];
  page.on("response", (r) => {
    const m = r.url().match(/tile\.openstreetmap\.org\/(\d+)\/(\d+)\/(\d+)\.png/);
    if (m && r.status() === 200) {
      const z = +m[1], x = +m[2], y = +m[3];
      tileCenters.push([tileLon(x + 0.5, z), tileLat(y + 0.5, z)]);
    }
  });

  async function runCmd(cmd) {
    await page.click(".input input");
    await page.$eval(".input input", (el) => (el.value = ""));
    await page.type(".input input", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 700));
  }

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query thecenter"); // a region in BBBike coordinates

  // Default: flat/Cartesian, no basemap.
  const mode = () => page.$eval(".mapview", (e) => e.dataset.geographic);
  check((await mode()) === "false", "flat by default (no basemap)");
  check(!(await page.$(".maplibregl-map")), "no MapLibre basemap while flat");

  // Switch projection to BerlinMOD -> OSM.
  await page.select(".projection-ctl select", "berlinmod");
  await page.waitForSelector(".maplibregl-map", { timeout: 8000 });
  const proj = await page.$eval(".mapview", (e) => e.dataset.projection);
  check(proj === "berlinmod", `map switched to the BerlinMOD projection (${proj})`);
  check((await mode()) === "true", "map is in geographic mode");
  check(!!(await page.$(".maplibregl-map")), "MapLibre basemap present");

  await new Promise((r) => setTimeout(r, 3500)); // let tiles load
  check(tileCenters.length > 0, `OSM tiles loaded (${tileCenters.length})`);

  // At least one requested tile must be centered near Berlin (~13.4E, 52.5N).
  const nearBerlin = tileCenters.some(
    ([lon, lat]) => Math.abs(lon - 13.4) < 1.0 && Math.abs(lat - 52.5) < 1.0
  );
  check(nearBerlin,
        `basemap is over Berlin (sample tile: ${tileCenters[0]?.map((v) => v.toFixed(2)).join(",")})`);

  await page.screenshot({ path: `${OUT}/berlinmod-osm.png` });
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
