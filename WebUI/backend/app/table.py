"""Convert a SECONDO relation result into a column/row payload for the table view.

This is the third render payload next to ``geojson`` (static spatial) and
``temporal`` (moving objects): the one for results that are *rows*. It mirrors
``Javagui/viewer/update2/Head.java:75``, which parses

    ((rel (tuple ((Name Type) (Name Type) ...))) ( (value value ...) ... ))

into columns, and ``AttributeFormatter.fromListExprToString`` (:45), which turns
each attribute value into something displayable.

Only the *flat* relation types are handled. ``nrel`` / ``arel`` carry relations
inside their tuples and have no meaningful grid form; the Java GUI excludes them
from the editable relations too (``UpdateViewerController.retrieveRelationNames``),
and here they simply fall back to the textual view.
"""
from __future__ import annotations

import math
import re
from typing import Any

from .nlparser import Node, parse
from .nlwriter import ATOMIC_TYPES, to_text

# Head.java:64 -- minus the nested ones (nrel/arel), which have no grid form.
RELATION_TYPES = frozenset({"rel", "mrel", "trel", "orel"})
TUPLE_TYPES = frozenset({"tuple", "mtuple"})

# The attribute `addid` appends. Never editable, and skipped when inserting.
TID_TYPE = "tid"

# A relation of Trains is 562 tuples of multi-megabyte mpoint text. Neither the
# wire nor the browser wants that, so both dimensions are capped and the payload
# says so rather than silently showing a prefix.
#
# For a *stored* relation the cap is not the end of the story: /api/table/load
# cuts pages out of the relation server-side (app.updates.load_command), and
# MAX_ROWS is then only the ceiling on how large a page may be. The cap is a
# real truncation just for results the backend did not build the query for --
# an ad-hoc join arrives whole or not at all.
MAX_ROWS = 1000
MAX_CELL_CHARS = 4000

# What /api/table/load reads when the client does not ask for a size. Small
# enough that a page of mpoints is still a sensible HTTP response.
DEFAULT_PAGE_ROWS = 200

_UNDEF = frozenset({"undef", "undefined"})

# `query Staedte` and nothing else: the one command shape whose result is exactly
# one stored relation, so the table can offer to edit it.
_QUERY_ONE_OBJECT = re.compile(r"\A\s*query\s+([A-Za-z]\w*)\s*\Z", re.IGNORECASE)
# What the optimizer plans `select * from staedte` to.
_PLAN_ONE_OBJECT = re.compile(r"\A\s*([A-Za-z]\w*)\s+feed\s+consume\s*\Z", re.IGNORECASE)


def _type_name(type_expr: Node) -> str:
    """An attribute's type as a label: the symbol itself, or the constructor of
    a compound type (``(arel (tuple ...))`` -> ``arel``)."""
    if isinstance(type_expr, str):
        return type_expr
    if isinstance(type_expr, list) and type_expr:
        return _type_name(type_expr[0])
    return "?"


def columns_of(type_expr: Node) -> list[dict] | None:
    """``(rel (tuple ((Name Type)...)))`` -> column descriptors, or None."""
    if not (isinstance(type_expr, list) and len(type_expr) == 2):
        return None
    if _type_name(type_expr[0]) not in RELATION_TYPES:
        return None
    tuple_expr = type_expr[1]
    if not (isinstance(tuple_expr, list) and len(tuple_expr) == 2):
        return None
    if _type_name(tuple_expr[0]) not in TUPLE_TYPES:
        return None
    attrs = tuple_expr[1]
    if not isinstance(attrs, list) or not attrs:
        return None

    columns: list[dict] = []
    for attr in attrs:
        if not (isinstance(attr, list) and len(attr) >= 2):
            return None
        name = attr[0]
        if not isinstance(name, str):
            return None
        type_name = _type_name(attr[1])
        columns.append(
            {
                "name": name,
                "type": type_name,
                # Whether a plain input can edit it. Everything else needs raw
                # nested-list syntax, as it does in the Java GUI.
                "atomic": type_name in ATOMIC_TYPES,
            }
        )
    return columns


