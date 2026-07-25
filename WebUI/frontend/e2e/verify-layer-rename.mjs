// Verifies manual layer renaming: the style editor carries a name field seeded
// with the auto-derived name, typing into it retitles the layer everywhere it
// appears (row, details header, plot tooltip) while the original command stays
// on the row as its tooltip, and clearing the field restores the auto-name.
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

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1200, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  async function runCmd(cmd) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 700));
  }
  const rowName = () => page.$eval(".lp-name", (el) => el.textContent.trim());
  const rowTitle = () => page.$eval(".lp-name", (el) => el.getAttribute("title"));
  // Type into the rename field the way a user does, so React sees real input
  // events. Select-all via Ctrl+A rather than a triple click: the field is
  // narrower than the query text it holds, and a synthetic triple click lands a
  // caret mid-string instead of selecting the line.
  async function typeName(value) {
    await page.click(".lp-rename");
    await page.keyboard.down("Control");
    await page.keyboard.press("KeyA");
    await page.keyboard.up("Control");
    await page.keyboard.press("Backspace");
    if (value) await page.type(".lp-rename", value);
    await new Promise((r) => setTimeout(r, 300));
  }

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");

  const CMD = "query Flaechen feed head[5] consume";
  await runCmd(CMD);
  await page.waitForSelector(".lp-item", { timeout: 8000 });

  // 1) The field appears in the style editor, seeded with the auto-name.
  const autoName = await rowName();
  await page.click(".lp-name");
  await page.waitForSelector(".lp-style .lp-rename", { timeout: 3000 });
  const seeded = await page.$eval(".lp-rename", (el) => el.value);
  check(seeded === autoName,
        `rename field is seeded with the auto-name ("${seeded}" == "${autoName}")`);
  const placeholder = await page.$eval(".lp-rename", (el) => el.placeholder);
  check(placeholder === CMD,
        `rename field's placeholder is the full command ("${placeholder}")`);

  // 2) Typing retitles the row, and the command survives as its tooltip -- the
  //    name is a label, the command is the layer's identity.
  await typeName("Districts");
  check((await rowName()) === "Districts",
        `typing a name retitles the layer row (got "${await rowName()}")`);
  check((await rowTitle()) === CMD,
        `the row tooltip still holds the original command ("${await rowTitle()}")`);

  // 3) The name survives collapsing and reopening the style editor.
  await page.click(".lp-name");
  await new Promise((r) => setTimeout(r, 200));
  await page.click(".lp-name");
  await page.waitForSelector(".lp-style .lp-rename", { timeout: 3000 });
  check((await page.$eval(".lp-rename", (el) => el.value)) === "Districts",
        `the name persists across collapse/expand`);

  // 4) The details header reads the name live. It used to be snapshotted at
  //    click time, which left a stale title after a rename.
  const findTarget = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
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
      for (let y = R; y < h - R; y++)
        for (let x = R; x < w - R; x++)
          if (filled(x, y) && filled(x + R, y) && filled(x - R, y) &&
              filled(x, y + R) && filled(x, y - R))
            return { x: rect.left + (x / w) * rect.width,
                     y: rect.top + ((h - y) / h) * rect.height };
      return null;
    });
  let header = null;
  for (let attempt = 0; attempt < 5 && header === null; attempt++) {
    const target = await findTarget();
    if (!target) { await new Promise((r) => setTimeout(r, 300)); continue; }
    await page.mouse.click(target.x, target.y);
    await new Promise((r) => setTimeout(r, 300));
    header = await page.$eval(".details-head span", (el) => el.textContent.trim())
      .catch(() => null);
  }
  check(header === "Districts",
        `the details header shows the renamed layer (got "${header}")`);
  await page.screenshot({ path: `${OUT}/layer-rename.png` });

  // 5) Clearing the field falls back to the auto-derived name -- that is how a
  //    rename is undone, so it needs no separate reset control.
  await page.waitForSelector(".lp-style .lp-rename", { timeout: 3000 });
  await typeName("");
  check((await rowName()) === autoName,
        `clearing the field restores the auto-name (got "${await rowName()}")`);

  // 6) A renamed layer's plots carry the new name too (the plot label's tooltip
  //    is "<layer> · <type>").
  await runCmd("query mreal5000");
  await page.waitForSelector(".plot", { timeout: 8000 });
  const names = await page.$$eval(".lp-name", (els) => els.map((e) => e.textContent.trim()));
  await page.click(".lp-name"); // topmost row == the mreal layer
  await page.waitForSelector(".lp-style .lp-rename", { timeout: 3000 });
  await typeName("Speed");
  const plotTitle = await page.$eval(".plot-label", (el) => el.getAttribute("title"));
  check(plotTitle.startsWith("Speed"),
        `the plot label follows the rename ("${plotTitle}", rows were ${names.join(", ")})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
