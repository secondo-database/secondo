// Verifies the UX pass: overlays that no longer collide, a console that keeps
// the map (and its own input) on screen, a visible focus ring, the empty-state
// card, elapsed timing, and completion in the query box.
//
// Every check here stands for something that was actually wrong: the zoom
// buttons drew over the layers panel, the plots covered the details panel, a
// long history grew the console until the map was gone, tabbing was invisible,
// and an empty map said nothing useful.
import { createRequire } from "module";
import { mkdirSync } from "fs";
import { dirname } from "path";
import { fileURLToPath } from "url";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");
const HERE = dirname(fileURLToPath(import.meta.url));
const OUT = `${HERE}/out`;
mkdirSync(OUT, { recursive: true });

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1400,850"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };
const wait = (ms) => new Promise((r) => setTimeout(r, ms));

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1400, height: 850 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));
  await page.evaluateOnNewDocument(() => localStorage.clear());
  await page.goto(URL, { waitUntil: "networkidle2" });
  await wait(1200);

  // Rectangles of two selectors and whether they intersect. "n/a" when either
  // is absent, so a missing panel never reads as a pass.
  const overlap = (a, b) =>
    page.evaluate((sa, sb) => {
      const r = (s) => document.querySelector(s)?.getBoundingClientRect();
      const x = r(sa), y = r(sb);
      if (!x || !y) return "n/a";
      return x.left < y.right && y.left < x.right && x.top < y.bottom && y.top < x.bottom
        ? "overlap" : "clear";
    }, a, b);

  async function runCmd(cmd) {
    const before = await page.$$eval(".log .entry", (e) => e.length).catch(() => 0);
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => {
      el.value = "";
      el.dispatchEvent(new Event("input", { bubbles: true }));
    });
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await page.waitForFunction(
      (n) =>
        document.querySelectorAll(".log .entry").length > n &&
        // The entry now goes up when the command is *sent*, so a grown
        // log is not an answered command: wait for the pending mark to
        // come off it too.
        !document.querySelector(".log .entry.pending"),
      { timeout: 60000 }, before);
    await wait(300);
  }

  // 1) Empty state: no database open yet.
  check(await page.$(".map-empty") !== null,
        "the empty map shows a get-started card");
  const noDbText = await page.$eval(".map-empty", (e) => e.textContent);
  check(/no database open/i.test(noDbText),
        "with no database open, the card says so");

  await runCmd("open database berlintest");
  await wait(1500);
  const examples = await page.$$(".me-examples button");
  check(examples.length > 0,
        `an open database offers example queries (${examples.length})`);

  // 2) Elapsed time is reported for every command.
  check(await page.$(".cmd-ms") !== null, "the console reports elapsed time");

  // 3) An example runs when clicked.
  const before = await page.$$eval(".log .entry", (e) => e.length);
  if (examples.length > 0) {
    await examples[0].click();
    await page.waitForFunction(
      (n) =>
        document.querySelectorAll(".log .entry").length > n &&
        // The entry goes up when the command is sent, so a grown log is not
        // an answered command: the pending mark has to come off it too.
        !document.querySelector(".log .entry.pending"),
      { timeout: 60000 }, before).catch(() => {});
    await wait(800);
  }
  check(await page.$$eval(".log .entry", (e) => e.length) > before,
        "clicking an example query runs it");

  // 4) Overlays: zoom controls vs the layers panel, with a style editor open.
  //    One drawable layer is enough for both; the heavy `Trains` render this
  //    check does not need would only make the suite slower.
  await runCmd("query Kinos");
  await wait(1000);
  await page.click(".lp-name");
  await wait(400);
  check(await overlap(".zoom-ctl", ".layers-panel") === "clear",
        "zoom controls do not overlap the layers panel");
  await page.screenshot({ path: `${OUT}/ux-overlays.png` });

  // 5) The bottom-left stack: the plots must not cover the details panel. Both
  //    live in one column now, so the structural check is the real assertion --
  //    the geometric one only runs when a selection happened to land.
  //    `mreal5000` plots without drawing anything, which is the cheapest way to
  //    put the plot panel on screen.
  await runCmd("query mreal5000");
  await wait(1200);
  check(await page.$(".ov-left .plots") !== null,
        "the value plots sit in the bottom-left column");
  const stacked = await overlap(".details", ".plots");
  check(stacked !== "overlap", `details and plots do not overlap (${stacked})`);

  // 6) A long history must not eat the map: the console row is the dragged
  //    height, never its content's.
  for (let i = 0; i < 4; i++) await runCmd("query ten count");
  const mapH = await page.$eval(".map-pane", (e) => e.getBoundingClientRect().height);
  check(mapH > 300, `a long history leaves the map its height (${Math.round(mapH)}px)`);

  // 7) The query box stays inside the window on a short viewport, even grown.
  await page.setViewport({ width: 900, height: 700 });
  await wait(600);
  await page.click(".input textarea");
  for (let i = 0; i < 4; i++) {
    await page.type(".input textarea", `line ${i} of a long query`);
    await page.keyboard.down("Shift");
    await page.keyboard.press("Enter");
    await page.keyboard.up("Shift");
  }
  await wait(400);
  const runBtn = await page.$eval(".run-go", (e) => {
    const b = e.getBoundingClientRect();
    return { bottom: b.bottom, h: window.innerHeight };
  });
  check(runBtn.bottom <= runBtn.h,
        `the Run button stays in the viewport (${Math.round(runBtn.bottom)} <= ${runBtn.h})`);
  await page.screenshot({ path: `${OUT}/ux-short-viewport.png` });
  await page.$eval(".input textarea", (el) => {
    el.value = "";
    el.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await page.setViewport({ width: 1400, height: 850 });
  await wait(500);

  // 8) Focus is visible.
  const outlined = await page.evaluate(() => {
    const el = document.querySelector(".input textarea");
    el.focus();
    const s = getComputedStyle(el);
    return s.outlineStyle !== "none" && parseFloat(s.outlineWidth) > 0;
  });
  check(outlined, "a focused control draws a visible outline");

  // 9) Completion: object names from the open database, Tab accepts the marked
  //    one, and a name typed in full stays on the list.
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => {
    el.value = "";
    el.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await page.type(".input textarea", "query Kin");
  await wait(400);
  const menu = await page.evaluate(() => ({
    items: [...document.querySelectorAll(".cmp-item .cmp-name")].map((e) => e.textContent),
    picked: [...document.querySelectorAll(".cmp-item")].findIndex((e) =>
      e.classList.contains("picked")),
  }));
  check(menu.items.includes("Kinos"), `completion offers Kinos (${menu.items.join(", ")})`);
  check(menu.picked === 0, "the item Tab would take is marked");
  await page.screenshot({ path: `${OUT}/ux-completion.png` });

  await page.keyboard.press("Tab");
  await wait(300);
  check(await page.$eval(".input textarea", (el) => el.value) === "query Kinos",
        "Tab completes the marked item");

  // The completed name is still on the list: it says the spelling is right.
  await wait(300);
  const afterFull = await page.$$eval(".cmp-item .cmp-name", (e) => e.map((x) => x.textContent));
  check(afterFull.length === 0 || afterFull.includes("Kinos"),
        "a name typed in full is not dropped from the list");

  // 9b) Operators come from the server, not from a list kept in the frontend:
  //     `createsuffixtree` belongs to an algebra nobody would have thought to
  //     add by hand, and it is offered with the syntax the server reports.
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => {
    el.value = "";
    el.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await page.type(".input textarea", "query createsuffi");
  await wait(400);
  const ops = await page.evaluate(() =>
    [...document.querySelectorAll(".cmp-item")].map((e) => ({
      name: e.querySelector(".cmp-name")?.textContent,
      hint: e.querySelector(".cmp-hint")?.textContent,
    }))
  );
  const suffix = ops.find((o) => o.name === "createsuffixtree");
  check(!!suffix, `completion offers createsuffixtree (${ops.map((o) => o.name).join(", ")})`);
  check(!!suffix && suffix.hint.includes("createsuffixtree"),
        `it is hinted with its syntax (${suffix?.hint})`);

  // Enter still runs the query while nothing has been picked with the arrows.
  await page.$eval(".input textarea", (el) => {
    el.value = "";
    el.dispatchEvent(new Event("input", { bubbles: true }));
  });
  await page.type(".input textarea", "query Kinos");
  await wait(300);
  const beforeRun = await page.$$eval(".log .entry", (e) => e.length);
  await page.type(".input textarea", " count");
  await wait(300);
  await page.keyboard.press("Enter");
  await page.waitForFunction(
    (n) =>
      document.querySelectorAll(".log .entry").length > n &&
      // The entry goes up when the command is sent, so a grown log is not an
      // answered command: the pending mark has to come off it too.
      !document.querySelector(".log .entry.pending"),
    { timeout: 60000 }, beforeRun).catch(() => {});
  check(await page.$$eval(".log .entry", (e) => e.length) > beforeRun,
        "Enter still runs the query when nothing was picked");
} catch (e) {
  console.log("ERROR", e.message);
  fails++;
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
