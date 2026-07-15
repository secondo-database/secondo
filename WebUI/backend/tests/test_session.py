"""Session lifecycle tests.

Every session holds a SECONDO connection and the server forks a process per
connection, so idle sessions must be reclaimed or they leak (observed as
orphaned SecondoBDB processes).
"""
from __future__ import annotations

import asyncio
import types

import pytest


@pytest.fixture()
def session_mod(monkeypatch):
    import app.session as session_mod

    closed: list[str] = []

    class FakeConnection:
        def __init__(self, *_a, **_k):
            pass

        def secondo(self, command: str) -> str:
            return "()"

        def close(self) -> None:
            closed.append("closed")

    fake = types.ModuleType("secondo_native")
    fake.Connection = FakeConnection
    monkeypatch.setattr(session_mod, "secondo_native", fake)
    session_mod._closed = closed  # expose for assertions
    return session_mod


def test_idle_session_is_reaped(session_mod):
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        session.last_used -= 10_000  # pretend it has been idle for ages
        reaped = await mgr.reap(max_idle=60)
        return reaped, mgr.count()

    reaped, remaining = asyncio.run(scenario())
    assert reaped == 1
    assert remaining == 0
    assert session_mod._closed == ["closed"]  # connection really was closed


def test_recent_session_is_kept(session_mod):
    mgr = session_mod.SessionManager()

    async def scenario():
        await mgr.create()
        return await mgr.reap(max_idle=60), mgr.count()

    reaped, remaining = asyncio.run(scenario())
    assert reaped == 0
    assert remaining == 1


def test_busy_session_is_never_reaped(session_mod):
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        session.last_used -= 10_000
        async with session.lock:  # simulate a command in flight
            return await mgr.reap(max_idle=60)

    assert asyncio.run(scenario()) == 0


def test_get_touches_session_so_it_survives(session_mod):
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        session.last_used -= 10_000
        mgr.get(session.id)  # an active client keeps it alive
        return await mgr.reap(max_idle=60)

    assert asyncio.run(scenario()) == 0
