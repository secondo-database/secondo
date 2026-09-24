import { useEffect, useRef, useState } from "react";
import type { CoordinateSystem, Layer } from "@deck.gl/core";
import { PathLayer, PolygonLayer, ScatterplotLayer } from "@deck.gl/layers";
import { crossesPath, type ShapeType } from "./secondoValue";

// Drawing a query shape on the map, the WebUI's version of HoeseViewer's
// "Object Creation" menu. Everything here is in the map's *render*
// coordinates -- whatever deck unprojects a click to -- and the caller maps the
// finished shape back to the data's own before it is stored.

type P = [number, number];

const HINT: Record<ShapeType, string> = {
  point: "Click to place the point · Esc cancels",
  rect: "Drag to span the rectangle · Esc cancels",
  line: "Click to add vertices · double-click or Enter finishes · Backspace undoes · Esc cancels",
  region:
    "Click to add corners · double-click or Enter finishes · Backspace undoes · Esc cancels",
};

const INK: [number, number, number, number] = [255, 196, 0, 255];
const FILL: [number, number, number, number] = [255, 196, 0, 60];

/**
 * @param onDone called with the finished shape in render coordinates. A rect
 *   comes as its four corners, so a caller that unprojects them still gets the
 *   true extent under a projection that does not keep axes axis-aligned.
 */
