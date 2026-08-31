import { useEffect, useState } from "react";
import type { TablePayload } from "../api/client";
import { loadTable } from "../api/client";
import type { Layer, Selection } from "../layers/useLayers";
import { formatCell } from "./useTableEdit";

/**
 * The selected object, as its whole tuple.
 *
 * This replaces the old details panel, which listed the clicked feature's
 * GeoJSON `properties` -- and those hold only the values `geojson._scalar` kept,
 * so every region, mpoint and other non-atomic attribute was silently missing.
 * The tuple is read from the result's own table payload instead, by the same
 * ordinal the map feature carries, so what the card shows and what the grid
 * shows are the same row formatted the same way.
 *
 * The nearest thing in the Java GUI is the stretch of `QueryResult`'s JList
 * around the selected line: one `"attrname : value"` row per attribute, with
 * the clicked geometry's own row selected among them
 * (`TextWindow.ensureSelectedIndexIsVisible` centres it so the siblings show).
 * Here they are a card rather than a slice of a longer list, so the tuple has
 * an actual boundary, but the content is the same and `◆` marks the same line.
 */

/** How much of a value is worth showing inline. The grid clips at the server's
 *  MAX_CELL_CHARS (4000); a 320px card wants far less, and the whole value is
 *  one `title` away.
 *
 *  Nested-list values get the tighter budget. A `Trip` written out as text has
 *  no prefix worth reading -- it says "this attribute is an mpoint, and here is
 *  where it starts" and nothing more -- so letting it run to 160 characters
 *  cost five lines of a card whose other three attributes fit on one each. */
const MAX_INLINE = 160;
const MAX_INLINE_MONO = 90;

interface Props {
  layer: Layer;
  selection: Selection;
  onClose: () => void;
  /** Open the result's table and scroll to this row. Absent when the result
   *  has no rows to show -- an individual object, not a relation. */
  onShowRow: (() => void) | null;
  /** Move the selection to the previous/next tuple. Null for a result with no
   *  rows -- an individual object has no neighbours to step to. */
  onStep: ((delta: number) => void) | null;
  /** Inclusive row bounds the stepper may move within: the page the grid holds,
   *  since a row off it has no values to show. Null disables stepping. */
  stepBounds: [number, number] | null;
}

interface Line {
  name: string;
  /** Already formatted for display. */
  value: string;
  /** Right-aligned like the grid's numeric columns. */
  numeric: boolean;
  /** Monospaced like the grid's non-atomic cells: raw nested-list syntax. */
  mono: boolean;
  /** True for the geometry that was actually clicked. */
  clicked: boolean;
  /** The value is absent (SECONDO undefined), drawn as the grid's `∅`. */
  empty: boolean;
  /** How many characters of `value` to show before clipping. */
  cap: number;
  /** The attribute exists but this row's value is not loaded. `value` then
   *  holds the column's *type*, so the line still says what the attribute is. */
  unknown: boolean;
}

const NUMERIC = new Set(["int", "real", "longint", "tid"]);

/** The tuple, preferring the table payload -- it has every attribute, where the
 *  feature's properties have only the scalar ones.
 *
 *  When the row is not on the page the grid holds, the *schema* still is: the
 *  attribute list comes from the columns either way, and only the values that
 *  happen to be missing are drawn as their type. The card therefore has the
 *  same shape whichever page the grid is on -- and, in particular, never drops
 *  the clicked geometry, which is the one attribute whose absence is impossible
 *  to make sense of while looking at it on the map.
 *
 *  Only a result with no table at all -- an individual object, `query
 *  BGrenzenLine` -- falls back to the feature's own properties. */
