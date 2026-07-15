import { useState } from "react";
import type { Layer, LayerStyle, RGB, TemporalMode } from "./useLayers";
import { downloadGeoJSON } from "./exportGeoJSON";

const toHex = ([r, g, b]: RGB): string =>
  "#" + [r, g, b].map((v) => v.toString(16).padStart(2, "0")).join("");

const fromHex = (h: string): RGB => [
  parseInt(h.slice(1, 3), 16),
  parseInt(h.slice(3, 5), 16),
  parseInt(h.slice(5, 7), 16),
];

interface Props {
  layers: Layer[];
  onToggle: (id: string) => void;
  onRemove: (id: string) => void;
  onMove: (id: string, dir: "up" | "down") => void;
  onStyle: (id: string, patch: Partial<LayerStyle>) => void;
  onClear: () => void;
}

export function LayersPanel({
  layers,
  onToggle,
  onRemove,
  onMove,
  onStyle,
  onClear,
}: Props) {
  const [expanded, setExpanded] = useState<string | null>(null);
  const [collapsed, setCollapsed] = useState(false);

  if (layers.length === 0) return null;

  // Show topmost draw layer first.
  const ordered = [...layers].reverse();

  return (
    <div className="layers-panel">
      <div className="lp-head">
        <button
          className="lp-collapse"
          onClick={() => setCollapsed((c) => !c)}
          title={collapsed ? "Show layers" : "Hide layers"}
          aria-expanded={!collapsed}
        >
          <span className="lp-chevron">{collapsed ? "▸" : "▾"}</span>
          Layers
          <span className="lp-count">{layers.length}</span>
        </button>
        <span className={"lp-actions" + (collapsed ? " lp-hidden" : "")}>
          <button
            className="lp-clear"
            onClick={() => downloadGeoJSON(layers)}
            title="Export visible layers as GeoJSON"
          >
            export
          </button>
          <button className="lp-clear" onClick={onClear} title="Remove all layers">
            clear
          </button>
        </span>
      </div>
      {!collapsed && (
      <ul className="lp-list">
        {ordered.map((layer, idx) => {
          const isTop = idx === 0;
          const isBottom = idx === ordered.length - 1;
          const open = expanded === layer.id;
          return (
            <li key={layer.id} className="lp-item">
              <div className="lp-row">
                <input
                  type="checkbox"
                  checked={layer.visible}
                  onChange={() => onToggle(layer.id)}
                  title="Toggle visibility"
                />
                <span
                  className="lp-swatch"
                  style={{ background: toHex(layer.style.color) }}
                />
                <button
                  className="lp-name"
                  title={layer.command}
                  onClick={() => setExpanded(open ? null : layer.id)}
                >
                  {layer.name}
                  {layer.temporal ? " ◷" : ""}
                </button>
                <button
                  className="lp-mini"
                  disabled={isTop}
                  onClick={() => onMove(layer.id, "up")}
                  title="Bring forward"
                >
                  ↑
                </button>
                <button
                  className="lp-mini"
                  disabled={isBottom}
                  onClick={() => onMove(layer.id, "down")}
                  title="Send backward"
                >
                  ↓
                </button>
                <button
                  className="lp-mini lp-x"
                  onClick={() => onRemove(layer.id)}
                  title="Remove layer"
                >
                  ✕
                </button>
              </div>

              {open && (
                <div className="lp-style">
                  <label>
                    color
                    <input
                      type="color"
                      value={toHex(layer.style.color)}
                      onChange={(e) =>
                        onStyle(layer.id, { color: fromHex(e.target.value) })
                      }
                    />
                  </label>
                  <label>
                    opacity
                    <input
                      type="range"
                      min={0.1}
                      max={1}
                      step={0.05}
                      value={layer.style.opacity}
                      onChange={(e) =>
                        onStyle(layer.id, { opacity: Number(e.target.value) })
                      }
                    />
                  </label>
                  <label>
                    point
                    <input
                      type="range"
                      min={1}
                      max={14}
                      step={1}
                      value={layer.style.pointRadius}
                      onChange={(e) =>
                        onStyle(layer.id, { pointRadius: Number(e.target.value) })
                      }
                    />
                  </label>
                  <label>
                    line
                    <input
                      type="range"
                      min={0.5}
                      max={8}
                      step={0.5}
                      value={layer.style.lineWidth}
                      onChange={(e) =>
                        onStyle(layer.id, { lineWidth: Number(e.target.value) })
                      }
                    />
                  </label>
                  <label className="lp-check">
                    <input
                      type="checkbox"
                      checked={layer.style.filled}
                      onChange={(e) =>
                        onStyle(layer.id, { filled: e.target.checked })
                      }
                    />
                    fill regions
                  </label>
                  {/* Trail/positions only apply to moving points. */}
                  {layer.temporal && layer.temporal.trips.length > 0 && (
                    <label className="lp-check">
                      moving
                      <select
                        value={layer.style.temporalMode}
                        onChange={(e) =>
                          onStyle(layer.id, {
                            temporalMode: e.target.value as TemporalMode,
                          })
                        }
                      >
                        <option value="trail">trail</option>
                        <option value="points">positions</option>
                        <option value="both">both</option>
                      </select>
                    </label>
                  )}
                </div>
              )}
            </li>
          );
        })}
      </ul>
      )}
    </div>
  );
}
