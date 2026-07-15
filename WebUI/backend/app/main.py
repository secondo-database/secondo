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
from .config import settings
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


async def _session_for(response: Response, sid: str | None) -> Session:
    """Return the session for this cookie, creating one on first contact."""
    session = manager.get(sid)
    if session is None:
        session = await manager.create()
        response.set_cookie(
            SESSION_COOKIE, session.id, httponly=True, samesite="lax"
        )
    return session


@app.get("/api/health")
async def health() -> dict:
    return {"status": "ok", "secondo": f"{settings.secondo_host}:{settings.secondo_port}"}


@app.post("/api/query", response_model=QueryResponse)
async def query(
    req: QueryRequest,
    response: Response,
    secondo_sid: str | None = Cookie(default=None),
) -> QueryResponse:
    session = await _session_for(response, secondo_sid)
    try:
        text = await session.run(req.command)
    except RuntimeError as exc:  # SECONDO error / connection error
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    # Track the open database so the UI can show it.
    m = re.match(r"\s*open\s+database\s+(\w+)", req.command, re.IGNORECASE)
    if m:
        session.open_db = m.group(1)
    # Best-effort conversion; never let it break a successful query.
    geojson = temporal = None
    try:
        geojson, temporal = convert(text)
    except Exception:  # noqa: BLE001 - conversion must not fail the request
        logger.exception("Result conversion failed for command: %s", req.command)
    return QueryResponse(text=text, geojson=geojson, temporal=temporal)


@app.get("/api/databases")
async def databases(
    response: Response, secondo_sid: str | None = Cookie(default=None)
) -> dict:
    session = await _session_for(response, secondo_sid)
    text = await session.run("list databases")
    # (inquiry (databases (NAME1 NAME2 ...)))  -> pull the names out
    names = re.findall(r"\b[A-Z][A-Z0-9_]*\b", text)
    return {"databases": names, "open": session.open_db}


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
