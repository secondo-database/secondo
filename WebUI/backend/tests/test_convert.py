"""Every way of reading an answer produces the same payloads.

An answer can reach `Answer` three ways -- pushed tuple by tuple as the client
decodes it, handed over as a type half and a tuple iterator, or handed over
whole -- and which one it is depends on the transfer mode and the shape of the
result, neither of which the payloads may depend on. These tests pin all three
against `convert`, the whole-tree path the fixture tests use.
"""
from __future__ import annotations

import pytest

from app.convert import Answer, convert
from app.nlparser import parse
from nativefake import answer as bridge_answer

# Two contiguous units of one train's trip; the same literal test_temporal
# uses, so the trip these produce is one already asserted in detail there.
UNITS = (
    '(("2003-11-20-06:03" "2003-11-20-06:03:52.685" TRUE FALSE)'
    " (13506.0 11159.0 13336.0 10785.0))"
    '(("2003-11-20-06:03:52.685" "2003-11-20-06:04:08" TRUE FALSE)'
    " (13336.0 10785.0 13287.0 10675.0))"
)

# One relation with something for each of the three payloads: a point for the
# GeoJSON, an mpoint for the trips, and a name for the grid.
MIXED = (
    "((rel (tuple ((Name string) (Pos point) (Trip mpoint)))) ("
    f'("Alpha" (1.0 2.0) ({UNITS}))'
    f'("Beta" (3.0 4.0) ({UNITS}))'
    "))"
)

PLAIN = '((rel (tuple ((No int) (Name string)))) ((1 "a") (2 "b") (3 "c")))'

# Not relations. `point` still has the (type value) shape the reader streams
# by, so it goes down the push path too and has to survive being taken apart
# into two reals and put back together.
POINT = "(point (9396.0 9871.0))"
COUNT = "(int 212099)"
MPOINT = f"(mpoint ({UNITS}))"

# Symbolic trajectories. A label is only ever drawn beside a moving point, so
# the shape that matters is the pairing -- and with two of them, since the row
# they share is what puts both lines on the same dot.
LABEL_UNITS = (
    '(("2003-11-20-06:03" "2003-11-20-06:03:52.685" TRUE FALSE) "footway")'
    '(("2003-11-20-06:03:52.685" "2003-11-20-06:04:08" TRUE FALSE) "residential")'
)
NAME_UNITS = (
    '(("2003-11-20-06:03" "2003-11-20-06:04:08" TRUE FALSE) "Wandsestrasse")'
)
SYMBOLIC = (
    "((rel (tuple ((Trip mpoint) (RoadType mlabel) (RoadName mstring)))) ("
    f"(({UNITS}) ({LABEL_UNITS}) ({NAME_UNITS}))"
    "))"
)

SHAPES = [MIXED, PLAIN, POINT, COUNT, MPOINT, SYMBOLIC]


def read(text: str, *, page=None, table_only=False, can_stream=True):
    """One answer through the bridge and out as payloads, the way /api/query
    does it. `nativefake.answer` applies the same rules the C++ does."""
    a = Answer(table_only=table_only)
    raw = bridge_answer(parse(text), False, a, can_stream=can_stream)
    if not raw["streamed"]:
        a.read(raw["type"], raw["tuples"], raw["tree"])
    return a.payloads(page=page), raw["streamed"]


def whole(text: str, *, page=None):
    return convert(parse(text), page=page)


@pytest.mark.parametrize("text", SHAPES)
@pytest.mark.parametrize("page", [None, 2])
def test_pushed_matches_whole_tree(text, page):
    got, _ = read(text, page=page)
    assert got == whole(text, page=page)


@pytest.mark.parametrize("text", SHAPES)
@pytest.mark.parametrize("page", [None, 2])
def test_split_matches_whole_tree(text, page):
    """The same answer over a connection whose results the reader cannot
    stream: it arrives as a tuple iterator, or whole."""
    got, streamed = read(text, page=page, can_stream=False)
    assert streamed is False
    assert got == whole(text, page=page)


def test_a_relation_really_is_pushed():
    _, streamed = read(PLAIN)
    assert streamed is True


def test_symbolic_labels_survive_the_push_path():
    """Not just equal to the whole-tree path: the row that ties a label to its
    trip is assigned as the tuples go by, so it is the pushed path that can get
    it wrong."""
    (_, temporal, _), streamed = read(SYMBOLIC)
    assert streamed is True
    assert [s["attr"] for s in temporal["labels"]] == ["RoadType", "RoadName"]
    assert {s["row"] for s in temporal["labels"]} == {0}
    assert temporal["trips"][0]["properties"]["_row"] == 0


def test_pushing_never_holds_more_than_one_tuple():
    """Not just equal output: nothing may keep the tuples. The grid keeps the
    first page of them on purpose (table.RelationRows), so this asks the two
    payloads that must not."""
    a = Answer()
    a.begin(parse(MIXED)[0])
    for tup in parse(MIXED)[1]:
        a.elem(tup)
    # Features and trips are derived values; the tuples they came from are not
    # referenced by anything the collectors kept.
    assert a.features is not None and len(a.features.features) == 2
    assert a.moving is not None and len(a.moving.trips) == 2
    assert len(a.rows.kept) == 2  # the page, and only because it is the page


def test_table_only_builds_only_the_table():
    (geo, temporal, tabular), _ = read(MIXED, table_only=True)
    assert geo is None and temporal is None
    assert tabular is not None and tabular["rowCount"] == 2
    # ... and the same answer read in full does have the other two, so the
    # difference is the request, not the data.
    assert all(p is not None for p in read(MIXED)[0])


def test_a_relation_with_nothing_spatial_yields_no_geojson():
    (geo, temporal, tabular), _ = read(PLAIN)
    assert geo is None
    assert temporal is None
    assert tabular["rowCount"] == 3


def test_total_survives_the_row_cap():
    """A page is cut from what was kept, but the total counts every tuple --
    the grid's pager reads it, and the tuples past the cap were never held."""
    rows = 2500
    text = "((rel (tuple ((No int)))) (" + " ".join(
        f"({i})" for i in range(rows)
    ) + "))"
    (_, _, paged), _ = read(text, page=200)
    assert paged["rowCount"] == 200
    assert paged["totalRows"] == rows
    assert paged["truncated"] is False  # every row is one Next away

    (_, _, capped), _ = read(text)
    assert capped["rowCount"] == 1000  # table.MAX_ROWS
    assert capped["totalRows"] == rows
    assert capped["truncated"] is True


def test_a_failing_payload_never_fails_the_command():
    """Reading now happens inside the bridge call, where a raise would abort a
    command that had in fact succeeded. It must come back as no payload."""
    a = Answer()
    a.begin(parse(MIXED)[0])

    def explode(_tup):
        raise ValueError("a payload builder gave up")

    a._feeds = [explode]
    a.elem(parse(MIXED)[1][0])
    assert a.payloads() == (None, None, None)


def test_a_tuple_that_is_not_a_tuple_is_skipped():
    """Malformed rows are dropped, not fatal -- the same as the whole-tree
    path, which filters them out of the loop."""
    a = Answer()
    a.begin(parse(PLAIN)[0])
    for tup in [parse(PLAIN)[1][0], "not a tuple", parse(PLAIN)[1][1]]:
        a.elem(tup)
    assert a.payloads()[2]["rowCount"] == 2
    assert a.payloads()[2]["totalRows"] == 3
