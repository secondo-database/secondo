import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import {
  listOperators,
  loadTable,
  runQuery,
  uploadGpx,
  type CatalogObject,
  type OperatorInfo,
} from "./api/client";
import { Console, type Entry, type RunIntent } from "./console/Console";
import { Catalog } from "./catalog/Catalog";
import { GpxImportDialog, type StepOutcome } from "./catalog/GpxImportDialog";
import { MapView } from "./map/MapView";
import { MAP_TAB, ResultTabs } from "./table/ResultTabs";
import { RowCard } from "./table/RowCard";
import { attrOf, attrOfRow, featurePropertiesOfRow, rowOf } from "./layers/rows";
import { TableView } from "./table/TableView";
import { Timeline } from "./timeline/Timeline";
import { useAnimator } from "./timeline/useAnimator";
import { isDrawable, useLayers } from "./layers/useLayers";
import { LayersPanel } from "./layers/LayersPanel";
import { PlotPanel, type PlotEntry } from "./plots/PlotPanel";
import { type Projection } from "./map/projection";
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
// Below either of these the stylesheet drops the desktop grid for the
// one-column phone layout; keep them in step with the media query in
// styles.css. The height counts too: a phone in landscape is wide enough for
// the desktop grid and nowhere near tall enough.
const NARROW_W = 760;
const NARROW_H = 480;

function loadGeometry(): Geometry {
  // A phone opens on the map: both panels start out of the way, since the
  // catalog is a full-screen sheet there and the console a docked prompt. Only
  // the first visit is decided here -- a stored geometry still wins, and the
  // media query caps it either way.
  const initial =
    window.innerWidth <= NARROW_W || window.innerHeight <= NARROW_H
      ? { ...DEFAULT_GEO, catalogCollapsed: true, consoleCollapsed: true }
      : DEFAULT_GEO;
  try {
    const raw = localStorage.getItem(GEO_KEY);
    return raw ? { ...DEFAULT_GEO, ...JSON.parse(raw) } : initial;
  } catch {
    return initial;
  }
}

const clamp = (v: number, lo: number, hi: number) => Math.min(hi, Math.max(lo, v));

