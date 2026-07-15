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


@pytest.fixture()
def client(monkeypatch):
    # Stub the native module before app import so config/session load cleanly.
    fake = types.ModuleType("secondo_native")

    class FakeConnection:
        def __init__(self, *_args, **_kwargs):
            self.db = None

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

        def close(self):
            pass

    fake.Connection = FakeConnection
    monkeypatch.setitem(sys.modules, "secondo_native", fake)

    from app.main import app  # imported after the stub is in place

    return TestClient(app)


def test_health(client):
    r = client.get("/api/health")
    assert r.status_code == 200
    assert r.json()["status"] == "ok"


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

        def secondo(self, command):
            raise ValueError("kaboom")

        def close(self):
            pass

    import types

    fake = types.ModuleType("secondo_native")
    fake.Connection = Boom
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
