"""Tests for moving-object (mpoint) -> trips conversion.

Inputs are in the real ``mpoint`` text form captured from berlintest Trains.
"""
from __future__ import annotations

from app.convert import convert
from app.nlparser import parse
from app.temporal import from_tree, parse_instant

# Two contiguous units of a single train's trip.
MPOINT = (
    "(mpoint ("
    '(("2003-11-20-06:03" "2003-11-20-06:03:52.685" TRUE FALSE)'
    " (13506.0 11159.0 13336.0 10785.0))"
    '(("2003-11-20-06:03:52.685" "2003-11-20-06:04:08" TRUE FALSE)'
    " (13336.0 10785.0 13287.0 10675.0))"
    "))"
)


def test_parse_instant_formats():
    a = parse_instant("2003-11-20-06:03")
    b = parse_instant("2003-11-20-06:03:52.685")
    assert a is not None and b is not None
    assert abs((b - a) - 52.685) < 1e-6  # 52.685 s apart


def test_mpoint_to_single_trip():
    payload = from_tree(parse(MPOINT))
    assert payload is not None
    (trip,) = payload["trips"]
    # 2 contiguous units -> 3 waypoints / 3 timestamps
    assert trip["path"] == [
        [13506.0, 11159.0],
        [13336.0, 10785.0],
        [13287.0, 10675.0],
    ]
    assert len(trip["timestamps"]) == 3
    assert trip["timestamps"][0] < trip["timestamps"][1] < trip["timestamps"][2]


def test_time_domain_and_bbox():
    payload = from_tree(parse(MPOINT))
    ts = payload["trips"][0]["timestamps"]
    assert payload["timeDomain"] == [ts[0], ts[-1]]
    assert payload["bbox"] == [13287.0, 10675.0, 13506.0, 11159.0]


def test_temporal_gap_splits_into_two_trips():
    # second unit starts an hour after the first ends -> separate trips
    gapped = (
        "(mpoint ("
        '(("2003-11-20-06:00" "2003-11-20-06:01" TRUE FALSE) (0.0 0.0 1.0 1.0))'
        '(("2003-11-20-07:00" "2003-11-20-07:01" TRUE FALSE) (5.0 5.0 6.0 6.0))'
        "))"
    )
    payload = from_tree(parse(gapped))
    assert len(payload["trips"]) == 2


def test_relation_of_mpoint():
    rel = (
        "((rel (tuple ((Id int) (Trip mpoint)))) ("
        '(1 (('
        '("2003-11-20-06:00" "2003-11-20-06:01" TRUE FALSE) (0.0 0.0 1.0 1.0))))'
        "))"
    )
    payload = from_tree(parse(rel))
    assert len(payload["trips"]) == 1
    assert payload["trips"][0]["properties"]["Id"] == 1
    assert payload["trips"][0]["properties"]["_attr"] == "Trip"


def test_convert_returns_every_channel_independently():
    # `convert` takes the tree the bridge hands over, so these parse the text
    # themselves -- in the running system nothing does.
    geo, temp, tab = convert(parse("(point (1.0 2.0))"))
    assert geo is not None and temp is None and tab is None
    geo, temp, tab = convert(parse(MPOINT))
    assert geo is None and temp is not None and tab is None
    # A relation of scalars has no geometry and no motion -- only rows.
    geo, temp, tab = convert(parse("((rel (tuple ((No int)))) ((1)))"))
    assert geo is None and temp is None and tab is not None


# --- moving regions (Milestone 6) ------------------------------------------

# One unit, one face, one triangular cycle translating +10 east / +5 north.
MREGION = (
    "(mregion ("
    '(("2003-11-20-06:00" "2003-11-20-07:00" TRUE TRUE)'
    " ((((0.0 0.0 10.0 5.0) (4.0 0.0 14.0 5.0) (0.0 3.0 10.0 8.0))))"
    ")))"
)


def test_mregion_units_and_vertices():
    payload = from_tree(parse(MREGION))
    assert payload is not None
    assert payload["trips"] == []
    (region,) = payload["regions"]
    (unit,) = region["units"]
    (face,) = unit["faces"]
    (cycle,) = face
    # moving vertices keep both endpoints so the client can interpolate
    assert cycle[0] == [0.0, 0.0, 10.0, 5.0]
    assert len(cycle) == 3
    assert unit["interval"][0] < unit["interval"][1]


