// Verifies symbolic trajectories (mlabel/mstring) drawn beside a moving point:
// every one of them gets a line, the lines are read at the current instant
// rather than being constants of the tuple, and a gap in a series takes its
// line away without taking the dot away.
//
// Stock berlintest has no mlabel objects, so the track builds two out of
// train7's own units -- `Half` changes once at the midpoint (so seeking must
// change the text) and `Phase` changes every third unit (so there is a second
// line to stack). Both are plain kernel queries; this runs against a server
// without the optimizer too.
import { createRequire } from "module";
import { mkdirSync } from "fs";
import { dirname, join } from "path";
import { fileURLToPath } from "url";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const HERE = dirname(fileURLToPath(import.meta.url));
const OUT = join(HERE, "out");
mkdirSync(OUT, { recursive: true });

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

// One mlabel over train7's units, valued by `expr` over the unit counter N.
const mlabel = (expr) =>
  "units(train7) transformstream addcounter[N, 1] projectextend[" +
  `; L: the_unit(tolabel(${expr}), inst(initial(.Elem)), ` +
  "inst(final(.Elem)), TRUE, FALSE)] makemvalue[L]";

const SYMBOLIC =
  "query intstream(1,1) transformstream projectextend[; Trip: train7" +
  `, Half: ${mlabel('ifthenelse(.N < 90, "first", "second")')}` +
  `, Phase: ${mlabel('ifthenelse((.N mod 3) = 0, "stop", "run")')}` +
  "] consume";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1280,900"],
});
const page = await browser.newPage();
await page.setViewport({ width: 1280, height: 900 });
const pageErrors = [];
page.on("pageerror", (e) => {
  pageErrors.push(e.message);
  console.log("[pageerror]", e.message);
});
await page.goto(URL, { waitUntil: "networkidle0" });

let fails = 0;
const check = (ok, m, d) => {
  console.log(`${ok ? "ok  " : "FAIL"} ${m}${d ? ` -- ${d}` : ""}`);
  if (!ok) fails++;
};

async function run(cmd) {
  const before = (await page.$$(".log .entry")).length;
  await page.click(".input textarea");
  await page.$eval(".input textarea", (el) => (el.value = ""));
  await page.type(".input textarea", cmd);
  await page.keyboard.press("Enter");
  await page.waitForFunction(
    (n) =>
      document.querySelectorAll(".log .entry").length > n &&
      !document.querySelector(".log .entry.pending"),
    { timeout: 60_000 },
    before
  );
}

// What the symbolic trajectories say this frame; MapView puts it on .mapview
// because the feature is the text, which no pixel count can report.
const symbolic = () =>
  page.$eval(".mapview", (el) => el.dataset.symbolicLabels ?? "");

const drawnPixels = () =>
  page.evaluate(() => {
    const c = document.querySelector(".mapview canvas");
    if (!c) return -1;
    const gl =
      c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
      c.getContext("webgl", { preserveDrawingBuffer: true });
    if (!gl) return -1;
    const px = new Uint8Array(c.width * c.height * 4);
    gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, px);
    let drawn = 0;
    for (let i = 0; i < px.length; i += 4) if (px[i + 3] > 10) drawn++;
    return drawn;
  });

async function seekFraction(frac) {
  await page.$eval(
    ".tl-range",
    (el, f) => {
      const max = Number(el.max);
      const nativeSetter = Object.getOwnPropertyDescriptor(
        window.HTMLInputElement.prototype,
        "value"
      ).set;
      nativeSetter.call(el, String(max * f));
      el.dispatchEvent(new Event("input", { bubbles: true }));
    },
    frac
  );
  await new Promise((r) => setTimeout(r, 900));
}

await run("open database berlintest");
await run(SYMBOLIC);
await page.waitForSelector(".lp-item", { timeout: 15_000 });
await page.waitForSelector(".tl-range", { timeout: 15_000 });
await new Promise((r) => setTimeout(r, 1500));

// Pause: the assertions below are about a specific instant.
await page.click(".tl-play");
await new Promise((r) => setTimeout(r, 300));

await seekFraction(0.2);
const early = await symbolic();
await page.screenshot({ path: `${OUT}/symbolic-t20.png` });
check(early !== "", "symbolic labels are drawn without being asked for", early);
check(
  early.split(" / ").length === 2,
  "every symbolic attribute gets a line",
  early
);

