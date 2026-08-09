"""Per-browser-session SECONDO connections.

Mirrors WebGui2's per-session connection model: each browser session owns one
`secondo_native.Connection`. Every command on a session is serialized behind an
asyncio lock and the blocking C++ call is run in a worker thread, so a session's
commands keep their order and a second request on the same session waits in the
event loop instead of tying up a worker.

The lock is per session and stays that way, because a session is stateful -- it
has a database open, possibly a transaction -- so its commands have to keep
their order. Sessions do not wait for each other: the client library underneath
supports one connection per thread, so their commands run in parallel, which is
what the server was doing anyway (it forks a process per connection). See
WebUI/backend/native/secondo_native.cpp.
"""
from __future__ import annotations

import asyncio
import contextlib
import logging
import re
import secrets
import time
from collections.abc import Callable
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import secondo_native  # provided by ../native (see config.py)

from .config import require_config, settings

logger = logging.getLogger("secondo.webui")

# Command levels the server resolves to; see include/SQLLanguage.h.
LEVEL_SQL = 2
LEVEL_OPT_DIRECTIVE = 3

# Kernel commands that add, change or remove objects. After one of these the
# optimizer's idea of the catalog is stale (it only rereads it when the database
# changes), so it is told to refresh -- see `settings.auto_update_catalog`.
CATALOG_CMD = re.compile(r"^\s*(let|create|delete|update|derive|kill)\b", re.IGNORECASE)

# What a caller passes to `execute` to have the answer *read* instead of handed
# back: an object with `begin(type)` / `elem(tuple)`, which the bridge pushes a
# relation's tuples into as it decodes them, and `read(type, tuples, tree)` for
# the answers it could not stream. `app.convert.Answer` is the implementation;
# the type stays loose here so the session layer does not depend on the payload
# layer.
ResultSink = Any


@dataclass
class CommandResult:
    """One command's outcome, as the API layer needs it.

    `text` is always the *result* nested list -- for an SQL command the result
    half of the server's `(plan result costs)` answer -- so the GeoJSON/temporal
    conversion works on it regardless of which language the command was in.
    """

    text: str
    # The same list as `text`, already as Python objects -- the bridge builds it
    # from the ListExpr it is holding anyway, so nothing re-parses the text.
    # None when a sink read the answer instead: it is then in the sink.
    tree: Any = None
    level: int | None = None
    plan: str | None = None
    costs: float | None = None
    # What an optimizer directive (showOptions, setOption, ...) printed.
    message: str | None = None
    # The user wrote the "optimizer " prefix on SQL: optimized, not executed.
    plan_only: bool = False
    # A create/drop the optimizer carried out itself while translating.
    executed_by_optimizer: bool = False


