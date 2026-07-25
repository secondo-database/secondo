// Verifies the command console UX: focus retention after Enter, input clearing,
// and arrow-up/down history recall.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});
const page = await browser.newPage();
await page.setViewport({ width: 1200, height: 800 });
page.on("pageerror", (e) => console.log("[pageerror]", e.message));

const val = () => page.$eval(".input textarea", (el) => el.value);
const focused = () => page.evaluate(() => document.activeElement === document.querySelector(".input textarea"));

async function type(cmd) {
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => (el.value = ""));
  await page.type(".input textarea", cmd);
}
async function enter() { await page.keyboard.press("Enter"); await new Promise(r=>setTimeout(r,400)); }

await page.goto(URL, { waitUntil: "networkidle0" });
let fails = 0;

// Submit three commands.
await type("open database berlintest"); await enter();
await type("query mehringdamm"); await enter();
await type("query 3 + 4"); await enter();

// 1) Focus retained after Enter, input cleared.
const f1 = await focused(); const v1 = await val();
console.log(`after Enter: focused=${f1} value="${v1}"`);
if (!f1) { console.log("FAIL: input lost focus after Enter"); fails++; }
if (v1 !== "") { console.log("FAIL: input not cleared"); fails++; }

// 2) ArrowUp recalls most recent, then older.
await page.click(".input textarea");
await page.keyboard.press("ArrowUp");
const up1 = await val();
await page.keyboard.press("ArrowUp");
const up2 = await val();
await page.keyboard.press("ArrowUp");
const up3 = await val();
console.log(`ArrowUp x3: "${up1}" | "${up2}" | "${up3}"`);
if (up1 !== "query 3 + 4") { console.log("FAIL: 1st ArrowUp"); fails++; }
if (up2 !== "query mehringdamm") { console.log("FAIL: 2nd ArrowUp"); fails++; }
if (up3 !== "open database berlintest") { console.log("FAIL: 3rd ArrowUp"); fails++; }

// 3) ArrowDown walks back toward newest, then to empty draft.
await page.keyboard.press("ArrowDown");
const dn1 = await val();
await page.keyboard.press("ArrowDown");
const dn2 = await val();
await page.keyboard.press("ArrowDown");
const dn3 = await val();
console.log(`ArrowDown x3: "${dn1}" | "${dn2}" | "${dn3}"`);
if (dn1 !== "query mehringdamm") { console.log("FAIL: 1st ArrowDown"); fails++; }
if (dn2 !== "query 3 + 4") { console.log("FAIL: 2nd ArrowDown"); fails++; }
if (dn3 !== "") { console.log("FAIL: ArrowDown past newest should clear"); fails++; }

// 4) Recall + submit works, and refocuses.
await page.keyboard.press("ArrowUp");   // "query 3 + 4"
await enter();
const f2 = await focused(); const v2 = await val();
console.log(`recall+submit: focused=${f2} value="${v2}"`);
if (!f2) { console.log("FAIL: focus after recall submit"); fails++; }

// 5) The box grows with a long query instead of scrolling sideways.
const height = () => page.$eval(".input textarea", (el) => el.offsetHeight);
await type("query mehringdamm");
const oneLine = await height();
await type(
  "select * from [staedte as s, plz as p] where [s.sname = p.ort, " +
  "p.plz > 40000, p.plz < 50000] orderby [p.plz asc] first 20"
);
const wrapped = await height();
console.log(`height: one line=${oneLine}px, long query=${wrapped}px`);
if (!(wrapped > oneLine)) { console.log("FAIL: box did not grow with the query"); fails++; }
// ...and shrinks back once the query is short again.
await type("query mehringdamm");
const backDown = await height();
if (backDown !== oneLine) { console.log(`FAIL: box did not shrink back (${backDown} vs ${oneLine})`); fails++; }

// 6) Shift+Enter breaks a line instead of running the query.
const entries = () => page.$$eval(".log .entry", (els) => els.length);
const beforeShift = await entries();
await type("query");
await page.keyboard.down("Shift");
await page.keyboard.press("Enter");
await page.keyboard.up("Shift");
await page.type(".input textarea", "  mehringdamm");
await new Promise((r) => setTimeout(r, 300));
const twoLine = await val();
console.log(`Shift+Enter value=${JSON.stringify(twoLine)} entries=${await entries()}`);
if (!twoLine.includes("\n")) { console.log("FAIL: Shift+Enter did not insert a newline"); fails++; }
if ((await entries()) !== beforeShift) { console.log("FAIL: Shift+Enter submitted the query"); fails++; }
const grown = await height();
if (!(grown > oneLine)) { console.log("FAIL: box did not grow with the second line"); fails++; }

// ...and plain Enter still runs it, line break and all.
await enter();
if ((await val()) !== "") { console.log("FAIL: Enter did not submit the multi-line query"); fails++; }
if ((await entries()) <= beforeShift) { console.log("FAIL: multi-line query produced no log entry"); fails++; }
const logged = await page.$$eval(".log .entry .cmd", (els) => els[els.length - 1].textContent);
console.log(`logged: ${JSON.stringify(logged)}`);
if (logged.includes("\n")) { console.log("FAIL: the line break was sent to the server"); fails++; }
if ((await height()) !== oneLine) { console.log("FAIL: box did not reset after submit"); fails++; }

await browser.close();
console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
