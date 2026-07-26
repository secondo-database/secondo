import { useCallback, useMemo, useState } from "react";
import type { TableColumn, TableEdits, TablePayload } from "../api/client";

// Pending edits for one table, modelled on viewer/update2/Change.java: changes
// are keyed by the tuple identifier, never by a row position. Sorting or
// filtering the grid therefore cannot desynchronise them from their tuples.

/** A row that does not exist on the server yet. */
export interface PendingRow {
  key: string;
  values: Record<string, string>;
}

/** Every cell is edited as text and converted by the server according to the
 *  column's type -- the same division of labour as the Java GUI, where
 *  AttributeFormatter owns the string <-> ListExpr rules. */
export function formatCell(v: string | number | boolean | null): string {
  if (v === null || v === undefined) return "";
  if (typeof v === "boolean") return v ? "TRUE" : "FALSE";
  return String(v);
}

export function useTableEdit() {
  const [updates, setUpdates] = useState<Record<number, Record<string, string>>>({});
  const [deletes, setDeletes] = useState<number[]>([]);
  const [inserts, setInserts] = useState<PendingRow[]>([]);

  const dirty =
    Object.keys(updates).length > 0 || deletes.length > 0 || inserts.length > 0;

  const reset = useCallback(() => {
    setUpdates({});
    setDeletes([]);
    setInserts([]);
  }, []);

  const setCell = useCallback((tid: number, column: string, value: string) => {
    setUpdates((prev) => ({ ...prev, [tid]: { ...prev[tid], [column]: value } }));
  }, []);

  /** Drop one pending change -- used when a cell is edited back to its original
   *  value, so an accidental keystroke does not leave the table dirty. */
  const clearCell = useCallback((tid: number, column: string) => {
    setUpdates((prev) => {
      const row = prev[tid];
      if (!row || !(column in row)) return prev;
      const { [column]: _dropped, ...rest } = row;
      const next = { ...prev };
      if (Object.keys(rest).length === 0) delete next[tid];
      else next[tid] = rest;
      return next;
    });
  }, []);

  const isChanged = useCallback(
    (tid: number, column: string) => updates[tid]?.[column] !== undefined,
    [updates]
  );

  const toggleDelete = useCallback((tid: number) => {
    setDeletes((prev) =>
      prev.includes(tid) ? prev.filter((t) => t !== tid) : [...prev, tid]
    );
  }, []);

  const isDeleted = useCallback((tid: number) => deletes.includes(tid), [deletes]);

  const addRow = useCallback((columns: TableColumn[]) => {
    const values: Record<string, string> = {};
    for (const c of columns) if (c.type !== "tid") values[c.name] = "";
    setInserts((prev) => [
      ...prev,
      { key: `new${Date.now()}${prev.length}`, values },
    ]);
  }, []);

  const setInsertCell = useCallback(
    (key: string, column: string, value: string) => {
      setInserts((prev) =>
        prev.map((r) =>
          r.key === key ? { ...r, values: { ...r.values, [column]: value } } : r
        )
      );
    },
    []
  );

  const removeInsert = useCallback((key: string) => {
    setInserts((prev) => prev.filter((r) => r.key !== key));
  }, []);

  const toEdits = useCallback(
    (relation: string): TableEdits => ({
      relation,
      updates: Object.entries(updates).map(([tid, values]) => ({
        tid: Number(tid),
        values,
      })),
      deletes,
      inserts: inserts.map((r) => ({ values: r.values })),
    }),
    [updates, deletes, inserts]
  );

  const counts = useMemo(
    () => ({
      updated: Object.keys(updates).length,
      deleted: deletes.length,
      inserted: inserts.length,
    }),
    [updates, deletes, inserts]
  );

  return {
    updates,
    inserts,
    dirty,
    counts,
    reset,
    setCell,
    clearCell,
    isChanged,
    toggleDelete,
    isDeleted,
    addRow,
    setInsertCell,
    removeInsert,
    toEdits,
  };
}

/** What a cell currently shows in edit mode: the pending value if there is one,
 *  otherwise what the server sent. */
export function pendingOr(
  updates: Record<number, Record<string, string>>,
  tid: number,
  column: string,
  original: string | number | boolean | null
): string {
  const p = updates[tid]?.[column];
  return p !== undefined ? p : formatCell(original);
}

/** Whether a payload can be edited at all: it must carry tuple identifiers and
 *  name the stored relation to write back to. */
export function isEditable(t: TablePayload): boolean {
  return t.tidIndex !== null && !!t.relation;
}
