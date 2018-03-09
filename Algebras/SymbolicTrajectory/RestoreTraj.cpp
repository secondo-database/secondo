
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
Tileareas::Tileareas(const Tileareas& _src) : 
  raster(_src.raster), minX(_src.minX), maxX(_src.maxX), minY(_src.minY),
  maxY(_src.maxY), noTransitions(_src.noTransitions), areas(_src.areas), 
  tileToArea(_src.tileToArea), transitions(_src.transitions),
  areaFile(_src.areaFile), ttaFile(_src.ttaFile), transFile(_src.transFile) {}

Tileareas::~Tileareas() {
  if (areaFile.IsOpen()) {
    areaFile.Close();
  }
  if (ttaFile.IsOpen()) {
    ttaFile.Close();
  }
  if (transFile.IsOpen()) {
    transFile.Close();
  }
}

void Tileareas::processTile(const int x, const int y, int& value) {
  int rasterPos[2];
  rasterPos[0] = x;
  rasterPos[1] = y;
  raster2::sint::index_type rasterIndex(rasterPos);
  int newValue = raster->get(rasterIndex);
  if (newValue == INT_MIN) {
    value = newValue;
    return;
  }
  set<NewPair<int, int> > newArea;
  rasterPos[0] = x - 1;
  raster2::sint::index_type leftRasterIndex(rasterPos);
  int leftValue = raster->get(leftRasterIndex);
  if (newValue == leftValue && y > minY && newValue == value) {//unite areas
    int leftAreaNo = tileToArea.get(x - 1, y);
    int lowerAreaNo = tileToArea.get(x, y - 1);
    if (leftAreaNo == lowerAreaNo) { // both tiles already belong to same area
      tileToArea.set(x, y, leftAreaNo);
      areas[leftAreaNo].insert(NewPair<int, int>(x, y));
    }
    else { // move smaller area into larger area
      int sourceAreaNo = (areas[leftAreaNo].size() < areas[lowerAreaNo].size()
                          ? leftAreaNo : lowerAreaNo);
      int destAreaNo = (sourceAreaNo == leftAreaNo ? lowerAreaNo : leftAreaNo);
//       if (x == -1181 && y == 1549) {
//         cout << sourceAreaNo << " " << destAreaNo << " | " 
//              << areas[sourceAreaNo].size() << " " << areas[destAreaNo].size()
//              << " | ";
//       }
      areas[destAreaNo].insert(NewPair<int, int>(x, y));
      tileToArea.set(x, y, destAreaNo);
      areas[destAreaNo].insert(areas[sourceAreaNo].begin(), 
                               areas[sourceAreaNo].end());
      for (set<NewPair<int, int> >::iterator it = areas[sourceAreaNo].begin();
           it != areas[sourceAreaNo].end(); it++) {
        tileToArea.set(it->first, it->second, destAreaNo);
      }
      areas[sourceAreaNo].clear();
//       if (x == -640 && y == 1815) {
//         cout << areas[sourceAreaNo].size() << " " << areas[destAreaNo].size()
//              << endl;
//       }
    } 
  }
  else if (newValue == leftValue) { // extend area of left neighbor
    int leftAreaNo = tileToArea.get(x - 1, y);
    tileToArea.set(x, y, leftAreaNo);
    areas[leftAreaNo].insert(NewPair<int, int>(x, y));
  }
  else if (y > minY && newValue == value) { // extend area of lower neighbor
    int lowerAreaNo = tileToArea.get(x, y - 1);
    tileToArea.set(x, y, lowerAreaNo);
    areas[lowerAreaNo].insert(NewPair<int, int>(x, y));
  }
  else { // create new area
    newArea.insert(NewPair<int, int>(x, y));
    tileToArea.set(x, y, areas.size());
    areas.push_back(newArea);
  }
  value = newValue;
}

