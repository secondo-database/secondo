"""Parse a SECONDO result nested list once and derive all render payloads.

Keeps parsing in one place so ``/api/query`` can attach the static GeoJSON
(Milestone 2), the temporal trips (Milestone 3) and the table rows (Milestone 9)
without walking the text three times.
"""
from __future__ import annotations

from . import geojson, table, temporal
from .nlparser import parse


def convert(nested_text: str) -> tuple[dict | None, dict | None, dict | None]:
    """Return ``(geojson, temporal, table)`` for a result nested list (text)."""
    tree = parse(nested_text)
    return geojson.from_tree(tree), temporal.from_tree(tree), table.from_tree(tree)
