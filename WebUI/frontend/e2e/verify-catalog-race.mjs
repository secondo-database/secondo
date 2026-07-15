// Verifies the catalog open/refresh race is fixed: clicking a database as soon
// as the page loads (before the session cookie settles) still fills the object
// list, and a loading spinner shows while objects are fetched.
import { createRequire } from "module";
const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1280,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };

try {
  // --- Test 1: fast first click still fills objects (run several times) ---
  let filledRuns = 0;
  const RUNS = 4;
  for (let i = 0; i < RUNS; i++) {
    const page = await browser.newPage();
    const client = await page.target().createCDPSession();
    await client.send("Network.clearBrowserCookies"); // force a fresh session
    // Load without waiting for network idle, then click ASAP.
    await page.goto(URL, { waitUntil: "domcontentloaded" });
    await page.waitForSelector(".cat-db", { timeout: 8000 });
    // Click SYMTRAJSMALL the instant the button exists (races the cookie).
    await page.evaluate(() => {
      const b = [...document.querySelectorAll(".cat-db")]
        .find((x) => x.textContent.includes("SYMTRAJSMALL"));
      b && b.click();
    });
    try {
      await page.waitForSelector(".cat-obj", { timeout: 8000 });
      const n = await page.$$eval(".cat-obj", (els) => els.length);
      if (n > 0) filledRuns++;
    } catch { /* stayed empty */ }
    await page.close();
  }
  check(filledRuns === RUNS,
        `fast first click fills objects on every run (${filledRuns}/${RUNS})`);

  // --- Test 2: loading spinner shows while objects load (throttled) ---
  const page = await browser.newPage();
  await page.setRequestInterception(true);
  page.on("request", async (r) => {
    if (r.url().includes("/api/objects")) {
      await new Promise((x) => setTimeout(x, 700));
    }
    r.continue();
  });
  await page.goto(URL, { waitUntil: "networkidle0" });
  await page.waitForSelector(".cat-db", { timeout: 8000 });
  await page.evaluate(() => {
    const b = [...document.querySelectorAll(".cat-db")]
      .find((x) => x.textContent.includes("BERLINTEST"));
    b && b.click();
  });
  let sawSpinner = false;
  for (let i = 0; i < 25; i++) {
    if (await page.$(".cat-spin")) { sawSpinner = true; break; }
    await new Promise((r) => setTimeout(r, 40));
  }
  check(sawSpinner, "loading spinner shown while objects load");
  await page.waitForSelector(".cat-obj", { timeout: 8000 });
  const finalN = await page.$$eval(".cat-obj", (els) => els.length);
  check(finalN > 0, `objects fill after load (${finalN})`);
  await page.close();
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
