"""FastAPI bridge between the web UI and a SECONDO server.

Milestone 1 (pipeline spike): connect, run a command, return the result nested
list as text. GeoJSON conversion is layered on top in Milestone 2 (the response
already carries a `geojson` field, currently null).
"""
from __future__ import annotations

import asyncio
import contextlib
import logging
import os
import re
import tempfile
from collections.abc import AsyncIterator
from contextlib import asynccontextmanager
from pathlib import Path
from typing import Literal

from fastapi import Cookie, FastAPI, HTTPException, Request, Response
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

from . import table as table_mod
from . import updates as updates_mod
from .catalog import object_type_expr, parse_objects, parse_operators
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


def _configure_logging() -> None:
    """Make this app's log actually come out.

    `logging.getLogger("secondo.webui")` has no handler of its own, and uvicorn
    configures only its own loggers -- it leaves the root logger at WARNING with
    no handler. So every `logger.info` here (the config in force, the session
    reaper, which mode this process is in) went nowhere, and only warnings and
    errors surfaced, through logging's last-resort handler. Borrowing uvicorn's
    handler keeps one format and one stream for the whole process; run outside
    uvicorn (a test, a script) there is nothing to borrow, so fall back to a
    plain stderr handler.
    """
    if logger.handlers:
        return
    # "uvicorn" is the one carrying the handler in uvicorn's default config;
    # "uvicorn.error" is configured with a level only and propagates to it.
    parent = logging.getLogger("uvicorn")
    if parent.handlers:
        logger.handlers = parent.handlers
        logger.setLevel(parent.level or logging.INFO)
    else:
        logging.basicConfig(level=logging.INFO)
        logger.setLevel(logging.INFO)


