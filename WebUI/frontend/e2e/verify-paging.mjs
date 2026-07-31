// Verifies that a relation larger than one page can be browsed and edited: the
// pager steps through it, sorting reorders the *relation* rather than the page
// on screen, and pending edits made on different pages are written by one save.
//
// It works on a relation it creates and drops itself, so it never mutates the
// shipped berlintest data.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const REL = "webuipagingtest";
const ROWS = 450; // more than two default pages of 200

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});
const page = await browser.newPage();
await page.setViewport({ width: 1200, height: 800 });
page.on("pageerror", (e) => console.log("[pageerror]", e.message));

const wait = (ms) => new Promise((r) => setTimeout(r, ms));

async function run(cmd) {
  const before = await page.$$eval(".log .entry", (els) => els.length);
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => (el.value = ""));
  await page.type(".input textarea", cmd);
  await page.keyboard.press("Enter");
  await page.waitForFunction(
    (n) => document.querySelectorAll(".log .entry").length > n,
    { timeout: 60000 },
    before
  );
  await wait(150);
}

async function clickButton(scope, label) {
  const handle = await page.evaluateHandle(
    (sel, text) =>
      [...document.querySelectorAll(`${sel} button`)].find((b) =>
        b.textContent.includes(text)
      ),
    scope,
    label
  );
  const el = handle.asElement();
  if (!el) throw new Error(`no "${label}" button in ${scope}`);
  await el.click();
}

/** Click a pager button and wait for the page it loads. */
async function pageTo(label) {
  const before = await range();
  await clickButton(".tv-pager", label);
  await page.waitForFunction(
    (was) => document.querySelector(".tv-range")?.textContent.trim() !== was,
    { timeout: 30000 },
    before
  );
  await wait(200);
}

const range = () =>
  page.$eval(".tv-range", (e) => e.textContent.trim()).catch(() => null);
const bodyRows = () => page.$$eval(".tv-grid tbody tr", (els) => els.length);
const firstColumn = () =>
  page.$$eval(".tv-grid tbody tr", (rows) =>
    rows.map((r) => {
      const c = r.querySelector("td:not(.tv-gutter)");
      return c?.querySelector("input")?.value ?? c?.textContent.trim() ?? "";
    })
  );

await page.goto(URL, { waitUntil: "networkidle0" });
let fails = 0;
const fail = (m) => {
  console.log("FAIL: " + m);
  fails++;
};

await run("open database berlintest");

// --- 0) `query <Rel>` is paged from the start, not capped ----------------
// The reported bug: a large relation opened as a truncated table, grew a pager
// only after ✎ edit, and kept it after ✕ discard. It is the *same* view now.
await run("query Orte"); // 506 rows: more than one page, nothing written
await page.waitForSelector(".tv-grid", { timeout: 30000 });
await wait(400);
if ((await page.$(".tv-warn")) !== null)
  fail("querying a whole relation still reports a truncated table");
if ((await range()) !== "rows 1–200 of 506")
  fail(`a queried relation is not paged: ${JSON.stringify(await range())}`);
const readOnlyHeaders = await page.$$eval(".tv-grid th", (e) =>
  e.map((x) => x.textContent)
);
if (readOnlyHeaders.some((h) => h.includes("TID")))
  fail("a table that is only being read shows the TID column");

await clickButton(".tv-head", "edit");
await page.waitForFunction(
  () =>
    [...document.querySelectorAll(".tv-grid th")].some((t) =>
      t.textContent.includes("TID")
    ),
  { timeout: 30000 }
);
await wait(300);
if ((await range()) !== "rows 1–200 of 506")
  fail(`edit mode changed the page: ${JSON.stringify(await range())}`);
await clickButton(".tv-head", "discard");
await wait(400);
if ((await range()) !== "rows 1–200 of 506")
  fail(`discard changed the page: ${JSON.stringify(await range())}`);

await run(`delete ${REL}`); // in case an earlier run left one behind
await run(`let ${REL} = intstream(1, ${ROWS}) transformstream consume`);
// `let` is a kernel command, which does not make the catalog re-read itself.
await run("close database");
await run("open database berlintest");

// Open it from the catalog's ▤ button, the path that loads a relation with TIDs.
await page.waitForFunction(
  (name) =>
    [...document.querySelectorAll(".cat-objs li")].some((e) =>
      e.textContent.includes(name)
    ),
  { timeout: 30000 },
  REL
);
const entry = await page.evaluateHandle(
  (name) =>
    [...document.querySelectorAll(".cat-objs li")]
      .find((e) => e.textContent.includes(name))
      ?.querySelector(".cat-table"),
  REL
);
if (!entry.asElement()) fail(`${REL} has no "open as table" button in the catalog`);
await entry.asElement().click();
await page.waitForSelector(".tv-grid", { timeout: 30000 });
await wait(400);

// --- 1) one page, and the true total ------------------------------------
const rows1 = await bodyRows();
const range1 = await range();
console.log(`page 1: ${rows1} rows, range ${JSON.stringify(range1)}`);
if (rows1 !== 200) fail(`expected a 200-row first page, got ${rows1}`);
if (range1 !== `rows 1–200 of ${ROWS}`)
  fail(`the pager reads ${JSON.stringify(range1)}`);
