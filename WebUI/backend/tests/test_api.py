"""API smoke tests that do NOT require a live SecondoMonitor.

The SECONDO connection is faked so the FastAPI routing, session cookie handling
and error mapping can be tested in isolation. Real end-to-end verification
against a monitor is documented in WebUI/README.md.
"""
from __future__ import annotations

import re
import sys
import tempfile
import types
from pathlib import Path

import pytest
from fastapi.testclient import TestClient

from app import table as table_mod
from app.config import settings
from app.nlparser import parse
from nativefake import answer


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
    # A relation whose own attribute collides with the paging counter's name.
    "(OBJECT rowno () ((rel (tuple ((RowNo int)))))) "
    ")))"
)
# `ten` really does hold ten tuples here, so a page can be smaller than it.
TEN_TUPLES = [(n, 10 + n) for n in range(1, 11)]
# What `query ten` itself answers: the whole relation, which is where the table's
# first page is now cut from rather than from a second read of the same rows.
TEN = "((rel (tuple ((No int)))) (%s))" % " ".join(f"({n})" for n, _ in TEN_TUPLES)

# Two algebras' worth of `list operators`, including an operator registered
# twice (overloaded) and one spelled as a symbol the editor cannot type.
OPERATORS_INQUIRY = """(inquiry (operators (
  (feed ("Signature" "Syntax" "Meaning") (<text>rel -> stream</text--->
        <text>_ feed</text---> <text>Turns a relation into a stream.</text--->))
  (feed ("Signature" "Syntax") (<text>orel -> stream</text---> <text>_ feed</text--->))
  (createsuffixtree ("Signature" "Syntax") (<text>text -> suffixtree</text--->
        <text>createsuffixtree( _ )</text--->))
  (+ ("Signature" "Syntax") (<text>int x int -> int</text---> <text>_ + _</text--->))
  (nosyntax ("Signature") (<text>a -> b</text--->))
)))"""


def _page_of_ten(command: str) -> str:
    """What `query ten feed [addid] [page…] consume` answers.

    The fake reads the page out of the command instead of hardcoding one, so the
    tests prove the *generated* command selects the rows it claims to. Without
    `addid` there is no TID column, exactly as SECONDO would answer.
    """
    rows = TEN_TUPLES
    m = re.search(r"\.\w+ > (\d+)", command)
    if m:
        rows = rows[int(m.group(1)) :]
    m = re.search(r"head\[(\d+)\]", command)
    if m:
        rows = rows[: int(m.group(1))]
    if " addid" not in command:
        body = " ".join(f"({no})" for no, _ in rows)
        return f"((rel (tuple ((No int)))) ({body}))"
    body = " ".join(f"({no} {tid})" for no, tid in rows)
    return f"((rel (tuple ((No int) (TID tid)))) ({body}))"


# One-value results, for the unpacking /api/query does (app/scalar.py). One per
# atomic type, plus the undefined one -- the case that has a payload but no
# value -- and a type the backend does not know, which keeps its nested list.
SCALAR_ANSWERS = {
    "query 1 + 55": "(int 56)",
    "query 1.0 / 2.0": "(real 0.5)",
    "query 1 = 1": "(bool TRUE)",
    'query "abc"': '(string "abc")',
    "query 1.0 / 0.0": "(real undefined)",
    "query nonesuch": "(nonesuch 5)",
}