@dataclass
class Session:
    id: str
    conn: "secondo_native.Connection"
    open_db: str | None = None
    # Whether the server this session is connected to can run SQL. Probed once
    # at connect (see SessionManager.create) and never cached beyond the
    # session: it is a property of that server, not of the WebUI.
    optimizer: bool = False
    lock: asyncio.Lock = field(default_factory=asyncio.Lock)
    # Monotonic timestamp of the last use, for idle reaping.
    last_used: float = field(default_factory=time.monotonic)
    # Files this session uploaded (see /api/upload), removed when it closes.
    # They are only needed until the command that reads them has run, and
    # nothing else will ever clean them up: without this every page reload
    # would leave another track behind in the temp directory.
    uploads: list[Path] = field(default_factory=list)

    def touch(self) -> None:
        self.last_used = time.monotonic()

    async def run(self, command: str) -> str:
        """Execute one kernel command; returns the result nested list as text.

        The command goes out at kernel level, so this never reaches the
        optimizer. It is the path for commands the backend issues itself
        (`list databases`, `list objects`, and the relation-editing commands in
        app/updates.py); what the user types goes through `execute`.
        """
        return (await self._run_raw(command, want_tree=False))["text"]

    async def run_tree(self, command: str) -> Any:
        """As `run`, but hands back the result as Python objects rather than as
        text. For a caller that only wants to walk the answer this skips both
        `ToString` in the bridge and re-parsing it here."""
        return (await self._run_raw(command, want_text=False))["tree"]

    async def _run_raw(
        self, command: str, *, want_tree: bool = True, want_text: bool = True
    ) -> dict:
        # Both halves are a walk of the whole answer, and `run`/`run_tree` each
        # read exactly one of them -- asking for both meant `list objects` built
        # a Python tree to throw away, and every `run` a nested-list string to
        # throw away.
        async with self.lock:
            self.touch()
            try:
                return await asyncio.to_thread(
                    self.conn.secondo, command, want_tree, want_text
                )
            finally:
                self.touch()

    async def directive(self, goal: str) -> str:
        """Run one optimizer directive (a Prolog goal) on this session.

        Same serialization as `run`; used for the catalog/statistics nudges the
        backend sends after changing objects, which the JavaGUI sends too.
        """
        async with self.lock:
            self.touch()
            try:
                return await asyncio.to_thread(self.conn.optimizer_command, goal)
            finally:
                self.touch()

    def _invoke(
        self,
        fn: Callable[..., dict],
        args: tuple,
        sink: ResultSink | None,
        discard: bool = False,
    ) -> dict:
        """Run one native call and make sure the answer has been read.

        With a sink the bridge usually reads it during the call, pushing each
        tuple over as it comes off the socket; `streamed` says whether it
        could. When it could not, the answer is here now and is read the same
        way from what the bridge did hand back.

        That has to happen here, in the worker thread and under the session
        lock. A `tuples` iterator walks the connection's nested list, and the
        next command on that connection rolls it back
        (`Connection::beginCommand`) -- handing it out past this point would
        hand out something the very next request could invalidate. Reading it
        here also keeps the work off the event loop, where it would block every
        other session for as long as it took.
        """
        raw = fn(*args, sink, discard)
        if sink is not None and not raw["streamed"]:
            sink.read(raw["type"], raw["tuples"], raw["tree"])
        return raw

    async def execute(
        self,
        command: str,
        *,
        want_tree: bool = True,
        want_text: bool = True,
        sink: ResultSink | None = None,
        discard: bool = False,
    ) -> CommandResult:
        """Execute one command the user typed, in whichever language it is in.

        With the optimizer available the command goes out without a language
        label and the *server* classifies it (see include/SQLLanguage.h), which
        is what the JavaGUI and the TTY do -- the rules live in one place and no
        client carries a copy. Without it, SQL is not on offer at all and the
        level is derived from the command text as before.

        `want_tree` and `want_text` gate the two halves of the answer
        independently: the render payloads are built from the tree, the console
        shows the text, and a command run for its effect wants neither.

        `sink` asks for the answer to be *read* rather than handed back. The
        bridge then hands a relation's tuples over one at a time as it decodes
        them, so neither side ever holds the whole result; what the sink made
        of them is the caller's to collect from the sink it passed in. See
        `_invoke` for why it cannot happen any later.

        `discard` is the same thing for a command run purely for its effect:
        the answer is read the same way and nothing is kept, so the client's
        list does not grow to it either. Asking for any form of the answer
        overrides it.
        """
        addressed, rest = secondo_native.strip_optimizer_prefix(command)
        async with self.lock:
            self.touch()
            try:
                if not self.optimizer:
                    if addressed:
                        raise RuntimeError(
                            "The optimizer is not available on this server."
                        )
                    raw = await asyncio.to_thread(
                        self._invoke,
                        self.conn.secondo,
                        (command, want_tree, want_text),
                        sink,
                        discard,
                    )
                    return CommandResult(text=raw["text"], tree=raw["tree"])

                raw = await asyncio.to_thread(
                    self._invoke,
                    self.conn.secondo_auto,
                    (rest, addressed, want_tree, want_text),
                    sink,
                    discard,
                )
                level = raw["level"]
                plan = raw["plan"]
                # For a create/drop the optimizer did the work itself while
                # translating and sends the atom "done" instead of a plan
                # (SecondoServer::CallSql); that sentinel never reaches the UI.
                executed = level == LEVEL_SQL and (plan or "").strip() == "done"
                if executed:
                    plan = None
                if level not in (LEVEL_SQL, LEVEL_OPT_DIRECTIVE):
                    # Only kernel commands can leave the optimizer's catalog
                    # behind; SQL is translated by the optimizer itself, which
                    # refreshes as it goes.
                    await self._update_catalog_if_wanted(rest)
                return CommandResult(
                    text=raw["text"],
                    tree=raw["tree"],
                    level=level,
                    plan=plan,
                    costs=raw["costs"],
                    message=raw["message"],
                    plan_only=addressed and level == LEVEL_SQL,
                    executed_by_optimizer=executed,
                )
            finally:
                self.touch()

    async def _update_catalog_if_wanted(self, command: str) -> None:
        """Keep the optimizer's catalog in step with kernel object changes.

        `embeddedOptimizerUseDatabase` only rereads the schema when the database
        name changes, so an object created with a kernel `let` would stay
        invisible to SQL. Mirrors CommandPanel.updateCatalogIfWanted; failures
        are not the user's problem and never fail their command.
        """
        if not settings.auto_update_catalog or not CATALOG_CMD.match(command):
            return
        try:
            await asyncio.to_thread(self.conn.optimizer_command, "updateCatalog")
        except Exception:  # noqa: BLE001 - best effort, never fail the command
            logger.exception("updateCatalog failed after: %s", command)