function lines(
  layer: Layer,
  sel: Selection,
  fetched: TablePayload | null
): { rows: Line[]; full: boolean } {
  const t = layer.table;
  // Whichever payload holds this tuple: the grid's page, or the single row
  // fetched for it below. Both are the same shape, so the formatting is one
  // branch rather than two.
  const held =
    t && sel.row !== null && sel.row - t.offset >= 0 && sel.row - t.offset < t.rows.length
      ? t
      : fetched && sel.row !== null && sel.row === fetched.offset && fetched.rows.length > 0
        ? fetched
        : null;
  if (held && sel.row !== null) {
    const row = held.rows[sel.row - held.offset];
    return {
      full: true,
      rows: held.columns.map((c, n) => ({
        name: c.name,
        value: formatCell(row[n]),
        numeric: NUMERIC.has(c.type),
        mono: !c.atomic,
        clicked: c.name === sel.attr,
        empty: row[n] === null,
        cap: c.atomic ? MAX_INLINE : MAX_INLINE_MONO,
        unknown: false,
      })),
    };
  }
  if (t) {
    return {
      full: false,
      rows: t.columns.map((c) => {
        // The map carries a tuple's *scalar* attributes beside its geometry
        // (geojson._scalar), so this fills in some of the row and not the rest.
        const v = sel.properties[c.name];
        const known = v !== undefined;
        return {
          name: c.name,
          value: known ? String(v) : c.type,
          numeric: known && NUMERIC.has(c.type),
          mono: false,
          clicked: c.name === sel.attr,
          empty: v === null,
          cap: MAX_INLINE,
          unknown: !known,
        };
      }),
    };
  }
  return {
    full: false,
    rows: Object.entries(sel.properties)
      .filter(([k]) => !k.startsWith("_")) // hide internal _attr/_row/_layer
      .map(([k, v]) => ({
        name: k,
        value: String(v),
        numeric: typeof v === "number",
        mono: false,
        clicked: k === sel.attr,
        empty: v === null || v === undefined,
        cap: MAX_INLINE,
        unknown: false,
      })),
  };
}

