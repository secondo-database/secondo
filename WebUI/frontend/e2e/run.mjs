#!/usr/bin/env node
// Runs the end-to-end suite: every e2e/verify-*.mjs, sequentially, against a
// running stack (SecondoMonitor + FastAPI bridge + Vite dev server).
//
//   node e2e/run.mjs            # everything
//   node e2e/run.mjs plots      # only files whose name contains "plots"
//   npm run e2e -- plots        # same, via npm
//
// Each check script is a standalone program that exits 0 on success and
// non-zero on failure, so this runner keys off exit codes rather than parsing
// their output. Output from a failing script is echoed for diagnosis.
import { readdirSync } from "fs";
import { spawnSync } from "child_process";
import { dirname, join } from "path";
import { fileURLToPath } from "url";

const HERE = dirname(fileURLToPath(import.meta.url));
const filter = process.argv[2];
const TIMEOUT_MS = 180_000;

const BACKEND = process.env.WEBUI_API ?? "http://127.0.0.1:8000/api/health";
const URL = process.env.WEBUI_URL ?? "http://127.0.0.1:5173/";

async function reachable(url) {
  try {
    const res = await fetch(url, { signal: AbortSignal.timeout(3000) });
    return res.ok;
  } catch {
    return false;
  }
}

// Fail fast with a useful message instead of a wall of navigation timeouts.
const [apiUp, webUp] = await Promise.all([reachable(BACKEND), reachable(URL)]);
if (!apiUp || !webUp) {
  console.error("The e2e stack is not reachable:");
  if (!apiUp) console.error(`  bridge  ${BACKEND}  DOWN  (uvicorn app.main:app --port 8000)`);
  if (!webUp) console.error(`  web     ${URL}  DOWN  (npm run dev)`);
  console.error("Also make sure a SecondoMonitor is listening on port 1234.");
  process.exit(2);
}

const files = readdirSync(HERE)
  .filter((f) => /^verify-.*\.mjs$/.test(f))
  .filter((f) => !filter || f.includes(filter))
  .sort();

if (files.length === 0) {
  console.error(filter ? `No e2e checks match "${filter}".` : "No e2e checks found.");
  process.exit(2);
}

const name = (f) => f.replace(/^verify-/, "").replace(/\.mjs$/, "");
const failed = [];
const started = Date.now();

for (const file of files) {
  const t0 = Date.now();
  const run = spawnSync(process.execPath, [join(HERE, file)], {
    encoding: "utf8",
    timeout: TIMEOUT_MS,
    env: process.env,
  });
  const secs = ((Date.now() - t0) / 1000).toFixed(0);
  const ok = run.status === 0;
  const why = run.signal ? ` (killed: ${run.signal})` : "";
  console.log(`${ok ? "PASS" : "FAIL"}  ${name(file).padEnd(16)} ${secs}s${why}`);
  if (!ok) {
    failed.push(name(file));
    const noise = /\[vite\]|DevTools|React DevTools|Download the React/;
    const body = `${run.stdout ?? ""}${run.stderr ?? ""}`
      .split("\n")
      .filter((l) => l.trim() && !noise.test(l))
      .map((l) => `      ${l}`)
      .join("\n");
    if (body) console.log(body);
  }
}

const total = ((Date.now() - started) / 1000).toFixed(0);
console.log(
  `\n${files.length - failed.length}/${files.length} checks passed in ${total}s` +
    (failed.length ? `  —  failed: ${failed.join(", ")}` : "")
);
process.exit(failed.length === 0 ? 0 : 1);
