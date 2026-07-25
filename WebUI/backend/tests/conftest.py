"""Shared test setup.

Every test here runs against a *fake* SECONDO connection, so there is nothing
to configure -- but the bridge refuses to open a session when no
SecondoConfig.ini is configured (`app.config.require_config`), which is what
production wants and what these tests are not about. Point it at a real file
for the duration; the tests that are about the guard set it themselves.
"""
from __future__ import annotations

import pytest

from app.config import settings


@pytest.fixture(autouse=True)
def _configured(tmp_path, monkeypatch):
    ini = tmp_path / "SecondoConfig.ini"
    ini.write_text("[Environment]\n")
    monkeypatch.setattr(settings, "secondo_config", str(ini))
