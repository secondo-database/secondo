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

# A database holding one editable relation `ten` with a btree over its only
# attribute, so the table tests exercise index maintenance too.
OBJECTS = (
    "(inquiry (objects (OBJECTS "
    "(OBJECT ten () ((rel (tuple ((No int)))))) "
    "(OBJECT ten_No () ((btree (tuple ((No int))) int))) "
    "(OBJECT mehringdamm () ((point))) "
    ")))"
)
TEN = "((rel (tuple ((No int)))) ((1) (2)))"
TEN_WITH_TIDS = "((rel (tuple ((No int) (TID tid)))) ((1 11) (2 12)))"


def _fake_native(optimizer: bool = True):
    """A stand-in for the pybind11 module, without a SECONDO server."""
    fake = types.ModuleType("secondo_native")
    # Every command any connection was asked to run, so the table tests can
    # assert on the exact SECONDO commands the bridge generates.
    fake.commands = []
    fake.directives = []

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
            fake.directives.append(directive)
            return "ok"

        def secondo(self, command: str) -> str:
            fake.commands.append(command)
            if command.strip() == "list databases":
                return "(inquiry (databases (BERLINTEST OPT)))"
            if command.strip() == "list objects":
                return OBJECTS
            if command.startswith("open database"):
                return "()"
            if command == "query mehringdamm":
                return "(point (9396.0 9871.0))"
            if command == "query ten":
                return TEN
            if command == "query ten feed addid consume":
                return TEN_WITH_TIDS
            if command.endswith(" transaction"):
                return "()"
            if command == "query umlaut":
                # names decoded from Latin-1 arrive as ordinary Python str
                return '((rel (tuple ((Name string) (Pos point)))) (("Stölpchensee" (1.0 2.0))))'
            if command == "query boom":
                raise ValueError("unexpected non-RuntimeError")
            # The relation-editing commands (app/updates.py); ordinary SOS text
            # like every other command the backend issues.
            if command.endswith(" count") and (
                "deletebyid[" in command or "updatebyid[" in command
            ):
                if "value 666]" in command:
                    raise RuntimeError("SECONDO error 8 (pos 0): command failed")
                # A tuple another session already deleted: the operator reports
                # that it touched nothing.
                return "(int 0)" if "value 777]" in command else "(int 1)"
            if "inserttuple[" in command:
                return "((rel (tuple ((No int) (TID tid)))) ((7 99)))"
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
    fake = _fake_native()
    monkeypatch.setitem(sys.modules, "secondo_native", fake)

    import app.session as session_mod

    from app.main import app  # imported after the stub is in place

    monkeypatch.setattr(session_mod, "secondo_native", fake)
    c = TestClient(app)
    # The commands the bridge generated, for the table tests to assert on.
    c.fake = fake
    return c


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


# --- the table view (Milestone 9) -----------------------------------------


def test_query_attaches_a_table_payload(client):
    r = client.post("/api/query", json={"command": "query ten"})
    body = r.json()
    assert body["table"]["columns"] == [{"name": "No", "type": "int", "atomic": True}]
    assert body["table"]["rows"] == [[1], [2]]
    # `query ten` is exactly one stored relation, so the table can offer to edit it.
    assert body["relation"] == "ten"


def test_a_derived_result_names_no_relation(client):
    """There is nothing to write a filtered or joined result back to."""
    body = client.post("/api/query", json={"command": "select * from kinos"}).json()
    assert body["table"] is not None  # it is still readable as rows
    assert body["relation"] == "kinos"  # ...and this plan is a bare relation
    body = client.post("/api/query", json={"command": "query mehringdamm"}).json()
    assert body["table"] is None
    assert body["relation"] is None


def test_table_load_asks_for_tids(client):
    r = client.post("/api/table/load", json={"relation": "ten"})
    assert r.status_code == 200
    body = r.json()
    # Without `addid` there is no TID and nothing is editable.
    assert body["command"] == "query ten feed addid consume"
    assert body["table"]["tidIndex"] == 1
    assert body["table"]["relation"] == "ten"
    assert body["table"]["rows"] == [[1, 11], [2, 12]]


def test_table_load_refuses_a_non_relation(client):
    r = client.post("/api/table/load", json={"relation": "mehringdamm"})
    assert r.status_code == 400
    assert "not a relation" in r.json()["detail"]


def test_table_load_refuses_an_unknown_object(client):
    assert client.post("/api/table/load", json={"relation": "nosuch"}).status_code == 400


