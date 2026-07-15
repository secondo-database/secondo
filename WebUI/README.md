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
  `cd apis/api_cpp/cs && make`).
- Python 3.11+ and Node 20.19+ (required by Vite 8).

## Build & run

### 1. Backend (native module + FastAPI)

```bash
source ~/.secondorc
cd WebUI/backend
python3 -m venv .venv && . .venv/bin/activate
pip install -r requirements.txt

# Build the pybind11 wrapper over libsecondo.a
make -C native

# Run the bridge (reads SECONDO_HOST/PORT/CONFIG from env; defaults to
# 127.0.0.1:1234 and $SECONDO_BUILD_DIR/bin/SecondoConfig.ini)
uvicorn app.main:app --host 127.0.0.1 --port 8000
```

### 2. A running SecondoMonitor

```bash
cd $SECONDO_BUILD_DIR/bin
./SecondoMonitor -s      # listens on port 1234
```

### 3. Frontend

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

`e2e/run.mjs` discovers every `e2e/verify-*.mjs`, runs them sequentially and
reports pass/fail by **exit code** (each check is a standalone program), echoing
the output of any failure. It preflights the stack and tells you what is missing
rather than dumping navigation timeouts. Screenshots land in `e2e/out/`.
Override with `CHROMIUM=`, `WEBUI_URL=`, `WEBUI_API=`.

Current checks: `animation`, `catalog-basemap`, `catalog-race`, `console`,
`layers`, `map`, `mpoint-fit`, `mregion`, `plots`, `projection`, `remove-layer`,
`render-modes`, `ui-polish`, `viewfit`.

Real end-to-end sanity check (needs a monitor + berlintest):
`query mehringdamm` should return `(point (9396.0 9871.0))` and draw a dot.

## Layout

```
backend/
  native/        pybind11 wrapper (secondo_native.cpp, Makefile)
  app/           FastAPI app: config, session, main,
                 nlparser + geojson (static) + temporal (moving) + convert
  tests/         parser / geojson / temporal / API tests + fixtures
frontend/
  src/api/       bridge client + types
  src/catalog/   database + object browser
  src/console/   command console (history recall, focus retention)
  src/layers/    useLayers state + LayersPanel (style/reorder) + GeoJSON export
  src/map/       deck.gl MapView (Cartesian OR geographic MapLibre + OSM)
                 + projection (Berlin2WGS)
  src/plots/     PlotPanel: mreal/mint value plots as small multiples
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

The map is the product, so the layout defaults to giving it the space:

- **Console docked to the bottom** by default (the map keeps the full width) and
  kept short. Toggle it to the left with `⇦ left` / `⇩ bottom`.
- **Query history collapses** (`▾ history`) to leave just the command input as a
  slim bar — the history is rarely needed at length, and the map takes the
  freed height.
- **Catalog collapses** to a thin rail (`◂` / `▸`), handing its width to the map.
- **All panels are drag-resizable** via the splitters between them.
- Layout, sizes and collapse state persist in `localStorage`.

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
- **The native binding must never hold the GIL during server I/O.**
  `Connection`'s connect/terminate/`secondo` all release it. A connect that
  hangs (slow or wedged SECONDO) would otherwise freeze the whole Python
  process — including endpoints that never touch SECONDO.

## Roadmap (next)

- Streaming large results over the WebSocket (`/api/stream`) and query-history
  persistence.
- Include the optimizer (add it to the REST backend, implement the port), allow
  to run SQL-line queries from the WebUI. In the existing UI, you need to start
  the OptServer for that.
- Additional projections beyond BerlinMOD as needed.
- Remaining long-tail types (network/JNet, precise geometry, raster) still fall
  back to the textual nested-list view, as `DsplGeneric` does in the Java GUI.
- Implement a regular table view that also allows updates (like the UpdateViewer
  in the Java GUI).
