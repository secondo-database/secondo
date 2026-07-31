import { useCallback, useEffect, useMemo, useState } from "react";
import {
  listOperators,
  loadTable,
  runQuery,
  type CatalogObject,
  type OperatorInfo,
} from "./api/client";
import { Console, type Entry, type RunIntent } from "./console/Console";
import { Catalog } from "./catalog/Catalog";
import { MapView } from "./map/MapView";
import { MAP_TAB, ResultTabs } from "./table/ResultTabs";
import { TableView } from "./table/TableView";
import { Timeline } from "./timeline/Timeline";
import { useAnimator } from "./timeline/useAnimator";
import { isDrawable, useLayers } from "./layers/useLayers";
import { LayersPanel } from "./layers/LayersPanel";
import { PlotPanel, type PlotEntry } from "./plots/PlotPanel";
import { PROJECTION_LABEL, type Projection } from "./map/projection";
import { applyTheme, loadTheme, type Theme } from "./theme";

// Commands after which the catalog on the left is stale: they switch the
// database (open/close/create/delete/restore database), or they add, remove,
// rename or overwrite an object inside it. Anything else -- `query`, `list`,
// `save`, `set` -- leaves the object list exactly as it was, so it does not
// pay for two round trips.
const CATALOG_CMD =
  /^\s*(open|close|create|delete|restore|let_?|letnt|derive|update|kill|changename|drop)\b/i;

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
    setTable,
    clear,
    selected,
    setSelected,
  } = useLayers();

  // Which results have a table open, and which tab the pane is showing. Neither
  // is persisted: layers do not survive a reload, so a tab pointing at one
  // cannot either.
  const [openTables, setOpenTables] = useState<string[]>([]);
  const [activeTab, setActiveTab] = useState<string>(MAP_TAB);

  const [history, setHistory] = useState<Entry[]>([]);
  const [busy, setBusy] = useState(false);
  // Authoritative database state comes from the catalog (see Catalog.onState).
  const [openDb, setOpenDb] = useState<string | null>(null);
  // Whether the connected server runs SQL; null until the catalog has asked.
  const [optimizer, setOptimizer] = useState<boolean | null>(null);
  // The open database's objects, reported by the catalog (see Catalog.onState).
  // The console completes on these names, and the empty state suggests queries
  // built from them -- neither fetches its own copy.
  const [objects, setObjects] = useState<CatalogObject[]>([]);
  // What the *server* can do, as opposed to what the open database holds. Fetched
  // once: `list operators` reads the algebra catalog, which no command changes,
  // so unlike `objects` this never needs refreshing.
  const [operators, setOperators] = useState<OperatorInfo[]>([]);
  const [refreshKey, setRefreshKey] = useState(0);
  const [projection, setProjection] = useState<Projection>("none");
  const [geo, setGeo] = useState<Geometry>(loadGeometry);
  const [theme, setTheme] = useState<Theme>(loadTheme);

  useEffect(() => {
    applyTheme(theme);
  }, [theme]);

  // Best-effort: without it the editor falls back to its built-in shortlist, so
  // a failure costs completeness, not completion.
  useEffect(() => {
    listOperators()
      .then((r) => setOperators(r.operators))
      .catch(() => undefined);
  }, []);

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

  // Show a past result. Opening a table tab is what makes it visible; the map
  // is always there.
  const showResult = useCallback((layerId: string, target: "map" | "table") => {
    if (target === "map") {
      setActiveTab(MAP_TAB);
      return;
    }
    setOpenTables((t) => (t.includes(layerId) ? t : [...t, layerId]));
    setActiveTab(layerId);
  }, []);

  // Closing a tab only puts the table away -- the result stays a layer and its
  // console entry opens it again.
  const closeTable = useCallback((layerId: string) => {
    setOpenTables((t) => t.filter((id) => id !== layerId));
    setActiveTab((a) => (a === layerId ? MAP_TAB : a));
  }, []);

  // Single command runner shared by the console and the catalog. Returns
  // whether the command succeeded so callers can refresh.
  const run = useCallback(
    async (command: string, intent?: RunIntent): Promise<boolean> => {
      // "Explain" is the only intent that changes what is sent. The prefix is
      // the kernel's own (stripOptimizerPrefix); the frontend just writes it.
      const sent = intent === "explain" ? `optimizer ${command}` : command;
      setBusy(true);
      // Wall clock around the round trip, so it includes the bridge and the
      // conversion -- which is what the user actually waited for. The console
      // labels it as elapsed rather than as the server's own query time.
      const started = performance.now();
      try {
        // "Run as table" is answered by the server: it converts the rows only,
        // so a relation of moving points does not build and ship a trips payload
        // that would be thrown away here.
        const res = await runQuery(sent, intent === "table" ? "table" : undefined);
        const fc = res.geojson ?? null;
        const temp = res.temporal ?? null;
        const tab = res.table ?? null;
        const layerId =
          fc || temp || tab
            ? add(command, fc, temp, tab, res.relation ?? null)
            : undefined;
        setHistory((h) => [
          ...h,
          {
            command: sent,
            result: res.text,
            hasGeometry: !!fc,
            hasMotion: !!temp,
            layerId,
            rowCount: tab?.rowCount,
            plan: res.plan ?? undefined,
            costs: res.costs ?? undefined,
            message: res.message ?? undefined,
            planOnly: res.plan_only,
            executedByOptimizer: res.executed_by_optimizer,
            // Nothing opened and nothing was drawn; say why, or the menu item
            // looks like it did nothing at all.
            noTable: intent === "table" && !tab,
            elapsedMs: performance.now() - started,
          },
        ]);
        // A plain run opens a table only when the map cannot show the result at
        // all, so a mappable result never steals focus from wherever the user
        // is: where it goes is decided *after* it arrives, from the console
        // entry or the layers row. "Run as table" asked for one up front, and
        // with no geometry converted there is nothing else it could mean.
        if (layerId && tab && (intent === "table" || (!fc && !temp)))
          showResult(layerId, "table");
        // The catalog refreshes on this and reports the new database state back
        // -- which also keeps the console's completions in step with it. SQL
        // that the optimizer executed itself (`create table x`, `insert into`)
        // arrives already flagged, so it does not have to match the pattern.
        if (CATALOG_CMD.test(command) || res.executed_by_optimizer)
          setRefreshKey((k) => k + 1);
        return true;
      } catch (e) {
        setHistory((h) => [
          ...h,
          {
            command: sent,
            error: e instanceof Error ? e.message : String(e),
            elapsedMs: performance.now() - started,
          },
        ]);
        return false;
      } finally {
        setBusy(false);
      }
    },
    [add, showResult]
  );

  // Open a stored relation straight in the table, with its tuple identifiers --
  // the catalog's path into editing, as the Java GUI's relation chooser is.
  const openRelation = useCallback(
    async (name: string): Promise<boolean> => {
      setBusy(true);
      try {
        const res = await loadTable(name);
        const layerId = add(res.command, null, null, res.table, name);
        setHistory((h) => [
          ...h,
          { command: res.command, result: "", layerId, rowCount: res.table.rowCount },
        ]);
        showResult(layerId, "table");
        return true;
      } catch (e) {
        setHistory((h) => [
          ...h,
          {
            command: `table ${name}`,
            error: e instanceof Error ? e.message : String(e),
          },
        ]);
        return false;
      } finally {
        setBusy(false);
      }
    },
    [add, showResult]
  );

  const visible = useMemo(() => layers.filter((l) => l.visible), [layers]);

  // What the map has to show. A table-only result is still a layer -- it holds
  // the rows and the tab that opens them -- but it draws nothing, so counting
  // layers would call the map "not empty" while it is blank. `query ten`, or
  // anything run through "Show result as table", left it with no explanation of
  // itself at all once its tab was closed.
  const drawn = useMemo(() => layers.filter(isDrawable), [layers]);

  // Three things worth looking at in *this* database, for the empty map: the
  // objects that draw something come first (spatial, then moving), since the
  // suggestion is only useful if clicking it puts something on screen.
  const suggestions = useMemo(() => {
    const rank = { spatial: 0, temporal: 1, other: 2 } as const;
    return [...objects]
      .filter((o) => o.kind !== "other")
      .sort((a, b) => rank[a.kind] - rank[b.kind])
      .slice(0, 3)
      .map((o) => `query ${o.name}`);
  }, [objects]);

  // A removed result takes its tab with it, and the pane falls back to the map.
  useEffect(() => {
    const ids = new Set(layers.map((l) => l.id));
    setOpenTables((t) =>
      t.every((id) => ids.has(id)) ? t : t.filter((id) => ids.has(id))
    );
  }, [layers]);
  useEffect(() => {
    if (activeTab !== MAP_TAB && !openTables.includes(activeTab))
      setActiveTab(MAP_TAB);
  }, [openTables, activeTab]);

  const tableTabs = useMemo(
    () =>
      openTables
        .map((id) => layers.find((l) => l.id === id))
        .filter((l): l is NonNullable<typeof l> => !!l)
        .map((l) => ({ id: l.id, name: l.name, command: l.command })),
    [openTables, layers]
  );
  const activeLayer =
    activeTab === MAP_TAB ? null : layers.find((l) => l.id === activeTab) ?? null;

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
        {geo.catalogCollapsed && (
          <button
            className="rail-btn"
            title="Show catalog"
            aria-label="Show the catalog"
            onClick={() => setGeo((g) => ({ ...g, catalogCollapsed: false }))}
          >
            ▸
          </button>
        )}
        {/* Hidden while collapsed rather than unmounted: it is what reports the
            open database and its objects, and an unmounted catalog stops
            reporting -- the header and the console's completions would then go
            stale the moment the rail is used. */}
        <div className="cat-wrap" hidden={geo.catalogCollapsed}>
          <Catalog
            onRun={run}
            onOpenTable={openRelation}
            refreshKey={refreshKey}
            onState={({ open, optimizer, objects }) => {
              setOpenDb(open);
              setOptimizer(optimizer);
              setObjects(objects);
            }}
            onCollapse={() => setGeo((g) => ({ ...g, catalogCollapsed: true }))}
          />
        </div>
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
          objects={objects}
          operators={operators}
          onToggleTheme={() => setTheme((t) => (t === "dark" ? "light" : "dark"))}
          onClearHistory={() => setHistory([])}
          onToggleCollapse={() =>
            setGeo((g) => ({ ...g, consoleCollapsed: !g.consoleCollapsed }))
          }
          onSubmit={run}
          onShowResult={showResult}
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
                setGeo((g) => ({
                  ...g,
                  // The ceiling is measured against the window, not a constant:
                  // on a short screen a fixed 640 would leave the map with no
                  // height at all -- and the controls that give it back are in
                  // the map pane.
                  consoleH: clamp(
                    start - dy,
                    200,
                    Math.max(200, Math.min(640, window.innerHeight - 240))
                  ),
                }))
              )
        }
      />

      <div className="pane map-pane">
        <ResultTabs
          tabs={tableTabs}
          active={activeTab}
          onSelect={setActiveTab}
          onClose={closeTable}
        />

        {/* The map stays mounted behind an open table: it owns its view state
            and its WebGL context, so covering it is what makes switching back
            free. `--bottom-inset` is where the bottom-anchored overlays stop:
            above the timeline when there is one, at the pane's edge when not. */}
        <div
          className="result-body"
          style={
            { "--bottom-inset": domain ? "4.5rem" : "0.75rem" } as React.CSSProperties
          }
        >
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
          theme={theme}
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

        {/* The empty map used to say only "Run a spatial or moving-object
            query", which tells a first-time user neither where to start nor
            what a query looks like here. The examples are built from the open
            database's own objects, so they always work. */}
        {drawn.length === 0 && (
          <div className="map-empty">
            {/* On its own surface rather than as bare text: with a projection
                chosen the map draws light OSM raster tiles underneath, and text
                sized for the dark empty pane disappears into them. */}
            <div className="map-empty-card">
            {!openDb ? (
              <>
                <strong>No database open</strong>
                <p>Pick one in the catalog on the left to see what it holds.</p>
              </>
            ) : (
              <>
                <strong>Nothing on the map yet</strong>
                <p>
                  Click an object in the catalog, or run a query below
                  {suggestions.length > 0 ? " — for example:" : "."}
                </p>
                {suggestions.length > 0 && (
                  <ul className="me-examples">
                    {suggestions.map((cmd) => (
                      <li key={cmd}>
                        <button onClick={() => void run(cmd)} disabled={busy}>
                          {cmd}
                        </button>
                      </li>
                    ))}
                  </ul>
                )}
              </>
            )}
            </div>
          </div>
        )}

        <LayersPanel
          layers={layers}
          onToggle={toggle}
          onRemove={remove}
          onMove={move}
          onRename={rename}
          onStyle={setStyle}
          onClear={clear}
          onShowTable={(id) => showResult(id, "table")}
        />

        {/* One bottom-left column, so the plots can never cover the attributes
            of the feature that was just clicked. */}
        <div className="ov-left">
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
        </div>

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

        {activeLayer?.table && (
          // Keyed by result: switching tabs must not carry one table's edit
          // mode, pending changes, sort or filter over to another.
          <TableView
            key={activeLayer.id}
            name={activeLayer.name}
            table={activeLayer.table}
            relation={activeLayer.relation}
            onTable={(t) => setTable(activeLayer.id, t)}
          />
        )}
        </div>
      </div>
    </div>
  );
}