def test_mregion_bbox_spans_start_and_end_positions():
    payload = from_tree(parse(MREGION))
    # bbox must cover the region across its whole motion, not just t0
    assert payload["bbox"] == [0.0, 0.0, 14.0, 8.0]


def test_mregion_time_domain():
    payload = from_tree(parse(MREGION))
    unit = payload["regions"][0]["units"][0]
    assert payload["timeDomain"] == [unit["interval"][0], unit["interval"][1]]


def test_relation_mixing_mpoint_and_mregion():
    rel = (
        "((rel (tuple ((Id int) (Trip mpoint) (Area mregion)))) ("
        '(1 ((("2003-11-20-06:00" "2003-11-20-06:01" TRUE FALSE) (0.0 0.0 1.0 1.0)))'
        '   ((("2003-11-20-06:00" "2003-11-20-07:00" TRUE TRUE)'
        "     ((((0.0 0.0 10.0 5.0) (4.0 0.0 14.0 5.0) (0.0 3.0 10.0 8.0))))))"
        ")))"
    )
    payload = from_tree(parse(rel))
    assert len(payload["trips"]) == 1
    assert len(payload["regions"]) == 1
    assert payload["regions"][0]["properties"]["_attr"] == "Area"
    assert payload["trips"][0]["properties"]["_attr"] == "Trip"


def test_mpoint_payload_still_has_empty_regions():
    payload = from_tree(parse(MPOINT))
    assert payload["regions"] == []


# --- scalar value plots: mreal / mint (Milestone 6) -------------------------


def test_mreal_constant():
    payload = from_tree(parse('(mreal ((("2003-11-20" "2003-11-21" TRUE TRUE)'
                              " (0.0 0.0 5000.0 FALSE))))"))
    (plot,) = payload["plots"]
    assert plot["kind"] == "line"
    assert plot["valueRange"] == [5000.0, 5000.0]  # mreal5000 really is constant
    assert all(v == 5000.0 for _, v in plot["series"])


def test_mreal_sqrt_matches_secondo_ground_truth():
    # Real unit 0 of `distance(train7, mehringdamm)` from berlintest. SECONDO's
    # own val(... atinstant ...) gives 11376.19382746268 at the start and
    # 11177.34248379283 at the end -- t is measured in DAYS.
    text = (
        '(mreal ((("2003-11-20-06:06" "2003-11-20-06:06:08.692" TRUE FALSE)'
        " (3965029173458.559 -44978595490.10584 129417786.0 TRUE))))"
    )
    plot = from_tree(parse(text))["plots"][0]
    first_v = plot["series"][0][1]
    last_v = plot["series"][-1][1]
    assert abs(first_v - 11376.19382746268) < 1e-4
    assert abs(last_v - 11177.34248379283) < 1e-4


def test_mreal_quadratic_is_sampled_not_just_endpoints():
    text = (
        '(mreal ((("2003-11-20-06:06" "2003-11-20-06:06:08.692" TRUE FALSE)'
        " (3965029173458.559 -44978595490.10584 129417786.0 TRUE))))"
    )
    plot = from_tree(parse(text))["plots"][0]
    assert len(plot["series"]) > 2  # curved unit needs intermediate samples


def test_mint_is_a_step_series_and_handles_begin_of_time():
    text = (
        '(mint ((("begin of time" "2003-11-20-06:18:16.027" TRUE FALSE) 0)'
        ' (("2003-11-20-06:18:16.027" "2003-11-20-06:35:29.353" TRUE FALSE) 1)'
        ' (("2003-11-20-06:35:29.353" "2003-11-20-06:37:30.647" TRUE TRUE) 2)))'
    )
    payload = from_tree(parse(text))
    (plot,) = payload["plots"]
    assert plot["kind"] == "step"
    assert plot["valueRange"] == [0.0, 2.0]
    # the unbounded "begin of time" is resolved to the finite extent, not dropped
    assert all(t == t and abs(t) != float("inf") for t, _ in plot["series"])
    assert len(plot["series"]) == 6  # two points per unit


