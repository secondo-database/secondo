// Verifies Milestone 5: catalog DB/object browser, geographic MapLibre + OSM
// basemap for lon/lat data, and GeoJSON export.
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

  // Count successful OSM tile loads to prove the basemap really renders.
  let osmTiles = 0;
  page.on("response", (r) => {
    if (r.url().includes("tile.openstreetmap.org") && r.status() === 200) osmTiles++;
  });

  await page.goto(URL, { waitUntil: "networkidle0" });
  await new Promise((r) => setTimeout(r, 800));

  // 1) Catalog lists databases.
  const dbs = await page.$$eval(".cat-db", (els) => els.map((e) => e.textContent));
  check(dbs.includes("SYMTRAJSMALL"), `catalog lists databases (${dbs.join(",")})`);

  // 2) Open SYMTRAJSMALL from the catalog -> object list appears.
  const target = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".cat-db")].find((b) => b.textContent === "SYMTRAJSMALL"));
  await target.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 8000 });
  const objNames = await page.$$eval(".cat-obj .cat-oname", (els) =>
    els.map((e) => e.textContent));
  check(objNames.includes("EdgesExtDo"),
        `objects listed after open (${objNames.length} objects)`);

  // 3) Click EdgesExtDo (lon/lat sline) -> geographic mode + OSM basemap.
  const edges = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".cat-obj")].find((b) =>
      b.textContent.includes("EdgesExtDo")));
  await edges.asElement().click();
  await page.waitForSelector(".maplibregl-map", { timeout: 10000 });
  check((await page.$eval(".mapview", (e) => e.dataset.geographic)) === "true",
        "geographic mode activated (MapLibre basemap present)");

  const hasMapLibre = await page.$(".maplibregl-map");
  check(!!hasMapLibre, "MapLibre basemap container present");

  await new Promise((r) => setTimeout(r, 3500)); // let tiles load
  check(osmTiles > 0, `OSM tiles loaded (${osmTiles})`);
  await page.screenshot({ path: `${OUT}/map-geo.png` });

  // 4) Export visible layers as GeoJSON (capture the blob).
  await page.evaluate(() => {
    window.__exports = [];
    const orig = URL.createObjectURL;
    URL.createObjectURL = (blob) => {
      blob.text().then((t) => window.__exports.push(t));
      return orig(blob);
    };
  });
  const exportBtn = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".lp-clear")].find((b) => b.textContent === "export"));
  await exportBtn.asElement().click();
  await new Promise((r) => setTimeout(r, 600));
  const exported = await page.evaluate(() => window.__exports[0] ?? null);
  let ok = false;
  if (exported) {
    const fc = JSON.parse(exported);
    ok = fc.type === "FeatureCollection" && fc.features.length > 0 &&
         !!fc.features[0].geometry;
  }
  check(ok, `export produced a GeoJSON FeatureCollection with features`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
