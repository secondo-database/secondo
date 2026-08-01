"""The commands that change a stored relation (Milestone 9).

Same operators as Javagui/viewer/update2/CommandGenerator.java, written in SOS
text syntax -- the forms documented in Algebras/UpdateRel/UpdateRelation.examples
and Algebras/BTree/BTree.examples. These tests pin the command shapes so a change
here is a deliberate one.
"""
from __future__ import annotations

import pytest

from app import updates
from app.nlparser import parse
from app.nlwriter import InvalidValue, literal

STAEDTE = [
    {"name": "SName", "type": "string", "atomic": True},
    {"name": "Bev", "type": "int", "atomic": True},
    {"name": "PLZ", "type": "int", "atomic": True},
]

# `list objects` with a btree and an rtree over Staedte, plus one index that
# belongs to a different relation and one over an attribute Staedte lacks.
OBJECTS = (
    "(inquiry (objects (OBJECTS "
    "(OBJECT Staedte () ((rel (tuple ((SName string) (Bev int) (PLZ int)))))) "
    "(OBJECT Staedte_Bev () ((btree (tuple ((SName string))) int))) "
    "(OBJECT Staedte_GeoData () ((rtree (tuple ((SName string))) point))) "
    "(OBJECT Staedte_Nope () ((btree (tuple ((SName string))) int))) "
    "(OBJECT Orte_Bev () ((btree (tuple ((Ort string))) int))) "
    ")))"
)


# --- the load that makes a table editable ---------------------------------


def test_load_appends_addid():
    """Without `addid` there is no TID, and without a TID nothing is editable."""
    assert updates.load_command("Staedte") == "query Staedte feed addid consume"


def test_load_with_filter_project_and_sort():
    cmd = updates.load_command(
        "Staedte", [".Bev > 500000"], ["SName", "Bev"], ["SName"]
    )
    assert cmd == (
        "query Staedte feed filter [ .Bev > 500000 ] "
        "project [ SName, Bev ] sortby [ SName ] addid consume"
    )


# --- paging ----------------------------------------------------------------


def test_the_first_page_needs_no_counter():
    """`head` alone is the whole restriction at offset 0, and it ends the scan
    once the page is full."""
    assert updates.load_command("Staedte", limit=200) == (
        "query Staedte feed addid head[200] consume"
    )


def test_a_later_page_counts_and_skips():
    """SECONDO has no `skip`, so an offset is `addcounter` plus a `filter`; the
    counter is removed again so the page has the relation's own attributes."""
    assert updates.load_command("Staedte", offset=400, limit=200) == (
        "query Staedte feed addid addcounter[RowNo, 1] filter [ .RowNo > 400 ] "
        "head[200] remove[RowNo] consume"
    )


def test_paging_comes_after_addid():
    """The TIDs must be the stored ones, whichever page is read -- so the page is
    cut out after `addid`, never before it."""
    cmd = updates.load_command(
        "Staedte", [".Bev > 500000"], ["SName", "Bev"], ["SName"], offset=10, limit=5
    )
    assert cmd.index("addid") < cmd.index("addcounter")
    assert cmd.startswith(
        "query Staedte feed filter [ .Bev > 500000 ] "
        "project [ SName, Bev ] sortby [ SName ] addid "
    )


def test_counter_name_avoids_the_relations_own_attributes():
    assert updates.counter_name(STAEDTE) == "RowNo"
    assert updates.counter_name([{"name": "RowNo"}]) == "RowNo2"
    # Case is not a defence: SECONDO would still see a duplicate.
    assert updates.counter_name([{"name": "rowno"}, {"name": "RowNo2"}]) == "RowNo3"


def test_count_command_keeps_the_filter_and_drops_the_rest():
    """Neither `project` nor `sortby` can change a count, so the scan the pager
    pays for stays as cheap as it can be."""
    assert updates.count_command("Staedte", [".Bev > 500000"]) == (
        "query Staedte feed filter [ .Bev > 500000 ] count"
    )
    assert updates.count_command("Staedte") == "query Staedte feed count"


# --- DML ------------------------------------------------------------------


def test_insert_command():
    cmd = updates.insert_command(
        "Staedte", STAEDTE, {"SName": "Bremen", "Bev": "526000", "PLZ": "2800"}
    )
    # UpdateRelation.examples:110 is exactly this shape. `consume`, not `count`:
    # the answer carries the tuple with the TID the server assigned.
    assert cmd == 'query Staedte inserttuple["Bremen", 526000, 2800] consume'


def test_insert_requires_every_attribute():
    with pytest.raises(InvalidValue) as exc:
        updates.insert_command("Staedte", STAEDTE, {"SName": "Bremen"})
    assert "Bev" in str(exc.value)


def test_insert_skips_the_tid_column():
    """`addid` puts a TID in the loaded table; the stored relation has none."""
    columns = STAEDTE + [{"name": "TID", "type": "tid", "atomic": False}]
    cmd = updates.insert_command(
        "Staedte", columns, {"SName": "Bremen", "Bev": "1", "PLZ": "2"}
    )
    assert cmd == 'query Staedte inserttuple["Bremen", 1, 2] consume'


def test_delete_command():
    # UpdateRelation.examples:36
    assert updates.delete_command("Staedte", 5) == (
        "query Staedte deletebyid[[const tid value 5]] count"
    )


def test_update_command_only_names_changed_attributes():
    # UpdateRelation.examples:124
    cmd = updates.update_command("Staedte", 10, STAEDTE, {"Bev": "526000"})
    assert cmd == (
        "query Staedte updatebyid[[const tid value 10]; Bev: 526000] count"
    )


