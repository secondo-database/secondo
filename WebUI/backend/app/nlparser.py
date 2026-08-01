"""The shape a SECONDO nested list has in Python, and the one way to make one
out of text.

Nothing here parses. A result from the server arrives already built, because the
bridge walks the ``ListExpr`` the C++ client is holding (``treeOf`` in
``native/secondo_native.cpp``); and the one thing that does arrive as text -- a
value the *user* typed into a table cell -- is parsed by SECONDO's own
``NLParser``/``NLLex`` through ``secondo_native.parse_nl``.

This used to be a hand-written parser, and the reason it is gone is not tidiness.
The only question worth asking about a typed-in value is whether *SECONDO* will
accept it, and a second implementation of a grammar agrees with the first only by
accident. It did drift: the old parser accepted ``(9396.0 9871.0))``, which the
kernel rejects, so the value passed the field check and then failed on the server
in the middle of a save -- the exact outcome the check exists to prevent.

Atoms come back as native Python values:

* int      -> int
* real     -> float
* TRUE/FALSE -> bool
* string, symbol and text -> str

Strings and symbols are not told apart: nothing downstream cares, and a type name
is always a symbol.
"""
from __future__ import annotations

from typing import Union

# Importing the config module is what puts ../native on sys.path. Named here
# rather than left to import order: this module is reached through app.table
# before app.config in some orders, and depending on that would break in a way
# that looks unrelated to either.
from . import config as _config  # noqa: F401  (imported for its side effect)

import secondo_native

Node = Union[list, int, float, bool, str]


def parse(text: str) -> Node:
    """Parse nested-list text the way the kernel does.

    Raises ``ValueError`` -- carrying the parser's own complaint, naming the
    token, line and column -- if SECONDO would not accept the text.
    """
    return secondo_native.parse_nl(text)
