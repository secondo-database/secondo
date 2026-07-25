import { useCallback, useEffect, useMemo, useState } from "react";
import { runQuery } from "./api/client";
import { Console, type Entry } from "./console/Console";
import { Catalog } from "./catalog/Catalog";
import { MapView } from "./map/MapView";
import { Timeline } from "./timeline/Timeline";
import { useAnimator } from "./timeline/useAnimator";
import { useLayers } from "./layers/useLayers";
import { LayersPanel } from "./layers/LayersPanel";
import { PlotPanel, type PlotEntry } from "./plots/PlotPanel";
import { PROJECTION_LABEL, type Projection } from "./map/projection";
import { applyTheme, loadTheme, type Theme } from "./theme";

const DB_CMD = /^\s*(open|close|create|delete|restore)\s+database/i;

// Persisted panel geometry.
interface Geometry {
  catalogW: number;
  consoleH: number;
  catalogCollapsed: boolean;
  consoleCollapsed: boolean;
}
const GEO_KEY = "secondo.webui.geometry";
// The map is the product, so it gets the space: the console is docked under it
// across the full width and kept short, since the query history is rarely
// needed at length. The catalog collapses away, and so does the history.
const DEFAULT_GEO: Geometry = {
  catalogW: 210,
  consoleH: 220,
  catalogCollapsed: false,
  consoleCollapsed: false,
};
const RAIL = 34; // collapsed catalog width

function loadGeometry(): Geometry {
  try {
    const raw = localStorage.getItem(GEO_KEY);
    return raw ? { ...DEFAULT_GEO, ...JSON.parse(raw) } : DEFAULT_GEO;
  } catch {
    return DEFAULT_GEO;
  }
}

const clamp = (v: number, lo: number, hi: number) => Math.min(hi, Math.max(lo, v));

// Drag a splitter: capture the size at mousedown and apply deltas until mouseup.
function startDrag(
  e: React.MouseEvent,
  start: number,
  apply: (start: number, dx: number, dy: number) => void
) {
  e.preventDefault();
  const sx = e.clientX;
  const sy = e.clientY;
  const move = (ev: MouseEvent) => apply(start, ev.clientX - sx, ev.clientY - sy);
  const up = () => {
    window.removeEventListener("mousemove", move);
    window.removeEventListener("mouseup", up);
    document.body.classList.remove("dragging");
  };
  window.addEventListener("mousemove", move);
  window.addEventListener("mouseup", up);
  document.body.classList.add("dragging");
}

