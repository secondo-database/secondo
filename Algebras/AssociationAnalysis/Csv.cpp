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

#include "Csv.h"

#include "Common.h"

#include "Algebras/Collection/CollectionAlgebra.h"
#include "Algebras/Collection/IntSet.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"

#include <fstream>
#include <iostream>
#include <utility>

namespace AssociationAnalysis {

// Loads transactions from a csv.
csvLoadTransactionsLI::csvLoadTransactionsLI(std::string path) {
  std::fstream file;
  file.open(path, std::ios::in);
  if (file.is_open()) {
    std::string line;
    int id = 0;
    while (std::getline(file, line)) {
      std::istringstream lineStream(line);
      Transaction transaction;
      std::string item;
      while (std::getline(lineStream, item, ' ')) {
        try {
          transaction.itemset.insert(std::stoi(item));
        } catch (const std::exception &e) {
          continue;
        }
      }
      if (!transaction.itemset.empty()) {
        transaction.id = id;
        id += 1;
        this->transactions.push_back(transaction);
      }
    }
  }

  this->it = this->transactions.cbegin();

  // Setup resulting tuple type.
  this->tupleType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(transactionsTupleType()));
}

// Returns the next transactions as a tuple.
Tuple *csvLoadTransactionsLI::getNext() {
  if (this->it != this->transactions.cend()) {
    auto &transaction = *this->it;
    auto tuple = new Tuple(this->tupleType);
    tuple->PutAttribute(0, new CcInt(transaction.id));
    tuple->PutAttribute(1, new collection::IntSet(transaction.itemset));
    this->it++;
    return tuple;
  } else {
    return nullptr;
  }
}

// Type mapping for the csvLoadTransactions operator.
ListExpr csvLoadTransactionsTM(ListExpr args) {
  NList type(args);

  if (type.length() == 1) {
    const NList &arg = type.elem(1);
    if (!arg.first().isSymbol(FText::BasicType())) {
      return NList::typeError("The path argument must be of type text.");
    }
  } else {
    return NList::typeError("1 argument expected but " +
                            std::to_string(type.length()) + " received.");
  }

  NList tupleType = NList(transactionsTupleType());
  return NList().streamOf(tupleType).listExpr();
}

// Value mapping for the csvLoadTransactions operator.
int csvLoadTransactionsVM(Word *args, Word &result, int message, Word &local,
                          Supplier s) {
  auto *li = (csvLoadTransactionsLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    std::string path = ((FText *)args[0].addr)->GetValue();
    local.addr = new csvLoadTransactionsLI(path);
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

ListExpr csvSaveTransactionsTM(ListExpr args) {
  NList type(args);

  NList attrs;
  if (type.length() == 3) {
    if (!type.elem(1).first().checkStreamTuple(attrs)) {
      return NList::typeError(
          "Argument number 1 must be of type stream(tuple(...)).");
    }
    if (!type.elem(2).isSymbol(1)) {
      return NList::typeError("Argument number 2 must name an attribute in the "
                              "tuple stream given as the first argument.");
    }
    if (!type.elem(3).first().isSymbol(FText::BasicType())) {
      return NList::typeError("Argument number 3 must be of type text.");
    }
  } else {
    return NList::typeError("3 arguments expected but " +
                            std::to_string(type.length()) + " received.");
  }

  std::string itemsetAttrName = type.elem(2).first().str();
  int itemsetAttr = -1;
  for (int i = 1; i <= (int)attrs.length(); i += 1) {
    NList attr = attrs.elem(i);
    if (attr.elem(1).isSymbol(itemsetAttrName)) {
      itemsetAttr = i;
    }
  }

  if (itemsetAttr == -1) {
    return NList::typeError("Argument number 2 must name an attribute in the "
                            "tuple stream given as the first argument.");
  }

  return NList(Symbols::APPEND(), NList().intAtom(itemsetAttr - 1).enclose(),
               NList().symbolAtom(CcBool::BasicType()))
      .listExpr();
}

int csvSaveTransactionsVM(Word *args, Word &result, int message, Word &local,
                          Supplier s) {
  result = qp->ResultStorage(s);
  auto success = (CcBool *)result.addr;
  auto transactions = new Stream<Tuple>(args[0]);
  std::string path = ((FText *)args[2].addr)->GetValue();
  int itemsetAttr = ((CcInt *)args[3].addr)->GetIntval();

  std::fstream file;
  file.open(path, std::ios::out);
  if (file.is_open()) {
    transactions->open();
    Tuple *t;
    while ((t = transactions->request())) {
      auto itemset = (collection::IntSet *)t->GetAttribute(itemsetAttr);
      for (std::size_t i = 0; i < itemset->getSize(); i += 1) {
        file << itemset->get(i);
        if (i + 1 != itemset->getSize()) {
          file << ' ';
        }
      }
      file << '\n';
    }
    transactions->close();
    success->Set(true, true);
  } else {
    success->Set(true, false);
  }
  return 0;
}

extendItemNamesLI::extendItemNamesLI(
    Stream<Tuple> *stream, const std::string &path,
    std::vector<std::pair<int, int>> attrMapping, ListExpr tupleType)
    : stream(stream), attrMapping(std::move(attrMapping)) {
  this->tupleType =
      new TupleType(SecondoSystem::GetCatalog()->NumericType(tupleType));
  this->stream->open();
  std::fstream file;
  file.open(path, std::ios::in);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      std::istringstream lineStream(line);
      std::string item, name;
      std::getline(lineStream, item, ',');
      std::getline(lineStream, name, '\n');
      try {
        this->nameMapping[std::stoi(item)] = name;
      } catch (std::exception &e) {
        continue;
      }
    }
  }
}

