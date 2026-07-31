import { useCallback, useMemo, useState } from "react";
import type { TableColumn, TablePayload } from "../api/client";
import { commitTable, loadTable } from "../api/client";
import { formatCell, isEditable, pendingOr, useTableEdit } from "./useTableEdit";

interface Props {
  /** The result's name, as shown on its tab. */
  name: string;
  table: TablePayload;
  /** The stored relation this result came from, if the server could name one.
   *  Editing needs it; without it the table is read-only. */
  relation: string | null;
  /** Replace this result's rows -- after loading with TIDs, or after a commit. */
  onTable: (table: TablePayload) => void;
}

type Sort = { column: number; dir: 1 | -1 } | null;

const NUMERIC = new Set(["int", "real", "longint", "tid"]);

/** Page sizes on offer. Bounded by the backend's MAX_ROWS (app/table.py), which
 *  clamps anything larger. */
const PAGE_SIZES = [100, 200, 500, 1000];

/** Cells that are shown but never edited: the tuple identifier is the tuple's
 *  handle, not one of its attributes (RelationTableModel.isCellEditable). */
const isTid = (c: TableColumn) => c.type === "tid";

function compare(a: unknown, b: unknown): number {
  if (a === null || a === undefined) return b === null || b === undefined ? 0 : 1;
  if (b === null || b === undefined) return -1;
  if (typeof a === "number" && typeof b === "number") return a - b;
  return String(a).localeCompare(String(b), undefined, { numeric: true });
}

