// Verifies the table view and relation editing (Milestone 9): a relation result
// opens as a table, a stored relation can be edited (change a cell, add a row,
// delete a row) and the changes really reach SECONDO.
//
// It works on a relation it creates and drops itself, so it never mutates the
// shipped berlintest data.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const REL = "webuitabletest";

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

// Wait for the answer rather than guessing how long it takes: a command is done
// when the console log has one more entry than before.
async function run(cmd) {
  const before = await page.$$eval(".log .entry", (els) => els.length);
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => (el.value = ""));
  await page.type(".input textarea", cmd);
  await page.keyboard.press("Enter");
  await page.waitForFunction(
    (n) => document.querySelectorAll(".log .entry").length > n,
    { timeout: 30000 },
    before
  );
  await wait(150); // let the render that follows the state update settle
}

/** Click the button in `scope` whose text contains `label`. */
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

/** Enter edit mode and wait for the reload that brings the TIDs. */
async function startEditing() {
  await clickButton(".tv-head", "edit");
  await page.waitForFunction(
    () =>
      [...document.querySelectorAll(".tv-grid th")].some((t) =>
        t.textContent.includes("TID")
      ),
    { timeout: 30000 }
  );
}

/** Save and wait for the reload that follows a successful commit. */
async function save() {
  await clickButton(".tv-head", "save");
  await page.waitForFunction(
    () => !document.querySelector(".tv-pending") || document.querySelector(".tv-error"),
    { timeout: 30000 }
  );
  await wait(400);
}

const tabs = () =>
  page.$$eval(".rt-tab", (els) => els.map((e) => e.textContent.trim()));
const activeTab = () =>
  page.$$eval(".rt-tab.active", (els) => els.map((e) => e.textContent.trim())[0] ?? null);
const headers = () =>
  page.$$eval(".tv-grid th", (els) =>
    els.map((e) => e.firstChild?.textContent?.trim() ?? "")
  );
const bodyRows = () => page.$$eval(".tv-grid tbody tr", (els) => els.length);
const cellTexts = () =>
  page.$$eval(".tv-grid tbody tr td", (els) => els.map((e) => e.textContent.trim()));
// In edit mode the cells are inputs, so their text content is empty.
const cellValues = () =>
  page.$$eval(".tv-grid tbody tr td input", (els) => els.map((e) => e.value));

await page.goto(URL, { waitUntil: "networkidle0" });
let fails = 0;
const fail = (m) => {
  console.log("FAIL: " + m);
  fails++;
};

await run("open database berlintest");

// A private copy of `ten`, so nothing shipped is edited.
await run(`delete ${REL}`); // in case an earlier run left one behind
await run(`let ${REL} = ten feed consume`);

// --- 1) no tabs while the map is the only view ---------------------------
if ((await tabs()).length !== 0)
  fail("a tab strip is shown before any table is open");

// --- 2) a relation with no geometry opens its own table ------------------
await run(`query ${REL}`);
// ...and draws nothing, so it must not take a row in the layers panel (which
// is the map's legend: visibility, draw order, colour).
if ((await page.$$eval(".lp-name", (els) => els.length)) !== 0)
  fail("a result that draws nothing was listed as a layer");
const t2 = await tabs();
console.log(`tabs after query: ${JSON.stringify(t2)} active=${await activeTab()}`);
if (t2.length !== 2) fail(`expected Map + one table tab, got ${JSON.stringify(t2)}`);
if (!(await activeTab())?.includes(REL))
  fail("a result the map cannot show did not open its table");

const h = await headers();
console.log(`headers: ${JSON.stringify(h)} rows=${await bodyRows()}`);
if (!h.includes("No")) fail(`the No column is missing: ${JSON.stringify(h)}`);
if ((await bodyRows()) !== 10) fail(`expected 10 rows, got ${await bodyRows()}`);

// --- 3) edit a cell and save --------------------------------------------
await startEditing();
console.log(`headers in edit mode: ${JSON.stringify(await headers())}`);

const firstCell = await page.$(".tv-grid tbody tr td input[type=text]");
if (!firstCell) fail("no editable cell in edit mode");
else {
  await firstCell.click();
  await page.keyboard.down("Control");
  await page.keyboard.press("KeyA");
  await page.keyboard.up("Control");
  await firstCell.type("4242");
  await wait(200);
  const typed = await page.$eval(
    ".tv-grid tbody tr td input[type=text]",
    (el) => el.value
  );
  if (typed !== "4242") fail(`the cell holds ${JSON.stringify(typed)}, not "4242"`);
  if ((await page.$(".tv-changed")) === null)
    fail("an edited cell is not marked as changed");
}

await save();
if ((await page.$(".tv-error")) !== null)
  fail("saving the cell edit reported an error: " +
    (await page.$eval(".tv-error", (e) => e.textContent)));

const savedRows = await bodyRows();
if (!(await cellValues()).includes("4242"))
  fail("the edited value is not in the reloaded grid");

// ...and it is really in SECONDO, not only in the browser.
await run(`query ${REL}`);
if (!(await cellTexts()).includes("4242"))
  fail("the edited value did not reach SECONDO");
console.log("cell edit persisted");

// --- 4) add a row, then delete it ---------------------------------------
await startEditing();
const before = await bodyRows();

