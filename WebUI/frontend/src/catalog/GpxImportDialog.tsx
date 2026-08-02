import { useEffect, useMemo, useRef, useState } from "react";

// Import a dropped GPX file into the open database.
//
// The four commands below are the pipeline from bin/Scripts/ReadingASetOfGPXFiles.sec,
// cut down to a single file: gpximport reads the track, approximate turns the
// timestamped points into one moving point, and trajectory/bbox derive what the
// map and a spatial index want. They run one after the other on the user's own
// session -- not server-side in a batch -- so the optimizer's catalog is
// refreshed after each `let` (Session._update_catalog_if_wanted) and every
// command is left in the console, where it can be read and run again.

/** What one step of the import is doing. */
type StepState = "pending" | "running" | "done" | "error";

interface Step {
  /** Appended to the chosen name to get the object this step creates. */
  suffix: string;
  /** What that object *is* -- the answer to "why are there four of them?". */
  kind: string;
  /** What the step is doing, shown in place of the kind while it runs. */
  label: string;
  command: (name: string, path: string) => string;
}

/** The result of running one command; the message is SECONDO's own. */
export interface StepOutcome {
  ok: boolean;
  error?: string;
}

interface Props {
  /** The dropped file. Only its name is used here; the bytes are already up. */
  file: File;
  /** Where the bridge stored it, or null while the upload is still running. */
  path: string | null;
  /** Why the upload failed, if it did. */
  uploadError?: string;
  /** The open database, named in the collision messages. */
  database: string;
  /** Every object already in that database, so a name can be refused before
   *  the first command runs rather than halfway through. */
  existingNames: string[];
  /** Run one command; the caller logs it to the console. */
  runStep: (command: string) => Promise<StepOutcome>;
  /** Called when the dialog is closed, with the objects that now exist. */
  onClose: (created: string[]) => void;
}

// A SECONDO object name as this dialog insists on writing it: lowercase, no
// whitespace, nothing that needs quoting. The kernel would take more (the
// backend's own check is /[A-Za-z]\w*/), but a name that is only distinguished
// from another by its case is a trap in a catalog listed alphabetically.
const NAME_RE = /^[a-z][a-z0-9_]*$/;
// Leaves room for the longest suffix (`_trajectory`) within SECONDO's limit.
const MAX_NAME = 40;

/** The import, in order: what each step creates, what that object is, and the
 *  command that makes it. One list, so the preview shown while the name is
 *  being chosen and the progress shown while it runs cannot drift apart -- and
 *  so the name check below covers exactly the objects that will be written. */
const PLAN: Step[] = [
  {
    suffix: "",
    kind: "raw GPX import",
    label: "importing GPX file",
    command: (name, path) => `let ${name} = gpximport('${path}') consume`,
  },
  {
    suffix: "_mp",
    kind: "moving point",
    label: "calculating moving point",
    command: (name) =>
      `let ${name}_mp = ${name} feed` +
      ` projectextend[; T: .Time, P: makepoint(.Lon, .Lat)]` +
      ` approximate[T, P]`,
  },
  {
    suffix: "_trajectory",
    kind: "trajectory",
    label: "calculating trajectory",
    command: (name) => `let ${name}_trajectory = trajectory(${name}_mp)`,
  },
  {
    suffix: "_bbox",
    kind: "bounding box",
    label: "calculating bounding box",
    command: (name) => `let ${name}_bbox = bbox(${name}_trajectory)`,
  },
];

/** Propose a name from the filename: `2026-07-26_Wanderung.gpx` becomes
 *  `gpx_2026_07_26_wanderung`. Runs of anything else collapse to one
 *  underscore, and a name that would start with a digit gets a prefix -- a
 *  SECONDO identifier has to start with a letter. */
export function defaultName(filename: string): string {
  const stem = filename.replace(/\.[^.]*$/, "");
  let name = stem
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, "_")
    .replace(/^_+|_+$/g, "");
  if (!/^[a-z]/.test(name)) name = "gpx_" + name;
  return name.slice(0, MAX_NAME).replace(/_+$/, "");
}

const STATE_ICON: Record<StepState, string> = {
  pending: "·",
  running: "",
  done: "✓",
  error: "✕",
};

