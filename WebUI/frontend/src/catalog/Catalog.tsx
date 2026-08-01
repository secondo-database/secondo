import { useCallback, useEffect, useRef, useState } from "react";
import {
  listDatabases,
  listObjects,
  type CatalogObject,
} from "../api/client";

// A left-hand browser: pick a database to open, then click an object to query
// it. Mirrors the HoeseViewer's object list; the console still works alongside.
interface Props {
  // Run a SECONDO command through the same session as the console, and report
  // whether it succeeded so the catalog can refresh.
  onRun: (command: string) => Promise<boolean>;
  // Open a stored relation directly in the table view, loaded with its tuple
  // identifiers so it can be edited. The catalog is the discoverable way in,
  // as the relation chooser is in the Java GUI's UpdateViewer2.
  onOpenTable: (relation: string) => Promise<boolean>;
  // Bumped by the parent whenever a command that changes catalog state runs
  // (open/close/create/delete database, or object creation) so we refresh.
  refreshKey: number;
  // The catalog owns the authoritative database state (it is what queries the
  // backend); it reports it up so the rest of the UI doesn't fetch it twice.
  // `objects` travels with it for the same reason: the console completes on
  // these names and the empty state suggests queries from them, and neither
  // should ask the server for a list this panel already has.
  onState?: (state: {
    open: string | null;
    databases: string[];
    optimizer: boolean;
    objects: CatalogObject[];
  }) => void;
  // Collapse the whole catalog to a rail, giving the space to the map.
  onCollapse?: () => void;
  // A GPX file was dropped (or picked) here: upload it and open the import
  // dialog. Only ever called with a database open.
  onImport: (file: File) => void;
  // A file is being dragged somewhere over the window. The drop zone is down
  // among the objects, so it says so while there is something to drop --
  // otherwise the target for a drag already in progress is easy to miss.
  dragArmed?: boolean;
}

const KIND_ICON: Record<CatalogObject["kind"], string> = {
  spatial: "◆",
  temporal: "◷",
  other: "·",
};

