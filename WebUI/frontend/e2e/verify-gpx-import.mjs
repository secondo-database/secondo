// Verifies the GPX import end to end: a file picked in the browser is uploaded
// to the bridge, the dialog proposes a name from the filename, and the four
// commands run in order and create four objects the catalog then lists.
//
// It drives the drop zone's file input rather than a synthetic drag: puppeteer
// can attach a real file to an <input type=file>, and a hand-built DataTransfer
// would only prove that the handler runs, not that the browser hands over a
// readable File. The same `accept()` is behind both gestures.
//
// Needs berlintest (any database would do) and bin/Trk_MapMatchTest.gpx from
// the checkout. Deletes what it created, including after a failure.
import { createRequire } from "module";
import { dirname, resolve } from "path";
import { fileURLToPath } from "url";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const HERE = dirname(fileURLToPath(import.meta.url));
const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const DB = "BERLINTEST";
// Named so the proposed name is predictable: `Trk_MapMatchTest.gpx` becomes
// `trk_mapmatchtest` (lowercased, non-alphanumerics collapsed).
const GPX = resolve(HERE, "../../../bin/Trk_MapMatchTest.gpx");
const NAME = "trk_mapmatchtest";
const CREATED = [NAME, `${NAME}_mp`, `${NAME}_trajectory`, `${NAME}_bbox`];

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };
const wait = (ms) => new Promise((r) => setTimeout(r, ms));

