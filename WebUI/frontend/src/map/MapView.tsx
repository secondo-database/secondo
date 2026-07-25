import { useMemo, useState } from "react";
import DeckGL from "@deck.gl/react";
import {
  COORDINATE_SYSTEM,
  MapView as DeckMapView,
  OrthographicView,
  WebMercatorViewport,
} from "@deck.gl/core";
import type { PickingInfo } from "@deck.gl/core";
import {
  GeoJsonLayer,
  IconLayer,
  PathLayer,
  PolygonLayer,
  ScatterplotLayer,
  TextLayer,
} from "@deck.gl/layers";
import { TripsLayer } from "@deck.gl/geo-layers";
import { Map as BaseMap } from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";
import type { MovingRegion, Trip } from "../api/client";
import type { Layer, RGB } from "../layers/useLayers";
import { iconAtlas } from "../layers/icons";
import {
  projectBBox,
  projectGeoJSON,
  projectRegions,
  projectTrips,
  type Projection,
} from "./projection";

type BBox = [number, number, number, number];

const orthoView = new OrthographicView({ id: "ortho", flipY: false });
const geoView = new DeckMapView({ id: "geo", repeat: true });

// Raster OpenStreetMap basemap (no external style file / API key needed).
const OSM_STYLE = {
  version: 8 as const,
  sources: {
    osm: {
      type: "raster" as const,
      tiles: ["https://a.tile.openstreetmap.org/{z}/{x}/{y}.png"],
      tileSize: 256,
      attribution: "© OpenStreetMap contributors",
    },
  },
  layers: [{ id: "osm", type: "raster" as const, source: "osm" }],
};

// A bbox is geographic when it fits within valid lon/lat ranges. SECONDO's own
// coordinate systems (e.g. berlintest ~10000) fall outside and stay Cartesian.
function isGeographic(bbox: BBox | undefined): boolean {
  if (!bbox) return false;
  const [minx, miny, maxx, maxy] = bbox;
  return (
    minx >= -180 && maxx <= 180 && miny >= -90 && maxy <= 90 &&
    // reject a degenerate 0-area bbox around (0,0)
    !(minx === 0 && miny === 0 && maxx === 0 && maxy === 0)
  );
}

// Interpolate a moving object's position along its (normalized) path at time
// `t`, or null when it is not defined at t. Same piecewise-linear model the
// HoeseViewer uses; here it drives the exact-position dots.
function positionAt(trip: Trip, t: number): [number, number] | null {
  const ts = trip.timestamps;
  const path = trip.path;
  if (ts.length < 2 || t < ts[0] || t > ts[ts.length - 1]) return null;
  for (let i = 0; i < ts.length - 1; i++) {
    if (t >= ts[i] && t <= ts[i + 1]) {
      const span = ts[i + 1] - ts[i] || 1;
      const f = (t - ts[i]) / span;
      const [x0, y0] = path[i];
      const [x1, y1] = path[i + 1];
      return [x0 + f * (x1 - x0), y0 + f * (y1 - y0)];
    }
  }
  return null;
}

// The polygon rings of a moving region at time `t`, or null when it is not
// defined then. Each vertex interpolates linearly across its unit -- the same
// model as moving points, and what the HoeseViewer's Dsplmovingregion does.
function facesAt(region: MovingRegion, t: number): [number, number][][][] | null {
  for (const unit of region.units) {
    const [t0, t1] = unit.interval;
    if (t >= t0 && t <= t1) {
      const f = t1 > t0 ? (t - t0) / (t1 - t0) : 0;
      return unit.faces.map((face) =>
        face.map((cycle) =>
          cycle.map(
            ([x0, y0, x1, y1]) =>
              [x0 + f * (x1 - x0), y0 + f * (y1 - y0)] as [number, number]
          )
        )
      );
    }
  }
  return null;
}

// Where a feature's label goes: the point itself, or the mean of a line's or
// region's vertices -- the same "middle of the thing" the HoeseViewer writes a
// region's label at. Good enough for a label; not a true area centroid.
function labelAnchor(geometry: {
  coordinates?: unknown;
}): [number, number] | null {
  let sx = 0;
  let sy = 0;
  let n = 0;
  const walk = (c: unknown) => {
    if (
      Array.isArray(c) &&
      c.length >= 2 &&
      typeof c[0] === "number" &&
      typeof c[1] === "number"
    ) {
      sx += c[0];
      sy += c[1];
      n++;
    } else if (Array.isArray(c)) {
      for (const x of c) walk(x);
    }
  };
  walk(geometry?.coordinates);
  return n > 0 ? [sx / n, sy / n] : null;
}

