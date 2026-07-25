// Point symbols for the map, baked into a single PNG atlas.
//
// The icons are Maki (https://github.com/mapbox/maki, CC0-1.0) -- a set drawn
// specifically for cartography at small sizes, which is what a 16px symbol over
// a basemap needs. Each one is a 15x15 SVG holding exactly one <path>, so the
// whole set can be rasterised into one canvas here and handed to deck.gl as a
// data URL: deck resolves it through IconLayer's async image prop, fetches it
// once, and shares the texture across every layer.
//
// Icons are drawn in solid white and the atlas entries carry `mask: true`, so
// the shader tints each glyph with the layer's own colour -- one atlas serves
// every layer regardless of palette.

import airport from "@mapbox/maki/icons/airport.svg?raw";
import bank from "@mapbox/maki/icons/bank.svg?raw";
import bar from "@mapbox/maki/icons/bar.svg?raw";
import bicycle from "@mapbox/maki/icons/bicycle.svg?raw";
import building from "@mapbox/maki/icons/building.svg?raw";
import bus from "@mapbox/maki/icons/bus.svg?raw";
import cafe from "@mapbox/maki/icons/cafe.svg?raw";
import car from "@mapbox/maki/icons/car.svg?raw";
import chargingStation from "@mapbox/maki/icons/charging-station.svg?raw";
import circleStroked from "@mapbox/maki/icons/circle-stroked.svg?raw";
import fastFood from "@mapbox/maki/icons/fast-food.svg?raw";
import ferry from "@mapbox/maki/icons/ferry.svg?raw";
import fireStation from "@mapbox/maki/icons/fire-station.svg?raw";
import fuel from "@mapbox/maki/icons/fuel.svg?raw";
import harbor from "@mapbox/maki/icons/harbor.svg?raw";
import hospital from "@mapbox/maki/icons/hospital.svg?raw";
import lodging from "@mapbox/maki/icons/lodging.svg?raw";
import marker from "@mapbox/maki/icons/marker.svg?raw";
import monument from "@mapbox/maki/icons/monument.svg?raw";
import park from "@mapbox/maki/icons/park.svg?raw";
import parking from "@mapbox/maki/icons/parking.svg?raw";
import police from "@mapbox/maki/icons/police.svg?raw";
import post from "@mapbox/maki/icons/post.svg?raw";
import rail from "@mapbox/maki/icons/rail.svg?raw";
import railLight from "@mapbox/maki/icons/rail-light.svg?raw";
import railMetro from "@mapbox/maki/icons/rail-metro.svg?raw";
import restaurant from "@mapbox/maki/icons/restaurant.svg?raw";
import school from "@mapbox/maki/icons/school.svg?raw";
import scooter from "@mapbox/maki/icons/scooter.svg?raw";
import shop from "@mapbox/maki/icons/shop.svg?raw";
import star from "@mapbox/maki/icons/star.svg?raw";
import taxi from "@mapbox/maki/icons/taxi.svg?raw";
import triangle from "@mapbox/maki/icons/triangle.svg?raw";
import water from "@mapbox/maki/icons/water.svg?raw";

// Maki ships 215 icons; offering all of them would bloat both the atlas and the
// dropdown. This is the subset a SECONDO result plausibly *is* -- vehicles for
// moving objects, POI symbols for point relations, plain shapes as neutral
// alternatives to the circle. Adding another is one import plus one entry here.
const SVG = {
  // transport
  bus,
  rail,
  "rail-metro": railMetro,
  "rail-light": railLight,
  car,
  taxi,
  bicycle,
  scooter,
  ferry,
  harbor,
  airport,
  fuel,
  "charging-station": chargingStation,
  parking,
  // amenities
  restaurant,
  cafe,
  bar,
  "fast-food": fastFood,
  lodging,
  shop,
  // civic
  hospital,
  police,
  "fire-station": fireStation,
  school,
  bank,
  post,
  // places and plain shapes
  building,
  monument,
  park,
  water,
  marker,
  star,
  triangle,
  "circle-stroked": circleStroked,
} as const;

export type IconName = keyof typeof SVG;

// Alphabetical: the dropdown is a flat list of 34 names with no headings, so
// the only way to find one is to know where it sorts. The grouping above is
// for whoever edits this file, not for whoever uses the picker.
export const ICON_NAMES = (Object.keys(SVG) as IconName[]).sort();

const CELL = 64; // atlas cell: ~4x Maki's 15px design grid, crisp well past 48px
const INSET = 6; // transparent margin, which doubles as the mip-bleed guard
const COLS = 8;

export interface IconAtlas {
  url: string;
  mapping: Record<string, {
    x: number;
    y: number;
    width: number;
    height: number;
    mask: boolean;
  }>;
}

// 77 of Maki's 215 icons write XML character references (&#xA;, &#x9;) as
// whitespace inside `d` -- restaurant, cafe, bar, bicycle and airport among
// them. Path2D's parser stops dead at the '&' and silently renders a truncated
// glyph, so the attribute has to be decoded first. A regex looks equivalent
// here and is not: let the DOM do it.
function pathOf(raw: string): string {
  const doc = new DOMParser().parseFromString(raw, "image/svg+xml");
  return doc.querySelector("path")?.getAttribute("d") ?? "";
}

let cached: IconAtlas | null | undefined;

// Built on first use rather than at import: a session that never assigns an
// icon pays nothing for the atlas. Memoising also keeps the URL string and the
// mapping object at stable identities, which matters -- deck compares async
// props by ===, so a stable atlas is fetched exactly once and a stable mapping
// never invalidates the getIcon attribute.
export function iconAtlas(): IconAtlas | null {
  if (cached !== undefined) return cached;
  cached = build();
  return cached;
}

function build(): IconAtlas | null {
  if (typeof document === "undefined") return null;
  const rows = Math.ceil(ICON_NAMES.length / COLS);
  const canvas = document.createElement("canvas");
  canvas.width = COLS * CELL;
  canvas.height = rows * CELL;
  const ctx = canvas.getContext("2d");
  if (!ctx) return null;

  const mapping: IconAtlas["mapping"] = {};
  // White, because the atlas entries are masks: deck multiplies the texel alpha
  // by the layer colour, so the glyph's own colour must not tint the result.
  ctx.fillStyle = "#ffffff";
  const scale = (CELL - 2 * INSET) / 15;

  ICON_NAMES.forEach((name, i) => {
    const x = (i % COLS) * CELL;
    const y = Math.floor(i / COLS) * CELL;
    const d = pathOf(SVG[name]);
    if (d) {
      ctx.save();
      ctx.translate(x + INSET, y + INSET);
      ctx.scale(scale, scale);
      // Maki uses no fill-rule anywhere, so Canvas2D's default nonzero winding
      // is the right one.
      ctx.fill(new Path2D(d));
      ctx.restore();
    }
    mapping[name] = { x, y, width: CELL, height: CELL, mask: true };
  });

  return { url: canvas.toDataURL("image/png"), mapping };
}

// The raw path data, for the picker's inline SVG preview -- names alone are a
// poor clue as to what a glyph looks like.
export function iconPathData(name: IconName): string {
  return pathOf(SVG[name]);
}