// Drag a splitter: capture the size at pointerdown and apply deltas until the
// pointer is released. Pointer events rather than mouse events, so a tablet --
// which is above the narrow breakpoint and therefore keeps the desktop grid --
// can resize by finger as well.
function startDrag(
  e: React.PointerEvent,
  start: number,
  apply: (start: number, dx: number, dy: number) => void
) {
  e.preventDefault();
  const sx = e.clientX;
  const sy = e.clientY;
  const move = (ev: PointerEvent) => apply(start, ev.clientX - sx, ev.clientY - sy);
  const up = () => {
    window.removeEventListener("pointermove", move);
    window.removeEventListener("pointerup", up);
    window.removeEventListener("pointercancel", up);
    document.body.classList.remove("dragging");
  };
  window.addEventListener("pointermove", move);
  window.addEventListener("pointerup", up);
  window.addEventListener("pointercancel", up);
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
  // The GPX import in progress: the dropped file, where the bridge put it (null
  // until the upload finishes) and why it could not be stored.
  const [gpxImport, setGpxImport] = useState<{
    file: File;
    path: string | null;
    error?: string;
  } | null>(null);
  // Whether a file is being dragged anywhere over the window, so the catalog's
  // drop zone can announce itself while the drag is still in the air.
  const [dragArmed, setDragArmed] = useState(false);
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

  // Watch for a file being dragged over the window, for two reasons: the drop
  // zone lights up so a drag already in the air can find it, and a *missed*
  // drop is swallowed. Without the latter the browser's default takes over and
  // navigates the tab to the dropped file, losing the session and every layer.
  // Only file drags count -- the splitter drags are mouse events, but a text
  // selection dragged within the page would otherwise arm this too.
  useEffect(() => {
    const hasFiles = (e: DragEvent) =>
      Array.from(e.dataTransfer?.types ?? []).includes("Files");
    let depth = 0;
    const onEnter = (e: DragEvent) => {
      if (!hasFiles(e)) return;
      depth++;
      setDragArmed(true);
    };
    const onOver = (e: DragEvent) => {
      if (hasFiles(e)) e.preventDefault();
    };
    const onLeave = (e: DragEvent) => {
      if (!hasFiles(e)) return;
      if (--depth <= 0) {
        depth = 0;
        setDragArmed(false);
      }
    };
    const onDrop = (e: DragEvent) => {
      if (!hasFiles(e)) return;
      e.preventDefault(); // the catalog's own handler has already had its turn
      depth = 0;
      setDragArmed(false);
    };
    window.addEventListener("dragenter", onEnter);
    window.addEventListener("dragover", onOver);
    window.addEventListener("dragleave", onLeave);
    window.addEventListener("drop", onDrop);
    return () => {
      window.removeEventListener("dragenter", onEnter);
      window.removeEventListener("dragover", onOver);
      window.removeEventListener("dragleave", onLeave);
      window.removeEventListener("drop", onDrop);
    };
  }, []);

  // Release the SECONDO connection when the tab goes away. Each session holds a
  // server-side process, so waiting for the idle timeout would pile them up.
  // `pagehide` fires for closes, navigations and reloads (unlike `unload` it is
  // bfcache-safe), and sendBeacon still delivers during teardown.
  useEffect(() => {
    const release = () => navigator.sendBeacon?.("/api/close");
    window.addEventListener("pagehide", release);
    return () => window.removeEventListener("pagehide", release);
  }, []);

  // Every command reaches the log twice: once when it is sent, so a query that
  // takes a minute is on screen while it runs rather than after it, and once
  // when the answer (or the error) is here. `id` is what ties the two together
  // -- the entry's position is not enough, since a GPX import runs its steps
  // one after another and each appends.
  const nextEntryId = useRef(1);

  const begin = useCallback((command: string): number => {
    const id = nextEntryId.current++;
    setBusy(true);
    setHistory((h) => [
      ...h,
      { id, command, pending: true, startedAt: performance.now() },
    ]);
    return id;
  }, []);

  const settle = useCallback(
    (id: number, command: string, fields: Partial<Entry>) => {
      setHistory((h) => {
        const started = h.find((e) => e.id === id)?.startedAt;
        const done: Entry = {
          ...fields,
          id,
          command,
          pending: false,
          // The pending entry is gone: the log was cleared while this ran. The
          // answer still belongs in it, so it goes on the end -- but the
          // stopwatch it would have been measured against went with the entry,
          // so it goes up without a time rather than with a made-up one.
          elapsedMs: started === undefined ? undefined : performance.now() - started,
        };
        return started === undefined
          ? [...h, done]
          : h.map((e) => (e.id === id ? done : e));
      });
    },
    []
  );

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
  // Which row the open table should scroll to, and which geometry the map
  // should fit. Both carry a nonce rather than a bare row number: asking twice
  // for the same row has to act twice, or the second press of "show row in
  // table" looks like it did nothing.
  const [tableFocus, setTableFocus] = useState<{
    layerId: string;
    row: number;
    nonce: number;
  } | null>(null);
  const [mapFocus, setMapFocus] = useState<{
    layerId: string;
    row: number | null;
    attr: string | null;
    nonce: number;
  } | null>(null);

  // map -> table. The grid resolves the row, because it is what owns the page
  // and the sort: the ordinal addresses a scan position, so it only means a row
  // while the relation is in scan order.
  const showRow = useCallback(
    (layerId: string, row: number) => {
      showResult(layerId, "table");
      setTableFocus((f) => ({ layerId, row, nonce: (f?.nonce ?? 0) + 1 }));
    },
    [showResult]
  );

  // table -> map. Selecting a row selects its geometry; `properties` comes from
  // whichever feature that tuple produced, so the card can still say something
  // about a row the grid holds and the map does not.
  const selectRow = useCallback(
    (layerId: string, row: number) => {
      const layer = layers.find((l) => l.id === layerId);
      if (!layer) return;
      const props = featurePropertiesOfRow(layer, row);
      setSelected({
        layerId,
        properties: props ?? {},
        row,
        attr: attrOf(props),
        from: "table",
      });
    },
    [layers, setSelected]
  );

  // The one path that moves the view. Everything else leaves it where the user
  // put it -- selecting on the map above all, which is the `isMouseSelected`
  // guard around HoeseViewer.makeSelectionVisible.
  const locateRow = useCallback(
    (layerId: string, row: number) => {
      const layer = layers.find((l) => l.id === layerId);
      if (!layer) return;
      selectRow(layerId, row);
      setActiveTab(MAP_TAB);
      setMapFocus((f) => ({
        layerId,
        row,
        attr: attrOfRow(layer, row),
        nonce: (f?.nonce ?? 0) + 1,
      }));
    },
    [layers, selectRow]
  );

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
      // Logged here, answered below. The elapsed time is wall clock from this
      // point, so it includes the bridge and the conversion -- which is what
      // the user actually waited for. The console labels it as elapsed rather
      // than as the server's own query time.
      const id = begin(sent);
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
        settle(id, sent, {
          result: res.text,
          scalar: res.scalar ?? undefined,
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
        });
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
        settle(id, sent, {
          error: e instanceof Error ? e.message : String(e),
        });
        return false;
      } finally {
        setBusy(false);
      }
    },
    [add, begin, settle, showResult]
  );

  // One command of a GPX import. A sibling of `run` rather than a use of it:
  // the answer to `let x = <a track> consume` is the whole created object, and
  // the import wants neither a layer nor a table out of it -- only whether it
  // worked. The console still gets the command, so the import is readable
  // afterwards and every step can be run again by hand.
  const runStep = useCallback(
    async (command: string): Promise<StepOutcome> => {
      const id = begin(command);
      try {
        await runQuery(command, "none");
        settle(id, command, { result: "" });
        return { ok: true };
      } catch (e) {
        const error = e instanceof Error ? e.message : String(e);
        settle(id, command, { error });
        return { ok: false, error };
      } finally {
        setBusy(false);
      }
    },
    [begin, settle]
  );

  // Take a dropped GPX file: put it where the SECONDO server can read it, then
  // let the dialog drive the import. The dialog opens straight away, before the
  // upload finishes, so a multi-megabyte track does not look like a dead click.
  const startGpxImport = useCallback((file: File) => {
    setGpxImport({ file, path: null });
    void uploadGpx(file)
      .then(({ path }) => setGpxImport((s) => (s?.file === file ? { file, path } : s)))
      .catch((e) =>
        setGpxImport((s) =>
          s?.file === file
            ? { file, path: null, error: e instanceof Error ? e.message : String(e) }
            : s
        )
      );
  }, []);

  // Open a stored relation straight in the table, with its tuple identifiers --
  // the catalog's path into editing, as the Java GUI's relation chooser is.
  const openRelation = useCallback(
    async (name: string): Promise<boolean> => {
      // The command the bridge will build is only known once it answers, so
      // the pending line says what was asked for; `settle` writes the real one
      // over it. Loading a big relation is exactly the wait this is about.
      const asked = `table ${name}`;
      const id = begin(asked);
      try {
        const res = await loadTable(name);
        const layerId = add(res.command, null, null, res.table, name);
        settle(id, res.command, {
          result: "",
          layerId,
          rowCount: res.table.rowCount,
        });
        showResult(layerId, "table");
        return true;
      } catch (e) {
        settle(id, asked, {
          error: e instanceof Error ? e.message : String(e),
        });
        return false;
      } finally {
        setBusy(false);
      }
    },
    [add, begin, settle, showResult]
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

  // The layer the selection belongs to, resolved live rather than snapshotted
  // into the Selection: it can be renamed, restyled or reloaded with more rows
  // while the card is open, and the card must follow.
  const selectedLayer = selected
    ? layers.find((l) => l.id === selected.layerId) ?? null
    : null;
  // How far the card's ‹ › may walk. With a stored relation behind the result
  // the card fetches whatever row it is given, so the steppers walk the whole
  // thing -- the grid's page has stopped being the limit. A derived result has
  // nothing to fetch from, so there they are held to the rows that were
  // actually sent.
  const stepBounds = useMemo<[number, number] | null>(() => {
    const t = selectedLayer?.table;
    if (!t || t.rowCount === 0) return null;
    const source = t.relation ?? selectedLayer?.relation ?? null;
    if (source && t.totalKnown) return [0, t.totalRows - 1];
    return [t.offset, t.offset + t.rowCount - 1];
  }, [selectedLayer]);

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
            onImport={startGpxImport}
            dragArmed={dragArmed}
          />
        </div>
      </div>

      {/* Catalog | rest splitter (no resizing while collapsed) */}
      <div
        className="split split-a"
        onPointerDown={(e) =>
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
        onPointerDown={(e) =>
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
            above the timeline when there is one, at the pane's edge when not.
            How tall the timeline actually is belongs to the stylesheet -- it
            wraps to two rows on a phone -- so only the choice is made here. */}
        <div
          className="result-body"
          style={
            {
              "--bottom-inset": domain ? "var(--timeline-h)" : "0.75rem",
            } as React.CSSProperties
          }
        >
        {busy && (
          <div className="loading" role="status">
            <span className="spinner" />
            running query…
          </div>
        )}
        <MapView
          layers={visible}
          globalT0={domain ? domain[0] : 0}
          currentTime={time}
          projection={projection}
          onProjectionChange={setProjection}
          theme={theme}
          selection={selected}
          focus={mapFocus}
          onSelect={(layerId, object) => {
            const props =
              object &&
              (object as { properties?: Record<string, unknown> }).properties;
            if (layerId && props) {
              const p = props as Record<string, unknown>;
              setSelected({
                layerId,
                properties: p,
                // Stamped by the backend on every feature built from a
                // relation; absent for an individual object, which has no row.
                row: rowOf(p),
                attr: attrOf(p),
                from: "map",
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
                {/* Not "on the left": on a phone the catalog is a sheet behind
                    the button over the map, not a column beside it. */}
                <p>Pick one in the catalog to see what it holds.</p>
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
        {selectedLayer && selected && (
          <RowCard
            layer={selectedLayer}
            selection={selected}
            onClose={() => setSelected(null)}
            onShowRow={
              // Not offered when the row cannot be reached: an ad-hoc result is
              // capped rather than paged, so a row past the cap has no page to
              // turn to and the jump could only switch tabs to say so. The card
              // says it instead, where the user is already looking.
              selected.row !== null &&
              selectedLayer.table &&
              (selectedLayer.table.pageable ||
                selected.row <
                  selectedLayer.table.offset + selectedLayer.table.rowCount)
                ? () => showRow(selected.layerId, selected.row!)
                : null
            }
            onStep={(d) => selectRow(selected.layerId, selected.row! + d)}
            stepBounds={stepBounds}
          />
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
            focus={
              tableFocus && tableFocus.layerId === activeLayer.id
                ? tableFocus
                : null
            }
            selectedRow={
              selected && selected.layerId === activeLayer.id ? selected.row : null
            }
            onSelectRow={(row) => selectRow(activeLayer.id, row)}
            onLocateRow={
              isDrawable(activeLayer)
                ? (row) => locateRow(activeLayer.id, row)
                : null
            }
          />
        )}
        </div>
      </div>

      {/* Keyed by the file so a second drop starts a clean dialog rather than
          reusing the last one's name and step states. */}
      {gpxImport && openDb && (
        <GpxImportDialog
          key={gpxImport.file.name + gpxImport.file.lastModified}
          file={gpxImport.file}
          path={gpxImport.path}
          uploadError={gpxImport.error}
          database={openDb}
          existingNames={objects.map((o) => o.name)}
          runStep={runStep}
          onClose={() => {
            setGpxImport(null);
            // Whatever the import managed to create is in the database now,
            // including after a failure part-way through.
            setRefreshKey((k) => k + 1);
          }}
        />
      )}
    </div>
  );
}
