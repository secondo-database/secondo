import { useCallback, useRef, useState } from "react";
import type { FeatureCollection, TemporalPayload } from "../api/client";

export type RGB = [number, number, number];

export type TemporalMode = "trail" | "points" | "both";

export interface LayerStyle {
  color: RGB;
  opacity: number; // 0..1
  pointRadius: number; // px (also the moving-object position-dot radius)
  lineWidth: number; // px (also the moving-object trail width)
  filled: boolean;
  temporalMode: TemporalMode; // how moving objects render
}

export interface Layer {
  id: string;
  name: string;
  command: string;
  geojson: FeatureCollection | null;
  temporal: TemporalPayload | null;
  visible: boolean;
  style: LayerStyle;
}

export interface Selection {
  layerId: string;
  layerName: string;
  properties: Record<string, unknown>;
}

// Categorical palette, assigned in fixed order (never cycled by rank) so a
// layer keeps its colour as others come and go. These are the dark-surface
// steps of a validated categorical theme -- they pass the lightness-band,
// chroma-floor, CVD-separation and contrast checks against a dark chart
// surface, and being mid-dark they also read against the light OSM basemap.
// The one adjacent pair below CVD 12 (green/yellow) is always accompanied by
// secondary encoding: a swatch, the layer name and direct labels.
const PALETTE: RGB[] = [
  [57, 135, 229], // blue    #3987e5
  [25, 158, 112], // aqua    #199e70
  [201, 133, 0], //  yellow  #c98500
  [0, 131, 0], //    green   #008300
  [144, 133, 233], // violet #9085e9
  [230, 103, 103], // red    #e66767
  [213, 81, 129], //  magenta #d55181
  [217, 89, 38], //   orange #d95926
];

function deriveName(command: string): string {
  const s = command.replace(/^\s*query\s+/i, "").trim();
  return s.length > 30 ? s.slice(0, 30) + "…" : s;
}

export function useLayers() {
  const [layers, setLayers] = useState<Layer[]>([]);
  const [selected, setSelected] = useState<Selection | null>(null);
  const idRef = useRef(0);

  const add = useCallback(
    (
      command: string,
      geojson: FeatureCollection | null,
      temporal: TemporalPayload | null
    ) => {
      idRef.current += 1;
      const id = `layer${idRef.current}`;
      setLayers((prev) => {
        const color = PALETTE[prev.length % PALETTE.length];
        const layer: Layer = {
          id,
          name: deriveName(command),
          command,
          geojson,
          temporal,
          visible: true,
          style: {
            color,
            opacity: 0.85,
            pointRadius: 4,
            lineWidth: 1.5,
            filled: true,
            temporalMode: "both",
          },
        };
        return [...prev, layer];
      });
    },
    []
  );

  const remove = useCallback((id: string) => {
    setLayers((prev) => prev.filter((l) => l.id !== id));
    setSelected((s) => (s?.layerId === id ? null : s));
  }, []);

  const toggle = useCallback((id: string) => {
    setLayers((prev) =>
      prev.map((l) => (l.id === id ? { ...l, visible: !l.visible } : l))
    );
  }, []);

  // Move a layer up (toward the top of the draw order) or down.
  const move = useCallback((id: string, dir: "up" | "down") => {
    setLayers((prev) => {
      const i = prev.findIndex((l) => l.id === id);
      if (i < 0) return prev;
      const j = dir === "up" ? i + 1 : i - 1; // higher index = drawn on top
      if (j < 0 || j >= prev.length) return prev;
      const next = [...prev];
      [next[i], next[j]] = [next[j], next[i]];
      return next;
    });
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
    setStyle,
    clear,
    selected,
    setSelected,
  };
}
