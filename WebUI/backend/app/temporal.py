"""Convert SECONDO moving/temporal nested lists into animation trips.

Milestone 3 handles ``mpoint`` (and relations carrying ``mpoint`` attributes),
the signature HoeseViewer feature. An ``mpoint`` is a list of *units*:

    (mpoint ( unit* ))
    unit     = ( interval mapping )
    interval = ( "start-instant" "end-instant" lc rc )
    mapping  = ( x0 y0 x1 y1 )     ; linear from (x0,y0)@start to (x1,y1)@end

This mirrors the piecewise-linear model the HoeseViewer interpolates at render
time (``Dsplmovingpoint`` + ``IntervalSearch``). We emit deck.gl ``TripsLayer``
data: for each continuous run of units, a ``path`` of waypoints and a matching
list of ``timestamps`` (seconds). The frontend interpolates position from a
single animated ``currentTime`` -- the same math, moved to the GPU/JS side.
"""
from __future__ import annotations

import datetime as _dt
import math

from .nlparser import Node

MOVING_POINT_TYPES = {"mpoint"}
MOVING_REGION_TYPES = {"mregion"}
# Scalar-valued moving types: plotted as a value/time series rather than drawn.
PLOT_TYPES = {"mreal", "mint", "mbool"}
MOVING_TYPES = MOVING_POINT_TYPES | MOVING_REGION_TYPES | PLOT_TYPES

# Instants SECONDO uses for unbounded intervals.
_UNBOUNDED = {"begin of time", "end of time"}
# Samples per non-linear mreal unit (a quadratic needs a few points to curve).
_UREAL_SAMPLES = 16
_SECONDS_PER_DAY = 86400.0

_UNDEF = {"undef", "undefined"}
# Split a moving point into separate trips when the temporal gap between two
# consecutive units exceeds this (seconds); within a run, units are contiguous.
_GAP_EPS = 1e-3


def _num(x: object) -> float | None:
    if isinstance(x, bool):
        return None
    if isinstance(x, (int, float)):
        return float(x)
    return None


def parse_instant(s: str) -> float | None:
    """SECONDO instant string -> POSIX seconds (UTC).

    Formats seen: ``2003-11-20`` (date only -> midnight), ``2003-11-20-06:03``,
    ``2003-11-20-06:04:50``, ``2003-11-20-06:03:52.685`` (date and time
    separated by ``-``). Unbounded markers ("begin of time" / "end of time")
    return None; callers resolve them against the object's finite extent.
    """
    if not isinstance(s, str):
        return None
    s = s.strip()
    if len(s) == 10:  # date only
        date_part, time_part = s, ""
    elif len(s) >= 12 and s[10] == "-":
        date_part, time_part = s[:10], s[11:]
    else:
        return None
    try:
        year, month, day = (int(v) for v in date_part.split("-"))
        hour = minute = 0
        seconds = 0.0
        if time_part:
            hms = time_part.split(":")
            hour = int(hms[0])
            minute = int(hms[1])
            seconds = float(hms[2]) if len(hms) > 2 else 0.0
        whole = int(seconds)
        micro = int(round((seconds - whole) * 1_000_000))
        dt = _dt.datetime(
            year, month, day, hour, minute, whole, micro,
            tzinfo=_dt.timezone.utc,
        )
    except (ValueError, IndexError):
        return None
    return dt.timestamp()


def _parse_unit(unit: Node) -> tuple[float, float, float, float, float, float] | None:
    try:
        interval, mapping = unit[0], unit[1]  # type: ignore[index]
        t0 = parse_instant(interval[0])
        t1 = parse_instant(interval[1])
        if t0 is None or t1 is None:
            return None
        x0, y0, x1, y1 = (float(mapping[i]) for i in range(4))
    except (IndexError, TypeError, ValueError):
        return None
    return t0, t1, x0, y0, x1, y1


def mpoint_to_trips(value: Node, properties: dict | None = None) -> list[dict]:
    """Turn an ``mpoint`` value (list of units) into TripsLayer trips."""
    if isinstance(value, str) and value in _UNDEF:
        return []
    if not isinstance(value, list):
        return []

    props = properties or {}
    trips: list[dict] = []
    path: list[list[float]] = []
    timestamps: list[float] = []
    prev_t1: float | None = None

    def flush() -> None:
        if len(path) >= 2:
            trips.append(
                {"path": list(path), "timestamps": list(timestamps), "properties": props}
            )

    for unit in value:
        parsed = _parse_unit(unit)
        if parsed is None:
            continue
        t0, t1, x0, y0, x1, y1 = parsed
        if prev_t1 is None or t0 > prev_t1 + _GAP_EPS:
            flush()
            path = [[x0, y0]]
            timestamps = [t0]
        path.append([x1, y1])
        timestamps.append(t1)
        prev_t1 = t1
    flush()
    return trips


