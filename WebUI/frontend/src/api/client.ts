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

export interface TemporalPayload {
  trips: Trip[];
  regions: MovingRegion[];
  plots: Plot[];
  timeDomain: [number, number];
  bbox: [number, number, number, number] | null;
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

export function runQuery(command: string): Promise<QueryResponse> {
  return post<QueryResponse>("/api/query", { command });
}

export async function listDatabases(): Promise<{ databases: string[]; open: string | null }> {
  return get("/api/databases");
}

export interface CatalogObject {
  name: string;
  type: string;
  kind: "spatial" | "temporal" | "other";
}

export async function listObjects(): Promise<{
  objects: CatalogObject[];
  open: string | null;
}> {
  return get("/api/objects");
}
