import { useState } from "react";
import { isDrawable, type Layer, type LayerStyle, type RGB, type TemporalMode } from "./useLayers";
import { downloadGeoJSON } from "./exportGeoJSON";
import { labelCandidates, symbolicAttributes } from "./labels";
import { IconPicker } from "./IconPicker";

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
      Name
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
  // Open this result's rows in the table view. Only offered for a layer that
  // has rows -- a single object such as `query train7` is not a relation.
  onShowTable: (id: string) => void;
}

export function LayersPanel({
  layers,
  onToggle,
  onRemove,
  onMove,
  onRename,
  onStyle,
  onClear,
  onShowTable,
}: Props) {
  const [expanded, setExpanded] = useState<string | null>(null);
  const [collapsed, setCollapsed] = useState(false);

  // This panel is the map's legend: visibility, draw order, colour and style.
  // A result that draws nothing (a relation of scalars, e.g. `query ten`) has
  // none of those, so it is not listed here -- its table is opened from the
  // console entry that produced it. `onClear` still clears everything.
  const drawn = layers.filter(isDrawable);
  if (drawn.length === 0) return null;

  // Show topmost draw layer first.
  const ordered = [...drawn].reverse();

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
          <span className="lp-count">{drawn.length}</span>
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
                  aria-label={`Show layer ${layer.name}`}
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
                {layer.table && (
                  <button
                    className="lp-mini"
                    onClick={() => onShowTable(layer.id)}
                    title={`Show ${layer.table.rowCount} rows as a table`}
                    aria-label={`Show ${layer.name} as a table`}
                  >
                    ▤
                  </button>
                )}
                <button
                  className="lp-mini"
                  disabled={isTop}
                  onClick={() => onMove(layer.id, "up")}
                  title="Bring forward"
                  aria-label={`Bring ${layer.name} forward`}
                >
                  ↑
                </button>
                <button
                  className="lp-mini"
                  disabled={isBottom}
                  onClick={() => onMove(layer.id, "down")}
                  title="Send backward"
                  aria-label={`Send ${layer.name} backward`}
                >
                  ↓
                </button>
                <button
                  className="lp-mini lp-x"
                  onClick={() => onRemove(layer.id)}
                  title="Remove layer"
                  aria-label={`Remove layer ${layer.name}`}
                >
                  ✕
                </button>
              </div>

              {open && (
                <div className="lp-style">
                  <NameField layer={layer} onRename={onRename} />
                  <label>
                    Color
                    <input
                      type="color"
                      value={toHex(layer.style.color)}
                      onChange={(e) =>
                        onStyle(layer.id, { color: fromHex(e.target.value) })
                      }
                    />
                  </label>
                  {/* Symbol for point geometry -- colour and icon are the two
                      identity controls, so they sit together. The picker shows
                      the glyphs themselves, in the layer's own colour, since
                      the names alone are a poor clue as to what they are. */}
                  <div className="lp-check lp-check-label">
                    <span className="lp-icon-label">Icon</span>
                    <IconPicker
                      value={layer.style.icon}
                      color={toHex(layer.style.color)}
                      onChange={(icon) => onStyle(layer.id, { icon })}
                    />
                  </div>
                  <label>
                    Opacity
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
                    Point
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
                    Line
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
                    Fill regions
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
                          Label
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
                        Label
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
                  {/* Symbolic trajectories: one line each beside the moving
                      point, all of them by default. Unlike the attribute label
                      this is a set rather than a choice, so it is checkboxes
                      rather than a dropdown; the section only appears when the
                      result actually carries one. */}
                  {(() => {
                    const attrs = symbolicAttributes(layer.temporal);
                    if (attrs.length === 0) return null;
                    const hidden = layer.style.symbolicHidden;
                    return (
                      <div className="lp-symbolic">
                        <div className="lp-symbolic-head">MLabel options</div>
                        {attrs.map((a) => (
                          <label className="lp-check" key={a}>
                            <input
                              type="checkbox"
                              className="lp-symbolic-attr"
                              data-attr={a}
                              checked={!hidden.includes(a)}
                              onChange={(e) =>
                                onStyle(layer.id, {
                                  symbolicHidden: e.target.checked
                                    ? hidden.filter((h) => h !== a)
                                    : [...hidden, a],
                                })
                              }
                            />
                            {a}
                          </label>
                        ))}
                        <label className="lp-check">
                          Show Key Prefix
                          <select
                            className="lp-symbolic-prefix"
                            value={layer.style.symbolicPrefix ? "true" : "false"}
                            onChange={(e) =>
                              onStyle(layer.id, {
                                symbolicPrefix: e.target.value === "true",
                              })
                            }
                          >
                            <option value="false">false</option>
                            <option value="true">true</option>
                          </select>
                        </label>
                      </div>
                    );
                  })()}
                  {/* Trail/positions only apply to moving points. */}
                  {layer.temporal && layer.temporal.trips.length > 0 && (
                    <label className="lp-check">
                      Show as
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
                        {/* The stored mode stays `points`; only the word
                            shown changes. */}
                        <option value="points">point</option>
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
