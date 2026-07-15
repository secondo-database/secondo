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


def geometry_from(type_name: str, value: Node) -> dict | None:
    """Build a GeoJSON geometry from a type name and its *value* nested list."""
    if isinstance(value, str) and value in _UNDEF:
        return None

    if type_name == "point":
        c = _coord(value)
        return {"type": "Point", "coordinates": c} if c else None

    if type_name == "points":
        if not isinstance(value, list):
            return None
        pts = [c for c in (_coord(p) for p in value) if c]
        return {"type": "MultiPoint", "coordinates": pts} if pts else None

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
        segs = []
        for seg in segments:
            if isinstance(seg, list) and len(seg) >= 4:
                a, b, c, d = (_num(seg[0]), _num(seg[1]), _num(seg[2]), _num(seg[3]))
                if None not in (a, b, c, d):
                    segs.append([[a, b], [c, d]])
        return {"type": "MultiLineString", "coordinates": segs} if segs else None

    if type_name == "region":
        return _region(value)

    if type_name == "rect":
        if isinstance(value, list) and len(value) >= 4:
            x1, x2, y1, y2 = (_num(value[0]), _num(value[1]),
                              _num(value[2]), _num(value[3]))
            if None not in (x1, x2, y1, y2):
                ring = [[x1, y1], [x2, y1], [x2, y2], [x1, y2], [x1, y1]]
                return {"type": "Polygon", "coordinates": [ring]}
        return None

    return None


def _region(value: Node) -> dict | None:
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
            rings.append(ring)
        if rings:
            polygons.append(rings)
    if not polygons:
        return None
    return {"type": "MultiPolygon", "coordinates": polygons}


def _feature(geometry: dict, properties: dict | None = None) -> dict:
    return {"type": "Feature", "geometry": geometry, "properties": properties or {}}


def _scalar(v: Any) -> Any | None:
    """Keep only scalar attribute values as GeoJSON properties."""
    if isinstance(v, (int, float, bool, str)):
        return v
    return None


def _relation_features(type_expr: list, tuples: Node) -> list[dict]:
    # type_expr = ['rel', ['tuple', [[name, atype], ...]]]
    try:
        attrs = type_expr[1][1]
        names = [a[0] for a in attrs]
        types = [a[1] for a in attrs]
    except (IndexError, TypeError):
        return []
    spatial_idx = [i for i, t in enumerate(types) if t in SPATIAL_TYPES]
    if not spatial_idx or not isinstance(tuples, list):
        return []

    features: list[dict] = []
    for tup in tuples:
        if not isinstance(tup, list):
            continue
        props = {
            names[i]: _scalar(tup[i])
            for i in range(min(len(names), len(tup)))
            if i not in spatial_idx and _scalar(tup[i]) is not None
        }
        for i in spatial_idx:
            if i < len(tup):
                geom = geometry_from(types[i], tup[i])
                if geom:
                    features.append(_feature(geom, {**props, "_attr": names[i]}))
    return features


def _iter_coords(geometry: dict):
    def walk(coords):
        if (isinstance(coords, list) and len(coords) == 2
                and all(isinstance(v, (int, float)) for v in coords)):
            yield coords
        elif isinstance(coords, list):
            for c in coords:
                yield from walk(c)

    yield from walk(geometry.get("coordinates", []))


def _bbox(features: list[dict]) -> list[float] | None:
    minx = miny = math.inf
    maxx = maxy = -math.inf
    for f in features:
        for x, y in _iter_coords(f["geometry"]):
            minx, miny = min(minx, x), min(miny, y)
            maxx, maxy = max(maxx, x), max(maxy, y)
    if minx is math.inf:
        return None
    return [minx, miny, maxx, maxy]


def from_tree(tree: Node) -> dict | None:
    """Turn a parsed ``(type value)`` (or relation) tree into a
    GeoJSON FeatureCollection, or ``None`` if nothing spatial is present."""
    if not isinstance(tree, list) or len(tree) < 2:
        return None

    type_expr, value = tree[0], tree[1]
    features: list[dict] = []

    if isinstance(type_expr, str):  # atomic spatial type: (point (...)) etc.
        geom = geometry_from(type_expr, value)
        if geom:
            features.append(_feature(geom))
    elif isinstance(type_expr, list) and type_expr and type_expr[0] == "rel":
        features = _relation_features(type_expr, value)

    if not features:
        return None
    fc: dict = {"type": "FeatureCollection", "features": features}
    bbox = _bbox(features)
    if bbox:
        fc["bbox"] = bbox
    return fc


def to_geojson(nested_text: str) -> dict | None:
    """Parse nested-list text and convert to a GeoJSON FeatureCollection."""
    return from_tree(parse(nested_text))
