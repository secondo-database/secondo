import { useEffect, useRef, useState } from "react";
import type { CatalogObject } from "../api/client";
import type { Theme } from "../theme";
import {
  applyCompletion,
  completionsFor,
  type Completion,
} from "./completion";
import { loadCommands, saveCommands } from "./history";

/** What a one-shot Run menu item asks for; `undefined` is a plain Run.
 *
 *  Only "explain" remains. The menu is about *how to run*, not about where the
 *  result goes: routing is better decided once the answer is back, which is why
 *  it lives on the console entry and the layers row instead. ("Run and show on
 *  map" could not even keep its promise -- for a result with no geometry there
 *  is nothing to show, so it silently opened the table instead.) */
export type RunIntent = "explain";

export interface Entry {
  command: string;
  result?: string;
  error?: string;
  hasGeometry?: boolean;
  hasMotion?: boolean;
  // The result this entry produced, so its hints can bring it back up.
  layerId?: string;
  // Rows, when the result was a relation.
  rowCount?: number;
  // The executable plan the optimizer generated for an SQL command, and what it
  // thinks that plan costs.
  plan?: string;
  costs?: number;
  // What an optimizer directive (showOptions, setOption, ...) printed.
  message?: string;
  // The user wrote the "optimizer " prefix: optimized, deliberately not run.
  planOnly?: boolean;
  // A create/drop the optimizer carried out itself while translating.
  executedByOptimizer?: boolean;
  // How long the round trip took, bridge and conversion included.
  elapsedMs?: number;
}

/** Elapsed time, at the precision that reads as a duration rather than as a
 *  measurement: sub-second in milliseconds, past that in seconds. */
function fmtElapsed(ms: number): string {
  return ms < 1000 ? `${Math.round(ms)} ms` : `${(ms / 1000).toFixed(2)} s`;
}

interface Props {
  history: Entry[];
  busy: boolean;
  openDb: string | null;
  // Whether this server can run SQL; null until the session state is known.
  optimizer: boolean | null;
  collapsed: boolean;
  theme: Theme;
  /** The open database's objects, for completing names as they are typed. */
  objects: CatalogObject[];
  onToggleCollapse: () => void;
  onToggleTheme: () => void;
  onClearHistory: () => void;
  onSubmit: (command: string, intent?: RunIntent) => Promise<boolean | void>;
  /** Bring a past result back up in the result pane. */
  onShowResult: (layerId: string, target: "map" | "table") => void;
}

