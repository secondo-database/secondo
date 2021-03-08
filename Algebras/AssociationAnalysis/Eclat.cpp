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

#include "Eclat.h"

#include "Common.h"

#include "NList.h"
#include "StandardTypes.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace AssociationAnalysis {

// Implementation of a triangular matrix. It is used to efficiently determine
// the support count of all 2-itemsets in a single database scan.
class TriangularMatrix {
public:
  // Insert the given 2-itemset {a,b} into the triangular matrix and increase
  // its support count.
  void insert(std::size_t a, std::size_t b) {
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

  // Returns the support count of the given 2-itemset {a,b}.
  int count(size_t a, size_t b) {
    auto [_a, _b] = std::minmax(a, b);
    if (_b < this->matrix.size() && _a < this->matrix[_b].size()) {
      return this->matrix[_b][_a];
    } else {
      return 0;
    }
  }

private:
  std::vector<std::vector<int>> matrix;
};

// Type mapping for the eclat operator.
ListExpr eclatTM(ListExpr args) {
  NList type(args);

  NList attrs;
  if (type.length() == 3) {
    if (!type.elem(1).checkRel(attrs)) {
      return NList::typeError("Wrong argument type passed.");
    }
    if (!type.isSymbol(2)) {
      return NList::typeError("Wrong argument type passed.");
    }
    if (!type.elem(3).isSymbol(CcReal::BasicType())) {
      return NList::typeError("Wrong argument type passed.");
    }
  } else {
    return NList::typeError("Wrong number of arguments passed.");
  }

  std::string itemsetAttrName = type.elem(2).str();
  int itemsetAttr = -1;
  for (int i = 1; i <= (int)attrs.length(); i += 1) {
    NList attr = attrs.elem(i);
    if (attr.elem(1).isSymbol(itemsetAttrName)) {
      itemsetAttr = i;
    }
  }

  if (itemsetAttr == -1) {
    return NList::typeError(
        "The given attribute was not found in the tuple description.");
  }

  NList tupleType = NList(frequentItemsetTupleType());
  return NList(Symbols::APPEND(), NList().intAtom(itemsetAttr - 1).enclose(),
               NList().streamOf(tupleType))
      .listExpr();
}

// Value mapping for the eclat operator.
int eclatVM(Word *args, Word &result, int message, Word &local, Supplier s) {
  auto *li = (eclatLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    local.addr =
        new eclatLI((GenericRelation *)args[0].addr,        // relation
                    ((CcReal *)args[2].addr)->GetRealval(), // minSupport
                    ((CcInt *)args[3].addr)->GetIntval()    // attrIndex
        );
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

// Performs a bottom-up search of frequent itemsets by recursively combining
// the atoms to larger itemsets and examining the support of the resulting
// tidsets.
void eclat(
    double minSupport, int transactionCount,
    const std::vector<std::pair<std::vector<int>, std::vector<int>>> &atoms,
    std::vector<std::pair<std::vector<int>, double>> &collect) {
  for (std::size_t i = 0; i < atoms.size(); i += 1) {
    // Atom set for the next level.
    std::vector<std::pair<std::vector<int>, std::vector<int>>> newAtoms;

    for (size_t j = i + 1; j < atoms.size(); j += 1) {
      auto const &[itemset1, tidset1] = atoms[i];
      auto const &[itemset2, tidset2] = atoms[j];
      std::vector<int> tidset;
      std::set_intersection(tidset1.cbegin(), tidset1.cend(), tidset2.cbegin(),
                            tidset2.cend(), std::back_inserter(tidset));
      double support = (double)tidset.size() / (double)transactionCount;
      if (support >= minSupport) {
        std::vector<int> itemset;
        std::set_union(itemset1.cbegin(), itemset1.cend(), itemset2.cbegin(),
                       itemset2.cend(), std::back_inserter(itemset));
        // Place the resulting itemset and tidset into the atom set for the next
        // level of the bottom-up search.
        newAtoms.emplace_back(itemset, tidset);
        // The resulting itemset is frequent -> safe it.
        collect.emplace_back(itemset, support);
      }
    }

    if (!newAtoms.empty()) {
      // New atoms were found, use them to find larger itemsets.
      eclat(minSupport, transactionCount, newAtoms, collect);
    }
  }
}

// Finds all frequent itemsets that satisfy the support given by minSupport.
// The itemset of a transaction is extracted from each tuple of the relation
// by an index given by itemsetAttr.
eclatLI::eclatLI(GenericRelation *relation, double minSupport,
                 int itemsetAttr) {
  int transactionCount = relation->GetNoTuples();

  std::vector<std::pair<int, std::vector<int>>> atoms;
  TriangularMatrix triangularMatrix;

  // Collect all frequent items and populate the triangular matrix with the
  // support counts of all 2-itemsets.
  {
    // Mapping from an item to its tidset.
    std::unordered_map<int, std::set<int>> itemTidsets;

    // Database scan.
    std::unique_ptr<GenericRelationIterator> rit(relation->MakeScan());
    Tuple *t;
    int tid = 0;
    while ((t = rit->GetNextTuple()) != nullptr) {
      auto transaction = (collection::IntSet *)t->GetAttribute(itemsetAttr);

      for (size_t i = 0; i < transaction->getSize(); i += 1) {
        // Insert the tid into the items tidset.
        itemTidsets[transaction->get(i)].insert(tid);

        // Insert all 2-itemsets that are part of the transaction into the
        // triangular matrix.
        for (size_t j = i + 1; j < transaction->getSize(); j += 1) {
          triangularMatrix.insert(transaction->get(i), transaction->get(j));
        }
      }

      tid += 1;
      t->DeleteIfAllowed();
    }

    // Find frequent items and insert them into the atom set.
    for (auto const &[item, tidset] : itemTidsets) {
      double support = (double)tidset.size() / (double)transactionCount;
      if (support >= minSupport) {
        std::vector<int> tidsetv(tidset.cbegin(), tidset.cend());
        atoms.emplace_back(item, tidsetv);
        std::vector<int> itemset = {item};
        this->frequentItemsets.emplace_back(itemset, support);
      }
    }
  }

  // The atoms are sorted descending by their corresponding tidset size. This
  // reduces the number of tidset intersections in the bottom-up search.
  std::sort(atoms.begin(), atoms.end(), [](auto &a, auto &b) -> bool {
    return a.second.size() < b.second.size();
  });

  // Perform a bottom-up search of frequent itemsets by recursively combining
  // the atoms to larger itemsets and examining the support of the
  // resulting tidsets.
  //
  // We inline the first level of the eclat function here so that we can use
  // the triangular matrix for faster support checking.
  for (size_t i = 0; i < atoms.size(); i += 1) {
    // Atom set for the next level.
    std::vector<std::pair<std::vector<int>, std::vector<int>>> newAtoms;

    // Combine atoms to and see if the resulting itemset satisfies minSupport.
    for (size_t j = i + 1; j < atoms.size(); j += 1) {
      auto const &[item1, tidset1] = atoms[i];
      auto const &[item2, tidset2] = atoms[j];
      // Compute the support by consulting the triangular matrix for the support
      // count of the 2-itemset.
      double support = (double)triangularMatrix.count(item1, item2) /
                       (double)transactionCount;
      if (support >= minSupport) {
        // Place the resulting itemset and tidset into the atom set for the next
        // level of the bottom-up search.
        std::vector<int> itemset(
            {std::min(item1, item2), std::max(item1, item2)});
        std::vector<int> tidset;
        std::set_intersection(tidset1.cbegin(), tidset1.cend(),
                              tidset2.cbegin(), tidset2.cend(),
                              std::back_inserter(tidset));
        newAtoms.emplace_back(itemset, tidset);
        // Safe the itemset for the result stream.
        this->frequentItemsets.emplace_back(itemset, support);
      }
    }

    if (!newAtoms.empty()) {
      // New atoms were found, use them to find larger itemsets.
      eclat(minSupport, transactionCount, newAtoms, this->frequentItemsets);
    }
  }

  // Setup iterator for the result stream.
  this->it = this->frequentItemsets.cbegin();

  // Setup resulting tuple type.
  this->tupleType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(frequentItemsetTupleType()));
}

// Returns the next frequent itemset as a tuple.
Tuple *eclatLI::getNext() {
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
} // namespace AssociationAnalysis
