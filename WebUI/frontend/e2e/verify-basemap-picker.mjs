// Verifies the basemap picker: the choice of raster basemap under geographic
// mode, that it is absent when there is no basemap to pick, that switching it
// really changes which tiles are fetched, that the label contrast follows the
// chosen basemap rather than the mode, and that the choice survives a reload.
//
// Needs only berlintest. Reaches tile hosts other than OSM (Esri), so a
// restrictive proxy will fail the switching steps on network rather than logic.
//
// This checks the wiring, not the provider's terms: a basemap can be dead and
// still answer 200 with a watermarked tile (see the CARTO note in the README),
// which no assertion here can see. That is what the screenshots are for.
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
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1280, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  // Count tiles per source: the only way to prove a switch changed what is
  // actually fetched rather than only what the dropdown says. Satellite and
  // Dark are both Esri, so the host alone no longer separates them -- bucket on
  // the service path instead.
  const tiles = { osm: 0, imagery: 0, darkgray: 0 };
  page.on("response", (r) => {
    if (r.status() !== 200) return;
    const u = r.url();
    if (u.includes("tile.openstreetmap.org")) tiles.osm++;
    if (u.includes("World_Imagery")) tiles.imagery++;
    if (u.includes("World_Dark_Gray_Base")) tiles.darkgray++;
  });

  await page.goto(URL, { waitUntil: "networkidle0" });
  await sleep(800);

  // 1) Open berlintest and draw something. BBBike coordinates are not lon/lat,
  //    so the map stays Cartesian -- where there is no basemap at all.
  const db = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".cat-db")].find((b) => b.textContent === "BERLINTEST"));
  await db.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 8000 });
  const line = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".cat-obj")].find((b) =>
      b.textContent.includes("BGrenzenLine")));
  await line.asElement().click();
  await page.waitForFunction(
    () => document.querySelectorAll(".lp-item").length > 0, { timeout: 10000 });

  check((await page.$eval(".mapview", (e) => e.dataset.geographic)) === "false",
        "Cartesian mode (BBBike coordinates are not lon/lat)");
  check((await page.$(".basemap-ctl")) === null,
        "picker absent in Cartesian mode -- nothing to pick");

  // 2) Project to WGS84: geographic mode, picker appears on the default OSM.
  await page.select(".projection-ctl select", "berlinmod");
  await page.waitForSelector(".maplibregl-map", { timeout: 10000 });
  await page.waitForSelector(".basemap-ctl", { timeout: 5000 });
  check(true, "picker appears once the map is geographic");

  const osmState = await page.$eval(".mapview", (e) => ({
    basemap: e.dataset.basemap, onLight: e.dataset.onLight,
  }));
  check(osmState.basemap === "osm", `defaults to OpenStreetMap (${osmState.basemap})`);
  check(osmState.onLight === "true", "labels drawn for a light canvas over OSM");

  await sleep(3500); // let tiles load
  check(tiles.osm > 0, `OSM tiles loaded (${tiles.osm})`);
  await page.screenshot({ path: `${OUT}/basemap-osm.png` });

  // 3) Switch to satellite: different host, and the contrast must flip with it.
  //    Imagery is dark, so near-black label ink on a white halo would vanish.
  await page.select(".basemap-ctl select", "satellite");
  await sleep(4000);

  const satState = await page.$eval(".mapview", (e) => ({
    basemap: e.dataset.basemap, onLight: e.dataset.onLight,
  }));
  check(satState.basemap === "satellite", `switched to satellite (${satState.basemap})`);
  check(satState.onLight === "false",
        "label contrast follows the basemap, not the mode");
  check(tiles.imagery > 0, `Esri imagery tiles loaded (${tiles.imagery})`);
  await page.screenshot({ path: `${OUT}/basemap-satellite.png` });

  // 4) Switch to dark. Same host as satellite, so only the service path proves
  //    the switch reached the network.
  await page.select(".basemap-ctl select", "dark");
  await sleep(4000);

  const darkState = await page.$eval(".mapview", (e) => ({
    basemap: e.dataset.basemap, onLight: e.dataset.onLight,
  }));
  check(darkState.basemap === "dark", `switched to dark (${darkState.basemap})`);
  check(darkState.onLight === "false", "labels drawn for a dark canvas");
  check(tiles.darkgray > 0, `Esri dark-gray tiles loaded (${tiles.darkgray})`);
  await page.screenshot({ path: `${OUT}/basemap-dark.png` });

  // 5) The choice is a display preference, so it outlives the page.
  await page.reload({ waitUntil: "networkidle0" });
  await sleep(800);
  const remembered = await page.evaluate(() =>
    localStorage.getItem("secondo.webui.basemap"));
  check(remembered === "dark", `choice remembered across a reload (${remembered})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
