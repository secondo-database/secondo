// Verifies that opening a database is *visible*: the catalog's selector shows
// the open one and the console header says which one queries run against.
//
// The regression this guards: `list databases` reports names uppercased while
// the session remembers the name as the open command spelled it, so a literal
// comparison never matched and the open database was never shown as selected.
// The same comparison guarded the open path, which made picking the already-open
// database close and reopen it for nothing.
//
// It also guards the space the selector takes: one row however many databases
// the server holds, with the object list scrolling on its own underneath.
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
  await page.waitForSelector(".cat-db-select", { timeout: 15000 });

  const selected = () => page.$eval(".cat-db-select", (e) => e.value);
  const logLines = () =>
    page.$$eval(".log .cmd-text", (els) => els.map((e) => e.textContent.trim()));

  // 1) Nothing is open at first: the selector shows its prompt, and the header
  //    says so.
  check(await selected() === "", "no database selected before one is opened");
  const prompt = await page.$eval(".cat-db-select", (e) => e.selectedOptions[0]?.textContent);
  check(/open a database/.test(prompt ?? ""), `the selector prompts to open one (${prompt})`);
  const before = await page.$eval(".console header", (e) => e.textContent);
  check(before.includes("no database open"), "header states that no database is open");

  // 2) Opening one selects it, and the prompt goes: it is no database to pick.
  await page.select(".cat-db-select", DB);
  await page.waitForSelector(".cat-obj", { timeout: 15000 });
  await wait(200);
  check(await selected() === DB, `the open database is selected (${await selected()})`);
  check(await page.$('.cat-db-select option[value=""]') === null,
        "the prompt is gone once a database is open");

  // 3) The header names it too -- this is the only indicator once the catalog
  //    collapses to its rail, so it must not depend on the catalog being shown.
  const chip = await page.$eval(".st-chip.db", (e) => e.textContent.trim());
  check(chip.includes(DB), `header chip names the open database (${chip})`);

  await page.click(".cat-collapse");
  await wait(200);
  const collapsed = await page.$eval(".st-chip.db", (e) => e.textContent.trim());
  check(collapsed.includes(DB), "header still names it with the catalog collapsed");
  await page.click(".rail-btn");
  await page.waitForSelector(".cat-db-select", { timeout: 5000 });

  // 4) The optimizer chip is present either way -- the capability is stated, not
  //    left to be discovered by a query that fails.
  const sql = await page.$eval(".st-chip.ok, .st-chip.warn", (e) => e.textContent.trim());
  check(/SQL (ready|off)/.test(sql), `server SQL capability is stated (${sql})`);

  // 5) Every database is offered, in one row of the panel.
  const total = await page.evaluate(async () =>
    (await (await fetch("/api/databases")).json()).databases.length);
  const offered = await page.$$eval(".cat-db-select option", (els) => els.length);
  check(offered === total, `the selector offers every database (${offered}/${total})`);
  const rowH = await page.$eval(".cat-dbs", (e) => e.getBoundingClientRect().height);
  check(rowH < 48, `the selector takes one row (${rowH}px)`);

  // 6) Picking the database that is already open is a no-op: it must not close
  //    and reopen it, which would throw away every layer on the map.
  const linesBefore = await logLines();
  await page.select(".cat-db-select", DB);
  await wait(1200);
  const linesAfter = await logLines();
  const added = linesAfter.slice(linesBefore.length);
  check(added.length === 0,
        `re-picking the open database runs nothing (${JSON.stringify(added)})`);
  check(await selected() === DB, "it stays selected afterwards");

  // 7) The object list scrolls by itself: at the bottom of berlintest's objects
  //    the selector is still in view above them.
  const objs = await page.$eval(".cat-objs", (e) => {
    e.scrollTop = e.scrollHeight;
    return { sh: e.scrollHeight, ch: e.clientHeight };
  });
  check(objs.sh > objs.ch, `the object list overflows on its own (${objs.sh} > ${objs.ch})`);
  await wait(100);
  const inView = await page.evaluate(() => {
    const sel = document.querySelector(".cat-db-select").getBoundingClientRect();
    const cat = document.querySelector(".catalog").getBoundingClientRect();
    return sel.top >= cat.top && sel.bottom <= cat.bottom;
  });
  check(inView, "the selector stays in view while the objects scroll");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
