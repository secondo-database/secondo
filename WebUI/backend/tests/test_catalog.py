"""Tests for `list objects` parsing and the sline fix (Milestone 5).

Inputs are real captures from SYMTRAJSMALL / berlintest.
"""
from __future__ import annotations

from pathlib import Path

from app.catalog import parse_objects, parse_operators
from app.nlparser import parse
from app.geojson import to_geojson

# A slice of a real `list operators` from this build -- the entries picked to
# cover what varies: an operator registered twice (abs, feed), one whose name
# the editor can never type (#, +), and the paging operators the table view
# depends on.
LIST_OPERATORS = (Path(__file__).parent / "fixtures" / "list_operators.nl").read_text()

OBJECTS = (
    "(inquiry (objects (OBJECTS "
    "(OBJECT Part () ((rel (tuple ((Moid int) (MP mpoint) (ML mlabels)))))) "
    "(OBJECT EdgesExtDo () ((rel (tuple ((WayId longint) (Segment sline)))))) "
    "(OBJECT thecenter () ((region))) "
    "(OBJECT Orte () ((rel (tuple ((Ort string)))))) "
    ")))"
)


def test_parse_objects_names_sorted():
    objs = parse_objects(parse(OBJECTS))
    names = [o["name"] for o in objs]
    assert names == ["EdgesExtDo", "Orte", "Part", "thecenter"]


def test_parse_objects_kinds():
    by_name = {o["name"]: o for o in parse_objects(parse(OBJECTS))}
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


# --- list operators --------------------------------------------------------


def test_operators_are_named_and_deduplicated():
    """One entry per operator, whatever it is registered as. `abs` and `feed`
    are in the catalog twice each (different algebras); a completion menu wants
    them once."""
    ops = parse_operators(parse(LIST_OPERATORS))
    names = [o["name"] for o in ops]
    assert names.count("abs") == 1
    assert names.count("feed") == 1
    assert names == sorted(names, key=str.lower)


def test_operators_carry_their_syntax():
    """The line the menu shows next to the name, whitespace-collapsed: a
    Signature runs over several lines in more than one algebra."""
    syntax = {o["name"]: o["syntax"] for o in parse_operators(parse(LIST_OPERATORS))}
    assert syntax["createsuffixtree"] == "createsuffixtree (_)"
    assert syntax["feed"] == "_ feed"
    assert syntax["addcounter"] == "stream addcounter[AttrName, Initial]"


def test_untypeable_operator_names_are_left_out():
    """`#` and `+` are real operators, but the editor's token regex cannot
    produce a word for them, so offering them would only pad the response."""
    names = {o["name"] for o in parse_operators(parse(LIST_OPERATORS))}
    assert "#" not in names and "+" not in names


def test_a_missing_field_loses_the_field_not_the_entry():
    """The label set is up to the algebra, so labels and values are zipped
    rather than indexed."""
    ops = parse_operators(parse(
        "(inquiry (operators ((odd (\"Meaning\") ('does something')))))"
    ))
    assert ops == [{"name": "odd", "syntax": ""}]


def test_a_non_inquiry_answer_yields_nothing():
    assert parse_operators(parse("(int 3)")) == []
