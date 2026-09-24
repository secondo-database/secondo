// A shape drawn on the map, written as a SECONDO constant -- the inverse of the
// backend's geojson.geometry_from. The coordinates are already the data's own
// (unprojected); this only spells them as nested lists.

export type ShapeType = "rect" | "region" | "point" | "line";

export interface Shape {
  type: ShapeType;
  /** point: one vertex; rect: two opposite corners; line/region: the vertices
   *  in drawing order, the region ring not closed. */
  coords: [number, number][];
}

// A real atom has to read as one: `3` would lex as an int, and a point of ints
// is not a point.
function real(v: number): string {
  const s = String(v);
  return /[.eE]/.test(s) ? s : `${s}.0`;
}

const xy = ([x, y]: [number, number]) => `${real(x)} ${real(y)}`;

/** The value list, i.e. what follows `value` in `[const <type> value ...]`. */
export function valueList(shape: Shape): string {
  const c = shape.coords;
  switch (shape.type) {
    case "point":
      return `(${xy(c[0])})`;
    case "rect": {
      // SECONDO's rect is (minx maxx miny maxy) -- x bounds first, then y.
      const xs = c.map((p) => p[0]);
      const ys = c.map((p) => p[1]);
      return `(${[Math.min(...xs), Math.max(...xs), Math.min(...ys), Math.max(...ys)]
        .map(real)
        .join(" ")})`;
    }
    case "line": {
      const segs: string[] = [];
      for (let i = 0; i + 1 < c.length; i++) {
        const [a, b] = [c[i], c[i + 1]];
        if (a[0] === b[0] && a[1] === b[1]) continue; // a zero-length segment
        segs.push(`(${xy(a)} ${xy(b)})`);
      }
      return `(${segs.join(" ")})`;
    }
    case "region":
      // One face with one cycle. The kernel closes the cycle itself and does
      // not care which way round it runs.
      return `(((${c.map((p) => `(${xy(p)})`).join(" ")})))`;
  }
}

/** The command that stores the shape as a database object. */
export function letCommand(name: string, shape: Shape): string {
  return `let ${name} = [const ${shape.type} value ${valueList(shape)}]`;
}

// --- validity -------------------------------------------------------------

type P = [number, number];

function cross(o: P, a: P, b: P): number {
  return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0]);
}

/** Whether segments ab and cd properly cross or overlap (touching at a shared
 *  endpoint of neighbouring edges does not count; callers skip those). */
function segmentsIntersect(a: P, b: P, c: P, d: P): boolean {
  const d1 = cross(c, d, a);
  const d2 = cross(c, d, b);
  const d3 = cross(a, b, c);
  const d4 = cross(a, b, d);
  if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
      ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
    return true;
  const on = (p: P, q: P, r: P) =>
    Math.min(p[0], q[0]) <= r[0] && r[0] <= Math.max(p[0], q[0]) &&
    Math.min(p[1], q[1]) <= r[1] && r[1] <= Math.max(p[1], q[1]);
  return (
    (d1 === 0 && on(c, d, a)) ||
    (d2 === 0 && on(c, d, b)) ||
    (d3 === 0 && on(a, b, c)) ||
    (d4 === 0 && on(a, b, d))
  );
}

/** Whether the edge from the last vertex to `next` crosses an earlier edge of
 *  the open path -- the check HoeseViewer's `haveIntersections` makes per
 *  click. With `closing`, the edge is the one back to the first vertex, which
 *  legitimately shares that vertex with the first edge. */
export function crossesPath(path: P[], next: P, closing = false): boolean {
  if (path.length < 2) return false;
  const a = path[path.length - 1];
  // Edge i runs path[i] -> path[i+1]. The last one shares `a` with the new
  // edge; when closing, the first one shares `next` (= path[0]).
  const first = closing ? 1 : 0;
  for (let i = first; i < path.length - 2; i++) {
    if (segmentsIntersect(path[i], path[i + 1], a, next)) return true;
  }
  return false;
}