export function useDraw(
  onDone: (type: ShapeType, coords: P[]) => void,
  /** A pointer position -> render coordinates, or null when `target` is not
   *  the map itself but a control lying over it. */
  unproject: (clientX: number, clientY: number, target: Element) => P | null
) {
  const [mode, setModeState] = useState<ShapeType | null>(null);
  // Held in a ref as well as in state: deck calls the handlers below from its
  // own event loop, and two clicks can arrive before React has re-rendered and
  // handed deck the new closures -- reading `verts` from the render would then
  // drop a vertex. State only drives the redraw.
  const vertsRef = useRef<P[]>([]);
  const [verts, setVertsState] = useState<P[]>([]);
  const setVerts = (v: P[]) => {
    vertsRef.current = v;
    setVertsState(v);
  };
  const [cursor, setCursor] = useState<P | null>(null);
  // A ref for the same reason as the vertices: a quick drag ends before the
  // render that would have seen it start.
  const dragRef = useRef<P | null>(null);
  const [dragStart, setDragStartState] = useState<P | null>(null);
  const setDragStart = (p: P | null) => {
    dragRef.current = p;
    setDragStartState(p);
  };
  const [warning, setWarning] = useState<string | null>(null);

  const reset = () => {
    setVerts([]);
    setCursor(null);
    setDragStart(null);
    setWarning(null);
  };

  /** Pick a mode; picking the active one again leaves drawing altogether. */
  const setMode = (m: ShapeType | null) => {
    reset();
    setModeState((cur) => (cur === m ? null : m));
  };

  const done = (type: ShapeType, coords: P[]) => {
    reset();
    setModeState(null);
    onDone(type, coords);
  };

  const finish = () => {
    const verts = vertsRef.current;
    if (mode === "line") {
      if (verts.length < 2) return setWarning("A line needs at least two vertices.");
      done("line", verts);
    } else if (mode === "region") {
      if (verts.length < 3) return setWarning("A region needs at least three corners.");
      if (crossesPath(verts, verts[0], true))
        return setWarning("Closing the region here would cross one of its edges.");
      done("region", verts);
    }
  };

  const undo = () => {
    setWarning(null);
    setVerts(vertsRef.current.slice(0, -1));
  };

  // Keys act only while drawing, and never while typing somewhere -- Backspace
  // in the console must still delete a character.
  useEffect(() => {
    if (!mode) return;
    const onKey = (e: KeyboardEvent) => {
      const t = e.target as HTMLElement | null;
      if (t && (t.tagName === "INPUT" || t.tagName === "TEXTAREA" || t.isContentEditable))
        return;
      if (e.key === "Escape") setMode(null);
      else if (e.key === "Enter") finish();
      else if (e.key === "Backspace") undo();
      else return;
      e.preventDefault();
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  });

  // Pointer handling is native, on the map's wrapper, rather than through
  // deck's onClick/onDrag: deck holds a click back to rule out a double-tap
  // and swallows the clicks it counts as one, so vertices clicked at drawing
  // speed went missing and a double-click finished the shape before its last
  // corner had arrived. Here a click is a vertex the moment it lands.
  const downAt = useRef<[number, number] | null>(null);

  /** Where a pointer event is on the map, or null off it (on a control). */
  const at = (e: React.MouseEvent): P | null =>
    unproject(e.clientX, e.clientY, e.target as Element);

  const addVertex = (p: P) => {
    const verts = vertsRef.current;
    const last = verts[verts.length - 1];
    // The second click of a double-click lands on the same spot; so does an
    // accidental double tap. Neither is a vertex.
    if (last && Math.hypot(last[0] - p[0], last[1] - p[1]) === 0) return;
    if (mode === "region" && crossesPath(verts, p)) {
      setWarning("That edge would cross another one; pick a different corner.");
      return;
    }
    setWarning(null);
    setVerts([...verts, p]);
  };

  const handlers = {
    onPointerDown: (e: React.PointerEvent) => {
      if (!mode || e.button !== 0) return;
      downAt.current = [e.clientX, e.clientY];
      const p = at(e);
      if (mode === "rect" && p) {
        setDragStart(p);
        setCursor(p);
      }
    },
    onPointerMove: (e: React.PointerEvent) => {
      if (!mode) return;
      if (mode === "line" || mode === "region" || (mode === "rect" && dragRef.current)) {
        const p = at(e);
        if (p) setCursor(p);
      }
    },
    onPointerUp: (e: React.PointerEvent) => {
      const start = dragRef.current;
      if (mode !== "rect" || !start) return;
      const end = at(e) ?? cursor;
      if (!end || end[0] === start[0] || end[1] === start[1]) {
        setDragStart(null);
        setWarning("The rectangle has no area; drag across the map.");
        return;
      }
      const [x0, y0] = start;
      const [x1, y1] = end;
      done("rect", [[x0, y0], [x1, y0], [x1, y1], [x0, y1]]);
    },
    onClick: (e: React.MouseEvent) => {
      if (!mode || mode === "rect") return;
      // A drag that panned the map ends in a click too; it is not a vertex.
      const d = downAt.current;
      if (d && Math.hypot(e.clientX - d[0], e.clientY - d[1]) > 4) return;
      const p = at(e);
      if (!p) return;
      if (mode === "point") done("point", [p]);
      else addVertex(p);
    },
    onDoubleClick: (e: React.MouseEvent) => {
      if (at(e)) finish();
    },
  };

  /** The shape so far, drawn over every data layer. */
  const layers = (coordinateSystem: CoordinateSystem): Layer[] => {
    if (!mode) return [];
    const out: Layer[] = [];
    if (mode === "rect" && dragStart && cursor) {
      const [x0, y0] = dragStart;
      const [x1, y1] = cursor;
      out.push(
        new PolygonLayer<P[]>({
          id: "draw-rect",
          data: [[[x0, y0], [x1, y0], [x1, y1], [x0, y1]]],
          coordinateSystem,
          getPolygon: (d: P[]) => d,
          getFillColor: FILL,
          getLineColor: INK,
          getLineWidth: 2,
          lineWidthUnits: "pixels",
        })
      );
    }
    if (verts.length > 0) {
      const path = cursor ? [...verts, cursor] : verts;
      if (mode === "region" && path.length >= 3) {
        out.push(
          new PolygonLayer<P[]>({
            id: "draw-region",
            data: [path],
            coordinateSystem,
            getPolygon: (d: P[]) => d,
            getFillColor: FILL,
            stroked: false,
          })
        );
      }
      out.push(
        new PathLayer<P[]>({
          id: "draw-path",
          data: [mode === "region" && path.length >= 3 ? [...path, path[0]] : path],
          coordinateSystem,
          getPath: (d: P[]) => d,
          getColor: INK,
          getWidth: 2,
          widthUnits: "pixels",
        }),
        new ScatterplotLayer<P>({
          id: "draw-verts",
          data: verts,
          coordinateSystem,
          getPosition: (d) => d,
          getRadius: 5,
          radiusUnits: "pixels",
          getFillColor: INK,
          stroked: true,
          getLineColor: [0, 0, 0, 200],
          lineWidthMinPixels: 1,
        })
      );
    }
    return out;
  };

  return {
    mode,
    vertexCount: verts.length,
    setMode,
    finish,
    cancel: () => setMode(null),
    canFinish: (mode === "line" && verts.length >= 2) || (mode === "region" && verts.length >= 3),
    hint: mode ? HINT[mode] : null,
    warning,
    handlers,
    layers,
  };
}
