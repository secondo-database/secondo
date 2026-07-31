"""Parse SECONDO's ``list objects`` and ``list operators`` inquiries.

``list objects``:

    (inquiry (objects (OBJECTS
        (OBJECT <name> (<args>) (<typeexpr>))
        ...)))

For each object we surface its name, a short type label (the outermost type
constructor, e.g. ``rel`` / ``mpoint`` / ``region``), a ``kind`` hint
(``spatial`` / ``temporal`` / ``other``) so the UI can flag what will draw, and
whether it is a flat relation -- the only thing the table view can open and edit.

``list operators`` is the vocabulary half of the same idea: what the *server*
can do, as opposed to what the open database holds. See ``parse_operators``.
"""
from __future__ import annotations

import re

from .nlparser import Node, parse
from .table import RELATION_TYPES

_SPATIAL = {"point", "points", "line", "sline", "dline", "region", "rect"}
_TEMPORAL = {
    "mpoint", "mregion", "mreal", "mint", "mbool", "mstring",
    "mlabel", "mlabels", "mplace", "mplaces", "upoint", "intime",
}


def _symbols(node: Node) -> list[str]:
    out: list[str] = []
    if isinstance(node, str):
        out.append(node)
    elif isinstance(node, list):
        for n in node:
            out.extend(_symbols(n))
    return out


def _first_symbol(node: Node) -> str | None:
    for s in _symbols(node):
        return s
    return None


def _entries(text: str) -> list[Node]:
    """The ``(OBJECT name (args) (typeexpr))`` entries of a ``list objects``."""
    tree = parse(text)
    try:
        # (inquiry (objects (OBJECTS obj*)))
        objects = tree[1][1]  # type: ignore[index]
    except (IndexError, TypeError):
        return []
    if not (isinstance(objects, list) and objects and objects[0] == "OBJECTS"):
        return []
    return objects[1:]


def object_type_expr(text: str, name: str) -> Node | None:
    """One object's *full* type expression, so a relation's schema can be read
    without a second command. ``parse_objects`` only keeps a label.

    The inquiry wraps the type expression in one extra list -- an object of type
    ``region`` is listed as ``((region))`` -- which is peeled off here so the
    result is the type expression itself.
    """
    for obj in _entries(text):
        if isinstance(obj, list) and len(obj) >= 4 and obj[0] == "OBJECT" and obj[1] == name:
            wrapper = obj[3]
            if isinstance(wrapper, list) and len(wrapper) == 1:
                return wrapper[0]
            return wrapper
    return None


def parse_objects(text: str) -> list[dict]:
    entries = _entries(text)

    result: list[dict] = []
    for obj in entries:
        if not (isinstance(obj, list) and len(obj) >= 4 and obj[0] == "OBJECT"):
            continue
        name = obj[1]
        type_expr = obj[3]
        syms = set(_symbols(type_expr))
        if syms & _TEMPORAL:
            kind = "temporal"
        elif syms & _SPATIAL:
            kind = "spatial"
        else:
            kind = "other"
        type_label = _first_symbol(type_expr) or "?"
        result.append(
            {
                "name": name,
                "type": type_label,
                "kind": kind,
                "relation": type_label in RELATION_TYPES,
            }
        )
    result.sort(key=lambda o: str(o["name"]).lower())
    return result


# What a completion menu can insert. SECONDO also registers operators spelled
# `+`, `<=` or `#`, but the editor's token regex (console/completion.ts) can
# never produce a word for those, so listing them only pads the response.
_OPERATOR_NAME = re.compile(r"\A[A-Za-z_]\w*\Z")


def parse_operators(text: str) -> list[dict]:
    """Every operator the connected server has, from ``list operators``.

    Result shape (``SecondoCatalog::ListOperators``, SecondoCatalog.cpp:3048):

        (inquiry (operators (
            (<name> (<label> ...) (<value> ...))
            ...)))

    The two sublists are ``Operator::GetSpecList()`` -- the labels an algebra
    chose (usually ``Signature Syntax Meaning Example``) and their values, as
    string or text atoms. Only ``Syntax`` is kept: it is what a completion menu
    can show in one line, and carrying every ``Meaning`` would multiply the size
    of a response that already lists on the order of 1500 operators.

    An overloaded operator is registered once per algebra, so names repeat; the
    first spelling of each wins.
    """
    tree = parse(text)
    try:
        # (inquiry (operators (op*)))
        entries = tree[1][1]  # type: ignore[index]
    except (IndexError, TypeError):
        return []
    if not isinstance(entries, list):
        return []

    seen: dict[str, dict] = {}
    for entry in entries:
        if not (isinstance(entry, list) and len(entry) >= 3):
            continue
        name = entry[0]
        if not isinstance(name, str) or not _OPERATOR_NAME.match(name):
            continue
        if name in seen:
            continue
        seen[name] = {"name": name, "syntax": _spec_value(entry[1], entry[2], "Syntax")}
    return sorted(seen.values(), key=lambda o: str(o["name"]).lower())


def _spec_value(labels: Node, values: Node, wanted: str) -> str:
    """One labelled field of an operator specification.

    The label set is up to the algebra, so labels and values are zipped rather
    than indexed by position -- an operator that spells its specification
    differently loses the field, not the entry.
    """
    if not (isinstance(labels, list) and isinstance(values, list)):
        return ""
    for label, value in zip(labels, values):
        if label == wanted and isinstance(value, str):
            return " ".join(value.split())
    return ""
