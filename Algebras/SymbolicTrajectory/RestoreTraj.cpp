
/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

Started March 2012, Fabio Vald\'{e}s

[TOC]

\section{Overview}
This is the implementation of the Symbolic Trajectory Algebra.

\section{Defines and Includes}

*/

#include "RestoreTraj.h"

using namespace temporalalgebra;
using namespace std;
using namespace raster2;

namespace stj {
  
const int SPEED_MAX = 1000;
  
const int MaxspeedRaster::getMaxspeedFromRoadInfo(string roadInfo1, 
                                                  string roadInfo2) {
  NewPair<string, string> info(roadInfo1, roadInfo2);
  if (info.first == "access" && 
     (info.second == "agricultural" || info.second == "destination" ||
      info.second == "private" || info.second == "yes")) {
    return 30;
  }
  if (info.first == "bicycle" && (info.second == "designated" || 
      info.second == "yes")) {
    return 50;
  }
  if (info.first == "cycleway" && info.second == "lane") {
    return 50;
  }
  if (info.first == "cycleway:left" && info.second == "track") {
    return 50;
  }
  if (info.first == "foot" && (info.second == "designated" || 
      info.second == "official" || info.second == "permissive" ||
      info.second == "yes")) {
    return 15;
  }
  if (info.first == "footway" && info.second == "sidewalk") {
    return 15;
  }
  if (info.first == "highway") {
    if (info.second == "bridleway" || info.second == "cycleway" ||
        info.second == "tertiary" || info.second == "track" ||
        info.second == "residential" || info.second == "service") {
      return 50;
    }
    if (info.second == "footway" || info.second == "path" || 
        info.second == "steps" || info.second == "living_street") {
      return 15;
    }
    if (info.second == "motorway" || info.second == "motorway_link") {
      return 200;
    }
    if (info.second == "primary"  || info.second == "secondary") {
      return 130;
    }
  }
  if (info.first == "maxspeed" && (info.second == "10" ||
      info.second == "100" || info.second == "120" || info.second == "130" ||
      info.second == "20" || info.second == "30" || info.second == "40" ||
      info.second == "50" || info.second == "60" || info.second == "70" ||
      info.second == "80")) {
    int result;
    istringstream(info.second) >> result;
    return result + 20;
  }
  if (info.first == "motor_vehicle" && (info.second == "agricultural" ||
      info.second == "forestry" || info.second == "private")) {
    return 50;
  }
  if (info.first == "service" && (info.second == "parking_aisle" ||
      info.second == "driveway" || info.second == "alley")) {
    return 30;
  }
  if (info.first == "sidewalk" && (info.second == "both" ||
      info.second == "left" || info.second == "right")) {
    return 15;
  }
  if (info.first == "vehicle" && (info.second == "agricultural" ||
      info.second == "forestry" || info.second == "private")) {
    return 50;
  }
  return SPEED_MAX;
}

const int MaxspeedRaster::getMaxspeedFromLeaf(TupleId leafinfo) {
  Tuple *ntuple = 0, *tuple = 0, *atuple = 0;
  ntuple = primary->GetTuple(leafinfo, false);
  tuple = nrel->NestedTuple2Tuple(ntuple);
  AttributeRelation *arel = (AttributeRelation*)(tuple->GetAttribute(4));
  DbArray<TupleId> *ids = arel->getTupleIds();
  TupleId id;
  int result = SPEED_MAX, tempResult = SPEED_MAX;
  for (int i = 0; i < ids->Size(); i++) {
    ids->Get(i, id);
    atuple = arel->getRel()->GetTuple(id, false);
    if (atuple != 0) {
      FText *roadInfo1 = ((FText*)(atuple->GetAttribute(0)));
      FText *roadInfo2 = ((FText*)(atuple->GetAttribute(1)));
      if (roadInfo1->IsDefined() && roadInfo2->IsDefined()) {
        tempResult = getMaxspeedFromRoadInfo(roadInfo1->GetValue(),
                                             roadInfo2->GetValue());
        if (tempResult < result) {
          result = tempResult;
        }
      }
      atuple->DeleteIfAllowed();
    }
  }
  tuple->DeleteIfAllowed();
  ntuple->DeleteIfAllowed();
  return result;
}

const int MaxspeedRaster::getMaxspeed(const RasterIndex<2> pos) {
  Rectangle<2> cell = grid.getCell(pos);
  R_TreeLeafEntry<2, TupleId> leaf;
  int result = SPEED_MAX, tempResult = SPEED_MAX;
  if (rtree->First(cell, leaf)) {
    tempResult = getMaxspeedFromLeaf(leaf.info);
    if (tempResult < result) {
      result = tempResult;
    }
  }
  else {
    return 0; // occurs if no road is found; other values are conceivable here
  }
  while (rtree->Next(leaf)) {
    tempResult = getMaxspeedFromLeaf(leaf.info);
    if (tempResult < result) {
      result = tempResult;
    }
  }
  return result;
}
  
std::ostream& operator<<(std::ostream& o, const Tile tile) {
  if (!tile.path.empty()) {
    o << "<";
  }
  for (unsigned int i = 1; i < tile.path.size(); i++) {
    o << "(" << tile.path[i-1].first << ", " << tile.path[i-1].second << "), ";
  }
  if (!tile.path.empty()) {
    o << "(" << tile.path[tile.path.size() - 1].first << ", " 
      << tile.path[tile.path.size() - 1].second << ")> ";
  }
  o << "(" << tile.x << ", " << tile.y << ")";
  return o;
}

/*
\section{Implementation of class ~Tileareas~}

*/
Tileareas::Tileareas(raster2::sint *_raster) {
  retrieveAreas(_raster);
}

Tileareas::Tileareas(const Tileareas& _src) : raster(_src.raster),
  areas(_src.areas), rtree(_src.rtree), transitions(_src.transitions) {}

void Tileareas::processTile(vector<vector<bool> >& visited, 
                   const unsigned int x, const unsigned int y, int& tileValue) {
  if (visited[x][y]) {
    return;
  }
  int rasterPos[2];
  rasterPos[0] = x;
  rasterPos[1] = y;
  raster2::sint::index_type rasterIndex(rasterPos);
  visited[x][y] = true;
  if (tileValue == raster->get(rasterIndex)) { // same value as previous tile
    // insert tile into current set of tiles
    processTile(visited, x - 1, y,     tileValue);
    processTile(visited, x,     y - 1, tileValue);
    processTile(visited, x + 1, y,     tileValue);
    processTile(visited, x,     y + 1, tileValue);
  }
}

void Tileareas::retrieveAreas(raster2::sint *_raster) {
  raster = _raster;
  raster2::grid2 grid = raster->getGrid();
  double length(grid.getLength());
  Rectangle<2> bbox = raster->bbox();
  double minX(bbox.MinD(0)), maxX(bbox.MaxD(0)), 
         minY(bbox.MinD(1)), maxY(bbox.MaxD(1));
  unsigned int noTilesX = (unsigned int)(ceil((maxX - minX) / length)),
               noTilesY = (unsigned int)(ceil((maxY - minY) / length));
  vector<vector<bool> > visited(noTilesX, vector<bool>(noTilesY, false));
  int rasterPos[2];
  rasterPos[0] = 0;
  rasterPos[1] = 0;
  raster2::sint::index_type rasterIndex(rasterPos);
  int tileValue = raster->get(rasterIndex);
  processTile(visited, 0, 0, tileValue); // start with 1st value
  visited[0][0] = true;
  for (unsigned int i = 0; i < noTilesX; i++) {
    for (unsigned int j = 0; j < noTilesY; j++) {
      processTile(visited, i, j, tileValue);
    }
  }
}

ListExpr Tileareas::Property() {
  return ( nl->TwoElemList (
      nl->FourElemList (
        nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List")),
      nl->FourElemList (
        nl->StringAtom("-> SIMPLE"),
        nl->StringAtom(Tileareas::BasicType()),
        nl->StringAtom("No list representation"),
        nl->StringAtom("No example available"))));
}

ListExpr Tileareas::Out(ListExpr typeInfo, Word value) {
  return nl->SymbolAtom("No list representation");
}

Word Tileareas::In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word res((void*)0);
  correct = true;
  return res;
}

Word Tileareas::Create(const ListExpr typeInfo) {
  Word w;
  w.addr = (new Tileareas(0));
  return w;
}

void Tileareas::Delete(const ListExpr typeInfo, Word& w) {
  Tileareas *t = (Tileareas*)w.addr;
  delete t;
  w.addr = 0;
}

bool Tileareas::Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo, Word& value) {
  bool ok = true;
  return ok;
}

