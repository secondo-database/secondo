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

#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "NestedList.h"
#include "Operator.h"

#include <random>
#include <vector>

namespace AssociationAnalysis {

ListExpr genTransactionsTupleType();

ListExpr genTransactionsTM(ListExpr args);

class getTransactionsLI {
public:
  getTransactionsLI(int numOfTransactions, int transactionSizeMean,
                    int frequentItemsetSizeMean, int numOfFrequentItemsets,
                    int numOfItems);

  ~getTransactionsLI() { this->tupleType->DeleteIfAllowed(); }

  Tuple *getNext();

private:
  int numOfTransactions;
  int t;
  std::mt19937 gen;
  std::poisson_distribution<int> genTransactionSize;
  std::discrete_distribution<int> genPotentialFrequentItemset;
  std::vector<std::vector<int>> potentialFrequentItemsets;
  std::vector<double> corruptionLevels;
  bool allowOversizedTransaction;
  TupleType *tupleType;
};

int genTransactionsVM(Word *args, Word &result, int message, Word &local,
                      Supplier s);

struct genTransactionsInfo : OperatorInfo {
  genTransactionsInfo() : OperatorInfo() {
    this->name = "genTransactions";
    this->signature =
        "int int int int int -> stream(tuple([Id: int, Itemset: intset]))";
    this->syntax = "genTransaction(_, _, _, _, _)";
    this->meaning =
        "Generates a stream of transactions. The expected arguments are: the "
        "number of transactions, the mean of the transaction size, the mean of "
        "the frequent itemset size, the number of frequent itemsets and the "
        "number of items.";
  }
};
} // namespace AssociationAnalysis
