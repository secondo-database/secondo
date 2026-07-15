"""Parse a SECONDO result nested list once and derive all render payloads.

Keeps parsing in one place so ``/api/query`` can attach both the static
GeoJSON (Milestone 2) and the temporal trips (Milestone 3) without walking the
text twice.
"""
from __future__ import annotations

from . import geojson, temporal
from .nlparser import parse


def convert(nested_text: str) -> tuple[dict | None, dict | None]:
    """Return ``(geojson, temporal)`` for a result nested list (text)."""
    tree = parse(nested_text)
    return geojson.from_tree(tree), temporal.from_tree(tree)
