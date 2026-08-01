"""Parser for SECONDO nested lists in their *text* representation.

This is **not** how a server answer reaches the converters. The bridge walks the
``ListExpr`` the C++ client is already holding and hands over the Python tree
directly (``treeOf`` in ``native/secondo_native.cpp``), which is both the native
way to read a nested list and one traversal instead of three -- printing it,
scanning the print, and rebuilding it.

What is left for this module is the text nobody else can decode:

* a nested-list value **the user typed** into a table cell, which has no
  ``ListExpr`` behind it and has to be rejected with a readable message before
  it reaches the server as a syntax error (``nlwriter.literal``);
* fixtures in the tests, where a text literal is easier to read than a
  hand-built tree, and where it stands in for what the C++ walk produces.

The two must therefore keep agreeing on the shape below.

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
* '...'   -> str        (the short form ``ToString`` actually emits for text)
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
        if c == "'":
            # `NestedList::ToString` writes a text atom in the short form
            # `'...'`, not as `<text>...</text--->` -- so this is what a `text`
            # attribute actually looks like coming back from the server. A
            # symbol can never contain a quote (NLLex.l's `otherChar` excludes
            # it), so a leading one is unambiguous.
            return parse_simple_text()
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
        # The kernel's lexer (Tools/NestedLists/NLLex.l:222) accepts `\"` inside
        # a string atom and unescapes `\"` -> `"` and `\\` -> `\`, so a value
        # containing a quote must not be taken to end the atom.
        nonlocal pos
        pos += 1  # consume opening quote
        out: list[str] = []
        while pos < n and text[pos] != '"':
            if text[pos] == "\\" and pos + 1 < n and text[pos + 1] in '\\"':
                out.append(text[pos + 1])
                pos += 2
                continue
            out.append(text[pos])
            pos += 1
        pos += 1  # consume closing quote
        return "".join(out)

    def parse_text_atom() -> str:
        # Likewise NLLex.l:170-178: `\</text--->` is the literal close tag and
        # `\\` a backslash, so neither ends the atom early.
        nonlocal pos
        pos += len(_TEXT_OPEN)
        out: list[str] = []
        while pos < n:
            if text.startswith(_TEXT_CLOSE, pos):
                pos += len(_TEXT_CLOSE)
                return "".join(out)
            if text[pos] == "\\":
                if text.startswith(_TEXT_CLOSE, pos + 1):
                    out.append(_TEXT_CLOSE)
                    pos += 1 + len(_TEXT_CLOSE)
                    continue
                if pos + 1 < n and text[pos + 1] == "\\":
                    out.append("\\")
                    pos += 2
                    continue
            out.append(text[pos])
            pos += 1
        return "".join(out)  # unterminated: take what there is

    def parse_simple_text() -> str:
        # NLLex.l:190-212 (state TEXTSIMPLE): `\'` is a literal quote, `\\` a
        # backslash, and an unescaped `'` ends the atom.
        nonlocal pos
        pos += 1  # consume the opening quote
        out: list[str] = []
        while pos < n and text[pos] != "'":
            if text[pos] == "\\" and pos + 1 < n and text[pos + 1] in "\\'":
                out.append(text[pos + 1])
                pos += 2
                continue
            out.append(text[pos])
            pos += 1
        pos += 1  # consume the closing quote
        return "".join(out)

    def parse_atom() -> Node:
        nonlocal pos
        start = pos
        while pos < n and not text[pos].isspace() and text[pos] not in "()":
            pos += 1
        return _classify(text[start:pos])

    result = parse_item()
    return result