// The header counts the relation, not the page: capping is over.
const count = await page.$eval(".tv-count", (e) => e.textContent.trim());
if (count !== `${ROWS} rows`) fail(`the header reads ${JSON.stringify(count)}`);
if ((await page.$(".tv-warn")) !== null)
  fail("a paged relation was reported as truncated");

const first1 = (await firstColumn())[0];
if (first1 !== "1") fail(`the first page starts at ${JSON.stringify(first1)}`);

// --- 2) stepping forward and back ---------------------------------------
await pageTo("›");
if ((await range()) !== `rows 201–400 of ${ROWS}`)
  fail(`after Next the pager reads ${JSON.stringify(await range())}`);
if ((await firstColumn())[0] !== "201")
  fail(`page 2 starts at ${JSON.stringify((await firstColumn())[0])}`);

await pageTo("››");
const lastRange = await range();
console.log(`last page: ${JSON.stringify(lastRange)}, ${await bodyRows()} rows`);
if (lastRange !== `rows 401–${ROWS} of ${ROWS}`)
  fail(`the last page reads ${JSON.stringify(lastRange)}`);
if ((await bodyRows()) !== ROWS - 400)
  fail(`the last page has ${await bodyRows()} rows`);
// The end of the relation: there is nowhere further to go.
const nextDisabled = await page.evaluate(
  () =>
    [...document.querySelectorAll(".tv-pager button")].find((b) =>
      b.title.includes("Next")
    )?.disabled
);
if (!nextDisabled) fail("Next is still enabled on the last page");

await pageTo("‹‹");
if ((await range()) !== `rows 1–200 of ${ROWS}`)
  fail(`First did not return to the start: ${JSON.stringify(await range())}`);

// --- 3) the page size is the user's choice ------------------------------
await page.select(".tv-pagesize select", "100");
await page.waitForFunction(
  () => document.querySelectorAll(".tv-grid tbody tr").length === 100,
  { timeout: 30000 }
);
if ((await range()) !== `rows 1–100 of ${ROWS}`)
  fail(`after resizing the pager reads ${JSON.stringify(await range())}`);
await page.select(".tv-pagesize select", "200");
await page.waitForFunction(
  () => document.querySelectorAll(".tv-grid tbody tr").length === 200,
  { timeout: 30000 }
);

// --- 4) sorting reorders the relation, not the page ----------------------
// Descending by Elem must put 450 on the *first* page. Sorting in the browser
// could only ever have reached 200.
await page.click(".tv-grid th"); // asc
await wait(600);
await page.click(".tv-grid th"); // desc
await page.waitForFunction(
  () =>
    document.querySelector(".tv-grid tbody tr td:not(.tv-gutter)")?.textContent.trim() !==
    "1",
  { timeout: 30000 }
);
await wait(300);
const top = (await firstColumn())[0];
console.log(`first row when sorted descending: ${top}`);
if (top !== String(ROWS))
  fail(`sorting only reordered the page: the first row is ${top}, not ${ROWS}`);
if ((await range()) !== `rows 1–200 of ${ROWS}`)
  fail(`sorting lost the total: ${JSON.stringify(await range())}`);

// Back to ascending order for the edit checks.
await page.click(".tv-grid th");
await wait(800);

// --- 5) edits made on two pages are saved together -----------------------
await clickButton(".tv-head", "edit");
await page.waitForFunction(
  () =>
    [...document.querySelectorAll(".tv-grid th")].some((t) =>
      t.textContent.includes("TID")
    ),
  { timeout: 30000 }
);
await wait(300);

async function editFirstCell(value) {
  const cell = await page.$(".tv-grid tbody tr td input[type=text]");
  if (!cell) throw new Error("no editable cell");
  await cell.click();
  await page.keyboard.down("Control");
  await page.keyboard.press("KeyA");
  await page.keyboard.up("Control");
  await cell.type(value);
  await wait(200);
}

await editFirstCell("9001");
await pageTo("›"); // page 2 -- the pending change must survive the move
const pending = await page.$eval(".tv-pending", (e) => e.textContent).catch(() => "");
console.log(`pending banner on page 2: ${JSON.stringify(pending.trim())}`);
if (!pending.includes("1 changed"))
  fail("the edit made on page 1 was lost by paging");
if (!pending.includes("across all pages"))
  fail("the pending banner does not say the changes span pages");
await editFirstCell("9002");

await clickButton(".tv-head", "save");
await page.waitForFunction(
  () => !document.querySelector(".tv-pending") || document.querySelector(".tv-error"),
  { timeout: 60000 }
);
await wait(600);
if ((await page.$(".tv-error")) !== null)
  fail("saving across pages reported an error: " +
    (await page.$eval(".tv-error", (e) => e.textContent)));

// Both are really in SECONDO, and each is on a different page of it.
await run(`query ${REL} feed filter[.Elem = 9001] count`);
await run(`query ${REL} feed filter[.Elem = 9002] count`);
const answers = await page.evaluate(() =>
  [...document.querySelectorAll(".log .entry")]
    .slice(-2)
    .map((e) => e.textContent.replace(/\s+/g, " "))
);
console.log(`counts: ${JSON.stringify(answers)}`);
if (!answers.every((a) => a.includes("(int 1)")))
  fail("an edit made on one of the two pages never reached SECONDO");

await page.screenshot({ path: "e2e/out/paging.png" });

// --- cleanup -------------------------------------------------------------
await run(`delete ${REL}`);

await browser.close();
console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
