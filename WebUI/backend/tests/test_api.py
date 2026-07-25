"""API smoke tests that do NOT require a live SecondoMonitor.

The SECONDO connection is faked so the FastAPI routing, session cookie handling
and error mapping can be tested in isolation. Real end-to-end verification
against a monitor is documented in WebUI/README.md.
"""
from __future__ import annotations

import sys
import types

import pytest
from fastapi.testclient import TestClient


# A relation of points, the shape the GeoJSON conversion recognizes. Used as an
# SQL result so the tests can prove the conversion still sees the *result* half
# of the server's (plan result costs) answer.
KINOS = '((rel (tuple ((Name string) (GeoData point)))) (("Kino" (1.0 2.0))))'


def _fake_native(optimizer: bool = True):
    """A stand-in for the pybind11 module, without a SECONDO server."""
    fake = types.ModuleType("secondo_native")

    # A deliberately crude copy of the kernel's rule -- acceptable only because
    # this is a fake; the real code calls stripOptimizerPrefix in C++.
    def strip_optimizer_prefix(command: str):
        stripped = command.lstrip()
        if stripped.lower().startswith("optimizer "):
            return True, stripped[len("optimizer ") :].lstrip()
        return False, command

    fake.strip_optimizer_prefix = strip_optimizer_prefix

    class FakeConnection:
        def __init__(self, *_args, **_kwargs):
            self.db = None
            self.directives: list[str] = []

        def optimizer_available(self) -> bool:
            return optimizer

        def optimizer_command(self, directive: str) -> str:
            self.directives.append(directive)
            return "ok"

        def secondo(self, command: str) -> str:
            if command.strip() == "list databases":
                return "(inquiry (databases (BERLINTEST OPT)))"
            if command.startswith("open database"):
                return "()"
            if command == "query mehringdamm":
                return "(point (9396.0 9871.0))"
            if command == "query umlaut":
                # names decoded from Latin-1 arrive as ordinary Python str
                return '((rel (tuple ((Name string) (Pos point)))) (("Stölpchensee" (1.0 2.0))))'
            if command == "query boom":
                raise ValueError("unexpected non-RuntimeError")
            raise RuntimeError("SECONDO error 3 (pos 0): not evaluable")

        # The answers the server gives once it classifies the command itself.
        def secondo_auto(self, command: str, optimizer_addressed: bool = False):
            assert optimizer, "secondo_auto must not be used without an optimizer"
            cmd = command.strip()
            if cmd == "select * from kinos":
                return {
                    "level": 2,
                    "text": KINOS,
                    "plan": "kinos feed consume",
                    "costs": 12.5,
                    "message": None,
                }
            if cmd.startswith("create table") or cmd.startswith("drop table"):
                # The optimizer carried it out itself while translating.
                return {
                    "level": 2,
                    "text": "()",
                    "plan": "done",
                    "costs": 0.0,
                    "message": None,
                }
            if cmd == "showOptions":
                return {
                    "level": 3,
                    "text": "()",
                    "plan": None,
                    "costs": None,
                    "message": "  subqueries: on\n  rewriteInference: off",
                }
            if cmd == "select * from nosuchrel":
                raise RuntimeError(
                    "SECONDO error 33 (pos 0): Optimization failed: "
                    "Unknown relation nosuchrel"
                )
            return {
                "level": 1,
                "text": self.secondo(cmd),
                "plan": None,
                "costs": None,
                "message": None,
            }

        def close(self):
            pass

    fake.Connection = FakeConnection
    return fake


@pytest.fixture()
def client(monkeypatch):
    # Stub the native module before app import so config/session load cleanly.
    monkeypatch.setitem(sys.modules, "secondo_native", _fake_native())

    from app.main import app  # imported after the stub is in place

    return TestClient(app)


@pytest.fixture()
def plain_client(monkeypatch):
    """A client talking to a server built or configured without the optimizer."""
    fake = _fake_native(optimizer=False)
    monkeypatch.setitem(sys.modules, "secondo_native", fake)

    import app.session as session_mod

    from app.main import app

    monkeypatch.setattr(session_mod, "secondo_native", fake)
    return TestClient(app)


def test_health(client):
    r = client.get("/api/health")
    assert r.status_code == 200
    body = r.json()
    assert body["status"] == "ok"
    assert body["config_error"] is None


# Without a config file the client's runtime flags do not match the server's and
# the first command deadlocks in a socket read that has no timeout -- health
# keeps answering while every other endpoint hangs forever. The bridge must
# refuse to open the connection instead, and say why.


def test_missing_config_is_refused_not_hung(client, monkeypatch):
    from app.config import settings

    monkeypatch.setattr(settings, "secondo_config", "/nowhere/SecondoConfig.ini")
    r = client.get("/api/databases")
    assert r.status_code == 503
    assert "/nowhere/SecondoConfig.ini" in r.json()["detail"]
    assert "SECONDO_CONFIG" in r.json()["detail"]


def test_unconfigured_bridge_is_refused_not_hung(client, monkeypatch):
    from app.config import settings

    monkeypatch.setattr(settings, "secondo_config", None)
    r = client.post("/api/query", json={"command": "query mehringdamm"})
    assert r.status_code == 503
    assert "SECONDO_CONFIG" in r.json()["detail"]


