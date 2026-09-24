import { useEffect, useMemo, useRef, useState } from "react";
import { letCommand, type Shape } from "./secondoValue";
import type { StepOutcome } from "../../catalog/GpxImportDialog";

// Store a shape drawn on the map as a named object, the way HoeseViewer's
// "Object Creation" does with `let <name> = [const <type> value ...]`. Unlike
// there, the name is checked against the catalog before anything is sent, so a
// clash is a message here rather than a server error afterwards.

interface Props {
  shape: Shape;
  database: string;
  existingNames: string[];
  runStep: (command: string) => Promise<StepOutcome>;
  /** Called with the stored object's name, or null when nothing was stored. */
  onClose: (saved: string | null) => void;
}

// What the backend itself accepts as an object name (main.py _OBJECT_NAME).
const NAME_RE = /^[A-Za-z]\w*$/;
const MAX_NAME = 48;

/** `region1`, `region2`, ... -- the first one the database does not have. */
export function freeName(type: string, taken: Set<string>): string {
  for (let i = 1; ; i++) if (!taken.has(`${type}${i}`)) return `${type}${i}`;
}

export function SaveShapeDialog({ shape, database, existingNames, runStep, onClose }: Props) {
  // Compared case-insensitively: a name that differs from another only by its
  // case is a trap in a catalog listed alphabetically.
  const taken = useMemo(
    () => new Set(existingNames.map((n) => n.toLowerCase())),
    [existingNames]
  );
  const [name, setName] = useState(() => freeName(shape.type, taken));
  const [saving, setSaving] = useState(false);
  const [failure, setFailure] = useState<string | null>(null);
  const inputRef = useRef<HTMLInputElement>(null);

  const problem = useMemo(() => {
    if (!name) return "Give the object a name.";
    if (name.length > MAX_NAME) return `At most ${MAX_NAME} characters.`;
    if (!NAME_RE.test(name))
      return "Letters, digits and underscores only, starting with a letter.";
    if (taken.has(name.toLowerCase())) return `${name} already exists in ${database}.`;
    return null;
  }, [name, taken, database]);

  const command = letCommand(name || "…", shape);

  useEffect(() => {
    inputRef.current?.focus();
    inputRef.current?.select();
  }, []);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape" && !saving) {
        e.stopPropagation();
        onClose(null);
      }
    };
    window.addEventListener("keydown", onKey, true);
    return () => window.removeEventListener("keydown", onKey, true);
  });

  async function save() {
    if (problem || saving) return;
    setSaving(true);
    setFailure(null);
    const outcome = await runStep(letCommand(name, shape));
    setSaving(false);
    if (outcome.ok) onClose(name);
    else setFailure(outcome.error ?? "The command failed.");
  }

  return (
    <div
      className="gpx-backdrop"
      onMouseDown={(e) => {
        if (e.target === e.currentTarget && !saving) onClose(null);
      }}
    >
      <div
        className="gpx-dialog save-shape"
        role="dialog"
        aria-modal="true"
        aria-labelledby="shape-title"
      >
        <h2 className="gpx-title" id="shape-title">
          Save {shape.type} as object
        </h2>
        <label className="gpx-label" htmlFor="shape-name">
          Name in {database}
        </label>
        <input
          id="shape-name"
          ref={inputRef}
          className="gpx-name"
          value={name}
          spellCheck={false}
          autoComplete="off"
          disabled={saving}
          onChange={(e) => setName(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter") void save();
          }}
        />
        {problem && <p className="gpx-note err-text">{problem}</p>}
        <details className="gpx-cmds">
          <summary>Command</summary>
          <code className="shape-cmd">{command}</code>
        </details>
        {failure && <pre className="err">{failure}</pre>}
        <div className="gpx-actions">
          <button
            className="gpx-import"
            disabled={!!problem || saving}
            onClick={() => void save()}
          >
            {saving ? "saving…" : "Save"}
          </button>
          <button className="gpx-close" disabled={saving} onClick={() => onClose(null)}>
            Discard
          </button>
        </div>
      </div>
    </div>
  );
}
