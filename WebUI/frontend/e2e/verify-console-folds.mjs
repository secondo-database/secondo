// Verifies the console log's three quality-of-life changes: a one-value result
// shown as that value, and the two blocks -- the optimized query and the nested
// list -- that can be folded away per entry and switched off for all of them.
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

async function type(cmd) {
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => (el.value = ""));
  await page.type(".input textarea", cmd);
}
async function run(cmd) {
  await type(cmd);
  await page.keyboard.press("Enter");
  // Wait for the answer rather than for a fixed time: the optimizer's first
  // SQL command of a session takes seconds, and a sleep long enough for that
  // would be one every other command pays.
  await page.waitForFunction(() => !document.querySelector(".log .entry.pending"),
                             { timeout: 60000 });
  await new Promise((r) => setTimeout(r, 150));
}

/** The last log entry, read as the pieces this check is about. */
const last = () =>
  page.$$eval(".log .entry", (els) => {
    const e = els[els.length - 1];
    const folds = [...e.querySelectorAll(".fold")].map((b) => ({
      label: b.textContent.trim(),
      open: b.getAttribute("aria-expanded") === "true",
    }));
    return {
      scalar: e.querySelector(".scalar-value")?.textContent.trim() ?? null,
      type: e.querySelector(".scalar-type")?.textContent.trim() ?? null,
      folds,
      pre: [...e.querySelectorAll("pre")].map((p) => p.textContent.trim()),
    };
  });

/** Click the fold row whose label starts with `name` on the last entry. */
const clickFold = (name) =>
  page.$$eval(
    ".log .entry",
    (els, n) => {
      const e = els[els.length - 1];
      [...e.querySelectorAll(".fold")]
        .find((b) => b.textContent.includes(n))
        ?.click();
    },
    name
  );

const switchOf = (label) =>
  page.$$eval(
    ".console header .dock-btn",
    (els, l) => els.find((b) => b.textContent.trim().endsWith(l))?.click(),
    label
  );
const pressed = (label) =>
  page.$$eval(
    ".console header .dock-btn",
    (els, l) =>
      els.find((b) => b.textContent.trim().endsWith(l))?.getAttribute("aria-pressed"),
    label
  );

await page.goto(URL, { waitUntil: "networkidle0" });
let fails = 0;
const check = (ok, msg) => { if (!ok) { console.log(`FAIL: ${msg}`); fails++; } };

await run("open database berlintest");

// 1) A one-value result is shown as the value, with its type beside it, and
//    its nested list starts folded because it would only repeat it.
await run("query 1 + 55");
let e = await last();
console.log("query 1 + 55:", JSON.stringify(e));
check(e.scalar === "56", `expected the value 56, got ${e.scalar}`);
check(e.type === "int", `expected the type int, got ${e.type}`);
const nl = e.folds.find((f) => f.label.includes("nested list"));
check(!!nl, "no nested-list fold row");
check(nl && !nl.open, "the nested list should start folded when a value is shown");
check(!e.pre.some((p) => p.includes("(int 56)")), "the nested list was printed anyway");

// ...and it is one click away.
await clickFold("nested list");
e = await last();
console.log("after expanding:", JSON.stringify(e.pre));
check(e.pre.some((p) => p === "(int 56)"), "expanding did not reveal the nested list");

// 2) A result that is not one value keeps its list open, and has no value line.
await run("query mehringdamm");
e = await last();
console.log("query mehringdamm:", JSON.stringify(e));
check(e.scalar === null, "a point is not a single value");
check(
  e.pre.some((p) => p.includes("(point (9396.0 9871.0))")),
  "the point's nested list should be shown"
);

// 3) SQL: the plan gets a fold row carrying its costs, and folds away.
await run("select count(*) from Staedte");
e = await last();
console.log("select count(*):", JSON.stringify(e));
const plan = e.folds.find((f) => f.label.includes("optimized query"));
check(!!plan, "no optimized-query fold row");
check(plan && plan.open, "the optimized query should start shown");
check(e.scalar === "58", `expected 58 cities, got ${e.scalar}`);
await clickFold("optimized query");
e = await last();
check(
  !e.pre.some((p) => p.includes("count")),
  "folding the optimized query did not hide the plan"
);

// 4) The header switches set the default for every entry, and survive a reload.
check((await pressed("result")) === "true", "the result switch should start on");
await switchOf("result");
await new Promise((r) => setTimeout(r, 200));
check((await pressed("result")) === "false", "the result switch did not turn off");
const anyList = await page.$$eval(".log .entry", (els) =>
  els.some((el) => [...el.querySelectorAll("pre.ok")].length > 0)
);
check(!anyList, "a nested list is still shown with the switch off");

await page.reload({ waitUntil: "networkidle0" });
check((await pressed("result")) === "false", "the switch was not remembered across a reload");
await switchOf("result");
await new Promise((r) => setTimeout(r, 200));
check((await pressed("result")) === "true", "the switch did not turn back on");

await browser.close();
console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