export function TableView({ name, table, relation, onTable }: Props) {
  const [filter, setFilter] = useState("");
  const [sort, setSort] = useState<Sort>(null);
  const [editing, setEditing] = useState(false);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const edit = useTableEdit();

  const tidIndex = table.tidIndex;
  const canEdit = !!relation;
  // A table loaded through /api/table/load holds one page of a relation; the
  // server can be asked for any other. An ad-hoc query result cannot -- the
  // backend did not write that query, so it has no way to say "the next 200".
  const paged = table.pageable;
  const pageSize = table.limit ?? PAGE_SIZES[1];
  const source = table.relation ?? relation;

  // Row order is a view concern only: every pending change is keyed by TID, so
  // sorting and filtering never move an edit onto a different tuple. On a paged
  // table sorting is the *server's* job -- sorting the 200 rows that happen to
  // be on screen would order a page rather than the relation.
  const view = useMemo(() => {
    let idx = table.rows.map((_, i) => i);
    if (filter.trim()) {
      const needle = filter.trim().toLowerCase();
      idx = idx.filter((i) =>
        table.rows[i].some((v) => formatCell(v).toLowerCase().includes(needle))
      );
    }
    if (sort && !paged) {
      const { column, dir } = sort;
      idx = [...idx].sort(
        (a, b) => dir * compare(table.rows[a][column], table.rows[b][column])
      );
    }
    return idx;
  }, [table.rows, filter, sort, paged]);

  /** Re-read the relation. `wantTotal` costs a counting scan, so it is asked for
   *  only when the count can have changed -- opening the table, and saving.
   *  Otherwise the total already on screen is carried over: paging and sorting
   *  cannot change how many tuples there are. */
  const reload = useCallback(
    async (next: {
      offset?: number;
      limit?: number;
      sort?: Sort;
      wantTotal?: boolean;
      tids?: boolean;
    }): Promise<boolean> => {
      if (!source) return false;
      const order = next.sort === undefined ? sort : next.sort;
      const wantTotal = next.wantTotal ?? false;
      setBusy(true);
      setError(null);
      try {
        const res = await loadTable(source, {
          offset: Math.max(0, next.offset ?? table.offset),
          limit: next.limit ?? pageSize,
          sort: order
            ? [`${table.columns[order.column].name} ${order.dir === 1 ? "asc" : "desc"}`]
            : [],
          wantTotal,
          // Keep the column once it is there -- paging must not take editing
          // away mid-edit -- but do not add it to a table that is only read.
          tids: next.tids ?? tidIndex !== null,
        });
        let loaded = res.table;
        if (!wantTotal && table.totalKnown) {
          loaded = { ...loaded, totalRows: table.totalRows, totalKnown: true };
        }
        // Everything was deleted out from under this page. Rather than show an
        // empty grid with rows behind it, fall back to the first page.
        if (loaded.rowCount === 0 && loaded.offset > 0) {
          const first = await loadTable(source, { limit: pageSize, wantTotal: true });
          loaded = first.table;
        }
        onTable(loaded);
        return true;
      } catch (e) {
        setError(e instanceof Error ? e.message : String(e));
        return false;
      } finally {
        setBusy(false);
      }
    },
    [source, sort, pageSize, table, tidIndex, onTable]
  );

  const toggleSort = (column: number) => {
    const next: Sort =
      sort && sort.column === column
        ? sort.dir === 1
          ? { column, dir: -1 }
          : null
        : { column, dir: 1 };
    setSort(next);
    // Sorting a relation reorders every page, so it starts again at the first.
    if (paged) void reload({ offset: 0, sort: next });
  };

  // Entering edit mode reloads the relation through `addid`, because only that
  // query carries the TIDs every update is addressed by. An already-paged table
  // stays on the page it is showing.
  const startEditing = useCallback(async () => {
    if (!relation) return;
    const ok = await reload({
      offset: paged ? table.offset : 0,
      wantTotal: true,
      tids: true,
    });
    if (!ok) return;
    edit.reset();
    setEditing(true);
  }, [relation, paged, table.offset, reload, edit]);

  const discard = useCallback(() => {
    edit.reset();
    setEditing(false);
    setError(null);
  }, [edit]);

  const save = useCallback(async () => {
    if (!table.relation) return;
    setBusy(true);
    setError(null);
    try {
      await commitTable(edit.toEdits(table.relation));
    } catch (e) {
      setError(e instanceof Error ? e.message : String(e));
      setBusy(false);
      return;
    }
    setBusy(false);
    // Re-read rather than patch: the server assigned TIDs to the new tuples
    // and may have rejected nothing but changed everything else. Inserts and
    // deletes moved the row count, so this is one of the two loads that pays
    // for a fresh count.
    if (await reload({ wantTotal: true })) edit.reset();
  }, [table.relation, edit, reload]);

  const editable = editing && isEditable(table) && tidIndex !== null;

  // Where this page sits in the relation. `totalKnown` is false only while
  // stepping pages without a fresh count, which cannot happen before one page
  // has been counted -- so in practice it holds.
  const from = table.offset + 1;
  const to = table.offset + table.rowCount;
  const atStart = table.offset === 0;
  const atEnd = table.totalKnown
    ? to >= table.totalRows
    : table.rowCount < pageSize;
  // A relation that fits on one page needs no pager.
  const showPager = paged && !(atStart && atEnd);

  return (
    <div className="tableview">
      <header className="tv-head">
        <strong className="tv-name" title={name}>
          {name}
        </strong>
        <span className="tv-count">
          {paged
            ? `${table.totalKnown ? table.totalRows.toLocaleString() : `${to}+`} rows`
            : table.truncated
              ? `${table.rowCount} of ${table.totalRows} rows`
              : `${table.rowCount} ${table.rowCount === 1 ? "row" : "rows"}`}
        </span>
        {table.truncated && (
          <span
            className="tv-warn"
            // A stored relation is paged instead of capped, so this is only
            // ever a derived result -- there is no relation to ask for page two
            // of, and narrowing the query is the way to see the rest.
            title="The server capped this derived result; refine the query to see the rest"
          >
            truncated
          </span>
        )}
        <input
          className="tv-filter"
          type="text"
          // Client-side, so where there is more than one page it narrows the
          // rows on screen and not the relation. Said plainly rather than left
          // to be guessed -- but only when it makes a difference: on a relation
          // that fits on one page, "this page" would be a distinction without one.
          placeholder={showPager ? "filter this page…" : "filter rows…"}
          title={
            showPager
              ? "Narrows the rows on this page. Sort a column to reorder the whole relation."
              : "Narrows the rows shown"
          }
          value={filter}
          spellCheck={false}
          onChange={(e) => setFilter(e.target.value)}
        />
        {editable ? (
          <>
            <button
              className="dock-btn"
              onClick={() => edit.addRow(table.columns)}
              disabled={busy}
              title="Append a new tuple"
            >
              + row
            </button>
            <button
              className="dock-btn tv-save"
              onClick={() => void save()}
              disabled={busy || !edit.dirty}
              title="Write every pending change to the relation"
            >
              {busy ? "…" : "✓ save"}
            </button>
            <button className="dock-btn" onClick={discard} disabled={busy}>
              ✕ discard
            </button>
          </>
        ) : (
          <button
            className="dock-btn"
            onClick={() => void startEditing()}
            disabled={!canEdit || busy}
            title={
              canEdit
                ? `Edit ${relation} — reloads it with tuple identifiers`
                : "Only a stored relation can be edited; this result is derived"
            }
          >
            {busy ? "…" : "✎ edit"}
          </button>
        )}
      </header>

      {editable && edit.dirty && (
        <div className="tv-pending">
          {[
            edit.counts.updated && `${edit.counts.updated} changed`,
            edit.counts.deleted && `${edit.counts.deleted} to delete`,
            edit.counts.inserted && `${edit.counts.inserted} to add`,
          ]
            .filter(Boolean)
            .join(" · ")}
          {paged
            ? " across all pages — nothing is written until you save."
            : " — nothing is written until you save."}
        </div>
      )}
      {error && <div className="tv-error">{error}</div>}

      <div className="tv-scroll">
        <table className="tv-grid">
          <thead>
            <tr>
              {editable && <th className="tv-gutter" />}
              {table.columns.map((c, i) => (
                <th
                  key={c.name}
                  onClick={() => toggleSort(i)}
                  title={`${c.name}: ${c.type}`}
                  className={NUMERIC.has(c.type) ? "num" : undefined}
                >
                  {c.name}
                  <span className="tv-type">{c.type}</span>
                  {sort?.column === i && (
                    <span className="tv-sort">{sort.dir === 1 ? "▲" : "▼"}</span>
                  )}
                </th>
              ))}
            </tr>
          </thead>
          <tbody>
            {view.map((r) => {
              const row = table.rows[r];
              const tid = tidIndex !== null ? Number(row[tidIndex]) : -1;
              const deleted = editable && edit.isDeleted(tid);
              return (
                <tr key={tid >= 0 ? tid : r} className={deleted ? "tv-deleted" : undefined}>
                  {editable && (
                    <td className="tv-gutter">
                      <button
                        className="tv-rowbtn"
                        onClick={() => edit.toggleDelete(tid)}
                        title={deleted ? "Keep this tuple" : "Delete this tuple"}
                      >
                        {deleted ? "↩" : "✕"}
                      </button>
                    </td>
                  )}
                  {table.columns.map((c, i) => (
                    <Cell
                      key={c.name}
                      column={c}
                      value={row[i]}
                      editable={editable && !isTid(c) && !deleted}
                      changed={editable && edit.isChanged(tid, c.name)}
                      current={
                        editable ? pendingOr(edit.updates, tid, c.name, row[i]) : ""
                      }
                      onChange={(v) =>
                        v === formatCell(row[i])
                          ? edit.clearCell(tid, c.name)
                          : edit.setCell(tid, c.name, v)
                      }
                    />
                  ))}
                </tr>
              );
            })}

            {editable &&
              edit.inserts.map((newRow) => (
                <tr key={newRow.key} className="tv-new">
                  <td className="tv-gutter">
                    <button
                      className="tv-rowbtn"
                      onClick={() => edit.removeInsert(newRow.key)}
                      title="Drop this new tuple"
                    >
                      ✕
                    </button>
                  </td>
                  {table.columns.map((c) => (
                    <Cell
                      key={c.name}
                      column={c}
                      value={null}
                      editable={!isTid(c)}
                      changed={false}
                      current={newRow.values[c.name] ?? ""}
                      placeholder={isTid(c) ? "assigned on save" : c.type}
                      onChange={(v) => edit.setInsertCell(newRow.key, c.name, v)}
                    />
                  ))}
                </tr>
              ))}
          </tbody>
        </table>

        {view.length === 0 && (
          <div className="tv-empty">
            {table.rowCount === 0 ? "no rows" : "no row matches the filter"}
          </div>
        )}
      </div>

      {showPager && (
        <footer className="tv-pager">
          <button
            className="dock-btn"
            onClick={() => void reload({ offset: 0 })}
            disabled={busy || atStart}
            title="First page"
          >
            ‹‹
          </button>
          <button
            className="dock-btn"
            onClick={() => void reload({ offset: table.offset - pageSize })}
            disabled={busy || atStart}
            title="Previous page"
          >
            ‹
          </button>
          <span className="tv-range">
            {`rows ${from.toLocaleString()}–${to.toLocaleString()}`}
            {table.totalKnown && ` of ${table.totalRows.toLocaleString()}`}
          </span>
          <button
            className="dock-btn"
            onClick={() => void reload({ offset: table.offset + pageSize })}
            disabled={busy || atEnd}
            title="Next page"
          >
            ›
          </button>
          <button
            className="dock-btn"
            onClick={() =>
              void reload({
                // The last whole page, so the final row is always on it.
                offset: Math.max(
                  0,
                  Math.floor((table.totalRows - 1) / pageSize) * pageSize
                ),
              })
            }
            disabled={busy || atEnd || !table.totalKnown}
            title="Last page"
          >
            ››
          </button>
          <label className="tv-pagesize">
            rows per page
            <select
              value={pageSize}
              disabled={busy}
              onChange={(e) => {
                const limit = Number(e.target.value);
                // Keep the first row of the current page in view rather than
                // jumping back to the start of the relation.
                void reload({
                  limit,
                  offset: Math.floor(table.offset / limit) * limit,
                });
              }}
            >
              {PAGE_SIZES.map((n) => (
                <option key={n} value={n}>
                  {n}
                </option>
              ))}
            </select>
          </label>
        </footer>
      )}
    </div>
  );
}

