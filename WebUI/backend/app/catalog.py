"""Parse SECONDO's ``list objects`` inquiry into a browsable catalog.

Result shape:

    (inquiry (objects (OBJECTS
        (OBJECT <name> (<args>) (<typeexpr>))
        ...)))

For each object we surface its name, a short type label (the outermost type
constructor, e.g. ``rel`` / ``mpoint`` / ``region``) and a ``kind`` hint
(``spatial`` / ``temporal`` / ``other``) so the UI can flag what will draw.
"""
from __future__ import annotations

from .nlparser import Node, parse

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


def parse_objects(text: str) -> list[dict]:
    tree = parse(text)
    # Drill down to the OBJECTS list.
    try:
        # (inquiry (objects (OBJECTS obj*)))
        objects = tree[1][1]  # type: ignore[index]
        if not (isinstance(objects, list) and objects and objects[0] == "OBJECTS"):
            return []
        entries = objects[1:]
    except (IndexError, TypeError):
        return []

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
        result.append(
            {
                "name": name,
                "type": _first_symbol(type_expr) or "?",
                "kind": kind,
            }
        )
    result.sort(key=lambda o: str(o["name"]).lower())
    return result
