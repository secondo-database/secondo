import { useState } from "react";
import { ICON_NAMES, iconPathData, type IconName } from "./icons";

// The point-symbol picker. A native <select> cannot draw anything but text, and
// "rail-metro" or "monument" is a poor description of a 15px glyph, so this is a
// grid of the glyphs themselves: click the trigger, click a symbol. The grid
// expands in flow rather than floating over the panel -- the layer list is a
// scroll container with `overflow: hidden` on its panel, which would clip an
// absolutely positioned menu.
//
// Every glyph is drawn in the layer's own colour, so the choice is made against
// the same appearance the map will have.

const CIRCLE_LABEL = "circle";

// null is the default point symbol: deck's own disc, drawn here as one.
function Glyph({ name }: { name: IconName | null }) {
  return (
    <svg className="lp-glyph" viewBox="0 0 15 15" aria-hidden>
      {name ? (
        <path d={iconPathData(name)} fill="currentColor" />
      ) : (
        <circle cx="7.5" cy="7.5" r="4.5" fill="currentColor" />
      )}
    </svg>
  );
}

const OPTIONS: (IconName | null)[] = [null, ...ICON_NAMES];

interface Props {
  value: IconName | null;
  // The layer's colour, as a CSS hex string.
  color: string;
  onChange: (icon: IconName | null) => void;
}

export function IconPicker({ value, color, onChange }: Props) {
  const [open, setOpen] = useState(false);
  // `undefined` means "nothing hovered", which is distinct from `null` -- that
  // is the circle. The caption falls back to the current selection.
  const [hover, setHover] = useState<IconName | null | undefined>(undefined);
  const captioned = hover === undefined ? value : hover;

  return (
    <div
      className="lp-iconpick"
      onKeyDown={(e) => {
        if (e.key === "Escape" && open) {
          e.stopPropagation();
          setOpen(false);
        }
      }}
    >
      <button
        className="lp-icon"
        data-value={value ?? ""}
        aria-expanded={open}
        title="Choose the point symbol"
        onClick={() => setOpen((o) => !o)}
      >
        <span className="lp-glyph-box" style={{ color }}>
          <Glyph name={value} />
        </span>
        <span className="lp-icon-name">{value ?? CIRCLE_LABEL}</span>
        <span className="lp-icon-caret">{open ? "▾" : "▸"}</span>
      </button>
      {open && (
        <div className="lp-icon-menu">
          <div
            className="lp-icon-grid"
            role="listbox"
            aria-label="Point symbol"
            onMouseLeave={() => setHover(undefined)}
          >
            {OPTIONS.map((name) => (
              <button
                key={name ?? CIRCLE_LABEL}
                role="option"
                aria-selected={name === value}
                aria-label={name ?? CIRCLE_LABEL}
                className={"lp-icon-cell" + (name === value ? " lp-on" : "")}
                data-icon={name ?? ""}
                title={name ?? CIRCLE_LABEL}
                style={{ color }}
                onMouseEnter={() => setHover(name)}
                onFocus={() => setHover(name)}
                onClick={() => {
                  onChange(name);
                  setOpen(false);
                }}
              >
                <Glyph name={name} />
              </button>
            ))}
          </div>
          {/* The grid alone is 41 unlabelled glyphs; this names whichever one
              the pointer or focus is on, so the set stays browsable without
              waiting on a tooltip. */}
          <div className="lp-icon-caption">{captioned ?? CIRCLE_LABEL}</div>
        </div>
      )}
    </div>
  );
}
