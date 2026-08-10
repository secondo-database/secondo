// Verifies the Run menu's "Show result as table": the result opens as a table and no
// map layer is made, where the same query run plainly draws on the map.
//
// The promise is kept end to end -- the server converts the rows alone, so there
// is no geometry to draw rather than geometry that is drawn and ignored -- and
// where it cannot be kept (a result that is not a relation) the console says so
// instead of quietly doing nothing.
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

  const layerCount = () => page.$$eval(".lp-item", (els) => els.length);
  const entryCount = () => page.$$eval(".log .entry", (els) => els.length);
  const lastEntry = () =>
    page.$$eval(".log .entry", (els) => els[els.length - 1]?.textContent ?? "");

  /** Type a command and run it, either plainly or through a Run menu item. */
  async function run(cmd, menuItem) {
    const before = await entryCount();
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    if (menuItem) {
      await page.click(".run-more");
      await page.waitForSelector(".run-menu", { timeout: 5000 });
      const handle = await page.evaluateHandle(
        (label) =>
          [...document.querySelectorAll(".run-menu button")].find((b) =>
            b.textContent.includes(label)
          ),
        menuItem
      );
      const el = handle.asElement();
      if (!el) throw new Error(`no "${menuItem}" item in the Run menu`);
      await el.click();
    } else {
      await page.keyboard.press("Enter");
    }
    await page.waitForFunction(
      (n) =>
      document.querySelectorAll(".log .entry").length > n &&
      // The entry now goes up when the command is *sent*, so a grown log
      // is not an answered command: wait for it to stop being pending too.
      !document.querySelector(".log .entry.pending"),
      { timeout: 30000 },
      before
    );
    await wait(400); // let the render that follows the state update settle
  }

  // Open the database the suite is allowed to depend on.
  const target = await page.evaluateHandle(
    (n) => [...document.querySelectorAll(".cat-db")].find((b) => b.textContent.trim() === n),
    DB
  );
  await target.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 15000 });
  check((await layerCount()) === 0, "no layers before anything is queried");

  // 1) A relation of points, asked for as a table. Kinos would draw on the map
  //    if it were run plainly -- which is exactly what makes it the test.
  await run("query Kinos", "Show result as table");
  await page.waitForSelector(".tv-grid", { timeout: 15000 });
  check((await layerCount()) === 0, "no map layer was made");
  const active = await page.$eval(".rt-tab.active", (e) => e.textContent.trim());
  check(!active.includes("Map"), `the table tab is the active one (${active})`);
  check((await page.$$eval(".tv-grid tbody tr", (e) => e.length)) > 0,
        "the table has rows");
  const tableEntry = await lastEntry();
  check(!tableEntry.includes("rendered on map"),
        "the console entry offers no map hint, having nothing on the map");
  check(!tableEntry.includes("Not a relation"), "and does not claim there were no rows");

  // 1b) Closing the table leaves a blank map, and a blank map has to explain
  //     itself. It did not: a table-only result is still a layer, so counting
  //     layers said the map had something on it while nothing was drawn.
  await page.click(".rt-close");
  await wait(400);
  const emptyAgain = await page.$$eval(".map-empty", (els) => els.length);
  check(emptyAgain === 1, "the empty-map card is back once the table is closed");
  check((await page.$$eval(".rt-tab", (els) => els.length)) === 0,
        "and the tab strip is gone with the last table");

  // 2) The same query run plainly still draws, so only the intent changed.
  await run("query Kinos");
  await page.waitForFunction(() => document.querySelectorAll(".lp-item").length > 0,
                             { timeout: 15000 });
  check((await layerCount()) === 1, "a plain run of the same query does make a layer");
  check((await lastEntry()).includes("rendered on map"),
        "and its console entry offers the map");

  // 2b) Running a query empties the input, so the menu is normally reached with
  //     nothing left in it to run. It used to bail before even closing itself:
  //     a click that did nothing whatsoever. With nothing typed the item acts on
  //     the last result that had rows, and re-runs no query to do it.
  const entriesBefore = await entryCount();
  await page.click(".run-more");
  await page.waitForSelector(".run-menu", { timeout: 5000 });
  const note = await page.$$eval(".run-menu .run-note", (els) =>
    els.map((e) => e.textContent.trim()));
  check(note.includes("last result"),
        `the menu says what it would act on (${JSON.stringify(note)})`);
  const item = await page.evaluateHandle(() =>
    [...document.querySelectorAll(".run-menu button")].find((b) =>
      b.textContent.includes("Show result as table")));
  await item.asElement().click();
  await wait(600);
  check(!(await page.$(".run-menu")), "the menu closes on the click");
  check((await entryCount()) === entriesBefore, "and no query was re-run");
  check(!!(await page.$(".tv-grid")), "the last result's table is open");
  await page.click(".rt-close");
  await wait(300);

  // 3) A point is not a relation: there are no rows to show, and the entry has
  //    to say why nothing opened.
  const tabsBefore = await page.$$eval(".rt-tab", (els) => els.length);
  await run("query mehringdamm", "Show result as table");
  check((await lastEntry()).includes("Not a relation"),
        "a non-relation result says it has no rows to tabulate");
  check((await page.$$eval(".rt-tab", (els) => els.length)) === tabsBefore,
        "and opens no table tab");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
