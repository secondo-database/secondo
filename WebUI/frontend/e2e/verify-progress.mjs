// Verifies that a long query shows the server's progress estimate while it
// runs, as the TTY's progress bar does.
//
// The estimate travels as (progress (k n)) messages on the query's own
// connection; the bridge keeps the latest, and the UI polls /api/progress
// outside the request queue. So this guards three things at once: the bridge
// picks the messages up, the endpoint answers while /api/query is still open,
// and the console draws it on the running entry -- and takes it away again.
//
// Needs only berlintest. The query reads Kinos and plz and creates nothing; it
// takes a few seconds, which is long enough for several estimates.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const DB = "BERLINTEST";
const SLOW = "query Kinos feed {a} plz feed {b} product filter[.PLZ_b > 5000] count";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };

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

  await page.click(".input textarea");
  await page.type(".input textarea", SLOW);
  await page.keyboard.press("Escape"); // a completion popup would eat the Enter
  await page.keyboard.press("Enter");

  // Sample the running entry until the answer is back.
  const samples = [];
  for (;;) {
    const s = await page.evaluate(() => {
      const e = document.querySelector(".log .entry.pending");
      if (!e) return null;
      const bar = e.querySelector(".cmd-progress");
      return {
        value: bar ? Number(bar.getAttribute("aria-valuenow")) : null,
        eta: e.querySelector(".cmd-eta")?.textContent.trim() ?? null,
        overlay: document.querySelector(".loading")?.textContent.trim() ?? "",
        strip: !!document.querySelector(".run-strip"),
      };
    });
    if (s === null) break;
    samples.push(s);
    await new Promise((r) => setTimeout(r, 200));
  }

  const shown = samples.filter((s) => s.value !== null);
  console.log(`  ${samples.length} samples, values: ${shown.map((s) => s.value).join(" ")}`);
  check(shown.length >= 2, `the running entry shows a progress bar (${shown.length} samples)`);
  check(shown.every((s) => s.value >= 0 && s.value <= 100), "its value is a percentage");
  check(shown.some((s) => s.value > 0), "and it moves off zero");
  check(shown.every((s) => /^\d+ %/.test(s.eta ?? "")),
        `with the percentage beside the counter (${shown[shown.length - 1]?.eta})`);
  check(shown.some((s) => /~\d+:\d\d left$/.test(s.eta ?? "")),
        "and an estimate of the time left once it has started");
  check(shown.some((s) => /\d+ %$/.test(s.overlay)),
        `the map's overlay gives the percentage too (${shown[shown.length - 1]?.overlay})`);
  check(samples.every((s) => !s.strip), "with the history shown, no second copy of it above the input");

  const after = await page.evaluate(() => {
    const entries = document.querySelectorAll(".log .entry");
    const last = entries[entries.length - 1];
    return {
      bars: document.querySelectorAll(".cmd-progress").length,
      etas: document.querySelectorAll(".cmd-eta").length,
      // A count is one value, so it is shown unpacked with the nested list
      // folded away; either is the result being there.
      result:
        last?.querySelector(".scalar-value")?.textContent.trim() ??
        last?.querySelector("pre.ok")?.textContent.trim() ??
        "",
    };
  });
  check(after.bars === 0 && after.etas === 0, "the bar goes away when the answer is back");
  check(after.result.includes("3258082"), `the result is there (${after.result})`);

  // A command without an estimate must not inherit the last one's.
  const progress = await page.evaluate(async () => (await fetch("/api/progress")).json());
  check(progress.total === 0, `nothing is reported once it is over (${JSON.stringify(progress)})`);

  // With the history hidden the running entry is hidden too, so the estimate
  // has to show up next to the input instead.
  await page.click(".console .dock-btn.first");
  await page.waitForSelector(".console.collapsed", { timeout: 5000 });
  await page.click(".input textarea");
  await page.type(".input textarea", SLOW);
  await page.keyboard.press("Escape");
  await page.keyboard.press("Enter");

  const hidden = [];
  for (;;) {
    const s = await page.evaluate(() => {
      if (!document.querySelector(".log .entry.pending")) return null;
      const strip = document.querySelector(".run-strip");
      const bar = strip?.querySelector(".cmd-progress");
      const visible = (el) => !!el && el.getBoundingClientRect().height > 0;
      return {
        strip: visible(strip),
        text: strip?.querySelector(".cmd-text")?.textContent.trim() ?? null,
        timer: strip?.querySelector(".cmd-ms.running")?.textContent.trim() ?? null,
        value: visible(bar) ? Number(bar.getAttribute("aria-valuenow")) : null,
        eta: strip?.querySelector(".cmd-eta")?.textContent.trim() ?? null,
      };
    });
    if (s === null) break;
    hidden.push(s);
    await new Promise((r) => setTimeout(r, 200));
  }
  const hiddenShown = hidden.filter((s) => s.value !== null);
  console.log(`  history hidden: ${hidden.length} samples, values: ${hiddenShown.map((s) => s.value).join(" ")}`);
  check(hidden.length > 0 && hidden.every((s) => s.strip),
        "with the history hidden, the running command shows above the input");
  check(hidden[0]?.text === SLOW && /^\d+\.\d s$/.test(hidden[0]?.timer ?? ""),
        `with its command and counter (${hidden[0]?.timer})`);
  check(hiddenShown.length >= 2 && hiddenShown.some((s) => s.value > 0),
        `and a visible progress bar that moves (${hiddenShown.length} samples)`);
  check(hiddenShown.some((s) => /^\d+ % · ~\d+:\d\d left$/.test(s.eta ?? "")),
        `and the percentage and time left (${hiddenShown[hiddenShown.length - 1]?.eta})`);
  const stripAfter = await page.$$eval(".run-strip", (els) => els.length);
  check(stripAfter === 0, "the strip goes away when the answer is back");
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
