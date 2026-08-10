import { useCallback, useRef, useState } from "react";
import type {
  FeatureCollection,
  TablePayload,
  TemporalPayload,
} from "../api/client";
import type { IconName } from "./icons";

export type RGB = [number, number, number];

export type TemporalMode = "trail" | "points" | "both";

export interface LayerStyle {
  color: RGB;
  opacity: number; // 0..1
  pointRadius: number; // px (also the moving-object position-dot radius)
  lineWidth: number; // px (also the moving-object trail width)
  filled: boolean;
  temporalMode: TemporalMode; // how moving objects render
  // Attribute written next to each feature. Null (no labels) unless chosen in
  // the layers panel; see ./labels for how the candidates are ranked.
  label: string | null;
  // A fixed caption for the whole layer, for results that carry no attributes
  // to label with -- an individual object such as `query Train7`. Null unless
  // typed in the layers panel; `label` (an attribute) wins when both are set.
  labelText: string | null;
  // Symbol drawn for point geometry and for moving-object current positions.
  // Null -- the default -- keeps the plain circle; a name indexes the icon
  // atlas in ./icons.
  icon: IconName | null;
  // Symbolic trajectories (mlabel/mstring) write a line each next to the moving
  // point. Which ones is stored as the ones *left out* rather than the ones
  // taken: the default is all of them, and a layer reloaded with an attribute
  // it did not have before should show it rather than silently drop it.
  symbolicHidden: string[];
  // Whether each of those lines is prefixed with its attribute name
  // ("RoadType: footway" rather than "footway"). Off by default: with one
  // trajectory the name is noise, and it is the second one that makes telling
  // them apart worth the width.
  symbolicPrefix: boolean;
}

export interface Layer {
  id: string;
  name: string;
  command: string;
  geojson: FeatureCollection | null;
  temporal: TemporalPayload | null;
  // Rows, for a result that is a relation. A layer can carry only this (a
  // relation of scalars draws nothing), only geometry, or both.
  table: TablePayload | null;
  // The stored relation the rows can be written back to, if the server could
  // name one. Null for a derived result -- it is read-only.
  relation: string | null;
  visible: boolean;
  style: LayerStyle;
}

// The layer's name is deliberately *not* copied in here: it can be renamed
// while the details panel is open, and a snapshot would go stale. Callers
// resolve the name from `layers` by id at render time.
export interface Selection {
  layerId: string;
  properties: Record<string, unknown>;
}

// Categorical palette, assigned in fixed order (never cycled by rank) so a
// layer keeps its colour as others come and go.
//
// These are the *bright* steps of a validated categorical theme. The previous
// set used the dark-surface steps, which are mid-dark by design -- on the
// near-black map canvas a thin 1px line in #008300 or #c98500 was genuinely
// hard to see. Each hue is now taken at its brighter step, and the order is
// the theme's own (blue, orange, aqua, yellow, magenta, green, violet, red)
// rather than the ad-hoc one it had, which put red next to magenta: that pair
// measured ΔE 7.8 to normal vision, i.e. two layers a full-colour reader could
// not tell apart.
//
// Validated with the data-viz palette checker against *both* surfaces the map
// has -- the near-black canvas (#0b0d11) and the light OSM basemap (#f2efe9):
// chroma floor, CVD separation (worst adjacent ΔE 9.1, protan) and the
// normal-vision floor (19.6) pass on both, and contrast passes 3:1 on the dark
// canvas. Two deliberate trade-offs, both the price of "brighter":
//   - three slots sit above the dark-mode lightness band. That band assumes one
//     dark surface; this palette serves a light one as well, and it is inside
//     the light band as a set.
//   - over the OSM basemap several slots fall under 3:1, which the method
//     allows only with secondary encoding. That is satisfied here: every layer
//     carries a swatch and a name in the layers panel, and can carry a direct
//     label on the map.
const PALETTE: RGB[] = [
  [57, 135, 229], //  blue    #3987e5
  [235, 104, 52], //  orange  #eb6834
  [27, 175, 122], //  aqua    #1baf7a
  [237, 161, 0], //   yellow  #eda100
  [232, 123, 164], // magenta #e87ba4
  [53, 177, 58], //   green   #35b13a
  [144, 133, 233], // violet  #9085e9
  [230, 103, 103], // red     #e66767
];

