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

The workloads, selected with `--mode` (default `both` = `connect` + `parse`):

- `connect` opens and closes connections from several threads while running
  commands on them. This is the path with the process-wide state: runtime flags,
  the message centre's handler list, the connection counter. Needs a server.
- `parse` builds and re-reads nested lists, each thread on its own
  `NestedList`. This is the path through `NLParser`, and needs no server.
- `shared` puts every thread on *one* `NestedList`, which is what the running
  system does -- `Distributed2` copies each worker's result into the global
  list and reads it back, and every `Out` function an operator runs in a worker
  thread uses the global `nl` too. Needs no server.
- `handover` passes one list from thread to thread, each joined before the next
  starts. That keeps the threading contract, so it must run clean.
- `contract` breaks the contract on purpose and must abort. Needs a build with
  `-DNL_CHECK_CONCURRENCY`; see below.

Use `--list-mem 1` with the last three: it sizes the tables to a single page,
so `BigArray` runs out of room within the first rounds and has to `ftruncate`
and `mremap` while other threads read through the mapping it is moving. At the
default sizes the whole workload fits in the mapping made at construction and
never grows it. The run prints how far the node table grew and fails if it did
not outgrow that first mapping.

`--expect-binary` / `--expect-text` assert which list transfer mode the server
announced. The mode is negotiated, so the *server* decides it: to cover the
textual path, point `--port` at a monitor whose `SecondoConfig.ini` has
`Server:BinaryTransfer` switched off. Asserting it stops a mis-configured server
from quietly testing the same path twice.

## ThreadSanitizer

There is no sanitizer build here any more. This directory used to recompile a
hand-listed mirror of the client library's sources with `-fsanitize=thread`,
which left the `QueryProcessor`, every algebra and the four server daemons
uninstrumented -- and instrumenting a subset is also what produces phantom
reports, because TSan cannot see the synchronization in objects it was not
compiled into.

Instead the whole tree is built with it, into a clean tree:

    make SECONDO_TSAN=1 compileJava=false
    make -C apis/api_cpp/cs SECONDO_TSAN=1
    cd Tests/csloadtest && make && ./app --mode shared --threads 4 --rounds 2 --list-mem 1

`./app` is then itself instrumented, and so is the server it connects to. See
the `SECONDO_TSAN` block in `makefile.env`; objects built with and without it
must never be mixed, and it forces `-O1` where the tree otherwise builds
without any `-O`, so every object changes. The `build-linux-tsan` CI job does
exactly this, with `TSAN_OPTIONS` and `CM-Scripts/tsan.supp`.

Instrument SECONDO rather than the Python bridge in `WebUI/backend`: CPython is
not instrumented and would bury real findings in noise.

## The threading contract checker

TSan reports accesses that race but do not overlap in time. The other half of
`NestedList`'s contract -- "operations on the same list need the caller's own
lock" -- is about accesses that *do* overlap, and it is the caller's to keep,
so it cannot be checked by construction:

    make app-check       # ./app-check
    ./app-check --mode shared   --threads 8 --rounds 5 --list-mem 1   # must pass
    ./app-check --mode handover --threads 4 --rounds 5 --list-mem 1   # must pass
    ./app-check --mode contract --threads 4 --rounds 1 --list-mem 1   # must abort

This is not a sanitizer build. It recompiles `NestedList.cpp` and this test with
`-DNL_CHECK_CONCURRENCY` over the stock `libsecondo.a` -- only those two, since
`NodeAccessGuard` is a free class rather than a `NestedList` member, so the
class layout does not change, and the `NL_WRITING_NODE` / `NL_READING_NODE`
macros appear in no other translation unit. The objects are named ahead of
`-lsecondo`, so the archive's own copy of `NestedList.o` is never pulled in.

Assert both directions. A checker that never fires and one that always fires
look alike from one side only, and what the `contract` run has to produce is
the report `NestedList threading contract was broken` -- not merely a non-zero
exit, which a crash or a binary built without the define gives just as well.

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
