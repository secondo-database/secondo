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

Started November 2019, Fabio Vald\'{e}s

*/

#include "PatternMining.h"

using namespace std;
// using namespace datetime;
// using namespace temporalalgebra;

namespace stj {

void RelAgg::compute(Relation *rel, NewPair<int, int> indexes) {
  string label;
  for (int i = 0; i < rel->GetNoTuples(); i++) {
//     Tuple *tuple = rel->GetTuple(i, false);
//     MLabel *ml = tuple->GetAttribute(indexes.first);
//     for (int j = 0; j < ml->GetNoComponents(); j++) {
//       ml->GetBasic(j, label);
//       
//     }
  }
}

GetPatternsLI::GetPatternsLI(Relation *r, NewPair<int,int> i, double ms, int ma)
  : rel(r), indexes(i), minSupp(ms), minNoAtoms(ma) {
  agg.clear();
  agg.compute(rel, indexes);
}


Tuple* GetPatternsLI::getNextResult() {
  return 0; // TODO: a lot
}


}
