"""Relation nested list -> table payload (Milestone 9).

Fixture shapes are the ones captured in tests/fixtures/nested_lists.md.
"""
from __future__ import annotations

from app import table
from app.nlparser import parse


def convert(text: str):
    return table.to_table(text)


def test_columns_and_typed_cells():
    t = convert(
        '((rel (tuple ((No int) (Name string) (Ratio real) (Ok bool))))'
        ' ((1 "Aachen" 0.5 TRUE) (2 "Berlin" 1.25 FALSE)))'
    )
    assert [c["name"] for c in t["columns"]] == ["No", "Name", "Ratio", "Ok"]
    assert [c["type"] for c in t["columns"]] == ["int", "string", "real", "bool"]
    assert all(c["atomic"] for c in t["columns"])
    # Atomic values arrive as native JSON so the grid can align and format them.
    assert t["rows"] == [[1, "Aachen", 0.5, True], [2, "Berlin", 1.25, False]]
    assert t["rowCount"] == 2
    assert t["truncated"] is False
    assert t["tidIndex"] is None


def test_non_atomic_attributes_become_nested_list_text():
    """A point/region/mpoint cell shows its nested list, as the Java GUI's
    AttributeFormatter falls back to toString() for non-atomic types."""
    t = convert('((rel (tuple ((Name string) (GeoData point)))) (("Kino" (1.0 2.0))))')
    assert t["columns"][1] == {"name": "GeoData", "type": "point", "atomic": False}
    assert t["rows"][0][1] == "(1.0 2.0)"


def test_undefined_becomes_null():
    t = convert('((rel (tuple ((No int) (Val real)))) ((undefined undefined)))')
    assert t["rows"] == [[None, None]]


def test_a_numeric_looking_string_stays_a_string():
    """The parser classifies "1234" as an int; the column type decides."""
    t = convert('((rel (tuple ((PLZ string)))) (("1234")))')
    assert t["rows"] == [["1234"]]


def test_a_short_tuple_is_padded_not_dropped():
    t = convert('((rel (tuple ((A int) (B int)))) ((1)))')
    assert t["rows"] == [[1, None]]


def test_row_cap_is_reported(monkeypatch):
    monkeypatch.setattr(table, "MAX_ROWS", 2)
    t = convert("((rel (tuple ((No int)))) ((1) (2) (3) (4)))")
    assert t["rowCount"] == 2
    assert t["totalRows"] == 4
    assert t["truncated"] is True


def test_long_cells_are_clipped(monkeypatch):
    monkeypatch.setattr(table, "MAX_CELL_CHARS", 8)
    t = convert('((rel (tuple ((T text)))) ((<text>aaaaaaaaaaaaaaaa</text--->)))')
    assert t["rows"][0][0].endswith("…")
    assert len(t["rows"][0][0]) < 16


def test_tid_column_is_found():
    """What `addid` appends -- the handle every update is addressed by."""
    t = convert("((rel (tuple ((No int) (TID tid)))) ((1 5)))")
    assert t["tidIndex"] == 1
    assert t["columns"][1]["atomic"] is False


def test_trel_and_mrel_are_relations_too():
    assert convert("((trel (tuple ((No int)))) ((1)))")["rowCount"] == 1
    assert convert("((mrel (mtuple ((No int)))) ((1)))")["rowCount"] == 1


def test_nested_relations_are_not_offered_as_tables():
    """nrel/arel carry relations inside their tuples; they have no grid form and
    fall back to the textual view, as they do in the Java GUI."""
    assert convert("((nrel (tuple ((No int)))) ((1)))") is None


def test_non_relations_are_not_tables():
    assert convert("(point (9396.0 9871.0))") is None
    assert convert("(int 13)") is None
    assert convert("(inquiry (databases (BERLINTEST)))") is None
    assert convert("()") is None


def test_arel_attribute_keeps_its_constructor_as_the_label():
    t = convert('((rel (tuple ((Name string) (Sub (arel (tuple ((A int)))))))) ())')
    assert t["columns"][1]["type"] == "arel"
    assert t["columns"][1]["atomic"] is False


# --- naming the base relation --------------------------------------------


def test_base_relation_from_a_plain_query():
    assert table.base_relation("query Staedte") == "Staedte"
    assert table.base_relation("  QUERY  ten  ") == "ten"


def test_base_relation_from_an_optimizer_plan():
    assert table.base_relation("select * from ten", "ten feed consume") == "ten"


def test_anything_derived_is_not_a_base_relation():
    """A filtered or joined result has no single relation to write back to."""
    assert table.base_relation("query Staedte feed head[5] consume") is None
    assert table.base_relation("select * from ten where no > 5",
                               "ten feed filter[.No > 5] consume") is None
    assert table.base_relation("list objects") is None


# --- the parser's string/text escaping (used far more by the table view) ---


def test_a_quote_inside_a_string_does_not_end_the_atom():
    # Tools/NestedLists/NLLex.l:222 accepts \" inside a string atom.
    assert parse(r'("say \"hi\"" 1)') == ['say "hi"', 1]


def test_an_escaped_close_tag_does_not_end_a_text_atom():
    # NLLex.l:170 -- \</text---> is the literal close tag.
    assert parse("(<text>a\\</text--->b</text--->)") == ["a</text--->b"]


def test_a_text_attribute_comes_back_in_the_short_quoted_form():
    """`NestedList::ToString` writes a text atom as `'...'`, not as
    `<text>...</text--->`. Parsed as a bare token it stopped at the first space,
    so a text cell showed as `'a` -- truncated and with a stray quote."""
    t = convert("((rel (tuple ((Note text)))) (('a </text---> b') ('hello'))))")
    assert t["rows"] == [["a </text---> b"], ["hello"]]


def test_escapes_inside_a_short_text_atom():
    # NLLex.l:203-208 -- \' is a literal quote and \\ a backslash.
    assert parse(r"('it\'s' 1)") == ["it's", 1]
    assert parse(r"('a\\b')") == ["a\\b"]