def test_plot_only_object_has_no_bbox_but_has_a_time_domain():
    payload = from_tree(parse('(mreal ((("2003-11-20" "2003-11-21" TRUE TRUE)'
                              " (0.0 0.0 5000.0 FALSE))))"))
    assert payload["bbox"] is None
    assert payload["timeDomain"][0] < payload["timeDomain"][1]
    assert payload["trips"] == [] and payload["regions"] == []


def test_parse_instant_date_only():
    # mreal5000 uses date-only instants; these must not be rejected.
    a = parse_instant("2003-11-20")
    b = parse_instant("2003-11-21")
    assert a is not None and b is not None
    assert abs((b - a) - 86400) < 1e-6


def test_parse_instant_rejects_unbounded_markers():
    assert parse_instant("begin of time") is None
    assert parse_instant("end of time") is None


# --- symbolic trajectories: mlabel / mstring (Milestone 12) -----------------

# Two units of a map-matched hiking track's road type, in the shape
# `omapmatchmht ... makemvalue` produces.
LABEL_UNITS = (
    '(("2026-08-09-08:24:11.448" "2026-08-09-09:10:18.140" TRUE FALSE) "secondary")'
    '(("2026-08-09-09:10:18.140" "2026-08-09-09:40:00" TRUE FALSE) "footway")'
)
MLABEL = f"(mlabel ({LABEL_UNITS}))"

# The driving case: one moving point with the road type it was matched onto.
# `_payload` drops a label with no moving point, so every label test that wants
# a payload back has to pair it with one.
TRIP_UNIT = (
    '(("2026-08-09-08:24:11.448" "2026-08-09-09:40:00" TRUE FALSE)'
    " (10.11 53.60 10.05 53.57))"
)


def _rel(schema: str, *tuples: str) -> str:
    """A relation of a Trip plus whatever `schema` adds, as parseable text."""
    return (
        f"((rel (tuple ((Trip mpoint) {schema}))) "
        f"({' '.join(tuples)}))"
    )


def _labelled(*attrs: str, rows: int = 1) -> str:
    """A relation of a Trip plus one mlabel per name in `attrs`, each holding a
    single unit whose text is the attribute's own name."""
    schema = " ".join(f"({a} mlabel)" for a in attrs)
    values = " ".join(
        f'((("2026-08-09-08:24:11.448" "2026-08-09-09:40:00" TRUE FALSE) "{a}"))'
        for a in attrs
    )
    return _rel(schema, *(f"(({TRIP_UNIT}) {values})" for _ in range(rows)))


def test_mlabel_series_shape():
    payload = from_tree(parse(_rel("(RoadType mlabel)",
                                   f"(({TRIP_UNIT}) ({LABEL_UNITS}))")))
    (series,) = payload["labels"]
    assert series["attr"] == "RoadType"
    assert series["type"] == "mlabel"
    assert series["row"] == 0
    assert [u[2] for u in series["units"]] == ["secondary", "footway"]
    # A label covers a span: the domain ends at the last interval's *end*.
    assert series["timeDomain"] == [series["units"][0][0], series["units"][-1][1]]


def test_mstring_is_the_same_shape_as_mlabel():
    def payload_of(type_name: str) -> dict:
        text = _rel(f"(Road {type_name})", f"(({TRIP_UNIT}) ({LABEL_UNITS}))")
        return from_tree(parse(text))["labels"][0]

    a, b = payload_of("mlabel"), payload_of("mstring")
    assert b.pop("type") == "mstring" and a.pop("type") == "mlabel"
    assert a == b


def test_equal_adjacent_labels_merge_into_one_interval():
    units = (
        '(("2026-08-09-08:00" "2026-08-09-08:10" TRUE FALSE) "footway")'
        '(("2026-08-09-08:10" "2026-08-09-08:20" TRUE FALSE) "footway")'
        '(("2026-08-09-08:20" "2026-08-09-08:30" TRUE FALSE) "residential")'
    )
    text = _rel("(RoadType mlabel)", f"(({TRIP_UNIT}) ({units}))")
    (series,) = from_tree(parse(text))["labels"]
    assert [u[2] for u in series["units"]] == ["footway", "residential"]


