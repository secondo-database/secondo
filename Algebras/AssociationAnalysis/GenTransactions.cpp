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

#include "GenTransactions.h"

#include "Algebras/Collection/IntSet.h"
#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "NList.h"
#include "NestedList.h"
#include "StandardTypes.h"

namespace AssociationAnalysis {

// Returns the list representation of the tuple type that is used for the
// generated transactions: tuple(Id: int, Itemset: intset)
ListExpr genTransactionsTupleType() {
  NList attrs = NList(
      NList(NList().symbolAtom("Id"), NList().symbolAtom(CcInt::BasicType())),
      NList(NList().symbolAtom("Itemset"),
            NList().symbolAtom(collection::IntSet::BasicType())));
  ListExpr type = NList().tupleOf(attrs).listExpr();
  return type;
}

// Type mapping for the genTransactions operator.
ListExpr genTransactionsTM(ListExpr args) {
  NList type(args);

  if (type.length() == 5) {
    for (int i = 1; i <= 5; i += 1) {
      const NList &arg = type.elem(i);
      if (!arg.first().isSymbol(CcInt::BasicType()) ||
          arg.second().intval() < 1) {
        return NList::typeError("Argument number " + std::to_string(i) +
                                " must be of type int and >= 1.");
      }
    }
  } else {
    return NList::typeError("5 arguments expected but " +
                            std::to_string(type.length()) + " received.");
  }

  NList tupleType = NList(genTransactionsTupleType());
  return NList().streamOf(tupleType).listExpr();
}

// Value mapping for the genTransactions operator.
int genTransactionsVM(Word *args, Word &result, int message, Word &local,
                      Supplier s) {
  auto *li = (getTransactionsLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    local.addr = new getTransactionsLI(
        ((CcInt *)args[0].addr)->GetIntval(), // numOfTransactions
        ((CcInt *)args[1].addr)->GetIntval(), // transactionSizeMean
        ((CcInt *)args[2].addr)->GetIntval(), // frequentItemsetSizeMean
        ((CcInt *)args[3].addr)->GetIntval(), // numOfFrequentItemsets
        ((CcInt *)args[4].addr)->GetIntval()  // numOfItems
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

// Prepares the set of potentially frequent itemsets and everything else that is
// needed for the generation of transactions.
getTransactionsLI::getTransactionsLI(size_t numOfTransactions,
                                     size_t transactionSizeMean,
                                     size_t frequentItemsetSizeMean,
                                     size_t numOfFrequentItemsets,
                                     size_t numOfItems)
    : numOfTransactions(numOfTransactions), t(0),
      genTransactionSize((int)transactionSizeMean - 1),
      allowOversizedTransaction(false) {
  // Prepare the random number distributions.
  std::poisson_distribution<int> genFrequentItemsetSize(
      (int)frequentItemsetSizeMean - 1);
  std::uniform_int_distribution<int> genItem(1, numOfItems);
  std::exponential_distribution<float> genReuseFraction(2);
  std::exponential_distribution<double> genWeight(1);

  this->potentialFrequentItemsets.reserve(numOfFrequentItemsets);

  // Generate the first frequent itemset.
  {
    size_t size = genFrequentItemsetSize(this->gen) + 1;
    std::vector<int> itemset;
    itemset.reserve(size);
    for (size_t i = 0; i < size; i += 1) {
      itemset.push_back(genItem(this->gen));
    }
    this->potentialFrequentItemsets.push_back(itemset);
  }

  // Generate the rest of the frequent itemsets.
  for (size_t i = 0; i < numOfFrequentItemsets; i += 1) {
    size_t size = genFrequentItemsetSize(this->gen) + 1;

    std::vector<int> itemset;

    // Reuse items from the previously generated itemset.
    std::vector<int> &prevItemset = this->potentialFrequentItemsets[i];
    shuffle(prevItemset.begin(), prevItemset.end(), this->gen);
    size_t reuseNum = std::round(std::min(genReuseFraction(this->gen), 1.0f) *
                                 prevItemset.size());
    for (size_t j = 0; j < reuseNum; j += 1) {
      itemset.push_back(prevItemset[j]);
    }

    // Fill the itemset with random items until the desired size is
    // reached.
    while (itemset.size() < size) {
      itemset.push_back(genItem(this->gen));
    }

    this->potentialFrequentItemsets.push_back(itemset);
  }

  // Setup random number distribution for the itemset selection.
  std::vector<double> itemsetWeights;
  itemsetWeights.reserve(numOfFrequentItemsets);
  for (size_t i = 0; i < numOfFrequentItemsets; i += 1) {
    itemsetWeights.push_back(genWeight(this->gen));
  }
  this->genPotentialFrequentItemset.~discrete_distribution();
  new (&this->genPotentialFrequentItemset)
      std::discrete_distribution(itemsetWeights.begin(), itemsetWeights.end());

  // Setup corruption levels.
  std::normal_distribution randCorruptionLevel(0.5, (double)std::sqrt(0.1L));
  this->corruptionLevels.reserve(numOfFrequentItemsets);
  for (size_t i = 0; i < numOfFrequentItemsets; i += 1) {
    this->corruptionLevels.push_back(
        std::clamp(randCorruptionLevel(this->gen), 0.0, 1.0));
  }

  // Setup resulting tuple type.
  this->tupleType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(genTransactionsTupleType()));
}

// Returns the next generated transaction as a tuple.
Tuple *getTransactionsLI::getNext() {
  std::uniform_real_distribution<double> randCorruptionLevel(0, 1);
  if (this->t < this->numOfTransactions) {
    size_t transactionSize = this->genTransactionSize(this->gen) + 1;
    collection::IntSet transaction(true);

    // Fill transaction with items from the potential frequent itemsets
    // till the desired transaction size is reached.
    size_t count = 0; // counter to ensure termination
    while (transaction.getSize() < transactionSize &&
           count < this->potentialFrequentItemsets.size()) {
      count += 1;

      int index = this->genPotentialFrequentItemset(this->gen);

      std::vector<int> &itemset = this->potentialFrequentItemsets[index];

      // Create a corrupted itemset.
      shuffle(itemset.begin(), itemset.end(), this->gen);
      auto end = itemset.cend();
      while (end != itemset.cbegin() &&
             (randCorruptionLevel(this->gen) < this->corruptionLevels[index])) {
        end -= 1;
      }
      collection::IntSet corruptedItemset(std::set<int>(itemset.cbegin(), end));

      if (transaction.add(corruptedItemset).getSize() > transactionSize) {
        if (transaction.getSize() == 0 &&
            transactionSize < corruptedItemset.getSize() &&
            !this->allowOversizedTransaction) {
          // We would end up with an empty transaction here,
          // because the generated transaction is still empty and
          // the itemset that we want to add is larger than the
          // desired transaction size. To prevent this we continue
          // with the next itemset.
          continue;
        } else {
          // Allow an oversized transaction in half the cases.
          if (this->allowOversizedTransaction) {
            transaction = transaction.add(corruptedItemset);
          }
          this->allowOversizedTransaction = !this->allowOversizedTransaction;
        }
        break;
      } else {
        transaction = transaction.add(corruptedItemset);
      }
    }

    auto tuple = new Tuple(this->tupleType);
    tuple->PutAttribute(0, new CcInt((int)this->t));
    tuple->PutAttribute(1, new collection::IntSet(transaction));

    this->t += 1;
    return tuple;
  } else {
    return nullptr;
  }
}
} // namespace AssociationAnalysis