export function Catalog({
  onRun,
  onOpenTable,
  refreshKey,
  onState,
  onCollapse,
  onImport,
  dragArmed,
}: Props) {
  const [databases, setDatabases] = useState<string[]>([]);
  const [open, setOpen] = useState<string | null>(null);
  const [objects, setObjects] = useState<CatalogObject[]>([]);
  const [filter, setFilter] = useState("");
  const [loadingObjects, setLoadingObjects] = useState(false);
  const [pendingDb, setPendingDb] = useState<string | null>(null);
  // The file is over the drop zone itself, as opposed to somewhere else.
  const [over, setOver] = useState(false);
  // Why the last file was refused, shown in the zone rather than as an alert.
  const [dropError, setDropError] = useState<string | null>(null);
  // dragenter/dragleave fire for every child element the pointer crosses, so
  // the highlight is kept on a depth count rather than on the last event.
  const dragDepth = useRef(0);
  const fileInput = useRef<HTMLInputElement>(null);
  // Monotonic id so a slower, stale refresh can't overwrite a newer one.
  const seqRef = useRef(0);
  // Held in a ref so `refresh` stays stable regardless of the callback identity.
  const onStateRef = useRef(onState);
  onStateRef.current = onState;

  const refresh = useCallback(async () => {
    const seq = ++seqRef.current;
    try {
      const dbs = await listDatabases();
      if (seq !== seqRef.current) return;
      setDatabases(dbs.databases);
      setOpen(dbs.open);
      const report = (objects: CatalogObject[]) =>
        onStateRef.current?.({
          open: dbs.open,
          databases: dbs.databases,
          optimizer: dbs.optimizer,
          objects,
        });
      if (dbs.open) {
        setLoadingObjects(true);
        // Report the database straight away so the header is right while the
        // objects are still loading, then again once they are in.
        report([]);
        const objs = await listObjects();
        if (seq !== seqRef.current) return;
        setObjects(objs.objects);
        report(objs.objects);
      } else {
        setObjects([]);
        report([]);
      }
    } catch {
      /* backend not reachable yet */
    } finally {
      if (seq === seqRef.current) setLoadingObjects(false);
    }
  }, []);

  useEffect(() => {
    void refresh();
  }, [refresh, refreshKey]);

  // `list databases` reports the names uppercased while `session.open_db` keeps
  // them as the open command spelled them, so the two are only ever the same
  // database, never the same string. One helper for the comparison, used by both
  // the highlight and the guard below, so they cannot drift apart again.
  const isOpen = (name: string) => open?.toLowerCase() === name.toLowerCase();

  async function openDb(name: string) {
    if (pendingDb) return; // ignore rapid repeat clicks while one is in flight
    setPendingDb(name);
    try {
      if (open && !isOpen(name)) await onRun("close database");
      if (!isOpen(name)) await onRun(`open database ${name}`);
      await refresh();
    } finally {
      setPendingDb(null);
    }
  }

  // One way in for both gestures: the drop and the file chooser behind the
  // click, which is the keyboard path and what the e2e test drives.
  function accept(file: File | undefined | null) {
    if (!file) return;
    if (!file.name.toLowerCase().endsWith(".gpx")) {
      setDropError(`${file.name} is not a .gpx file.`);
      return;
    }
    setDropError(null);
    onImport(file);
  }

  const shown = objects.filter((o) =>
    o.name.toLowerCase().includes(filter.toLowerCase())
  );
  const busy = loadingObjects || pendingDb !== null;
  const shownDb = open ?? pendingDb;

  return (
    <div className="catalog">
      <div className="cat-section">
        databases
        {onCollapse && (
          <button className="cat-collapse" onClick={onCollapse} title="Hide catalog">
            ◂
          </button>
        )}
      </div>
      <div className="cat-dbs">
        {databases.length === 0 && (
          <p className="cat-hint">
            No databases on the server, or it is not reachable yet.
          </p>
        )}
        {databases.map((db) => (
          <button
            key={db}
            className={
              "cat-db" +
              (isOpen(db) ? " active" : "") +
              (pendingDb === db ? " pending" : "")
            }
            disabled={pendingDb !== null}
            // The chips are a single-select group; without this the selected one
            // is only a colour, which is nothing to a screen reader.
            aria-pressed={isOpen(db)}
            onClick={() => void openDb(db)}
            title={isOpen(db) ? `${db} is open` : `open database ${db}`}
          >
            {db}
            {pendingDb === db && <span className="cat-spin" />}
          </button>
        ))}
      </div>

      {!shownDb && databases.length > 0 && (
        <p className="cat-hint">Click a database to open it.</p>
      )}

      {shownDb && (
        <>
          <div className="cat-section">
            objects in {shownDb}
            {busy ? (
              <span className="cat-spin" />
            ) : (
              <span className="cat-count">{objects.length}</span>
            )}
          </div>
          {open && (
            <input
              className="cat-filter"
              placeholder="filter objects…"
              value={filter}
              spellCheck={false}
              onChange={(e) => setFilter(e.target.value)}
            />
          )}
          <ul className="cat-objs">
            {busy && objects.length === 0 && (
              <li className="cat-loading">
                <span className="cat-spin" /> loading objects…
              </li>
            )}
            {!busy && objects.length > 0 && shown.length === 0 && (
              <li className="cat-hint">No object matches “{filter}”.</li>
            )}
            {shown.map((o) => (
              <li key={o.name}>
                <button
                  className={"cat-obj kind-" + o.kind}
                  onClick={() => void onRun(`query ${o.name}`)}
                  title={`query ${o.name}   (${o.type})`}
                >
                  <span className="cat-ic">{KIND_ICON[o.kind]}</span>
                  <span className="cat-oname">{o.name}</span>
                  <span className="cat-otype">{o.type}</span>
                </button>
                {/* Only a stored relation can be opened as an editable table. */}
                {o.relation && (
                  <button
                    className="cat-table"
                    onClick={() => void onOpenTable(o.name)}
                    title={`Open ${o.name} as an editable table`}
                  >
                    ▤
                  </button>
                )}
              </li>
            ))}
          </ul>
        </>
      )}

      {/* Importing a GPX file is a drag away, and the zone says so even when
          nothing is being dragged -- a drop target nothing announces is a
          feature nobody finds. Without a database open there is nowhere to
          import *to*, so it says that instead of silently refusing. */}
      <div
        className={
          "cat-drop" +
          (open ? "" : " disabled") +
          (over ? " over" : "") +
          (dragArmed && open ? " armed" : "")
        }
        onDragEnter={(e) => {
          if (!open) return;
          e.preventDefault();
          dragDepth.current++;
          setOver(true);
        }}
        onDragOver={(e) => {
          if (!open) return;
          e.preventDefault();
          e.dataTransfer.dropEffect = "copy";
        }}
        onDragLeave={() => {
          if (--dragDepth.current <= 0) {
            dragDepth.current = 0;
            setOver(false);
          }
        }}
        onDrop={(e) => {
          e.preventDefault();
          dragDepth.current = 0;
          setOver(false);
          if (!open) return;
          accept(e.dataTransfer.files?.[0]);
        }}
      >
        {open ? (
          <>
            <button
              className="cat-drop-btn"
              onClick={() => fileInput.current?.click()}
              title={`Import a GPX track into ${open}`}
            >
              <span className="cat-drop-ic">⬆</span>
              <span className="cat-drop-text">Drop a GPX file here</span>
              <span className="cat-drop-sub">or click to choose one</span>
            </button>
            <input
              ref={fileInput}
              className="cat-drop-input"
              type="file"
              accept=".gpx"
              onChange={(e) => {
                accept(e.target.files?.[0]);
                // Let the same file be picked twice in a row.
                e.target.value = "";
              }}
            />
            {dropError && <p className="cat-drop-err err-text">{dropError}</p>}
          </>
        ) : (
          <p className="cat-drop-sub">Open a database to import a GPX file.</p>
        )}
      </div>
    </div>
  );
}