// Labels have to stay readable on the dark Cartesian canvas *and* on the light
// OSM basemap, and a saturated layer colour manages neither at this size.
// Lightening it most of the way to white keeps a hint of the layer's hue --
// enough to tell two layers' labels apart -- while the plate behind the text
// does the contrast work.
function labelColor([r, g, b]: RGB): [number, number, number] {
  const lift = (v: number) => Math.round(v + (255 - v) * 0.8);
  return [lift(r), lift(g), lift(b)];
}

// Shared by the static and the moving-object labels, which otherwise drift
// apart.
//
// Deliberately *not* SDF text with an outline, which is what this used to be.
// deck renders SDF glyphs from a distance-field atlas, and at label sizes the
// field is too coarse to reconstruct 13px letterforms: the strokes come out
// eroded and the tops of the letters sheared off, which is what made these
// labels look eaten away. Raising fontSettings.buffer/radius does not fix it.
// Plain bitmap glyphs are exact at this size.
//
// Losing the outline means losing what kept text legible over a busy basemap,
// so the contrast comes from a translucent plate instead. On the dark canvas it
// disappears into the background; over the OSM tiles it is what separates a
// SECONDO label from the dozen OSM ones around it.
const LABEL_TEXT = {
  getSize: 13,
  sizeUnits: "pixels" as const,
  getTextAnchor: "middle" as const,
  getAlignmentBaseline: "top" as const,
  // SECONDO data is Latin-1: without an automatic character set the umlauts in
  // street and restaurant names come out as placeholders.
  characterSet: "auto" as const,
  background: true,
  getBackgroundColor: [10, 12, 16, 190] as [number, number, number, number],
  backgroundPadding: [4, 1, 4, 1] as [number, number, number, number],
  backgroundBorderRadius: 2,
};

interface LabelDatum {
  position: [number, number];
  text: string;
}

// A moving object's exact position at the current instant, plus the tuple it
// came from so picking and labelling still have the attributes.
interface PositionDatum {
  position: [number, number];
  properties: unknown;
}

function unionBBox(layers: Layer[]): BBox | undefined {
  let minx = Infinity, miny = Infinity, maxx = -Infinity, maxy = -Infinity;
  for (const l of layers) {
    for (const b of [l.geojson?.bbox, l.temporal?.bbox]) {
      if (b) {
        minx = Math.min(minx, b[0]);
        miny = Math.min(miny, b[1]);
        maxx = Math.max(maxx, b[2]);
        maxy = Math.max(maxy, b[3]);
      }
    }
  }
  return minx === Infinity ? undefined : [minx, miny, maxx, maxy];
}

function fitCartesian(bbox: BBox | undefined) {
  if (!bbox) return { target: [0, 0, 0] as [number, number, number], zoom: 0 };
  const [minx, miny, maxx, maxy] = bbox;
  const spanX = Math.max(maxx - minx, 1e-6);
  const spanY = Math.max(maxy - miny, 1e-6);
  return {
    target: [(minx + maxx) / 2, (miny + maxy) / 2, 0] as [number, number, number],
    zoom: Math.log2(Math.min((900 * 0.9) / spanX, (700 * 0.9) / spanY)),
  };
}

function fitGeographic(bbox: BBox | undefined) {
  // No layers (e.g. the last one was just removed): show a neutral world view.
  if (!bbox) return { longitude: 0, latitude: 0, zoom: 1 };
  // Defensive clamp: Web Mercator blows up outside these ranges, which would
  // silently degrade into a whole-world view.
  const clampLon = (v: number) => Math.min(180, Math.max(-180, v));
  const clampLat = (v: number) => Math.min(85, Math.max(-85, v));
  const [minx, miny, maxx, maxy] = [
    clampLon(bbox[0]),
    clampLat(bbox[1]),
    clampLon(bbox[2]),
    clampLat(bbox[3]),
  ];
  if (maxx - minx < 1e-9 || maxy - miny < 1e-9) {
    return { longitude: (minx + maxx) / 2, latitude: (miny + maxy) / 2, zoom: 14 };
  }
  const vp = new WebMercatorViewport({ width: 720, height: 800 });
  const { longitude, latitude, zoom } = vp.fitBounds(
    [
      [minx, miny],
      [maxx, maxy],
    ],
    { padding: 40 }
  );
  return { longitude, latitude, zoom: Math.min(zoom, 18) };
}

