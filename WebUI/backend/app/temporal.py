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
# Symbolic trajectories: a text value over time. They draw nothing of their own
# -- they are written next to the moving point of the same tuple.
LABEL_TYPES = {"mlabel", "mstring"}
MOVING_TYPES = (
    MOVING_POINT_TYPES | MOVING_REGION_TYPES | PLOT_TYPES | LABEL_TYPES
)

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


def _label_text(v: object) -> str | None:
    """The text of one mlabel/mstring unit, or None when there is none.

    Trailing whitespace goes, for the reason `geojson._scalar` gives: it is
    fixed-width padding out of the source data -- a shapefile import gives every
    ``roads`` tuple a 28-character ``Fclass``, so the label for a footway is
    ``"footway"`` followed by 21 spaces -- and it was never visible on the map.
    Left in, it also makes the value compare unequal to the obvious
    ``tolabel("footway")``, which is a confusing thing to hand someone.
    """
    if not isinstance(v, str):
        return None
    return v.rstrip() or None


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


def mlabel_to_series(
    type_name: str, value: Node, attr: str, row: int = 0
) -> dict | None:
    """Turn an ``mlabel`` / ``mstring`` into a time-indexed text series.

        (mlabel ( unit* ))
        unit = ( interval "footway" )

    The interval is the one every moving type uses, so this shares
    ``_plot_units`` with the scalar plots. What differs is the value: text
    cannot be plotted, so instead of a series of samples this emits the
    intervals themselves as ``[t0, t1, text]`` and the frontend writes the text
    next to the moving point that shares its ``row``.

    Piecewise *constant*, unlike an mpoint: there is nothing to interpolate
    between two labels, and a gap between two units means the label is simply
    not defined there rather than that it slid from one value to the next.
    """
    if isinstance(value, str) and value in _UNDEF:
        return None
    units = _plot_units(value)
    if not units:
        return None

    out: list[list] = []
    for t0, t1, uval in units:
        text = _label_text(uval)
        if text is None:
            continue
        # SECONDO splits a symbolic trajectory where its *source* changes -- a
        # network edge, for a map-matched track -- not where the label does, so
        # one road type spans long runs of units. Merging the runs is the
        # difference between a handful of intervals and a few thousand, and the
        # frontend rescans them on every animation frame. Only contiguous runs:
        # a real gap has to survive, or the label would appear to persist
        # across a stretch where it was never defined.
        if out and out[-1][2] == text and t0 - out[-1][1] <= _GAP_EPS:
            out[-1][1] = t1
            continue
        out.append([t0, t1, text])

    if not out:
        return None
    return {
        "attr": attr,
        "type": type_name,
        "row": row,
        "units": out,
        # The end of the last interval, not its start: a label covers a span,
        # where a plot's series is a list of instants.
        "timeDomain": [out[0][0], out[-1][1]],
    }


