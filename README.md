<div align="center">

<img src="https://secondo-database.github.io/images/logo.gif" width="420" alt="SECONDO">

**An extensible database system for non-standard applications —
spatial, spatio-temporal, moving objects, graphs, and whatever you implement next.**

[![Build Status](https://github.com/secondo-database/secondo/actions/workflows/build.yml/badge.svg)](https://github.com/secondo-database/secondo/actions/workflows/build.yml)
[![Format Check](https://github.com/secondo-database/secondo/actions/workflows/format.yml/badge.svg)](https://github.com/secondo-database/secondo/actions/workflows/format.yml)
[![License: GPL v2](https://img.shields.io/badge/license-GPL--2.0-blue.svg)](LICENSE.TXT)

[Website](https://secondo-database.github.io/) ·
[User Manual](https://secondo-database.github.io/files/Documentation/General/SecondoManual.pdf) ·
[Programmer's Guide](Documents/ProgrammersGuide.pdf)

</div>

---

## What is SECONDO?

SECONDO is a *generic* database system frame that can be filled with implementations of
different data models. Most database systems let you add data types; SECONDO additionally
lets you replace the core data model — relational, object-oriented, spatial, spatio-temporal, graph-based, or
something entirely new. It was designed and developed by the
[Database Systems for New Applications](https://web.archive.org/web/20190717163530/http://dna.fernuni-hagen.de/index.html)
group at the [FernUniversität in Hagen](https://www.fernuni-hagen.de/).

Everything is built from **algebra modules**: self-contained bundles of data types and
operators that plug into the kernel through a well-defined interface. Around 190 algebras ship
with the system today, from `RelationAlgebra` and `RTreeAlgebra` to `SpatialAlgebra` (spatial data / GIS), `TemporalAlgebra` (spatio-temporal data / moving objects), `Distributed2` (distributed query processing) and
`RasterAlgebra`.

SECONDO is used to

- **prototype research ideas** in database systems without building a DBMS from scratch,
- **teach** database architecture on a code base that students can actually read, and
- **work with data types** that mainstream systems do not support.

### Architecture

SECONDO consists of three components that can be used together or independently:

| Component | Language | Role |
| --- | --- | --- |
| **Kernel** | C++ | Query processing over the implemented algebras; extensible by algebra modules; uses Berkeley DB as storage manager. |
| **Optimizer** | Prolog | Conjunctive query optimization and an SQL-like query language, translated into executable query plans. |
| **WebUI** | Python + TypeScript/React | Browser-based interface: a FastAPI bridge in front of the kernel, and a React/deck.gl frontend with a map view, layer styling, and timeline animation for moving objects. |

The WebUI can send query plans directly to the kernel, or ask the optimizer to turn an SQL query
into a plan. The optimizer, in turn, queries the kernel for schemas, cardinalities and
selectivities. The kernel runs standalone or as a server (`SecondoMonitor`) that serves several
clients concurrently.

---

## Quick start

Tested continuously on **Ubuntu 22.04 / 24.04 / 26.04** and **macOS 15 / 26** (Intel and Apple
Silicon). Other Linux distributions generally work as well.

On Ubuntu you can install a prebuilt package instead of building from source (replace `24.04`
with your release):

```bash
echo 'deb [trusted=yes] https://secondo-database.github.io/secondo/apt/ubuntu/24.04/ ./' \
  | sudo tee /etc/apt/sources.list.d/secondo.list
sudo apt-get update
sudo apt-get install secondo
/opt/secondo/bin/secondo_installer.sh    # sets up ~/.secondorc and your database directory
```

### 1. Install the prerequisites

All dependencies (compiler, flex, bison, Berkeley DB, Boost,
SWI-Prolog, a JDK, …) come from your package manager. The per-platform package lists are kept in
the **[installation instructions](https://secondo-database.github.io/content_install.html)**;
follow the section for your system before continuing.

### 2. Get the sources and set up the environment

```bash
git clone https://github.com/secondo-database/secondo.git
cd secondo

export SECONDO_BUILD_DIR="$PWD"
export SECONDO_CONFIG="$SECONDO_BUILD_DIR/bin/SecondoConfig.ini"
```

That is all the build needs. `makefile.detect` derives the platform, Berkeley DB, SWI-Prolog
and the JDK from the tools it finds, every time you run `make`. Check what it found with:

```bash
make -f makefile.detect check-env
```

To make the setup permanent, copy `CM-Scripts/secondorc.example` to `~/.secondorc`, set
`SECONDO_BUILD_DIR` in it, and source it from your shell profile.

### 3. Build

```bash
make -j$(nproc)      # macOS: make -j$(sysctl -n hw.ncpu)
```

This builds the kernel, the optimizer and the legacy Java GUI. Useful partial targets — see
`make help` for the full list:

| Target | Builds |
| --- | --- |
| `make TTY` | Kernel and the single-user shell interface only |
| `make optimizer` | `SecondoPL` and the embedded optimizer engine |
| `make java` | The legacy Java GUI (`Javagui`) — superseded by the [WebUI](#6-start-the-webui) below |
| `make runtests` | The automatic test suite |
| `make clean` | All objects, libraries and applications |

Which algebras are compiled is controlled by `makefile.algebras` (created from
`makefile.algebras.sample` on the first build).

### 4. Run your first query

Start the single-user shell and restore the bundled demo database:

```bash
cd bin
./SecondoTTYBDB
```

```
Secondo => restore database berlintest from berlintest;
Secondo => list objects;
```

Now run a query at the *executable level* — a query plan written directly in the algebra
operators:

```
Secondo => query Staedte feed filter[.Bev > 500000] project[SName, Bev] consume;
```

A spatial one — the cinemas located inside the region `thecenter` (the centre of Berlin):

```
Secondo => query Kinos feed filter[.GeoData inside thecenter] project[Name] consume;
```

Commands end with `;` or an empty line, `?` shows the interface commands, and `q` quits.

### 5. Write SQL queries

You can also run queries in SQL via the integrated optimizer:

```sql
Secondo => select * from ten;
Secondo => select * from ten as t where [ t.No > 5 ];
```

You can also just run the optimizer to get the query plan without executing it:

```sql
Secondo => optimizer select * from ten as t where [ t.No > 5 ];

Optimized plan: query ten  feed{t} filter[(.No_t > 5)]{0.4995, 7.5} consume
Estimated costs: 75.0261
```

To get more details about the optimizer, run `helpMe` or `showOptions`.

### 6. Start the WebUI

The WebUI is a browser-based UI containing: a command console, a
`deck.gl`/MapLibre map that renders points, lines, regions and moving objects, and a timeline
that animates them. It is a client, so a server has to be running first:

```bash
cd bin && ./SecondoMonitor -s          # start the database server

cd ../WebUI
source ~/.secondorc
make venv native                       # once: Python virtualenv + the pybind11 bridge
make prod                              # build the frontend, then serve everything on :8000
```

Open `http://localhost:8000`, open `berlintest`, and try `query UBahn` — the underground network
draws on the map as a background layer. `berlintest`'s coordinates are not WGS84, so switching
the projection dropdown (top left) to **BerlinMOD → OSM** is what places it on a real
OpenStreetMap basemap of Berlin:

<img src="https://secondo-database.github.io/images/screen_ubahn_mapped.jpg" alt="SECONDO WebUI showing the Berlin U-Bahn network on an OpenStreetMap basemap">

*Map data © [OpenStreetMap](https://www.openstreetmap.org/copyright) contributors*

See [WebUI/README.md](WebUI/README.md) for the full setup, prerequisites and development mode.

---

## Moving objects

SECONDO was one of the first DBMS able to manage **moving objects**: not just the
current position of something, but its *complete history of movement*, stored as a single value
and queryable like any other attribute.

The key idea are spatio-temporal data types. A **moving point** (`mpoint`) is a function from time
into points — an entire trip of a vehicle is one `mpoint` value. A **moving region** (`mregion`)
is a function from time into regions, e.g. a snowfall area or a forest fire spreading over time.
The `TemporalAlgebra` adds the operations to query them:

| Operation | Signature | Meaning |
| --- | --- | --- |
| `trajectory` | `mpoint → line` | Projection of the movement into the 2D plane. |
| `distance` | `mpoint × point → mreal` | Time-dependent distance — the result is a *moving real*. |
| `passes` | `mpoint × region → bool` | Did the object ever pass through this area? |
| `atinstant` | `mpoint × instant → ipoint` | The position at a given point in time. |
| `inside` | `mpoint × mregion → mbool` | Time-dependent boolean: when was the object inside the moving region? |
| `intersection` | `mpoint × mregion → mpoint` | The part of the movement that lies inside the moving region. |

### Try it

The `berlintest` database restored above contains the movements of 562 underground trains in
Berlin on 2003-11-20, in a relation `Trains(Id: int, Line: int, Up: bool, Trip: mpoint)`:

```
Secondo => query Trains count;                                        # 562
```

Which trains ever passed through the region `tiergarten`?

```
Secondo => query Trains feed filter[.Trip passes tiergarten] count;   # 80
```

The path a single train took, and its distance to the station `mehringdamm` over time — the
latter is an `mreal`, a real number that changes continuously:

```
Secondo => query trajectory(train7);
Secondo => query distance(train7, mehringdamm);
```

This is where the [WebUI](#6-start-the-webui) pays off: the same queries, typed into its console
against `berlintest`, draw and animate directly on the map. `query train7` animates the train
along its route with a play/pause/scrub timeline:

<img src="https://secondo-database.github.io/images/screen_ubahn_moving.jpg" alt="SECONDO WebUI animating train7 along the U-Bahn network on the timeline">

*Map data © [OpenStreetMap](https://www.openstreetmap.org/copyright) contributors*

`query distance(train7, mehringdamm)` plots the same `mreal` alongside it, dropping to zero as
the train passes through the station:

<img src="https://secondo-database.github.io/images/screen_ubahn_mehringdam_mreal.jpg" alt="SECONDO WebUI animating train7 alongside its distance to mehringdamm as an mreal plot">

*Map data © [OpenStreetMap](https://www.openstreetmap.org/copyright) contributors*

The same questions in SQL, via the optimizer:

```prolog
?- sql select count(*) from trains where trip passes mehringdamm.
```

Where were the trains that pass `mehringdamm` at 7:05 on that day?

```prolog
?- let 'seven05 = theInstant(2003,11,20,7,5)'.
?- sql select [id, line, up, val(trip atinstant seven05) as pos]
       from trains where [trip passes mehringdamm, trip present seven05].
```

And which trains ran into the moving snow area `msnow` (60 of them) — the predicate intersects a
moving point with a moving region and asks whether the result is defined at any time at all:

```prolog
?- sql select * from trains where [not(isempty(deftime(intersection(trip, msnow))))].
```

Back at the executable level, `intersection` gives you not only *which* trains, but *which part*
of each trip was inside the snow area:

```
Secondo => query Trains feed filter[not(isempty(deftime(intersection(.Trip, msnow))))]
           projectextend[Id; Insnow: intersection(.Trip, msnow)] consume;
```

The full walk-through — installation, every query above run from the WebUI console, and the
query optimizer in action — is the
[Getting Started guide](https://secondo-database.github.io/content_getting_started.html).

---

## User interfaces at a glance

| Interface | Location | Mode | Notes |
| --- | --- | --- | --- |
| `SecondoTTYBDB` | `bin/` | single user | Textual shell linked directly against the kernel. |
| `SecondoTTYCS` | `bin/` | client | Same shell, but talks to `SecondoMonitor` over TCP/IP. |
| `SecondoPL` | `Optimizer/` | single user | Prolog shell with SQL-like queries and the optimizer. |
| WebUI | `WebUI/` | client (browser) | Map view, layer styling, moving-object animation, SQL console. |
| `TestRunner` | `bin/` | — | Runs `.test` scripts and checks expected results. |

---

## Documentation

| Document | |
| --- | --- |
| **Getting Started** — installation, first queries, moving objects | [Website](https://secondo-database.github.io/content_getting_started.html) |
| **User Manual** — commands, interfaces, optimizer, GUI | [PDF](https://secondo-database.github.io/files/Documentation/General/SecondoManual.pdf) |
| **Programmer's Guide** — writing your own algebra | [PDF](Documents/ProgrammersGuide.pdf) |
| **WebUI** — architecture, setup, milestones | [WebUI/README.md](WebUI/README.md) |
| **Installation Instructions** | [Website](https://secondo-database.github.io/content_install.html) |
| **Algebra and viewer documentation** | [Website](https://secondo-database.github.io/content_docu.html) |
| **Distributed Query Processing in SECONDO** | [PDF](https://secondo-database.github.io/files/Documentation/General/DistributedQueryProcessinginSecondo.pdf) |
| **Wiki, FAQ, build notes** | [GitHub Wiki](https://github.com/secondo-database/secondo/wiki) |


---

## Contributing

Bug reports, questions and pull requests are welcome — please use the
[issue tracker](https://github.com/secondo-database/secondo/issues).

## License

SECONDO is released under the **GNU General Public License, version 2**. See
[LICENSE.TXT](LICENSE.TXT).

