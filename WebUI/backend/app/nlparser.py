"""Parser for SECONDO nested lists in their *text* representation.

The heavy lifting (wire protocol, binary list decoding) happens in the trusted
C++ client; what reaches Python is a nested list already rendered as text by
``NestedList::ToString``. This module turns that text into a Python tree so the
GeoJSON converter can walk it. Keeping only text parsing on the Python side is
deliberate -- it is small and easy to fixture-test.

Grammar (informal):

    list   := '(' item* ')'
    item   := list | atom
    atom   := int | real | bool | string | symbol | text

Atoms are returned as native Python values:

* int     -> int
* real    -> float
* TRUE/FALSE -> bool
* "..."   -> str        (quoted string; quotes stripped)
* <text>...</text---> -> str
* symbol  -> str        (e.g. type names like ``point``)

Quoted strings and symbols both become ``str``; the distinction never matters
for the positions the GeoJSON converter inspects (a type name is always a
symbol, never a quoted string).
"""
from __future__ import annotations

from typing import Union

Node = Union[list, int, float, bool, str]

_TEXT_OPEN = "<text>"
_TEXT_CLOSE = "</text--->"


def _classify(token: str) -> Node:
    """Turn a bare atom token into a Python value."""
    try:
        return int(token)
    except ValueError:
        pass
    try:
        return float(token)
    except ValueError:
        pass
    if token == "TRUE":
        return True
    if token == "FALSE":
        return False
    return token  # symbol (or undefined marker like ``undef``)


def parse(text: str) -> Node:
    """Parse a nested-list text into a Python tree. Returns the first element."""
    pos = 0
    n = len(text)

    def skip_ws() -> None:
        nonlocal pos
        while pos < n and text[pos].isspace():
            pos += 1

    def parse_item() -> Node:
        nonlocal pos
        skip_ws()
        if pos >= n:
            raise ValueError("unexpected end of nested list")
        c = text[pos]
        if c == "(":
            return parse_list()
        if c == ")":
            raise ValueError(f"unexpected ')' at {pos}")
        if c == '"':
            return parse_string()
        if text.startswith(_TEXT_OPEN, pos):
            return parse_text_atom()
        return parse_atom()

    def parse_list() -> list:
        nonlocal pos
        pos += 1  # consume '('
        items: list[Node] = []
        while True:
            skip_ws()
            if pos >= n:
                raise ValueError("unterminated list")
            if text[pos] == ")":
                pos += 1
                return items
            items.append(parse_item())

    def parse_string() -> str:
        nonlocal pos
        pos += 1  # consume opening quote
        start = pos
        while pos < n and text[pos] != '"':
            pos += 1
        s = text[start:pos]
        pos += 1  # consume closing quote
        return s

    def parse_text_atom() -> str:
        nonlocal pos
        start = pos + len(_TEXT_OPEN)
        end = text.find(_TEXT_CLOSE, start)
        if end == -1:
            end = n
            pos = n
        else:
            pos = end + len(_TEXT_CLOSE)
        return text[start:end]

    def parse_atom() -> Node:
        nonlocal pos
        start = pos
        while pos < n and not text[pos].isspace() and text[pos] not in "()":
            pos += 1
        return _classify(text[start:pos])

    result = parse_item()
    return result
