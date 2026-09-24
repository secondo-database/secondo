// Verifies drawing query shapes on the map and storing them as objects, the
// WebUI's version of HoeseViewer's "Object Creation": each of the four tools
// draws a shape, the dialog stores it with `let <name> = [const <type> value
// ...]`, and the result is a real object the catalog lists and a query can use.
//
// Needs berlintest, which is Cartesian: a click there is already in the data's
// own units. One rect is also drawn under the BerlinMOD -> OSM projection, where
// the click is lon/lat and has to be mapped back before it is stored. Deletes
// what it created, including after a failure.
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";
const DB = "BERLINTEST";
const CREATED = [
  "drawtest_region",
  "drawtest_region_cw",
  "drawtest_rect",
  "drawtest_line",
  "drawtest_point",
  "drawtest_osm",
];

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };
const wait = (ms) => new Promise((r) => setTimeout(r, ms));

let page;
// A command on the page's own session, answered as JSON -- for reading a value
// back, where the console would only give text to scrape.
const api = (command, view = "auto") =>
  page.evaluate(
    async (command, view) => {
      const r = await fetch("/api/query", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        credentials: "same-origin",
        body: JSON.stringify({ command, view }),
      });
      return { ok: r.ok, body: await r.json().catch(() => null) };
    },
    command,
    view
  );

