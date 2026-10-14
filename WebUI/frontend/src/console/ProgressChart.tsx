import { useState } from "react";
import type { ProgressSample } from "../api/client";

/** How the server's progress estimates moved while a query ran. One small chart per
 *  estimate, each against the line it would have followed had it been right
 *  from the start: the gap between the two is how far off it was.
 *
 *  The x axis is the server's CPU time since the query started, the clock the
 *  estimates are made with; the time estimate is in the same unit. */

const W = 300; // viewBox units; the SVG stretches to the column
const H = 72;
const PAD = 5; // keeps a line at the top or bottom of the scale off the edge

interface Measure {
  title: string;
  /** What the dashed line is, in the readout. */
  refName: string;
  value: (s: ProgressSample) => number;
  /** The reference at sample `s`, given the first and the last one. */
  ref: (s: ProgressSample, first: ProgressSample, last: ProgressSample) => number;
  fmt: (v: number) => string;
  /** A fixed scale; otherwise it spans the values shown -- from zero, a
   *  cardinality estimate drifting by 3 % is a flat line. */
  range?: [number, number];
}

const pct = (v: number) => `${Math.round(v)} %`;
const count = (v: number) => Math.round(v).toLocaleString();
const secs = (ms: number) =>
  ms < 10000 ? `${(ms / 1000).toFixed(1)} s` : `${Math.round(ms / 1000)} s`;

/** Share of the query's time gone by at `s`, 0..100: where progress would be if
 *  every step took as long as every other. */
function ideal(s: ProgressSample, first: ProgressSample, last: ProgressSample): number {
  const span = last[0] - first[0];
  return span > 0 ? ((s[0] - first[0]) / span) * 100 : 0;
}

const MEASURES: Measure[] = [
  {
    title: "progress",
    refName: "linear",
    value: (s) => s[3] * 100,
    ref: ideal,
    fmt: pct,
    range: [0, 100],
  },
  {
    title: "estimated cardinality",
    refName: "final",
    value: (s) => s[1],
    ref: (_s, _first, last) => last[1],
    fmt: count,
  },
  {
    title: "estimated total time",
    refName: "actual",
    value: (s) => s[2],
    ref: (_s, first, last) => last[0] - first[0],
    fmt: secs,
  },
];

/** The largest gap, in percentage points, between the progress estimate and
 *  the linear progress it should have shown -- the fold row's one-line answer
 *  to "how good was the estimate". */
export function progressGap(samples: ProgressSample[]): number {
  const first = samples[0];
  const last = samples[samples.length - 1];
  return samples.reduce(
    (gap, s) => Math.max(gap, Math.abs(s[3] * 100 - ideal(s, first, last))),
    0
  );
}

export function ProgressChart({ samples }: { samples: ProgressSample[] }) {
  // The sample under the pointer, shared by all three charts so one crosshair
  // reads all estimates at the same instant; null shows the latest.
  const [hover, setHover] = useState<number | null>(null);
  const first = samples[0];
  const last = samples[samples.length - 1];
  const span = last[0] - first[0] || 1;
  const x = (s: ProgressSample) => ((s[0] - first[0]) / span) * W;
  const at = hover !== null && hover < samples.length ? samples[hover] : last;

  /** The sample nearest the pointer, by time. */
  function pick(ev: React.PointerEvent<SVGSVGElement>) {
    const box = ev.currentTarget.getBoundingClientRect();
    const t = first[0] + ((ev.clientX - box.left) / box.width) * span;
    let best = 0;
    samples.forEach((s, i) => {
      if (Math.abs(s[0] - t) < Math.abs(samples[best][0] - t)) best = i;
    });
    setHover(best);
  }

  return (
    <div className="prog-chart">
      <div className="prog-legend">
        <span className="prog-key prog-key-est" /> estimate
        <span className="prog-key prog-key-ref" /> had it been right
      </div>
      {MEASURES.map((m) => {
        const shown = samples.flatMap((s) => [m.value(s), m.ref(s, first, last)]);
        const [lo, hi] = m.range ?? [Math.min(...shown), Math.max(...shown)];
        const span = hi - lo || Math.abs(hi) || 1;
        const y = (v: number) => H - PAD - ((v - lo) / span) * (H - 2 * PAD);
        const line = (f: (s: ProgressSample) => number) =>
          samples.map((s) => `${x(s).toFixed(1)},${y(f(s)).toFixed(1)}`).join(" ");
        return (
          <div className="prog-plot" key={m.title}>
            <div className="prog-title">
              <span>{m.title}</span>
              <span className="prog-read">
                {m.fmt(m.value(at))}
                <span className="prog-read-ref">
                  {" "}
                  · {m.refName} {m.fmt(m.ref(at, first, last))}
                </span>
              </span>
            </div>
            <div className="prog-frame">
              <div className="prog-scale">
                <span>{m.fmt(hi)}</span>
                <span>{m.fmt(lo)}</span>
              </div>
              <svg
                viewBox={`0 0 ${W} ${H}`}
                preserveAspectRatio="none"
                className="prog-svg"
                onPointerMove={pick}
                onPointerLeave={() => setHover(null)}
              >
                <line x1={0} y1={H} x2={W} y2={H} className="plot-axis" vectorEffect="non-scaling-stroke" />
                <polyline
                  points={line((s) => m.ref(s, first, last))}
                  className="prog-ref"
                  vectorEffect="non-scaling-stroke"
                />
                <polyline
                  points={line(m.value)}
                  className="prog-est"
                  vectorEffect="non-scaling-stroke"
                />
                {hover !== null && (
                  <line
                    x1={x(at)}
                    y1={0}
                    x2={x(at)}
                    y2={H}
                    className="plot-cursor"
                    vectorEffect="non-scaling-stroke"
                  />
                )}
              </svg>
            </div>
          </div>
        );
      })}
      <div className="plot-range prog-x">
        <span>0 s</span>
        <span>{hover !== null ? `at ${secs(at[0] - first[0])} · ` : ""}server CPU time</span>
        <span>{secs(last[0] - first[0])}</span>
      </div>
    </div>
  );
}
