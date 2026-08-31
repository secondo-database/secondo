"""Unpack a result that is a single value from its ``(type value)`` wrapper.

The fourth render payload, next to ``geojson`` (static spatial), ``temporal``
(moving objects) and ``table`` (rows): the one for results that are neither --
``query 1 + 55`` answers ``(int 56)``, where the wrapper is longer than the
answer. The console shows the value itself and keeps the nested list a fold
away.

Deliberately narrow. Only the atomic types are unpacked -- ``ATOMIC_TYPES``,
the same set ``nlwriter.literal`` writes back as a bare atom -- and only when
the value half really is one atom. Everything else, ``(point (9396.0 9871.0))``
included, keeps its nested list: a type this does not know is a type whose
value has no reading without its label.

This runs on the backend rather than in the browser because the browser only
ever has the answer as *text*, and turning that back into a value means a
second implementation of the kernel's lexer. See ``app/nlparser.py`` for what
that cost the last time.
"""
from __future__ import annotations

from typing import Any

from . import table
from .nlparser import Node
from .nlwriter import ATOMIC_TYPES


def from_tree(tree: Node) -> dict[str, Any] | None:
    """``["int", 56]`` -> ``{"type": "int", "value": 56}``; None for the rest.

    ``value`` is ``None`` for an undefined one -- ``(real undefined)`` -- which
    the console says out loud rather than showing as an empty box.
    """
    if not (isinstance(tree, list) and len(tree) == 2):
        return None
    type_name, value = tree
    if not isinstance(type_name, str) or type_name not in ATOMIC_TYPES:
        return None
    # A list here is a value of several parts, whatever the label says.
    if isinstance(value, list):
        return None
    return {"type": type_name, "value": table.cell(type_name, value)}
