// The command history the console recalls with ↑/↓, kept in localStorage next
// to the layout and theme preferences so it survives a browser reload — a
// history file, as SecondoTTY keeps one in `.secondo_history`.
//
// Commands only: results belong to the session that ran them (their geometry is
// on the map, which a reload drops anyway), so nothing of an answer is stored.

const KEY = "secondo.webui.history";
// Deep enough to scroll back through a working session, bounded so a long-lived
// browser profile cannot grow it without limit.
const MAX_COMMANDS = 200;

/** The remembered commands, oldest first; empty if there are none. */
export function loadCommands(): string[] {
  try {
    const raw = localStorage.getItem(KEY);
    if (!raw) return [];
    const parsed: unknown = JSON.parse(raw);
    if (!Array.isArray(parsed)) return [];
    return parsed.filter((c): c is string => typeof c === "string").slice(-MAX_COMMANDS);
  } catch {
    // Storage unavailable, or a format written by an older build.
    return [];
  }
}

/** Persist `commands`; an empty list is how the history is forgotten. */
export function saveCommands(commands: string[]): void {
  try {
    localStorage.setItem(KEY, JSON.stringify(commands.slice(-MAX_COMMANDS)));
  } catch {
    /* storage unavailable or full */
  }
}
