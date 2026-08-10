// Which attribute of a query result can be written next to its geometry?
//
// A relation carries its non-spatial attributes along as feature properties
// (see backend `geojson._relation_features`), and for many results one of them
// is what a person would write on a map: the name. Labelling is opt-in -- a
// layer starts unlabelled and the layers panel offers the choice -- so this
// module only ranks the candidates, putting the most label-like one first.

import type { FeatureCollection, TemporalPayload } from "../api/client";

type Props = Record<string, unknown>;

/**
 * The symbolic trajectories (mlabel/mstring) a layer carries, by attribute
 * name, in relation-schema order and without the duplicates that several rows
 * of the same relation produce.
 *
 * These are not label *candidates* in the sense the rest of this module means:
 * there is nothing to rank and nothing to choose between, because they are all
 * drawn. The layers panel offers a checkbox each so a noisy one can be dropped
 * -- an OSM road name is empty for most of a footpath -- and this is the list
 * it enumerates.
 */
export function symbolicAttributes(temporal: TemporalPayload | null): string[] {
  if (!temporal || temporal.trips.length === 0) return [];
  const seen = new Set<string>();
  const out: string[] = [];
  for (const s of temporal.labels ?? []) {
    if (seen.has(s.attr)) continue;
    seen.add(s.attr);
    out.push(s.attr);
  }
  return out;
}

// A layer's attributes, wherever its geometry came from: static features and
// moving objects both carry the tuple's non-spatial attributes along, so both
// can be labelled the same way.
function propertiesOf(
  geojson: FeatureCollection | null,
  temporal: TemporalPayload | null
): Props[] {
  const props: Props[] = [];
  if (geojson) {
    for (const f of geojson.features as { properties?: Props }[]) {
      props.push(f.properties ?? {});
    }
  }
  if (temporal) {
    for (const t of temporal.trips) props.push(t.properties ?? {});
    for (const r of temporal.regions ?? []) props.push(r.properties ?? {});
  }
  return props;
}

// The optimizer renames the attributes of a join after the alias the query used
// -- `select [r:name, r:strasse]` comes back as `Name_r`, `Strasse_r` -- so the
// suffix is stripped before looking for a meaningful word in the name.
function baseName(key: string): string {
  return key.replace(/_[A-Za-z0-9]{1,3}$/, "");
}

/** How good a label this attribute would make. Higher is better. */
function score(key: string, values: unknown[]): number {
  const strings = values.filter((v) => typeof v === "string") as string[];
  const scalars = values.filter(
    (v) => typeof v === "string" || typeof v === "number"
  );
  if (scalars.length === 0) return -Infinity;

  const base = baseName(key);
  let s = 0;
  if (/name/i.test(base)) s += 100;
  else if (/(bez|label|title|descr|text|art)/i.test(base)) s += 60;
  // A name is text; an id or a coordinate is not what you write on a map.
  if (strings.length === scalars.length) s += 20;

  // Values that repeat label nothing, and long ones smear across the map.
  const distinct = new Set(scalars.map(String)).size;
  s += (distinct / scalars.length) * 15;
  const avgLen =
    scalars.reduce<number>((n, v) => n + String(v).length, 0) / scalars.length;
  if (avgLen > 24) s -= 20;

  return s;
}

/** The attributes that can be shown as a label, most label-like first. */
export function labelCandidates(
  geojson: FeatureCollection | null,
  temporal: TemporalPayload | null = null
): string[] {
  const props = propertiesOf(geojson, temporal);
  if (props.length === 0) return [];
  const keys = new Set<string>();
  for (const p of props) {
    // `_attr` and friends are how the backend marks which attribute a feature's
    // geometry came from; they are plumbing, not data.
    for (const k of Object.keys(p)) if (!k.startsWith("_")) keys.add(k);
  }
  return [...keys]
    .map((k) => ({ k, s: score(k, props.map((p) => p[k])) }))
    .filter(({ s }) => s > -Infinity)
    .sort((a, b) => b.s - a.s)
    .map(({ k }) => k);
}