def mregion_to_moving(value: Node, properties: dict | None = None) -> dict | None:
    """Turn an ``mregion`` value into time-interpolatable moving faces.

        (mregion ( uregion* ))
        uregion = ( interval ( face* ) )
        face    = ( cycle* )                  ; first cycle outer, rest holes
        cycle   = ( vertex* )
        vertex  = ( xStart yStart xEnd yEnd ) ; moves linearly across the unit

    Emitted shape: ``{units: [{interval: [t0, t1], faces: [[cycle, ...], ...]}]}``
    so the client can interpolate each vertex at the current instant -- the same
    piecewise-linear model as moving points.
    """
    if isinstance(value, str) and value in _UNDEF:
        return None
    if not isinstance(value, list):
        return None

    units: list[dict] = []
    for uregion in value:
        try:
            interval, faces = uregion[0], uregion[1]  # type: ignore[index]
            t0 = parse_instant(interval[0])
            t1 = parse_instant(interval[1])
        except (IndexError, TypeError):
            continue
        if t0 is None or t1 is None or not isinstance(faces, list):
            continue

        parsed_faces: list[list[list[list[float]]]] = []
        for face in faces:
            if not isinstance(face, list):
                continue
            cycles: list[list[list[float]]] = []
            for cycle in face:
                if not isinstance(cycle, list):
                    continue
                verts: list[list[float]] = []
                for v in cycle:
                    if isinstance(v, list) and len(v) >= 4:
                        vals = [_num(v[i]) for i in range(4)]
                        if None not in vals:
                            verts.append([float(x) for x in vals])  # type: ignore[arg-type]
                if len(verts) >= 3:
                    cycles.append(verts)
            if cycles:
                parsed_faces.append(cycles)
        if parsed_faces:
            units.append({"interval": [t0, t1], "faces": parsed_faces})

    if not units:
        return None
    return {"units": units, "properties": properties or {}}


def _ureal_value(a: float, b: float, c: float, root: bool, t_days: float) -> float:
    """Evaluate a SECONDO ``ureal`` unit function.

    The unit stores ``(a b c r)`` and the value is ``a*t^2 + b*t + c``, square
    rooted when ``r`` is TRUE. ``t`` is the time since the unit's start **in
    days** -- verified against SECONDO itself (`val(mreal atinstant i)`).
    """
    v = a * t_days * t_days + b * t_days + c
    if root:
        return math.sqrt(max(v, 0.0))  # guard tiny negatives from rounding
    return v


def _plot_units(value: Node) -> list[tuple[float, float, Node]] | None:
    """Units as ``(t0, t1, unit_value)``, resolving unbounded instants
    ("begin of time" / "end of time") to the object's finite extent."""
    if not isinstance(value, list):
        return None
    raw: list[tuple[float | None, float | None, Node]] = []
    for unit in value:
        try:
            interval, uval = unit[0], unit[1]  # type: ignore[index]
            raw.append((parse_instant(interval[0]), parse_instant(interval[1]), uval))
        except (IndexError, TypeError):
            continue
    finite = [t for a, b, _ in raw for t in (a, b) if t is not None]
    if not finite:
        return None
    lo, hi = min(finite), max(finite)
    return [(a if a is not None else lo, b if b is not None else hi, v) for a, b, v in raw]


def scalar_to_plot(type_name: str, value: Node, label: str) -> dict | None:
    """Turn an ``mreal`` / ``mint`` / ``mbool`` into a value-over-time series."""
    if isinstance(value, str) and value in _UNDEF:
        return None
    units = _plot_units(value)
    if not units:
        return None

    series: list[list[float]] = []
    for t0, t1, uval in units:
        if type_name == "mreal":
            try:
                a, b, c = (float(uval[i]) for i in range(3))  # type: ignore[index]
                root = bool(uval[3])  # type: ignore[index]
            except (IndexError, TypeError, ValueError):
                continue
            span_days = (t1 - t0) / _SECONDS_PER_DAY
            # A linear unit only needs its endpoints; a quadratic needs samples.
            n = 1 if a == 0 else _UREAL_SAMPLES
            for i in range(n + 1):
                f = i / n
                series.append([t0 + f * (t1 - t0), _ureal_value(a, b, c, root, f * span_days)])
        else:  # mint / mbool: piecewise constant -> a step
            v = uval
            if isinstance(v, bool):
                v = 1.0 if v else 0.0
            num = _num(v)
            if num is None:
                continue
            series.append([t0, num])
            series.append([t1, num])

    if not series:
        return None
    values = [v for _, v in series]
    return {
        "label": label,
        "kind": "step" if type_name in ("mint", "mbool") else "line",
        "type": type_name,
        "series": series,
        "valueRange": [min(values), max(values)],
        "timeDomain": [series[0][0], series[-1][0]],
    }


def _plot_time_domain(plots: list[dict]) -> list[float] | None:
    if not plots:
        return None
    return [
        min(p["timeDomain"][0] for p in plots),
        max(p["timeDomain"][1] for p in plots),
    ]


