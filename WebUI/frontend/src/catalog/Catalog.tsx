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
  // Bumped by the parent whenever a command that changes catalog state runs
  // (open/close/create/delete database, or object creation) so we refresh.
  refreshKey: number;
  // The catalog owns the authoritative database state (it is what queries the
  // backend); it reports it up so the rest of the UI doesn't fetch it twice.
  onState?: (state: {
    open: string | null;
    databases: string[];
    optimizer: boolean;
  }) => void;
  // Collapse the whole catalog to a rail, giving the space to the map.
  onCollapse?: () => void;
}

const KIND_ICON: Record<CatalogObject["kind"], string> = {
  spatial: "◆",
  temporal: "◷",
  other: "·",
};

export function Catalog({ onRun, refreshKey, onState, onCollapse }: Props) {
  const [databases, setDatabases] = useState<string[]>([]);
  const [open, setOpen] = useState<string | null>(null);
  const [objects, setObjects] = useState<CatalogObject[]>([]);
  const [filter, setFilter] = useState("");
  const [loadingObjects, setLoadingObjects] = useState(false);
  const [pendingDb, setPendingDb] = useState<string | null>(null);
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
      onStateRef.current?.({
        open: dbs.open,
        databases: dbs.databases,
        optimizer: dbs.optimizer,
      });
      if (dbs.open) {
        setLoadingObjects(true);
        const objs = await listObjects();
        if (seq !== seqRef.current) return;
        setObjects(objs.objects);
      } else {
        setObjects([]);
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

  async function openDb(name: string) {
    if (pendingDb) return; // ignore rapid repeat clicks while one is in flight
    setPendingDb(name);
    const lname = name.toLowerCase();
    try {
      if (open && open !== lname) await onRun("close database");
      if (open !== lname) await onRun(`open database ${name}`);
      await refresh();
    } finally {
      setPendingDb(null);
    }
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
        {databases.map((db) => (
          <button
            key={db}
            className={
              "cat-db" +
              (open === db.toLowerCase() ? " active" : "") +
              (pendingDb === db ? " pending" : "")
            }
            disabled={pendingDb !== null}
            onClick={() => void openDb(db)}
            title={`open database ${db}`}
          >
            {db}
            {pendingDb === db && <span className="cat-spin" />}
          </button>
        ))}
      </div>

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
              </li>
            ))}
          </ul>
        </>
      )}
    </div>
  );
}