bool Tileareas::Save(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo, Word& value) {
  bool ok = true;
  return ok;
}

void Tileareas::Close(const ListExpr typeInfo, Word& w) {
  Tileareas *t = (Tileareas*)w.addr;
  delete t;
  w.addr = 0;
}

Word Tileareas::Clone(const ListExpr typeInfo, const Word& w) {
  Tileareas *t = (Tileareas*)w.addr;
  Word res;
  res.addr = new Tileareas(*t);
  return res;
}

bool Tileareas::TypeCheck(ListExpr typeList, ListExpr& errorInfo) {
  return nl->IsEqual(typeList, BasicType());
}

int Tileareas::SizeOfObj() {
  return INT_MAX;
}

/*
\subsection{Type Constructor}

*/
TypeConstructor tileareasTC(
  Tileareas::BasicType(), Tileareas::Property,
  Tileareas::Out, Tileareas::In,
  0, 0,
  Tileareas::Create, Tileareas::Delete,
  Tileareas::Open, Tileareas::Save,
  Tileareas::Close, Tileareas::Clone,
  0,
  Tileareas::SizeOfObj,
  Tileareas::TypeCheck);

/*
\section{Implementation of class ~RestoreTrajLI~}

Applied for the operator ~restoreTraj~.

*/
RestoreTrajLI::RestoreTrajLI(Relation *e, BTree *ht, RTree2TID *st, 
 raster2::sint *r, Hash *rh, raster2::sint *mr, MLabel *h, MLabel *d, MLabel *s)
    : edgesRel(e), heightBtree(ht), segmentsRtree(st), raster(r), rhash(rh),
      maxspeedRaster(mr), height(h), direction(d), speed(s) {
//   RefinementPartition<MLabel, MLabel, ULabel, ULabel> rp(*h, *d);
  set<Tile> tiles;
  retrieveTilesFromHeight(0, tiles);
  cout << "INITIALIZATION: " << tiles.size() << " tiles" << endl;
  int counter = 0;
  for (set<Tile>::iterator it = tiles.begin(); it != tiles.end(); it++) {
    if (counter % 1000 == 0) {
      cout << counter << " : " << *it << endl;
    }
    counter++;
  }
  cout << endl << endl;
  int pos = 0;
  bool sequelFound = !tiles.empty();
  while (pos < height->GetNoComponents() - 1 && sequelFound) {
    sequelFound = retrieveSequel(pos, tiles);
    pos++;
    counter = 0;
    cout << "loop# " << pos << "; " << tiles.size() << " tiles updated" << endl;
    for (set<Tile>::iterator it = tiles.begin(); it != tiles.end(); it++) {
      if (counter % 100 == 0) {
        cout << counter << " : " << *it << endl;;
      }
      counter++;
    }
    cout << endl << endl;
  }
  edgesRel = 0;
  speed = 0;
  heightBtree = 0;
  segmentsRtree = 0;
  
  

}

