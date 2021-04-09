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
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[pow] [\verb+^+]

[1] Association Analysis Algebra Implementation

January 2021 - April 2021, P. Fedorow for bachelor thesis.

*/

#pragma once

#include "Algebras/Collection/IntSet.h"
#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "NestedList.h"
#include "Operator.h"
#include "Stream.h"

#include <set>
#include <string>
#include <vector>

namespace AssociationAnalysis {

// Local info class for the csvLoadTransactions operator.
class csvLoadTransactionsLI {
public:
  // Loads transactions from a csv.
  csvLoadTransactionsLI(std::string path);

  ~csvLoadTransactionsLI() { this->tupleType->DeleteIfAllowed(); }

  // Returns the next rule as a tuple.
  Tuple *getNext();

private:
  struct Transaction {
    int id;
    std::set<int> itemset;
  };

  // Used to generate a stream of the transactions.
  std::vector<Transaction>::const_iterator it;

  // Contains the loaded transactions.
  std::vector<Transaction> transactions;

  // Describes the resulting tuple type: tuple(Id: int, Itemset: intset).
  TupleType *tupleType;
};

// Type mapping for the csvLoadTransactions operator.
ListExpr csvLoadTransactionsTM(ListExpr args);

// Value mapping for the csvLoadTransactions operator.
int csvLoadTransactionsVM(Word *args, Word &result, int message, Word &local,
                          Supplier s);

// Operator info for the csvLoadTransactions operator.
struct csvLoadTransactionsInfo : OperatorInfo {
  csvLoadTransactionsInfo() : OperatorInfo() {
    this->name = "csvLoadTransactions";
    this->signature = "text -> stream(tuple(Id: int, Itemset: intset))";
    this->syntax = "csvLoadTransactions(_)";
    this->meaning = "Loads transactions from a transaction file. The parameter "
                    "is expected to be a path to the file. The expected "
                    "format is: one transaction per line, items are numbers "
                    "separated by a single space.";
    this->usesArgsInTypeMapping = true;
  }
};
} // namespace AssociationAnalysis
