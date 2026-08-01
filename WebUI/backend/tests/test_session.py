"""Session lifecycle tests.

Every session holds a SECONDO connection and the server forks a process per
connection, so idle sessions must be reclaimed or they leak (observed as
orphaned SecondoBDB processes).
"""
from __future__ import annotations

import asyncio
import threading
import time
import types

import pytest


def _install_fake(monkeypatch, *, optimizer=True, probe_raises=False):
    import app.session as session_mod

    closed: list[str] = []
    directives: list[str] = []

    class FakeConnection:
        def __init__(self, *_a, **_k):
            pass

        def optimizer_available(self) -> bool:
            if probe_raises:
                raise RuntimeError("this server does not know the question")
            return optimizer

        def optimizer_command(self, directive: str) -> str:
            directives.append(directive)
            return "ok"

        def secondo(self, command: str, want_tree: bool = True) -> dict:
            return {"text": "()", "tree": []}

        def secondo_auto(self, command: str, optimizer_addressed: bool = False,
                         want_tree: bool = True):
            return {
                "level": 1,
                "text": "()",
                "tree": [],
                "plan": None,
                "costs": None,
                "message": None,
            }

        def close(self) -> None:
            closed.append("closed")

    fake = types.ModuleType("secondo_native")
    fake.Connection = FakeConnection
    fake.strip_optimizer_prefix = lambda command: (False, command)
    monkeypatch.setattr(session_mod, "secondo_native", fake)
    session_mod._closed = closed  # expose for assertions
    session_mod._directives = directives
    return session_mod


@pytest.fixture()
def session_mod(monkeypatch):
    return _install_fake(monkeypatch)


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


def test_capability_is_probed_at_connect(session_mod):
    mgr = session_mod.SessionManager()
    assert asyncio.run(mgr.create()).optimizer is True


def test_failing_probe_degrades_to_no_optimizer(monkeypatch):
    """An older server answers an unknown tag with an error block instead of
    yes/no. That must leave a usable session, not kill it."""
    mod = _install_fake(monkeypatch, probe_raises=True)
    assert asyncio.run(mod.SessionManager().create()).optimizer is False


def test_object_changing_command_refreshes_the_optimizer_catalog(session_mod):
    """The optimizer only rereads the schema when the database changes, so a
    kernel `let` would otherwise stay invisible to SQL for the whole session."""
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        await session.execute("let x = 3")
        await session.execute("query x")  # not an object change
        return session_mod._directives

    assert asyncio.run(scenario()) == ["updateCatalog"]


def test_no_catalog_refresh_without_an_optimizer(monkeypatch):
    mod = _install_fake(monkeypatch, optimizer=False)
    mgr = mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        await session.execute("let x = 3")
        return mod._directives

    assert asyncio.run(scenario()) == []


def test_close_waits_for_a_command_in_flight(session_mod):
    """Closing must not free the connection under a running command.

    `Connection::close` deletes the SecondoInterfaceCS and with it the socket's
    SocketBuffer; a command still reading the response dereferences the freed
    buffer (`SocketIO.cpp:153`) and the whole bridge process dies of a general
    protection fault. The frontend closes its session with a `sendBeacon` on
    `pagehide`, so reloading the page mid-query is exactly this race.
    """
    order: list[str] = []
    started = None

    class SlowConnection:
        def __init__(self, *_a, **_k):
            pass

        def optimizer_available(self) -> bool:
            return False

        def secondo(self, command: str, want_tree: bool = True) -> dict:
            started.set()
            time.sleep(0.2)  # still on the socket
            order.append("command finished")
            return {"text": "()", "tree": []}

        def close(self) -> None:
            order.append("closed")

    session_mod.secondo_native.Connection = SlowConnection

    async def scenario():
        nonlocal started
        started = threading.Event()
        mgr = session_mod.SessionManager()
        session = await mgr.create()
        running = asyncio.create_task(session.run("query ten"))
        await asyncio.to_thread(started.wait, 5)
        await mgr.close(session.id)
        await running
        return mgr.count()

    assert asyncio.run(scenario()) == 0
    assert order == ["command finished", "closed"]


def test_close_removes_the_session_before_waiting(session_mod):
    """The id is popped first, so a request arriving while the close waits for
    a running command cannot pick the session up again."""
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        async with session.lock:  # pretend a command is running
            closing = asyncio.create_task(mgr.close(session.id))
            await asyncio.sleep(0)  # let it get as far as the lock
            found = mgr.get(session.id)
        await closing
        return found

    assert asyncio.run(scenario()) is None