bool RestoreTrajLI::retrieveSequel(const int startPos, set<Tile>& tiles) {
  if (startPos < 0 || startPos + 1 >= height->GetNoComponents()) {
    return false;
  }
  string nextHeightLabel;
  SecInterval iv1(true), iv2(true);
  height->GetInterval(startPos, iv1);
  height->GetInterval(startPos + 1, iv2);
  if (iv1.end != iv2.start) { // intervals have to be adjacent
    return false;
  }
  height->GetValue(startPos + 1, nextHeightLabel); // next height value
  int nextHeightNum = stoi(nextHeightLabel.substr(0,nextHeightLabel.find("-")));
  set<Tile> newTiles;
  cout << "check neighbors for " << tiles.size() << " origins; height range is "
       << nextHeightLabel << " (" << nextHeightNum << "), instant " << iv2.start
       << endl;
  for (set<Tile>::iterator it = tiles.begin(); it != tiles.end(); it++) {
    processNeighbors(*it, iv2.start, nextHeightNum, newTiles);
  }
  tiles = newTiles;
  return !tiles.empty();
}

/*
\subsection{Function ~processNeighbors~}

For the tile ~origin~, each of its three neighbors in the direction assumed at
the instant ~inst~ and adjacent directions are inserted into ~result~, iff their
altitude and maximum speed match the symbolic trajectories.

*/
void RestoreTrajLI::processNeighbors(Tile origin, const Instant& inst,
                                     const int height, set<Tile>& result) {
  ILabel dirILabel(true), speedILabel(true);
  Label dirLabel(true);
  direction->AtInstant(inst, dirILabel);
  speed->AtInstant(inst, speedILabel);
  if (!dirILabel.IsDefined() || !speedILabel.IsDefined()) {
    return;
  }
  DirectionNum dirNum = dirLabelToNum(dirILabel.value);
  vector<Tile> neighbors;
  getNeighbors(origin, dirNum, neighbors);
  int currentSpeed = getSpeedFromLabel(speedILabel.value, false);
  int rasterPos[2];
  for (unsigned int i = 0; i < 3; i++) {
    rasterPos[0] = neighbors[i].x;
    rasterPos[1] = neighbors[i].y;
    raster2::sint::index_type rasterIndex(rasterPos);
    if (raster->get(rasterIndex) == height && 
        maxspeedRaster->get(rasterIndex) >= currentSpeed) {
      result.insert(neighbors[i]);
    }
  }
}