def _region_bbox(regions: list[dict]) -> list[float] | None:
    minx = miny = math.inf
    maxx = maxy = -math.inf
    for region in regions:
        for unit in region["units"]:
            for face in unit["faces"]:
                for cycle in face:
                    for x0, y0, x1, y1 in cycle:
                        minx = min(minx, x0, x1)
                        maxx = max(maxx, x0, x1)
                        miny = min(miny, y0, y1)
                        maxy = max(maxy, y0, y1)
    if minx is math.inf:
        return None
    return [minx, miny, maxx, maxy]


def _region_time_domain(regions: list[dict]) -> list[float] | None:
    tmin = math.inf
    tmax = -math.inf
    for region in regions:
        for unit in region["units"]:
            tmin = min(tmin, unit["interval"][0])
            tmax = max(tmax, unit["interval"][1])
    if tmin is math.inf:
        return None
    return [tmin, tmax]


def _merge(a: list[float] | None, b: list[float] | None, bbox: bool) -> list[float] | None:
    """Union two bboxes or two time domains."""
    if a is None:
        return b
    if b is None:
        return a
    if bbox:
        return [min(a[0], b[0]), min(a[1], b[1]), max(a[2], b[2]), max(a[3], b[3])]
    return [min(a[0], b[0]), max(a[1], b[1])]


def _bbox(trips: list[dict]) -> list[float] | None:
    minx = miny = math.inf
    maxx = maxy = -math.inf
    for trip in trips:
        for x, y in trip["path"]:
            minx, miny = min(minx, x), min(miny, y)
            maxx, maxy = max(maxx, x), max(maxy, y)
    if minx is math.inf:
        return None
    return [minx, miny, maxx, maxy]


def _time_domain(trips: list[dict]) -> list[float] | None:
    tmin = math.inf
    tmax = -math.inf
    for trip in trips:
        if trip["timestamps"]:
            tmin = min(tmin, trip["timestamps"][0])
            tmax = max(tmax, trip["timestamps"][-1])
    if tmin is math.inf:
        return None
    return [tmin, tmax]


def _scalar(v: object) -> object | None:
    return v if isinstance(v, (int, float, bool, str)) else None


def _relation_moving(
    type_expr: list, tuples: Node
) -> tuple[list[dict], list[dict], list[dict]]:
    """Collect trips, moving regions and scalar plots from a relation."""
    try:
        attrs = type_expr[1][1]
        names = [a[0] for a in attrs]
        types = [a[1] for a in attrs]
    except (IndexError, TypeError):
        return [], [], []
    moving_idx = [i for i, t in enumerate(types) if t in MOVING_TYPES]
    if not moving_idx or not isinstance(tuples, list):
        return [], [], []

    trips: list[dict] = []
    regions: list[dict] = []
    plots: list[dict] = []
    for row, tup in enumerate(tuples):
        if not isinstance(tup, list):
            continue
        props = {
            names[i]: _scalar(tup[i])
            for i in range(min(len(names), len(tup)))
            if i not in moving_idx and _scalar(tup[i]) is not None
        }
        for i in moving_idx:
            if i >= len(tup):
                continue
            attr_props = {**props, "_attr": names[i]}
            if types[i] in MOVING_POINT_TYPES:
                trips.extend(mpoint_to_trips(tup[i], attr_props))
            elif types[i] in MOVING_REGION_TYPES:
                region = mregion_to_moving(tup[i], attr_props)
                if region:
                    regions.append(region)
            elif types[i] in PLOT_TYPES:
                # Label plots so several rows stay distinguishable.
                label = names[i] if len(tuples) == 1 else f"{names[i]} #{row + 1}"
                plot = scalar_to_plot(types[i], tup[i], label)
                if plot:
                    plots.append(plot)
    return trips, regions, plots


def from_tree(tree: Node) -> dict | None:
    """Turn a parsed ``(type value)`` (or relation) tree into a temporal payload
    ``{trips, regions, timeDomain, bbox}``, or ``None`` if nothing temporal."""
    if not isinstance(tree, list) or len(tree) < 2:
        return None
    type_expr, value = tree[0], tree[1]

    trips: list[dict] = []
    regions: list[dict] = []
    plots: list[dict] = []

    if isinstance(type_expr, str) and type_expr in MOVING_POINT_TYPES:
        trips = mpoint_to_trips(value)
    elif isinstance(type_expr, str) and type_expr in MOVING_REGION_TYPES:
        region = mregion_to_moving(value)
        if region:
            regions = [region]
    elif isinstance(type_expr, str) and type_expr in PLOT_TYPES:
        plot = scalar_to_plot(type_expr, value, type_expr)
        if plot:
            plots = [plot]
    elif isinstance(type_expr, list) and type_expr and type_expr[0] == "rel":
        trips, regions, plots = _relation_moving(type_expr, value)
    else:
        return None

    if not trips and not regions and not plots:
        return None
    time_domain = _merge(_time_domain(trips), _region_time_domain(regions), bbox=False)
    time_domain = _merge(time_domain, _plot_time_domain(plots), bbox=False)
    return {
        "trips": trips,
        "regions": regions,
        "plots": plots,
        "timeDomain": time_domain,
        "bbox": _merge(_bbox(trips), _region_bbox(regions), bbox=True),
    }
