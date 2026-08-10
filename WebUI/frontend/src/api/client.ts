// Thin client for the FastAPI bridge. Cookies (the session id) are sent
// automatically because requests are same-origin via the Vite proxy.

export interface Trip {
  path: [number, number][];
  timestamps: number[];
  properties: Record<string, unknown>;
}

/** A vertex of a moving region: [xStart, yStart, xEnd, yEnd] across the unit. */
export type MovingVertex = [number, number, number, number];

export interface MovingRegionUnit {
  interval: [number, number];
  /** face -> cycle (first outer, rest holes) -> moving vertices */
  faces: MovingVertex[][][];
}

export interface MovingRegion {
  units: MovingRegionUnit[];
  properties: Record<string, unknown>;
}

/** A scalar moving value (mreal/mint/mbool) sampled over time. */
export interface Plot {
  label: string;
  kind: "line" | "step";
  type: string;
  /** [posixSeconds, value] pairs */
  series: [number, number][];
  valueRange: [number, number];
  timeDomain: [number, number];
}

/** One interval of a symbolic trajectory: `[t0, t1, text]` in POSIX seconds.
 *  The value holds for the whole interval -- there is nothing to interpolate
 *  between two labels -- and a gap between two of them means the label is not
 *  defined then, not that it kept its last value. */
export type LabelUnit = [number, number, string];

/** A time-varying text value (`mlabel` / `mstring`). It draws nothing of its
 *  own: it is written next to the moving point of the same `row`. Several may
 *  share a row, and they arrive in relation-schema order -- the order their
 *  lines are stacked in. */
export interface LabelSeries {
  attr: string;
  type: string;
  row: number;
  units: LabelUnit[];
  timeDomain: [number, number];
}

export interface TemporalPayload {
  trips: Trip[];
  regions: MovingRegion[];
  plots: Plot[];
  labels: LabelSeries[];
  timeDomain: [number, number];
  bbox: [number, number, number, number] | null;
}

/** One attribute of a relation. `atomic` types are editable with a plain input;
 *  everything else takes raw nested-list syntax, as it does in the Java GUI. */
export interface TableColumn {
  name: string;
  type: string;
  atomic: boolean;
}

/** A relation result as rows and columns. */
export interface TablePayload {
  columns: TableColumn[];
  /** Cell values in column order: numbers/booleans/strings for atomic types,
   *  nested-list text for everything else, null for undefined. */
  rows: (string | number | boolean | null)[][];
  rowCount: number;
  /** The server capped the rows and the rest are *unreachable* -- only an
   *  ad-hoc query result can be, since the backend did not write that query.
   *  A page is never truncated: the next one is a request away. */
  truncated: boolean;
  totalRows: number;
  /** Whether `totalRows` is the real total or only what has been seen so far
   *  (paging skips the counting scan unless something invalidated the count). */
  totalKnown: boolean;
  /** Index of the first row shown, 0 when the whole result is here. */
  offset: number;
  /** Page size the server applied, null when the result is not paged. */
  limit: number | null;
  /** Whether another page of this table can be asked for. */
  pageable: boolean;
  /** Which column holds the tuple identifier, or null when the result was not
   *  loaded with `addid` -- i.e. when it cannot be edited. */
  tidIndex: number | null;
  /** The stored relation this table writes back to, once loaded for editing. */
  relation: string | null;
}

export interface FeatureCollection {
  type: "FeatureCollection";
  features: unknown[];
  bbox?: [number, number, number, number];
}

export interface QueryResponse {
  text: string;
  geojson: FeatureCollection | null;
  temporal: TemporalPayload | null;
  table: TablePayload | null;
  // The stored relation this result came from, when the server could name one
  // without guessing. Only a hint for offering the table's Edit button.
  relation?: string | null;
  // Optimizer fields; absent for an ordinary kernel command. The server decides
  // which language a command is in and reports the level it resolved it to:
  // 2 = SQL (plan + costs), 3 = an optimizer directive (message).
  level?: number | null;
  plan?: string | null;
  costs?: number | null;
  message?: string | null;
  plan_only?: boolean;
  executed_by_optimizer?: boolean;
}

// Parse a response body defensively: a failing backend may return a non-JSON
// body (e.g. a plain-text 500), which must surface as a readable error rather
// than a cryptic "JSON.parse: unexpected character" from res.json().
async function parseResponse<T>(res: Response): Promise<T> {
  const text = await res.text();
  let data: unknown;
  try {
    data = text ? JSON.parse(text) : {};
  } catch {
    data = { detail: text.slice(0, 500) || `Request failed (${res.status})` };
  }
  if (!res.ok) {
    const detail = (data as { detail?: string })?.detail;
    throw new Error(detail ?? `Request failed (${res.status})`);
  }
  return data as T;
}