export function GpxImportDialog({
  file,
  path,
  uploadError,
  database,
  existingNames,
  runStep,
  onClose,
}: Props) {
  const [name, setName] = useState(() => defaultName(file.name));
  // Null until Import is pressed; afterwards one state per step of PLAN. The
  // name is frozen at that point, so the rows keep naming what was created.
  const [states, setStates] = useState<StepState[] | null>(null);
  const [failure, setFailure] = useState<string | null>(null);
  const inputRef = useRef<HTMLInputElement>(null);
  const closeRef = useRef<HTMLButtonElement>(null);

  const started = states !== null;
  const running = started && failure === null && states.includes("running");
  const finished = started && !running;
  const created = states
    ? PLAN.filter((_, i) => states[i] === "done").map((s) => name + s.suffix)
    : [];

  // Refuse a name the first command would only fail on -- or, worse, a name
  // whose *later* objects collide, which would leave the import half done.
  const problem = useMemo(() => {
    if (!name) return "Give the import a name.";
    if (name.length > MAX_NAME) return `At most ${MAX_NAME} characters.`;
    if (!NAME_RE.test(name))
      return "Lowercase letters, digits and underscores only, starting with a letter.";
    const taken = new Set(existingNames.map((n) => n.toLowerCase()));
    for (const step of PLAN) {
      const candidate = name + step.suffix;
      if (taken.has(candidate)) return `${candidate} already exists in ${database}.`;
    }
    return null;
  }, [name, existingNames, database]);

  useEffect(() => {
    inputRef.current?.focus();
    inputRef.current?.select();
  }, []);

  // Move the focus onto Close once there is nothing else to do, so the dialog
  // can be dismissed from the keyboard without hunting for it.
  useEffect(() => {
    if (finished) closeRef.current?.focus();
  }, [finished]);

  // Esc dismisses -- but never mid-import, where it would leave commands
  // running with nothing showing their outcome.
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape" && !running) {
        e.stopPropagation();
        onClose(created);
      }
    };
    window.addEventListener("keydown", onKey, true);
    return () => window.removeEventListener("keydown", onKey, true);
  });

  async function start() {
    if (problem || !path) return;
    // Held locally as well: setState is not visible to the next iteration.
    const progress: StepState[] = PLAN.map(() => "pending");
    setStates([...progress]);
    for (let i = 0; i < PLAN.length; i++) {
      progress[i] = "running";
      setStates([...progress]);
      const outcome = await runStep(PLAN[i].command(name, path));
      if (!outcome.ok) {
        progress[i] = "error";
        setStates([...progress]);
        setFailure(outcome.error ?? "The command failed.");
        return;
      }
      progress[i] = "done";
      setStates([...progress]);
    }
  }

  const uploading = path === null && !uploadError;

  return (
    <div
      className="gpx-backdrop"
      // Clicking away dismisses only when nothing is in flight; during the
      // import there is no safe thing for a stray click to do.
      onMouseDown={(e) => {
        if (e.target === e.currentTarget && !running) onClose(created);
      }}
    >
      <div
        className="gpx-dialog"
        role="dialog"
        aria-modal="true"
        aria-labelledby="gpx-title"
      >
        <h2 className="gpx-title" id="gpx-title">
          Import GPX
        </h2>
        <p className="gpx-file" title={file.name}>
          {file.name}
        </p>

        {uploadError && <pre className="err">{uploadError}</pre>}

        {!uploadError && (
          <>
            <label className="gpx-label" htmlFor="gpx-name">
              Name in {database}
            </label>
            <input
              id="gpx-name"
              ref={inputRef}
              className="gpx-name"
              value={name}
              spellCheck={false}
              autoComplete="off"
              disabled={started}
              onChange={(e) => setName(e.target.value)}
              onKeyDown={(e) => {
                if (e.key === "Enter" && !problem && !uploading) void start();
              }}
            />
            {!started && problem && <p className="gpx-note err-text">{problem}</p>}

            {/* The same four rows before and during the import: first as a
                preview of what the name will produce -- one GPX track becomes
                four objects, and which is which is not guessable from the
                suffixes -- then as the progress, so nothing moves when Import
                is pressed and every row keeps saying what its object is. */}
            {name && (
              <>
                <p className="gpx-label" id="gpx-creates">
                  {started ? "Objects" : "Will create"}
                </p>
                <ol className="gpx-steps" aria-labelledby="gpx-creates">
                  {PLAN.map((step, i) => {
                    const state = states?.[i] ?? "pending";
                    return (
                      <li key={step.suffix} className={"gpx-step is-" + state}>
                        <span className="gpx-step-ic">
                          {state === "running" ? (
                            <span className="cat-spin" />
                          ) : (
                            STATE_ICON[state]
                          )}
                        </span>
                        <span className="gpx-step-obj">{name + step.suffix}</span>
                        <span className="gpx-step-what">
                          {state === "running" ? `${step.label}…` : step.kind}
                        </span>
                      </li>
                    );
                  })}
                </ol>

                {/* The same four commands the console will show, but here
                    before they run and next to the objects they create --
                    folded away, because the import is meant to be usable
                    without reading them, and unfolded by anyone who wants to
                    know what `gpximport` is being handed or to type the
                    pipeline again by hand. */}
                <details className="gpx-cmds">
                  <summary>Commands</summary>
                  <ol>
                    {PLAN.map((step, i) => (
                      <li key={step.suffix}>
                        <code>{step.command(name, path ?? "…")}</code>
                        {i === 0 && path === null && !uploadError && (
                          <span className="gpx-cmds-note">
                            {" "}
                            (the path is filled in when the upload lands)
                          </span>
                        )}
                      </li>
                    ))}
                  </ol>
                </details>
              </>
            )}
          </>
        )}

        {failure && (
          <>
            <pre className="err">{failure}</pre>
            <p className="gpx-note">
              {created.length > 0
                ? `${created.join(", ")} ${
                    created.length === 1 ? "was" : "were"
                  } created and left in ${database}.`
                : `Nothing was created in ${database}.`}
            </p>
          </>
        )}

        <div className="gpx-actions">
          {!started && !uploadError && (
            <button
              className="gpx-import"
              disabled={!!problem || uploading}
              onClick={() => void start()}
            >
              {uploading ? "uploading…" : "Import"}
            </button>
          )}
          <button
            ref={closeRef}
            className="gpx-close"
            disabled={running}
            onClick={() => onClose(created)}
          >
            {started ? "Close" : "Cancel"}
          </button>
        </div>
      </div>
    </div>
  );
}