def test_a_real_gap_is_not_merged_away():
    """Two runs of the same label an hour apart stay two intervals -- otherwise
    the label would appear to persist across a stretch it never covered."""
    units = (
        '(("2026-08-09-08:00" "2026-08-09-08:10" TRUE FALSE) "footway")'
        '(("2026-08-09-09:00" "2026-08-09-09:10" TRUE FALSE) "footway")'
    )
    text = _rel("(RoadType mlabel)", f"(({TRIP_UNIT}) ({units}))")
    (series,) = from_tree(parse(text))["labels"]
    assert len(series["units"]) == 2


def test_trailing_whitespace_is_trimmed():
    """Shapefile imports are fixed-width padded: `roads.Fclass` is 28 characters
    for a 7-character road type."""
    units = f'(("2026-08-09-08:00" "2026-08-09-08:10" TRUE FALSE) "footway{" " * 21}")'
    text = _rel("(RoadType mlabel)", f"(({TRIP_UNIT}) ({units}))")
    (series,) = from_tree(parse(text))["labels"]
    assert series["units"][0][2] == "footway"


def test_undefined_and_empty_labels_yield_no_series():
    for value in ("undefined", "()"):
        text = _rel("(RoadType mlabel)", f"(({TRIP_UNIT}) {value})")
        assert from_tree(parse(text))["labels"] == []


def test_a_label_alone_is_not_a_payload():
    """It would draw nothing while widening the animation domain every other
    layer shares."""
    assert from_tree(parse(MLABEL)) is None


def test_relation_pairs_a_trip_and_a_label_by_row():
    payload = from_tree(parse(_labelled("RoadType")))
    assert payload["trips"][0]["properties"]["_row"] == 0
    assert payload["labels"][0]["row"] == 0
    assert payload["labels"][0]["attr"] == "RoadType"


def test_several_label_attributes_come_out_in_schema_order():
    payload = from_tree(parse(_labelled("RoadType", "RoadName")))
    assert [s["attr"] for s in payload["labels"]] == ["RoadType", "RoadName"]
    assert {s["row"] for s in payload["labels"]} == {0}


def test_two_rows_keep_their_own_labels():
    payload = from_tree(parse(_labelled("RoadType", rows=2)))
    assert [s["row"] for s in payload["labels"]] == [0, 1]
    assert [t["properties"]["_row"] for t in payload["trips"]] == [0, 1]


def test_a_row_with_an_undefined_label_does_not_shift_the_others():
    """The row number has to come from the tuple counter, not from how many
    series have been collected -- otherwise row 2's label lands on row 1."""
    tup = f"(({TRIP_UNIT}) ({LABEL_UNITS}))"
    undef = f"(({TRIP_UNIT}) undefined)"
    payload = from_tree(parse(_rel("(RoadType mlabel)", tup, undef, tup)))
    assert [s["row"] for s in payload["labels"]] == [0, 2]


def test_gap_split_trips_share_one_label_series():
    gapped = (
        '(("2026-08-09-08:00" "2026-08-09-08:01" TRUE FALSE) (0.0 0.0 1.0 1.0))'
        '(("2026-08-09-09:00" "2026-08-09-09:01" TRUE FALSE) (5.0 5.0 6.0 6.0))'
    )
    text = _rel("(RoadType mlabel)", f"(({gapped}) ({LABEL_UNITS}))")
    payload = from_tree(parse(text))
    assert len(payload["trips"]) == 2
    assert {t["properties"]["_row"] for t in payload["trips"]} == {0}
    assert len(payload["labels"]) == 1


def test_time_domain_covers_a_label_outliving_its_trip():
    short = '(("2026-08-09-08:00" "2026-08-09-08:01" TRUE FALSE) (0.0 0.0 1.0 1.0))'
    late = '(("2026-08-09-08:00" "2026-08-09-10:00" TRUE FALSE) "footway")'
    text = _rel("(RoadType mlabel)", f"(({short}) ({late}))")
    payload = from_tree(parse(text))
    assert payload["timeDomain"][1] == payload["labels"][0]["timeDomain"][1]
    assert payload["timeDomain"][1] > payload["trips"][0]["timestamps"][-1]


def test_a_label_contributes_no_bbox():
    payload = from_tree(parse(_labelled("RoadType")))
    assert payload["bbox"] == [10.05, 53.57, 10.11, 53.60]


def test_mpoint_payload_still_has_empty_labels():
    assert from_tree(parse(MPOINT))["labels"] == []
