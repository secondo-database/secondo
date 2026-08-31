// Verifies the link between the map and the table view: clicking an object on
// the map shows its whole tuple and jumps to the highlighted row, and clicking
// a row selects (and locates) its geometry.
//
// This is the web equivalent of the HoeseViewer's map <-> QueryResult coupling
// (SelMouseAdapter.mouseClicked -> JList.setSelectedValue ->
// QueryListSelectionListener), including its `isMouseSelected` guard: a
// selection made on the map must not move the map.
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
         "--enable-unsafe-swiftshader", "--window-size=1200,800"],
});

let fails = 0;
const check = (cond, msg) => { console.log(`${cond ? "PASS" : "FAIL"} ${msg}`); if (!cond) fails++; };
const wait = (ms) => new Promise((r) => setTimeout(r, ms));

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  // Wait for the answer rather than guessing how long it takes; see
  // verify-table.mjs, which this is copied from.
  async function runCmd(cmd) {
    const before = await page.$$eval(".log .entry", (els) => els.length);
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await page.waitForFunction(
      (n) =>
        document.querySelectorAll(".log .entry").length > n &&
        !document.querySelector(".log .entry.pending"),
      { timeout: 30000 },
      before
    );
    await wait(200);
  }

  /** Click the button in `scope` whose text contains `label`. */
  async function clickButton(scope, label) {
    const handle = await page.evaluateHandle(
      (sel, text) =>
        [...document.querySelectorAll(`${sel} button`)].find((b) =>
          b.textContent.includes(text)
        ),
      scope,
      label
    );
    const el = handle.asElement();
    if (!el) throw new Error(`no "${label}" button in ${scope}`);
    await el.click();
  }

  /** A filled pixel in the interior of a drawn shape, in page coordinates.
   *  Requiring the neighbourhood to be filled too avoids landing on a thin line
   *  or an anti-aliased edge, where picking legitimately misses. */
  const findTarget = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      if (!c) return null;
      const rect = c.getBoundingClientRect();
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const w = c.width, h = c.height;
      const px = new Uint8Array(w * h * 4);
      gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, px);
      const filled = (x, y) => {
        const i = (y * w + x) * 4;
        return px[i + 3] > 60 && Math.max(px[i], px[i + 1], px[i + 2]) > 60;
      };
      const R = 4;
      for (let y = R; y < h - R; y++) {
        for (let x = R; x < w - R; x++) {
          if (filled(x, y) && filled(x + R, y) && filled(x - R, y) &&
              filled(x, y + R) && filled(x, y - R)) {
            // readPixels is bottom-up; CSS is top-down.
            return {
              x: rect.left + (x / w) * rect.width,
              y: rect.top + ((h - y) / h) * rect.height,
            };
          }
        }
      }
      return null;
    });

  /** The map's current view, read off the deck canvas as a cheap fingerprint:
   *  a re-fit changes what is drawn, a pure selection does not move anything
   *  except the highlight. */
  const canvasFingerprint = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const px = new Uint8Array(c.width * c.height * 4);
      gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, px);
      // Column occupancy: insensitive to the highlight's few hundred pixels,
      // sensitive to the whole scene moving.
      const cols = [];
      for (let x = 0; x < c.width; x += 16) {
        let n = 0;
        for (let y = 0; y < c.height; y += 4)
          if (px[(y * c.width + x) * 4 + 3] > 10) n++;
        cols.push(n);
      }
      return cols.join(",");
    });

  await page.goto(URL, { waitUntil: "networkidle2" });
  await page.waitForSelector(".cat-db", { timeout: 15000 });
  await runCmd("open database berlintest");
  await page.waitForSelector(".input textarea", { timeout: 3000 });

  // Kinos: a relation with a point attribute and several scalar ones, so the
  // tuple has more in it than the geometry.
  await runCmd("query Kinos");
  await wait(600);

  // --- the cursor -------------------------------------------------------
  // deck's default resting cursor is `grab`, an open hand that covers the very
  // feature being aimed at. Arrow at rest, hand only over something pickable --
  // so the cursor says whether a click would land before it is made.
  const cursor = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      for (let el = c; el; el = el.parentElement)
        if (el.style?.cursor) return el.style.cursor;
      return "none";
    });
  const mapBox = await (await page.$(".mapview canvas")).boundingBox();
  await page.mouse.move(mapBox.x + 30, mapBox.y + 30);
  await wait(400);
  check((await cursor()) === "default", `the map rests under an arrow, not a hand`);

  // --- map -> card ------------------------------------------------------
  let row = null;
  for (let attempt = 0; attempt < 6 && row === null; attempt++) {
    const target = await findTarget();
    if (!target) { await wait(300); continue; }
    await page.mouse.click(target.x, target.y);
    await wait(300);
    row = await page.$eval(".details", (el) => el.dataset.row).catch(() => null);
    if (row === "") row = null;
  }
  check(row !== null, `clicking a feature selects a tuple (row ${row})`);
  check((await cursor()) === "pointer", `hovering a pickable feature shows the hand`);

  // The whole tuple, not just the scalar properties the GeoJSON carries. Kinos
  // has a point attribute, which `_scalar` drops -- so a card built from the
  // table has strictly more lines than one built from the properties.
  const cardKeys = await page.$$eval(".details .dk", (els) =>
    els.map((e) => e.textContent.trim())
  );
  const marked = cardKeys.find((k) => k.startsWith("◆"));
  check(marked !== undefined, `the clicked geometry attribute is marked (${marked ?? "none"})`);
  // The marked attribute is the *spatial* one, and geojson._scalar drops those
  // from a feature's properties -- so its presence is proof the card is built
  // from the result's rows and shows the whole tuple, which is the whole point
  // of the change. It must not be the only line either.
  check(
    cardKeys.length > 1,
    `the card lists the geometry attribute and ${cardKeys.length - 1} more (${cardKeys.join(", ")})`
  );

  // --- the isMouseSelected guard ---------------------------------------
  // Selecting on the map must not move the map. Compare before/after a click on
  // a *different* feature: only the highlight may change, not the framing.
  const before = await canvasFingerprint();
  const second = await findTarget();
  if (second) {
    await page.mouse.click(second.x, second.y);
    await wait(400);
  }
  const after = await canvasFingerprint();
  check(before === after, "selecting on the map does not move the map");

  // --- card -> table ----------------------------------------------------
  const cardRow = await page.$eval(".details", (el) => el.dataset.row);
  await clickButton(".details-foot", "show row in table");
  await page.waitForSelector(".tv-grid", { timeout: 10000 });
  await wait(400);

  const selected = await page
    .$eval(".tv-grid tr.tv-selected", (el) => el.dataset.row)
    .catch(() => null);
  check(
    selected === cardRow,
    `the table highlights the clicked tuple (row ${selected}, wanted ${cardRow})`
  );

  const centred = await page.$eval(".tv-grid tr.tv-selected", (el) => {
    const box = el.getBoundingClientRect();
    const pane = el.closest(".tv-scroll").getBoundingClientRect();
    return box.top >= pane.top && box.bottom <= pane.bottom;
  });
  check(centred, "the highlighted row is scrolled into view");

  // --- table -> map -----------------------------------------------------
  // A different row: clicking it moves the selection with no tab switch.
  const other = await page.evaluate(() => {
    const rows = [...document.querySelectorAll(".tv-grid tbody tr")];
    const target = rows.find((r) => !r.classList.contains("tv-selected"));
    if (!target) return null;
    target.click();
    return target.dataset.row;
  });
  await wait(300);
  const moved = await page
    .$eval(".tv-grid tr.tv-selected", (el) => el.dataset.row)
    .catch(() => null);
  check(other !== null && moved === other, `clicking a row selects it (row ${moved})`);

  // ◎ switches to the map and fits the view to that tuple, which *is* allowed
  // to move the map -- it is the one path that may.
  const framing = await page.evaluate(() => {
    const el = document.querySelector(".tv-grid tr.tv-selected .tv-locate");
    if (!el) return false;
    el.click();
    return true;
  });
  check(framing, "the selected row offers a locate button");
  await wait(700);

  const onMap = await page.$eval(
    '.result-tabs [data-tab="map"]',
    (el) => el.getAttribute("aria-selected") === "true"
  );
  check(onMap, "locating a row switches back to the map");

  const refit = await canvasFingerprint();
  check(refit !== after, "locating a row moves the map to that tuple");

  // The card follows the grid's selection, and still names the tuple.
  const cardAfter = await page.$eval(".details", (el) => el.dataset.row).catch(() => null);
  check(cardAfter === other, `the card follows the grid (row ${cardAfter})`);

  // --- a tuple the grid's page does not hold -----------------------------
  // `Kinos` fits on one page, so this needs a relation that does not: the card
  // has to fetch that one row itself rather than show a hole, and it must do so
  // without moving the page the user put the grid on.
  await page.click('.result-tabs [data-tab="map"]');
  await runCmd("query strassen");
  await wait(1200);
  await page.evaluate(() =>
    [...document.querySelectorAll(".geohint button")]
      .reverse()
      .find((b) => b.textContent.includes("show as table"))
      .click()
  );
  await page.waitForSelector(".tv-grid", { timeout: 20000 });
  await wait(800);
  await page.evaluate(() => document.querySelectorAll(".tv-grid tbody tr")[3].click());
  await wait(300);
  const pagedAway = await page.evaluate(() => {
    const last = [...document.querySelectorAll(".tv-pager button")].find((b) =>
      b.textContent.includes("\u203a\u203a")
    );
    if (!last || last.disabled) return false;
    last.click();
    return true;
  });
  check(pagedAway, "a multi-page relation offers a pager to page away with");
  if (pagedAway) {
    await page.waitForFunction(
      () => !/rows 1[–-]/.test(document.querySelector(".tv-range")?.textContent ?? ""),
      { timeout: 20000 }
    );
    const pageRange = await page.$eval(".tv-range", (el) => el.textContent);
    await page.click('.result-tabs [data-tab="map"]');
    await wait(2500);
    const keys = await page.$$eval(".details .dk", (els) =>
      els.map((e) => e.textContent.trim())
    );
    const holes = await page.$$eval(".details .dr-unloaded", (els) => els.length);
    check(
      keys.some((k) => k.startsWith("◆")) && holes === 0,
      `the card fetches a row the grid's page lacks (${keys.join(", ")})`
    );
    // Back to that table: it must still be where it was left.
    await page.evaluate(() => {
      [...document.querySelectorAll('.result-tabs [role="tab"]')]
        .reverse()
        .find((b) => b.dataset.tab !== "map")
        ?.click();
    });
    await wait(800);
    check(
      (await page.$eval(".tv-range", (el) => el.textContent)) === pageRange,
      `fetching for the card leaves the grid on its page (${pageRange})`
    );
  }

  await page.screenshot({ path: `${OUT}/select-link.png` });
} finally {
  await browser.close();
}

process.exit(fails === 0 ? 0 : 1);