// Serialize all API calls into a single in-order queue. The session cookie is
// minted by the backend on the first request; running requests concurrently
// before that cookie is set would create multiple sessions (and e.g. leave the
// object list querying a session where no database is open). One-at-a-time also
// matches SECONDO, which handles one command per connection anyway.
let chain: Promise<unknown> = Promise.resolve();
function enqueue<T>(task: () => Promise<T>): Promise<T> {
  const result = chain.then(task, task);
  chain = result.then(
    () => undefined,
    () => undefined
  );
  return result;
}

async function post<T>(path: string, body: unknown): Promise<T> {
  return enqueue(async () => {
    const res = await fetch(path, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      credentials: "same-origin",
      body: JSON.stringify(body),
    });
    return parseResponse<T>(res);
  });
}

async function get<T>(path: string): Promise<T> {
  return enqueue(async () => {
    const res = await fetch(path, { credentials: "same-origin" });
    return parseResponse<T>(res);
  });
}

/** Run a command. `view: "table"` asks the server for the rows only -- it skips
 *  the spatial and temporal conversion instead of converting payloads the
 *  caller is going to discard. `view: "none"` asks for nothing at all: the GPX
 *  import runs `let x = ... consume`, whose answer is the whole created object.
 *  Omitted means "auto": convert whatever fits. */
export function runQuery(
  command: string,
  view?: "table" | "none"
): Promise<QueryResponse> {
  return post<QueryResponse>("/api/query", view ? { command, view } : { command });
}

/** Put a file where a SECONDO import operator can read it, and return the path
 *  to name in the command.
 *
 *  The file is the request body rather than a multipart part -- one file needs
 *  none of what multipart buys. It goes through the same queue as every other
 *  call so it cannot race the session cookie, and the server deletes it when
 *  the session closes.
 *
 *  The path is the *bridge's*, and `gpximport` opens it on the SECONDO server:
 *  the two have to share a filesystem, which they do in the default deployment.
 */
export async function uploadGpx(file: File): Promise<{
  path: string;
  filename: string;
  size: number;
}> {
  return enqueue(async () => {
    const res = await fetch(
      `/api/upload?filename=${encodeURIComponent(file.name)}`,
      {
        method: "POST",
        headers: { "Content-Type": "application/gpx+xml" },
        credentials: "same-origin",
        body: file,
      }
    );
    return parseResponse(res);
  });
}

export async function listDatabases(): Promise<{
  databases: string[];
  open: string | null;
  // Whether the connected server can run SQL. A property of that server, so it
  // comes with the session state rather than being remembered by the UI.
  optimizer: boolean;
}> {
  return get("/api/databases");
}

export interface CatalogObject {
  name: string;
  type: string;
  kind: "spatial" | "temporal" | "other";
  // A flat relation -- the only thing the table view can open and edit.
  relation: boolean;
}

/** Read one page of a stored relation *with* its tuple identifiers, which is
 *  what makes a table editable (`query <Rel> feed ... addid consume`).
 *
 *  The page is cut server-side, so a relation of any size can be browsed and
 *  edited. `wantTotal` costs a counting scan; leave it off when only the page
 *  changed and the caller already knows the total. */
export function loadTable(
  relation: string,
  opts: {
    filters?: string[];
    project?: string[];
    sort?: string[];
    offset?: number;
    limit?: number;
    wantTotal?: boolean;
    /** Ask for the TID column. Only a table loaded with it can be edited. */
    tids?: boolean;
  } = {}
): Promise<{ table: TablePayload; command: string }> {
  const { wantTotal, ...rest } = opts;
  return post("/api/table/load", {
    relation,
    ...rest,
    ...(wantTotal === undefined ? {} : { want_total: wantTotal }),
  });
}

/** One operator the connected server knows. A property of the *server*, not of
 *  the open database, so the query editor fetches it once per session. */
export interface OperatorInfo {
  name: string;
  /** How it is written, e.g. `_ feed` -- shown next to the name while typing. */
  syntax: string;
}

export async function listOperators(): Promise<{ operators: OperatorInfo[] }> {
  return get("/api/operators");
}

export interface TableEdits {
  relation: string;
  /** Only the changed attributes, per tuple identifier. */
  updates: { tid: number; values: Record<string, string> }[];
  deletes: number[];
  /** Every non-TID attribute, per new tuple. */
  inserts: { values: Record<string, string> }[];
}

/** Apply a batch of edits. The whole batch succeeds or none of it does. */
export function commitTable(edits: TableEdits): Promise<{
  applied: number;
  inserted: number[];
  transactional: boolean;
}> {
  return post("/api/table/commit", edits);
}

export async function listObjects(): Promise<{
  objects: CatalogObject[];
  open: string | null;
}> {
  return get("/api/objects");
}