def _cell(type_name: str, value: Node) -> Any:
    """One attribute value, encoded for JSON.

    Atomic types become native JSON values so the grid can align and format
    them; anything else becomes its nested-list text, which is what the Java
    GUI shows (and edits) for a point, a region or an mpoint.
    """
    if isinstance(value, str) and value in _UNDEF and type_name not in ("string", "text"):
        return None

    if type_name in ("int", TID_TYPE):
        # A TID is a number even though it is not an editable attribute: it is
        # what `(tid <n>)` in every update command is built from.
        return value if isinstance(value, int) and not isinstance(value, bool) else None
    if type_name == "real":
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            return None
        f = float(value)
        # JSON has no NaN/Infinity; an undefined real must not break the response.
        return f if math.isfinite(f) else None
    if type_name == "bool":
        return value if isinstance(value, bool) else None
    if type_name in ("string", "text"):
        if isinstance(value, list):
            return None
        # A numeric-looking string parses as int/float; the column type decides.
        return _clip(value if isinstance(value, str) else to_text(value))

    return _clip(to_text(value))


def _clip(text: str) -> str:
    return text if len(text) <= MAX_CELL_CHARS else text[:MAX_CELL_CHARS] + " …"


def from_tree(
    tree: Node,
    *,
    offset: int = 0,
    limit: int | None = None,
    total: int | None = None,
) -> dict | None:
    """A parsed ``(type value)`` result -> a table payload, or None if it is not
    a flat relation.

    ``limit`` says the result is already one page of a larger relation, cut out
    server-side; ``offset`` is where that page starts and ``total`` how many rows
    there are altogether (``None`` when the caller chose not to count them). A
    page is never *truncated*: every row is reachable, just not at once.
    """
    if not (isinstance(tree, list) and len(tree) >= 2):
        return None
    columns = columns_of(tree[0])
    if columns is None:
        return None
    tuples = tree[1]
    if not isinstance(tuples, list):
        return None

    paged = limit is not None
    types = [c["type"] for c in columns]
    rows: list[list[Any]] = []
    for tup in tuples if paged else tuples[:MAX_ROWS]:
        if not isinstance(tup, list):
            continue
        rows.append(
            [_cell(types[i], tup[i]) if i < len(tup) else None for i in range(len(columns))]
        )

    tid_index = next(
        (i for i, c in enumerate(columns) if c["type"] == TID_TYPE), None
    )
    return {
        "columns": columns,
        "rows": rows,
        "rowCount": len(rows),
        # Rows were dropped with no way to ask for them. A page never is: the
        # client just asks for the next one.
        "truncated": (not paged) and len(tuples) > MAX_ROWS,
        "totalRows": (total if total is not None else offset + len(rows)) if paged
        else len(tuples),
        # Whether `totalRows` is the real total or just what has been seen so far.
        "totalKnown": (total is not None) if paged else True,
        "offset": offset if paged else 0,
        "limit": limit,
        "pageable": paged,
        "tidIndex": tid_index,
        # Set by the editable load path (see app.updates); a table that just
        # came out of /api/query is read-only until it is loaded with TIDs.
        "relation": None,
    }


def to_table(nested_text: str) -> dict | None:
    return from_tree(parse(nested_text))


def base_relation(command: str, plan: str | None = None) -> str | None:
    """The stored relation this result came from, when it can be named without
    guessing -- ``query Staedte``, or SQL the optimizer planned as
    ``staedte feed consume``. Only a hint for offering the Edit button;
    ``/api/table/load`` validates the name against the catalog regardless.
    """
    m = _QUERY_ONE_OBJECT.match(command or "")
    if m:
        return m.group(1)
    m = _PLAN_ONE_OBJECT.match(plan or "")
    if m:
        return m.group(1)
    return None
