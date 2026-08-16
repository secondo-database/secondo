# SECONDO Web UI

A modern web replacement for the Java Swing **HoeseViewer**: send SECONDO
commands, view results as text and (from Milestone 2 on) on a map, and animate
moving objects on a timeline.

See the design/plan write-up for the full stack rationale and milestone
breakdown. This README covers running what exists today.

## Architecture

```
Browser (React + Vite)
   │  HTTP  /api/*
   ▼
FastAPI bridge (backend/)
   │  in-process call
   ▼
secondo_native  (pybind11 wrapper, backend/native/)
   │  links libsecondo.a
   ▼
SECONDO C++ client  (apis/api_cpp/cs)  ──TCP 1234──►  SecondoMonitor
```

The bridge is mandatory: a browser cannot speak SECONDO's raw TCP nested-list
protocol. Crucially, the **wire protocol is never reimplemented** — the pybind11
module wraps the in-tree, prebuilt reference client (`apis/api_cpp/cs`,
`libsecondo.a`). Only nested-list *text* crosses into Python, where it is turned
into GeoJSON (Milestone 2+).

## Status

**Milestone 1 (pipeline spike) — done & verified end-to-end.**
Connect · `list databases` · `open database` · run a command · get the result
nested list as text, with SECONDO errors surfaced to the UI. A command console
frontend drives it.

**Milestone 2 (static spatial rendering) — done & verified end-to-end.**
The bridge parses the result nested list and converts `point` / `points` /
`line` / `region` / `rect` (and relations carrying such attributes) into
GeoJSON (`app/nlparser.py`, `app/geojson.py`). The frontend renders it with
**deck.gl** in a Cartesian orthographic view, auto-fit to the data bounds, with
hover tooltips for relation attributes. Verified in a headless browser
(`npm run e2e`) drawing point, line and region from berlintest onto the WebGL
canvas.

