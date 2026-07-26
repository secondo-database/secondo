# csloadtest -- concurrency test for the client library

Drives `libsecondo.a` from several threads at once and checks what comes back.
The C++ client was written assuming one connection per process; this is what
tells us how far that assumption has been removed, rather than arguing about it
from the source.

Every check has an expected answer and the program exits non-zero if one fails,
so it can run unattended.

## Building and running

    make                 # ./app
    ./app --help

    # needs a running SecondoMonitor
    ./app --threads 10 --rounds 10

Host and port default to `SecondoHost`/`SecondoPort` in `Config.ini`, which is
copied from `bin/SecondoConfig.ini` by the makefile, so they cannot drift out of
step with the installation.

Two workloads, selected with `--mode`:

- `connect` opens and closes connections from several threads while running
  commands on them. This is the path with the process-wide state: runtime flags,
  the message centre's handler list, the connection counter. Needs a server.
- `parse` builds and re-reads nested lists, each thread on its own
  `NestedList`. This is the path through `NLParser`, and needs no server.

`--expect-binary` / `--expect-text` assert which list transfer mode the server
announced. The mode is negotiated, so the *server* decides it: to cover the
textual path, point `--port` at a monitor whose `SecondoConfig.ini` has
`Server:BinaryTransfer` switched off. Asserting it stops a mis-configured server
from quietly testing the same path twice.

## ThreadSanitizer

    make tsan            # ./app-tsan
    TSAN_OPTIONS="halt_on_error=0" ./app-tsan --mode connect --threads 4 --rounds 2

The client sources are recompiled with `-fsanitize=thread` into `tsan/` and
archived there, so the objects of the normal build are left alone and the
linker pulls in archive members exactly as the real build does. Everything the
test links is instrumented; mixing instrumented and uninstrumented objects is
what produces phantom reports.

`make tsan-check-sources` fails if the source list has drifted from `CSLIBS` in
`apis/api_cpp/cs/makefile` -- an archive member we forget to instrument would
make the sanitizer quietly blind.

Instrument this harness rather than the Python bridge in `WebUI/backend`:
CPython is not instrumented and would bury real findings in noise.

## Where it stands

Last run, 4 threads x 2 rounds:

- `--mode parse`: no warnings. Expected, and not yet meaningful --
  `NLParser::parse` serializes on a static mutex, so there is nothing to race.
  The number to watch is this one after that mutex is removed.
- `--mode connect`: one race, `SecondoInterfaceCS::initNo++`
  (`ClientServer/SecondoInterfaceCS.cpp:223`), a counter used to name trace log
  files. Everything else on the connect path is either per-connection or
  locked.

The bridge in `WebUI/backend/native` still holds a process-wide lock, so none of
this is reachable from the WebUI today; it is what has to be true before that
lock can go.
