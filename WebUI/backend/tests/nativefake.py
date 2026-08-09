"""The one piece of `secondo_native` the fakes cannot get away with sketching.

How an answer crosses into Python decides everything downstream, and there are
three ways it can (`Connection::answer`):

  * pushed into the sink as the client reads it, which is what a `query` does;
  * handed back split into a type half and a tuple iterator, for an answer the
    reader could not stream;
  * handed back whole.

A fake that always chose the last would run the tests down a path production
no longer takes, so the rules are mirrored here once and used by every fake
connection.
"""
from __future__ import annotations

from typing import Any


def _pair(tree: Any) -> bool:
    """Whether this is a `(type value)` answer with a list on the right -- the
    shape, and the only thing either rule looks at."""
    return isinstance(tree, list) and len(tree) == 2 and isinstance(tree[1], list)


def answer(tree: Any, want_tree: bool, sink: Any, *, can_stream: bool = True) -> dict:
    """The `streamed` / `type` / `tuples` / `tree` entries for one answer.

    With a sink and a shape the reader can take, the sink is pushed into here
    and `streamed` is true -- there is then nothing left to hand back.
    `can_stream=False` is the fake's way of being a connection whose answer the
    reader could not stream (a textual transfer, an SQL `(plan result costs)`),
    which must still come back through the split.
    """
    if sink is not None and can_stream and _pair(tree):
        sink.begin(tree[0])
        for element in tree[1]:
            sink.elem(element)
        return {"streamed": True, "type": None, "tuples": None, "tree": None}

    split = sink is not None and _pair(tree)
    return {
        "streamed": False,
        "type": tree[0] if split else None,
        "tuples": iter(tree[1]) if split else None,
        "tree": tree if want_tree or (sink is not None and not split) else None,
    }
