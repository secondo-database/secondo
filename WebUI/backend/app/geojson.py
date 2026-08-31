"""Convert parsed SECONDO nested lists into GeoJSON.

This mirrors, in spirit, the Hoese display classes
(``Javagui/viewer/hoese/algebras/Dspl*.java``) but emits GeoJSON geometries
instead of ``java.awt.Shape``. Milestone 2 covers the static spatial types;
moving/temporal types are added in Milestone 3.

IMPORTANT about coordinates: SECONDO spatial values carry coordinates in the
dataset's own world/coordinate system (e.g. berlintest uses a local planar
system, not WGS84). The GeoJSON produced here therefore holds *native* world
coordinates. The frontend renders them in a Cartesian view; geographic
reprojection / OSM background is a later milestone.
"""
from __future__ import annotations

import math
from typing import Any

from .nlparser import Node, parse

# Static spatial types understood so far (attribute-type names as they appear
# in a relation's tuple schema).
SPATIAL_TYPES = {"point", "points", "line", "sline", "dline", "region", "rect"}

_UNDEF = {"undef", "undefined"}


class _Bounds:
    """The running extent of everything that made it into the output.

    Accumulated while the geometries are built rather than walked out of the
    finished FeatureCollection afterwards. The second pass was over every
    coordinate in the answer -- 1,701,498 of them for ``roads`` -- through a
    recursive generator, one frame per nesting level per point, for four
    numbers construction already had in hand.

    ``add`` is only ever called for a coordinate that is being appended to a
    geometry that is being kept, which is what makes it equivalent to the walk:
    rings too short to be a ring and segments that failed to parse contribute
    to neither.
    """

    __slots__ = ("minx", "miny", "maxx", "maxy")

    def __init__(self) -> None:
        self.minx = self.miny = math.inf
        self.maxx = self.maxy = -math.inf

    def add(self, x: float, y: float) -> None:
        if x < self.minx:
            self.minx = x
        if x > self.maxx:
            self.maxx = x
        if y < self.miny:
            self.miny = y
        if y > self.maxy:
            self.maxy = y

    def as_list(self) -> list[float] | None:
        if self.minx is math.inf:
            return None
        return [self.minx, self.miny, self.maxx, self.maxy]


def new_bounds() -> _Bounds:
    """A fresh extent accumulator, for a caller that drives the feature
    building itself (``app/convert.py``) instead of going through
    ``from_tree``."""
    return _Bounds()


def _num(x: Any) -> float | None:
    if isinstance(x, bool):
        return None
    if isinstance(x, (int, float)):
        return float(x)
    return None


def _coord(pair: Any) -> list[float] | None:
    """A ``(x y)`` nested list -> ``[x, y]``."""
    if not isinstance(pair, list) or len(pair) < 2:
        return None
    x, y = _num(pair[0]), _num(pair[1])
    if x is None or y is None:
        return None
    return [x, y]


def _stitch(segments: list, bounds: _Bounds | None) -> list[list[list[float]]]:
    """Half-segments -> polylines.

    A SECONDO ``line`` is a *set* of half-segments, and ``OutLine`` writes each
    undirected segment once. Emitting one two-point part per segment therefore
    repeats every interior vertex: a road of *k* segments cost 2*k* coordinates
    where the polyline it describes needs *k*+1. Across ``roads`` that was
    1,701,498 coordinates in 850,749 parts for the 1,182,183 in 331,434 that
    the same geometry needs, and the duplicates are not free downstream either
    -- deck.gl tessellates each part separately.

    Nothing in the type promises path order, so this chains greedily off the
    running end and starts a new part whenever the next segment does not attach.
    An import that walks each way in order -- which is what shapefile data is --
    collapses to a single part; anything else degrades, part by part, to exactly
    the old output. Endpoints are compared exactly on purpose: the shared vertex
    of two adjacent half-segments is the same literal in the nested list, so it
    parses to the same float, and a tolerance here would join geometry the
    source kept apart.
    """
    parts: list[list[list[float]]] = []
    cur: list[list[float]] | None = None
    for seg in segments:
        if not isinstance(seg, list) or len(seg) < 4:
            continue
        a, b, c, d = (_num(seg[0]), _num(seg[1]), _num(seg[2]), _num(seg[3]))
        if None in (a, b, c, d):
            continue
        p, q = [a, b], [c, d]
        if cur is not None and cur[-1] == p:
            cur.append(q)
            if bounds is not None:
                bounds.add(c, d)
        elif cur is not None and cur[-1] == q:
            cur.append(p)
            if bounds is not None:
                bounds.add(a, b)
        else:
            cur = [p, q]
            parts.append(cur)
            if bounds is not None:
                bounds.add(a, b)
                bounds.add(c, d)
    return parts