export function Console({
  history,
  busy,
  openDb,
  optimizer,
  collapsed,
  theme,
  objects,
  onToggleCollapse,
  onToggleTheme,
  onClearHistory,
  onSubmit,
  onShowResult,
}: Props) {
  const [command, setCommand] = useState("");
  // What ↑/↓ walks through, seeded with what earlier sessions typed.
  const [commands, setCommands] = useState<string[]>(loadCommands);
  const [histIndex, setHistIndex] = useState(-1);
  const [menuOpen, setMenuOpen] = useState(false);
  // What the word under the caret could become, and which of those Tab would
  // take. The first is marked from the start, so it is never a guess which one
  // Tab means. `moved` records whether that mark was *chosen* with the arrows:
  // until it is, Enter still runs the query, so the primary action is never
  // hijacked by a suggestion nobody asked for.
  const [items, setItems] = useState<Completion[]>([]);
  const [picked, setPicked] = useState(0);
  const [moved, setMoved] = useState(false);
  const bottom = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLTextAreaElement>(null);
  const runRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    // Smooth only when motion is welcome; the jump still lands in the same
    // place for everyone else.
    const reduced = window.matchMedia?.("(prefers-reduced-motion: reduce)").matches;
    bottom.current?.scrollIntoView({ behavior: reduced ? "auto" : "smooth" });
  }, [history]);

  // A long query has to be readable while it is written, so the box is sized to
  // its content instead of scrolling sideways in a one-line slit. Reset to
  // `auto` first, or scrollHeight -- which never reports less than the current
  // height -- would let it grow but never shrink back. CSS caps the height and
  // takes over with a scrollbar past that. Driven from the value rather than
  // from onInput so history recall and the clear after a submit resize too.
  // (`field-sizing: content` would do this in CSS alone, but no Firefox or
  // Safari release supports it yet.)
  useEffect(() => {
    const el = inputRef.current;
    if (!el) return;
    const top = el.scrollTop;
    el.style.height = "auto";
    el.style.height = `${el.scrollHeight}px`;
    // Measuring against `auto` scrolls the box back to the top, which would
    // hide what is being typed once the query is past the cap. Follow the caret
    // when it is at the end -- the usual case, still typing -- and otherwise
    // leave the view where it was.
    el.scrollTop =
      el.selectionStart === el.value.length ? el.scrollHeight : top;
  }, [command]);

  // The recalled commands outlive the tab, so every change is written through.
  useEffect(() => {
    saveCommands(commands);
  }, [commands]);

  // The Run menu closes on Esc and on a click anywhere else, as a menu should.
  useEffect(() => {
    if (!menuOpen) return;
    const close = (e: MouseEvent) => {
      if (!runRef.current?.contains(e.target as Node)) setMenuOpen(false);
    };
    const onEsc = (e: KeyboardEvent) => {
      if (e.key === "Escape") setMenuOpen(false);
    };
    window.addEventListener("mousedown", close);
    window.addEventListener("keydown", onEsc);
    return () => {
      window.removeEventListener("mousedown", close);
      window.removeEventListener("keydown", onEsc);
    };
  }, [menuOpen]);

  // Empty the log and forget the recalled commands; the effect above writes the
  // empty list through, which is what clears them in storage too.
  function forget() {
    setCommands([]);
    setHistIndex(-1);
    onClearHistory();
  }

  function recall(text: string) {
    setCommand(text);
    closeCompletions();
    requestAnimationFrame(() => {
      const el = inputRef.current;
      if (el) el.setSelectionRange(el.value.length, el.value.length);
    });
  }

  function closeCompletions() {
    setItems([]);
    setPicked(0);
    setMoved(false);
  }

  /** Recompute the offer from the word under the caret. */
  function refreshCompletions(el: HTMLTextAreaElement) {
    setItems(completionsFor(el.value.slice(0, el.selectionStart), objects));
    setPicked(0);
    setMoved(false);
  }

  /** Put `choice` in place of the word under the caret. */
  function accept(choice: Completion) {
    const el = inputRef.current;
    if (!el) return;
    const next = applyCompletion(el.value, el.selectionStart, choice);
    setCommand(next.value);
    closeCompletions();
    requestAnimationFrame(() => {
      inputRef.current?.setSelectionRange(next.caret, next.caret);
      inputRef.current?.focus();
    });
  }

  function onKeyDown(e: React.KeyboardEvent<HTMLTextAreaElement>) {
    const el = e.currentTarget;

    // The completion list gets first refusal on these keys, but only over what
    // it needs: Tab and the arrows while it is open, Enter *only* once
    // something has been picked from it. Everything else -- and every key when
    // it is closed -- falls through to the behaviour the box always had.
    if (items.length > 0) {
      if (e.key === "Escape") {
        e.preventDefault();
        closeCompletions();
        return;
      }
      if (e.key === "Tab") {
        e.preventDefault();
        accept(items[picked]);
        return;
      }
      if (e.key === "Enter" && moved && !e.shiftKey && !e.altKey) {
        e.preventDefault();
        accept(items[picked]);
        return;
      }
      if (e.key === "ArrowDown") {
        e.preventDefault();
        // The first press confirms the mark that is already on the first item
        // rather than skipping past it.
        setPicked((i) => (moved ? (i + 1) % items.length : i));
        setMoved(true);
        return;
      }
      if (e.key === "ArrowUp" && moved) {
        e.preventDefault();
        setPicked((i) => (i <= 0 ? items.length - 1 : i - 1));
        return;
      }
    }

    if (e.key === "Enter" && !e.shiftKey && !e.altKey) {
      // Enter still runs the query, as it did when this was a one-line input;
      // Shift+Enter (and Alt+Enter) is what breaks a line. Ctrl/Cmd+Enter is
      // the habit from other editors and submits as well.
      e.preventDefault();
      void submit(command);
    } else if (e.key === "ArrowUp") {
      // In a box that can hold several lines the arrows have to move the caret
      // between them first, so recall only takes over once there is no line to
      // move to -- the way a shell's does. A one-line query is always on both
      // its first and its last line, so for those nothing has changed.
      if (commands.length === 0) return;
      if (el.value.slice(0, el.selectionStart).includes("\n")) return;
      e.preventDefault();
      const next =
        histIndex === -1 ? commands.length - 1 : Math.max(0, histIndex - 1);
      setHistIndex(next);
      recall(commands[next]);
    } else if (e.key === "ArrowDown") {
      if (histIndex === -1) return;
      if (el.value.slice(el.selectionEnd).includes("\n")) return;
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

  async function submit(cmd: string, intent?: RunIntent) {
    const trimmed = cmd.trim();
    if (!trimmed || busy) return;
    setCommand("");
    setHistIndex(-1);
    setMenuOpen(false);
    closeCompletions();
    // Recall keeps the query as it was written -- the line breaks are what make
    // a long one readable -- while the server and the log see a single line.
    // Only the breaks and the indentation around them go: spacing inside a line
    // is left alone, since it may be inside a string literal the query compares
    // against.
    setCommands((c) => (c[c.length - 1] === trimmed ? c : [...c, trimmed]));
    await onSubmit(trimmed.replace(/\s*\n\s*/g, " "), intent);
    inputRef.current?.focus();
  }

  return (
    <div className={"console" + (collapsed ? " collapsed" : "")}>
      <header>
        <strong>SECONDO</strong>
        {/* The database list lives in the catalog panel; don't duplicate it. */}
        <span className="db">{openDb ? `db: ${openDb}` : "no database open"}</span>
        {/* Only worth saying when SQL is *not* on offer, so that a failing
            `select ...` explains itself instead of looking like a bug. */}
        {optimizer === false && (
          <span className="db" title="This server runs without the optimizer">
            sql: off
          </span>
        )}
        <button
          className="dock-btn first"
          onClick={onToggleCollapse}
          aria-expanded={!collapsed}
          title={collapsed ? "Show query history" : "Hide query history"}
        >
          <span className="dock-ic">{collapsed ? "▴" : "▾"}</span> history
        </button>
        {/* The recalled commands outlive the tab, so they need a way back to
            empty; the log on screen goes with them. */}
        <button
          className="dock-btn"
          onClick={forget}
          disabled={history.length === 0 && commands.length === 0}
          title="Clear the log and the remembered commands"
        >
          <span className="dock-ic">⌫</span> clear
        </button>
        <button
          className="dock-btn"
          onClick={onToggleTheme}
          title={theme === "dark" ? "Switch to light theme" : "Switch to dark theme"}
        >
          <span className="dock-ic">{theme === "dark" ? "☀" : "☾"}</span>{" "}
          {theme === "dark" ? "light" : "dark"}
        </button>
      </header>

      {/* A result -- and an error -- has to reach someone who is not watching
          the pane. `additions` keeps it to the new entry rather than re-reading
          the whole log every time. */}
      <div className="log" role="log" aria-live="polite" aria-relevant="additions">
        {history.map((e, i) => (
          <div key={i} className="entry">
            <div className="cmd">
              <span className="prompt">&gt;</span>
              <span className="cmd-text">{e.command}</span>
              {/* How long the answer took -- the first thing anyone asks of a
                  database, and nothing said it before. */}
              {e.elapsedMs !== undefined && (
                <span
                  className="cmd-ms"
                  title="Time from sending the command to having the result, bridge included"
                >
                  {fmtElapsed(e.elapsedMs)}
                </span>
              )}
            </div>
            {e.plan !== undefined && (
              <pre className="plan">
                {`Optimized plan: ${e.plan}` +
                  // Costs are only meaningful when the optimizer estimated
                  // any; the Java GUI hides them otherwise too.
                  (e.costs !== undefined && e.costs > 0
                    ? `\nEstimated costs: ${e.costs}`
                    : "")}
              </pre>
            )}
            {e.planOnly && <div className="optnote">Plan only — not executed.</div>}
            {e.executedByOptimizer && (
              <div className="optnote">
                Executed by the optimizer (no plan to run).
              </div>
            )}
            {/* The hints are the way back to a result: this is where the query
                was typed, so this is where you decide what to look at. */}
            {(e.hasGeometry || e.hasMotion || e.rowCount !== undefined) && (
              <div className="geohint">
                {e.hasGeometry && (
                  <button onClick={() => e.layerId && onShowResult(e.layerId, "map")}>
                    ▸ rendered on map
                  </button>
                )}
                {e.hasMotion && (
                  <button onClick={() => e.layerId && onShowResult(e.layerId, "map")}>
                    ▸ animated on timeline
                  </button>
                )}
                {e.rowCount !== undefined && (
                  <button
                    onClick={() => e.layerId && onShowResult(e.layerId, "table")}
                    title="Open this result as a table"
                  >
                    ▤ {e.rowCount} {e.rowCount === 1 ? "row" : "rows"} — show as table
                  </button>
                )}
              </div>
            )}
            {/* An optimizer directive prints its own text and has no result;
                showOptions lays it out with leading whitespace, which is why
                it is not trimmed anywhere along the way. */}
            {e.message !== undefined && <pre className="ok">{e.message}</pre>}
            {e.result !== undefined &&
              e.message === undefined &&
              !e.planOnly &&
              !e.executedByOptimizer && (
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
        <textarea
          ref={inputRef}
          rows={1}
          autoFocus
          spellCheck={false}
          // Short enough to fit the box at any width: it used to wrap to two
          // lines in a one-line box and get clipped. The examples now live in
          // the empty-state card, where there is room for them.
          placeholder="query, select …   ⇧⏎ newline · ↑↓ history · ⇥ complete"
          value={command}
          onChange={(e) => {
            setCommand(e.target.value);
            refreshCompletions(e.target);
          }}
          onBlur={closeCompletions}
          onKeyDown={onKeyDown}
          aria-autocomplete="list"
          aria-expanded={items.length > 0}
        />

        {items.length > 0 && (
          <ul className="cmp-menu" role="listbox" aria-label="Completions">
            {items.map((c, i) => (
              <li key={`${c.text}-${c.hint}`}>
                <button
                  type="button"
                  role="option"
                  aria-selected={i === picked}
                  className={"cmp-item" + (i === picked ? " picked" : "")}
                  // The blur that a click would cause closes the list before
                  // the click lands, so act on mousedown instead.
                  onMouseDown={(ev) => {
                    ev.preventDefault();
                    accept(c);
                  }}
                >
                  <span className="cmp-name">{c.text}</span>
                  <span className="cmp-hint">{c.hint}</span>
                  {/* Says which key takes this one, on the one it would take. */}
                  {i === picked && <span className="cmp-key">⇥</span>}
                </button>
              </li>
            ))}
            <li className="cmp-help">⇥ complete · ↑↓ pick · esc dismiss</li>
          </ul>
        )}
        {/* A split button: `Run` is unchanged, and the menu holds the ways of
            running that are not plain execution. Nothing here is remembered. */}
        <div className="run-split" ref={runRef}>
          <button type="submit" className="run-go" disabled={busy}>
            {busy ? "…" : "Run"}
          </button>
          <button
            type="button"
            className="run-more"
            disabled={busy}
            aria-haspopup="menu"
            aria-expanded={menuOpen}
            title="Other ways to run this"
            onClick={() => setMenuOpen((o) => !o)}
          >
            ▾
          </button>
          {menuOpen && (
            <div className="run-menu" role="menu">
              <button
                role="menuitem"
                disabled={optimizer === false}
                title={
                  optimizer === false
                    ? "This server runs without the optimizer"
                    : "Optimize the query but do not run it"
                }
                onClick={() => void submit(command, "explain")}
              >
                <span className="run-ic">≡</span> Explain — plan only
              </button>
            </div>
          )}
        </div>
      </form>
    </div>
  );
}
