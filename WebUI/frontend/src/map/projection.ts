import type {
  FeatureCollection,
  MovingRegion,
  MovingVertex,
  Trip,
} from "../api/client";

// Projections a user can apply to a dataset's world coordinates.
export type Projection = "none" | "berlinmod";

export const PROJECTION_LABEL: Record<Projection, string> = {
  none: "Flat (no map)",
  berlinmod: "BerlinMOD → OSM",
};

type BBox = [number, number, number, number];

// BBBike (BerlinMOD / berlintest local) coordinates -> WGS84 lon/lat.
// Constants and formula taken verbatim from SECONDO's
// Algebras/Spatial/Berlin2WGS.cpp (which in turn is from the BBBike sources).
const X0 = -780761.760862528;
const X1 = 67978.2421158527;
const X2 = -2285.59137120724;
const Y0 = -5844741.03397902;
const Y1 = 1214.24447469596;
const Y2 = 111217.945663725;

export function berlin2wgs(x: number, y: number): [number, number] {
  const lon = ((x - X0) * Y2 - (y - Y0) * X2) / (X1 * Y2 - Y1 * X2);
  const lat = ((x - X0) * Y1 - (y - Y0) * X1) / (X2 * Y1 - X1 * Y2);
  return [lon, lat];
}

const PROJECTORS: Record<Projection, ((x: number, y: number) => [number, number]) | null> = {
  none: null,
  berlinmod: berlin2wgs,
};

// Recursively map the [x,y] leaves of a GeoJSON coordinates array.
type Coords = number[] | Coords[];
function mapCoords(c: Coords, fn: (x: number, y: number) => [number, number]): Coords {
  if (typeof c[0] === "number") {
    return fn(c[0] as number, c[1] as number);
  }
  return (c as Coords[]).map((child) => mapCoords(child, fn));
}

// The transform is linear, so a rectangle maps to a parallelogram whose
// axis-aligned bounds are attained at the projected corners.
export function projectBBox(b: BBox, projection: Projection): BBox {
  const fn = PROJECTORS[projection];
  if (!fn) return b;
  const [minx, miny, maxx, maxy] = b;
  const pts = [
    fn(minx, miny),
    fn(maxx, miny),
    fn(maxx, maxy),
    fn(minx, maxy),
  ];
  const xs = pts.map((p) => p[0]);
  const ys = pts.map((p) => p[1]);
  return [Math.min(...xs), Math.min(...ys), Math.max(...xs), Math.max(...ys)];
}

export function projectGeoJSON(
  fc: FeatureCollection,
  projection: Projection
): FeatureCollection {
  const fn = PROJECTORS[projection];
  if (!fn) return fc;
  const features = (fc.features as Array<{ geometry: { coordinates: Coords } }>).map(
    (f) => ({
      ...f,
      geometry: { ...f.geometry, coordinates: mapCoords(f.geometry.coordinates, fn) },
    })
  );
  return {
    ...fc,
    features,
    bbox: fc.bbox ? projectBBox(fc.bbox, projection) : undefined,
  };
}

export function projectTrips(trips: Trip[], projection: Projection): Trip[] {
  const fn = PROJECTORS[projection];
  if (!fn) return trips;
  return trips.map((t) => ({
    ...t,
    path: t.path.map(([x, y]) => fn(x, y)),
  }));
}

export function projectRegions(
  regions: MovingRegion[],
  projection: Projection
): MovingRegion[] {
  const fn = PROJECTORS[projection];
  if (!fn) return regions;
  // Both endpoints of every moving vertex must be projected.
  return regions.map((r) => ({
    ...r,
    units: r.units.map((u) => ({
      ...u,
      faces: u.faces.map((face) =>
        face.map((cycle) =>
          cycle.map(([x0, y0, x1, y1]): MovingVertex => {
            const [a, b] = fn(x0, y0);
            const [c, d] = fn(x1, y1);
            return [a, b, c, d];
          })
        )
      ),
    })),
  }));
}
