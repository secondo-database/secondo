"""Derive every render payload from one parsed SECONDO result.

Keeps the conversion in one place so ``/api/query`` can attach the static
GeoJSON (Milestone 2), the temporal trips (Milestone 3) and the table rows
(Milestone 9) from a single tree.
"""
from __future__ import annotations

from . import geojson, table, temporal
from .nlparser import Node


def convert(
    tree: Node, *, page: int | None = None
) -> tuple[dict | None, dict | None, dict | None]:
    """Return ``(geojson, temporal, table)`` for a parsed result nested list.

    The tree comes from the bridge, which builds it from the ``ListExpr`` it is
    already holding (``secondo_native``). Parsing the text form here instead
    meant walking the same list twice: once to print it and once to read it
    back.

    ``page`` cuts the table payload down to the first ``page`` rows of a stored
    relation. It is passed in rather than applied to a finished payload because
    building the rows is the expensive half -- a row of ``Trains`` is an mpoint
    written back out as text -- so the rows past the page are never built.
    """
    tabular = (
        table.first_page(tree, limit=page) if page is not None
        else table.from_tree(tree)
    )
    return geojson.from_tree(tree), temporal.from_tree(tree), tabular