export function RowCard({
  layer,
  selection,
  onClose,
  onShowRow,
  onStep,
  stepBounds,
}: Props) {
  // The tuple, when the grid's page does not hold it. Fetching one row is the
  // same request the pager makes, with limit 1 -- and it leaves `layer.table`
  // alone, so the grid stays on whatever page the user put it on. It costs a
  // scan of `row` tuples server-side (SECONDO has no `skip`; see the paging
  // notes in the README), which is the price the pager already pays to reach
  // that page, and it is only ever one row per selection.
  const [fetched, setFetched] = useState<TablePayload | null>(null);
  const [loading, setLoading] = useState(false);
  const source = layer.table?.relation ?? layer.relation;
  const row = selection.row;
  // Whether the grid's own page already has it -- then there is nothing to ask
  // for, which is the common case and must not cost a round trip.
  const onPage =
    !!layer.table &&
    row !== null &&
    row >= layer.table.offset &&
    row < layer.table.offset + layer.table.rows.length;

  useEffect(() => {
    if (onPage || row === null || !source) {
      setFetched(null);
      return;
    }
    let cancelled = false;
    setLoading(true);
    // `wantTotal` is deliberately absent: the count is a full scan, and the
    // header already has a total from the query that built the layer.
    //
    // `tids` follows the grid rather than the endpoint's default, which is
    // `true` -- otherwise this row would grow a TID column the grid is not
    // showing, and the card would list an attribute that vanishes again the
    // moment the grid's own page holds the row.
    loadTable(source, {
      offset: row,
      limit: 1,
      tids: (layer.table?.tidIndex ?? null) !== null,
    })
      .then((r) => {
        if (!cancelled) setFetched(r.table);
      })
      .catch(() => {
        // Best effort. The card falls back to naming the types, and says so.
        if (!cancelled) setFetched(null);
      })
      .finally(() => {
        if (!cancelled) setLoading(false);
      });
    return () => {
      cancelled = true;
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [source, row, onPage, selection.layerId, layer.table?.tidIndex]);

  const { rows, full } = lines(layer, selection, fetched);
  const t = layer.table;
  // Why the card is falling back to the map's properties, when it is.
  //
  //  - "paged": the tuple is in the relation, just not on the page the grid
  //    holds. Asking for it pages there, so the jump is worth offering.
  //  - "capped": an ad-hoc result is capped rather than paged -- the backend
  //    did not write that query, so there is nothing to ask for the next
  //    thousand of. The tuple is genuinely unreachable, and the jump is *not*
  //    offered: it could only switch tabs to say so.
  // Why values are still missing, when they are. "paged" is now only ever the
  // brief moment before the fetch lands, or a fetch that failed; "capped" is
  // the case that cannot be fixed by asking -- a derived result has no relation
  // behind it, so the rows past the server's cap were never sent and there is
  // nothing to request them from.
  const missing: null | "paged" | "capped" =
    full || selection.row === null || !t ? null : source ? "paged" : "capped";
  // What the message points at. Naming the one attribute beats "the grey
  // values" when there is only one -- and with a relation whose only unmapped
  // attribute is its geometry, which is the common case, there always is.
  const unloaded = rows.filter((l) => l.unknown);
  const which =
    unloaded.length === 1 ? unloaded[0].name : `the ${unloaded.length} grey values`;
  // Stepping needs a row to step from and a page to step within.
  const stepper = onStep !== null && stepBounds !== null && selection.row !== null;
  // "row 13 of 58" -- 1-based, like the grid's pager. `totalKnown` is false only
  // while paging without a fresh count, and then the total is a floor.
  const where =
    selection.row !== null && t
      ? `row ${(selection.row + 1).toLocaleString()} of ${
          t.totalKnown ? t.totalRows.toLocaleString() : `${t.totalRows}+`
        }`
      : null;

  return (
    // `data-row` rather than the visible text: the e2e suite reads state off
    // data attributes, as it does for .mapview[data-projection].
    <div className="details" data-row={selection.row ?? ""} data-attr={selection.attr ?? ""}>
      <div className="details-head">
        {/* Resolved live rather than snapshotted, so a rename while the panel
            is open is reflected here too. */}
        <span className="details-name" title={layer.command}>
          {layer.name}
        </span>
        {where && <span className="details-where">{where}</span>}
        <button onClick={onClose} title="Close" aria-label="Close the selection">
          ✕
        </button>
      </div>

      <table>
        <tbody>
          {rows.map((l) => (
            <tr key={l.name} className={l.clicked ? "dr-clicked" : undefined}>
              <td className="dk">
                {/* The attribute whose geometry was clicked. With one spatial
                    attribute this is merely reassuring; with two -- a From and
                    a To point -- it is the only thing saying which one. */}
                {l.clicked && <span className="dr-mark" title="The object you clicked">◆</span>}
                {l.name}
              </td>
              <td
                className={
                  ["dv", l.numeric ? "num" : "", l.mono ? "mono" : ""]
                    .filter(Boolean)
                    .join(" ")
                }
                title={l.value.length > l.cap ? l.value : undefined}
              >
                {l.unknown ? (
                  // Not "missing": the attribute is there, this row's value just
                  // is not loaded. Naming the type says what would be here.
                  <span className="dr-unloaded" title="Not loaded — open the table to read this value">
                    {l.value}
                  </span>
                ) : l.empty ? (
                  <span className="tv-null">∅</span>
                ) : l.value.length > l.cap ? (
                  l.value.slice(0, l.cap) + "…"
                ) : (
                  l.value
                )}
              </td>
            </tr>
          ))}
        </tbody>
      </table>

      {/* Only the attributes the map carries: the rest of the tuple is in the
          table, and this says so rather than looking complete. */}
      {missing === "paged" && unloaded.length > 0 && (
        <div className="details-partial">
          {loading
            ? `Reading ${which} for this row…`
            : `Could not read ${which} for this row.`}
        </div>
      )}
      {missing === "capped" && unloaded.length > 0 && (
        <div className="details-partial">
          This tuple is past the {t!.rowCount.toLocaleString()}-row cap the
          server put on this result, so {which}{" "}
          {unloaded.length === 1 ? "was" : "were"} never sent — narrow the query
          to read {unloaded.length === 1 ? "it" : "them"}.
        </div>
      )}

      {(onShowRow || stepper) && (
        <div className="details-foot">
          {stepper && (
            <>
              <button
                className="dock-btn"
                onClick={() => onStep!(-1)}
                disabled={selection.row! <= stepBounds![0]}
                title="Select the previous tuple"
                aria-label="Previous tuple"
              >
                ‹
              </button>
              <button
                className="dock-btn"
                onClick={() => onStep!(1)}
                disabled={selection.row! >= stepBounds![1]}
                title="Select the next tuple"
                aria-label="Next tuple"
              >
                ›
              </button>
            </>
          )}
          {onShowRow && (
            <button
              className="dock-btn details-show"
              onClick={onShowRow}
              title="Open this result as a table, with this row in view"
            >
              <span className="dock-ic">▤</span>
              <span className="dock-lbl">show row in table</span>
            </button>
          )}
        </div>
      )}
    </div>
  );
}