def geometry_from(
    type_name: str, value: Node, bounds: _Bounds | None = None
) -> dict | None:
    """Build a GeoJSON geometry from a type name and its *value* nested list.

    ``bounds``, when given, collects the extent of the coordinates that end up
    in the result, so the caller never has to walk them again.
    """
    if isinstance(value, str) and value in _UNDEF:
        return None

    if type_name == "point":
        c = _coord(value)
        if not c:
            return None
        if bounds is not None:
            bounds.add(c[0], c[1])
        return {"type": "Point", "coordinates": c}

    if type_name == "points":
        if not isinstance(value, list):
            return None
        pts = [c for c in (_coord(p) for p in value) if c]
        if not pts:
            return None
        if bounds is not None:
            for x, y in pts:
                bounds.add(x, y)
        return {"type": "MultiPoint", "coordinates": pts}

    if type_name in ("line", "sline", "dline"):
        if not isinstance(value, list):
            return None
        # sline/dline wrap the segment list with a direction flag:
        #   (sline ( (seg*) bool ))   vs   (line ( seg* ))
        segments = value
        if (
            len(value) == 2
            and isinstance(value[0], list)
            and isinstance(value[1], bool)
        ):
            segments = value[0]
        parts = _stitch(segments, bounds)
        return {"type": "MultiLineString", "coordinates": parts} if parts else None

    if type_name == "region":
        return _region(value, bounds)

    if type_name == "rect":
        if isinstance(value, list) and len(value) >= 4:
            x1, x2, y1, y2 = (_num(value[0]), _num(value[1]),
                              _num(value[2]), _num(value[3]))
            if None not in (x1, x2, y1, y2):
                ring = [[x1, y1], [x2, y1], [x2, y2], [x1, y2], [x1, y1]]
                if bounds is not None:
                    for x, y in ring:
                        bounds.add(x, y)
                return {"type": "Polygon", "coordinates": [ring]}
        return None

    return None


def _region(value: Node, bounds: _Bounds | None = None) -> dict | None:
    """SECONDO region: a list of faces; each face is a list of cycles; the
    first cycle is the outer boundary, the rest are holes."""
    if not isinstance(value, list):
        return None
    polygons = []
    for face in value:
        if not isinstance(face, list):
            continue
        rings = []
        for cycle in face:
            if not isinstance(cycle, list):
                continue
            ring = [c for c in (_coord(p) for p in cycle) if c]
            if len(ring) < 3:
                continue
            if ring[0] != ring[-1]:  # GeoJSON rings must be closed
                ring.append(ring[0])
            if bounds is not None:
                for x, y in ring:
                    bounds.add(x, y)
            rings.append(ring)
        if rings:
            polygons.append(rings)
    if not polygons:
        return None
    return {"type": "MultiPolygon", "coordinates": polygons}


def _feature(geometry: dict, properties: dict | None = None) -> dict:
    return {"type": "Feature", "geometry": geometry, "properties": properties or {}}


def _scalar(v: Any) -> Any | None:
    """Keep only scalar attribute values as GeoJSON properties.

    Trailing whitespace goes. It is fixed-width padding out of the source data
    -- a shapefile import gives every ``roads`` tuple a 12-character ``Osm_id``,
    a 28-character ``Fclass``, a 100-character ``Name`` -- and it was 21.9% of
    the GeoJSON. These properties feed map tooltips, where the padding was never
    visible in the first place.

    Only here, deliberately: `table.py` serves the editable grid, and a Save
    writes its cells back to the relation. Trimming values on the way out of
    that path would silently rewrite the stored strings.
    """
    if isinstance(v, str):
        return v.rstrip()
    if isinstance(v, (int, float, bool)):
        return v
    return None


