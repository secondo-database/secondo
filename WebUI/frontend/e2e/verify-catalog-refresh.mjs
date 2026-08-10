// Verifies that the catalog reloads after a command that changes what the
// database holds -- `let` and `delete` here -- without the user reopening the
// database or reloading the page, and that a read-only command does not make
// it reload.
//
// The regression this guards: the refresh used to fire only for
// open/close/create/delete *database* and for SQL the optimizer executed
// itself, so a plain `let x = ...` left the object list (and the console's
// completions, which are fed from the same state) showing a database that no
// longer held what it said.
//
// Needs only berlintest. Creates one object and deletes it again; if it fails
// in between, `delete webui_refresh_probe` cleans up.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const DB = "BERLINTEST";
const OBJ = "webui_refresh_probe";

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

  // Whether the catalog refetched at all -- the only way to tell "did not
  // refresh" apart from "refreshed and nothing changed".
  let objectCalls = 0;
  page.on("request", (r) => {
    if (r.url().includes("/api/objects")) objectCalls++;
  });

  await page.goto(URL, { waitUntil: "networkidle0" });
  await page.waitForSelector(".cat-db", { timeout: 15000 });

  // The rendered list, which is what the user actually sees -- not the state
  // behind it.
  const names = () =>
    page.$$eval(".cat-objs .cat-oname", (els) => els.map((e) => e.textContent.trim()));
  const entryCount = () => page.$$eval(".log .entry", (els) => els.length);
  const lastEntry = () =>
    page.$$eval(".log .entry", (els) => els[els.length - 1]?.textContent ?? "");

  // Submit and wait for the command to come back rather than for a fixed time:
  // the first `let` of a session takes seconds (SECONDO writes the catalog),
  // while `query 3 + 4` is back in milliseconds.
  const run = async (cmd) => {
    const before = await entryCount();
    await page.click(".input textarea");
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Escape"); // a completion popup would eat the Enter
    await page.keyboard.press("Enter");
    await page.waitForFunction(
      (n) =>
      document.querySelectorAll(".log .entry").length > n &&
      // The entry now goes up when the command is *sent*, so a grown log
      // is not an answered command: wait for it to stop being pending too.
      !document.querySelector(".log .entry.pending"),
      { timeout: 60000 },
      before
    );
  };
  // The refresh is two calls behind the command, so poll for the list the
  // command should have produced. On a timeout the caller's assertion reports
  // what the list actually holds, which says more than "waitForFunction failed".
  const listSettles = async (wanted) => {
    try {
      await page.waitForFunction(
        (name, want) =>
          [...document.querySelectorAll(".cat-objs .cat-oname")]
            .some((e) => e.textContent.trim() === name) === want,
        { timeout: 15000, polling: 250 },
        OBJ,
        wanted
      );
    } catch { /* fall through to the assertion */ }
    return names();
  };

  const handle = await page.evaluateHandle(
    (n) => [...document.querySelectorAll(".cat-db")].find((b) => b.textContent.trim() === n),
    DB
  );
  await handle.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 15000 });
  await wait(300);

  const before = await names();
  check(before.length > 0, `${DB} lists objects to begin with (${before.length})`);
  check(!before.includes(OBJ), `${OBJ} does not exist yet`);

  // 1) `let` -- the case that was broken.
  await run(`let ${OBJ} = 42`);
  const afterLet = await listSettles(true);
  check(afterLet.includes(OBJ),
        `the catalog shows ${OBJ} after let, with no reopen (${afterLet.length} objects)`);

  // 2) A read-only command must not refetch: doing it on every command would
  //    cost two extra round trips per query.
  const callsBefore = objectCalls;
  await run("query 3 + 4");
  await wait(1500); // long enough for a refresh to have started, if one would
  check(objectCalls === callsBefore,
        `a plain query does not refetch the object list (${objectCalls - callsBefore} calls)`);
  check((await names()).includes(OBJ), "and leaves the list as it was");

  // 3) `delete` -- the same path in reverse.
  await run(`delete ${OBJ}`);
  const afterDelete = await listSettles(false);
  check(!afterDelete.includes(OBJ), "the catalog drops it again after delete");
  check(afterDelete.length === before.length,
        `back to the original list (${afterDelete.length} vs ${before.length})`);
  check(!/error/i.test(await lastEntry()), "the delete itself reported no error");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
