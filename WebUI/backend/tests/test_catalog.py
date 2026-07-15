"""Tests for `list objects` parsing and the sline fix (Milestone 5).

Inputs are real captures from SYMTRAJSMALL / berlintest.
"""
from __future__ import annotations

from app.catalog import parse_objects
from app.geojson import to_geojson

OBJECTS = (
    "(inquiry (objects (OBJECTS "
    "(OBJECT Part () ((rel (tuple ((Moid int) (MP mpoint) (ML mlabels)))))) "
    "(OBJECT EdgesExtDo () ((rel (tuple ((WayId longint) (Segment sline)))))) "
    "(OBJECT thecenter () ((region))) "
    "(OBJECT Orte () ((rel (tuple ((Ort string)))))) "
    ")))"
)


def test_parse_objects_names_sorted():
    objs = parse_objects(OBJECTS)
    names = [o["name"] for o in objs]
    assert names == ["EdgesExtDo", "Orte", "Part", "thecenter"]


def test_parse_objects_kinds():
    by_name = {o["name"]: o for o in parse_objects(OBJECTS)}
    assert by_name["Part"]["kind"] == "temporal"  # has mpoint
    assert by_name["EdgesExtDo"]["kind"] == "spatial"  # has sline
    assert by_name["thecenter"]["kind"] == "spatial"  # region
    assert by_name["Orte"]["kind"] == "other"  # plain relation
    assert by_name["Part"]["type"] == "rel"


def test_sline_wrapped_with_direction_flag():
    # sline value is ( (segments) bool ) -- must not be treated as segments.
    text = "(sline (((7.4699 51.5132 7.4699 51.5124) (7.47 51.51 7.48 51.52)) TRUE))"
    fc = to_geojson(text)
    geom = fc["features"][0]["geometry"]
    assert geom["type"] == "MultiLineString"
    assert len(geom["coordinates"]) == 2
    assert geom["coordinates"][0][0] == [7.4699, 51.5132]


def test_plain_line_still_works():
    text = "(line ((0.0 0.0 1.0 1.0) (1.0 1.0 2.0 2.0)))"
    geom = to_geojson(text)["features"][0]["geometry"]
    assert geom["type"] == "MultiLineString"
    assert len(geom["coordinates"]) == 2
