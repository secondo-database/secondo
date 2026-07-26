"""Render values back into SECONDO text.

The inverse direction of ``nlparser``. Two jobs, deliberately separate:

* ``to_text`` renders an already-parsed tree back to nested-list text. It is used
  to show a non-atomic attribute (a point, a region, an mpoint) in a table cell,
  the way ``AttributeFormatter.fromListExprToString`` falls back to
  ``toString()`` in the Java GUI. It is a *display* function -- the parser does
  not record whether an atom was written as a symbol or as a quoted string, so a
  round trip is not guaranteed to be byte-identical.

* ``literal`` turns what a user typed in a cell into a SECONDO **SOS text**
  literal for one attribute value, given the attribute's type. This one *is*
  exact: it is what goes into an ``inserttuple`` / ``updatebyid`` command. It
  mirrors ``Javagui/viewer/update2/AttributeFormatter.java:89``, except that the
  Java GUI builds a ``ListExpr`` because it sends its commands as nested lists;
  here the same values are written as SOS literals (see app/updates.py for why).

The escaping rules come from the kernel's own lexer, ``Tools/NestedLists/NLLex.l``.
The SOS parser accepts the same string and text forms -- verified against a live
server, including a string holding ``"``, ``,`` and ``]``, and a text holding an
escaped ``</text--->``:

* a string atom is ``"..."`` in which ``\\"`` reads as ``"`` and ``\\\\`` as ``\\``
  (NLLex.l:222-234), and its content is truncated past ``MAX_STRINGSIZE`` -- 48,
  from ``include/NestedList.h:339``. Truncation is only a warning on the server,
  so an over-long value is rejected here instead of being silently shortened.
* a text atom is ``<text>...</text--->`` in which ``\\</text--->`` reads as the
  literal close tag and ``\\\\`` as ``\\`` (NLLex.l:170-186).
"""
from __future__ import annotations

import re

from .nlparser import Node, parse

# include/NestedList.h:339 -- MAX_STRINGSIZE = 2 * STRINGSIZE (24).
MAX_STRING = 48

TEXT_OPEN = "<text>"
TEXT_CLOSE = "</text--->"

# What can be written without quotes. Anything else is quoted so it survives the
# lexer as one atom.
_SYMBOL = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")

# Types with a dedicated atom form; everything else is written as
# ``(<type> <value>)`` with the value passed through as the user typed it.
ATOMIC_TYPES = frozenset({"int", "real", "bool", "string", "text"})


class InvalidValue(ValueError):
    """A cell value that cannot be turned into a value of its attribute type."""

    def __init__(self, attribute: str, type_name: str, reason: str) -> None:
        super().__init__(f"{attribute} ({type_name}): {reason}")
        self.attribute = attribute
        self.type_name = type_name
        self.reason = reason


def quote_string(value: str) -> str:
    """A ``string`` atom. Backslashes first, then quotes -- the reverse of the
    order the lexer undoes them in."""
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def text_atom(value: str) -> str:
    """A ``text`` atom, with the close tag escaped so it cannot end the atom."""
    escaped = value.replace("\\", "\\\\").replace(TEXT_CLOSE, "\\" + TEXT_CLOSE)
    return f"{TEXT_OPEN}{escaped}{TEXT_CLOSE}"


def to_text(node: Node) -> str:
    """Render a parsed tree back to nested-list text, for display."""
    if isinstance(node, list):
        return "(" + " ".join(to_text(n) for n in node) + ")"
    if isinstance(node, bool):  # before int -- bool is an int in Python
        return "TRUE" if node else "FALSE"
    if isinstance(node, (int, float)):
        return repr(node)
    if _SYMBOL.match(node):
        return node
    return quote_string(node)


def literal(attribute: str, type_name: str, value: str) -> str:
    """One attribute value as a SECONDO SOS-text literal of type ``type_name``.

    ``value`` is what the user typed. For a non-atomic type that is the value's
    own nested list -- ``(9396.0 9871.0)`` for a point, which is exactly what the
    table cell shows, since that is how the server rendered it -- and it becomes
    ``[const <type> value <value>]``. Raises ``InvalidValue`` naming the
    attribute so the caller can point at the offending cell.
    """
    value = value if value is not None else ""

    if type_name == "bool":
        v = value.strip().upper()
        if v not in ("TRUE", "FALSE"):
            raise InvalidValue(attribute, type_name, "expected TRUE or FALSE")
        return v

    if type_name == "int":
        v = value.strip()
        if not v:
            return "0"  # AttributeFormatter treats an empty cell as 0
        try:
            return str(int(v))
        except ValueError:
            raise InvalidValue(attribute, type_name, f"not an integer: {value!r}") from None

    if type_name == "real":
        v = value.strip()
        if not v:
            return "0.0"
        try:
            f = float(v)
        except ValueError:
            raise InvalidValue(attribute, type_name, f"not a real: {value!r}") from None
        # A real atom must be recognisable as one: `3` lexes as an int.
        return repr(f) if ("." in repr(f) or "e" in repr(f)) else f"{f}.0"

    if type_name == "string":
        if len(value) > MAX_STRING:
            raise InvalidValue(
                attribute,
                type_name,
                f"longer than {MAX_STRING} characters (SECONDO would truncate it)",
            )
        return quote_string(value)

    if type_name == "text":
        return f"[const text value {text_atom(value)}]"

    # Any other type: the user typed the value's own nested list. Parse it here
    # so a malformed cell fails with a readable message naming the attribute,
    # rather than reaching the server as a syntax error. Wrapping in parentheses
    # for the check is what catches a value that is several items rather than
    # one -- `9396.0 9871.0` instead of `(9396.0 9871.0)` -- which `parse` would
    # otherwise accept while silently dropping everything after the first.
    try:
        parsed = parse(f"({value})")
    except ValueError as exc:
        raise InvalidValue(attribute, type_name, str(exc)) from None
    if not isinstance(parsed, list) or len(parsed) != 1:
        raise InvalidValue(
            attribute,
            type_name,
            f"expected one {type_name} value in nested-list syntax, e.g. "
            f"({type_name} …) written as its value list",
        )
    return f"[const {type_name} value {value.strip()}]"