try {
  page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  await page.goto(URL, { waitUntil: "networkidle0" });
  await page.waitForSelector(".cat-db", { timeout: 15000 });

  check(!(await page.$(".draw-toggle")), "no draw button before a database is open");

  const dbBtn = await page.evaluateHandle(
    (n) => [...document.querySelectorAll(".cat-db")].find((b) => b.textContent.trim() === n),
    DB
  );
  await dbBtn.asElement().click();
  await page.waitForSelector(".cat-obj", { timeout: 15000 });
  await page.waitForSelector(".draw-toggle", { timeout: 5000 });
  check(true, "the draw button appears once a database is open");
  check(
    !(await page.$(".draw-ctl [data-draw]")),
    "and the shape types stay folded away until it is pressed"
  );
  await page.click(".draw-toggle");
  check(
    (await page.$$(".draw-ctl [data-draw]")).length === 4,
    "pressing it shows the four shape types"
  );
  await page.click(".draw-toggle");
  check(!(await page.$(".draw-ctl")), "pressing it again folds them away");

  // Leftovers of an aborted run would make every name below collide.
  for (const n of CREATED) await api(`delete ${n}`, "none");

  // Something to draw over, so the view is framed on Berlin.
  await page.click(".input textarea");
  await page.type(".input textarea", "query Kneipen");
  await page.keyboard.press("Escape");
  await page.keyboard.press("Enter");
  await page.waitForFunction(() => document.querySelectorAll(".lp-item").length > 0 &&
    !document.querySelector(".log .entry.pending"), { timeout: 30000 }).catch(() => undefined);
  await wait(800);

  const box = await (await page.$(".mapview")).boundingBox();
  const cx = box.x + box.width / 2;
  const cy = box.y + box.height / 2;
  const at = (dx, dy) => [cx + dx, cy + dy];
  const click = async ([x, y]) => { await page.mouse.click(x, y); await wait(350); };
  // The tools are folded behind one button and fold away after each shape.
  const tool = async (t) => {
    if (!(await page.$(".draw-ctl"))) await page.click(".draw-toggle");
    await page.click(`.draw-ctl [data-draw="${t}"]`);
  };
  const mode = () => page.$eval(".mapview", (el) => el.dataset.drawMode);

  const catalogNames = () =>
    page.$$eval(".cat-objs .cat-oname", (els) => els.map((e) => e.textContent.trim()));

  // Name the drawn shape and store it; returns the command the dialog showed.
  const save = async (name) => {
    await page.waitForSelector(".save-shape", { timeout: 5000 });
    const cmd = await page.$eval(".shape-cmd", (el) => el.textContent);
    // The dialog opens with the proposed name focused and selected, so typing
    // replaces it.
    await page.type("#shape-name", name);
    await page.click(".save-shape .gpx-import");
    await page.waitForFunction(() => !document.querySelector(".save-shape"), { timeout: 20000 })
      .catch(() => undefined);
    const err = await page.$eval(".save-shape pre.err", (e) => e.textContent).catch(() => "");
    check(!err, `${name} was stored${err ? " -- " + err : ""}`);
    if (err) await page.click(".save-shape .gpx-close");
    return cmd.replace(/^let \S+/, `let ${name}`);
  };

  // --- region: four clicks, finished by a double-click ----------------------
  await tool("region");
  check((await mode()) === "region", "the region tool is active");
  await page.waitForSelector(".draw-hint", { timeout: 2000 });
  await click(at(-120, -80));
  await click(at(120, -80));
  await click(at(120, 80));
  // A real double-click: both clicks land, the second one on the vertex the
  // first just placed, and the dblclick then finishes the shape.
  await page.mouse.click(...at(-120, 80), { count: 2 });
  await page.waitForSelector(".save-shape", { timeout: 5000 });
  check(true, "a double-click finishes the region and opens the dialog");
  check(
    (await page.$eval("#shape-name", (el) => el.value)) === "region1" ||
      /^region\d+$/.test(await page.$eval("#shape-name", (el) => el.value)),
    "the dialog proposes a free regionN name"
  );
  const regionCmd = await save("drawtest_region");
  check(
    /^let drawtest_region = \[const region value \(\(\(\(\S+ \S+\) \(\S+ \S+\) \(\S+ \S+\) \(\S+ \S+\)\)\)\)\]$/.test(regionCmd),
    `the region is one face, one cycle of four vertices (${regionCmd})`
  );
  check((await mode()) === "", "drawing ends after the shape is stored");
  check(!(await page.$(".draw-ctl")), "and the tools fold away again");

  // The same square the other way round: the kernel orders cycles itself.
  await tool("region");
  await click(at(-60, -40));
  await click(at(-60, 40));
  await click(at(60, 40));
  await click(at(60, -40));
  await page.keyboard.press("Enter");
  await save("drawtest_region_cw");

  // A crossing edge is refused where it is drawn, not by the server later.
  await tool("region");
  await click(at(-100, -100));
  await click(at(100, 100));
  await click(at(100, -100));
  await click(at(-100, 100)); // crosses the first edge
  const warn = await page.$eval(".draw-warn", (e) => e.textContent).catch(() => "");
  check(/cross/.test(warn), `a self-crossing edge is refused ("${warn}")`);
  await page.keyboard.press("Escape");
  check((await mode()) === "", "Esc leaves the draw mode");

  // --- rect: press, drag, release ------------------------------------------
  await tool("rect");
  await page.mouse.move(...at(-150, -100));
  await page.mouse.down();
  await page.mouse.move(...at(0, 0), { steps: 5 });
  await page.mouse.move(...at(150, 100), { steps: 5 });
  await page.mouse.up();
  const rectCmd = await save("drawtest_rect");
  const rv = (rectCmd.match(/value \(([^)]*)\)/)?.[1] ?? "").split(" ").map(Number);
  check(
    rv.length === 4 && rv[0] < rv[1] && rv[2] < rv[3],
    `the rect is (minx maxx miny maxy) (${rv.join(" ")})`
  );

  // --- line: three clicks and the ✓ button ---------------------------------
  await tool("line");
  await click(at(-100, 0));
  await click(at(0, 60));
  await click(at(100, 0));
  await page.click(".draw-finish");
  const lineCmd = await save("drawtest_line");
  check(
    (lineCmd.match(/\(\S+ \S+ \S+ \S+\)/g) ?? []).length === 2,
    `the line is two segments (${lineCmd})`
  );

  // --- point: one click ------------------------------------------------------
  await tool("point");
  await click(at(10, 10));
  const pointCmd = await save("drawtest_point");
  check(
    /value \(-?\d+(\.\d+)?(e[-+]?\d+)? -?\d+(\.\d+)?(e[-+]?\d+)?\)\]$/.test(pointCmd) &&
      /\./.test(pointCmd.split("value")[1]),
    `the point is two reals (${pointCmd})`
  );

  // --- the stored objects are real, usable objects --------------------------
  await page.waitForFunction(
    (wanted) => {
      const shown = [...document.querySelectorAll(".cat-objs .cat-oname")]
        .map((e) => e.textContent.trim());
      return wanted.every((n) => shown.includes(n));
    },
    { timeout: 20000, polling: 250 },
    CREATED.slice(0, 5)
  ).catch(() => undefined);
  const listed = await catalogNames();
  for (const n of CREATED.slice(0, 5)) check(listed.includes(n), `the catalog lists ${n}`);

  const num = async (cmd) => {
    const r = await api(cmd);
    const v = r.body?.scalar?.value;
    return { ok: r.ok, v: Number(String(v).trim()), raw: JSON.stringify(r.body).slice(0, 200) };
  };
  for (const n of ["drawtest_region", "drawtest_region_cw"]) {
    const a = await num(`query area(${n})`);
    check(a.ok && a.v > 0, `${n} has a positive area (${a.ok ? a.v : a.raw})`);
  }
  const inside = await num("query Kneipen feed filter[.GeoData inside drawtest_region] count");
  check(inside.ok && Number.isFinite(inside.v), `a region query runs against it (${inside.ok ? inside.v : inside.raw})`);
  const len = await num("query size(drawtest_line)");
  check(len.ok && len.v > 0, `the line has a length (${len.ok ? len.v : len.raw})`);
  const isRect = await api("query drawtest_rect");
  check(isRect.ok, "the rect reads back");

  // --- under a projection the stored shape is in the data's own units -------
  await page.select(".projection-ctl select", "berlinmod");
  await page.waitForFunction(
    () => document.querySelector(".mapview")?.dataset.geographic === "true",
    { timeout: 5000 }
  );
  await wait(1000);
  await tool("rect");
  await page.mouse.move(...at(-80, -60));
  await page.mouse.down();
  await page.mouse.move(...at(80, 60), { steps: 8 });
  await page.mouse.up();
  const osmCmd = await save("drawtest_osm");
  const ov = (osmCmd.match(/value \(([^)]*)\)/)?.[1] ?? "").split(" ").map(Number);
  check(
    ov.length === 4 && ov.every((v) => Math.abs(v) > 1000),
    `drawn over OSM, the rect is stored in BBBike units, not lon/lat (${ov.join(" ")})`
  );
  // And it lands where the Cartesian rect drawn at the same screen spot did:
  // both are around the middle of the same framing of Kneipen.
  const overlap = await api("query drawtest_osm intersects drawtest_rect");
  check(
    overlap.ok && /TRUE/i.test(JSON.stringify(overlap.body)),
    `it overlaps the rect drawn without a projection (${JSON.stringify(overlap.body?.text ?? overlap.body).slice(0, 80)})`
  );
} finally {
  if (page) for (const n of CREATED) await api(`delete ${n}`, "none").catch(() => undefined);
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