/*
\subsection{Function ~getNeighbors~}

For the tile ~origin~, its neighbors in the direction ~dir~ and the two
adjacent directions are returned.

*/
void RestoreTrajLI::getNeighbors(Tile origin, const DirectionNum dir,
                                 vector<Tile>& result) {
  Tile n1 = origin.moveTo(dir);
  Tile n2 = origin.moveTo(static_cast<DirectionNum>((dir + 7) % 8));
  Tile n3 = origin.moveTo(static_cast<DirectionNum>((dir + 1) % 8));
  result.push_back(n1);
  result.push_back(n2);
  result.push_back(n3);
}

/*
\subsection{Function ~retrieveTiles~}

First, we determine the speed at the start instant of the unit corresponding to
~pos~. Then all tiles with a lower or equal maximum speed are inserted into
~result~.

*/
void RestoreTrajLI::retrieveTilesFromHeight(const int pos, set<Tile>& result) {
  ULabel heightULabel(true);
  ILabel speedILabel(true);
  height->Get(pos, heightULabel);
  if (!heightULabel.IsDefined()) {
    return;
  }
  string heightStr = heightULabel.constValue.GetValue();
  int heightNum = stoi(heightStr.substr(0, heightStr.find("-")));
  CcInt *key = new CcInt(heightNum, true);
  HashIterator *hit = rhash->ExactMatch(key);
  Tile tile(0, 0);
  speed->AtInstant(heightULabel.timeInterval.start, speedILabel);
  if (!speedILabel.IsDefined()) {
    return;
  }
  int currentSpeed = getSpeedFromLabel(speedILabel.value, false);
  int rasterPos[2];
  while (hit->Next()) {
    tile.x = (int)hit->GetId();
    if (hit->Next()) {
      tile.y = (int)hit->GetId();
      rasterPos[0] = tile.x;
      rasterPos[1] = tile.y;
      raster2::sint::index_type rasterIndex(rasterPos);
      if (maxspeedRaster->get(rasterIndex) >= currentSpeed) {
        result.insert(tile);
      }
    }
  }
  key->DeleteIfAllowed();
}

const DirectionNum RestoreTrajLI::dirLabelToNum(const Label& dirLabel) {
  if (dirLabel == "East")      return EAST;
  if (dirLabel == "Northeast") return NORTHEAST;
  if (dirLabel == "North")     return NORTH;
  if (dirLabel == "Northwest") return NORTHWEST;
  if (dirLabel == "West")      return WEST;
  if (dirLabel == "Southwest") return SOUTHWEST;
  if (dirLabel == "South")     return SOUTH;
  if (dirLabel == "Southeast") return SOUTHEAST;
  return DIR_ERROR;
}

const int RestoreTrajLI::getDirectionDistance(const DirectionNum dir1,
                                              const DirectionNum dir2) {
  return min(abs(dir1 - dir2), abs(8 - abs(dir1 - dir2)));
}

const int RestoreTrajLI::getSpeedFromLabel(const Label& speedLabel, 
                                           const bool getMax) {
  if (speedLabel == "0-6")     return (getMax ? 6 : 0);
  if (speedLabel == "6-15")    return (getMax ? 15 : 6);
  if (speedLabel == "15-30")   return (getMax ? 30 : 15);
  if (speedLabel == "30-50")   return (getMax ? 50 : 30);
  if (speedLabel == "50-100")  return (getMax ? 100 : 50);
  if (speedLabel == "100-200") return (getMax ? 200 : 100);
  return SPEED_MAX;
}

MLabel* RestoreTrajLI::nextCandidate() {
  return 0;
}

}