@asynccontextmanager
async def lifespan(_app: FastAPI) -> AsyncIterator[None]:
    _configure_logging()
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
    # Which of the two shapes this process is: the whole WebUI, or the API half
    # of a dev setup where Vite serves the rest. A blank page is otherwise
    # unexplained. (STATIC_DIR is assigned at the end of this module, which has
    # finished importing long before any of this runs.)
    if STATIC_DIR:
        logger.info("Serving the WebUI from %s", STATIC_DIR)
    else:
        logger.info(
            "No frontend build at %s -- serving the API only. Run "
            "`make -C WebUI build`, or use the Vite dev server.",
            settings.static_dir,
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
    # Which render payloads the caller wants back. "auto" derives whatever the
    # result supports; "table" is the UI's "run as table", and asks for the rows
    # alone -- a relation of moving points is megabytes of trips that would only
    # be discarded, so it is not built rather than built and dropped. "none" is
    # for a command run for its effect: `let x = ... consume` answers with the
    # whole created object, and the GPX import that issues it wants none of it.
    view: Literal["auto", "table", "none"] = "auto"


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
        if req.view == "none":
            pass  # run for the effect; the answer is not going to be rendered
        elif req.view == "table":
            # Asked for rows and nothing else, so only the rows are derived.
            tabular = table_mod.to_table(result.text)
        else:
            geojson, temporal, tabular = convert(result.text)
    except Exception:  # noqa: BLE001 - conversion must not fail the request
        logger.exception("Result conversion failed for command: %s", req.command)
    relation = table_mod.base_relation(req.command, result.plan) if tabular else None
    # `query plz` is a whole stored relation, so it is served as its *first page*
    # rather than as a capped copy of the answer: the rows the cap would have
    # dropped are only ever a Next away, and the table behaves the same before
    # and after pressing Edit. Only a genuinely derived result -- a join, a
    # projection, anything the backend did not write the query for -- is still
    # capped, because there is no relation to ask for page two of.
    if relation:
        tabular = await _page_of(session, relation) or tabular
    return QueryResponse(
        # `let x = <a long track> consume` answers with the whole created
        # object. A caller that asked for no payloads is not going to show it
        # either, so it does not cross the wire.
        text="" if req.view == "none" else result.text,
        geojson=geojson,
        temporal=temporal,
        table=tabular,
        relation=relation,
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


# --- uploading a file for an import operator to read -----------------------
#
# `gpximport` and its kind take a *path* and open it themselves, so a file in
# the browser has to be put on a disk the SECONDO server can read before any
# command can name it. The bridge writes it into its own temp directory, which
# is the server's temp directory too whenever the two share a machine -- the
# default deployment. With a remote SECONDO_HOST the import fails on the first
# command with the server's own "file not found", which says as much.

# What a written file may be called: no directory separators, nothing that
# needs quoting. The path is pasted into a SECONDO text literal ('...'), so an
# apostrophe in a filename would end the literal -- it cannot survive this.
_UNSAFE_IN_NAME = re.compile(r"[^A-Za-z0-9._-]")


def _safe_upload_name(filename: str, suffix: str) -> str:
    """A filename that cannot escape the temp directory or a text literal."""
    # `Path(...).name` drops any directory the browser (or a caller) put in
    # front; the substitution then leaves only characters that need no quoting.
    stem = Path(_UNSAFE_IN_NAME.sub("_", Path(filename).name)).stem
    return (stem[:60] or "upload") + suffix


@app.post("/api/upload")
async def upload(
    request: Request,
    response: Response,
    filename: str,
    secondo_sid: str | None = Cookie(default=None),
) -> dict:
    """Store an uploaded file where an import operator can read it.

    The body is the file itself rather than a multipart part: one file needs
    none of what multipart buys, and this way the bridge does not grow a
    dependency (`python-multipart`) for it.

    The file belongs to the session that uploaded it and is deleted when that
    session closes -- see `Session.uploads`.
    """
    session = await _session_for(response, secondo_sid)
    suffix = Path(filename).suffix.lower()
    if suffix != ".gpx":
        raise HTTPException(
            status_code=400, detail=f"Only .gpx files can be imported, not {filename!r}"
        )
    body = await request.body()
    if not body:
        raise HTTPException(status_code=400, detail="The uploaded file is empty.")
    if len(body) > settings.max_upload_bytes:
        raise HTTPException(
            status_code=413,
            detail=(
                f"The file is {len(body)} bytes; the limit is "
                f"{settings.max_upload_bytes}."
            ),
        )
    safe = _safe_upload_name(filename, suffix)
    # A named temp file rather than the plain name: two sessions importing the
    # same track must not write over each other, and the name is not the
    # user's to choose on the bridge's disk.
    fd, path = tempfile.mkstemp(prefix="secondo-webui-", suffix="-" + safe)
    dest = Path(path)
    try:
        await asyncio.to_thread(dest.write_bytes, body)
    except OSError as exc:
        with contextlib.suppress(OSError):
            dest.unlink()
        raise HTTPException(status_code=500, detail=f"Cannot store the upload: {exc}")
    finally:
        os.close(fd)
    session.uploads.append(dest)
    logger.info("Stored upload %s (%d bytes)", dest, len(body))
    return {"path": str(dest), "filename": safe, "size": len(body)}


@app.get("/api/operators")
async def operators(
    response: Response, secondo_sid: str | None = Cookie(default=None)
) -> dict:
    """Every operator this server has, with its syntax -- the query editor's
    vocabulary.

    A property of the *server*, not of a database: `list operators` reads the
    algebra catalog directly and, unlike `list types`, needs no open database
    (SecondoInterfaceTTY.cpp:1302), so the editor can be complete from the first
    keystroke of a session.
    """
    session = await _session_for(response, secondo_sid)
    try:
        text = await session.run("list operators")
    except RuntimeError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    return {"operators": parse_operators(text)}


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
    # One page of the relation. The page is cut server-side, so a relation of any
    # size can be browsed and edited -- see app.updates.load_command.
    offset: int = 0
    limit: int = table_mod.DEFAULT_PAGE_ROWS
    # Counting is a full scan, so it is asked for only when the answer can have
    # changed: the first load, and any change of filter or sort. Stepping through
    # pages carries the total the client already has.
    want_total: bool = True
    # `addid`, and with it the TID column every edit is addressed by. Off for a
    # table that is only being read, which is one column less to look at.
    tids: bool = True


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


async def _load_page(session: Session, req: TableLoadRequest) -> tuple[dict, str]:
    """One page of a stored relation, as a table payload and the command that
    read it. Raises HTTPException for anything the caller asked for that the
    open database cannot answer."""
    if req.offset < 0 or req.limit < 1:
        raise HTTPException(
            status_code=400, detail="offset must be >= 0 and limit >= 1"
        )
    limit = min(req.limit, table_mod.MAX_ROWS)
    _, columns = await _relation_schema(session, req.relation)

    total: int | None = None
    if req.want_total:
        try:
            total = _count(await session.run(
                updates_mod.count_command(req.relation, req.filters)
            ))
        except RuntimeError as exc:
            raise HTTPException(status_code=400, detail=str(exc)) from exc

    command = updates_mod.load_command(
        req.relation,
        req.filters,
        req.project,
        req.sort,
        offset=req.offset,
        limit=limit,
        tids=req.tids,
        # The counter is checked against the *stored* schema; `project` can only
        # remove attributes, so a name free there is free in the page too.
        counter=updates_mod.counter_name(columns),
    )
    try:
        text = await session.run(command)
    except RuntimeError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    payload = table_mod.from_tree(
        parse(text), offset=req.offset, limit=limit, total=total
    )
    if payload is None:
        raise HTTPException(
            status_code=400, detail=f"{req.relation} did not load as a relation"
        )
    payload["relation"] = req.relation
    return payload, command


async def _page_of(session: Session, relation: str) -> dict | None:
    """The first page of a stored relation, read-only, or None if the name does
    not turn out to be one in the open database.

    `base_relation` only *guesses* a name out of the command text, so this is
    allowed to come back empty; the caller then keeps the payload it already had.
    """
    try:
        payload, _ = await _load_page(
            session, TableLoadRequest(relation=relation, tids=False)
        )
    except HTTPException:
        return None
    except Exception:  # noqa: BLE001 - a bonus payload must not fail the request
        logger.exception("Could not page %s", relation)
        return None
    return payload


@app.post("/api/table/load")
async def table_load(
    req: TableLoadRequest,
    response: Response,
    secondo_sid: str | None = Cookie(default=None),
) -> dict:
    """Load one page of a stored relation *with* its tuple identifiers, so it
    can be edited.

    This is `CommandGenerator.generateLoad`: `query <Rel> feed … addid consume`,
    with the page cut out of the stream before `consume` so that neither the
    server nor the browser ever holds the whole relation.
    """
    session = await _session_for(response, secondo_sid)
    payload, command = await _load_page(session, req)
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


def mount_static(app: FastAPI, directory: str | None = None) -> Path | None:
    """Serve the built frontend from this same app; return what was mounted, or
    None if there is no build to serve.

    This is what makes a deployment one server on one port: the API and the UI
    share an origin, so the session cookie needs no proxy to stay same-site.

    Mounted *last* on purpose. Starlette matches routes in the order they were
    added, so every /api route declared above still wins and this catches what
    is left. A checkout that has not run `npm run build` is not an error -- the
    Vite dev server is serving the UI then -- so this is a no-op rather than a
    failure, and the startup log says which of the two is happening.

    `index.html` rather than the directory decides, so an empty or half-removed
    `dist/` counts as no build instead of mounting a 404 factory.
    """
    d = Path(directory or settings.static_dir)
    if not (d / "index.html").is_file():
        return None
    # html=True is what serves index.html for "/". There is no client-side
    # router, so an unknown path is honestly a 404 and needs no rewrite.
    app.mount("/", StaticFiles(directory=d, html=True), name="webui")
    return d


STATIC_DIR = mount_static(app)
