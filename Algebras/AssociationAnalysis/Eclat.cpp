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

#include "StandardTypes.h"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>

namespace AssociationAnalysis {
// Performs a bottom-up search of frequent itemsets by recursively combining
// the atoms to larger itemsets and examining the support of the resulting
// tidsets.
void eclat(
    int minSupport, int transactionCount,
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
      if ((int)tidset.size() >= minSupport) {
        std::vector<int> itemset;
        std::set_union(itemset1.cbegin(), itemset1.cend(), itemset2.cbegin(),
                       itemset2.cend(), std::back_inserter(itemset));
        // Place the resulting itemset and tidset into the atom set for the next
        // level of the bottom-up search.
        newAtoms.emplace_back(itemset, tidset);
        // The resulting itemset is frequent -> safe it.
        double support = (double)tidset.size() / (double)transactionCount;
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
eclatLI::eclatLI(GenericRelation *relation, int minSupport, int itemsetAttr,
                 int deoptimize) {
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
      if ((int)tidset.size() >= minSupport) {
        std::vector<int> tidsetv(tidset.cbegin(), tidset.cend());
        atoms.emplace_back(item, tidsetv);
        std::vector<int> itemset = {item};
        double support = (double)tidset.size() / (double)transactionCount;
        this->frequentItemsets.emplace_back(itemset, support);
      }
    }
  }

  // The atoms are sorted ascendingly by their corresponding tidset size. This
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
      // Compute the support count by consulting the triangular matrix for the
      // support count of the 2-itemset.
      if (triangularMatrix.count(item1, item2) >= minSupport) {
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
        double support = (double)triangularMatrix.count(item1, item2) /
                         (double)transactionCount;
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
