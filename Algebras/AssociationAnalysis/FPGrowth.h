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

#include "Algebras/Collection/IntSet.h"
#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "NestedList.h"
#include "Operator.h"

#include <utility>
#include <vector>

namespace AssociationAnalysis {

// Type mapping for the fpGrowth operator.
ListExpr fpGrowthTM(ListExpr args);

// Local info class for the fpGrowth operator. Contains the implementation of
// the fp-growth-algorithm.
class fpGrowthLI {
public:
  // Finds all frequent itemsets that satisfy the support given by minSupport.
  // The itemset of a transaction is extracted from each tuple of the relation
  // by an index given by itemsetAttr.
  fpGrowthLI(GenericRelation *relation, int minSupport, int itemsetAttr);

  ~fpGrowthLI() { this->tupleType->DeleteIfAllowed(); }

  // Returns the next frequent itemset as a tuple.
  Tuple *getNext();

private:
  // Used to generate a stream of the frequent itemsets.
  std::vector<std::pair<std::vector<int>, double>>::const_iterator it;

  // Contains the frequent itemsets annotated by their support.
  std::vector<std::pair<std::vector<int>, double>> frequentItemsets;

  // Describes the resulting tuple type: tuple(Itemset: intset, Support: real).
  TupleType *tupleType;
};

// Value mapping for the fpGrowth operator.
int fpGrowthVM(Word *args, Word &result, int message, Word &local, Supplier s);

// Operator info for the fpGrowth operator.
struct fpGrowthInfo : OperatorInfo {
  fpGrowthInfo() : OperatorInfo() {
    this->name = "fpGrowth";
    this->signature = "rel(tuple(...)) attr int -> stream(tuple(Itemset: "
                      "intset, Support: real))";
    this->syntax = "_ fpGrowth[_, _]";
    this->meaning = "Discovers the frequent itemsets in the given relation of "
                    "transactions by using the fp-growth-algorithm. The "
                    "expected arguments are: the relation that contains the "
                    "transactions, the name of the attribute that contains the "
                    "items as an intset and the minimum support to look for.";
    this->usesArgsInTypeMapping = true;
  }
};
} // namespace AssociationAnalysis