let page;
try {
  page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  await page.goto(URL, { waitUntil: "networkidle0" });
  await page.waitForSelector(".cat-db", { timeout: 15000 });

  const names = () =>
    page.$$eval(".cat-objs .cat-oname", (els) => els.map((e) => e.textContent.trim()));
  const entryCount = () => page.$$eval(".log .entry", (els) => els.length);
  const run = async (cmd) => {
    const before = await entryCount();
    await page.click(".input textarea");
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Escape"); // a completion popup would eat the Enter
    await page.keyboard.press("Enter");
    await page.waitForFunction(
      (n) => document.querySelectorAll(".log .entry").length > n,
      { timeout: 60000 },
      before
    );
  };

  // Without a database open there is nowhere to import to, and the zone has to
  // say so rather than silently swallow a drop.
  check(
    await page.$eval(".cat-drop", (el) => el.classList.contains("disabled")),
    "the drop zone is disabled before a database is open"
  );

  const dbBtn = await page.evaluateHandle(
    (n) => [...document.querySelectorAll(".cat-db")].find((b) => b.textContent.trim() === n),
    DB
  );
  await dbBtn.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 15000 });
  await wait(300);

  const before = await names();
  check(
    !CREATED.some((n) => before.includes(n)),
    `none of ${CREATED.join(", ")} exists yet`
  );
  check(
    await page.$eval(".cat-drop", (el) => !el.classList.contains("disabled")),
    "and the drop zone is live once a database is open"
  );

  // Hand the file over. The upload and the dialog follow from the change event.
  const input = await page.$(".cat-drop-input");
  await input.uploadFile(GPX);

  await page.waitForSelector(".gpx-dialog", { timeout: 15000 });
  check(true, "the dialog opens for the picked file");
  check(
    (await page.$eval(".gpx-name", (el) => el.value)) === NAME,
    `the name is proposed from the filename (expected ${NAME})`
  );

  // One GPX track becomes four objects, and the suffixes alone do not say
  // which is which -- the dialog previews all four with what each one is,
  // before anything runs.
  const preview = await page.$$eval(".gpx-step", (els) =>
    els.map((e) => [
      e.querySelector(".gpx-step-obj")?.textContent.trim(),
      e.querySelector(".gpx-step-what")?.textContent.trim(),
    ])
  );
  check(
    JSON.stringify(preview) ===
      JSON.stringify([
        [NAME, "raw GPX import"],
        [`${NAME}_mp`, "moving point"],
        [`${NAME}_trajectory`, "trajectory"],
        [`${NAME}_bbox`, "bounding box"],
      ]),
    `the four objects are previewed with their kinds (${JSON.stringify(preview)})`
  );

  // The commands are folded away, not absent: what the four rows above will
  // actually run is readable before pressing Import.
  check(
    await page.$eval(".gpx-cmds", (el) => !el.open),
    "the commands start collapsed"
  );
  await page.click(".gpx-cmds > summary");
  const cmds = await page.$$eval(".gpx-cmds li code", (els) =>
    els.map((e) => e.textContent.trim())
  );
  check(
    cmds.length === 4 && cmds[0].startsWith(`let ${NAME} = gpximport(`),
    `unfolding shows the four commands (${cmds[0] ?? "none"})`
  );
  await page.click(".gpx-cmds > summary");

  // The Import button is only enabled once the upload has landed a path.
  await page.waitForFunction(
    () => {
      const b = document.querySelector(".gpx-import");
      return b && !b.disabled;
    },
    { timeout: 30000 }
  );
  check(true, "the upload finished and Import became available");

  await page.click(".gpx-import");

  // Four steps, all green. `bbox` on a long track is the slow one.
  await page.waitForFunction(
    () => document.querySelectorAll(".gpx-step.is-done").length === 4,
    { timeout: 120000, polling: 500 }
  ).catch(() => undefined);

  const states = await page.$$eval(".gpx-step", (els) =>
    els.map((e) => e.className.replace(/.*is-/, ""))
  );
  const failure = await page.$eval(".gpx-dialog", (el) => {
    const pre = el.querySelector("pre.err");
    return pre ? pre.textContent.trim() : "";
  });
  check(
    states.length === 4 && states.every((s) => s === "done"),
    `all four steps completed (${states.join(", ")})${failure ? " -- " + failure : ""}`
  );

  // Close, and the catalog picks up what the import created.
  await page.click(".gpx-close");
  await page.waitForFunction(() => !document.querySelector(".gpx-dialog"), { timeout: 5000 });
  await page.waitForFunction(
    (wanted) => {
      const shown = [...document.querySelectorAll(".cat-objs .cat-oname")]
        .map((e) => e.textContent.trim());
      return wanted.every((n) => shown.includes(n));
    },
    { timeout: 20000, polling: 250 },
    CREATED
  ).catch(() => undefined);

  const after = await names();
  for (const n of CREATED) check(after.includes(n), `the catalog lists ${n}`);

  // A second import of the same file must be refused before it runs: every one
  // of the four names is taken now, and a half-done import is worse than none.
  await (await page.$(".cat-drop-input")).uploadFile(GPX);
  await page.waitForSelector(".gpx-dialog", { timeout: 15000 });
  const note = await page.$eval(".gpx-note", (el) => el.textContent.trim());
  check(
    /already exists/.test(note),
    `the name collision is reported before importing ("${note}")`
  );
  check(
    await page.$eval(".gpx-import", (el) => el.disabled),
    "and Import stays disabled"
  );
  await page.click(".gpx-close");

  // Clean up: delete in reverse, since nothing depends on anything here but it
  // reads as the undo of the import.
  for (const n of [...CREATED].reverse()) await run(`delete ${n}`);
  // The catalog refresh is two round trips behind the command that triggered
  // it, so the list is polled rather than read once -- the console entry says
  // the delete came back, not that the panel has caught up with it.
  await page.waitForFunction(
    (gone) => {
      const shown = [...document.querySelectorAll(".cat-objs .cat-oname")]
        .map((e) => e.textContent.trim());
      return !gone.some((n) => shown.includes(n));
    },
    { timeout: 20000, polling: 250 },
    CREATED
  ).catch(() => undefined); // let the assertion below report what is left
  const cleaned = await names();
  check(!CREATED.some((n) => cleaned.includes(n)), "the created objects are deleted again");
} finally {
  // Best effort even if an assertion above threw mid-import: the objects would
  // otherwise fail every later run of this check.
  if (page) {
    for (const n of [...CREATED].reverse()) {
      await page.evaluate(
        (cmd) =>
          fetch("/api/query", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            credentials: "same-origin",
            body: JSON.stringify({ command: cmd, view: "none" }),
          }).catch(() => undefined),
        `delete ${n}`
      ).catch(() => undefined);
    }
  }
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
