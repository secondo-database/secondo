"""Parse a SECONDO result nested list once and derive all render payloads.

Keeps parsing in one place so ``/api/query`` can attach the static GeoJSON
(Milestone 2), the temporal trips (Milestone 3) and the table rows (Milestone 9)
without walking the text three times.
"""
from __future__ import annotations

from . import geojson, table, temporal
from .nlparser import parse


def convert(
    nested_text: str, *, page: int | None = None
) -> tuple[dict | None, dict | None, dict | None]:
    """Return ``(geojson, temporal, table)`` for a result nested list (text).

    ``page`` cuts the table payload down to the first ``page`` rows of a stored
    relation. It is passed in rather than applied to a finished payload because
    building the rows is the expensive half -- a row of ``Trains`` is an mpoint
    written back out as text -- so the rows past the page are never built.
    """
    tree = parse(nested_text)
    tabular = (
        table.first_page(tree, limit=page) if page is not None
        else table.from_tree(tree)
    )
    return geojson.from_tree(tree), temporal.from_tree(tree), tabular