class SessionManager:
    def __init__(self) -> None:
        self._sessions: dict[str, Session] = {}
        self._create_lock = asyncio.Lock()

    async def create(self) -> Session:
        """Open a fresh SECONDO connection and register a session for it."""
        async with self._create_lock:
            # Checked here rather than at import so the process still starts
            # and can report the problem over HTTP instead of dying silently.
            require_config()
            conn = await asyncio.to_thread(
                secondo_native.Connection,
                settings.secondo_host,
                settings.secondo_port,
                settings.secondo_user,
                settings.secondo_passwd,
                settings.secondo_config,
            )
            # Ask the server whether it can run SQL, once, before any user
            # command. The probe is its own request/response on the socket, so
            # doing it at a fixed point keeps a server that does not understand
            # it failing reproducibly rather than mid-session.
            try:
                optimizer = await asyncio.to_thread(conn.optimizer_available)
            except Exception:  # noqa: BLE001 - a server without it is fine
                logger.exception("Optimizer capability probe failed")
                optimizer = False
            logger.info(
                "New SECONDO session (optimizer %s)",
                "available" if optimizer else "not available",
            )
            sid = secrets.token_urlsafe(24)
            session = Session(id=sid, conn=conn, optimizer=optimizer)
            self._sessions[sid] = session
            return session

    def get(self, sid: str | None) -> Session | None:
        if not sid:
            return None
        session = self._sessions.get(sid)
        if session is not None:
            session.touch()  # keep active sessions from being reaped
        return session

    async def close(self, sid: str) -> None:
        """Close a session's connection, once nothing is using it any more.

        The lock is not optional. `Connection::close` deletes the
        `SecondoInterfaceCS`, and with it the socket's `iostream` and its
        `SocketBuffer`; a command still in flight on another worker thread is
        sitting in `SocketBuffer::underflow` reading the response
        (`ClientServer/SocketIO.cpp:153`) and dereferences `gptr()` straight
        into the memory that was just freed.
        """
        session = self._sessions.pop(sid, None)
        if session is None:
            return
        async with session.lock:
            await asyncio.to_thread(session.conn.close)
            for path in session.uploads:
                # Best effort: a file the user already moved or a temp
                # directory that was swept is not a reason to fail a close.
                with contextlib.suppress(OSError):
                    path.unlink()
            session.uploads.clear()

    async def reap(self, max_idle: float) -> int:
        """Close sessions idle longer than `max_idle` seconds.

        Every session holds a SECONDO connection, and the server forks a
        process per connection -- without this, closed tabs and crashed
        clients would leak server processes indefinitely. Sessions currently
        running a command are never reaped.
        """
        now = time.monotonic()
        closed = 0
        for sid, session in list(self._sessions.items()):
            # `close` waits for the lock anyway; skipping a busy session here
            # keeps the reaper from parking on a long query while every other
            # idle connection stays open behind it.
            if session.lock.locked():
                continue
            if now - session.last_used > max_idle:
                await self.close(sid)
                closed += 1
        return closed

    async def close_all(self) -> None:
        for sid in list(self._sessions):
            await self.close(sid)

    def count(self) -> int:
        return len(self._sessions)


manager = SessionManager()
