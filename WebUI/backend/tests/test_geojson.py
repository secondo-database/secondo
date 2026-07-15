"""Parser + GeoJSON converter tests.

Inputs are real ``NestedList::ToString`` outputs captured from a live
SecondoMonitor (berlintest); see tests/fixtures/nested_lists.md.
"""
from __future__ import annotations

from app.geojson import to_geojson
from app.nlparser import parse

# --- parser ---------------------------------------------------------------


def test_parse_atoms_and_nesting():
    assert parse("(point (9396.0 9871.0))") == ["point", [9396.0, 9871.0]]
    assert parse("(int 13)") == ["int", 13]
    assert parse("(a (b c) TRUE)") == ["a", ["b", "c"], True]


def test_parse_string_and_symbol():
    assert parse('(x "hello world")') == ["x", "hello world"]


# --- point ----------------------------------------------------------------


def test_point():
    fc = to_geojson("(point (9396.0 9871.0))")
    assert fc["type"] == "FeatureCollection"
    (feat,) = fc["features"]
    assert feat["geometry"] == {"type": "Point", "coordinates": [9396.0, 9871.0]}
    assert fc["bbox"] == [9396.0, 9871.0, 9396.0, 9871.0]


# --- line -----------------------------------------------------------------


def test_line_segments_to_multilinestring():
    text = "(line ((-10849.0 1142.0 -10720.0 454.0) (-10720.0 454.0 -10688.0 243.0)))"
    fc = to_geojson(text)
    geom = fc["features"][0]["geometry"]
    assert geom["type"] == "MultiLineString"
    assert geom["coordinates"][0] == [[-10849.0, 1142.0], [-10720.0, 454.0]]
    assert len(geom["coordinates"]) == 2


# --- region ---------------------------------------------------------------


def test_region_face_with_hole():
    # one face: outer square + inner (hole) square
    text = (
        "(region ("
        "(((0.0 0.0) (10.0 0.0) (10.0 10.0) (0.0 10.0))"
        " ((2.0 2.0) (4.0 2.0) (4.0 4.0) (2.0 4.0)))"
        "))"
    )
    fc = to_geojson(text)
    geom = fc["features"][0]["geometry"]
    assert geom["type"] == "MultiPolygon"
    (poly,) = geom["coordinates"]
    outer, hole = poly
    assert outer[0] == outer[-1]  # closed ring
    assert len(outer) == 5
    assert hole[0] == [2.0, 2.0]


# --- rect -----------------------------------------------------------------


def test_rect():
    fc = to_geojson("(rect (0.0 5.0 0.0 3.0))")
    geom = fc["features"][0]["geometry"]
    assert geom["type"] == "Polygon"
    assert geom["coordinates"][0][0] == [0.0, 0.0]
    assert geom["coordinates"][0][2] == [5.0, 3.0]


# --- relation -------------------------------------------------------------


def test_relation_without_spatial_attr_yields_none():
    fc = to_geojson("((rel (tuple ((Id int) (Line int)))) ((1 1) (2 3)))")
    assert fc is None


def test_relation_with_point_attr():
    text = (
        "((rel (tuple ((Name string) (Pos point)))) "
        '(("a" (1.0 2.0)) ("b" (3.0 4.0))))'
    )
    fc = to_geojson(text)
    assert len(fc["features"]) == 2
    f0 = fc["features"][0]
    assert f0["geometry"] == {"type": "Point", "coordinates": [1.0, 2.0]}
    assert f0["properties"]["Name"] == "a"
    assert f0["properties"]["_attr"] == "Pos"


# --- non-spatial ----------------------------------------------------------


def test_scalar_has_no_geojson():
    assert to_geojson("(int 13)") is None