void Tileareas::trimAreaVector() {
  if (areas.empty()) {
    return;
  }
  int last = areas.size() - 1;
  for (int i = 0; i < (int)areas.size(); i++) {
    if (areas[i].empty()) {
      while (areas[last].empty() && last > i) {
        areas.pop_back();
        last--;
      }
      for (set<NewPair<int, int> >::iterator it = areas[last].begin();
           it != areas[last].end(); it++) {
        areas[i].insert(areas[i].end(), *it);
        tileToArea.set(it->first, it->second, i);
      }
      last--;
      areas.pop_back();
    }
  }
}

void Tileareas::recordAreaTransitions(const int x, const int y) {
//   cout << "rAT(" << x << ", " << y << ")" << endl;
  int rasterPos[2], rasterNeighbor[2];
  rasterPos[0] = x;
  rasterPos[1] = y;
  raster2::sint::index_type rasterIndex(rasterPos);
  int value = raster->get(rasterIndex);
  if (value == INT_MIN) {
    return;
  }
  int areaNo = tileToArea.get(x, y);
  int neighborAreaNo = -1;
  Tile tile(x, y);
  for (int dir = 0; dir <= 7; dir++) {
    DirectionNum dirNum = static_cast<DirectionNum>(dir);
    Tile neighborTile = tile.moveTo(dirNum);
    if (belongsToRaster(neighborTile.x, neighborTile.y)) {
      rasterNeighbor[0] = neighborTile.x;
      rasterNeighbor[1] = neighborTile.y;
      raster2::sint::index_type rasterIndexNeighbor(rasterNeighbor);
      int neighborValue = raster->get(rasterIndexNeighbor);
      if (value != neighborValue && neighborValue != INT_MIN) {
        neighborAreaNo = tileToArea.get(neighborTile.x, neighborTile.y);
        transitions[areaNo][dir].insert(neighborAreaNo);
        noTransitions++;
      }
    }
  }
}

void Tileareas::retrieveAreas(raster2::sint *_raster) {
  raster = _raster;
  raster2::grid2 grid = raster->getGrid();
  Rectangle<2> bbox = raster->bbox();
  raster2::RasterIndex<2> minIndex = grid.getIndex(bbox.MinD(0), bbox.MinD(1));
  raster2::RasterIndex<2> maxIndex = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));
  minX = minIndex[0];
  maxX = maxIndex[0];
  minY = minIndex[1];
  maxY = maxIndex[1];
  areas.clear();
  tileToArea.initialize(minX, maxX, minY, maxY, -1);
  int tileValue = INT_MIN;
  for (int i = minX; i <= maxX; i++) {
    for (int j = minY; j <= maxY; j++) {
      processTile(i, j, tileValue);
    }
  }
  trimAreaVector();
  transitions.resize(areas.size(), std::vector<std::set<int> >(8));
  cout << areas.size() << " areas successfully retrieved" << endl;
  for (int i = minX; i <= maxX; i++) {
    for (int j = minY; j <= maxY; j++) {
      recordAreaTransitions(i, j);
    }
  }
  raster = 0;
  cout << transitions.size() << " transitions found" << endl;
}

void Tileareas::print(const bool printRange, const bool printAreas,
                      const bool printTileToArea, const bool printTransitions) {
  if (printRange) {
    cout << "Tile range: " << "(" << minX << ", " << minY << ") -- (" << maxX 
         << ", " << maxY << ")" << endl;
  }
  if (printAreas) {
    cout << areas.size() << " areas." << endl;
    for (unsigned int i = 0; i < areas.size(); i++) {
       cout << "  area # " << i << ": ";
       for (set<NewPair<int, int> >::iterator it = areas[i].begin();
            it != areas[i].end(); it++) {
         cout << "(" << it->first << ", " << it->second << ")   ";
       }
       cout << endl;
    }
  }
  if (printTileToArea) {
    cout << "tileToArea:" << endl;
    for (int i = minX; i <= maxX; i++) {
      for (int j = minY; j <= maxY; j++) {
        cout << "(" << i << ", " << j << ") --> " << tileToArea.get(i, j) 
             << "    ";
      }
    }
    cout << endl;
  }
  if (printTransitions) {
    cout << noTransitions << " transitions:" << endl;
    for (unsigned int i = 0; i < transitions.size(); i++) {
      for (int j = 0; j < 8; j++) {
        if (!transitions[i][j].empty()) {
          cout << i << " --" 
               << RestoreTrajLI::dirNumToString(static_cast<DirectionNum>(j))
               << "-> {";
            for (set<int>::iterator it = transitions[i][j].begin(); 
                 it != transitions[i][j].end(); it++) {
              cout << *it << ", ";
            }
          cout << "}    ";
        }
      }
    }
    cout << endl;
  }
}

