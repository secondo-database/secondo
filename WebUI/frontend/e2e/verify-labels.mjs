// Verifies the opt-in attribute labels: a layer offers its non-spatial
// attributes (most label-like first), starts with none selected, and drawing
// really changes once one is chosen.
//
// Uses a plain kernel query so this runs against a server without the
// optimizer too.
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

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1280,900"],
});
const page = await browser.newPage();
await page.setViewport({ width: 1280, height: 900 });
page.on("pageerror", (e) => console.log("[pageerror]", e.message));
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
      // The entry now goes up when the command is *sent*, so a grown log
      // is not an answered command: wait for it to stop being pending too.
      !document.querySelector(".log .entry.pending"),
    { timeout: 60_000 },
    before
  );
}

// How much the canvas has drawn; text adds lit pixels where there were none.
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

// The label <select> of the topmost (newest) layer in the panel.
const labelSelect = () =>
  page.evaluate(() => {
    const s = document.querySelector(".lp-style .lp-label");
    if (!s) return null;
    return { options: [...s.options].map((o) => o.text), value: s.value };
  });

async function setLabel(value) {
  await page.select(".lp-style .lp-label", value);
  await new Promise((r) => setTimeout(r, 1800));
}

await run("open database berlintest");
// Flaechen is (Name string, GeoData region): one obvious label attribute.
await run("query Flaechen feed head[6] consume");
await page.waitForSelector(".lp-item", { timeout: 10_000 });
await new Promise((r) => setTimeout(r, 1500));

// Open the layer's style section.
await page.evaluate(() => document.querySelectorAll(".lp-name")[0]?.click());
await page.waitForSelector(".lp-style", { timeout: 5000 });

const sel = await labelSelect();
check(sel !== null, "the layer offers a label attribute", JSON.stringify(sel));
check(sel?.value === "", "no label is selected by default (opt-in)", sel?.value);
check(
  sel?.options[0] === "none" && sel?.options.includes("Name"),
  "candidates are offered with an explicit none",
  sel?.options.join(",")
);

const before = await drawnPixels();
await page.screenshot({ path: `${OUT}/labels-off.png` });

// Opt in.
await setLabel("Name");

const after = await drawnPixels();
await page.screenshot({ path: `${OUT}/labels-on.png` });
check(after > before, `labels draw on the map (${before} -> ${after} px)`);
check(
  (await labelSelect())?.value === "Name",
  "the chosen attribute sticks"
);

// Labels survive a light/dark switch without the view being touched.
//
// Their ink and halo swap over with the theme, and an earlier attempt to
// declutter them on the GPU made that switch wipe every label off the map until
// the next pan or zoom rebuilt deck's offscreen collision map. Nothing here
// moves the view, and `before`/`after` bracket what the labels are worth in
// pixels, so losing them is unmistakable.
const toggleTheme = async () => {
  await page.evaluate(() => {
    const b = [...document.querySelectorAll("button")].find((x) =>
      /light|dark/i.test(x.textContent ?? "")
    );
    b?.click();
  });
  await new Promise((r) => setTimeout(r, 1800));
};
const labelPixels = after - before;
await toggleTheme();
const switched = await drawnPixels();
await page.screenshot({ path: `${OUT}/labels-after-theme-switch.png` });
check(
  switched - before > labelPixels * 0.7,
  `labels survive a theme switch (${after} -> ${switched} px, ` +
    `labels are worth ${labelPixels})`
);
await toggleTheme();
const back = await drawnPixels();
check(
  back - before > labelPixels * 0.7,
  `and survive switching back (${switched} -> ${back} px)`
);

// ...and back off again.
await setLabel("");
const off = await drawnPixels();
check(off < after, `turning labels off removes them (${after} -> ${off} px)`);

// An individual object carries no attributes, so it gets a caption to type
// instead of a list to pick from. It goes on as a second layer; nothing on the
// map animates, so the pixel counts below still only move for the caption.
await run("query mehringdamm"); // a single point object: no tuple, no attributes
await page.waitForFunction(
  () => document.querySelectorAll(".lp-item").length > 1,
  { timeout: 10_000 }
);
await new Promise((r) => setTimeout(r, 1500));
await page.evaluate(() => document.querySelectorAll(".lp-name")[0]?.click());
await page.waitForSelector(".lp-style", { timeout: 5000 });

const textBox = await page.$(".lp-style .lp-label-text");
check(!!textBox, "a single object offers a label to type");
check(
  (await page.$(".lp-style .lp-label")) === null,
  "no attribute dropdown when there are no attributes"
);
check(
  (await page.$eval(".lp-style .lp-label-text", (el) => el.value)) === "",
  "the caption starts empty (opt-in)"
);
check(
  (await page.$eval(".lp-style .lp-label-text", (el) => el.placeholder)) ===
    "none",
  "the empty box says it draws nothing, like the dropdown's none"
);

const plain = await drawnPixels();
await page.click(".lp-style .lp-label-text");
await page.type(".lp-style .lp-label-text", "Mehringdamm");
await new Promise((r) => setTimeout(r, 1800));
const captioned = await drawnPixels();
await page.screenshot({ path: `${OUT}/labels-single-object.png` });
check(captioned > plain, `the caption draws (${plain} -> ${captioned} px)`);

// Clearing it takes the caption away again.
await page.click(".lp-style .lp-label-text");
await page.keyboard.down("Control");
await page.keyboard.press("KeyA");
await page.keyboard.up("Control");
await page.keyboard.press("Backspace");
await new Promise((r) => setTimeout(r, 1800));
check(
  (await page.$eval(".lp-style .lp-label-text", (el) => el.value)) === "",
  "the caption box is empty again"
);
const cleared = await drawnPixels();
check(cleared < captioned, `clearing it removes the caption (${captioned} -> ${cleared} px)`);

// The same box is what labels an individual moving object -- train7 is an
// mpoint, so it has no attributes either. It animates, which makes a pixel
// count meaningless here; the control being offered is the point.
await run("query train7");
await page.waitForFunction(
  () => document.querySelectorAll(".lp-item").length > 2,
  { timeout: 10_000 }
);
await new Promise((r) => setTimeout(r, 1500));
await page.evaluate(() => document.querySelectorAll(".lp-name")[0]?.click());
await page.waitForSelector(".lp-style", { timeout: 5000 });
check(
  !!(await page.$(".lp-style .lp-label-text")),
  "an individual moving object can be captioned too"
);

await browser.close();
console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
