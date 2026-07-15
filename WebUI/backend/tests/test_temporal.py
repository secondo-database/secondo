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


def test_convert_returns_both_channels_independently():
    geo, temp = convert("(point (1.0 2.0))")
    assert geo is not None and temp is None
    geo, temp = convert(MPOINT)
    assert geo is None and temp is not None


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