void Tileareas::deleteFiles() {
  areaFile.Close();
  areaFile.Drop();
  ttaFile.Close();
  ttaFile.Drop();
  transFile.Close();
  transFile.Drop();
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
//   t->deleteFiles();
  delete t;
  w.addr = 0;
}

bool Tileareas::Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo, Word& value) {
  Tileareas *ta = new Tileareas(true);
  unsigned int noAreas, noTiles, pos(0);
  char *buffer = 0;
  int areaNo;
  SmiRecord aRecord, ttaRecord, trRecord;
  SmiRecordId aRecordId, ttaRecordId, trRecordId;
  SmiFileId aFileId, ttaFileId, trFileId;
  // load min, max, fileIds, recordIds
  if (!valueRecord.Read(&(ta->minX), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  if (!valueRecord.Read(&(ta->maxX), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  if (!valueRecord.Read(&(ta->minY), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  if (!valueRecord.Read(&(ta->maxY), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  if (!valueRecord.Read(&aRecordId, sizeof(SmiRecordId), offset)) {
    return false;
  }
  offset += sizeof(SmiRecordId);
  if (!valueRecord.Read(&aFileId, sizeof(SmiFileId), offset)) {
    return false;
  }
  offset += sizeof(SmiFileId);
  if (!valueRecord.Read(&ttaRecordId, sizeof(SmiRecordId), offset)) {
    return false;
  }
  offset += sizeof(SmiRecordId);
  if (!valueRecord.Read(&ttaFileId, sizeof(SmiFileId), offset)) {
    return false;
  }
  offset += sizeof(SmiFileId);
  if (!valueRecord.Read(&trRecordId, sizeof(SmiRecordId), offset)) {
    return false;
  }
  offset += sizeof(SmiRecordId);
  if (!valueRecord.Read(&trFileId, sizeof(SmiFileId), offset)) {
    return false;
  }
  offset += sizeof(SmiFileId);
  ta->areaFile.Open(aFileId);
  ta->ttaFile.Open(ttaFileId);
  ta->transFile.Open(trFileId);
  // load areas
  ta->areaFile.SelectRecord(aRecordId, aRecord);
  buffer = new char[aRecord.Size()];
  if (aRecord.Read(buffer, aRecord.Size()) == 0) {
    return false;
  }
  memcpy(&noAreas, buffer + pos, sizeof(unsigned int));
  pos += sizeof(unsigned int);
  for (unsigned int i = 0; i < noAreas; i++) {
    memcpy(&noTiles, buffer + pos, sizeof(unsigned int));
    pos += sizeof(unsigned int);
    set<NewPair<int, int> > area;
    NewPair<int, int> tile;
    for (unsigned int j = 0; j < noTiles; j++) {
      memcpy(&tile, buffer + pos, sizeof(NewPair<int, int>));
      pos += sizeof(NewPair<int, int>);
      area.insert(tile);
    }
    ta->areas.push_back(area);
  }
  delete[] buffer;
  pos = 0;
  // load tileToArea
  ta->tileToArea.initialize(ta->minX, ta->maxX, ta->minY, ta->maxY, -1);
  ta->ttaFile.SelectRecord(ttaRecordId, ttaRecord);
  buffer = new char[ttaRecord.Size()];
  if (ttaRecord.Read(buffer, ttaRecord.Size()) == 0) {
    return false;
  }  
  for (int i = ta->minX; i <= ta->maxX; i++) {
    for (int j = ta->minY; j <= ta->maxY; j++) {
      memcpy(&areaNo, buffer + pos, sizeof(int));
      pos += sizeof(int);
      ta->tileToArea.set(i, j, areaNo);
    }
  }
  delete[] buffer;
  pos = 0;
  // load transitions
  ta->transitions.resize(ta->areas.size(), std::vector<std::set<int> >(8));
  ta->transFile.SelectRecord(trRecordId, trRecord);
  buffer = new char[trRecord.Size()];
  if (trRecord.Read(buffer, trRecord.Size()) == 0) {
    return false;
  }
  int dir(0);
  unsigned int sourceAreaNo = 0;
  vector<set<int> > transitionSets;
  set<int> newAreas;
  while (pos < trRecord.Size()) {
    memcpy(&areaNo, buffer + pos, sizeof(int));
    pos += sizeof(int);
    if (areaNo == INT_MAX) { // symbol for next direction / next source area
      transitionSets.push_back(newAreas);
      newAreas.clear();
      dir++;
      if (dir == 8) { // proceed to next source area and first direction
        ta->transitions[sourceAreaNo] = transitionSets;
        sourceAreaNo++;
        transitionSets.clear();
        dir = 0;
      }
    }
    else { // extend current set of target areas
      newAreas.insert(areaNo);
      ta->noTransitions++;
    }
  }
  delete[] buffer;
//   cout << *(ta->areas[2].begin()) << " | " << ta->tileToArea.get(-653, 1820)
//        << " | " << *(ta->transitions[2][0].begin()) << "  "
//        << *(ta->transitions[2][4].begin()) << endl;
  value.addr = ta;
  return true;
}

bool Tileareas::Save(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo, Word& value) {
  Tileareas *ta = static_cast<Tileareas*>(value.addr);
  SmiRecord aRecord, ttaRecord, trRecord;
  SmiRecordId aRecordId, ttaRecordId, trRecordId;
//   cout << *(ta->areas[2].begin()) << " | " << ta->tileToArea.get(-650, 1820)
//        << " | " << *(ta->transitions[2][0].begin()) << "  "
//        << *(ta->transitions[2][4].begin()) << endl;
  ta->areaFile.Create();
  ta->ttaFile.Create();
  ta->transFile.Create();
  ta->print(true, false, false, false);
  // store min, max
  if (!valueRecord.Write(&(ta->minX), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  if (!valueRecord.Write(&(ta->maxX), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  if (!valueRecord.Write(&(ta->minY), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  if (!valueRecord.Write(&(ta->maxY), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  // store areas
  size_t bufferSize = (ta->areas.size() + 1) * sizeof(unsigned int)
      + (ta->maxX - ta->minX + 1) * (ta->maxY - ta->minY + 1) * 2 * sizeof(int);
  char* buffer = new char[bufferSize];
  size_t pos = 0;
  int maxint = INT_MAX;
  unsigned int noAreas = ta->areas.size();
  cout << "begin storing " << noAreas << " areas" << endl;
  memcpy(buffer + pos, &noAreas, sizeof(unsigned int));
  pos += sizeof(unsigned int);
  for (unsigned int i = 0; i < ta->areas.size(); i++) {
    unsigned int noTiles = ta->areas[i].size();
    memcpy(buffer + pos, &noTiles, sizeof(unsigned int));
    pos += sizeof(unsigned int);
    for (set<NewPair<int, int> >::iterator it = ta->areas[i].begin(); 
         it != ta->areas[i].end(); it++) {
//       NewPair<int, int> tile(*it);
      memcpy(buffer + pos, &(*it), sizeof(NewPair<int, int>));
      pos += sizeof(NewPair<int, int>);
    }
//     if (i % 5000 == 0 && i > 0) {
//       cout << "  area # " << i << " stored..." << endl;
//     }
  }
  ta->areaFile.AppendRecord(aRecordId, aRecord);
  assert(aRecordId != 0);
  size_t bytesWritten = aRecord.Write(buffer, bufferSize);
  aRecord.Finish();
  SmiFileId aFileId = ta->areaFile.GetFileId();
  if (!valueRecord.Write(&aRecordId, sizeof(SmiRecordId), offset)) {
    return false;
  }
  offset += sizeof(SmiRecordId);
  if (!valueRecord.Write(&aFileId, sizeof(SmiFileId), offset)) {
    return false;
  }
  offset += sizeof(SmiFileId);
  cout << "......... all areas stored (" << bytesWritten << " bytes)" << endl;
  delete[] buffer;
  bufferSize = (ta->maxX - ta->minX+1) * (ta->maxY - ta->minY+1) * sizeof(int);
  buffer = new char[bufferSize];
  pos = 0;
  // store tileToArea
  cout << "begin storing tileToArea: " << ta->maxX - ta->minX + 1 << " columns"
       << endl;
  for (int i = ta->minX; i <= ta->maxX; i++) {
    for (int j = ta->minY; j <= ta->maxY; j++) {
      int areaNo = ta->tileToArea.get(i, j);
      memcpy(buffer + pos, &areaNo, sizeof(int));
      pos += sizeof(int);
    }
//     if ((i - ta->minX + 1) % 100 == 0) {
//       cout << "  " << i - ta->minX + 1 << " columns stored..." << endl;
//     }
  }
  ta->ttaFile.AppendRecord(ttaRecordId, ttaRecord);
  assert(ttaRecordId != 0);
  bytesWritten = ttaRecord.Write(buffer, bufferSize);
  ttaRecord.Finish();
  SmiFileId ttaFileId = ta->ttaFile.GetFileId();
  if (!valueRecord.Write(&ttaRecordId, sizeof(SmiRecordId), offset)) {
    return false;
  }
  offset += sizeof(SmiRecordId);
  if (!valueRecord.Write(&ttaFileId, sizeof(SmiFileId), offset)) {
    return false;
  }
  offset += sizeof(SmiFileId);
  cout << "......... tileToArea stored (" << bytesWritten << " bytes)" << endl;
  delete[] buffer;
  bufferSize = ta->noTransitions * sizeof(int) 
               + 8 * ta->transitions.size() * sizeof(int);
  buffer = new char[bufferSize];
  pos = 0;
  // store transitions
  cout << "begin storing " << ta->noTransitions << " transitions" << endl;
  for (unsigned int i = 0; i < ta->transitions.size(); i++) {
    for (int j = 0; j < 8; j++) {
      for (set<int>::iterator it = ta->transitions[i][j].begin();
           it != ta->transitions[i][j].end(); it++) {
        memcpy(buffer + pos, &(*it), sizeof(int));
        pos += sizeof(int);
      }
      memcpy(buffer + pos, &maxint, sizeof(int));
      pos += sizeof(int);
    }
  }
  ta->transFile.AppendRecord(trRecordId, trRecord);
  assert(trRecordId != 0);
  bytesWritten = trRecord.Write(buffer, bufferSize);
  trRecord.Finish();
  SmiFileId trFileId = ta->transFile.GetFileId();
  if (!valueRecord.Write(&trRecordId, sizeof(SmiRecordId), offset)) {
    return false;
  }
  offset += sizeof(SmiRecordId);
  if (!valueRecord.Write(&trFileId, sizeof(SmiFileId), offset)) {
    return false;
  }
  offset += sizeof(SmiFileId);
  cout << ".........transitions stored (" << bytesWritten << " bytes)" << endl;
  delete[] buffer;
  return true;
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

const string RestoreTrajLI::dirNumToString(const DirectionNum dirNum) {
  if (dirNum == EAST)      return "East";
  if (dirNum == NORTHEAST) return "Northeast";
  if (dirNum == NORTH)     return "North";
  if (dirNum == NORTHWEST) return "Northwest";
  if (dirNum == WEST)      return "West";
  if (dirNum == SOUTHWEST) return "Southwest";
  if (dirNum == SOUTH)     return "South";
  if (dirNum == SOUTHEAST) return "Southeast";
  return "Error";
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
