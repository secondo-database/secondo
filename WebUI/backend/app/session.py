"""Per-browser-session SECONDO connections.

Mirrors WebGui2's per-session connection model: each browser session owns one
`secondo_native.Connection`. A Connection is not thread-safe and SECONDO runs
one command at a time per client, so every command on a session is serialized
behind an asyncio lock and the blocking C++ call is run in a worker thread.
"""
from __future__ import annotations

import asyncio
import logging
import secrets
import time
from dataclasses import dataclass, field

import secondo_native  # provided by ../native (see config.py)

from .config import settings

logger = logging.getLogger("secondo.webui")


@dataclass
class Session:
    id: str
    conn: "secondo_native.Connection"
    open_db: str | None = None
    lock: asyncio.Lock = field(default_factory=asyncio.Lock)
    # Monotonic timestamp of the last use, for idle reaping.
    last_used: float = field(default_factory=time.monotonic)

    def touch(self) -> None:
        self.last_used = time.monotonic()

    async def run(self, command: str) -> str:
        """Execute one command; returns the result nested list as text."""
        async with self.lock:
            self.touch()
            try:
                return await asyncio.to_thread(self.conn.secondo, command)
            finally:
                self.touch()


class SessionManager:
    def __init__(self) -> None:
        self._sessions: dict[str, Session] = {}
        self._create_lock = asyncio.Lock()

    async def create(self) -> Session:
        """Open a fresh SECONDO connection and register a session for it."""
        async with self._create_lock:
            conn = await asyncio.to_thread(
                secondo_native.Connection,
                settings.secondo_host,
                settings.secondo_port,
                settings.secondo_user,
                settings.secondo_passwd,
                settings.secondo_config,
            )
            sid = secrets.token_urlsafe(24)
            session = Session(id=sid, conn=conn)
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