await seekFraction(0.8);
const late = await symbolic();
await page.screenshot({ path: `${OUT}/symbolic-t80.png` });
check(late !== "", "still labelled later in the animation", late);
// The whole feature in one assertion: the same dot, a different text, driven
// by the clock rather than by anything in the tuple.
check(early !== late, `the text changes over time (${early} -> ${late})`);
check(
  early.startsWith("first") && late.startsWith("second"),
  "and changes to the value the mlabel actually holds there",
  `${early} | ${late}`
);

// --- the layer-panel controls ----------------------------------------------
// Which trajectories are drawn, and whether each line names its attribute.
await page.evaluate(() => document.querySelectorAll(".lp-name")[0]?.click());
await page.waitForSelector(".lp-style", { timeout: 5000 });

const symbolicBoxes = () =>
  page.evaluate(() =>
    [...document.querySelectorAll(".lp-symbolic-attr")].map((b) => ({
      attr: b.dataset.attr,
      checked: b.checked,
    }))
  );

const boxes = await symbolicBoxes();
check(
  boxes.length === 2 && boxes.every((b) => b.checked),
  "every trajectory gets a checkbox, all ticked",
  JSON.stringify(boxes)
);
check(
  boxes.map((b) => b.attr).join(",") === "Half,Phase",
  "in relation-schema order",
  boxes.map((b) => b.attr).join(",")
);

async function toggle(selector) {
  await page.click(selector);
  await new Promise((r) => setTimeout(r, 900));
}

async function setPrefix(value) {
  await page.select(".lp-symbolic-prefix", value);
  await new Promise((r) => setTimeout(r, 900));
}

check(
  await page.$eval(".lp-symbolic-head", (el) => el.textContent) ===
    "MLabel options",
  "the section is headed MLabel options"
);
check(
  (await page.$eval(".lp-symbolic-prefix", (el) => el.value)) === "false",
  "the key prefix is off by default"
);

// Key prefix on.
await setPrefix("true");
const named = await symbolic();
check(
  named.startsWith("Half: ") && named.includes("Phase: "),
  "the key prefix names each line after its attribute",
  named
);

// Drop one of the two.
await toggle('.lp-symbolic-attr[data-attr="Phase"]');
const one = await symbolic();
check(
  one.split(" / ").length === 1 && one.startsWith("Half: "),
  "unticking a trajectory takes its line away and leaves the other",
  one
);

// Prefix off again, still one line.
await setPrefix("false");
const bare = await symbolic();
check(bare === "second", "and the prefix comes back off", bare);

// Drop the last one: no symbolic text at all, but the dot is still drawn.
await toggle('.lp-symbolic-attr[data-attr="Half"]');
check((await symbolic()) === "", "unticking all of them draws no label");
check((await drawnPixels()) > 0, "the moving point itself is untouched");

// Back on.
await toggle('.lp-symbolic-attr[data-attr="Half"]');
await toggle('.lp-symbolic-attr[data-attr="Phase"]');
const restored = await symbolic();
check(
  restored.split(" / ").length === 2,
  "ticking them again restores both lines",
  restored
);

// A moving point with no symbolic attribute is untouched: no lines, no
// checkboxes, and the opt-in attribute label still governs on its own.
await run("query train7");
await page.waitForFunction(
  () => document.querySelectorAll(".lp-item").length > 1,
  { timeout: 15_000 }
);
await new Promise((r) => setTimeout(r, 1500));
await page.evaluate(() => document.querySelectorAll(".lp-name")[0]?.click());
await page.waitForSelector(".lp-style", { timeout: 5000 });
check(
  (await page.$(".lp-symbolic")) === null,
  "no symbolic section for a result that carries none"
);

// An mlabel on its own draws nothing -- it has no moving point to ride -- and
// must not error or leave a layer that only distorts the shared timeline.
const layersBefore = (await page.$$(".lp-item")).length;
await run(`query ${mlabel('ifthenelse(.N < 90, "first", "second")')}`);
await new Promise((r) => setTimeout(r, 1500));
check(
  (await page.$$(".lp-item")).length === layersBefore,
  "an mlabel with no moving point makes no layer"
);
check(pageErrors.length === 0, "no page errors", pageErrors.join("; "));

const drawn = await drawnPixels();
check(drawn > 0, `the map is still drawing (${drawn} px)`);

await browser.close();
console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