def _fake_native(optimizer: bool = True):
    """A stand-in for the pybind11 module, without a SECONDO server."""
    fake = types.ModuleType("secondo_native")
    # Every command any connection was asked to run, so the table tests can
    # assert on the exact SECONDO commands the bridge generates.
    fake.commands = []
    fake.directives = []
    # One entry per bridge call: whether it asked for the answer to be read
    # and thrown away as it came (see `view:"none"`).
    fake.discards = []

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

        def secondo(self, command: str, want_tree: bool = True,
                    want_text: bool = True, sink=None,
                    discard: bool = False) -> dict:
            fake.discards.append(discard)
            # The real bridge answers with the list as text and as objects,
            # building both from the ListExpr it holds. Parsing the fake's
            # text here is the stand-in for that -- it is what the C++ walk
            # produces, and it keeps every canned answer below plain text.
            #
            # The flags are honoured rather than ignored so that a caller that
            # asks for a form it does not need shows up as a failing assertion
            # here instead of as silent work in production, and so that the
            # streamed path is the one these tests actually take.
            text = self._secondo_text(command)
            return {
                "text": text if want_text else "",
                **answer(parse(text), want_tree, sink),
            }

        def _secondo_text(self, command: str) -> str:
            fake.commands.append(command)
            if command.strip() == "list databases":
                return "(inquiry (databases (BERLINTEST OPT)))"
            if command.strip() == "list objects":
                return OBJECTS
            if command.strip() == "list operators":
                return OPERATORS_INQUIRY
            if command.startswith(("open database", "close database")):
                return "()"
            if command == "query mehringdamm":
                return "(point (9396.0 9871.0))"
            if command in SCALAR_ANSWERS:
                return SCALAR_ANSWERS[command]
            if command == "query ten":
                return TEN
            if command == "query ten feed count":
                return f"(int {len(TEN_TUPLES)})"
            if command.startswith("query ten feed") and command.endswith(" consume"):
                return _page_of_ten(command)
            if command.startswith("query rowno feed"):
                if command.endswith(" count"):
                    return "(int 3)"
                return "((rel (tuple ((RowNo int) (TID tid)))) ((1 11)))"
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
        def secondo_auto(self, command: str, optimizer_addressed: bool = False,
                         want_tree: bool = True, want_text: bool = True,
                         sink=None, discard: bool = False):
            assert optimizer, "secondo_auto must not be used without an optimizer"
            cmd = command.strip()

            # Mirrors Connection::extract: the flags gate the two halves of the
            # *result*, while the plan, the costs and a directive's message are
            # always extracted -- they are small, and they are the answer for
            # the levels that produce them.
            # An SQL answer is (plan result costs), which the reader does not
            # stream -- the pair worth streaming is nested inside it -- so this
            # one always comes back through the split.
            def halves(text: str) -> dict:
                return {
                    "text": text if want_text else "",
                    **answer(parse(text), want_tree, sink, can_stream=False),
                }

            if cmd == "select * from kinos":
                return {
                    "level": 2,
                    **halves(KINOS),
                    "plan": "kinos feed consume",
                    "costs": 12.5,
                    "message": None,
                }
            if cmd.startswith("create table") or cmd.startswith("drop table"):
                # The optimizer carried it out itself while translating.
                return {
                    "level": 2,
                    **halves("()"),
                    "plan": "done",
                    "costs": 0.0,
                    "message": None,
                }
            if cmd == "showOptions":
                return {
                    "level": 3,
                    # A directive's text is the constant "()", not a rendering
                    # of a result, so extract() does not gate it on want_text.
                    "text": "()",
                    **answer([], want_tree, sink, can_stream=False),
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
                # text and tree both, as the bridge sends them.
                **self.secondo(cmd, want_tree, want_text, sink, discard),
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


def test_view_none_reads_the_answer_and_keeps_nothing(client):
    """`view:"none"` asks the bridge to discard as it reads, and every other
    view does not -- they want the answer."""
    client.post("/api/query", json={"command": "open database berlintest"})
    client.fake.discards.clear()
    client.post("/api/query", json={"command": "query ten", "view": "none"})
    assert client.fake.discards == [True]

    for view in ("auto", "table"):
        client.fake.discards.clear()
        client.post("/api/query", json={"command": "query ten", "view": view})
        # `auto` follows up with a `list objects` to decide the page size, so
        # what matters is that no call on this path discards.
        assert client.fake.discards and not any(client.fake.discards), view


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


# --- serving the built frontend from the same server ----------------------
#
# These drive `mount_static` against a temporary directory rather than the real
# frontend/dist, so they hold whether or not the UI has been built.


def test_no_build_mounts_nothing(client, tmp_path):
    """A checkout that has not run `npm run build` is not an error -- in dev the
    Vite server is serving the UI. An empty dist counts as no build too."""
    from fastapi import FastAPI

    from app.main import mount_static

    assert mount_static(FastAPI(), str(tmp_path)) is None


def test_a_build_is_served_at_the_root(client, tmp_path):
    from fastapi import FastAPI

    from app.main import mount_static

    (tmp_path / "index.html").write_text("<title>SECONDO Web UI</title>")
    (tmp_path / "assets").mkdir()
    (tmp_path / "assets" / "app.js").write_text("console.log(1)")

    app = FastAPI()
    assert mount_static(app, str(tmp_path)) == tmp_path

    c = TestClient(app)
    # "/" serves index.html (StaticFiles html=True), and hashed assets are
    # reachable under the path Vite writes into that file.
    assert "SECONDO Web UI" in c.get("/").text
    assert c.get("/assets/app.js").status_code == 200
    # No client-side router, so an unknown path is honestly a 404.
    assert c.get("/nope").status_code == 404


def test_the_static_mount_does_not_shadow_the_api(client):
    """The regression that matters: a mount at "/" registered before the API
    routes would swallow every one of them. Starlette matches in registration
    order, so `mount_static` has to stay at the end of the module."""
    from app.main import STATIC_DIR

    assert client.get("/api/health").status_code == 200
    assert client.post("/api/query", json={"command": "query ten"}).status_code == 200
    if STATIC_DIR is not None:  # only when this checkout has a built frontend
        assert "<title>" in client.get("/").text


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

        def secondo(self, command, want_tree: bool = True,
                    want_text: bool = True, sink=None,
                    discard: bool = False):
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


def test_close_database_clears_open(client):
    """Closing from the console must drop the selection, or the catalog keeps
    the database highlighted and its objects listed."""
    client.post("/api/query", json={"command": "open database berlintest"})
    r = client.post("/api/query", json={"command": "close database"})
    assert r.status_code == 200
    assert client.get("/api/databases").json()["open"] is None


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
    # `query ten` is exactly one stored relation, so the table can offer to edit
    # it -- and the rows are that relation's first page rather than the answer's
    # (see test_querying_a_whole_relation_pages_it_instead_of_capping_it).
    assert body["table"]["rows"] == [[n] for n, _ in TEN_TUPLES]
    assert body["relation"] == "ten"


def test_a_derived_result_names_no_relation(client):
    """There is nothing to write a filtered or joined result back to."""
    body = client.post("/api/query", json={"command": "select * from kinos"}).json()
    assert body["table"] is not None  # it is still readable as rows
    assert body["relation"] == "kinos"  # ...and this plan is a bare relation
    body = client.post("/api/query", json={"command": "query mehringdamm"}).json()
    assert body["table"] is None
    assert body["relation"] is None


def test_table_view_asks_for_rows_only(client):
    """"Run as table" wants the rows and nothing else, so the spatial payload is
    not built rather than built and discarded -- for a relation of moving points
    that payload is the bulk of the response."""
    body = client.post(
        "/api/query", json={"command": "select * from kinos", "view": "table"}
    ).json()
    assert body["table"]["rows"] == [["Kino", "(1.0 2.0)"]]
    assert body["geojson"] is None
    assert body["temporal"] is None
    # Everything that is not a render payload is unaffected.
    assert body["plan"] == "kinos feed consume"
    assert body["relation"] == "kinos"

    # The same command without the flag still renders on the map, so the default
    # path is untouched.
    body = client.post("/api/query", json={"command": "select * from kinos"}).json()
    assert body["geojson"]["features"][0]["geometry"]["coordinates"] == [1.0, 2.0]


def test_table_view_of_a_non_relation_has_no_rows(client):
    """A point is not a relation. The UI says so rather than opening nothing."""
    body = client.post(
        "/api/query", json={"command": "query mehringdamm", "view": "table"}
    ).json()
    assert body["table"] is None
    assert body["geojson"] is None


def test_querying_a_whole_relation_pages_it_instead_of_capping_it(client):
    """`query ten` is a stored relation, so the table it opens is that relation's
    first page -- not a capped copy of the answer. The rows a cap would have
    dropped are a Next away, and pressing Edit changes nothing about the view."""
    client.post("/api/query", json={"command": "open database berlintest"})
    client.fake.commands.clear()
    body = client.post("/api/query", json={"command": "query ten"}).json()
    table = body["table"]
    assert body["relation"] == "ten"
    assert table["pageable"] is True
    assert table["truncated"] is False
    assert table["totalRows"] == len(TEN_TUPLES)
    # Read-only until Edit asks for the TIDs, so the column is not in the way.
    assert table["tidIndex"] is None
    assert [c["name"] for c in table["columns"]] == ["No"]
    # And the page is cut out of the answer already in hand: the relation is
    # never read a second time, and its total is len() rather than a count scan.
    # Re-reading it was about a third of the response time for `query Trains`.
    assert client.fake.commands == ["query ten", "list objects"]
    assert table["totalKnown"] is True


def test_a_derived_result_is_still_capped(client, monkeypatch):
    """There is no relation to ask for page two of, so the cap stands -- and the
    payload says so rather than showing a prefix silently."""
    monkeypatch.setattr(table_mod, "MAX_ROWS", 1)
    client.post("/api/query", json={"command": "open database berlintest"})
    body = client.post(
        "/api/query", json={"command": "select * from kinos", "view": "table"}
    ).json()
    # `kinos` is not an object of this database, so nothing can be paged.
    assert body["table"]["pageable"] is False


def test_a_command_that_only_looks_like_a_relation_keeps_its_own_answer(client):
    """`base_relation` guesses a name out of the command text. When the guess is
    not a relation here, the query's own result is what is shown."""
    client.fake.commands.clear()
    body = client.post("/api/query", json={"command": "query mehringdamm"}).json()
    assert body["table"] is None
    # An answer that is not rows settles that without asking the catalog: the
    # round trip could only ever come back "not a relation", and on berlintest
    # `list objects` is ~90 ms -- most of what such a query costs at all.
    assert client.fake.commands == ["query mehringdamm"]


def test_table_load_asks_for_tids(client):
    r = client.post("/api/table/load", json={"relation": "ten", "limit": 2})
    assert r.status_code == 200
    body = r.json()
    # Without `addid` there is no TID and nothing is editable.
    assert body["command"] == "query ten feed addid head[2] consume"
    assert body["table"]["tidIndex"] == 1
    assert body["table"]["relation"] == "ten"
    assert body["table"]["rows"] == [[1, 11], [2, 12]]


def test_table_load_reads_one_page_and_counts_the_rest(client):
    """The page is cut server-side, so the payload says which rows these are and
    how many there are in total -- the two numbers a pager needs."""
    body = client.post(
        "/api/table/load", json={"relation": "ten", "offset": 4, "limit": 3}
    ).json()
    assert body["command"] == (
        "query ten feed addid addcounter[RowNo, 1] filter [ .RowNo > 4 ] "
        "head[3] remove[RowNo] consume"
    )
    table = body["table"]
    assert table["rows"] == [[5, 15], [6, 16], [7, 17]]
    assert (table["offset"], table["limit"]) == (4, 3)
    assert table["totalRows"] == 10 and table["totalKnown"]
    # A page is not a truncation: every row is one request away.
    assert table["pageable"] and not table["truncated"]
    assert "query ten feed count" in client.fake.commands


def test_paging_does_not_recount_unless_asked(client):
    """Counting is a full scan. Stepping pages carries the total the client
    already has rather than paying for it again on every click."""
    client.post("/api/table/load", json={"relation": "ten", "limit": 3})
    client.fake.commands.clear()
    body = client.post(
        "/api/table/load",
        json={"relation": "ten", "offset": 3, "limit": 3, "want_total": False},
    ).json()
    assert "query ten feed count" not in client.fake.commands
    # Without a count the total is only what has been seen -- and says so.
    assert body["table"]["totalKnown"] is False


def test_a_page_never_exceeds_the_row_cap(client):
    """MAX_ROWS stops being a truncation and becomes the ceiling on a page."""
    body = client.post(
        "/api/table/load", json={"relation": "ten", "limit": 10_000}
    ).json()
    assert f"head[{table_mod.MAX_ROWS}]" in body["command"]
    assert body["table"]["limit"] == table_mod.MAX_ROWS


@pytest.mark.parametrize("page", [{"offset": -1}, {"limit": 0}])
def test_table_load_rejects_a_nonsensical_page(client, page):
    r = client.post("/api/table/load", json={"relation": "ten", **page})
    assert r.status_code == 400


def test_the_counter_attribute_avoids_the_relations_own_names(client):
    """`addcounter` appends its attribute to the tuple, so a relation with a
    RowNo of its own would otherwise make the load fail on a duplicate name."""
    body = client.post(
        "/api/table/load", json={"relation": "rowno", "offset": 1, "limit": 1}
    ).json()
    assert "addcounter[RowNo2, 1]" in body["command"]


def test_operators_lists_what_the_server_can_do(client):
    body = client.get("/api/operators").json()
    names = [o["name"] for o in body["operators"]]
    # Complete: an operator is offered because the server has it, not because
    # someone remembered to add it to a list in the frontend.
    assert "createsuffixtree" in names
    # An overloaded operator is registered once per algebra; the menu shows one.
    assert names.count("feed") == 1
    # `+` is a real operator, but the editor's token regex can never type it.
    assert "+" not in names
    assert names == sorted(names, key=str.lower)
    syntax = {o["name"]: o["syntax"] for o in body["operators"]}
    assert syntax["createsuffixtree"] == "createsuffixtree( _ )"
    # A specification without the field loses the field, not the entry.
    assert syntax["nosyntax"] == ""


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


# --- uploading a file for an import operator to read ------------------------


GPX = b"""<?xml version="1.0"?>
<gpx version="1.1"><trk><trkseg>
<trkpt lat="52.5" lon="13.4"><time>2026-07-26T08:00:00Z</time></trkpt>
</trkseg></trk></gpx>"""


def _upload(client, name: str, body: bytes = GPX):
    return client.post(
        "/api/upload",
        params={"filename": name},
        content=body,
        headers={"Content-Type": "application/gpx+xml"},
    )


def test_upload_stores_the_file_and_returns_a_readable_path(client, tmp_path):
    r = _upload(client, "Wanderung.gpx")
    assert r.status_code == 200
    body = r.json()
    path = Path(body["path"])
    assert path.is_file()
    assert path.read_bytes() == GPX
    assert body["size"] == len(GPX)
    assert path.suffix == ".gpx"
    path.unlink()


def test_upload_refuses_anything_but_gpx(client):
    r = _upload(client, "trip.csv")
    assert r.status_code == 400
    assert ".gpx" in r.json()["detail"]


def test_upload_refuses_an_empty_body(client):
    r = _upload(client, "empty.gpx", b"")
    assert r.status_code == 400


def test_upload_refuses_a_file_over_the_limit(client, monkeypatch):
    monkeypatch.setattr(settings, "max_upload_bytes", 10)
    r = _upload(client, "big.gpx")
    assert r.status_code == 413
    assert "limit" in r.json()["detail"]


def test_an_upload_name_can_neither_escape_the_directory_nor_a_text_literal(client):
    """The stored path is pasted into a SECONDO text literal ('...') by the
    caller, so a quote in the name would end the literal, and a directory in it
    would put the file wherever the uploader liked."""
    r = _upload(client, "../../etc/pass'wd.gpx")
    assert r.status_code == 200
    path = Path(r.json()["path"])
    assert path.parent == Path(tempfile.gettempdir())
    assert "'" not in str(path) and ".." not in path.name
    path.unlink()


def test_two_uploads_of_the_same_name_do_not_collide(client):
    a, b = _upload(client, "trip.gpx"), _upload(client, "trip.gpx")
    pa, pb = Path(a.json()["path"]), Path(b.json()["path"])
    assert pa != pb and pa.is_file() and pb.is_file()
    pa.unlink()
    pb.unlink()


def test_closing_a_session_removes_what_it_uploaded(client):
    path = Path(_upload(client, "trip.gpx").json()["path"])
    assert path.is_file()
    assert client.post("/api/close").status_code == 200
    assert not path.exists()


def test_a_command_run_for_its_effect_ships_no_payload(client):
    """`view: "none"` is the GPX import's: it runs `let x = ... consume`, whose
    answer is the whole created object and is never rendered."""
    r = client.post("/api/query", json={"command": "query ten", "view": "none"})
    assert r.status_code == 200
    assert r.json()["text"] == ""
    assert r.json()["table"] is None and r.json()["geojson"] is None


def test_a_one_value_result_is_unpacked(client):
    """`query 1 + 55` answers `(int 56)`; the console shows the 56. The nested
    list is still there -- the unpacking is an addition, not a replacement."""
    body = client.post("/api/query", json={"command": "query 1 + 55"}).json()
    assert body["scalar"] == {"type": "int", "value": 56}
    assert body["text"] == "(int 56)"


@pytest.mark.parametrize(
    "command,expected",
    [
        ("query 1 + 55", {"type": "int", "value": 56}),
        ("query 1.0 / 2.0", {"type": "real", "value": 0.5}),
        ("query 1 = 1", {"type": "bool", "value": True}),
        ('query "abc"', {"type": "string", "value": "abc"}),
        # Undefined: the type is known, the value is not. Both halves are said.
        ("query 1.0 / 0.0", {"type": "real", "value": None}),
        # Not an atomic type, so not unpacked -- the label carries the meaning.
        ("query nonesuch", None),
        # A value of several parts, and a relation: neither reads without its list.
        ("query mehringdamm", None),
        ("query ten", None),
    ],
)
def test_what_is_and_is_not_unpacked(client, command, expected):
    client.post("/api/query", json={"command": "open database berlintest"})
    body = client.post("/api/query", json={"command": command}).json()
    assert body["scalar"] == expected


def test_a_directive_has_no_value_to_unpack(client):
    body = client.post("/api/query", json={"command": "showOptions"}).json()
    assert body["scalar"] is None


def test_view_none_unpacks_nothing(client):
    """Nothing was read, so there is nothing to unpack -- and asking would be
    the walk `view:"none"` exists to avoid."""
    body = client.post(
        "/api/query", json={"command": "query 1 + 55", "view": "none"}
    ).json()
    assert body["scalar"] is None
    assert body["text"] == ""