interface Props {
  layers: Layer[];
  globalT0: number;
  currentTime: number;
  projection: Projection;
  onSelect: (layerId: string | null, object: unknown) => void;
}

export function MapView({
  layers,
  globalT0,
  currentTime,
  projection,
  onSelect,
}: Props) {
  // Apply the chosen projection to each layer's coordinates once (not per
  // frame). With "berlinmod" the local BBBike coordinates become WGS84 lon/lat.
  const layersToRender = useMemo(() => {
    if (projection === "none") return layers;
    return layers.map((l) => ({
      ...l,
      geojson: l.geojson ? projectGeoJSON(l.geojson, projection) : null,
      temporal: l.temporal
        ? {
            ...l.temporal,
            trips: projectTrips(l.temporal.trips, projection),
            regions: projectRegions(l.temporal.regions ?? [], projection),
            // The bbox must be projected too, or the view fit would treat raw
            // world coordinates as lon/lat (moving objects showed the globe).
            // Plot-only objects (mreal/mint) have no geometry, hence no bbox.
            bbox: l.temporal.bbox
              ? projectBBox(l.temporal.bbox, projection)
              : null,
          }
        : null,
    }));
  }, [layers, projection]);

  const bbox = useMemo(() => unionBBox(layersToRender), [layersToRender]);
  const geographic = useMemo(
    () => projection !== "none" || isGeographic(bbox),
    [bbox, projection]
  );
  // The view that fits the current data in the current projection. deck.gl's
  // OrthographicViewState / MapViewState union is awkward to thread through
  // controlled state, so this view state is intentionally loosely typed.
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  type VState = any;
  const fit = useMemo<VState>(
    () => (geographic ? fitGeographic(bbox) : fitCartesian(bbox)),
    [bbox, geographic]
  );

  // Controlled view state: user pan/zoom updates it. We deliberately re-fit
  // ONLY when a layer the map has never shown appears, or the projection
  // changes (which moves the data to a whole new coordinate space). Editing a
  // layer's style, toggling visibility or removing a layer must leave the view
  // exactly where the user put it. The "adjust state during render" pattern
  // keeps `views` and `viewState` in sync in the same render.
  const [viewState, setViewState] = useState<VState>(fit);
  const [fitState, setFitState] = useState<{ seen: string[]; projection: Projection }>(
    { seen: [], projection }
  );
  const unseen = layersToRender.filter((l) => !fitState.seen.includes(l.id));
  const projectionChanged = fitState.projection !== projection;
  if (unseen.length > 0 || projectionChanged) {
    setFitState({
      seen: [...fitState.seen, ...unseen.map((l) => l.id)],
      projection,
    });
    setViewState(fit);
  }

  const zoomBy = (delta: number) =>
    setViewState((vs: VState) => ({ ...vs, zoom: (vs.zoom ?? 0) + delta }));
  const recenter = () => setViewState(fit);
  const coordinateSystem = geographic
    ? COORDINATE_SYSTEM.LNGLAT
    : COORDINATE_SYSTEM.CARTESIAN;

  const tripsById = useMemo(() => {
    const m = new Map<string, Trip[]>();
    for (const l of layersToRender) {
      if (l.temporal) {
        m.set(
          l.id,
          l.temporal.trips.map((tr) => ({
            ...tr,
            timestamps: tr.timestamps.map((t) => t - globalT0),
          }))
        );
      }
    }
    return m;
  }, [layersToRender, globalT0]);

  // Same normalisation for moving regions: unit intervals are absolute POSIX
  // seconds, while `currentTime` runs from the shared t0.
  const regionsById = useMemo(() => {
    const m = new Map<string, MovingRegion[]>();
    for (const l of layersToRender) {
      const regions = l.temporal?.regions;
      if (regions && regions.length > 0) {
        m.set(
          l.id,
          regions.map((r) => ({
            ...r,
            units: r.units.map((u) => ({
              ...u,
              interval: [u.interval[0] - globalT0, u.interval[1] - globalT0] as [
                number,
                number,
              ],
            })),
          }))
        );
      }
    }
    return m;
  }, [layersToRender, globalT0]);

  // Label text and anchors per layer, computed once rather than per frame: the
  // map re-renders on every animation tick, and walking each feature's
  // coordinates there would be wasted work. Anchors come from the *projected*
  // geometry, so labels follow the data into geographic mode.
  const labelsById = useMemo(() => {
    const m = new Map<string, LabelDatum[]>();
    for (const l of layersToRender) {
      // An attribute if one was chosen, otherwise the caption typed for a layer
      // that has no attributes to offer (an individual object).
      const attr = l.style.label;
      const fixed = l.style.labelText;
      if ((!attr && !fixed) || !l.geojson) continue;
      const data: LabelDatum[] = [];
      for (const f of l.geojson.features as {
        properties?: Record<string, unknown>;
        geometry?: { coordinates?: unknown };
      }[]) {
        const value = attr ? f.properties?.[attr] : fixed;
        if (value === undefined || value === null || value === "") continue;
        const at = f.geometry ? labelAnchor(f.geometry) : null;
        if (at) data.push({ position: at, text: String(value) });
      }
      if (data.length > 0) m.set(l.id, data);
    }
    return m;
  }, [layersToRender]);

  const combinedDuration = useMemo(() => {
    let span = 0;
    for (const l of layersToRender)
      if (l.temporal)
        span = Math.max(span, l.temporal.timeDomain[1] - l.temporal.timeDomain[0]);
    return span;
  }, [layersToRender]);
  const trailLength = Math.max(combinedDuration * 0.08, 60);

  const deckLayers = [];
  for (const layer of layersToRender) {
    const [r, g, b] = layer.style.color;
    const s = layer.style;

    // Only touched when the layer actually wants an icon, so a session that
    // never assigns one never builds the atlas. And never ask deck for icons
    // without one: IconLayer would fall back to auto-packing, and that throws,
    // since getIcon hands it an id rather than the {url} auto-packing expects.
    const atlas = s.icon ? iconAtlas() : null;
    const icon = atlas ? s.icon : null;
    // The point slider stays the single size control. A glyph needs more room
    // than a disc to read, and the atlas insets it inside its cell, so the icon
    // box is ~4x the circle radius: the default r=4 gives a 16px box holding a
    // 13px glyph.
    const iconPx = Math.max(12, s.pointRadius * 4);
    // How far below a symbol its label sits: a circle's extent is its radius,
    // an icon's is half its square cell.
    const labelDrop = (icon ? iconPx / 2 : s.pointRadius) + 8;
    // Typed up front: inside a conditional spread the literal has no
    // contextual type to widen against deck's Color.
    const iconColor: [number, number, number, number] = [
      r,
      g,
      b,
      Math.round(s.opacity * 255),
    ];

    if (layer.geojson) {
      deckLayers.push(
        new GeoJsonLayer({
          id: `${layer.id}-static`,
          // eslint-disable-next-line @typescript-eslint/no-explicit-any
          data: layer.geojson as any,
          coordinateSystem,
          pickable: true,
          autoHighlight: true,
          highlightColor: [255, 255, 255, 90],
          stroked: true,
          filled: s.filled,
          // Only point geometry is affected; lines and polygons render the
          // same either way. The circle props below stay in place so clearing
          // the icon restores the old look with no further state.
          pointType: icon ? "icon" : "circle",
          getPointRadius: s.pointRadius,
          pointRadiusUnits: "pixels",
          ...(icon && {
            iconAtlas: atlas!.url,
            iconMapping: atlas!.mapping,
            getIcon: icon,
            getIconSize: iconPx,
            iconSizeUnits: "pixels" as const,
            getIconColor: iconColor,
          }),
          getLineWidth: s.lineWidth,
          lineWidthUnits: "pixels",
          getFillColor: [r, g, b, Math.round(s.opacity * 150)],
          getLineColor: [r, g, b, Math.round(s.opacity * 255)],
          updateTriggers: {
            getFillColor: [r, g, b, s.opacity, s.filled],
            getLineColor: [r, g, b, s.opacity],
            getPointRadius: s.pointRadius,
            getLineWidth: s.lineWidth,
            getIcon: icon,
            getIconSize: iconPx,
            getIconColor: [r, g, b, s.opacity],
          },
        })
      );
    }

    // The chosen attribute, written just below each feature so it never covers
    // the geometry it names.
    const labels = labelsById.get(layer.id);
    if (labels) {
      deckLayers.push(
        new TextLayer<LabelDatum>({
          id: `${layer.id}-labels`,
          data: labels,
          coordinateSystem,
          ...LABEL_TEXT,
          getPosition: (d) => d.position,
          getText: (d) => d.text,
          getColor: [...labelColor(layer.style.color), 255],
          // Push the text below the symbol rather than on top of it.
          getPixelOffset: [0, labelDrop],
          updateTriggers: {
            getColor: layer.style.color,
            getPixelOffset: labelDrop,
          },
        })
      );
    }

    // Moving regions: rebuild the polygon at the current instant each frame.
    const regions = regionsById.get(layer.id);
    if (regions && regions.length > 0) {
      const polys: { polygon: [number, number][][]; properties: unknown }[] = [];
      for (const region of regions) {
        const faces = facesAt(region, currentTime);
        if (faces) {
          for (const face of faces) {
            polys.push({ polygon: face, properties: region.properties });
          }
        }
      }
      if (polys.length > 0) {
        deckLayers.push(
          new PolygonLayer({
            id: `${layer.id}-mregion`,
            data: polys,
            coordinateSystem,
            pickable: true,
            filled: s.filled,
            stroked: true,
            getPolygon: (d: { polygon: [number, number][][] }) => d.polygon,
            getFillColor: [r, g, b, Math.round(s.opacity * 130)],
            getLineColor: [r, g, b, Math.round(s.opacity * 255)],
            getLineWidth: s.lineWidth,
            lineWidthUnits: "pixels",
            updateTriggers: {
              getFillColor: [r, g, b, s.opacity, s.filled],
              getLineColor: [r, g, b, s.opacity],
              getLineWidth: s.lineWidth,
            },
          })
        );

        // Labelled like everything else -- the text sits in the middle of the
        // face and is rebuilt with it, so it moves as the region does.
        const attr = s.label;
        const fixed = s.labelText;
        if (attr || fixed) {
          const texts: LabelDatum[] = [];
          for (const p of polys) {
            const value = attr
              ? (p.properties as Record<string, unknown> | undefined)?.[attr]
              : fixed;
            if (value === undefined || value === null || value === "") continue;
            const at = labelAnchor({ coordinates: p.polygon });
            if (at) texts.push({ position: at, text: String(value) });
          }
          if (texts.length > 0) {
            deckLayers.push(
              new TextLayer<LabelDatum>({
                id: `${layer.id}-mregion-labels`,
                data: texts,
                coordinateSystem,
                ...LABEL_TEXT,
                getPosition: (d) => d.position,
                getText: (d) => d.text,
                getColor: [...labelColor(s.color), 255],
                getPixelOffset: [0, labelDrop],
                updateTriggers: {
                  getColor: s.color,
                  getPixelOffset: labelDrop,
                },
              })
            );
          }
        }
      }
    }

    const trips = tripsById.get(layer.id);
    if (trips && trips.length > 0) {
      const showTrail = s.temporalMode === "trail" || s.temporalMode === "both";
      const showPoints = s.temporalMode === "points" || s.temporalMode === "both";

      if (showTrail) {
        // Faint full trajectory for context.
        deckLayers.push(
          new PathLayer({
            id: `${layer.id}-context`,
            data: trips,
            coordinateSystem,
            getPath: (d: Trip) => d.path,
            getColor: [r, g, b, 55],
            getWidth: 1,
            widthUnits: "pixels",
            widthMinPixels: 1,
            updateTriggers: { getColor: [r, g, b] },
          })
        );
        // Animated trail; lineWidth controls its thickness.
        deckLayers.push(
          new TripsLayer({
            id: `${layer.id}-trips`,
            data: trips,
            coordinateSystem,
            getPath: (d: Trip) => d.path,
            getTimestamps: (d: { timestamps: number[] }) => d.timestamps,
            getColor: [r, g, b],
            opacity: s.opacity,
            widthMinPixels: Math.max(s.lineWidth, 1),
            capRounded: true,
            jointRounded: true,
            trailLength,
            currentTime,
            fadeTrail: true,
            updateTriggers: { getColor: [r, g, b], getWidth: s.lineWidth },
          })
        );
      }

      if (showPoints) {
        // Exact current positions of each moving object; pointRadius controls
        // the dot size. Recomputed each frame as currentTime advances.
        const positions: PositionDatum[] = [];
        for (const trip of trips) {
          const pos = positionAt(trip, currentTime);
          if (pos) positions.push({ position: pos, properties: trip.properties });
        }
        // The two classes must not share an id. deck matches layers across
        // renders by id alone and transfers the old layer's state to the new
        // instance without checking the class, so a Scatterplot->Icon swap on
        // one id hands the IconLayer state that has no icon manager in it.
        // Picking is unaffected: it takes the id up to the first dash.
        deckLayers.push(
          icon
            ? new IconLayer<PositionDatum>({
                id: `${layer.id}-pos-icon`,
                data: positions,
                coordinateSystem,
                pickable: true,
                iconAtlas: atlas!.url,
                iconMapping: atlas!.mapping,
                getPosition: (d) => d.position,
                // IconLayer types getIcon as an accessor function only (unlike
                // GeoJsonLayer's, which takes the id directly).
                getIcon: () => icon,
                getSize: iconPx,
                sizeUnits: "pixels",
                getColor: iconColor,
                updateTriggers: {
                  getIcon: icon,
                  getSize: iconPx,
                  getColor: iconColor,
                },
              })
            : new ScatterplotLayer<PositionDatum>({
                id: `${layer.id}-pos`,
                data: positions,
                coordinateSystem,
                pickable: true,
                getPosition: (d) => d.position,
                getFillColor: [r, g, b],
                getRadius: s.pointRadius,
                radiusUnits: "pixels",
                radiusMinPixels: s.pointRadius,
                stroked: true,
                lineWidthMinPixels: 1,
                getLineColor: [255, 255, 255, 220],
                updateTriggers: {
                  getFillColor: [r, g, b],
                  getRadius: s.pointRadius,
                },
              })
        );

        // A moving object carries its tuple's attributes too, so it is labelled
        // the same way -- the label travels with the dot. The positions are
        // already computed for the dots above, so this costs only the text.
        // A lone mpoint (`query train7`) has no attributes and rides its typed
        // caption instead.
        const attr = layer.style.label;
        const fixed = layer.style.labelText;
        if (attr || fixed) {
          const moving: LabelDatum[] = [];
          for (const p of positions) {
            const value = attr
              ? (p.properties as Record<string, unknown> | undefined)?.[attr]
              : fixed;
            if (value === undefined || value === null || value === "") continue;
            moving.push({ position: p.position, text: String(value) });
          }
          if (moving.length > 0) {
            deckLayers.push(
              new TextLayer<LabelDatum>({
                id: `${layer.id}-pos-labels`,
                data: moving,
                coordinateSystem,
                ...LABEL_TEXT,
                getPosition: (d) => d.position,
                getText: (d) => d.text,
                getColor: [...labelColor(layer.style.color), 255],
                getPixelOffset: [0, labelDrop],
                updateTriggers: {
                  getColor: layer.style.color,
                  getPixelOffset: labelDrop,
                },
              })
            );
          }
        }
      }
    }
  }

  return (
    // The mode is already visible in the projection dropdown; expose it here as
    // state (not a second label) so tests can assert it.
    <div
      className="mapview"
      data-projection={projection}
      data-geographic={geographic ? "true" : "false"}
    >
      <DeckGL
        views={geographic ? geoView : orthoView}
        viewState={viewState}
        onViewStateChange={(e: { viewState: VState }) => setViewState(e.viewState)}
        controller={true}
        layers={deckLayers}
        onClick={(info: PickingInfo) => {
          if (info.object && info.layer) {
            onSelect(info.layer.id.split("-")[0], info.object);
          } else {
            onSelect(null, null);
          }
        }}
        getTooltip={({ object }) => {
          const props = (object as { properties?: Record<string, unknown> })
            ?.properties;
          if (!props) return null;
          const text = Object.entries(props)
            .filter(([k]) => !k.startsWith("_")) // hide internal _attr/_layer
            .map(([k, v]) => `${k}: ${v}`)
            .join("\n");
          return text ? { text } : null;
        }}
      >
        {geographic && <BaseMap reuseMaps mapStyle={OSM_STYLE} />}
      </DeckGL>
      <div className="zoom-ctl">
        <button onClick={() => zoomBy(0.6)} title="Zoom in">
          +
        </button>
        <button onClick={() => zoomBy(-0.6)} title="Zoom out">
          −
        </button>
        <button onClick={recenter} title="Fit to data" className="zoom-fit">
          ⤢
        </button>
      </div>
      {layers.length === 0 && (
        <div className="map-empty">Run a spatial or moving-object query</div>
      )}
    </div>
  );
}
