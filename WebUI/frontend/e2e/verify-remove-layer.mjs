// Regression: removing the last layer while a projection is active must not
// crash the map (fitGeographic used to destructure an undefined bbox).
import { createRequire } from "module";
const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

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
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));

  async function runCmd(cmd) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 700));
  }

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query thecenter");
  await page.select(".projection-ctl select", "berlinmod"); // projection active
  await page.waitForSelector(".maplibregl-map", { timeout: 8000 });
  await new Promise((r) => setTimeout(r, 800));

  // Remove the (only/last) layer via the ✕ button.
  const removeBtn = await page.$(".lp-item .lp-x");
  await removeBtn.click();
  await new Promise((r) => setTimeout(r, 800));

  const layerItems = await page.$$eval(".lp-item", (els) => els.length).catch(() => 0);
  check(layerItems === 0, "last layer removed");
  const crash = errors.find((e) => /bbox|Symbol\.iterator|undefined/.test(e));
  check(!crash, `no crash removing last layer under projection (errors: ${errors.length ? errors.join(" | ") : "none"})`);

  // The app must still be alive: run another query and see it render.
  await runCmd("query mehringdamm");
  await new Promise((r) => setTimeout(r, 600));
  const back = await page.$$eval(".lp-item", (els) => els.length).catch(() => 0);
  check(back === 1, "app still works after removal (new query adds a layer)");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