// Returns the next tuple.
Tuple *extendItemNamesLI::getNext() {
  Tuple *t;
  if ((t = this->stream->request())) {
    auto *nt = new Tuple(this->tupleType);
    for (int i = 0; i < t->GetNoAttributes(); i += 1) {
      nt->CopyAttribute(i, t, i);
    }
    ListExpr textSetType = SecondoSystem::GetCatalog()->NumericType(
        NList(NList().symbolAtom(Set::BasicType()),
              NList().symbolAtom(FText::BasicType()))
            .listExpr());
    for (auto [itemsetAttr, namesAttr] : this->attrMapping) {
      auto names = new collection::Collection(collection::CollectionType::set,
                                              textSetType, 10);
      names->SetDefined(true);
      auto itemset = (collection::IntSet *)t->GetAttribute(itemsetAttr);
      for (std::size_t i = 0; i < itemset->getSize(); i += 1) {
        int item = itemset->get(i);
        if (this->nameMapping.count(item) > 0) {
          names->Insert(new FText(true, this->nameMapping[item]), 1);
        }
      }
      nt->PutAttribute(namesAttr, names);
    }
    return nt;
  }
  return nullptr;
}

// Type mapping for the extendItemNames operator.
ListExpr extendItemNamesTM(ListExpr args) {
  NList type(args);

  NList attrs;
  NList newAttrs;
  if (type.length() == 3) {
    if (!type.elem(1).first().checkStreamTuple(attrs)) {
      return NList::typeError(
          "Argument number 1 must be of type stream(tuple(...)).");
    }
    newAttrs = attrs;
    if (!type.elem(2).first().isSymbol(FText::BasicType())) {
      return NList::typeError("Argument number 2 must be of type text.");
    }
    NList attrPairs = type.elem(3).first();
    for (std::size_t i = 1; i <= attrPairs.length(); i += 1) {
      if (attrPairs.elem(i).length() != 2) {
        return NList::typeError(
            "Argument number 3 must be a list of attribute pairs.");
      }
      std::string attrName = attrPairs.elem(i).second().str();
      bool attrFound = false;
      bool attrIsIntSet = false;
      bool attrConflict = false;
      for (std::size_t j = 1; j <= attrs.length(); j += 1) {
        NList attr = attrs.elem(j);
        if (attr.first().isSymbol(attrName)) {
          attrFound = true;
          attrIsIntSet =
              attr.second().isSymbol(collection::IntSet::BasicType());
        }
        if (attr.first().isSymbol(attrPairs.elem(i).first().str())) {
          attrConflict = true;
        }
      }
      if (attrFound) {
        if (!attrIsIntSet) {
          return NList::typeError(
              "Attribute " + attrName +
              " is not an intset in the given tuple stream.");
        }
      } else {
        return NList::typeError("Attribute " + attrName +
                                " not found in the given tuple stream.");
      }
      if (attrConflict) {
        return NList::typeError("Attribute " + attrPairs.elem(i).first().str() +
                                " already exists in the given tuple stream.");
      }
      newAttrs.append(NList(attrPairs.elem(i).first(),
                            NList(NList().symbolAtom(Set::BasicType()),
                                  NList().symbolAtom(FText::BasicType()))));
    }
  } else {
    return NList::typeError("3 arguments expected but " +
                            std::to_string(type.length()) + " received.");
  }

  return NList().streamOf(NList().tupleOf(newAttrs)).listExpr();
}

// Value mapping for the extendItemNames operator.
int extendItemNamesVM(Word *args, Word &result, int message, Word &local,
                      Supplier s) {
  auto *li = (extendItemNamesLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    auto *stream = new Stream<Tuple>(args[0]);
    std::string path = ((FText *)args[1].addr)->GetValue();
    NList resultType(qp->GetType(s));
    NList tupleType = resultType.second();
    std::vector<std::pair<int, int>> attrMapping;
    NList attrPairs(qp->GetType(args[2].addr));
    for (std::size_t i = 1; i <= attrPairs.length(); i += 1) {
      std::string namesAttrName = attrPairs.elem(i).first().str();
      std::string itemsetAttrName = attrPairs.elem(i).second().str();
      int namesAttr = -1;
      int itemsetAttr = -1;
      for (int j = 1; j <= (int)tupleType.second().length(); j += 1) {
        if (tupleType.second().elem(j).first() == namesAttrName) {
          namesAttr = j;
        }
        if (tupleType.second().elem(j).first() == itemsetAttrName) {
          itemsetAttr = j;
        }
      }
      assert(namesAttr != -1 && itemsetAttr != -1);
      attrMapping.emplace_back(itemsetAttr - 1, namesAttr - 1);
    };
    local.addr =
        new extendItemNamesLI(stream, path, attrMapping, tupleType.listExpr());
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
} // namespace AssociationAnalysis
