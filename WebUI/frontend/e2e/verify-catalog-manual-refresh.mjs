// Verifies the catalog's refresh button: it refetches the database and object
// lists on demand, spins while it does, and leaves the list intact.
//
// The automatic refresh (see verify-catalog-refresh.mjs) only fires for
// commands *this* UI ran, so a change made anywhere else -- another tab, the
// Java GUI, a script on the server -- is invisible until something reloads.
// This button is that something, and the check is that a click actually reaches
// the backend rather than redrawing what React already held.
//
// Needs only berlintest. Creates nothing, so there is nothing to clean up.
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

  let dbCalls = 0;
  let objectCalls = 0;
  page.on("request", (r) => {
    if (r.url().includes("/api/databases")) dbCalls++;
    if (r.url().includes("/api/objects")) objectCalls++;
  });

  await page.goto(URL, { waitUntil: "networkidle0" });
  await page.waitForSelector(".cat-db", { timeout: 15000 });

  const names = () =>
    page.$$eval(".cat-objs .cat-oname", (els) => els.map((e) => e.textContent.trim()));

  // The button is there before any database is open -- a database created
  // elsewhere is exactly the case where nothing else would bring it in.
  check((await page.$(".cat-refresh")) !== null,
        "the refresh button is present with no database open");
  const dbsBefore = dbCalls;
  await page.click(".cat-refresh");
  await wait(1200);
  check(dbCalls > dbsBefore,
        `clicking it refetches the database list (${dbCalls - dbsBefore} call(s))`);

  const handle = await page.evaluateHandle(
    (n) => [...document.querySelectorAll(".cat-db")].find((b) => b.textContent.trim() === n),
    DB
  );
  await handle.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 15000 });
  await wait(300);

  const before = await names();
  check(before.length > 0, `${DB} lists objects to begin with (${before.length})`);

  // With a database open the click must reload both lists, not just the names
  // of the databases.
  const dbsBefore2 = dbCalls;
  const objsBefore = objectCalls;
  await page.click(".cat-refresh");
  // The spinner is the only feedback for a refresh that changes nothing, so it
  // has to be observable while the request is in flight.
  let spun = false;
  for (let i = 0; i < 60 && !spun; i++) {
    spun = await page.$eval(".cat-refresh", (b) => b.classList.contains("spinning"))
      .catch(() => false);
    if (!spun) await wait(25);
  }
  check(spun, "the button spins while the refresh is in flight");

  await page.waitForFunction(
    () => !document.querySelector(".cat-refresh")?.classList.contains("spinning"),
    { timeout: 15000 }
  );
  check(dbCalls > dbsBefore2 && objectCalls > objsBefore,
        `it refetches databases and objects (${dbCalls - dbsBefore2}/${objectCalls - objsBefore})`);

  const after = await names();
  check(after.length === before.length && after.every((n, i) => n === before[i]),
        `the object list survives the refresh (${after.length} vs ${before.length})`);
  check(await page.$eval(".cat-refresh", (b) => !b.disabled),
        "and the button is clickable again afterwards");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
