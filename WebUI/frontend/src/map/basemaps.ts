// The raster basemaps offered under geographic mode, and the one thing about
// each that the rest of the map has to know: whether the canvas it paints is
// light. Labels take their ink and halo from that (see `onLightCanvas` in
// ./MapView) -- near-black on white over OSM, near-white on black over imagery.
//
// All three are key-free. Google's layers, which the old GWT WebGui2 offered,
// need an API key and carry terms of use, so they are deliberately absent.

/** A MapLibre style holding exactly one raster source. */
interface RasterStyle {
  version: 8;
  sources: Record<
    string,
    {
      type: "raster";
      tiles: string[];
      tileSize: number;
      maxzoom: number;
      attribution: string;
    }
  >;
  layers: { id: string; type: "raster"; source: string }[];
}

export type BasemapId = "osm" | "satellite" | "dark";

export interface Basemap {
  /** Shown in the picker. */
  label: string;
  /** Is the canvas light under the labels? Drives their ink and halo. */
  light: boolean;
  style: RasterStyle;
}

/** One raster source is a whole style; this spares each entry the boilerplate.
 *  `maxzoom` is not optional in practice: without it MapLibre keeps asking for
 *  deeper tiles than the provider has and the map goes blank when zoomed in. */
function rasterStyle(
  id: string,
  tiles: string[],
  maxzoom: number,
  attribution: string
): RasterStyle {
  return {
    version: 8,
    sources: {
      [id]: { type: "raster", tiles, tileSize: 256, maxzoom, attribution },
    },
    layers: [{ id, type: "raster", source: id }],
  };
}

export const BASEMAPS: Record<BasemapId, Basemap> = {
  osm: {
    // "OSM", not "OpenStreetMap": at 16px (the iOS rule) the long form crowds
    // its own dropdown arrow off a 360px screen, and the projection option
    // "BerlinMOD -> OSM" has already established the abbreviation in this UI.
    // The full name is still on the map, in the attribution.
    label: "OSM",
    light: true,
    style: rasterStyle(
      "osm",
      ["https://a.tile.openstreetmap.org/{z}/{x}/{y}.png"],
      19,
      "© OpenStreetMap contributors"
    ),
  },
  satellite: {
    label: "Satellite",
    light: false,
    style: rasterStyle(
      "satellite",
      // The ArcGIS REST tile scheme is {z}/{y}/{x} -- row before column, unlike
      // every other provider here. Swapping them silently returns tiles from
      // the wrong place rather than failing.
      [
        "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}",
      ],
      19,
      "© Esri, Maxar, Earthstar Geographics"
    ),
  },
  dark: {
    label: "Dark",
    light: false,
    style: rasterStyle(
      "dark",
      // No `{r}` retina placeholder: that is Leaflet's, and MapLibre would
      // request the two characters literally instead of substituting "@2x".
      ["https://a.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}.png"],
      20,
      "© OpenStreetMap contributors © CARTO"
    ),
  },
};

export const DEFAULT_BASEMAP: BasemapId = "osm";

const BASEMAP_KEY = "secondo.webui.basemap";

function isBasemapId(v: string): v is BasemapId {
  return Object.prototype.hasOwnProperty.call(BASEMAPS, v);
}

/** The remembered choice, or the default. Which basemap suits a user's screen
 *  and eyes is a preference like the theme, not a property of the dataset (the
 *  way the projection is), so it is remembered across reloads. */
export function loadBasemap(): BasemapId {
  try {
    const raw = localStorage.getItem(BASEMAP_KEY);
    if (raw && isBasemapId(raw)) return raw;
  } catch {
    /* storage unavailable */
  }
  return DEFAULT_BASEMAP;
}

export function saveBasemap(basemap: BasemapId): void {
  try {
    localStorage.setItem(BASEMAP_KEY, basemap);
  } catch {
    /* storage unavailable */
  }
}
