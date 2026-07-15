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
  PathLayer,
  PolygonLayer,
  ScatterplotLayer,
} from "@deck.gl/layers";
import { TripsLayer } from "@deck.gl/geo-layers";
import { Map as BaseMap } from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";
import type { MovingRegion, Trip } from "../api/client";
import type { Layer } from "../layers/useLayers";
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
          pointType: "circle",
          getPointRadius: s.pointRadius,
          pointRadiusUnits: "pixels",
          getLineWidth: s.lineWidth,
          lineWidthUnits: "pixels",
          getFillColor: [r, g, b, Math.round(s.opacity * 150)],
          getLineColor: [r, g, b, Math.round(s.opacity * 255)],
          updateTriggers: {
            getFillColor: [r, g, b, s.opacity, s.filled],
            getLineColor: [r, g, b, s.opacity],
            getPointRadius: s.pointRadius,
            getLineWidth: s.lineWidth,
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
        const positions = [];
        for (const trip of trips) {
          const pos = positionAt(trip, currentTime);
          if (pos) positions.push({ position: pos, properties: trip.properties });
        }
        deckLayers.push(
          new ScatterplotLayer({
            id: `${layer.id}-pos`,
            data: positions,
            coordinateSystem,
            pickable: true,
            getPosition: (d: { position: [number, number] }) => d.position,
            getFillColor: [r, g, b],
            getRadius: s.pointRadius,
            radiusUnits: "pixels",
            radiusMinPixels: s.pointRadius,
            stroked: true,
            lineWidthMinPixels: 1,
            getLineColor: [255, 255, 255, 220],
            updateTriggers: { getFillColor: [r, g, b], getRadius: s.pointRadius },
          })
        );
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
