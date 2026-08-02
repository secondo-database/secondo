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
    # One entry per bridge call, with the two half-of-the-answer flags as they
    # arrived, so the tests can assert nobody asks for a half it will not read.
    calls: list[dict] = []

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

        def secondo(self, command: str, want_tree: bool = True,
                    want_text: bool = True) -> dict:
            calls.append({"command": command,
                          "want_tree": want_tree, "want_text": want_text})
            return {"text": "()" if want_text else "",
                    "tree": [] if want_tree else None}

        def secondo_auto(self, command: str, optimizer_addressed: bool = False,
                         want_tree: bool = True, want_text: bool = True):
            calls.append({"command": command,
                          "want_tree": want_tree, "want_text": want_text})
            return {
                "level": 1,
                "text": "()" if want_text else "",
                "tree": [] if want_tree else None,
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
    session_mod._calls = calls
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


def test_run_asks_for_no_tree(session_mod):
    """`run` reads only the text, and building the Python objects is a walk of
    the whole answer -- `list objects` used to build a tree to throw away."""
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        assert await session.run("list objects") == "()"
        return session_mod._calls

    assert asyncio.run(scenario()) == [
        {"command": "list objects", "want_tree": False, "want_text": True}
    ]


def test_run_tree_asks_for_no_text(session_mod):
    """The mirror image: `run_tree` discards the text, so rendering the answer
    as a nested list is pure waste. For `query roads` that string is 81 MB."""
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        assert await session.run_tree("list objects") == []
        return session_mod._calls

    assert asyncio.run(scenario()) == [
        {"command": "list objects", "want_tree": True, "want_text": False}
    ]


def test_execute_can_ask_for_neither_half(session_mod):
    """What /api/query does for `view:"none"` -- a command run for its effect."""
    mgr = session_mod.SessionManager()

    async def scenario():
        session = await mgr.create()
        result = await session.execute(
            "query ten", want_tree=False, want_text=False
        )
        assert result.text == ""
        assert result.tree is None
        return session_mod._calls

    assert asyncio.run(scenario()) == [
        {"command": "query ten", "want_tree": False, "want_text": False}
    ]


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

        def secondo(self, command: str, want_tree: bool = True,
                    want_text: bool = True) -> dict:
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
