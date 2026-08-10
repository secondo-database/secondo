// Verifies that a command appears in the console log as soon as it is sent,
// not only once the answer is back.
//
// The regression this guards: the entry used to be appended in the `try` block
// *after* `runQuery` resolved, so a query that takes a minute left the log
// exactly as it was -- the typed text gone from the box, nothing where it went,
// and only the map's overlay saying anything at all. The entry now goes up
// pending, counts up while it waits, and is filled in on arrival, in place.
//
// Needs only berlintest. `intstream(1, 40000000) count` reads nothing from the
// database and creates nothing; it just takes a few seconds to count.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const DB = "BERLINTEST";
const SLOW = "query intstream(1, 40000000) count";

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

  const handle = await page.evaluateHandle(
    (n) => [...document.querySelectorAll(".cat-db")].find((b) => b.textContent.trim() === n),
    DB
  );
  await handle.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 15000 });

  const entries = () => page.$$eval(".log .entry", (els) => els.length);
  const before = await entries();

  await page.click(".input textarea");
  await page.type(".input textarea", SLOW);
  await page.keyboard.press("Escape"); // a completion popup would eat the Enter
  await page.keyboard.press("Enter");

  // The entry has to be there while the query is still running -- which is the
  // whole point -- so this must succeed long before the answer arrives.
  await page.waitForFunction(
    (n) => document.querySelectorAll(".log .entry").length > n,
    { timeout: 5000 },
    before
  );
  const pending = await page.$$eval(".log .entry.pending", (els) =>
    els.map((e) => ({
      text: e.querySelector(".cmd-text")?.textContent.trim(),
      timer: e.querySelector(".cmd-ms.running")?.textContent.trim() ?? null,
      spinner: !!e.querySelector(".cmd-spin"),
      // A pending entry has a command and nothing else: no result block, no
      // error, no hints about a result that does not exist yet.
      body: e.querySelectorAll("pre, .geohint").length,
    }))
  );
  check(pending.length === 1, `exactly one entry is pending while it runs (${pending.length})`);
  check(pending[0]?.text === SLOW, `it shows the command as sent (${pending[0]?.text})`);
  check(pending[0]?.spinner, "with a spinner on it");
  check(/^\d+\.\d s$/.test(pending[0]?.timer ?? ""),
        `and a running counter (${pending[0]?.timer})`);
  check(pending[0]?.body === 0, "and no result or error block yet");
  check(await page.$eval(".run-go", (b) => b.disabled), "the Run button is disabled meanwhile");

  // The counter is live, not a number printed once.
  const first = pending[0]?.timer;
  await wait(1100);
  const second = await page.$eval(".cmd-ms.running", (e) => e.textContent.trim());
  check(second !== first, `the counter ticks while it waits (${first} -> ${second})`);

  // And when the answer comes it lands in the same entry, not in a second one.
  await page.waitForFunction(
    () => document.querySelectorAll(".log .entry.pending").length === 0,
    { timeout: 120000, polling: 250 }
  );
  const after = await entries();
  check(after === before + 1, `the answer fills that entry in, adding no other (${after} vs ${before + 1})`);

  const last = await page.$$eval(".log .entry", (els) => {
    const e = els[els.length - 1];
    return {
      text: e.querySelector(".cmd-text")?.textContent.trim(),
      ms: e.querySelector(".cmd-ms")?.textContent.trim(),
      running: !!e.querySelector(".cmd-ms.running"),
      result: e.querySelector("pre.ok")?.textContent.trim() ?? "",
    };
  });
  check(last.text === SLOW, "the finished entry still shows the command");
  check(!last.running, "the counter has stopped");
  check(/\bs$/.test(last.ms ?? ""), `and reads as the elapsed time (${last.ms})`);
  check(last.result.includes("40000000"), `the result is there (${last.result.slice(0, 40)})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
