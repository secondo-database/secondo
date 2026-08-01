// Regression: removing a layer can move the *remaining* data into the other
// coordinate space, and the view state has to follow it.
//
// `strassen` is in berlintest's Cartesian range; a lon/lat literal is
// geographic. Their union bbox is outside lon/lat, so the map draws Cartesian.
// Dropping strassen leaves the geographic bbox on its own. The map then
// switched to the MapView while still holding an orthographic {target, zoom},
// and deck.gl's controller asserted on the first pan or key press ("deck.gl:
// assertion failed", then makeViewport() undefined).
//
// The geographic half is a literal rather than a stored object on purpose:
// stock berlintest is entirely Cartesian, and the GPS objects that would do
// (wanderung_line, wanderung_mp) are not part of it.
import { createRequire } from "module";
const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

const GEO_LINE =
  "query [const line value ((13.3 52.5 13.5 52.6) (13.5 52.6 13.6 52.45))]";

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
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));

  async function runCmd(cmd) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 400));
  }

  // strassen is ~3000 features; wait for the layer rather than guessing a delay.
  const waitForLayers = (n) =>
    page
      .waitForFunction(
        (want) => document.querySelectorAll(".lp-item").length === want,
        { timeout: 30000 },
        n
      )
      .then(() => true, () => false);

  const geographic = () =>
    page.$eval(".mapview", (el) => el.dataset.geographic).catch(() => null);

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query strassen");
  check(await waitForLayers(1), "strassen drawn");
  await runCmd(GEO_LINE);
  check(await waitForLayers(2), "geographic literal drawn");
  await new Promise((r) => setTimeout(r, 600));
  check((await geographic()) === "false", "mixed bbox keeps the map Cartesian");

  // Newest layer sits on top of the panel, so strassen is the last item.
  const items = await page.$$(".lp-item");
  await (await items[items.length - 1].$(".lp-x")).click();
  check(await waitForLayers(1), "strassen removed");
  await new Promise((r) => setTimeout(r, 800));
  check((await geographic()) === "true", "remaining lon/lat layer flips the map geographic");

  // The crash only surfaced once the controller was asked for a viewport.
  await page.mouse.move(640, 400);
  await page.mouse.down();
  await page.mouse.move(700, 440, { steps: 8 });
  await page.mouse.up();
  await page.keyboard.press("ArrowRight");
  await new Promise((r) => setTimeout(r, 600));

  const crash = errors.find((e) => /assertion|unproject|makeViewport|undefined/.test(e));
  check(!crash, `pan and keyboard work after the flip (errors: ${errors.length ? errors.join(" | ") : "none"})`);
  check(!!(await page.$(".maplibregl-map")), "basemap is up in geographic mode");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
