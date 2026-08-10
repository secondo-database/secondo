// Verifies that SQL runs from the console through the in-kernel optimizer:
// the generated plan is shown, the result still goes through the ordinary
// GeoJSON pipeline onto the map, the "optimizer " prefix stops after
// optimizing, optimizer directives print their output, and a create/drop the
// optimizer carried out itself is reported as such.
//
// Skips (exit 0) against a server built or configured without the optimizer --
// the suite must stay green on a build without SWI-Prolog.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const API = process.env.WEBUI_API_BASE ?? "http://127.0.0.1:8000";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

// Ask the bridge up front whether this server can run SQL at all.
const state = await (await fetch(`${API}/api/databases`)).json();
if (!state.optimizer) {
  console.log("SKIP: this SECONDO server runs without the optimizer");
  process.exit(0);
}

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});
const page = await browser.newPage();
await page.setViewport({ width: 1200, height: 800 });
page.on("pageerror", (e) => console.log("[pageerror]", e.message));

await page.goto(URL, { waitUntil: "networkidle0" });
let fails = 0;

// Submit a command and wait for its console entry to appear. Waiting on the
// history rather than on a clock matters here: the server loads SWI-Prolog and
// the whole optimizer lazily, so the *first* SQL command of a session takes
// far longer than any later one.
async function run(cmd) {
  const before = (await page.$$(".log .entry")).length;
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => (el.value = ""));
  await page.type(".input textarea", cmd);
  await page.keyboard.press("Enter");
  await page.waitForFunction(
    (n) =>
      document.querySelectorAll(".log .entry").length > n &&
      // The entry now goes up when the command is *sent*, so a grown log
      // is not an answered command: wait for it to stop being pending too.
      !document.querySelector(".log .entry.pending"),
    { timeout: 120_000 },
    before
  );
}

// The text of the most recent console entry, and how many layers exist.
const lastEntry = () =>
  page.$$eval(".log .entry", (els) =>
    els.length ? els[els.length - 1].innerText : ""
  );
const layerCount = () => page.$$eval(".lp-item", (els) => els.length);

function check(name, ok, detail) {
  console.log(`${ok ? "ok  " : "FAIL"} ${name}${detail ? ` -- ${detail}` : ""}`);
  if (!ok) fails++;
}

await run("open database berlintest");

// 1) A plain SQL select: plan shown, result rendered on the map. This is the
//    whole design in one assertion -- the server optimized it, and the result
//    half went through the unchanged GeoJSON conversion.
const before = await layerCount();
await run("select * from kinos");
let entry = await lastEntry();
check("plan is shown", entry.includes("Optimized plan:"), entry.slice(0, 200));
check("result rendered", (await layerCount()) > before);

// 2) The "optimizer " prefix: optimize, report, do not execute.
const beforePrefix = await layerCount();
await run("optimizer select * from kinos");
entry = await lastEntry();
check("plan only is reported", entry.includes("Plan only"), entry.slice(0, 200));
check("plan only added no layer", (await layerCount()) === beforePrefix);

// 3) An optimizer directive prints what the Prolog goal produced.
await run("showOptions");
entry = await lastEntry();
check(
  "directive printed output",
  entry.length > "showOptions".length + 10 && !entry.includes("SECONDO error"),
  entry.slice(0, 200)
);

// 4) A create the optimizer carries out itself while translating.
await run("drop table sqltest"); // ignore failure: may not exist yet
await run("create table sqltest columns [a: int]");
entry = await lastEntry();
check(
  "DDL reported as executed by the optimizer",
  entry.includes("Executed by the optimizer"),
  entry.slice(0, 200)
);
await run("drop table sqltest");

// 5) A broken query surfaces the server's own message as an error.
await run("select * from nosuchrelation");
const errText = await page.$$eval(".log .entry pre.err", (els) =>
  els.length ? els[els.length - 1].innerText : ""
);
check("bad SQL surfaces an error", errText.length > 0, errText.slice(0, 200));

await browser.close();
console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
