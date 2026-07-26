"""FastAPI bridge between the web UI and a SECONDO server.

Milestone 1 (pipeline spike): connect, run a command, return the result nested
list as text. GeoJSON conversion is layered on top in Milestone 2 (the response
already carries a `geojson` field, currently null).
"""
from __future__ import annotations

import asyncio
import contextlib
import logging
import re
from collections.abc import AsyncIterator
from contextlib import asynccontextmanager

from fastapi import Cookie, FastAPI, HTTPException, Request, Response
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel

from . import table as table_mod
from . import updates as updates_mod
from .catalog import object_type_expr, parse_objects
from .config import config_error, settings
from .convert import convert
from .nlparser import parse
from .nlwriter import InvalidValue
from .session import Session, manager

logger = logging.getLogger("secondo.webui")

SESSION_COOKIE = "secondo_sid"


async def _reaper() -> None:
    """Periodically close idle sessions so their SECONDO connections (and the
    per-connection server processes) are reclaimed."""
    while True:
        await asyncio.sleep(settings.session_reap_interval)
        try:
            n = await manager.reap(settings.session_idle_timeout)
            if n:
                logger.info("Closed %d idle SECONDO session(s)", n)
        except Exception:  # noqa: BLE001 - the reaper must never die
            logger.exception("Session reaper failed")


@asynccontextmanager
async def lifespan(_app: FastAPI) -> AsyncIterator[None]:
    # Say up front which config the client will use and whether it is usable:
    # the wrong one is the difference between a working bridge and one whose
    # every endpoint hangs, and it is invisible otherwise.
    problem = config_error()
    if problem:
        logger.error("%s Sessions will be refused.", problem)
    else:
        logger.info(
            "SECONDO %s:%s, config %s",
            settings.secondo_host,
            settings.secondo_port,
            settings.secondo_config,
        )
    task = asyncio.create_task(_reaper())
    try:
        yield
    finally:
        task.cancel()
        with contextlib.suppress(asyncio.CancelledError):
            await task
        with contextlib.suppress(Exception):
            await manager.close_all()


app = FastAPI(title="SECONDO Web UI bridge", version="0.1.0", lifespan=lifespan)


@app.exception_handler(Exception)
async def unhandled_exception(_request: Request, exc: Exception) -> JSONResponse:
    """Guarantee a JSON body for *any* failure so the browser never receives a
    plain-text 500 (which would fail `response.json()`)."""
    logger.exception("Unhandled error")
    return JSONResponse(
        status_code=500, content={"detail": f"{type(exc).__name__}: {exc}"}
    )
