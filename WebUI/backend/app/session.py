"""Per-browser-session SECONDO connections.

Mirrors WebGui2's per-session connection model: each browser session owns one
`secondo_native.Connection`. A Connection is not thread-safe and SECONDO runs
one command at a time per client, so every command on a session is serialized
behind an asyncio lock and the blocking C++ call is run in a worker thread.
"""
from __future__ import annotations

import asyncio
import logging
import re
import secrets
import time
from dataclasses import dataclass, field

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


@dataclass
class CommandResult:
    """One command's outcome, as the API layer needs it.

    `text` is always the *result* nested list -- for an SQL command the result
    half of the server's `(plan result costs)` answer -- so the GeoJSON/temporal
    conversion works on it regardless of which language the command was in.
    """

    text: str
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

    def touch(self) -> None:
        self.last_used = time.monotonic()

    async def run(self, command: str) -> str:
        """Execute one kernel command; returns the result nested list as text.

        The level is derived from the command text, so this never reaches the
        optimizer. It is the path for commands the backend issues itself
        (`list databases`, `list objects`); what the user types goes through
        `execute`.
        """
        async with self.lock:
            self.touch()
            try:
                return await asyncio.to_thread(self.conn.secondo, command)
            finally:
                self.touch()

    async def execute(self, command: str) -> CommandResult:
        """Execute one command the user typed, in whichever language it is in.

        With the optimizer available the command goes out without a language
        label and the *server* classifies it (see include/SQLLanguage.h), which
        is what the JavaGUI and the TTY do -- the rules live in one place and no
        client carries a copy. Without it, SQL is not on offer at all and the
        level is derived from the command text as before.
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
                    return CommandResult(
                        text=await asyncio.to_thread(self.conn.secondo, command)
                    )

                raw = await asyncio.to_thread(self.conn.secondo_auto, rest, addressed)
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
        session = self._sessions.pop(sid, None)
        if session is not None:
            await asyncio.to_thread(session.conn.close)

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
            if session.lock.locked():
                continue  # busy running a command
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
