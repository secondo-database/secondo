// Colour theme: dark (default) or a light grey one, chosen by the user and
// remembered per browser. The palette itself lives in styles.css; all this does
// is set `data-theme` on <html>, which selects the light token block.

export type Theme = "dark" | "light";

const THEME_KEY = "secondo.webui.theme";

/** The remembered choice, or -- on a first visit -- whatever the OS asks for.
 *  Once the toggle has been used the stored value wins, so the preference is
 *  only ever a starting point. */
export function loadTheme(): Theme {
  try {
    const raw = localStorage.getItem(THEME_KEY);
    if (raw === "dark" || raw === "light") return raw;
  } catch {
    /* storage unavailable */
  }
  return window.matchMedia?.("(prefers-color-scheme: light)").matches
    ? "light"
    : "dark";
}

/** Paint in `theme` and remember the choice. */
export function applyTheme(theme: Theme): void {
  document.documentElement.dataset.theme = theme;
  try {
    localStorage.setItem(THEME_KEY, theme);
  } catch {
    /* storage unavailable */
  }
}

/** Paint the remembered theme before React's first render, so a light-mode
 *  reload never flashes the dark palette. */
export function initTheme(): void {
  document.documentElement.dataset.theme = loadTheme();
}
