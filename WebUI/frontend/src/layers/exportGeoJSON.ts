import type { Layer } from "./useLayers";

// Build one GeoJSON FeatureCollection from the visible layers. Static layers
// contribute their features directly; moving-object layers contribute each
// trip as a LineString carrying its timestamps, so the export is self-contained.
export function buildExportFC(layers: Layer[]): {
  type: "FeatureCollection";
  features: unknown[];
} {
  const features: unknown[] = [];
  for (const l of layers) {
    if (!l.visible) continue;
    if (l.geojson) {
      for (const f of l.geojson.features as Array<{
        properties?: Record<string, unknown>;
      }>) {
        features.push({
          ...f,
          properties: { ...(f.properties ?? {}), _layer: l.name },
        });
      }
    }
    if (l.temporal) {
      for (const trip of l.temporal.trips) {
        features.push({
          type: "Feature",
          geometry: { type: "LineString", coordinates: trip.path },
          properties: {
            ...trip.properties,
            _layer: l.name,
            timestamps: trip.timestamps,
          },
        });
      }
    }
  }
  return { type: "FeatureCollection", features };
}

// Trigger a browser download of the visible layers as a .geojson file.
export function downloadGeoJSON(layers: Layer[]): void {
  const fc = buildExportFC(layers);
  const blob = new Blob([JSON.stringify(fc, null, 2)], {
    type: "application/geo+json",
  });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = "secondo-export.geojson";
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}