**Milestone 3 (moving objects + timeline) — done & verified end-to-end.**
The bridge converts `mpoint` (and relations of `mpoint`, e.g. `Trains`) into
deck.gl **TripsLayer** data — per continuous run a `path` of waypoints and
matching `timestamps`, plus a `timeDomain` (`app/temporal.py`). Instant strings
(`2003-11-20-06:03:52.685`) are parsed to POSIX seconds. The frontend animates a
single `currentTime` clock via `requestAnimationFrame` (`useAnimator`, replacing
the HoeseViewer's Swing timer) with a play/pause/scrub/speed timeline; the
moving object's position is interpolated on the GPU. Verified in a headless
browser (`npm run e2e -- animation`): the trail head moves >500px along the
Trains trajectories between t=20% and t=80% of the domain.

**Milestone 4 (layers, styling, selection) — done & verified end-to-end.**
Query results now accumulate as **layers** instead of replacing one another
(`layers/useLayers.ts`). A **layers panel** (`layers/LayersPanel.tsx`) toggles
visibility, reorders draw order, removes layers, and edits each layer's style
(color, opacity, point radius, line width, region fill) via a category editor.
The map renders N styled layers, auto-fits to their combined bounds, animates
all visible moving-object layers on one shared clock, and supports
**click-to-select** a feature with a details panel of its attributes. Verified
in a headless browser (`npm run e2e -- layers`): two layers listed, recolor
updates the swatch, hiding a layer changes what's drawn, reordering swaps the
top layer, and clicking a region shows its `Name`.

**Milestone 5 (catalog, geographic basemap, export) — done & verified end-to-end.**
- **DB & object browser** (`catalog/Catalog.tsx`, backend `/api/objects` +
  `app/catalog.py`): pick a database to open, then click a typed object to query
  it; objects show a kind hint (spatial ◆ / temporal ◷) and filter box.
  The list reloads after any command that can change it — `let`, `derive`,
  `delete`, `kill`, `update`, `changename`, `create`/`drop`, and the
  database commands (`CATALOG_CMD` in `App.tsx`) — so a newly created object
  is one command away from being clickable, and console completion sees it too.
- **Geographic MapLibre + OSM basemap** (`map/MapView.tsx`): when a result's
  coordinates fall within lon/lat ranges the map switches from the Cartesian
  orthographic view to a geographic `MapView` with a raster **OpenStreetMap**
  basemap (no API key), deck.gl layers reprojected to `LNGLAT`; otherwise it
  stays Cartesian. Also **fixed `sline`** decoding (its value wraps the segment
  list with a direction flag).
- **Export**: the layers panel exports all visible layers (static features +
  moving-object trips as LineStrings) as a downloadable `.geojson`
  (`layers/exportGeoJSON.ts`).
- Verified in a headless browser (`npm run e2e -- catalog`): catalog lists
  databases, opening SYMTRAJSMALL lists 27 objects, clicking `EdgesExtDo`
  (lon/lat) activates geographic mode and loads real OSM tiles with the road
  segments aligned to the Hagen street grid, and export yields a valid
  FeatureCollection.

**Milestone 6 (moving regions + value plots) — done & verified end-to-end.**
- **`mregion`** (`app/temporal.py`, `map/MapView.tsx`): a moving region is units
  of faces whose *vertices* each move linearly (`xStart yStart xEnd yEnd`), so
  the polygon is rebuilt at the current instant every frame — the same
  piecewise-linear model as moving points. `mrain` / `msnow` drift across the
  map on the timeline, and work under the BerlinMOD projection.
- **`mreal` / `mint` / `mbool` value plots** (`app/temporal.py`,
  `plots/PlotPanel.tsx`): scalar moving values are sampled into a series and
  drawn as **small multiples** — one plot per measure, each with its own y
  scale (never a second y-axis), coloured by its layer, directly labelled, with
  a cursor and readout that track the timeline. A `ureal` unit is
  `a·t² + b·t + c` (square-rooted when its flag is TRUE) with **t in days** —
  verified against SECONDO's own `val(… atinstant …)` to the 8th decimal.
  Objects with no geometry (e.g. `mreal5000`) plot without touching the map.

**Milestone 7 (SQL via the optimizer) — done & verified end-to-end.**
The console now takes SQL as well as executable commands:

```
select * from kinos
select [id, trip] from trains where id < 5
create table sqltest columns [a: int]
```

The WebUI does **not** decide what is SQL. It sends what the user typed without
a language label (command level `-1`) and the *server* classifies it
(`include/SQLLanguage.h`) and reports which of the three input languages it
resolved to — the same thing the JavaGUI and the TTY do, so the rules exist in
exactly one place and no client carries a copy that can drift. What comes back
depends on that level:

- **SQL (level 2)** answers with `(plan result costs)`. The console shows the
  generated executable plan and its estimated costs; the *result half* is
  byte-identical to what running that plan yourself would produce, so it feeds
  the existing GeoJSON/temporal pipeline unchanged — an optimized query draws
  on the map and animates on the timeline exactly like a kernel query.
- **An optimizer directive (level 3)** — `showOptions`, `setOption(subqueries)`,
  `updateCatalog` — answers with the text the Prolog goal printed.
- The **`optimizer ` prefix** means "optimize but do not execute" for SQL, and
  "run this as a directive" for anything else.
- For a `create`/`drop` the optimizer does the work itself while translating and
  answers with the atom `"done"`; the console says so instead of showing a plan,
  and the catalog refreshes.

There is no separate optimizer server any more: the kernel server runs the
optimizer in-process (the retired `OptServer` and the WebGui's port-1304 socket
client are gone), so SQL travels the connection the session already has.

**Milestone 8 (attribute labels) — done & verified end-to-end.**
Each layer can write one of its tuple's non-spatial attributes next to the
geometry it belongs to — the restaurant's `Name` under its point, a district's
name inside its region, a train's `Id` travelling with its dot. This is
**opt-in**: a layer starts unlabelled and the layers panel offers a `label`
dropdown listing the attributes, ordered so the most label-like one comes first
(`layers/labels.ts` scores names, text, and how distinct and short the values
are; a join's `Name_r` is recognised past the alias suffix the optimizer adds).
Text is drawn just below the symbol so it never covers it, on a translucent dark
plate so it reads on both the dark Cartesian canvas and the light OSM basemap.
(The plate replaced an SDF outline: deck's distance-field glyphs are too coarse
to reconstruct 13px letterforms and came out eroded and sheared off at the top,
so the labels use plain bitmap text — see `LABEL_TEXT` in `map/MapView.tsx`.)

An **individual object** carries no attributes to pick from — `query train7` is
one mpoint, not a tuple — so for those the same `label` row offers a **caption to
type** instead of a list to choose from (`style.labelText`). It is opt-in in
exactly the same way: the box starts empty and an empty box draws nothing. A caption rides a moving object as an attribute label does, and moving
*regions* are labelled too (they previously rendered no text at all).
Verified
in a headless browser (`npm run e2e -- labels`): the candidates are offered with
an explicit "none" selected, choosing one adds drawn pixels, and clearing it
returns the canvas to exactly its previous state; for a single object the typed
caption does the same.

**Milestone 9 (table view + relation editing) — done & verified end-to-end.**
A result that is a relation can be read as **rows and columns**, and a result
that came from a stored relation can be **edited** — the web equivalent of the
Java GUI's `RelViewer` and `UpdateViewer2`.

- **The control belongs to the query, not to the app.** The result pane grows a
  **tab strip** — `◱ Map` plus one closable tab per open table
  (`table/ResultTabs.tsx`). It is not drawn at all while the map is the only tab,
  so the map keeps that height until there is something to switch between. A
  table is opened from the console entry that produced it (`▤ 58 rows — show as
  table`), from the layers panel row, from the catalog's `▤` next to a relation,
  or **automatically** — but only for a result the map cannot show at all, so a
  spatial result never steals focus. Closing a tab puts the table away; the
  result stays a layer.
- **`Run ▾`** (`console/Console.tsx`) is about *how* to run, not about where the
  answer goes: it holds **Explain — plan only**, which writes the `optimizer `
  prefix the user otherwise has to know to type. The main button is unchanged.
  It briefly also offered *Run and open as table* / *Run and show on map*; those
  were removed. Declaring the destination before the answer exists is guesswork —
  auto-opening already covers the case that matters, and "show on map" could not
  keep its promise at all (a result with no geometry has nothing to show, so it
  silently opened the table instead). Routing is decided once the result is
  back, from the console entry or the layers row. *Explain* stays precisely
  because it is the one thing that cannot be decided afterwards: it changes what
  is sent, and a query cannot be un-run.
- **A result that draws nothing is not a layer.** The layers panel is the map's
  legend — visibility, draw order, colour, style — and `query ten` is a relation
  of scalars: real rows, nothing to draw. Such a result gets a tab and a console
  hint but no layer row and no palette slot (`isDrawable` in
  `layers/useLayers.ts`); reordering skips past them, so the arrows never look
  like they did nothing. A result that draws *and* has rows (e.g. `Kinos`) is
  both a layer and a table.
- **Reading** (`app/table.py`): `(rel (tuple ((Name Type)…)))` becomes typed
  columns and rows. Atomic values arrive as JSON numbers/booleans/strings;
  anything else (a point, a region, an mpoint) as its nested-list text, which is
  what `AttributeFormatter.fromListExprToString` shows in the Java GUI. Cells are
  clipped at `MAX_CELL_CHARS`, and an *ad-hoc* result — a join, a projection —
  is capped at `MAX_ROWS` rows, so `query Trains` reports a truncated table
  instead of pushing megabytes at the browser. A stored relation is not capped;
  it is paged (below).
- **A relation is read a page at a time** (`/api/table/load`). The page is cut
  server-side, inside the command the backend writes: `head[n]` alone for the
  first page, and `addcounter[RowNo, 1] filter[.RowNo > offset] head[n]
  remove[RowNo]` for any other, since SECONDO has no `skip`. `head` ends the
  scan once the page is full, so a page costs `offset + limit` tuples read and
  `limit` transferred, and neither the server nor the browser ever holds the
  whole relation. The pager under the grid steps through it; `rows per page` is
  clamped to `MAX_ROWS`. The row *total* comes from a separate `query <Rel> feed
  [filter…] count`, which is a full scan — so it is asked for only when it can
  have changed (opening the table, and saving), and carried over otherwise.
  The paging operators go **after** `addid`, so the tuple identifiers are the
  stored ones on every page. This is why a **filter box narrows the page while a
  column header sorts the relation**: `sortby` is part of the command and
  reorders all of it, whereas the free-text filter is applied in the browser to
  the rows on screen.
- **Editing** (`app/updates.py`) uses the operators of
  `Javagui/viewer/update2/CommandGenerator.java`. `✎ edit` reloads the relation
  through `query <Rel> feed addid … consume`, because the `TID` that `addid`
  appends is what every change is addressed by — a derived result (a filter, a
  join) has none and stays read-only. Cell edits, `+ row` and row deletions
  accumulate as pending changes keyed by TID (so sorting and filtering the grid
  cannot move an edit onto another tuple) until one **save** applies them as
  `inserttuple` / `updatebyid` / `deletebyid`. Keying by TID is also what lets
  pending changes **span pages**: an edit made on page 1 is still addressed to
  the same tuple after paging to page 3, and one save writes both.
- **Indexes are maintained with the data.** SECONDO does not keep them in step
  by itself, so every command is wrapped in `insert/delete/updatebtree` (and the
  rtree equivalents) for each `<Rel>_<Attr>` index found via `list objects` —
  `CommandGenerator.retrieveIndices` does exactly this. Leaving them stale is a
  silent wrong answer to any later query that uses one.
- **Order and atomicity.** The batch is bracketed in a transaction (which
  `UpdateViewer2` dropped but a browser Save needs: it is one gesture, and a
  half-applied batch is worse than none), and runs **updates → deletes → inserts,
  with deletes highest-TID-first**. That order is not cosmetic: a `deletebyid`
  **renumbers the tuple identifiers after it**, so deleting 2, 5, 8 in that order
  actually removes the 2nd, 6th and 10th tuple — verified against a live server.
  Each `updatebyid`/`deletebyid` result count is checked against 1, so a tuple
  another session already deleted is reported rather than silently skipped
  (`UpdateViewerController.java:598` does the same, there being no locking).
- **The DML commands are ordinary SOS text**, the same language the console
  takes — `query <Rel> updatebyid[[const tid value 10]; Bev: 526000] count` — so
  they go out through the same `Session.run` as `list objects` and need no new
  native entry point. They were nested lists first, as the Java GUI writes them;
  that required one, because `SecondoInterface::Secondo(text, …)` hardcodes
  `CMD_LEVEL_TEXT` (`QueryProcessor/SecondoInterfaceGeneral.cpp:443`) and a
  leading `(` does not switch levels — it reaches the SOS parser as a syntax
  error. Checked against a live server, the text form handles every case the
  list form did (a string containing `"`, `,` or `]`; a `text` holding an escaped
  `</text--->`; `point` and `region` constants), and its assignment list
  (`Attr: value`) is simpler than `(Attr (fun (tupleN TUPLE) value))`. The extra
  binding bought nothing, so it is gone.
- Verified in a headless browser (`npm run e2e -- table`) against a relation the
  check creates and drops itself: the table opens by itself for a non-spatial
  result, `✎ edit` brings in the TIDs, an edited cell survives a re-query, a row
  is added and removed again, a spatial result does not steal the active tab, and
  the console hint opens (and `✕` closes) a table for `Kinos` without disturbing
  its map layer.

**Milestone 10 (paged relations, complete operator list) — done & verified end-to-end.**
- **A relation is no longer capped, it is paged.** See the two table-view bullets
  above for how the page is cut (`head` / `addcounter`), why the count is a
  separate query, and why sorting goes to the server while the filter box stays
  in the browser. The 1000-row cap remains only where the backend does not write
  the query — an ad-hoc `query` — and the badge on such a result now points at
  `✎ edit` as the way to reach the whole relation.
- **Completion offers every operator the server has** (`/api/operators`,
  `catalog.parse_operators`, `console/completion.ts`). It used to offer a
  hardcoded list of 101 names kept in the frontend, so an operator like
  `createsuffixtree` was missing for no better reason than that nobody had added
  it. The list now comes from the `list operators` inquiry —
  `SecondoCatalog::ListOperators`, every operator of every loaded algebra, ~1800
  on a full build — which reads the algebra catalog directly and, unlike
  `list types`, needs no open database, so it is fetched once per session and
  never refreshed. Each entry carries the `Syntax` from its
  `Operator::GetSpecList()`, which becomes the hint beside the name. Enabling an
  algebra is now enough to make its operators complete.
- **The menu had to change shape for that.** With ~1800 names a substring match
  on a two-letter token is noise, so **operators match on their prefix only**
  while object names and command words still match anywhere — an object name is
  worth guessing at from the middle, an operator is not — and the menu holds 10
  items rather than 6.
- Verified in a headless browser (`npm run e2e -- paging`) against a 450-row
  relation the check creates and drops itself: the pager reports the true total,
  steps forward and back, resizes, and sorting descending puts row 450 on the
  *first* page — which browser-side sorting of a 200-row page could never do.
  A cell is edited on page 1, another on page 2, and one save writes both.
  `npm run e2e -- ux` additionally checks that `createsuffi` completes to
  `createsuffixtree` with its syntax.

**Milestone 11 (GPX import by drag-and-drop) — done & verified end-to-end.**
- **A GPX track is dropped on the catalog and imported.** The drop zone sits at
  the foot of the object list and announces itself whenever a file is dragged
  anywhere over the window; clicking it opens a file chooser, which is the same
  path by keyboard. Without an open database it says so rather than silently
  refusing. The file goes to the bridge (`POST /api/upload`) and is stored in
  its temp directory; an import dialog then proposes a name derived from the
  filename (`2026-07-26_Wanderung.gpx` → `gpx_2026_07_26_wanderung`) and lists
  the four objects that name will produce, each with what it *is* — the
  suffixes alone do not say, and one dropped track turning into four catalog
  entries is worth explaining before it happens:

  ```
  WILL CREATE
    ·  wanderung               raw GPX import
    ·  wanderung_mp            moving point
    ·  wanderung_trajectory    trajectory
    ·  wanderung_bbox          bounding box
  ```

  Pressing Import runs four commands in order and turns that same list into the
  progress — nothing moves, the row being worked on gets a spinner and says what
  it is doing, and a finished one gets a green check:

  ```
  let <n>            = gpximport('/tmp/…') consume
  let <n>_mp         = <n> feed projectextend[; T: .Time, P: makepoint(.Lon, .Lat)]
                              approximate[T, P]
  let <n>_trajectory = trajectory(<n>_mp)
  let <n>_bbox       = bbox(<n>_trajectory)
  ```

  This is the pipeline from `bin/Scripts/ReadingASetOfGPXFiles.sec` cut down to
  one file. `gpximport` is MapMatchingAlgebra, the rest Temporal/Spatial; all
  are in a stock build.
- **All four names are checked before the first command runs.** A name is
  refused unless it is lowercase `[a-z][a-z0-9_]*`, and unless *every* object
  the import would create is free — checking only the first would let the import
  fail halfway, which is the one outcome worth designing against. If a command
  fails anyway, the import stops there, shows SECONDO's own message under the
  failed step and names what it did create. Nothing is deleted behind the user's
  back; the catalog refreshes so the leftovers can be seen and dropped.
- **The commands run on the user's own session**, not server-side in a batch, so
  the optimizer's catalog is refreshed after each `let`
  (`Session._update_catalog_if_wanted`) and every command is left in the console
  where it can be read and run again. They use a new `view: "none"` on
  `/api/query`: `let x = <a long track> consume` answers with the whole created
  object, and neither the payload conversion nor the transfer is worth paying
  for when nothing will render it.
- **The upload is the request body, not a multipart part** — one file needs none
  of what multipart buys, and this way the bridge does not grow a
  `python-multipart` dependency for it. The filename is sanitized to a basename
  of `[A-Za-z0-9._-]` and stored under a `mkstemp` name: it is pasted into a
  SECONDO text literal, so an apostrophe in it would end the literal, and a
  directory in it would put the file wherever the uploader liked. Uploads are
  capped at `WEBUI_MAX_UPLOAD_BYTES` (64 MiB) and deleted when the session that
  made them closes.
- **`gpximport` opens the path on the SECONDO server**, while the upload lands
  on the *bridge*. The two therefore have to share a filesystem — which they do
  in the default deployment (`SECONDO_HOST=127.0.0.1`). Against a remote server
  the import fails on its first command with the server's own "file not found",
  which says as much.
- Verified in a headless browser (`npm run e2e -- gpx-import`) against
  `bin/Trk_MapMatchTest.gpx`: the zone is disabled until a database is open, the
  dialog proposes the name derived from the filename and previews all four
  objects with their kinds, all four steps go green, the catalog then lists all
  four objects, and a second import of the same file is refused with the
  collision named before anything runs. The check deletes what it created.

**Milestone 12 (symbolic trajectories) — done & verified end-to-end.**
`mlabel` and `mstring` — a *text value over time*, what SECONDO's
SymbolicTrajectory algebras call a symbolic trajectory — are drawn as text
riding along with the moving point of the same tuple, changing as the animation
runs. It is the feature the old GWT `WebGui2` had as its "label mode"
(`MVMPointController.animateMovingPoint`), on a clock that respects how long
each unit actually lasts rather than giving every unit the same 100 ms.

The value is read **at the current instant** by interval lookup (`labelAt` in
`map/MapView.tsx`, the constant-valued twin of `positionAt`). WebGui2 instead
flattened the mlabel into an array parallel to the mpoint's units, which capped
label resolution at the mpoint's granularity and assumed each mpoint unit nested
inside one label unit; nothing here needs the two to line up.

Unlike attribute labels this is **not opt-in**: every symbolic attribute of the
tuple gets its own line straight away, stacked under the dot in
relation-schema order. A symbolic attribute is in the result because the query
asked for it, and a relation may carry several — a map-matched track with both
its road class and its street name:

```
query intstream(1,1) transformstream projectextend[
    ; Trip: mmOnNetMP2, RoadType: mmRoadTypeMLc, RoadName: mmRoadNameMLc] consume

        ● footway
          Deichpromenade
```

Which of them, and whether each line says what it is, are then a **layer
setting** — an `MLabel options` block in the layers panel with a checkbox per
trajectory and a `Show Key Prefix` dropdown:

```
  MLabel options                   ● RoadType: footway
   ☑ RoadType                        RoadName: Deichpromenade
   ☑ RoadName
   Show Key Prefix  [true ▾]
```

The set is stored as the attributes left *out* (`style.symbolicHidden`) rather
than the ones taken, so re-running a query that grew an attribute shows it
instead of silently dropping it. The key prefix is off by default: with one
trajectory the name is noise, and it is the second one that makes telling them
apart worth the width.

Lines whose series has no value at that instant are simply absent, so a stack
grows and shrinks as the track moves in and out of what each trajectory
describes; the *top* line keeps its place while it does, rather than the block
re-centring. Labels share one stack with the chosen attribute label rather than
forming a second one — two labels at one anchor would collide and the
declutterer would drop one of them.

Backend: a fourth channel, `labels`, beside `trips`/`regions`/`plots`
(`app/temporal.py`), carrying `[t0, t1, text]` intervals and the tuple `row`
that ties them to a trip (`properties._row`). Runs of an equal label are merged
— SECONDO splits a symbolic trajectory where its *source* changes, not where
the label does, so a map-matched track gives 429 units holding 24 distinct runs
— and trailing whitespace is stripped, because a shapefile import pads
`Fclass` to 28 characters and the padding would otherwise make the value
compare unequal to the obvious `tolabel("footway")`. A label alone is not a
payload: with nothing to ride it would draw nothing while still widening the
animation domain every other layer shares.

Two things this does *not* do: `ilabel` (an instant, not a function of time)
and the set-valued `mlabels`/`mplaces` are still textual nested lists, and a
label needs the moving point drawn, so `moving: trail` shows none (as it
already did for attribute labels; the default is `both`).

Verified in a headless browser (`npm run e2e -- symbolic`), which builds two
mlabels out of berlintest `train7`'s own units since stock berlintest has none:
both attributes get a line, seeking from 20% to 80% changes the text to what the
mlabel actually holds there, the checkboxes add and remove their lines, `show
names` prefixes them, unticking everything leaves the dot alone, a plain mpoint
gets no `MLabel options` block at all, and an mlabel on its own makes no layer
and no page error.

### A note on coordinates & the basemap

SECONDO spatial values carry coordinates in the dataset's *own* world system
(berlintest uses a local planar system, not WGS84), so the default view is a
non-geographic **Cartesian** one — correct for any dataset, with no basemap.
Geographic rendering on an **OSM basemap** is opt-in on top of that, exactly as
the HoeseViewer treats its OSM background: automatic for datasets already in
lon/lat, and via the BerlinMOD transform for berlintest. See **Projections**
below.

## Prerequisites

- A built SECONDO tree with the environment sourced (`source ~/.secondorc`, which
  sets `SECONDO_BUILD_DIR`, `SECONDO_PLATFORM`, `BERKELEY_DB_DIR`).
- The prebuilt client lib `apis/api_cpp/cs/lib/libsecondo.a` (built via
  `cd apis/api_cpp/cs && make`). Rebuild it before `make -C native` after
  updating the tree: it now also carries the SQL language rules
  (`UserInterfaces/SQLLanguage.o`), which the native module links against.
- Python 3.11+ and Node 20.19+ (required by Vite 8).
- **For SQL:** a tree built with SWI-Prolog (without `PL_INCLUDE_DIR` the kernel
  is compiled `-DNO_OPTIMIZER`) and `[Environment] EnableOptimizer` not turned
  off in `bin/SecondoConfig.ini`. Everything else works without it; the console
  then shows `sql: off` and only takes executable commands.

## Build & run

Two shapes, and they are not variations of one setup:

- **Development** — two servers. Vite on `:5173` serves the frontend with hot
  reload and proxies `/api` to uvicorn on `:8000`. Steps 1-3 below.
- **Production** — *one* server. The bridge serves `/api/*` and the built
  frontend together on a single port, so there is no Vite process and no proxy.
  See [Production](#production-one-server).

### 1. Backend (native module + FastAPI)

```bash
source ~/.secondorc
cd WebUI/backend
python3 -m venv .venv && . .venv/bin/activate
pip install -r requirements.txt

# Build the pybind11 wrapper over libsecondo.a
make -C native

# Run the bridge (reads SECONDO_HOST/PORT/CONFIG from env; the server defaults
# to 127.0.0.1:1234, the config to $SECONDO_BUILD_DIR/bin/SecondoConfig.ini)
uvicorn app.main:app --host 127.0.0.1 --port 8000
```

**`source ~/.secondorc` matters for `uvicorn` too, not just for the build.**
The bridge refuses to open a connection it cannot configure — `503` with the
reason, an error line at startup, and `/api/health` reporting `"status":
"misconfigured"` plus the resolved path. That path is never guessed:
`SECONDO_CONFIG`, else `$SECONDO_BUILD_DIR/bin/SecondoConfig.ini`, else an
error.

This started as a *dead API*: `/api/health` answered while every other endpoint
hung forever. The cause was not the bridge. A SECONDO client takes
`Server:BinaryTransfer` from its own configuration and the server takes it from
its own, with nothing on the wire to agree on it — and a disagreement did not
fail, it deadlocked, the client blocking in a socket read (no timeout) for a
text end tag while the server had sent a binary list. A bridge started without
`SECONDO_CONFIG` had no flags at all and hit exactly that. **The server now
announces the mode in its connect handshake** (`BinaryTransfer=YES|NO` in
`<SecondoIntro>`, documented in `include/CSProtocol.h`) and every client — the
C++ one behind this bridge, and the Java one behind the GUI — adopts it instead
of consulting its own configuration.

### 2. A running SecondoMonitor

```bash
cd $SECONDO_BUILD_DIR/bin
./SecondoMonitor -s      # listens on port 1234
```

### 3. Frontend (development)

```bash
cd WebUI/frontend
npm install
npm run dev              # http://localhost:5173  (proxies /api -> :8000)
```

Open http://localhost:5173 and try (each spatial result draws on the map):

```
open database berlintest
query mehringdamm      # a point
query BGrenzenLine     # district boundaries (line)
query thecenter        # a region
query Flaechen feed head[5] consume            # a relation of regions
query Trains feed head[3] project[Id, Trip] consume   # moving points -> animated
```

The same console takes SQL, if the server has the optimizer:

```
select * from kinos                        # optimized, then drawn on the map
select [id, trip] from trains where id < 5 # animated on the timeline
optimizer select * from kinos              # show the plan, do not run it
showOptions                                # an optimizer directive
```

## Production (one server)

`npm run build` compiles the frontend into `frontend/dist/`, and the bridge
serves that directory itself (`mount_static` in `app/main.py`). The result is one
process on one port answering both `/api/*` and the UI — no Vite, no proxy, and
the session cookie is same-origin by construction rather than by proxy
configuration.

```bash
source ~/.secondorc
make -C WebUI venv native     # once: virtualenv + the pybind11 module
make -C WebUI prod            # build frontend/dist, then serve everything
```

Then open `http://<host>:8000`. A SecondoMonitor still has to be running
(step 2 above). The `Makefile` also has `build`, `serve`, `native`, `venv` and
`clean` separately, so a redeploy after a UI change is `make -C WebUI build` and
a restart.

**Listen address.** `HOST` and `PORT` are make variables:

```bash
make -C WebUI prod HOST=127.0.0.1     # this machine only
make -C WebUI prod PORT=9000
```

The default is `HOST=0.0.0.0`, i.e. reachable from the network. **The WebUI has
no authentication**: anyone who can reach the port can read and modify every
database on the connected server, `save`/`restore` typed into the console
write into the *server's* filesystem, and `/api/upload` writes into the
*bridge's* temp directory (capped by `WEBUI_MAX_UPLOAD_BYTES`, 64 MiB, and
cleaned up when the session closes). On anything but a trusted network, bind
`127.0.0.1` and put an authenticating reverse proxy in front of it.

**Run a single worker.** Sessions live in an in-process dict
(`SessionManager`, `app/session.py`) and each holds one SECONDO connection, so
with `--workers N` a session cookie would land on a worker that has never heard
of it — the object list would come back empty at random. The `Makefile` runs one
worker; keep it that way. Concurrency comes from the async bridge and from the
monitor forking a process per connection, not from uvicorn workers.

**Other things worth knowing:**

- `source ~/.secondorc` matters for the production server exactly as it does for
  the dev bridge — same 503, same `/api/health` reporting `misconfigured`.
- The served bundle is a build artifact, not the sources: after editing anything
  under `frontend/src`, `make -C WebUI build`. `frontend/dist/` is gitignored.
- With no build present the server still starts and serves the API only, saying
  so at startup (`No frontend build at … -- serving the API only`). That is the
  development case, not an error.
- `WEBUI_STATIC_DIR` overrides which directory is served, if the build lives
  somewhere else.

## Tests

Backend unit/API tests (no live monitor required — the connection is faked):

```bash
cd WebUI/backend && . .venv/bin/activate
python -m pytest            # nested-list parser, GeoJSON converter, API routes
```

Frontend end-to-end map test (needs the full stack up: monitor + bridge + vite):

```bash
cd WebUI/frontend
npm run e2e                # the whole suite
npm run e2e -- plots       # only checks whose name matches "plots"
```

The same suite runs against the **production** server — point it at the single
port and everything (built assets, API, same-origin cookie) is exercised through
one process:

```bash
make -C WebUI prod &
cd WebUI/frontend && WEBUI_URL=http://127.0.0.1:8000/ npm run e2e
```

`e2e/run.mjs` discovers every `e2e/verify-*.mjs`, runs them sequentially and
reports pass/fail by **exit code** (each check is a standalone program), echoing
the output of any failure. It preflights the stack and tells you what is missing
rather than dumping navigation timeouts. Screenshots land in `e2e/out/`.
Override with `CHROMIUM=`, `WEBUI_URL=`, `WEBUI_API=`.

Current checks: `animation`, `basemap-picker`, `catalog-basemap`,
`catalog-manual-refresh`, `catalog-race`, `catalog-refresh`, `console`,
`db-selection`, `gpx-import`, `labels`, `layer-icons`, `layer-rename`,
`layers`, `map`, `mpoint-fit`, `mregion`, `paging`, `pending-entry`, `plots`,
`projection`, `remove-layer`, `render-modes`, `space-flip`, `sql`,
`symbolic-labels`, `table`, `table-intent`, `theme`, `ui-polish`, `ux`,
`viewfit`.

`table`, `paging` and `catalog-refresh` write to the database — each creates an
object, works on it and deletes it again. None touches the shipped berlintest
objects.

Real end-to-end sanity check (needs a monitor + berlintest):
`query mehringdamm` should return `(point (9396.0 9871.0))` and draw a dot.

## Layout

```
Makefile         build + run the production server (make prod / build / native)
backend/
  native/        pybind11 wrapper (secondo_native.cpp, Makefile)
  app/           FastAPI app: config, session, main, catalog,
                 nlparser + nlwriter, geojson (static) + temporal (moving)
                 + table (rows) + updates (relation editing) + convert
  tests/         parser / geojson / temporal / table / updates / API tests
                 + fixtures
frontend/
  dist/          production build (npm run build); served by the bridge
  src/api/       bridge client + types
  src/catalog/   database + object browser
  src/console/   command console (history recall, focus retention)
                 + history.ts: recalled commands persisted in localStorage
  src/layers/    useLayers state + LayersPanel (style/reorder) + GeoJSON export
  src/map/       deck.gl MapView (Cartesian OR geographic MapLibre + basemap)
                 + projection (Berlin2WGS) + basemaps (the raster choices)
  src/plots/     PlotPanel: mreal/mint value plots as small multiples
  src/table/     result tabs + the row grid and its pending-change model
  src/timeline/  useAnimator hook + Timeline controls
  e2e/           headless-browser checks + run.mjs (the suite runner)
```

## Projections

A projection selector on the map (top-left) controls how world coordinates are
placed:

- **Flat (no map)** — the Cartesian orthographic view (default); correct for any
  dataset, no basemap.
- **BerlinMOD → OSM** — applies SECONDO's `Berlin2WGS` BBBike→WGS84 transform
  (`src/map/projection.ts`, constants from `Algebras/Spatial/Berlin2WGS.cpp`) to
  every layer, so berlintest / BerlinMOD data lands at its real Berlin location
  on the OpenStreetMap basemap (e.g. `mehringdamm` → 13.389°E, 52.495°N). Points,
  lines, regions and moving objects are all projected.

Datasets already in WGS84 (e.g. SYMTRAJSMALL `EdgesExtDo`) render on OSM
automatically under "Flat" via lon/lat detection — leave the selector on Flat for
those.

## Layout

The map is the product, so the layout gives it the space:

- **Console docked under the map**, across the full width and kept short. (It
  used to be dockable to the left as well; that only narrowed the map for no
  gain, so there is one layout.)
- **Query history collapses** (`▾ history`) to leave just the command input as a
  slim bar — the history is rarely needed at length, and the map takes the
  freed height.
- **Catalog collapses** to a thin rail (`◂` / `▸`), handing its width to the map.
- **All panels are drag-resizable** via the splitters between them.
- **Theme**: dark by default, switchable to a light grey one with `☀ light` /
  `☾ dark` in the console header. The whole UI is drawn from CSS custom
  properties (`src/styles.css`), so a theme is one palette block; the choice is
  applied as `data-theme` on `<html>` before the first render.
- Panel sizes, collapse state and theme persist in `localStorage`.

**The query box grows with the query.** It is a textarea sized to its content
(`src/console/Console.tsx`), so a long `select … from … where …` is readable
while it is written instead of scrolling sideways in a one-line slit; past
~11rem it stops growing and scrolls. `⏎` runs the command as it always did and
`⇧⏎` breaks a line; the line breaks are folded back to single spaces on the way
to the server, so the log stays one line per command while `↑` brings the query
back formatted. `↑`/`↓` recall from the first/last line, so in a multi-line query
the arrows move the caret first.

**The command history survives a reload.** The last 200 typed commands are kept
in `localStorage` (`src/console/history.ts`), so `↑`/`↓` still walks back into
what earlier sessions asked — a history file, as SecondoTTY keeps one in
`.secondo_history`. Only the commands are stored, not their answers: a result
belongs to the session that ran it (its geometry is on a map that a reload drops
anyway). `⌫ clear` in the console header empties the log on screen and forgets
the remembered commands.

Note that the map only auto-fits when new data arrives or the projection
changes — editing a layer's style, toggling visibility or removing a layer
leaves the view exactly where you put it. Use the `⤢` button to re-fit on demand.

## UX / rendering options

- **Loading indicator** while a query runs.
- **Moving-object render mode** (per layer, in the style editor): `trail`
  (animated fading trail), `positions` (a dot at each object's exact current
  position — clearer when many objects move at once), or `both`. The layer's
  **point radius** sizes the position dots and **line width** sizes the trail,
  so those style controls now affect moving objects too, not just static ones.
- **Layer name** (per layer, in the style editor): a layer is auto-named after
  the query that produced it, which is provenance rather than a legend. The
  `name` field renames it; the original command stays as the row's tooltip, and
  clearing the field restores the auto-name. The new name follows the layer
  everywhere — the row, the selection details header, the value plots and the
  `_layer` stamp in a GeoJSON export.
- **Point icon** (per layer, in the style editor): points and moving-object
  positions draw as a circle by default; the `icon` picker swaps in a symbol
  (`bus`, `rail`, `car`, `restaurant`, …) tinted with the layer's colour, so a
  trains layer and a buses layer are distinguishable by shape and not by colour
  alone. The **point radius** still sizes them. The picker (`layers/IconPicker`)
  opens a grid of the glyphs themselves rather than a list of names — a name
  like `rail-metro` says little about a 15px symbol — each drawn in the layer's
  colour, with the hovered one named underneath; Escape closes it. The icons are
  [Maki](https://github.com/mapbox/maki) (CC0-1.0, drawn for cartography at
  small sizes); `layers/icons.ts` rasterises the offered subset into one PNG
  atlas at first use and hands it to deck.gl, and offering another of Maki's 215
  icons is one import plus one entry there.
- **Basemap** (`map/basemaps.ts`): geographic mode draws over a raster basemap,
  and a picker beside the projection select chooses between **OSM**
  (OpenStreetMap, the default), **Satellite** (Esri World Imagery) and **Dark**
  (CARTO dark-matter). The two selects share one row (`.map-ctl`) and never
  wrap to two: below 520px the layers panel becomes a full-width band directly
  underneath, positioned to clear exactly one row of controls, and the controls
  outrank it, so a second row would put the panel's own `▾ Layers` toggle
  beyond reach. The row is bounded on the right and its selects shrink rather
  than overflow, so it survives a 320px screen.
  All three are key-free and carry their own attribution, which
  MapLibre renders; Google's layers — which the old GWT `WebGui2` offered
  through an OpenLayers `LayerSwitcher` — need an API key and terms of use, so
  they are deliberately absent. The picker only exists in geographic mode: in
  Cartesian there is no basemap to pick, and a permanently disabled dropdown is
  worse than an absent one. The choice is a display preference like the theme,
  not a property of the dataset the way the projection is, so it is remembered
  in `localStorage` across reloads.

  **Labels take their contrast from the basemap, not from the mode.**
  `onLightCanvas` used to read `geographic || theme === "light"`, which was only
  correct while the light OSM raster was the only basemap — over imagery or
  dark-matter every label would have kept near-black ink on a white halo and
  become unreadable. Each basemap now carries a `light` flag and that decides;
  the app theme still decides for the Cartesian canvas, which is `--bg-deep`.

  Two provider quirks worth not rediscovering: the ArcGIS REST tile scheme is
  `{z}/{y}/{x}` — row before column, unlike every other provider here — and
  CARTO's URL must not carry Leaflet's `{r}` retina placeholder, which MapLibre
  would request literally instead of substituting. Each source also sets an
  explicit `maxzoom`, or the map goes blank when zoomed past what the provider
  has.
- Internal keys (`_attr`, `_layer`) are hidden from the map tooltip and the
  selection details panel.
- **View fitting & zoom controls:** the map view is controlled and auto-fits to
  the visible data when new data arrives or the projection changes (so toggling
  BerlinMOD→OSM re-centers instead of showing the whole world). On-map buttons
  provide zoom in / zoom out / fit-to-data.

## Notes

- **Text encoding:** SECONDO stores strings in Latin-1 (ISO-8859-1). The native
  binding decodes results as Latin-1 (lossless) rather than UTF-8, so names with
  umlauts (e.g. `Stölpchensee`, `UFA-Filmbühne Wien` in berlintest `Kinos` /
  `WFlaechen`) round-trip correctly instead of crashing the query.
- **A `text` atom comes back as `'…'`, not `<text>…</text--->`.** That is what
  `NestedList::ToString` writes, so it is what a `text` attribute looks like on
  the way in. `nlparser` handles both forms (plus their `\'` / `\\` and
  `\</text--->` escapes, per `Tools/NestedLists/NLLex.l`); without the short
  form a text value was read as a bare symbol and stopped at the first space —
  `'a </text---> b'` arrived as `'a`.
- **Error handling:** any backend failure returns a JSON body (global exception
  handler), and the frontend parses responses defensively, so a server error can
  never surface as a raw "JSON.parse: unexpected character" in the UI.
- **Session cookie / request ordering:** API requests are serialized in a single
  queue (`src/api/client.ts`) so the first request establishes the session
  cookie before others fire. Without this, clicking a database immediately on
  load could mint a second session and leave the object list empty. The catalog
  also shows a loading spinner while objects are fetched and ignores repeat
  clicks while a database is opening.
- **Connections are a scarce resource.** Every session holds one SECONDO
  connection and the server forks a process (`SecondoBDB -srv`) per connection,
  so they must be reclaimed:
  - the frontend releases its session on `pagehide` via `sendBeacon`;
  - the backend reaps sessions idle beyond `SECONDO_SESSION_IDLE_TIMEOUT`
    (default 1800s, swept every `SECONDO_SESSION_REAP_INTERVAL`, default 60s),
    never reaping one that is mid-command;
  - all sessions are closed on shutdown.
- **Closing a session waits for the command it is running.** `SessionManager.close`
  takes the session lock, and this is not tidiness: `Connection::close` deletes
  the `SecondoInterfaceCS` and with it the socket's `iostream` and `SocketBuffer`,
  while a command still in flight on a worker thread is sitting in
  `SocketBuffer::underflow` reading the response. It then dereferences `gptr()`
  into freed memory (`ClientServer/SocketIO.cpp:153`) — a **general protection
  fault that kills the bridge process**, not an exception anything can catch.
  Reloading the page mid-query is exactly that race, since the frontend releases
  its session with a `pagehide` `sendBeacon` that does not wait for anything. It
  showed up as the backend dying part-way through a long e2e run, with
  `traps: python[…] general protection fault … in secondo_native…so` in `dmesg`
  and nothing at all in the bridge's own log. The session id is popped from the
  map before the lock is taken, so a request arriving meanwhile cannot pick the
  session up again and the wait is bounded by the one command already running.
- **The native binding must never hold the GIL during server I/O.**
  `Connection`'s connect/terminate/`secondo`/`secondo_auto` all release it. A
  connect that hangs (slow or wedged SECONDO) would otherwise freeze the whole
  Python process — including endpoints that never touch SECONDO.
- **The language rules are never reimplemented, only linked.** The backend does
  not guess what is SQL and does not own a regex for the `optimizer ` prefix: it
  calls `stripOptimizerPrefix` from `libsecondo.a` and lets the server classify
  the rest. This is the same principle as never reimplementing the wire
  protocol, applied one level up.
- **Whether SQL is on offer is a property of the connected server**, so it is
  probed once per session at connect (`<OptimizerAvailable/>`) and never cached
  beyond it. It rides along on `/api/databases`, which is the session-state
  endpoint the catalog polls anyway.
- **The optimizer's catalog is kept in step.** It only rereads the schema when
  the database changes, so after a kernel `let`/`create`/`delete`/`update` the
  backend sends `updateCatalog` (as the JavaGUI does). Disable with
  `SECONDO_AUTO_UPDATE_CATALOG=false`.
- **Results arrive whole, not streamed.** The client/server protocol has one
  response per command (`CSProtocol::ReadResponse` reads a single
  `<SecondoResponse>`) and tuple streams never leave the server process, so there
  is nothing for a WebSocket to deliver progressively — it would carry one message
  per query, at the moment the HTTP response arrives anyway. Large results are
  handled by asking for less instead. Where the backend writes the command — a
  stored relation, `/api/table/load` — that means **paging**: `head`/`addcounter`
  cut one page out of the stream server-side, so the size of the relation stops
  mattering. Where it does not — an ad-hoc `query`, whose text is the user's —
  there is nothing to rewrite, so the `MAX_ROWS` cap in `app/table.py` stands and
  the table says it truncated; `view: "table"` also skips building the spatial and
  temporal payloads. Pressing `✎ edit` on such a result is the way over to the
  paged path, and the truncation badge says so.
  SECONDO *does* push query-progress messages mid-execution (`ProgressView`, what
  the JavaGUI's progress bar reads); the bridge does not register a message
  handler for them, so the UI shows an indeterminate spinner.

## Roadmap (next)

- Additional projections beyond BerlinMOD as needed.
- Remaining long-tail types (network/JNet, precise geometry, raster) still fall
  back to the textual nested-list view, as `DsplGeneric` does in the Java GUI.
- Table view follow-ups: a load dialog for the filter/project/sort the backend
  already accepts (`/api/table/load`), nested `nrel`/`arel` relations, and
  linking a selected row to its geometry on the map.
- Kernel follow-ups surfaced while wiring up SQL, all pre-existing:
  `SecondoInterfaceCS::Secondo` assigns `resolvedCmdLevel` only on its "usual
  command" branch, so after a client-intercepted `save`/`restore` it reports the
  *previous* command's level (harmless only because every client also checks the
  answer's shape); `save`/`restore` typed into a browser writes into the
  *backend's* working directory, not the user's machine; and `Connection.secondo`
  decodes Latin-1 coming back but does not encode it going out.
