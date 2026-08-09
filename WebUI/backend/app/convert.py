"""Derive every render payload from one SECONDO result.

Keeps the conversion in one place so ``/api/query`` can attach the static
GeoJSON (Milestone 2), the temporal trips (Milestone 3) and the table rows
(Milestone 9) from a single answer.

``Answer`` is what reads one result, in a single pass over its tuples and
whichever way they arrive:

* **Pushed by the bridge**, tuple by tuple, as the client decodes them off the
  socket -- ``begin`` then ``elem``, called from C++ (``secondo_native``). The
  tuple is gone again before the next one is read, on *both* sides: neither the
  client's nested list nor Python ever holds the relation. This is the path a
  ``query`` takes.
* **Handed over whole**, when the reader could not stream that answer -- a
  textual transfer, or the optimizer's ``(plan result costs)``, in which the
  pair worth streaming is nested. ``read`` then feeds the same collectors from
  the bridge's tuple iterator, so at least the Python side is still one tuple
  at a time.

Either way the three payload builders see the answer once, and only the ones
that can produce something are fed -- a relation with nothing temporal in it is
no longer walked by the temporal builder at all.

``convert`` is the whole-tree entry point, for the text paths and the fixture
tests.
"""
from __future__ import annotations

import logging
from collections.abc import Iterable

from . import geojson, table, temporal
from .nlparser import Node

logger = logging.getLogger("secondo.webui")

Payloads = tuple[dict | None, dict | None, dict | None]


def convert(tree: Node, *, page: int | None = None) -> Payloads:
    """Return ``(geojson, temporal, table)`` for a parsed result nested list.

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


def _streamable(type_expr: Node) -> bool:
    """Whether the payload builders read the value half as a list of tuples.

    Exactly the two cases they have: a flat relation, which the grid builds
    rows from, and a ``rel``, which the GeoJSON and temporal builders read one
    tuple at a time.
    """
    if table.columns_of(type_expr) is not None:
        return True
    return isinstance(type_expr, list) and bool(type_expr) and type_expr[0] == "rel"


class Answer:
    """One result, read as it arrives and held until its payloads are wanted.

    The two steps are not one because the page size is not known while the
    answer is being read: deciding it needs a ``list objects`` round trip (see
    ``app/main.py:_is_stored_relation``), which is a command of its own and so
    cannot happen until this one is over.

    Nothing here raises. Deriving a payload is best effort -- a result the
    builders cannot make sense of still leaves the command successful and its
    text on the console -- and it has to stay that way now that the reading
    happens inside the bridge call, where an exception would abort a command
    that had in fact succeeded.
    """

    def __init__(self, *, table_only: bool = False) -> None:
        self.table_only = table_only
        self.rows: table.RelationRows | None = None
        self.features: geojson.RelationFeatures | None = None
        self.moving: temporal.RelationMoving | None = None
        # Where each tuple goes. One list so that the relation case and the
        # "collect it back into a value" case below share `elem`.
        self._feeds: list = []
        self._type: Node = None
        self._items: list | None = None
        self._tree: Node = None
        self._failed = False

    # -- what the bridge pushes into ---------------------------------------

    def begin(self, type_expr: Node) -> None:
        """The answer's type half, before any tuple."""
        try:
            self._type = type_expr
            if _streamable(type_expr):
                self.rows = table.RelationRows(type_expr)
                if not self.table_only:
                    self.features = geojson.RelationFeatures(
                        type_expr, geojson.new_bounds()
                    )
                    self.moving = temporal.RelationMoving(type_expr)
                self._feeds = [
                    s.feed
                    for s in (self.features, self.moving, self.rows)
                    if s is not None and s.wanted
                ]
            else:
                # The reader splits an answer by its *shape*, so this is not
                # only relations: `(point (9396.0 9871.0))` arrives here as two
                # reals. Collect them back into the value half -- the builders
                # want all of it, and nothing that is not a relation is large
                # enough for that to matter.
                self._items = []
                self._feeds = [self._items.append]
        except Exception:  # noqa: BLE001 - a payload must not fail the command
            self._fail()

    def elem(self, tup: Node) -> None:
        """One element of the value half."""
        try:
            for feed in self._feeds:
                feed(tup)
        except Exception:  # noqa: BLE001 - a payload must not fail the command
            self._fail()

    # -- what Python feeds it when the bridge could not stream --------------

    def read(self, type_expr: Node, tuples: Iterable[Node] | None,
             tree: Node) -> None:
        """Read an answer the bridge handed back rather than pushed.

        `tuples` is its tuple iterator when the answer was at least split into
        halves; otherwise `tree` is the whole thing.
        """
        if tuples is None:
            self._tree = tree
            return
        self.begin(type_expr)
        for tup in tuples:
            self.elem(tup)

    # -- the result ---------------------------------------------------------

    def payloads(self, *, page: int | None = None) -> Payloads:
        if self._failed:
            return None, None, None
        try:
            if self.rows is not None:
                return (
                    self.features.collection() if self.features is not None
                    else None,
                    self.moving.payload() if self.moving is not None else None,
                    self.rows.payload(page=page),
                )
            tree = self._tree if self._items is None else [self._type, self._items]
            if self.table_only:
                tabular = (
                    table.first_page(tree, limit=page) if page is not None
                    else table.from_tree(tree)
                )
                return None, None, tabular
            return convert(tree, page=page)
        except Exception:  # noqa: BLE001 - a payload must not fail the command
            logger.exception("Building the render payloads failed")
            return None, None, None

    def _fail(self) -> None:
        if not self._failed:
            logger.exception("Reading the result failed")
        self._failed = True
        self._feeds = []
