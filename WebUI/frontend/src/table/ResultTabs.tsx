import { useRef } from "react";

export const MAP_TAB = "map";

export interface ResultTab {
  id: string;
  name: string;
  /** The query that produced it -- provenance, shown as the tooltip. */
  command: string;
}

interface Props {
  tabs: ResultTab[];
  active: string;
  onSelect: (id: string) => void;
  onClose: (id: string) => void;
}

/**
 * The result pane's tab strip: the map, plus one tab per table a query opened.
 *
 * It renders nothing while the map is the only tab. There would be nothing to
 * switch between, and a permanent strip would take height from the map -- which
 * is the product. It appears with the first table and goes away with the last.
 */
export function ResultTabs({ tabs, active, onSelect, onClose }: Props) {
  const strip = useRef<HTMLDivElement>(null);

  if (tabs.length === 0) return null;

  const ids = [MAP_TAB, ...tabs.map((t) => t.id)];

  function onKeyDown(e: React.KeyboardEvent) {
    if (e.key !== "ArrowLeft" && e.key !== "ArrowRight") return;
    e.preventDefault();
    const i = ids.indexOf(active);
    const next = ids[(i + (e.key === "ArrowRight" ? 1 : ids.length - 1)) % ids.length];
    onSelect(next);
    // Move focus with the selection so the arrows keep working.
    requestAnimationFrame(() =>
      strip.current
        ?.querySelector<HTMLButtonElement>(`[data-tab="${next}"]`)
        ?.focus()
    );
  }

  return (
    <div className="result-tabs" role="tablist" ref={strip} onKeyDown={onKeyDown}>
      <button
        role="tab"
        data-tab={MAP_TAB}
        aria-selected={active === MAP_TAB}
        tabIndex={active === MAP_TAB ? 0 : -1}
        className={"rt-tab" + (active === MAP_TAB ? " active" : "")}
        onClick={() => onSelect(MAP_TAB)}
      >
        ◱ Map
      </button>
      {tabs.map((t) => (
        <span key={t.id} className={"rt-tab" + (active === t.id ? " active" : "")}>
          <button
            role="tab"
            data-tab={t.id}
            aria-selected={active === t.id}
            tabIndex={active === t.id ? 0 : -1}
            className="rt-label"
            title={t.command}
            onClick={() => onSelect(t.id)}
          >
            ▤ {t.name}
          </button>
          {/* Closing puts the table away; the result stays a layer and its
              console entry can open it again. */}
          <button
            className="rt-close"
            title="Close this table"
            onClick={() => onClose(t.id)}
          >
            ✕
          </button>
        </span>
      ))}
    </div>
  );
}