def test_a_relation_name_is_never_pasted_into_a_command_unchecked(client):
    """The name is concatenated into a SECONDO command, so it is validated both
    as a name and against the catalog before anything runs."""
    r = client.post("/api/table/load", json={"relation": "ten feed consume; kill ten"})
    assert r.status_code == 400
    assert not any("kill ten" in c for c in client.fake.commands)


def test_commit_generates_the_update_delete_and_insert_commands(client):
    client.post("/api/query", json={"command": "open database berlintest"})
    client.fake.commands.clear()
    r = client.post(
        "/api/table/commit",
        json={
            "relation": "ten",
            "updates": [{"tid": 11, "values": {"No": "42"}}],
            "deletes": [12],
            "inserts": [{"values": {"No": "7"}}],
        },
    )
    assert r.status_code == 200
    assert r.json()["applied"] == 3
    # The TID the server assigned to the new tuple comes back.
    assert r.json()["inserted"] == [99]

    dml = [c for c in client.fake.commands if "byid[" in c or "inserttuple[" in c]
    # Updates, then deletes, then inserts -- inserts last so their new TIDs are
    # assigned against a settled relation. Every command maintains the btree.
    assert dml == [
        "query ten updatebyid[[const tid value 11]; No: 42]"
        " ten_No updatebtree[No] count",
        "query ten deletebyid[[const tid value 12]] ten_No deletebtree[No] count",
        "query ten inserttuple[7] ten_No insertbtree[No] consume",
    ]


def test_deletes_run_highest_identifier_first(client):
    """A `deletebyid` shifts the identifiers of the tuples after it, so deleting
    2, 5, 8 in that order removes the 2nd, 6th and 10th tuple. Verified against a
    live server; descending order is what makes a multi-row delete correct."""
    client.fake.commands.clear()
    client.post("/api/table/commit", json={"relation": "ten", "deletes": [2, 8, 5]})
    tids = [
        int(c.split("value ")[1].split("]")[0])
        for c in client.fake.commands
        if "deletebyid[" in c
    ]
    assert tids == [8, 5, 2]


def test_commit_brackets_the_batch_in_a_transaction(client):
    client.fake.commands.clear()
    client.post(
        "/api/table/commit",
        json={"relation": "ten", "deletes": [12]},
    )
    assert "begin transaction" in client.fake.commands
    assert "commit transaction" in client.fake.commands
    assert client.fake.commands.index("begin transaction") < client.fake.commands.index(
        "commit transaction"
    )


def test_commit_tells_the_optimizer_the_relation_changed(client):
    client.fake.directives.clear()
    client.post("/api/table/commit", json={"relation": "ten", "deletes": [12]})
    assert "updateRel(ten)" in client.fake.directives


def test_commit_rejects_a_bad_value_before_running_anything(client):
    client.fake.commands.clear()
    r = client.post(
        "/api/table/commit",
        json={"relation": "ten", "updates": [{"tid": 11, "values": {"No": "twelve"}}]},
    )
    assert r.status_code == 400
    assert "No (int)" in r.json()["detail"]
    assert not any(c.startswith("(query") for c in client.fake.commands)


def test_a_failing_command_aborts_the_whole_batch(client):
    client.fake.commands.clear()
    r = client.post(
        "/api/table/commit",
        json={"relation": "ten", "deletes": [12, 666, 13]},
    )
    assert r.status_code == 400
    assert "no change was applied" in r.json()["detail"]
    assert "abort transaction" in client.fake.commands
    assert "commit transaction" not in client.fake.commands
    # The command after the failing one never ran.
    assert not any("(tid 13)" in c for c in client.fake.commands)


def test_a_tuple_deleted_by_someone_else_is_reported(client):
    """UpdateViewer2 has no locking either; it checks the operator's count."""
    r = client.post("/api/table/commit", json={"relation": "ten", "deletes": [777]})
    assert r.status_code == 400
    assert "no longer exists" in r.json()["detail"]


def test_an_empty_commit_runs_nothing(client):
    client.fake.commands.clear()
    r = client.post("/api/table/commit", json={"relation": "ten"})
    assert r.status_code == 200
    assert r.json()["applied"] == 0
    assert not any(c.startswith("(") for c in client.fake.commands)


def test_catalog_marks_which_objects_are_editable_relations(client):
    objs = {o["name"]: o for o in client.get("/api/objects").json()["objects"]}
    assert objs["ten"]["relation"] is True
    assert objs["mehringdamm"]["relation"] is False


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