export function App() {
  const {
    layers,
    add,
    remove,
    toggle,
    move,
    rename,
    setStyle,
    clear,
    selected,
    setSelected,
  } = useLayers();

  const [history, setHistory] = useState<Entry[]>([]);
  const [busy, setBusy] = useState(false);
  // Authoritative database state comes from the catalog (see Catalog.onState).
  const [openDb, setOpenDb] = useState<string | null>(null);
  // Whether the connected server runs SQL; null until the catalog has asked.
  const [optimizer, setOptimizer] = useState<boolean | null>(null);
  const [refreshKey, setRefreshKey] = useState(0);
  const [projection, setProjection] = useState<Projection>("none");
  const [geo, setGeo] = useState<Geometry>(loadGeometry);
  const [theme, setTheme] = useState<Theme>(loadTheme);

  useEffect(() => {
    applyTheme(theme);
  }, [theme]);

  useEffect(() => {
    try {
      localStorage.setItem(GEO_KEY, JSON.stringify(geo));
    } catch {
      /* storage unavailable */
    }
  }, [geo]);

  // Release the SECONDO connection when the tab goes away. Each session holds a
  // server-side process, so waiting for the idle timeout would pile them up.
  // `pagehide` fires for closes, navigations and reloads (unlike `unload` it is
  // bfcache-safe), and sendBeacon still delivers during teardown.
  useEffect(() => {
    const release = () => navigator.sendBeacon?.("/api/close");
    window.addEventListener("pagehide", release);
    return () => window.removeEventListener("pagehide", release);
  }, []);

  // Single command runner shared by the console and the catalog. Returns
  // whether the command succeeded so callers can refresh.
  const run = useCallback(
    async (command: string): Promise<boolean> => {
      setBusy(true);
      try {
        const res = await runQuery(command);
        const fc = res.geojson ?? null;
        const temp = res.temporal ?? null;
        setHistory((h) => [
          ...h,
          {
            command,
            result: res.text,
            hasGeometry: !!fc,
            hasMotion: !!temp,
            plan: res.plan ?? undefined,
            costs: res.costs ?? undefined,
            message: res.message ?? undefined,
            planOnly: res.plan_only,
            executedByOptimizer: res.executed_by_optimizer,
          },
        ]);
        if (fc || temp) add(command, fc, temp);
        // The catalog refreshes on this and reports the new database state back.
        // `create table`, `drop table` and `let x = select ...` never match
        // DB_CMD: they arrive as commands the optimizer executed itself.
        if (DB_CMD.test(command) || res.executed_by_optimizer)
          setRefreshKey((k) => k + 1);
        return true;
      } catch (e) {
        setHistory((h) => [
          ...h,
          { command, error: e instanceof Error ? e.message : String(e) },
        ]);
        return false;
      } finally {
        setBusy(false);
      }
    },
    [add]
  );

  const visible = useMemo(() => layers.filter((l) => l.visible), [layers]);

  const domain = useMemo(() => {
    let min = Infinity;
    let max = -Infinity;
    for (const l of visible) {
      if (l.temporal) {
        min = Math.min(min, l.temporal.timeDomain[0]);
        max = Math.max(max, l.temporal.timeDomain[1]);
      }
    }
    return min < max ? ([min, max] as [number, number]) : null;
  }, [visible]);

  const duration = domain ? domain[1] - domain[0] : 0;
  const { time, setTime, playing, setPlaying, speed, setSpeed } =
    useAnimator(duration);

  // Scalar moving values (mreal/mint) from every visible layer, each carrying
  // its layer's colour so identity matches the map.
  const plotEntries = useMemo<PlotEntry[]>(
    () =>
      visible.flatMap((l) =>
        (l.temporal?.plots ?? []).map((plot) => ({
          plot,
          color: l.style.color,
          layerName: l.name,
        }))
      ),
    [visible]
  );
  const [plotsCollapsed, setPlotsCollapsed] = useState(false);

  return (
    <div
      className={
        "app" +
        (geo.catalogCollapsed ? " cat-collapsed" : "") +
        (geo.consoleCollapsed ? " con-collapsed" : "")
      }
      style={
        {
          "--cat": geo.catalogCollapsed ? `${RAIL}px` : `${geo.catalogW}px`,
          "--conh": `${geo.consoleH}px`,
        } as React.CSSProperties
      }
    >
      <div className="pane catalog-pane">
        {geo.catalogCollapsed ? (
          <button
            className="rail-btn"
            title="Show catalog"
            onClick={() => setGeo((g) => ({ ...g, catalogCollapsed: false }))}
          >
            ▸
          </button>
        ) : (
          <Catalog
            onRun={run}
            refreshKey={refreshKey}
            onState={({ open, optimizer }) => {
              setOpenDb(open);
              setOptimizer(optimizer);
            }}
            onCollapse={() => setGeo((g) => ({ ...g, catalogCollapsed: true }))}
          />
        )}
      </div>

      {/* Catalog | rest splitter (no resizing while collapsed) */}
      <div
        className="split split-a"
        onMouseDown={(e) =>
          geo.catalogCollapsed
            ? undefined
            : startDrag(e, geo.catalogW, (start, dx) =>
                setGeo((g) => ({ ...g, catalogW: clamp(start + dx, 140, 460) }))
              )
        }
      />

      <div className="pane console-pane">
        <Console
          history={history}
          busy={busy}
          openDb={openDb}
          optimizer={optimizer}
          collapsed={geo.consoleCollapsed}
          theme={theme}
          onToggleTheme={() => setTheme((t) => (t === "dark" ? "light" : "dark"))}
          onClearHistory={() => setHistory([])}
          onToggleCollapse={() =>
            setGeo((g) => ({ ...g, consoleCollapsed: !g.consoleCollapsed }))
          }
          onSubmit={run}
        />
      </div>

      {/* Map | console splitter. A collapsed console is input-height only, so
          there is nothing to resize then. */}
      <div
        className="split split-b"
        onMouseDown={(e) =>
          geo.consoleCollapsed
            ? undefined
            : startDrag(e, geo.consoleH, (start, _dx, dy) =>
                // The floor leaves room for a fully grown query box: this row
                // is a fixed height, so a taller box would be clipped by it.
                setGeo((g) => ({ ...g, consoleH: clamp(start - dy, 200, 640) }))
              )
        }
      />

      <div className="pane map-pane">
        {busy && (
          <div className="loading" role="status">
            <span className="spinner" />
            running query…
          </div>
        )}
        <div className="projection-ctl">
          <select
            value={projection}
            onChange={(e) => setProjection(e.target.value as Projection)}
            title="Coordinate projection for the map"
          >
            {(Object.keys(PROJECTION_LABEL) as Projection[]).map((p) => (
              <option key={p} value={p}>
                {PROJECTION_LABEL[p]}
              </option>
            ))}
          </select>
        </div>

        <MapView
          layers={visible}
          globalT0={domain ? domain[0] : 0}
          currentTime={time}
          projection={projection}
          onSelect={(layerId, object) => {
            const props =
              object &&
              (object as { properties?: Record<string, unknown> }).properties;
            if (layerId && props) {
              setSelected({
                layerId,
                properties: props as Record<string, unknown>,
              });
            } else {
              setSelected(null);
            }
          }}
        />

        <LayersPanel
          layers={layers}
          onToggle={toggle}
          onRemove={remove}
          onMove={move}
          onRename={rename}
          onStyle={setStyle}
          onClear={clear}
        />

        {selected && (
          <div className="details">
            <div className="details-head">
              {/* Resolved live rather than snapshotted, so a rename while the
                  panel is open is reflected here too. */}
              <span>
                {layers.find((l) => l.id === selected.layerId)?.name ??
                  selected.layerId}
              </span>
              <button onClick={() => setSelected(null)} title="Close">
                ✕
              </button>
            </div>
            <table>
              <tbody>
                {Object.entries(selected.properties)
                  .filter(([k]) => !k.startsWith("_"))
                  .map(([k, v]) => (
                    <tr key={k}>
                      <td className="dk">{k}</td>
                      <td className="dv">{String(v)}</td>
                    </tr>
                  ))}
              </tbody>
            </table>
          </div>
        )}

        {domain && (
          <PlotPanel
            entries={plotEntries}
            t0={domain[0]}
            time={time}
            duration={duration}
            collapsed={plotsCollapsed}
            onToggle={() => setPlotsCollapsed((c) => !c)}
          />
        )}

        {domain && (
          <Timeline
            time={time}
            duration={duration}
            t0={domain[0]}
            playing={playing}
            speed={speed}
            onPlay={setPlaying}
            onSeek={setTime}
            onSpeed={setSpeed}
          />
        )}
      </div>
    </div>
  );
}
