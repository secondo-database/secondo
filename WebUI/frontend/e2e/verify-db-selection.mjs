// Verifies that opening a database is *visible*: the catalog marks the open one
// and the console header says which one queries run against.
//
// The regression this guards: `list databases` reports names uppercased while
// the session remembers the name as the open command spelled it, so a literal
// comparison never matched and the selected chip was never marked. The same
// comparison guarded the open path, which made clicking the already-open
// database close and reopen it for nothing.
//
// Needs only berlintest.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const DB = "BERLINTEST";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };
const wait = (ms) => new Promise((r) => setTimeout(r, ms));

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  await page.goto(URL, { waitUntil: "networkidle0" });
  await page.waitForSelector(".cat-db", { timeout: 15000 });

  const clickDb = async (name) => {
    const handle = await page.evaluateHandle(
      (n) => [...document.querySelectorAll(".cat-db")].find((b) => b.textContent.trim() === n),
      name
    );
    const el = handle.asElement();
    if (!el) throw new Error(`no catalog chip for ${name}`);
    await el.click();
  };
  const activeDbs = () =>
    page.$$eval(".cat-db.active", (els) => els.map((e) => e.textContent.trim()));
  const logLines = () =>
    page.$$eval(".log .cmd-text", (els) => els.map((e) => e.textContent.trim()));

  // 1) Nothing is open at first: no chip is marked, and the header says so.
  check((await activeDbs()).length === 0, "no database marked before one is opened");
  const before = await page.$eval(".console header", (e) => e.textContent);
  check(before.includes("no database open"), "header states that no database is open");

  // 2) Opening one marks exactly that chip.
  await clickDb(DB);
  await page.waitForSelector(".cat-obj", { timeout: 15000 });
  await wait(200);
  const marked = await activeDbs();
  check(marked.length === 1 && marked[0] === DB,
        `exactly the open database is marked (${JSON.stringify(marked)})`);
  check(await page.$eval(`.cat-db.active`, (e) => e.getAttribute("aria-pressed")) === "true",
        "the marked chip reports aria-pressed");

  // 3) The header names it too -- this is the only indicator once the catalog
  //    collapses to its rail, so it must not depend on the catalog being shown.
  const chip = await page.$eval(".st-chip.db", (e) => e.textContent.trim());
  check(chip.includes(DB), `header chip names the open database (${chip})`);

  await page.click(".cat-collapse");
  await wait(200);
  const collapsed = await page.$eval(".st-chip.db", (e) => e.textContent.trim());
  check(collapsed.includes(DB), "header still names it with the catalog collapsed");
  await page.click(".rail-btn");
  await page.waitForSelector(".cat-db", { timeout: 5000 });

  // 4) The optimizer chip is present either way -- the capability is stated, not
  //    left to be discovered by a query that fails.
  const sql = await page.$eval(".st-chip.ok, .st-chip.warn", (e) => e.textContent.trim());
  check(/SQL (ready|off)/.test(sql), `server SQL capability is stated (${sql})`);

  // 5) Clicking the database that is already open is a no-op: it must not close
  //    and reopen it, which would throw away every layer on the map.
  const linesBefore = await logLines();
  await clickDb(DB);
  await wait(1200);
  const linesAfter = await logLines();
  const added = linesAfter.slice(linesBefore.length);
  check(added.length === 0,
        `re-clicking the open database runs nothing (${JSON.stringify(added)})`);
  check((await activeDbs()).length === 1, "it stays marked afterwards");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
