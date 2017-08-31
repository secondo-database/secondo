
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

namespace stj {

/*
\section{Implementation of class ~RestoreTrajLI~}

Applied for the operator ~restoreTraj~.

*/
RestoreTrajLI::RestoreTrajLI(Relation *e, BTree *ht, RTree2TID *st, 
                  raster2::sint *r, Hash *rh, MLabel *h, MLabel *d, MLabel *s) :
    edgesRel(e), heightBtree(ht), segmentsRtree(st), raster(r), rhash(rh),
    height(h), direction(d), speed(s) {
  RefinementPartition<MLabel, MLabel, ULabel, ULabel> rp(*h, *d);
  vector<Tile> tiles;
  retrieveTiles(0, tiles);
  cout << "tiles.size() == " << tiles.size() << endl;
  vector<TileTransition> tileTransitions;
  retrieveTransitions(0, tiles, tileTransitions);
  exchangeTiles(tileTransitions, tiles);
  cout << tiles.size() << " updated" << endl;
  
  
  for (int i = 1; i < 2 /* height->GetNoComponents() */; i++) {
    
    
    
  }
  
  
  edgesRel = 0;
  speed = 0;
  heightBtree = 0;
  segmentsRtree = 0;
  
  
//   for (int i = 0; i < tiles.size(); i++) {
//     cout << tiles[i] << endl;
//   }
}

void RestoreTrajLI::exchangeTiles(const vector<TileTransition>& transitions,
                                  vector<Tile>& result) {
  result.clear();
  for (unsigned int i = 0; i < transitions.size(); i++) {
    result.push_back(transitions[i].first);
  }
}

bool RestoreTrajLI::retrieveTransitions(const int startPos, 
                        vector<Tile>& origins, vector<TileTransition>& result) {
  if (startPos < 0 || startPos + 1 >= height->GetNoComponents()) {
    return false;
  }
  string nextHeightLabel;
  SecInterval iv1(true), iv2(true);
  height->GetInterval(startPos, iv1);
  height->GetInterval(startPos + 1, iv2);
  if (iv1.end != iv2.start) {
    return false;
  }
  height->GetValue(startPos + 1, nextHeightLabel); // next height value
  int nextHeightNum = stoi(nextHeightLabel.substr(0,nextHeightLabel.find("-")));
  TileTransition transition;
  int counter = 0;
  for (unsigned int i = 0; i < origins.size(); i++) {
    if (checkNeighbor(origins[i].first, origins[i].second, iv2.start, 
                      nextHeightNum, transition)) {
      result.push_back(transition);
      counter++;
    }
  }
  cout << counter << endl;
  return true;
}

bool RestoreTrajLI::checkNeighbor(int x, int y, const Instant& inst,
                                  const int height, TileTransition& result) {
  ILabel iDir(true);
  Label dirLabel(true);
  direction->AtInstant(inst, iDir);
  iDir.Val(dirLabel);
  DirectionNum dirNum = dirLabelToNum(dirLabel);
  updateCoords(dirNum, x, y);
  int rasterPos[2];
  rasterPos[0] = x;
  rasterPos[1] = y;
  raster2::sint::index_type rasterIndex(rasterPos);
  if (raster->get(rasterIndex) == height) {
    cout << "transition (" << x << ", " << y << ")   " << height << endl;
    return true;
  }
  return false;
}

void RestoreTrajLI::retrieveTiles(const int pos, vector<Tile>& result) {
  string heightLabel;
  height->GetValue(pos, heightLabel);
  int heightNum = stoi(heightLabel.substr(0, heightLabel.find("-")));
  CcInt *key = new CcInt(heightNum, true);
  HashIterator *hit = rhash->ExactMatch(key);
  NewPair<int, int> tileCoords;
  while (hit->Next()) {
    tileCoords.first = (int)hit->GetId();
    if (hit->Next()) {
      tileCoords.second = (int)hit->GetId();
    }
    result.push_back(tileCoords);
  }
  key->DeleteIfAllowed();
  result.shrink_to_fit();
}

void RestoreTrajLI::updateCoords(const DirectionNum dir, int& x, int& y) {
  switch (dir) {
    case EAST: {
      x++;
      break;
    }
    case NORTHEAST: {
      x++;
      y--;
      break;
    }
    case NORTH: {
      y--;
      break;
    }
    case NORTHWEST: {
      x--;
      y--;
      break;
    }
    case WEST: {
      x--;
      break;
    }
    case SOUTHWEST: {
      x--;
      y++;
      break;
    }
    case SOUTH: {
      y++;
      break;
    }
    case SOUTHEAST: {
      x++;
      y++;
      break;
    }
    default: {
      break;
    }
  }
}

RestoreTrajLI::DirectionNum RestoreTrajLI::dirLabelToNum(const Label& dirLabel){
  if (dirLabel == "East")      return EAST;
  if (dirLabel == "Northeast") return NORTHEAST;
  if (dirLabel == "North")     return NORTH;
  if (dirLabel == "Northwest") return NORTHWEST;
  if (dirLabel == "West")      return WEST;
  if (dirLabel == "Southwest") return SOUTHWEST;
  if (dirLabel == "South")     return SOUTH;
  if (dirLabel == "Southeast") return SOUTHEAST;
  return ERROR;
}

MLabel* RestoreTrajLI::nextCandidate() {
  return 0;
}

}
