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

#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "NestedList.h"
#include "Operator.h"
#include "Stream.h"

#include <memory>
#include <set>
#include <string>
#include <utility>
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
    this->meaning =
        "Loads transactions from a transaction file. The parameter is expected "
        "to be the path of the import file. The expected format is: one "
        "transaction per line, items are numbers separated by a single space.";
    this->usesArgsInTypeMapping = true;
  }
};

// Type mapping for the csvSaveTransactions operator.
ListExpr csvSaveTransactionsTM(ListExpr args);

// Value mapping for the csvSaveTransactions operator.
int csvSaveTransactionsVM(Word *args, Word &result, int message, Word &local,
                          Supplier s);

// Operator info for the csvSaveTransactions operator.
struct csvSaveTransactionsInfo : OperatorInfo {
  csvSaveTransactionsInfo() : OperatorInfo() {
    this->name = "csvSaveTransactions";
    this->signature = "stream(tuple(...)) text attr -> bool";
    this->syntax = "_ csvSaveTransactions[_, _]";
    this->meaning =
        "Saves transactions from a transaction file. The parameter is expected "
        "to be the path of the export file. The expected format is: one "
        "transaction per line, items are numbers separated by a single space.";
    this->usesArgsInTypeMapping = true;
  }
};

// Local info class for the extendItemNames operator.
class extendItemNamesLI {
public:
  // Extends the tuples of a given tuple stream with a new attribute. This
  // attribute assigns names to the itemset in the tuple.
  extendItemNamesLI(Stream<Tuple> *stream, const std::string &path,
                    std::vector<std::pair<int, int>> attrMapping,
                    ListExpr tupleType);

  ~extendItemNamesLI() {
    this->tupleType->DeleteIfAllowed();
    this->stream->close();
  }

  // Returns the next tuple.
  Tuple *getNext();

private:
  // Incoming stream.
  std::unique_ptr<Stream<Tuple>> stream;

  // Mapping item->name.
  std::unordered_map<int, std::string> nameMapping;

  // Mapping (itemset attr)->(name attr).
  std::vector<std::pair<int, int>> attrMapping;

  // Describes the resulting tuple type.
  TupleType *tupleType;
};

// Type mapping for the extendItemNames operator.
ListExpr extendItemNamesTM(ListExpr args);

// Value mapping for the extendItemNames operator.
int extendItemNamesVM(Word *args, Word &result, int message, Word &local,
                      Supplier s);

// Operator info for the extendItemNames operator.
struct extendItemNamesInfo : OperatorInfo {
  extendItemNamesInfo() : OperatorInfo() {
    this->name = "extendItemNames";
    this->signature = "stream(tuple(...)) text attr attr -> stream(tuple(...))";
    this->syntax = "_ extendItemNames[_, _, _]";
    this->meaning =
        "Extends the tuples of a given tuple stream with a new attribute. This "
        "attribute assigns names to the itemset in the tuple. The first "
        "parameter is expected to be a tuple stream. The names of the items "
        "are expected to be in a csv which path is given by the second "
        "parameter. The third parameter is the attribute name of the itemset. "
        "The fourth parameter is the name of the new attribute.";
    this->usesArgsInTypeMapping = true;
  }
};

} // namespace AssociationAnalysis
