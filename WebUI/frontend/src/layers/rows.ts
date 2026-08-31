import type { Layer, Selection } from "./useLayers";

/**
 * The tuple ordinal <-> feature link.
 *
 * The backend stamps every feature it builds from a relation with `_row`, the
 * tuple's position in the answer's scan order (`geojson.RelationFeatures.feed`,
 * `temporal.RelationMoving.feed`). The table payload is built from the same
 * tuple stream in the same order, so that ordinal is also the row's scan
 * position -- and the row on screen is `_row - table.offset`, because
 * `/api/table/load` pages by scan position too (`addcounter … filter … head`).
 *
 * This is the web equivalent of the Hoese viewer's index arithmetic
 * (`QueryResult.getPick`: `row = tupleIndex * (attrCount + 1) + attrIndex`),
 * minus the fragility -- there the ordinal was implied by the position in a
 * flat JList and broke as soon as a Dspl class emitted a second line.
 */

type Props = Record<string, unknown> | undefined | null;

/** A feature's tuple ordinal, or null when the result is not a relation. */
export function rowOf(props: Props): number | null {
  const v = props?._row;
  return typeof v === "number" ? v : null;
}

/** Which geometry attribute a feature came from, or null. */
export function attrOf(props: Props): string | null {
  const v = props?._attr;
  return typeof v === "string" ? v : null;
}

/**
 * Whether a feature is the selected one. Matched on the tuple ordinal and the
 * attribute rather than on an array index, because the three things a click can
 * land on -- a static feature, a moving region's face, a trip's current
 * position -- live in three different arrays and only share these two keys.
 *
 * A result that is not a relation (`query BGrenzenLine`) carries neither, and
 * has exactly one object: selecting it selects that object.
 */
export function isSelectedFeature(props: Props, sel: Selection | null): boolean {
  if (!sel) return false;
  if (sel.row === null) return rowOf(props) === null;
  return rowOf(props) === sel.row && attrOf(props) === sel.attr;
}

/**
 * The properties of any feature of `row`, for a selection made in the grid --
 * the card falls back to these when the row is off the loaded page, and they
 * are what the old viewer would have had in hand from the list entry itself.
 * `attr` picks between the geometries of a tuple that has more than one.
 */
export function featurePropertiesOfRow(
  layer: Layer,
  row: number,
  attr?: string | null
): Record<string, unknown> | null {
  const pools: Props[] = [];
  // `FeatureCollection.features` is `unknown[]` in the client types -- the app
  // never reads geometry itself, it hands the collection straight to deck.
  for (const f of layer.geojson?.features ?? [])
    pools.push((f as { properties?: Record<string, unknown> })?.properties);
  for (const t of layer.temporal?.trips ?? []) pools.push(t.properties);
  for (const r of layer.temporal?.regions ?? []) pools.push(r.properties);
  let fallback: Record<string, unknown> | null = null;
  for (const p of pools) {
    if (rowOf(p) !== row) continue;
    if (!attr || attrOf(p) === attr) return p as Record<string, unknown>;
    fallback ??= p as Record<string, unknown>;
  }
  return fallback;
}

/** Which geometry attribute the row's first feature came from, so a selection
 *  made in the grid marks the same line the map would have. */
export function attrOfRow(layer: Layer, row: number): string | null {
  return attrOf(featurePropertiesOfRow(layer, row));
}