def _series_time_domain(items: list[dict]) -> list[float] | None:
    """Union of the ``timeDomain`` of every plot or label series."""
    if not items:
        return None
    return [
        min(p["timeDomain"][0] for p in items),
        max(p["timeDomain"][1] for p in items),
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


class RelationMoving:
    """Collects a relation's trips, moving regions, plots and symbolic label
    series, one tuple at a time.

    Split out of ``_relation_moving`` for the same reason as
    ``geojson.RelationFeatures``: so a caller that receives the tuples one by
    one never has to hold them all (see ``app/convert.py``).
    """

    __slots__ = ("names", "types", "moving_idx", "trips", "regions", "plots",
                 "labels", "_bare_labels", "_rows")

    def __init__(self, type_expr: Node) -> None:
        try:
            attrs = type_expr[1][1]  # type: ignore[index]
            self.names = [a[0] for a in attrs]
            self.types = [a[1] for a in attrs]
        except (IndexError, TypeError):
            self.names = []
            self.types = []
        self.moving_idx = [
            i for i, t in enumerate(self.types) if t in MOVING_TYPES
        ]
        self.trips: list[dict] = []
        self.regions: list[dict] = []
        self.plots: list[dict] = []
        # In relation-schema order within a row, and in tuple order across
        # rows, because `moving_idx` is ascending and `feed` walks it in order.
        # The frontend stacks a row's labels in this order, so it is a contract
        # rather than an accident.
        self.labels: list[dict] = []
        # A plot's label carries a row number only when there is more than one
        # row, and a streaming caller does not know that until the end. The
        # numbered label is written as it goes and the bare one kept beside it,
        # so the one-row case can still drop the number (see `finish`).
        self._bare_labels: list[str] = []
        self._rows = 0

    @property
    def wanted(self) -> bool:
        """Whether this relation has a moving attribute at all; see
        ``geojson.RelationFeatures.wanted``."""
        return bool(self.moving_idx)

    def feed(self, tup: Node) -> None:
        row = self._rows
        self._rows += 1
        if not isinstance(tup, list):
            return
        names, types, moving_idx = self.names, self.types, self.moving_idx
        props = {
            names[i]: _scalar(tup[i])
            for i in range(min(len(names), len(tup)))
            if i not in moving_idx and _scalar(tup[i]) is not None
        }
        for i in moving_idx:
            if i >= len(tup):
                continue
            # `_row` is what ties a trip back to the symbolic labels of the
            # same tuple: they are converted independently into separate lists
            # and there is otherwise nothing shared between them.
            attr_props = {**props, "_attr": names[i], "_row": row}
            if types[i] in MOVING_POINT_TYPES:
                self.trips.extend(mpoint_to_trips(tup[i], attr_props))
            elif types[i] in MOVING_REGION_TYPES:
                region = mregion_to_moving(tup[i], attr_props)
                if region:
                    self.regions.append(region)
            elif types[i] in PLOT_TYPES:
                # Label plots so several rows stay distinguishable.
                plot = scalar_to_plot(
                    types[i], tup[i], f"{names[i]} #{row + 1}"
                )
                if plot:
                    self.plots.append(plot)
                    self._bare_labels.append(names[i])
            elif types[i] in LABEL_TYPES:
                # `row`, not `len(self.labels)`: a row whose label is undefined
                # produces no series, and the rows after it have to keep their
                # own numbers or they would be drawn on the wrong trip.
                series = mlabel_to_series(types[i], tup[i], names[i], row)
                if series:
                    self.labels.append(series)

    def finish(self) -> tuple[list[dict], list[dict], list[dict], list[dict]]:
        if self._rows == 1:
            for plot, label in zip(self.plots, self._bare_labels):
                plot["label"] = label
        return self.trips, self.regions, self.plots, self.labels

    def payload(self) -> dict | None:
        """The temporal payload for everything fed so far, or None if nothing
        temporal came out of it."""
        return _payload(*self.finish())


def _relation_moving(
    type_expr: list, tuples: Node
) -> tuple[list[dict], list[dict], list[dict], list[dict]]:
    """Collect trips, moving regions, scalar plots and label series."""
    collector = RelationMoving(type_expr)
    if not collector.wanted or not isinstance(tuples, list):
        return [], [], [], []
    for tup in tuples:
        collector.feed(tup)
    return collector.finish()


def _payload(
    trips: list[dict],
    regions: list[dict],
    plots: list[dict],
    labels: list[dict],
) -> dict | None:
    # Labels alone are deliberately not a payload. They are only ever drawn
    # beside a moving point, so a result holding nothing else would make a
    # layer that renders nothing while still widening the animation domain
    # every other layer shares -- rescaling their timeline to show nothing.
    if not trips and not regions and not plots:
        return None
    time_domain = _merge(_time_domain(trips), _region_time_domain(regions), bbox=False)
    time_domain = _merge(time_domain, _series_time_domain(plots), bbox=False)
    time_domain = _merge(time_domain, _series_time_domain(labels), bbox=False)
    return {
        "trips": trips,
        "regions": regions,
        "plots": plots,
        "labels": labels,
        "timeDomain": time_domain,
        # A label has no geometry, so it contributes no bbox.
        "bbox": _merge(_bbox(trips), _region_bbox(regions), bbox=True),
    }


def from_tree(tree: Node) -> dict | None:
    """Turn a parsed ``(type value)`` (or relation) tree into a temporal payload
    ``{trips, regions, plots, labels, timeDomain, bbox}``, or ``None`` if
    nothing temporal came out of it."""
    if not isinstance(tree, list) or len(tree) < 2:
        return None
    type_expr, value = tree[0], tree[1]

    trips: list[dict] = []
    regions: list[dict] = []
    plots: list[dict] = []
    labels: list[dict] = []

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
    elif isinstance(type_expr, str) and type_expr in LABEL_TYPES:
        # A standalone object has no attribute name, so the type stands in --
        # the same choice `scalar_to_plot` makes. `_payload` then drops it for
        # want of anything to draw it against; the branch is here so the shape
        # is understood rather than falling through to "not temporal".
        series = mlabel_to_series(type_expr, value, type_expr)
        if series:
            labels = [series]
    elif isinstance(type_expr, list) and type_expr and type_expr[0] == "rel":
        trips, regions, plots, labels = _relation_moving(type_expr, value)
    else:
        return None

    return _payload(trips, regions, plots, labels)