/**
 * Whether a result puts anything on the map or the timeline -- geometry, moving
 * objects, or a value plot. `query ten` is a relation of scalars: it is a real
 * result with rows, but it draws nothing, so it does not belong in the layers
 * panel (which is the map's legend) and gets no colour, style or draw order.
 * Its table is reached from the console entry that produced it.
 */
export function isDrawable(layer: Layer): boolean {
  return !!layer.geojson || !!layer.temporal;
}

function deriveName(command: string): string {
  const s = command.replace(/^\s*query\s+/i, "").trim();
  return s.length > 30 ? s.slice(0, 30) + "…" : s;
}

export function useLayers() {
  const [layers, setLayers] = useState<Layer[]>([]);
  const [selected, setSelected] = useState<Selection | null>(null);
  const idRef = useRef(0);

  // Returns the new layer's id: the console entry links to it, and the result
  // pane may need to open its table tab.
  const add = useCallback(
    (
      command: string,
      geojson: FeatureCollection | null,
      temporal: TemporalPayload | null,
      table: TablePayload | null = null,
      relation: string | null = null
    ): string => {
      idRef.current += 1;
      const id = `layer${idRef.current}`;
      setLayers((prev) => {
        // Only results that draw consume a palette slot, so a run of table-only
        // queries does not silently walk the colours forward.
        const color = PALETTE[prev.filter(isDrawable).length % PALETTE.length];
        const layer: Layer = {
          id,
          name: deriveName(command),
          command,
          geojson,
          temporal,
          table,
          relation,
          visible: true,
          style: {
            color,
            opacity: 0.85,
            pointRadius: 4,
            lineWidth: 1.5,
            filled: true,
            temporalMode: "both",
            // Labelling is opt-in: a fresh layer draws no text until an
            // attribute is chosen -- or a caption typed -- in the layers panel.
            label: null,
            labelText: null,
            icon: null,
            // Symbolic trajectories are the exception to that: they are shown
            // straight away, because a result only carries one when the query
            // asked for it by name.
            symbolicHidden: [],
            symbolicPrefix: false,
          },
        };
        return [...prev, layer];
      });
      return id;
    },
    []
  );

  // Replace a result's rows -- after it is reloaded with tuple identifiers for
  // editing, or after a commit. `relation` follows, since loading for editing is
  // what establishes which relation the rows belong to.
  const setTable = useCallback((id: string, table: TablePayload) => {
    setLayers((prev) =>
      prev.map((l) =>
        l.id === id ? { ...l, table, relation: table.relation ?? l.relation } : l
      )
    );
  }, []);

  const remove = useCallback((id: string) => {
    setLayers((prev) => prev.filter((l) => l.id !== id));
    setSelected((s) => (s?.layerId === id ? null : s));
  }, []);

  const toggle = useCallback((id: string) => {
    setLayers((prev) =>
      prev.map((l) => (l.id === id ? { ...l, visible: !l.visible } : l))
    );
  }, []);

  // Move a layer up (toward the top of the draw order) or down. It swaps with
  // the nearest *drawable* neighbour, skipping table-only results: those are not
  // listed in the panel, so swapping with one would look like nothing happened.
  const move = useCallback((id: string, dir: "up" | "down") => {
    setLayers((prev) => {
      const i = prev.findIndex((l) => l.id === id);
      if (i < 0) return prev;
      const step = dir === "up" ? 1 : -1; // higher index = drawn on top
      let j = i + step;
      while (j >= 0 && j < prev.length && !isDrawable(prev[j])) j += step;
      if (j < 0 || j >= prev.length) return prev;
      const next = [...prev];
      [next[i], next[j]] = [next[j], next[i]];
      return next;
    });
  }, []);

  // The auto-derived name is query text, which is rarely what belongs on a
  // legend. `command` stays untouched -- it is the layer's identity and still
  // what the row's tooltip shows. Clearing the name falls back to the derived
  // one, so "reset to default" needs no extra state.
  const rename = useCallback((id: string, name: string) => {
    setLayers((prev) =>
      prev.map((l) =>
        l.id === id ? { ...l, name: name.trim() || deriveName(l.command) } : l
      )
    );
  }, []);

  const setStyle = useCallback((id: string, patch: Partial<LayerStyle>) => {
    setLayers((prev) =>
      prev.map((l) =>
        l.id === id ? { ...l, style: { ...l.style, ...patch } } : l
      )
    );
  }, []);

  const clear = useCallback(() => {
    setLayers([]);
    setSelected(null);
  }, []);

  return {
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
  };
}
