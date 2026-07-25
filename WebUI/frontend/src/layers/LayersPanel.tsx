import { useState } from "react";
import type { Layer, LayerStyle, RGB, TemporalMode } from "./useLayers";
import { downloadGeoJSON } from "./exportGeoJSON";
import { labelCandidates } from "./labels";
import { ICON_NAMES, iconPathData, type IconName } from "./icons";

const toHex = ([r, g, b]: RGB): string =>
  "#" + [r, g, b].map((v) => v.toString(16).padStart(2, "0")).join("");

const fromHex = (h: string): RGB => [
  parseInt(h.slice(1, 3), 16),
  parseInt(h.slice(3, 5), 16),
  parseInt(h.slice(5, 7), 16),
];

// The auto-name is the query text, which is rarely what belongs on a legend.
// Renaming lives in the style editor rather than on the row so a click on the
// row keeps its one meaning (open the editor).
//
// The field holds its own draft rather than reading layer.name directly: an
// empty value means "back to the auto-name", so binding the input to the store
// would refill it with query text the moment you cleared it, and you could
// never type a fresh name. The draft is seeded when the editor opens -- it only
// renders for the expanded layer, so switching layers remounts it.
function NameField({
  layer,
  onRename,
}: {
  layer: Layer;
  onRename: (id: string, name: string) => void;
}) {
  const [draft, setDraft] = useState(layer.name);
  return (
    <label>
      name
      <input
        className="lp-rename"
        type="text"
        value={draft}
        placeholder={layer.command}
        onChange={(e) => {
          setDraft(e.target.value);
          onRename(layer.id, e.target.value);
        }}
        onKeyDown={(e) => {
          if (e.key === "Enter") e.currentTarget.blur();
        }}
      />
    </label>
  );
}

interface Props {
  layers: Layer[];
  onToggle: (id: string) => void;
  onRemove: (id: string) => void;
  onMove: (id: string, dir: "up" | "down") => void;
  onRename: (id: string, name: string) => void;
  onStyle: (id: string, patch: Partial<LayerStyle>) => void;
  onClear: () => void;
}

export function LayersPanel({
  layers,
  onToggle,
  onRemove,
  onMove,
  onRename,
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
                  <NameField layer={layer} onRename={onRename} />
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
                  {/* Symbol for point geometry -- colour and icon are the two
                      identity controls, so they sit together. The preview
                      shows the glyph in the layer's own colour, since the
                      names alone are a poor clue as to what it looks like. */}
                  <label className="lp-check">
                    icon
                    <select
                      className="lp-icon"
                      value={layer.style.icon ?? ""}
                      onChange={(e) =>
                        onStyle(layer.id, {
                          icon: (e.target.value || null) as IconName | null,
                        })
                      }
                    >
                      <option value="">circle</option>
                      {ICON_NAMES.map((n) => (
                        <option key={n} value={n}>
                          {n}
                        </option>
                      ))}
                    </select>
                    {layer.style.icon && (
                      <svg
                        className="lp-icon-preview"
                        viewBox="0 0 15 15"
                        style={{ color: toHex(layer.style.color) }}
                        aria-hidden
                      >
                        <path
                          d={iconPathData(layer.style.icon)}
                          fill="currentColor"
                        />
                      </svg>
                    )}
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
                  {/* Write one of the tuple's attributes next to each feature.
                      Off by default; the candidates are ordered so the most
                      label-like attribute is the first one on offer. */}
                  {(() => {
                    const candidates = labelCandidates(
                      layer.geojson,
                      layer.temporal
                    );
                    // An individual object rather than a relation: it carries no
                    // attributes to label with, so offer a caption to type
                    // instead. Still opt-in -- the box starts empty and an empty
                    // box draws nothing, which is what the placeholder says; the
                    // layer's name is not a caption anyone asked for.
                    if (candidates.length === 0) {
                      return (
                        <label className="lp-check">
                          label
                          <input
                            className="lp-label-text"
                            type="text"
                            value={layer.style.labelText ?? ""}
                            placeholder="none"
                            onChange={(e) =>
                              onStyle(layer.id, {
                                labelText: e.target.value || null,
                              })
                            }
                          />
                        </label>
                      );
                    }
                    return (
                      <label className="lp-check">
                        label
                        <select
                          className="lp-label"
                          value={layer.style.label ?? ""}
                          onChange={(e) =>
                            onStyle(layer.id, { label: e.target.value || null })
                          }
                        >
                          <option value="">none</option>
                          {candidates.map((c) => (
                            <option key={c} value={c}>
                              {c}
                            </option>
                          ))}
                        </select>
                      </label>
                    );
                  })()}
                  {/* Trail/positions only apply to moving points. */}
                  {layer.temporal && layer.temporal.trips.length > 0 && (
                    <label className="lp-check">
                      moving
                      <select
                        className="lp-moving"
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
