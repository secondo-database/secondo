"""Runtime configuration for the SECONDO web bridge.

All values come from the environment so nothing (host/port/credentials) is
hard-coded -- unlike the retired WebGui2, which baked in a personal hostname.
"""
from __future__ import annotations

import os
import sys
from pathlib import Path

# The compiled `secondo_native` extension lives in ../native next to this
# package. Make it importable without an install step.
_NATIVE_DIR = Path(__file__).resolve().parent.parent / "native"
if str(_NATIVE_DIR) not in sys.path:
    sys.path.insert(0, str(_NATIVE_DIR))


def _config_path() -> str | None:
    """The SecondoConfig.ini to hand the client, or None if none is configured.

    Deliberately not guessed. A wrong config is worse than no config -- the
    client would come up with the wrong runtime flags -- and the checkout the
    bridge happens to sit in is not necessarily the installation it should
    talk to. Nothing configured means nothing configured; `require_config`
    turns that into an error.
    """
    explicit = os.environ.get("SECONDO_CONFIG")
    if explicit:
        return explicit
    build_dir = os.environ.get("SECONDO_BUILD_DIR")
    if build_dir:
        return str(Path(build_dir) / "bin" / "SecondoConfig.ini")
    return None


class Settings:
    # SECONDO server the bridge connects to on behalf of browser sessions.
    secondo_host: str = os.environ.get("SECONDO_HOST", "127.0.0.1")
    secondo_port: str = os.environ.get("SECONDO_PORT", "1234")
    secondo_user: str = os.environ.get("SECONDO_USER", "")
    secondo_passwd: str = os.environ.get("SECONDO_PASSWD", "")

    # Path to a SecondoConfig.ini, from SECONDO_CONFIG or SECONDO_BUILD_DIR.
    # The client reads its runtime flags from this file; which one is in force
    # decides how the client behaves, so it is required rather than guessed
    # (see `_config_path` and `require_config`).
    secondo_config: str | None = _config_path()

    # CORS origin for the Vite dev server.
    cors_origin: str = os.environ.get("WEBUI_CORS_ORIGIN", "http://localhost:5173")

    # The built frontend (`npm run build` output), served by the bridge itself
    # so a deployment is one server on one port. Unlike `secondo_config` this
    # one *is* guessed: it is a directory inside this checkout, and getting it
    # wrong costs a missing page rather than a client with the wrong flags.
    static_dir: str = os.environ.get(
        "WEBUI_STATIC_DIR",
        str(Path(__file__).resolve().parent.parent.parent / "frontend" / "dist"),
    )

    # Largest file /api/upload accepts. The body is read into memory before it
    # is written, and an unbounded upload on an unauthenticated bridge is a way
    # to fill the disk -- a GPX track of a long hike is a few MB, so this is
    # generous rather than tight.
    max_upload_bytes: int = int(
        os.environ.get("WEBUI_MAX_UPLOAD_BYTES", str(64 * 1024 * 1024))
    )

    # Each session holds a SECONDO connection (the server forks a process per
    # connection), so idle sessions are closed to avoid leaking them.
    session_idle_timeout: float = float(
        os.environ.get("SECONDO_SESSION_IDLE_TIMEOUT", "1800")  # 30 min
    )
    session_reap_interval: float = float(
        os.environ.get("SECONDO_SESSION_REAP_INTERVAL", "60")
    )

    # After a kernel command that changes objects, tell the optimizer to reread
    # the catalog -- it only reloads the schema when the database changes, so a
    # `let x = ...` would otherwise stay invisible to SQL for the rest of the
    # session. The JavaGUI does the same (CommandPanel.updateCatalogIfWanted).
    auto_update_catalog: bool = os.environ.get(
        "SECONDO_AUTO_UPDATE_CATALOG", "true"
    ).strip().lower() not in ("false", "no", "0")


settings = Settings()


SET_IT = (
    "Set SECONDO_CONFIG (or SECONDO_BUILD_DIR) before starting the bridge -- "
    "sourcing ~/.secondorc does both."
)


def config_error() -> str | None:
    """Why no connection can be opened, or None if the config looks usable.

    The bridge does not start a session it cannot configure, and says which
    file it wanted. The reference client refuses in the same situation
    ("Configuration file ... does not exist").

    This check earns its keep: starting `uvicorn` without sourcing
    `~/.secondorc` used to leave the client with no runtime flags while the
    server had `Server:BinaryTransfer` on, and the *first* command then
    deadlocked -- `/api/health` answered while every other endpoint hung
    forever, a dead API that looked like a network or a frontend problem. The
    transfer mode is now settled by the server during the connect handshake
    (see `csp::BINARY_TRANSFER_TAG` in include/CSProtocol.h), so that
    particular hang is gone, but which configuration the client runs with is
    still not something to leave to chance.
    """
    if not settings.secondo_config:
        return f"No SECONDO config file configured. {SET_IT}"
    if not Path(settings.secondo_config).is_file():
        return (
            f"SECONDO config file not found: {settings.secondo_config}. {SET_IT}"
        )
    return None


def require_config() -> None:
    """Raise unless a usable config file is configured. See `config_error`."""
    problem = config_error()
    if problem:
        raise RuntimeError(problem)
