// What the console log shows by default, kept in localStorage next to the
// command history and the theme so a working preference outlives a reload.
//
// Two blocks of an entry can be put away: the optimizer's plan and the result's
// nested list. Both start shown — turning one off is a choice, so it is never
// made for anyone — and both stay per-entry overridable in Console.tsx, so this
// only ever sets the default.

const KEY = "secondo.webui.console";

export interface ConsolePrefs {
  /** Show the executable plan the optimizer generated for an SQL command. */
  plan: boolean;
  /** Show the result as the nested list the server sent. */
  result: boolean;
}

export const DEFAULT_PREFS: ConsolePrefs = { plan: true, result: true };

/** The remembered choice, or the defaults. */
export function loadPrefs(): ConsolePrefs {
  try {
    const raw = localStorage.getItem(KEY);
    if (!raw) return DEFAULT_PREFS;
    const parsed: unknown = JSON.parse(raw);
    if (!parsed || typeof parsed !== "object") return DEFAULT_PREFS;
    const p = parsed as Partial<Record<keyof ConsolePrefs, unknown>>;
    // Each key on its own, so a value written by an older build — or a missing
    // one added by a later one — falls back to the default rather than
    // discarding the half that is still good.
    return {
      plan: typeof p.plan === "boolean" ? p.plan : DEFAULT_PREFS.plan,
      result: typeof p.result === "boolean" ? p.result : DEFAULT_PREFS.result,
    };
  } catch {
    // Storage unavailable, or a format written by an older build.
    return DEFAULT_PREFS;
  }
}

/** Persist the choice. */
export function savePrefs(prefs: ConsolePrefs): void {
  try {
    localStorage.setItem(KEY, JSON.stringify(prefs));
  } catch {
    /* storage unavailable or full */
  }
}