def test_health_reports_a_broken_config(client, monkeypatch):
    from app.config import settings

    monkeypatch.setattr(settings, "secondo_config", None)
    body = client.get("/api/health").json()
    assert body["status"] == "misconfigured"
    assert "SECONDO_CONFIG" in body["config_error"]


def test_query_and_session_cookie(client):
    r = client.post("/api/query", json={"command": "query mehringdamm"})
    assert r.status_code == 200
    assert r.json()["text"] == "(point (9396.0 9871.0))"
    assert "secondo_sid" in r.cookies


def test_secondo_error_maps_to_400(client):
    r = client.post("/api/query", json={"command": "query bogus"})
    assert r.status_code == 400
    assert "not evaluable" in r.json()["detail"]


def test_unexpected_error_still_returns_json(client, monkeypatch):
    # A non-RuntimeError from the connection must not become a plain-text 500
    # (which would break the browser's response.json()). Patch the reference
    # bound in app.session directly (module-level `import secondo_native`).
    import app.session as session_mod

    class Boom:
        def __init__(self, *a, **k):
            pass

        def optimizer_available(self):
            return False

        def secondo(self, command):
            raise ValueError("kaboom")

        def close(self):
            pass

    import types

    fake = types.ModuleType("secondo_native")
    fake.Connection = Boom
    fake.strip_optimizer_prefix = lambda command: (False, command)
    monkeypatch.setattr(session_mod, "secondo_native", fake)

    from app.main import app

    c = TestClient(app, raise_server_exceptions=False)  # observe the JSON 500
    r = c.post("/api/query", json={"command": "anything"})
    assert r.status_code == 500
    assert r.headers["content-type"].startswith("application/json")
    assert "ValueError" in r.json()["detail"]


def test_unicode_names_roundtrip(client):
    r = client.post("/api/query", json={"command": "query umlaut"})
    assert r.status_code == 200
    body = r.json()
    assert body["geojson"]["features"][0]["properties"]["Name"] == "Stölpchensee"


def test_databases_and_open_tracking(client):
    client.post("/api/query", json={"command": "open database berlintest"})
    r = client.get("/api/databases")
    assert r.status_code == 200
    body = r.json()
    assert "BERLINTEST" in body["databases"]
    assert body["open"] == "berlintest"
    assert body["optimizer"] is True


# --- SQL through the optimizer -------------------------------------------


def test_sql_query_returns_plan_costs_and_still_renders(client):
    """The point of the whole design: the *result* half of the server's
    (plan result costs) answer goes through the unchanged GeoJSON pipeline, so
    an optimized query draws on the map exactly like a kernel query."""
    r = client.post("/api/query", json={"command": "select * from kinos"})
    assert r.status_code == 200
    body = r.json()
    assert body["level"] == 2
    assert body["plan"] == "kinos feed consume"
    assert body["costs"] == 12.5
    assert body["text"] == KINOS
    assert body["geojson"]["features"][0]["geometry"]["coordinates"] == [1.0, 2.0]
    assert body["plan_only"] is False
    assert body["executed_by_optimizer"] is False


def test_sql_ddl_is_executed_by_the_optimizer(client):
    r = client.post(
        "/api/query", json={"command": "create table t columns [a: int]"}
    )
    assert r.status_code == 200
    body = r.json()
    assert body["executed_by_optimizer"] is True
    # The "done" sentinel is the server's way of saying there is no plan to
    # run; it must never reach the UI as a plan.
    assert body["plan"] is None


def test_optimizer_directive_returns_its_output(client):
    r = client.post("/api/query", json={"command": "showOptions"})
    assert r.status_code == 200
    body = r.json()
    assert body["level"] == 3
    assert "subqueries: on" in body["message"]
    # Indentation is what makes showOptions readable, so it is not trimmed.
    assert body["message"].startswith("  ")
    assert body["text"] == "()"


def test_optimizer_prefix_means_plan_only(client):
    r = client.post("/api/query", json={"command": "optimizer select * from kinos"})
    assert r.status_code == 200
    body = r.json()
    assert body["plan_only"] is True
    assert body["plan"] == "kinos feed consume"


def test_sql_error_passes_the_server_message_through(client):
    r = client.post("/api/query", json={"command": "select * from nosuchrel"})
    assert r.status_code == 400
    # Error 33 covers four situations; only the message says which, so it is
    # forwarded verbatim rather than translated.
    assert "Unknown relation nosuchrel" in r.json()["detail"]


# --- a server without the optimizer ---------------------------------------


def test_without_optimizer_capability_is_reported(plain_client):
    r = plain_client.get("/api/databases")
    assert r.json()["optimizer"] is False


def test_without_optimizer_kernel_commands_still_work(plain_client):
    # The fake's secondo_auto asserts if it is ever reached: without the
    # optimizer, commands go out at the level derived from their text.
    r = plain_client.post("/api/query", json={"command": "query mehringdamm"})
    assert r.status_code == 200
    assert r.json()["text"] == "(point (9396.0 9871.0))"
    assert r.json()["level"] is None


def test_without_optimizer_the_prefix_is_refused(plain_client):
    r = plain_client.post(
        "/api/query", json={"command": "optimizer select * from kinos"}
    )
    assert r.status_code == 400
    assert "not available" in r.json()["detail"]
