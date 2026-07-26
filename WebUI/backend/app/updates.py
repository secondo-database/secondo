"""Build the SECONDO commands that change a stored relation.

The operators are the ones ``Javagui/viewer/update2/CommandGenerator.java`` uses
-- the Java GUI is the reference implementation of relation editing in SECONDO --
but written in **SOS text syntax**, the same language the console takes:

* load    ``query <Rel> feed [filter…] [project…] [sortby…] addid consume``
* insert  ``query <Rel> inserttuple[<value>…] consume``
* delete  ``query <Rel> deletebyid[[const tid value <id>]] count``
* update  ``query <Rel> updatebyid[[const tid value <id>]; <Attr>: <value>…] count``

The Java GUI sends these as *nested lists* instead, and this module did too at
first. That cost a second entry point into the native module, because
``SecondoInterface::Secondo(text, …)`` hardcodes ``CMD_LEVEL_TEXT`` and a leading
``(`` does not switch levels. It bought nothing: checked against a live server,
the SOS form handles every case the list form does -- a string containing ``"``,
``,`` or ``]``; a ``text`` holding an escaped ``</text--->``; ``point`` and
``region`` constants -- and its assignment list (``Attr: value``) is simpler than
the list form's ``(Attr (fun (tupleN TUPLE) value))``. So the commands go out
through the ordinary ``Session.run`` like every other command the backend issues.

**Index maintenance.** SECONDO does not keep secondary indexes in step with a
relation by itself. ``retrieveIndices`` (CommandGenerator.java:395) finds them by
naming convention -- a btree or rtree object called ``<Rel>_<Attr>`` -- and each
DML command is chained into the matching ``insert``/``delete``/``update`` btree
or rtree operator (``<stream> <index> insertbtree[<attr>]``). Without that an
edited relation and its indexes disagree, which is a silent wrong answer to any
later query that uses the index.
"""
from __future__ import annotations

import re
from dataclasses import dataclass, field

from .catalog import parse_objects
from .nlwriter import InvalidValue, literal
from .table import TID_TYPE

# CommandGenerator.java:418 -- the first underscore splits relation from attribute.
_INDEX_NAME = re.compile(r"\A([^_]+)_(.+)\Z")


@dataclass
class Index:
    name: str
    attribute: str
    # "btree" or "rtree" -- the operator family to maintain it with.
    kind: str


@dataclass
class Indexes:
    """The indexes that have to be maintained alongside one relation."""

    entries: list[Index] = field(default_factory=list)

    def __bool__(self) -> bool:
        return bool(self.entries)


def find_indexes(objects_text: str, relation: str, attributes: list[str]) -> Indexes:
    """Indexes named ``<relation>_<attr>`` over one of ``attributes``.

    Mirrors ``CommandGenerator.retrieveIndices``: the relation part must match
    case-insensitively in its first character and exactly after it -- which is
    how the Java GUI copes with SECONDO lowercasing an object's initial.
    """
    found = Indexes()
    for obj in parse_objects(objects_text):
        kind = str(obj.get("type"))
        if kind not in ("btree", "rtree"):
            continue
        m = _INDEX_NAME.match(str(obj.get("name")))
        if not m:
            continue
        rel_part, attr = m.group(1), m.group(2)
        if rel_part.lower() != relation.lower() or rel_part[1:] != relation[1:]:
            continue
        if attr not in attributes:
            continue
        found.entries.append(Index(str(obj["name"]), attr, kind))
    return found


def _maintain(verb: str, indexes: Indexes) -> str:
    """The index-maintaining operators to chain after a DML operator.

    ``<index> <verb>btree[<attr>]``, one per index, as
    ``Algebras/BTree/BTree.examples:49`` chains them.
    """
    return "".join(
        f" {i.name} {verb}{i.kind}[{i.attribute}]" for i in indexes.entries
    )


def load_command(
    relation: str,
    filters: list[str] | None = None,
    project: list[str] | None = None,
    sort: list[str] | None = None,
) -> str:
    """``CommandGenerator.generateLoad`` -- the only load that yields TIDs.

    ``addid`` appends the ``TID`` attribute that every later update addresses the
    tuple by; without it a table is read-only.
    """
    parts = [f"query {relation} feed"]
    for f in filters or []:
        if f.strip():
            parts.append(f"filter [ {f} ]")
    if project:
        parts.append("project [ " + ", ".join(project) + " ]")
    if sort:
        parts.append("sortby [ " + ", ".join(sort) + " ]")
    parts.append("addid consume")
    return " ".join(parts)


def _values(
    columns: list[dict], values: dict[str, str], *, skip_missing: bool
) -> list[tuple[str, str]]:
    """``(name, literal)`` for the given cell values, in column order.

    Raises ``InvalidValue`` naming the attribute whose value will not convert.
    """
    out: list[tuple[str, str]] = []
    for col in columns:
        name, type_name = col["name"], col["type"]
        if type_name == TID_TYPE:
            continue  # never written; it identifies the tuple
        if name not in values:
            if skip_missing:
                continue
            raise InvalidValue(name, type_name, "no value given")
        out.append((name, literal(name, type_name, values[name])))
    return out


def tid_const(tid: int) -> str:
    """A tuple identifier as an SOS constant -- ``UpdateRelation.examples:36``."""
    return f"[const tid value {tid}]"


def insert_command(relation: str, columns: list[dict], values: dict[str, str],
                   indexes: Indexes | None = None) -> str:
    """``CommandGenerator.generateInsert``. Every non-TID attribute needs a value.

    ``consume`` rather than ``count`` so the answer carries the inserted tuple --
    and with it the TID the server assigned, which the caller needs to address the
    new row afterwards.
    """
    vals = _values(columns, values, skip_missing=False)
    fields = ", ".join(v for _, v in vals)
    return (
        f"query {relation} inserttuple[{fields}]"
        f"{_maintain('insert', indexes or Indexes())} consume"
    )


def delete_command(relation: str, tid: int, indexes: Indexes | None = None) -> str:
    """``CommandGenerator.generateDelete``."""
    return (
        f"query {relation} deletebyid[{tid_const(tid)}]"
        f"{_maintain('delete', indexes or Indexes())} count"
    )


def update_command(relation: str, tid: int, columns: list[dict],
                   values: dict[str, str], indexes: Indexes | None = None) -> str:
    """``CommandGenerator.generateUpdate`` -- only the changed attributes."""
    vals = _values(columns, values, skip_missing=True)
    if not vals:
        raise InvalidValue("<tuple>", "rel", "no attribute to update")
    assignments = ", ".join(f"{name}: {value}" for name, value in vals)
    return (
        f"query {relation} updatebyid[{tid_const(tid)}; {assignments}]"
        f"{_maintain('update', indexes or Indexes())} count"
    )