interface CellProps {
  column: TableColumn;
  value: string | number | boolean | null;
  editable: boolean;
  changed: boolean;
  current: string;
  placeholder?: string;
  onChange: (value: string) => void;
}

function Cell({
  column,
  value,
  editable,
  changed,
  current,
  placeholder,
  onChange,
}: CellProps) {
  const numeric = NUMERIC.has(column.type);
  // Anything but int/real/bool/string/text needs raw nested-list syntax, exactly
  // as AttributeFormatter requires in the Java GUI, so it is shown monospaced.
  const raw = !column.atomic;
  const cls = [
    numeric ? "num" : "",
    raw ? "mono" : "",
    changed ? "tv-changed" : "",
  ]
    .filter(Boolean)
    .join(" ");

  if (!editable) {
    const text = formatCell(value);
    return (
      <td className={cls} title={text.length > 40 ? text : undefined}>
        {value === null ? <span className="tv-null">∅</span> : text}
      </td>
    );
  }

  if (column.type === "bool") {
    return (
      <td className={cls}>
        <select value={current || "FALSE"} onChange={(e) => onChange(e.target.value)}>
          <option value="TRUE">TRUE</option>
          <option value="FALSE">FALSE</option>
        </select>
      </td>
    );
  }

  return (
    <td className={cls}>
      <input
        type="text"
        value={current}
        spellCheck={false}
        placeholder={placeholder}
        onChange={(e) => onChange(e.target.value)}
      />
    </td>
  );
}
