import type { Plot } from "../api/client";
import type { RGB } from "../layers/useLayers";

export interface PlotEntry {
  plot: Plot;
  color: RGB;
  layerName: string;
}

interface Props {
  entries: PlotEntry[];
  /** Absolute POSIX seconds of the shared time domain start. */
  t0: number;
  /** Seconds into the shared domain (the timeline cursor). */
  time: number;
  duration: number;
  collapsed: boolean;
  onToggle: () => void;
}

const W = 250; // plot width in px
const H = 40; // plot height in px

const rgb = ([r, g, b]: RGB) => `rgb(${r},${g},${b})`;

/** Value at time `t` (absolute seconds). Step series carry explicit points at
 *  each unit boundary, so plain linear interpolation is correct for both kinds. */
function valueAt(series: [number, number][], t: number): number | null {
  if (series.length === 0) return null;
  if (t <= series[0][0]) return series[0][1];
  const last = series[series.length - 1];
  if (t >= last[0]) return last[1];
  for (let i = 0; i < series.length - 1; i++) {
    const [ta, va] = series[i];
    const [tb, vb] = series[i + 1];
    if (t >= ta && t <= tb) {
      if (tb === ta) return vb;
      return va + ((t - ta) / (tb - ta)) * (vb - va);
    }
  }
  return last[1];
}

function fmt(v: number): string {
  if (Number.isInteger(v)) return String(v);
  if (Math.abs(v) >= 100) return v.toFixed(0);
  return v.toFixed(2);
}

export function PlotPanel({
  entries,
  t0,
  time,
  duration,
  collapsed,
  onToggle,
}: Props) {
  if (entries.length === 0) return null;
  const now = t0 + time;

  return (
    <div className="plots">
      <div className="plots-head">
        <span>values</span>
        <button
          className="plots-toggle"
          onClick={onToggle}
          title={collapsed ? "Show value plots" : "Hide value plots"}
        >
          {collapsed ? "▴" : "▾"}
        </button>
      </div>

      {!collapsed &&
        entries.map(({ plot, color, layerName }, i) => {
          // Small multiples: each measure keeps its own y scale. Never a second
          // y-axis on a shared plot.
          const [lo, hi] = plot.valueRange;
          const span = hi - lo || 1;
          const x = (t: number) =>
            duration > 0 ? ((t - t0) / duration) * W : 0;
          const y = (v: number) => H - ((v - lo) / span) * H;
          const points = plot.series
            .map(([t, v]) => `${x(t).toFixed(1)},${y(v).toFixed(1)}`)
            .join(" ");
          const cur = valueAt(plot.series, now);
          const cx = x(now);
          // A bare object's plot is labelled with its type ("mreal"), which is
          // ambiguous once two are shown; name it after its layer instead.
          const label = plot.label === plot.type ? layerName : plot.label;

          return (
            <div className="plot" key={`${layerName}-${plot.label}-${i}`}>
              <div className="plot-title">
                {/* Identity is carried by the swatch + name, never colour alone,
                    and the text keeps its own ink colour. */}
                <span className="plot-swatch" style={{ background: rgb(color) }} />
                <span className="plot-label" title={`${layerName} · ${plot.type}`}>
                  {label}
                </span>
                <span className="plot-value">{cur === null ? "–" : fmt(cur)}</span>
              </div>
              <svg width={W} height={H} className="plot-svg">
                {/* recessive baseline */}
                <line x1={0} y1={H - 0.5} x2={W} y2={H - 0.5} className="plot-axis" />
                <polyline
                  points={points}
                  fill="none"
                  stroke={rgb(color)}
                  strokeWidth={2}
                  strokeLinejoin="round"
                  strokeLinecap="round"
                />
                {/* cursor at the timeline instant */}
                <line x1={cx} y1={0} x2={cx} y2={H} className="plot-cursor" />
                {cur !== null && (
                  <circle cx={cx} cy={y(cur)} r={3} fill={rgb(color)} className="plot-dot" />
                )}
              </svg>
              <div className="plot-range">
                <span>{fmt(lo)}</span>
                <span>{fmt(hi)}</span>
              </div>
            </div>
          );
        })}
    </div>
  );
}