def test_update_names_several_attributes_in_column_order():
    cmd = updates.update_command(
        "Staedte", 10, STAEDTE, {"PLZ": "44225", "SName": "Bremen"}
    )
    assert cmd == (
        'query Staedte updatebyid[[const tid value 10]; '
        'SName: "Bremen", PLZ: 44225] count'
    )


def test_update_with_no_change_is_refused():
    with pytest.raises(InvalidValue):
        updates.update_command("Staedte", 10, STAEDTE, {})


# --- index maintenance ----------------------------------------------------


def test_find_indexes_by_naming_convention():
    idx = updates.find_indexes(parse(OBJECTS), "Staedte", ["SName", "Bev", "PLZ"])
    assert [(i.name, i.attribute, i.kind) for i in idx.entries] == [
        ("Staedte_Bev", "Bev", "btree")
    ]


def test_find_indexes_ignores_other_relations_and_unknown_attributes():
    idx = updates.find_indexes(parse(OBJECTS), "Staedte", ["SName", "Bev", "PLZ", "GeoData"])
    names = [i.name for i in idx.entries]
    assert "Staedte_GeoData" in names
    # Orte_Bev belongs to Orte; Staedte_Nope indexes an attribute that is gone.
    assert "Orte_Bev" not in names
    assert "Staedte_Nope" not in names


def test_find_indexes_tolerates_a_lowercased_initial():
    """SECONDO lowercases an object's first letter in some paths, which is why
    CommandGenerator.java:419 compares the first character case-insensitively."""
    objs = OBJECTS.replace("(OBJECT Staedte_Bev", "(OBJECT staedte_Bev")
    idx = updates.find_indexes(parse(objs), "Staedte", ["Bev"])
    assert [i.name for i in idx.entries] == ["staedte_Bev"]


def test_index_maintenance_is_chained_onto_every_command():
    """SECONDO does not keep secondary indexes in step by itself; leaving them
    stale is a silent wrong answer to any later query that uses one. The chained
    form is Algebras/BTree/BTree.examples:49."""
    idx = updates.find_indexes(parse(OBJECTS), "Staedte", ["SName", "Bev", "PLZ", "GeoData"])

    assert updates.delete_command("Staedte", 5, idx) == (
        "query Staedte deletebyid[[const tid value 5]]"
        " Staedte_Bev deletebtree[Bev]"
        " Staedte_GeoData deletertree[GeoData] count"
    )
    assert updates.insert_command(
        "Staedte", STAEDTE, {"SName": "A", "Bev": "1", "PLZ": "2"}, idx
    ) == (
        'query Staedte inserttuple["A", 1, 2]'
        " Staedte_Bev insertbtree[Bev]"
        " Staedte_GeoData insertrtree[GeoData] consume"
    )
    assert updates.update_command("Staedte", 1, STAEDTE, {"Bev": "2"}, idx) == (
        "query Staedte updatebyid[[const tid value 1]; Bev: 2]"
        " Staedte_Bev updatebtree[Bev]"
        " Staedte_GeoData updatertree[GeoData] count"
    )


# --- value conversion (nlwriter) ------------------------------------------


def test_literals_per_type():
    assert literal("A", "int", " 42 ") == "42"
    assert literal("A", "real", "1") == "1.0"  # must not lex as an int
    assert literal("A", "real", "1.5") == "1.5"
    assert literal("A", "bool", "true") == "TRUE"
    assert literal("A", "string", "Bremen") == '"Bremen"'
    assert literal("A", "text", "hello") == "[const text value <text>hello</text--->]"
    # AttributeFormatter treats an empty numeric cell as zero.
    assert literal("A", "int", "") == "0"
    assert literal("A", "real", "") == "0.0"


def test_string_and_text_escaping():
    # Tools/NestedLists/NLLex.l:222 / :170 -- these must not end the literal.
    # Verified against a live server: the SOS parser accepts both forms, and a
    # string holding a quote, a comma or a `]` round-trips unchanged.
    assert literal("A", "string", 'say "hi"') == '"say \\"hi\\""'
    assert literal("A", "text", "a</text--->b") == (
        "[const text value <text>a\\</text--->b</text--->]"
    )
    assert literal("A", "string", "A, B]") == '"A, B]"'


def test_string_longer_than_the_kernel_allows_is_refused():
    """SECONDO truncates past MAX_STRINGSIZE with only a warning, so a value that
    would be silently shortened is rejected instead."""
    with pytest.raises(InvalidValue) as exc:
        literal("SName", "string", "x" * 49)
    assert "48" in str(exc.value)


def test_non_atomic_types_become_a_const_of_their_nested_list():
    """The cell holds the value's own nested list -- which is how the server
    rendered it -- so it round-trips into `[const <type> value <value>]`."""
    assert literal("GeoData", "point", "(9396.0 9871.0)") == (
        "[const point value (9396.0 9871.0)]"
    )


def test_a_non_atomic_value_that_is_not_one_item_is_refused():
    """`9396.0 9871.0` without its parentheses is two values, not a point. The
    parser would take the first and silently drop the rest."""
    with pytest.raises(InvalidValue) as exc:
        literal("GeoData", "point", "9396.0 9871.0")
    assert "GeoData" in str(exc.value)


@pytest.mark.parametrize(
    "type_name,value",
    [("int", "twelve"), ("real", "x"), ("bool", "maybe"), ("point", "(1.0 2.0")],
)
def test_bad_values_fail_here_naming_the_attribute(type_name, value):
    with pytest.raises(InvalidValue) as exc:
        literal("Bev", type_name, value)
    assert "Bev" in str(exc.value)
