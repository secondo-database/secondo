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

#include "Common.h"

#include "Algebras/Collection/IntSet.h"
#include "NList.h"
#include "StandardTypes.h"

namespace AssociationAnalysis {
frequentItemsetStreamLI::frequentItemsetStreamLI(
    std::vector<std::pair<std::vector<int>, double>> &&frequentItemsets)
    : frequentItemsets(frequentItemsets), it(this->frequentItemsets.cbegin()),
      tupleType(new TupleType(SecondoSystem::GetCatalog()->NumericType(
          frequentItemsetTupleType()))) {}

frequentItemsetStreamLI::~frequentItemsetStreamLI() {
  this->tupleType->DeleteIfAllowed();
}

Tuple *frequentItemsetStreamLI::getNext() {
  if (this->it != this->frequentItemsets.cend()) {
    auto &[itemset, support] = *this->it;
    auto tuple = new Tuple(this->tupleType);
    tuple->PutAttribute(0, new collection::IntSet(std::set<int>(
                               itemset.cbegin(), itemset.cend())));
    tuple->PutAttribute(1, new CcReal(support));
    this->it++;
    return tuple;
  } else {
    return nullptr;
  }
}

ListExpr frequentItemsetTupleType() {
  NList attrs =
      NList(NList(NList().symbolAtom("Itemset"),
                  NList().symbolAtom(collection::IntSet::BasicType())),
            NList(NList().symbolAtom("Support"),
                  NList().symbolAtom(CcReal::BasicType())));
  ListExpr type = NList().tupleOf(attrs).listExpr();
  return type;
}

ListExpr transactionsTupleType() {
  NList attrs = NList(
      NList(NList().symbolAtom("Id"), NList().symbolAtom(CcInt::BasicType())),
      NList(NList().symbolAtom("Itemset"),
            NList().symbolAtom(collection::IntSet::BasicType())));
  ListExpr type = NList().tupleOf(attrs).listExpr();
  return type;
}

// Type mapping for a frequent itemset mining operator.
ListExpr mineTM(ListExpr args, ListExpr returnType) {
  NList type(args);

  NList appendList;
  bool relativeSupport = false;
  NList attrs;
  if (type.length() == 3 || type.length() == 4) {
    if (!type.elem(1).first().checkRel(attrs)) {
      return NList::typeError(
          "Argument number 1 must be of type rel(tuple(...)).");
    }
    if (!type.elem(2).isSymbol(1)) {
      return NList::typeError("Argument number 2 must name an attribute in the "
                              "relation given as the first argument.");
    }
    if (type.elem(3).first().isSymbol(CcInt::BasicType())) {
      if (type.elem(3).second().intval() <= 0) {
        return NList::typeError("Argument number 3 must be of type int and > 0 "
                                "or of type real and in the interval (0, 1).");
      }
    } else if (type.elem(3).first().isSymbol(CcReal::BasicType())) {
      if (type.elem(3).second().realval() <= 0.0 ||
          type.elem(3).second().realval() >= 1.0) {
        return NList::typeError("Argument number 3 must be of type int and > 0 "
                                "or of type real and in the interval (0, 1).");
      } else {
        relativeSupport = true;
      }
    } else {
      return NList::typeError("Argument number 3 must be of type int and > 0 "
                              "or of type real and in the interval (0, 1).");
    }
    if (!type.elem(3).first().isSymbol(CcInt::BasicType()) ||
        type.elem(3).second().intval() <= 0) {
    }
    if (type.length() == 4) {
      if (!type.elem(4).first().isSymbol(CcInt::BasicType())) {
        return NList::typeError(
            "The optional argument number 4 must be of type int.");
      }
    } else {
      // Add default value via the append-functionality.
      appendList.append(NList().intAtom(0));
    }
  } else {
    return NList::typeError("3 or 4 arguments expected but " +
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
                            "relation given as the first argument.");
  }

  appendList.append(NList().intAtom(itemsetAttr - 1));
  appendList.append(NList().boolAtom(relativeSupport));

  return NList(Symbols::APPEND(), appendList, returnType).listExpr();
}

ListExpr mineTM(ListExpr args) {
  NList tupleType = NList(frequentItemsetTupleType());
  return mineTM(args, NList().streamOf(tupleType).listExpr());
}

// Increments the integer that the given vector of booleans/bits represents.
// Returns true on overflow, false otherwise. This function is used as helper
// to built all subsets of an another vector of the same size.
bool increment(std::vector<bool> &bs) {
  for (std::size_t i = 0; i < bs.size(); i += 1) {
    if (bs[i]) {
      bs[i] = false;
    } else {
      bs[i] = true;
      return true;
    }
  }
  return false;
}

void TriangularMatrix::insert(std::size_t a, std::size_t b) {
  auto [_a, _b] = std::minmax(a, b);
  if (_b >= this->matrix.size()) {
    // We need to enlarge the 2d matrix vector as it is not large enough to
    // store the support count of the itemset {a,b}.
    std::size_t size = _b + 1;
    this->matrix.resize(size);
    for (std::size_t i = 0; i < size; i += 1) {
      this->matrix[i].resize(i + 1);
    }
  }
  // Increase the support count.
  this->matrix[_b][_a] += 1;
}

int TriangularMatrix::count(size_t a, size_t b) {
  auto [_a, _b] = std::minmax(a, b);
  if (_b < this->matrix.size() && _a < this->matrix[_b].size()) {
    return this->matrix[_b][_a];
  } else {
    return 0;
  }
}
} // namespace AssociationAnalysis