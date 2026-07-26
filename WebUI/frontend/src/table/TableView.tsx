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

  // Row order is a view concern only: every pending change is keyed by TID, so
  // sorting and filtering never move an edit onto a different tuple.
  const view = useMemo(() => {
    let idx = table.rows.map((_, i) => i);
    if (filter.trim()) {
      const needle = filter.trim().toLowerCase();
      idx = idx.filter((i) =>
        table.rows[i].some((v) => formatCell(v).toLowerCase().includes(needle))
      );
    }
    if (sort) {
      const { column, dir } = sort;
      idx = [...idx].sort(
        (a, b) => dir * compare(table.rows[a][column], table.rows[b][column])
      );
    }
    return idx;
  }, [table.rows, filter, sort]);

  const toggleSort = (column: number) =>
    setSort((s) =>
      s && s.column === column
        ? s.dir === 1
          ? { column, dir: -1 }
          : null
        : { column, dir: 1 }
    );

  // Entering edit mode reloads the relation through `addid`, because only that
  // query carries the TIDs every update is addressed by.
  const startEditing = useCallback(async () => {
    if (!relation) return;
    setBusy(true);
    setError(null);
    try {
      const res = await loadTable(relation);
      onTable(res.table);
      edit.reset();
      setEditing(true);
    } catch (e) {
      setError(e instanceof Error ? e.message : String(e));
    } finally {
      setBusy(false);
    }
  }, [relation, onTable, edit]);

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
      // Re-read rather than patch: the server assigned TIDs to the new tuples
      // and may have rejected nothing but changed everything else.
      const res = await loadTable(table.relation);
      onTable(res.table);
      edit.reset();
    } catch (e) {
      setError(e instanceof Error ? e.message : String(e));
    } finally {
      setBusy(false);
    }
  }, [table.relation, edit, onTable]);

  const editable = editing && isEditable(table) && tidIndex !== null;

  return (
    <div className="tableview">
      <header className="tv-head">
        <strong className="tv-name" title={name}>
          {name}
        </strong>
        <span className="tv-count">
          {table.truncated
            ? `${table.rowCount} of ${table.totalRows} rows`
            : `${table.rowCount} ${table.rowCount === 1 ? "row" : "rows"}`}
        </span>
        {table.truncated && (
          <span
            className="tv-warn"
            title="The server capped the result; refine the query to see the rest"
          >
            truncated
          </span>
        )}
        <input
          className="tv-filter"
          type="text"
          placeholder="filter rows…"
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
          {" — nothing is written until you save."}
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