await clickButton(".tv-head", "+ row");
await wait(200);
const newInput = await page.$(".tv-new input[type=text]");
if (!newInput) fail("+ row did not add an editable row");
else {
  await newInput.click();
  await newInput.type("777");
}
await save();
const afterInsert = await bodyRows();
console.log(`rows: ${before} -> ${afterInsert} (saved earlier: ${savedRows})`);
if (afterInsert !== before + 1) fail(`insert did not add a row (${afterInsert})`);
if (!(await cellValues()).includes("777"))
  fail("the inserted value is not in the grid");

// Delete it again, leaving the relation as it was.
const rowOf777 = await page.evaluate(() => {
  const rows = [...document.querySelectorAll(".tv-grid tbody tr")];
  return rows.findIndex((r) =>
    [...r.querySelectorAll("input")].some((i) => i.value === "777")
  );
});
if (rowOf777 < 0) fail("the inserted row is not editable after the reload");
else {
  const gutters = await page.$$(".tv-grid tbody tr .tv-rowbtn");
  await gutters[rowOf777].click();
  await wait(200);
  if ((await page.$(".tv-deleted")) === null)
    fail("a row marked for deletion is not struck through");
  await save();
}
const afterDelete = await bodyRows();
console.log(`rows after delete: ${afterDelete}`);
if (afterDelete !== before) fail(`delete did not remove the row (${afterDelete})`);

// --- 5) a mappable result never steals the active tab --------------------
const activeBefore = await activeTab();
await run("query mehringdamm");
if ((await activeTab()) !== activeBefore)
  fail("a result the map can show stole focus from the open table");
await page.click(".rt-tab"); // ◱ Map
await wait(600);
const drew = await page.evaluate(() => {
  const c = document.querySelector(".mapview canvas");
  return !!c && c.width > 0;
});
if (!drew) fail("the map no longer renders after switching tabs");

// --- 6) the console hint opens a table for a spatial result too ----------
await run("query Kinos");
// The log ends with a scroll anchor, so `.entry:last-child` matches nothing --
// take the last `.entry` explicitly.
const lastHints = () =>
  page.evaluate(() => {
    const entries = [...document.querySelectorAll(".log .entry")];
    const last = entries[entries.length - 1];
    return [...(last?.querySelectorAll(".geohint button") ?? [])].map((e) =>
      e.textContent.trim()
    );
  });
const hints = await lastHints();
console.log(`hints for Kinos: ${JSON.stringify(hints)}`);
if (!hints.some((t) => t.includes("rendered on map")))
  fail("Kinos lost its map hint");
if (!hints.some((t) => t.includes("show as table")))
  fail("Kinos has geometry but offers no table");

const rowHint = await page.evaluateHandle(() => {
  const entries = [...document.querySelectorAll(".log .entry")];
  const last = entries[entries.length - 1];
  return [...last.querySelectorAll(".geohint button")].find((e) =>
    e.textContent.includes("show as table")
  );
});
const tabsBefore = (await tabs()).length;
await rowHint.asElement().click();
await wait(600);
const tabsAfter = await tabs();
if (tabsAfter.length !== tabsBefore + 1)
  fail(`the hint did not open a table tab: ${JSON.stringify(tabsAfter)}`);
if (!(await activeTab())?.includes("Kinos"))
  fail("the hint did not activate the tab it opened");

// A result that *does* draw is still a layer, alongside its table.
const layerNames = await page.$$eval(".lp-name", (els) =>
  els.map((e) => e.textContent.trim())
);
if (!layerNames.some((n) => n.includes("Kinos")))
  fail(`Kinos draws on the map but is not a layer: ${JSON.stringify(layerNames)}`);
if (layerNames.some((n) => n.includes(REL)))
  fail("the table-only result reappeared in the layers panel");

// Reordering must swap past the hidden table-only results sitting between the
// drawable ones, or the button looks like it does nothing.
await run("query Flaechen");
const beforeOrder = await page.$$eval(".lp-name", (e) => e.map((x) => x.textContent.trim()));
const moved = await page.evaluate(() => {
  const items = [...document.querySelectorAll(".lp-item")];
  const up = [...items[items.length - 1].querySelectorAll(".lp-mini")].find(
    (b) => b.title === "Bring forward"
  );
  if (!up || up.disabled) return false;
  up.click();
  return true;
});
await wait(400);
const afterOrder = await page.$$eval(".lp-name", (e) => e.map((x) => x.textContent.trim()));
console.log(`order: ${JSON.stringify(beforeOrder)} -> ${JSON.stringify(afterOrder)}`);
if (!moved || afterOrder.join() === beforeOrder.join())
  fail("reordering did not swap past a hidden table-only result");

// Closing a tab puts the table away but keeps the layer on the map.
const layersBefore = await page.$$eval(".lp-item", (els) => els.length);
await page.click(".rt-tab.active .rt-close");
await wait(400);
if ((await tabs()).length !== tabsBefore)
  fail(`closing the tab did not remove it: ${JSON.stringify(await tabs())}`);
if ((await page.$$eval(".lp-item", (els) => els.length)) !== layersBefore)
  fail("closing a tab removed the layer");

await page.screenshot({ path: "e2e/out/table.png" });

// --- cleanup -------------------------------------------------------------
await run(`delete ${REL}`);

await browser.close();
console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
