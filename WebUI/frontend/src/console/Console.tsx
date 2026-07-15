import { useEffect, useRef, useState } from "react";

export interface Entry {
  command: string;
  result?: string;
  error?: string;
  hasGeometry?: boolean;
  hasMotion?: boolean;
}

interface Props {
  history: Entry[];
  busy: boolean;
  openDb: string | null;
  layout: "side" | "bottom";
  collapsed: boolean;
  onToggleCollapse: () => void;
  onToggleLayout: () => void;
  onSubmit: (command: string) => Promise<boolean | void>;
}

export function Console({
  history,
  busy,
  openDb,
  layout,
  collapsed,
  onToggleCollapse,
  onToggleLayout,
  onSubmit,
}: Props) {
  const [command, setCommand] = useState("");
  const [commands, setCommands] = useState<string[]>([]);
  const [histIndex, setHistIndex] = useState(-1);
  const bottom = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    bottom.current?.scrollIntoView({ behavior: "smooth" });
  }, [history]);

  function recall(text: string) {
    setCommand(text);
    requestAnimationFrame(() => {
      const el = inputRef.current;
      if (el) el.setSelectionRange(el.value.length, el.value.length);
    });
  }

  function onKeyDown(e: React.KeyboardEvent<HTMLInputElement>) {
    if (e.key === "ArrowUp") {
      if (commands.length === 0) return;
      e.preventDefault();
      const next =
        histIndex === -1 ? commands.length - 1 : Math.max(0, histIndex - 1);
      setHistIndex(next);
      recall(commands[next]);
    } else if (e.key === "ArrowDown") {
      if (histIndex === -1) return;
      e.preventDefault();
      const next = histIndex + 1;
      if (next >= commands.length) {
        setHistIndex(-1);
        recall("");
      } else {
        setHistIndex(next);
        recall(commands[next]);
      }
    }
  }

  async function submit(cmd: string) {
    const trimmed = cmd.trim();
    if (!trimmed || busy) return;
    setCommand("");
    setHistIndex(-1);
    setCommands((c) => (c[c.length - 1] === trimmed ? c : [...c, trimmed]));
    await onSubmit(trimmed);
    inputRef.current?.focus();
  }

  return (
    <div className={"console" + (collapsed ? " collapsed" : "")}>
      <header>
        <strong>SECONDO</strong>
        {/* The database list lives in the catalog panel; don't duplicate it. */}
        <span className="db">{openDb ? `db: ${openDb}` : "no database open"}</span>
        <button
          className="dock-btn"
          onClick={onToggleCollapse}
          title={collapsed ? "Show query history" : "Hide query history"}
        >
          {collapsed ? "▴ history" : "▾ history"}
        </button>
        <button
          className="dock-btn"
          onClick={onToggleLayout}
          title={
            layout === "side" ? "Dock console to the bottom" : "Dock console to the left"
          }
        >
          {layout === "side" ? "⇩ bottom" : "⇦ left"}
        </button>
      </header>

      <div className="log">
        {history.map((e, i) => (
          <div key={i} className="entry">
            <div className="cmd">
              <span className="prompt">&gt;</span> {e.command}
            </div>
            {e.hasGeometry && <div className="geohint">▸ rendered on map</div>}
            {e.hasMotion && <div className="geohint">▸ animated on timeline</div>}
            {e.result !== undefined && (
              <pre className="ok">
                {e.result.length > 4000
                  ? e.result.slice(0, 4000) + "\n… (truncated)"
                  : e.result}
              </pre>
            )}
            {e.error !== undefined && <pre className="err">{e.error}</pre>}
          </div>
        ))}
        <div ref={bottom} />
      </div>

      <form
        className="input"
        onSubmit={(ev) => {
          ev.preventDefault();
          void submit(command);
        }}
      >
        <span className="prompt">&gt;</span>
        <input
          ref={inputRef}
          autoFocus
          spellCheck={false}
          placeholder="e.g. open database berlintest   |   query mehringdamm   (↑/↓ history)"
          value={command}
          onChange={(e) => setCommand(e.target.value)}
          onKeyDown={onKeyDown}
        />
        <button type="submit" disabled={busy}>
          {busy ? "…" : "Run"}
        </button>
      </form>
    </div>
  );
}