app.add_middleware(
    CORSMiddleware,
    allow_origins=[settings.cors_origin],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class QueryRequest(BaseModel):
    command: str


class QueryResponse(BaseModel):
    text: str  # result nested list as text
    geojson: dict | None = None  # static spatial features (Milestone 2)
    temporal: dict | None = None  # moving-object trips + time domain (Milestone 3)
    table: dict | None = None  # relation rows + schema (Milestone 9)
    # The stored relation this result came from, when it can be named without
    # guessing. Only a hint for offering the table's Edit button -- the load
    # endpoint validates it against the catalog before it runs anything.
    relation: str | None = None
    # Optimizer fields - all absent for an ordinary kernel command.
    level: int | None = None  # command level the server resolved this to
    plan: str | None = None  # the executable plan the optimizer generated
    costs: float | None = None  # its estimated costs
    message: str | None = None  # what an optimizer directive printed
    plan_only: bool = False  # optimized but deliberately not executed
    executed_by_optimizer: bool = False  # a create/drop the optimizer did itself


async def _session_for(response: Response, sid: str | None) -> Session:
    """Return the session for this cookie, creating one on first contact."""
    session = manager.get(sid)
    if session is None:
        try:
            session = await manager.create()
        except RuntimeError as exc:
            # The bridge cannot reach SECONDO at all (no config, no server).
            # 503 rather than 400: nothing about the request is wrong, and it
            # keeps a misconfigured bridge distinguishable from a bad command.
            logger.error("Cannot open a SECONDO session: %s", exc)
            raise HTTPException(status_code=503, detail=str(exc)) from exc
        response.set_cookie(
            SESSION_COOKIE, session.id, httponly=True, samesite="lax"
        )
    return session


@app.get("/api/health")
async def health() -> dict:
    """Liveness only -- it deliberately does not touch SECONDO. It does report
    the config, so a health check that answers while every other endpoint fails
    is not mistaken for a healthy bridge."""
    problem = config_error()
    return {
        "status": "ok" if problem is None else "misconfigured",
        "secondo": f"{settings.secondo_host}:{settings.secondo_port}",
        "config": settings.secondo_config,
        "config_error": problem,
    }


@app.post("/api/query", response_model=QueryResponse)
async def query(
    req: QueryRequest,
    response: Response,
    secondo_sid: str | None = Cookie(default=None),
) -> QueryResponse:
    session = await _session_for(response, secondo_sid)
    try:
        # Whichever language the command is in -- the server classifies it.
        result = await session.execute(req.command)
    except RuntimeError as exc:  # SECONDO error / connection error
        # The server's own message is passed through unchanged. In particular
        # ERR_OPTIMIZER_NOT_AVAILABLE is not translated: SecondoServer uses that
        # one code for four different situations (no optimizer, no database
        # open, the SQL failed to optimize, a directive failed), and only its
        # message says which.
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    # Track the open database so the UI can show it.
    m = re.match(r"\s*open\s+database\s+(\w+)", req.command, re.IGNORECASE)
    if m:
        session.open_db = m.group(1)
    # Best-effort conversion; never let it break a successful query. For SQL the
    # result half is byte-identical to what the plan would produce on its own,
    # so this is the unchanged Milestone 2/3 pipeline.
    geojson = temporal = tabular = None
    try:
        geojson, temporal, tabular = convert(result.text)
    except Exception:  # noqa: BLE001 - conversion must not fail the request
        logger.exception("Result conversion failed for command: %s", req.command)
    return QueryResponse(
        text=result.text,
        geojson=geojson,
        temporal=temporal,
        table=tabular,
        relation=(
            table_mod.base_relation(req.command, result.plan) if tabular else None
        ),
        level=result.level,
        plan=result.plan,
        costs=result.costs,
        message=result.message,
        plan_only=result.plan_only,
        executed_by_optimizer=result.executed_by_optimizer,
    )


@app.get("/api/databases")
async def databases(
    response: Response, secondo_sid: str | None = Cookie(default=None)
) -> dict:
    """The session's state: which databases exist, which one is open, and
    whether this server can run SQL. The catalog polls it anyway, so the
    capability rides along instead of costing a request of its own."""
    session = await _session_for(response, secondo_sid)
    text = await session.run("list databases")
    # (inquiry (databases (NAME1 NAME2 ...)))  -> pull the names out
    names = re.findall(r"\b[A-Z][A-Z0-9_]*\b", text)
    return {
        "databases": names,
        "open": session.open_db,
        "optimizer": session.optimizer,
    }


@app.get("/api/objects")
async def objects(
    response: Response, secondo_sid: str | None = Cookie(default=None)
) -> dict:
    """List objects in the currently open database (name/type/kind)."""
    session = await _session_for(response, secondo_sid)
    try:
        text = await session.run("list objects")
    except RuntimeError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    return {"objects": parse_objects(text), "open": session.open_db}


# --- the table view: reading a relation with TIDs, and writing it back -----
#
# Editing a relation is the Java GUI's UpdateViewer2 (Javagui/viewer/update2/),
# ported operator for operator in app/updates.py. Two rules from there matter
# here: a tuple is addressed by the TID that `addid` appends, so only a relation
# loaded through /api/table/load is editable at all; and every command runs at
# kernel level (`Session.run`), never through the optimizer.

# A SECONDO object name. The relation name is concatenated into a command, so it
# is matched against this *and* against the catalog before anything is executed.
_OBJECT_NAME = re.compile(r"\A[A-Za-z]\w*\Z")


class TableLoadRequest(BaseModel):
    relation: str
    filters: list[str] = []
    project: list[str] = []
    sort: list[str] = []


class TableUpdate(BaseModel):
    tid: int
    values: dict[str, str]  # only the changed attributes


class TableInsert(BaseModel):
    values: dict[str, str]  # every non-TID attribute


class TableCommitRequest(BaseModel):
    relation: str
    updates: list[TableUpdate] = []
    deletes: list[int] = []
    inserts: list[TableInsert] = []


async def _relation_schema(session: Session, name: str) -> tuple[str, list[dict]]:
    """Validate a relation name against the open database and read its schema.

    Returns the raw ``list objects`` answer alongside the columns, so the caller
    can find the relation's indexes in it without asking a second time. The
    schema is the *stored* one -- no TID column, which is exactly what an insert
    or update has to supply.
    """
    if not _OBJECT_NAME.match(name or ""):
        raise HTTPException(status_code=400, detail=f"Not a valid object name: {name!r}")
    try:
        objects_text = await session.run("list objects")
    except RuntimeError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    columns = table_mod.columns_of(object_type_expr(objects_text, name))
    if columns is None:
        raise HTTPException(
            status_code=400,
            detail=f"{name} is not a relation in the open database",
        )
    return objects_text, columns


def _count(text: str) -> int | None:
    """The number a ``(query (count …))`` answered with."""
    try:
        tree = parse(text)
    except ValueError:
        return None
    if isinstance(tree, list) and len(tree) >= 2 and isinstance(tree[1], int):
        return tree[1]
    return None


@app.post("/api/table/load")
async def table_load(
    req: TableLoadRequest,
    response: Response,
    secondo_sid: str | None = Cookie(default=None),
) -> dict:
    """Load a stored relation *with* its tuple identifiers, so it can be edited.

    This is `CommandGenerator.generateLoad`: `query <Rel> feed … addid consume`.
    """
    session = await _session_for(response, secondo_sid)
    await _relation_schema(session, req.relation)
    command = updates_mod.load_command(
        req.relation, req.filters, req.project, req.sort
    )
    try:
        text = await session.run(command)
    except RuntimeError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    payload = table_mod.to_table(text)
    if payload is None:
        raise HTTPException(
            status_code=400, detail=f"{req.relation} did not load as a relation"
        )
    payload["relation"] = req.relation
    return {"table": payload, "command": command}


@app.post("/api/table/commit")
async def table_commit(
    req: TableCommitRequest,
    response: Response,
    secondo_sid: str | None = Cookie(default=None),
) -> dict:
    """Apply a batch of edits to a stored relation.

    Order matters, because a delete renumbers the tuple identifiers after it:
    updates first (so they still address the tuples the user saw), then deletes
    highest-identifier-first, then inserts (whose new identifiers the caller
    gets back). The whole batch is bracketed in a transaction -- as the *legacy*
    `viewer/update/ActionController` does; a Save in a browser is one gesture and
    a half-applied batch is worse than none -- and every affected index is
    maintained alongside the data.
    """
    session = await _session_for(response, secondo_sid)
    objects_text, columns = await _relation_schema(session, req.relation)
    indexes = updates_mod.find_indexes(
        objects_text, req.relation, [c["name"] for c in columns]
    )

    # Build every command up front: a value that will not convert must fail
    # before anything has been written, not halfway through the batch.
    try:
        commands: list[tuple[str, str]] = [
            ("update", updates_mod.update_command(
                req.relation, u.tid, columns, u.values, indexes))
            for u in req.updates
        ]
        # Highest tuple identifier first. A `deletebyid` shifts the identifiers
        # of the tuples after it, so deleting 2, 5, 8 in that order actually
        # removes the 2nd, 6th and 10th tuple. Descending order leaves every
        # not-yet-deleted target's identifier untouched. (Verified against a
        # live server; `CommandGenerator.generateDelete` iterates its set in
        # arbitrary order and has the same hazard.)
        commands += [
            ("delete", updates_mod.delete_command(req.relation, tid, indexes))
            for tid in sorted(set(req.deletes), reverse=True)
        ]
        commands += [
            ("insert", updates_mod.insert_command(
                req.relation, columns, i.values, indexes))
            for i in req.inserts
        ]
    except InvalidValue as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc

    if not commands:
        return {"applied": 0, "inserted": [], "transactional": False}

    # A server that will not open a transaction here still has to be usable, so
    # the bracket is best-effort and the answer says which it was.
    transactional = True
    try:
        await session.run("begin transaction")
    except RuntimeError:
        logger.info("No transaction bracket available; applying commands directly")
        transactional = False

    applied = 0
    inserted: list[int] = []
    try:
        for kind, command in commands:
            text = await session.run(command)
            if kind == "insert":
                new = table_mod.to_table(text)
                tid_index = (new or {}).get("tidIndex")
                if new and tid_index is not None and new["rows"]:
                    inserted.append(new["rows"][0][tid_index])
            else:
                # UpdateViewerController:598 -- the operator reports how many
                # tuples it touched. Zero means somebody else got there first.
                if _count(text) != 1:
                    raise RuntimeError(
                        f"The tuple this {kind} addresses no longer exists "
                        "(deleted by another session?)"
                    )
            applied += 1
    except RuntimeError as exc:
        if transactional:
            with contextlib.suppress(Exception):
                await session.run("abort transaction")
            raise HTTPException(
                status_code=400, detail=f"{exc} - no change was applied."
            ) from exc
        raise HTTPException(
            status_code=400,
            detail=f"{exc} - {applied} of {len(commands)} changes were applied.",
        ) from exc

    if transactional:
        try:
            await session.run("commit transaction")
        except RuntimeError as exc:
            raise HTTPException(status_code=400, detail=str(exc)) from exc

    # The optimizer keeps its own cardinality estimates; tell it they moved, as
    # RelViewer does. Never the user's problem if it fails.
    if session.optimizer:
        try:
            await session.directive(f"updateRel({req.relation})")
        except Exception:  # noqa: BLE001 - best effort
            logger.exception("updateRel failed after editing %s", req.relation)

    return {"applied": applied, "inserted": inserted, "transactional": transactional}


@app.post("/api/close")
async def close(secondo_sid: str | None = Cookie(default=None)) -> dict:
    if secondo_sid:
        await manager.close(secondo_sid)
    return {"closed": True}
