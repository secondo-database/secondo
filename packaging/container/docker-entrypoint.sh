#!/bin/bash
#
# PID 1 of the SECONDO container: restore the demo databases, start the
# SecondoMonitor and the WebUI bridge, and supervise both.
#
# Environment (all set to a default in the image):
#
#   SECONDO_PARAM_SecondoHome   database directory (a volume by default)
#   SECONDO_SKIP_DB_INIT        "true" skips the restore of the demo databases
#   WEBUI_HOST / WEBUI_PORT     listen address of the WebUI (0.0.0.0:8000)
#
# Anything passed to `docker run` after the image name is executed instead of
# this, so `docker run --rm -it secondo bash` still works.
#####################################################################

set -euo pipefail

if [ "$#" -gt 0 ]; then
   exec "$@"
fi

db_home=${SECONDO_PARAM_SecondoHome:-/var/lib/secondo/databases}
webui_host=${WEBUI_HOST:-0.0.0.0}
webui_port=${WEBUI_PORT:-8000}

# The databases shipped as nested-list dumps in bin/. Add a name here to
# preload another one.
demo_databases=(berlintest opt)

log() { echo "[entrypoint] $*"; }

# --- 1. The demo databases -------------------------------------------------
#
# Before the monitor, not after: the restore runs in the single-user kernel,
# which cannot share the database directory with a running monitor. This is
# also the order of the CI workflow.
restore_databases() {
   # cd bin: the kernel writes its temporary files relative to the working
   # directory, and the dumps are here.
   cd "$SECONDO_BUILD_DIR/bin"

   local db upper log_file
   for db in "${demo_databases[@]}"; do
      upper=${db^^}
      if [ -d "$db_home/$upper" ]; then
         log "database $upper is already present, not restoring it"
         continue
      fi

      log "restoring database $upper ..."
      log_file=$(mktemp)
      printf 'create database %s;\nrestore database %s from '\''%s/bin/%s'\'';\nclose database;\nquit;\n' \
         "$db" "$db" "$SECONDO_BUILD_DIR" "$db" \
         | ./SecondoTTYBDB > "$log_file" 2>&1 || true

      # The TTY exits 0 even when a command failed, so the database directory
      # is the only trustworthy evidence that the restore worked.
      if [ ! -d "$db_home/$upper" ]; then
         log "restoring $upper failed:"
         cat "$log_file" >&2
         rm -f "$log_file"
         return 1
      fi
      rm -f "$log_file"
      log "restored database $upper"
   done
}

mkdir -p "$db_home"

if [ "${SECONDO_SKIP_DB_INIT:-false}" = "true" ]; then
   log "SECONDO_SKIP_DB_INIT=true -- not preloading ${demo_databases[*]}"
else
   restore_databases
fi

# --- 2. The server ---------------------------------------------------------
#
# cwd must be bin/: the monitor spawns the listener, the registrar and the
# checkpoint service by the unqualified names from SecondoConfig.ini, and
# ProcessFactory execs them without searching PATH.
cd "$SECONDO_BUILD_DIR/bin"
log "starting SecondoMonitor ..."
SecondoMonitor -s &
monitor_pid=$!

for _ in $(seq 1 60); do
   if (exec 3<>/dev/tcp/127.0.0.1/1234) 2>/dev/null; then
      exec 3>&- 3<&-
      break
   fi
   if ! kill -0 "$monitor_pid" 2>/dev/null; then
      log "SecondoMonitor died during startup"
      exit 1
   fi
   sleep 1
done

if ! (exec 3<>/dev/tcp/127.0.0.1/1234) 2>/dev/null; then
   log "SecondoMonitor is not listening on port 1234"
   kill "$monitor_pid" 2>/dev/null || true
   exit 1
fi
exec 3>&- 3<&-
log "SecondoMonitor is listening on port 1234"

# --- 3. The WebUI ----------------------------------------------------------
#
# One worker, working directory backend/ -- the same shape as the `serve` rule
# of WebUI/Makefile: sessions live in an in-process dict, and the persistent
# nested list writes its temporary files into the working directory.
cd "$SECONDO_BUILD_DIR/WebUI/backend"
log "starting the WebUI bridge on $webui_host:$webui_port ..."
.venv/bin/uvicorn app.main:app --host "$webui_host" --port "$webui_port" &
webui_pid=$!

# --- 4. Supervision --------------------------------------------------------
#
# Stop the bridge first, then the monitor: a monitor that gets its SIGTERM
# shuts the storage manager down cleanly and checkpoints Berkeley DB, so the
# next start does not have to recover.
shutdown() {
   local code=${1:-0}
   trap - TERM INT
   log "shutting down ..."
   kill "$webui_pid" 2>/dev/null || true
   wait "$webui_pid" 2>/dev/null || true
   kill "$monitor_pid" 2>/dev/null || true
   wait "$monitor_pid" 2>/dev/null || true
   exit "$code"
}
trap 'shutdown 0' TERM INT

# If either process exits on its own, take the container down with it rather
# than leaving half a SECONDO running behind a healthy-looking container. That
# is a failure, not a stop, so it exits non-zero.
wait -n "$monitor_pid" "$webui_pid" || true
log "a service exited on its own -- stopping the container"
shutdown 1
