// Verifies that the catalog unfolds an object's full type under its row: a
// relation's attributes one per line, anything else as nested-list text, and
// that the name still queries the object.
//
// Needs only berlintest.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--window-size=1280,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };

// The <li> of one catalog object, found by its name.
const row = (page, name) =>
  page.evaluateHandle((n) =>
    [...document.querySelectorAll(".cat-objs li")].find(
      (li) => li.querySelector(".cat-oname")?.textContent === n), name);

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1280, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  await page.goto(URL, { waitUntil: "networkidle0" });
  await page.waitForSelector(".cat-db-select");
  await page.select(".cat-db-select", "BERLINTEST");
  await page.waitForSelector(".cat-obj", { timeout: 15000 });

  // 1) Nothing is unfolded to begin with.
  check((await page.$$(".cat-type")).length === 0, "no type panel before a click");
  const cmds = () => page.$$eval(".cmd", (els) => els.length);
  const cmdsBefore = await cmds();

  // 2) A relation unfolds into its attributes, in schema order.
  const strassen = await row(page, "strassen");
  check(strassen.asElement() !== null, "strassen is listed");
  await (await strassen.$(".cat-otype")).click();
  await page.waitForSelector(".cat-objs li .cat-type");
  const attrs = await strassen.$$eval(".cat-attr", (els) =>
    els.map((e) => [e.querySelector("dt").textContent, e.querySelector("dd").textContent]));
  check(JSON.stringify(attrs) === JSON.stringify(
    [["Name", "string"], ["Typ", "string"], ["GeoData", "line"]]),
    `strassen shows its attributes (${JSON.stringify(attrs)})`);
  check(await strassen.$eval(".cat-otype", (b) => b.getAttribute("aria-expanded")) === "true",
    "the badge reports itself expanded");

  // 3) A non-relation shows its type expression as text.
  const center = await row(page, "tiergarten");
  if (center.asElement()) {
    await (await center.$(".cat-otype")).click();
    const text = await center.$eval(".cat-type code", (c) => c.textContent);
    check(text === "region", `tiergarten shows "${text}"`);
  } else {
    check(false, "tiergarten is listed");
  }
  check((await page.$$(".cat-type")).length === 2, "two rows can be open at once");

  // 4) A second click folds it away again.
  await (await strassen.$(".cat-otype")).click();
  check((await strassen.$(".cat-type")) === null, "a second click folds strassen");

  // 5) Unfolding is local -- the type came with the catalog -- while the name
  //    still runs the query.
  const before = await cmds();
  check(before === cmdsBefore, `unfolding ran no command (${cmdsBefore} -> ${before})`);
  await (await strassen.$(".cat-obj")).click();
  await page.waitForFunction(
    (n) => document.querySelectorAll(".cmd").length > n, { timeout: 15000 }, before);
  check(true, "clicking the name runs a query");
} catch (e) {
  console.log("FAIL", e.message);
  fails++;
} finally {
  await browser.close();
}

process.exit(fails ? 1 : 0);
