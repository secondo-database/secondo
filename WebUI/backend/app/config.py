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


class Settings:
    # SECONDO server the bridge connects to on behalf of browser sessions.
    secondo_host: str = os.environ.get("SECONDO_HOST", "127.0.0.1")
    secondo_port: str = os.environ.get("SECONDO_PORT", "1234")
    secondo_user: str = os.environ.get("SECONDO_USER", "")
    secondo_passwd: str = os.environ.get("SECONDO_PASSWD", "")

    # Path to a SecondoConfig.ini (the client reads a few runtime flags from it).
    secondo_config: str = os.environ.get(
        "SECONDO_CONFIG",
        str(Path(os.environ.get("SECONDO_BUILD_DIR", "")) / "bin" / "SecondoConfig.ini"),
    )

    # CORS origin for the Vite dev server.
    cors_origin: str = os.environ.get("WEBUI_CORS_ORIGIN", "http://localhost:5173")

    # Each session holds a SECONDO connection (the server forks a process per
    # connection), so idle sessions are closed to avoid leaking them.
    session_idle_timeout: float = float(
        os.environ.get("SECONDO_SESSION_IDLE_TIMEOUT", "1800")  # 30 min
    )
    session_reap_interval: float = float(
        os.environ.get("SECONDO_SESSION_REAP_INTERVAL", "60")
    )


settings = Settings()
