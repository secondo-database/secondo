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

*/
#ifndef TEMPORAL_REASONER_H
#define TEMPORAL_REASONER_H

#include<queue>
#include<vector>
#include<set>
#include<ostream>
#include "NestedList.h"
extern NestedList *nl;

namespace STP{
enum PARelation{
  lss=0, leq=1, grt=2, geq=3, eql=4, neq=5, uni=6, inc=7, unknown=8};
class PointAlgebraReasoner
{
private:
  unsigned int n;
  PARelation plusTable[7][7], multTable[7][7];
  queue<pair<int, int> > Queue;
  vector<vector<PARelation> > Table;
  set<int> Intervals;  //CSP variables (i.e., variable indexes).
  PointAlgebraReasoner();
public:

  PointAlgebraReasoner(unsigned int _n);
  ~PointAlgebraReasoner();
  void Add(int i, int j, PARelation Rij);
  bool Close();
  bool Propagate(int i, int j);
  ostream& Print(ostream& os);
  vector<PARelation>& GetRelations(int varIndex);
  void Clear();
  ListExpr ExportToNestedList();
  bool ImportFromNestedList(ListExpr& args);
  bool ImportFromArray(int* args);
};
};
#endif //TEMPORAL_REASONER_H
