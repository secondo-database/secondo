/*
----
This file is part of SECONDO.

Copyright (C) 2021, University in Hagen, Department of Computer Science,
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

[1] Association Analysis Algebra Implementation

January 2021 - April 2021, P. Fedorow for bachelor thesis.

*/

#pragma once

#include "Algebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "NestedList.h"
#include "StandardTypes.h"

#include <cmath>

namespace AssociationAnalysis {
class frequentItemsetStreamLI {
public:
  explicit frequentItemsetStreamLI(
      std::vector<std::pair<std::vector<int>, double>> &&frequentItemsets);

  ~frequentItemsetStreamLI();

  // Returns the next frequent itemset as a tuple.
  Tuple *getNext();

private:
  // Contains the frequent itemsets annotated by their support.
  std::vector<std::pair<std::vector<int>, double>> frequentItemsets;

  // Used to generate a stream of the frequent itemsets.
  std::vector<std::pair<std::vector<int>, double>>::const_iterator it;

  // Describes the resulting tuple type: tuple(Itemset: intset, Support: real).
  TupleType *tupleType;
};

ListExpr frequentItemsetTupleType();

// Type mapping for a frequent itemset mining operator.
ListExpr mineTM(ListExpr args, ListExpr returnType);
ListExpr mineTM(ListExpr args);

// Value mapping for a frequent itemset mining operator.
template <class T>
int mineVM(Word *args, Word &result, int message, Word &local, Supplier s) {
  auto *li = (T *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    auto relation = (GenericRelation *)args[0].addr;
    bool relativeSupport = ((CcBool *)args[4].addr)->GetBoolval();
    int minSupport = 0;
    if (relativeSupport) {
      double support = ((CcReal *)args[2].addr)->GetRealval();
      minSupport = (int)(std::ceil(support * (double)relation->GetNoTuples()));
    } else {
      minSupport = ((CcInt *)args[2].addr)->GetIntval();
    }
    int attrIndex = ((CcInt *)args[3].addr)->GetIntval();
    local.addr = new T(relation, minSupport, attrIndex);
    return 0;
  }
  case REQUEST:
    result.addr = li ? li->getNext() : nullptr;
    return result.addr ? YIELD : CANCEL;
  case CLOSE:
    delete li;
    local.addr = nullptr;
    return 0;
  default:
    return 0;
  }
}

// Increments the integer that the given vector of booleans/bits represents.
// Returns true on overflow, false otherwise. This function is used as helper
// to built all subsets of an another vector of the same size.
bool increment(std::vector<bool> &bs);

} // namespace AssociationAnalysis