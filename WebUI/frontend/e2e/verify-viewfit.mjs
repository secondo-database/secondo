// Verifies auto-fit on projection change and the zoom/fit controls: toggling the
// projection re-centers/zooms to the data (no "whole world"), and the +/-/fit
// buttons change the view.
import { createRequire } from "module";
const require = createRequire(import.meta.url);
const puppeteer = require("puppeteer-core");

const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";
const CHROMIUM = process.env.CHROMIUM ?? "/usr/bin/chromium";

const browser = await puppeteer.launch({
  executablePath: CHROMIUM,
  headless: "new",
  args: ["--no-sandbox", "--use-gl=angle", "--use-angle=swiftshader",
         "--enable-unsafe-swiftshader", "--window-size=1280,800"],
});

let fails = 0;
const check = (c, m) => { console.log(`${c ? "PASS" : "FAIL"} ${m}`); if (!c) fails++; };

try {
  const page = await browser.newPage();
  await page.setViewport({ width: 1280, height: 800 });
  page.on("pageerror", (e) => console.log("[pageerror]", e.message));

  async function runCmd(cmd) {
    await page.click(".input textarea");
    await page.$eval(".input textarea", (el) => (el.value = ""));
    await page.type(".input textarea", cmd);
    await page.keyboard.press("Enter");
    await new Promise((r) => setTimeout(r, 700));
  }
  const drawn = () =>
    page.evaluate(() => {
      const c = document.querySelector(".mapview canvas");
      const gl = c.getContext("webgl2", { preserveDrawingBuffer: true }) ||
                 c.getContext("webgl", { preserveDrawingBuffer: true });
      const px = new Uint8Array(c.width * c.height * 4);
      gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, px);
      let n = 0;
      for (let i = 0; i < px.length; i += 4)
        if (px[i + 3] > 10 && Math.max(px[i], px[i + 1], px[i + 2]) > 70) n++;
      return n;
    });
  const clickTitle = async (t) => {
    const h = await page.evaluateHandle(
      (title) => [...document.querySelectorAll(".zoom-ctl button")]
        .find((b) => b.title === title), t);
    await h.asElement().click();
    await new Promise((r) => setTimeout(r, 350));
  };

  await page.goto(URL, { waitUntil: "networkidle0" });
  await runCmd("open database berlintest");
  await runCmd("query thecenter");
  await new Promise((r) => setTimeout(r, 500));
  const flatPx = await drawn();
  check(flatPx > 1000, `data visible in flat view (${flatPx})`);

  // Toggle projection on -> data must still be well-framed on OSM.
  await page.select(".projection-ctl select", "berlinmod");
  await page.waitForSelector(".maplibregl-map", { timeout: 8000 });
  await new Promise((r) => setTimeout(r, 2500));
  const geoPx = await drawn();
  check(geoPx > 1000, `data auto-fit under BerlinMOD projection (${geoPx})`);

  // Toggle projection back off -> must re-fit (NOT show the whole world/empty).
  await page.select(".projection-ctl select", "none");
  await new Promise((r) => setTimeout(r, 600));
  const backPx = await drawn();
  check(backPx > 1000, `data re-fit after turning projection off (${backPx})`);

  // Zoom controls.
  await clickTitle("Zoom out");
  await clickTitle("Zoom out");
  await clickTitle("Zoom out");
  const outPx = await drawn();
  check(outPx < backPx, `zoom-out shrinks the data (${outPx} < ${backPx})`);

  await clickTitle("Fit to data");
  const fitPx = await drawn();
  check(Math.abs(fitPx - backPx) < backPx * 0.25,
        `fit restores framing (${fitPx} ~ ${backPx})`);

  await clickTitle("Zoom in");
  await clickTitle("Zoom in");
  const inPx = await drawn();
  check(inPx > fitPx, `zoom-in enlarges the data (${inPx} > ${fitPx})`);
} finally {
  await browser.close();
}

console.log(fails === 0 ? "RESULT: PASS" : `RESULT: FAIL (${fails})`);
process.exit(fails === 0 ? 0 : 1);
