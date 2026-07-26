"""Parse SECONDO's ``list objects`` inquiry into a browsable catalog.

Result shape:

    (inquiry (objects (OBJECTS
        (OBJECT <name> (<args>) (<typeexpr>))
        ...)))

For each object we surface its name, a short type label (the outermost type
constructor, e.g. ``rel`` / ``mpoint`` / ``region``), a ``kind`` hint
(``spatial`` / ``temporal`` / ``other``) so the UI can flag what will draw, and
whether it is a flat relation -- the only thing the table view can open and edit.
"""
from __future__ import annotations

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
