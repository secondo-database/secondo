# Build and run the SECONDO WebUI.
#
# Production is one server on one port: the FastAPI bridge serves /api/* and the
# built frontend together (see mount_static in backend/app/main.py), so there is
# no Vite process and no cross-origin proxy.
#
#   source ~/.secondorc          # required, as for every other build here
#   make venv native             # once
#   make prod                    # build the UI, then serve everything
#
# Development is still two servers -- `uvicorn` here and `npm run dev` in
# frontend/ -- and needs none of this. See README.md.
#
# Override the listen address on the command line:
#   make prod HOST=127.0.0.1 PORT=9000

HOST ?= 0.0.0.0
PORT ?= 8000

VENV = backend/.venv

.PHONY: prod build native venv serve clean check-env

prod: venv native build serve

build:
	cd frontend && npm ci && npm run build

venv:
	cd backend && python3 -m venv .venv && .venv/bin/pip install -r requirements.txt

# backend/native/Makefile resolves pybind11 through plain `python`, so the venv
# has to be on PATH or it fails with "No module named pybind11".
native:
	PATH="$(CURDIR)/$(VENV)/bin:$$PATH" $(MAKE) -C backend/native

# `cd backend` rather than --app-dir: the persistent nested list writes its temp
# files into the working directory, and backend/temp_nested_list/ is where the
# rest of the tooling (and .gitignore) expects them. Calling the venv's uvicorn
# directly avoids needing `activate`, which would not survive the recipe line.
#
# One worker, always. Sessions live in an in-process dict and each holds a
# SECONDO connection, so a second worker would not recognise the first one's
# session cookie.
serve: check-env
	cd backend && .venv/bin/uvicorn app.main:app --host $(HOST) --port $(PORT)

# Fail with the reason now rather than with a 503 per request later.
check-env:
	@if [ -z "$$SECONDO_CONFIG" ] && [ -z "$$SECONDO_BUILD_DIR" ]; then \
	  echo "Neither SECONDO_CONFIG nor SECONDO_BUILD_DIR is set -- source ~/.secondorc first."; \
	  exit 1; fi
	@ls backend/native/secondo_native*.so >/dev/null 2>&1 || \
	  { echo "The native module is not built -- run: make native"; exit 1; }
	@test -x $(VENV)/bin/uvicorn || \
	  { echo "No backend virtualenv -- run: make venv"; exit 1; }

clean:
	rm -rf frontend/dist
	rm -rf $(VENV)
	$(MAKE) -C backend/native clean
