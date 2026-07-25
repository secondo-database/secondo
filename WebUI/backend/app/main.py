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

from .catalog import parse_objects
from .config import config_error, settings
from .convert import convert
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
    geojson = temporal = None
    try:
        geojson, temporal = convert(result.text)
    except Exception:  # noqa: BLE001 - conversion must not fail the request
        logger.exception("Result conversion failed for command: %s", req.command)
    return QueryResponse(
        text=result.text,
        geojson=geojson,
        temporal=temporal,
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


@app.post("/api/close")
async def close(secondo_sid: str | None = Cookie(default=None)) -> dict:
    if secondo_sid:
        await manager.close(secondo_sid)
    return {"closed": True}