class RelationFeatures:
    """Collects a relation's features, one tuple at a time.

    Split out of ``_relation_features`` so the tuples do not all have to be in
    hand at once: the bridge can hand them over as it decodes them (see
    ``app/convert.py``), and then only one tuple's Python objects are ever
    live instead of the whole relation's.

    Fed the same tuples in the same order it produces exactly what the whole-
    list function produces -- it *is* that function's loop body.
    """

    __slots__ = ("names", "types", "spatial_idx", "bounds", "features", "_rows")

    def __init__(self, type_expr: Node, bounds: _Bounds | None = None) -> None:
        # type_expr = ['rel', ['tuple', [[name, atype], ...]]]
        try:
            attrs = type_expr[1][1]  # type: ignore[index]
            self.names = [a[0] for a in attrs]
            self.types = [a[1] for a in attrs]
        except (IndexError, TypeError):
            self.names = []
            self.types = []
        self.spatial_idx = [
            i for i, t in enumerate(self.types) if t in SPATIAL_TYPES
        ]
        self.bounds = bounds
        self.features: list[dict] = []
        self._rows = 0

    @property
    def wanted(self) -> bool:
        """Whether feeding this anything can produce a feature at all. A
        relation with no spatial attribute never will, and a streaming caller
        uses this to skip the per-tuple work rather than do it 212,099 times
        for an empty answer."""
        return bool(self.spatial_idx)

    def feed(self, tup: Node) -> None:
        # Counted before anything can return, so the ordinal stays in step with
        # `table.RelationRows.total`, which is also incremented unconditionally.
        row = self._rows
        self._rows += 1
        if not isinstance(tup, list):
            return
        names, types, spatial_idx = self.names, self.types, self.spatial_idx
        props = {
            names[i]: _scalar(tup[i])
            for i in range(min(len(names), len(tup)))
            if i not in spatial_idx and _scalar(tup[i]) is not None
        }
        for i in spatial_idx:
            if i < len(tup):
                geom = geometry_from(types[i], tup[i], self.bounds)
                if geom:
                    # `_row` is the tuple's position in the answer's scan order,
                    # which is what ties a feature back to its row in the table
                    # payload -- built from the same tuple stream, in the same
                    # order, by `table.RelationRows`. `temporal.RelationMoving`
                    # emits the same key for the same reason. A tuple with two
                    # spatial attributes yields two features sharing one `_row`
                    # and told apart by `_attr`.
                    self.features.append(
                        _feature(geom, {**props, "_attr": names[i], "_row": row})
                    )

    def collection(self) -> dict | None:
        """The FeatureCollection for everything fed so far, or None if nothing
        spatial came out of it."""
        return _collection(self.features, self.bounds)


def _collection(features: list[dict], bounds: _Bounds | None) -> dict | None:
    if not features:
        return None
    fc: dict = {"type": "FeatureCollection", "features": features}
    bbox = bounds.as_list() if bounds is not None else None
    if bbox:
        fc["bbox"] = bbox
    return fc


def _relation_features(
    type_expr: list, tuples: Node, bounds: _Bounds | None = None
) -> list[dict]:
    collector = RelationFeatures(type_expr, bounds)
    if not collector.wanted or not isinstance(tuples, list):
        return []
    for tup in tuples:
        collector.feed(tup)
    return collector.features


def from_tree(tree: Node) -> dict | None:
    """Turn a parsed ``(type value)`` (or relation) tree into a
    GeoJSON FeatureCollection, or ``None`` if nothing spatial is present."""
    if not isinstance(tree, list) or len(tree) < 2:
        return None

    type_expr, value = tree[0], tree[1]
    features: list[dict] = []
    bounds = _Bounds()

    if isinstance(type_expr, str):  # atomic spatial type: (point (...)) etc.
        geom = geometry_from(type_expr, value, bounds)
        if geom:
            features.append(_feature(geom))
    elif isinstance(type_expr, list) and type_expr and type_expr[0] == "rel":
        features = _relation_features(type_expr, value, bounds)

    return _collection(features, bounds)


def to_geojson(nested_text: str) -> dict | None:
    """Parse nested-list text and convert to a GeoJSON FeatureCollection."""
    return from_tree(parse(nested_text))
