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

#include "Algorithms.h"
#include "../Raster2/sint.h"
#include "HashAlgebra.h"

namespace stj {

/*
\section{class ~RestoreTrajLI~}

Applied for the operator ~restoreTraj~.

*/
class RestoreTrajLI {
 public:
  enum DirectionNum {ERROR = -1, EAST, NORTHEAST, NORTH, NORTHWEST, WEST,
                     SOUTHWEST, SOUTH, SOUTHEAST};
  typedef NewPair<int, int> Tile;
  typedef NewPair<NewPair<int, int>, DirectionNum> TileTransition;
   
  RestoreTrajLI(Relation *e, BTree *ht, RTree2TID *st, raster2::sint *r,
                Hash *rh, MLabel *h, MLabel *d, MLabel *s);
  
  RestoreTrajLI() {}
  
  void exchangeTiles(const std::vector<TileTransition>& transitions,
                     std::vector<Tile>& result);
  bool retrieveTransitions(const int startPos, std::vector<Tile>& origins,
                           std::vector<TileTransition>& result);
  bool checkNeighbor(int x, int y, const Instant& inst,
                     const int height, TileTransition& result);
  void retrieveTiles(const int pos, std::vector<Tile>& result);
  void updateCoords(const DirectionNum dir, int& x, int& y);
  DirectionNum dirLabelToNum(const Label& dirLabel);
  MLabel* nextCandidate();
  
 private:
  Relation *edgesRel;
  BTree *heightBtree;
  RTree2TID *segmentsRtree;
  raster2::sint *raster;
  Hash *rhash;
  MLabel *height;
  MLabel *direction;
  MLabel *speed;
  
//   std::vector<std::vector<NewPair<int> > > tileSequences;
};

}
