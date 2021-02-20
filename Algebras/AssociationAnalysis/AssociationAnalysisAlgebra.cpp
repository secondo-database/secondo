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

[TOC]

1 Introduction

2 Defines and Includes

*/
#include "Algebra.h"
#include "Algebras/Collection/IntSet.h"
#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "ListUtils.h"
#include "NList.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Stream.h"

#include <cmath>
#include <random>
#include <set>

/*

3 Algebra Implementation

*/

namespace AssociationAnalysis {

using namespace std;

// region Implementation of the genTransactions operator.

ListExpr genTransactionsTupleType() {
  NList attrs = NList(
      NList(NList().symbolAtom("Id"), NList().symbolAtom(CcInt::BasicType())),
      NList(NList().symbolAtom("Itemset"),
            NList().symbolAtom(collection::IntSet::BasicType())));
  ListExpr type = NList().tupleOf(attrs).listExpr();
  return type;
}

ListExpr genTransactionsTM(ListExpr args) {
  NList type(args);

  if (type.length() == 5) {
    for (int i = 1; i <= 5; i += 1) {
      if (!type.elem(i).isSymbol(CcInt::BasicType())) {
        return NList::typeError("Wrong argument type passed.");
      }
    }
  } else {
    return NList::typeError("Wrong number of arguments passed.");
  }

  NList tupleType = NList(genTransactionsTupleType());
  return NList().streamOf(tupleType).listExpr();
}

class getTransactionsLI {
public:
  getTransactionsLI(int numOfTransactions, int transactionSizeMean,
                    int frequentItemsetSizeMean, int numOfFrequentItemsets,
                    int numOfItems)
      : numOfTransactions(numOfTransactions), t(0),
        genTransactionSize(transactionSizeMean - 1),
        allowOversizedTransaction(false) {
    // Prepare the random number distributions.
    poisson_distribution<int> genFrequentItemsetSize(frequentItemsetSizeMean -
                                                     1);
    uniform_int_distribution<int> genItem(1, numOfItems);
    exponential_distribution<float> genReuseFraction(2);
    exponential_distribution<double> genWeight(1);

    this->potentialFrequentItemsets.reserve(numOfFrequentItemsets);

    // Generate the first frequent itemset.
    {
      int size = genFrequentItemsetSize(this->gen) + 1;
      vector<int> itemset;
      itemset.reserve(size);
      for (int i = 0; i < size; i += 1) {
        itemset.push_back(genItem(this->gen));
      }
      this->potentialFrequentItemsets.push_back(itemset);
    }

    // Generate the rest of the frequent itemsets.
    for (int i = 0; i < numOfFrequentItemsets; i += 1) {
      int size = genFrequentItemsetSize(this->gen) + 1;

      vector<int> itemset;

      // Reuse items from the previously generated itemset.
      vector<int> &prevItemset = this->potentialFrequentItemsets[i];
      shuffle(prevItemset.begin(), prevItemset.end(), this->gen);
      int reuseNum = (int)round(min(genReuseFraction(this->gen), 1.0f) *
                                prevItemset.size());
      for (int j = 0; j < reuseNum; j += 1) {
        itemset.push_back(prevItemset[j]);
      }

      // Fill the itemset with random items until the desired size is
      // reached.
      while ((int)itemset.size() < size) {
        itemset.push_back(genItem(this->gen));
      }

      this->potentialFrequentItemsets.push_back(itemset);
    }

    // Setup random number distribution for the itemset selection.
    vector<double> itemsetWeights;
    itemsetWeights.reserve(numOfFrequentItemsets);
    for (int i = 0; i < numOfFrequentItemsets; i += 1) {
      itemsetWeights.push_back(genWeight(this->gen));
    }
    this->genPotentialFrequentItemset.~discrete_distribution();
    new (&this->genPotentialFrequentItemset)
        discrete_distribution(itemsetWeights.begin(), itemsetWeights.end());

    // Setup corruption levels.
    normal_distribution randCorruptionLevel(0.5, (double)sqrt(0.1L));
    this->corruptionLevels.reserve(numOfFrequentItemsets);
    for (int i = 0; i < numOfFrequentItemsets; i += 1) {
      this->corruptionLevels.push_back(
          clamp(randCorruptionLevel(this->gen), 0.0, 1.0));
    }

    // Setup resulting tuple type.
    this->tupleType = new TupleType(
        SecondoSystem::GetCatalog()->NumericType(genTransactionsTupleType()));
  }

  ~getTransactionsLI() { this->tupleType->DeleteIfAllowed(); }

  Tuple *getNext() {
    uniform_real_distribution<double> randCorruptionLevel(0, 1);
    if (this->t < this->numOfTransactions) {
      int transactionSize = this->genTransactionSize(this->gen) + 1;
      collection::IntSet transaction(true);

      while ((int)transaction.getSize() < transactionSize) {
        int index = this->genPotentialFrequentItemset(this->gen);

        vector<int> &itemset = this->potentialFrequentItemsets[index];

        // Create a corrupted itemset.
        shuffle(itemset.begin(), itemset.end(), this->gen);
        auto end = itemset.cend();
        while (end != itemset.cbegin() && (randCorruptionLevel(this->gen) <
                                           this->corruptionLevels[index])) {
          end -= 1;
        }
        collection::IntSet corruptedItemset(set<int>(itemset.cbegin(), end));

        if ((int)transaction.add(corruptedItemset).getSize() >
            transactionSize) {
          if (transaction.getSize() == 0 &&
              transactionSize < (int)corruptedItemset.getSize() &&
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
      tuple->PutAttribute(0, new CcInt(this->t));
      tuple->PutAttribute(1, new collection::IntSet(transaction));

      this->t += 1;
      return tuple;
    } else {
      return nullptr;
    }
  }

private:
  int numOfTransactions;
  int t;
  mt19937 gen;
  poisson_distribution<int> genTransactionSize;
  discrete_distribution<int> genPotentialFrequentItemset;
  vector<vector<int>> potentialFrequentItemsets;
  vector<double> corruptionLevels;
  bool allowOversizedTransaction;
  TupleType *tupleType;
};

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

// endregion

ListExpr frequentItemsetTupleType() {
  NList attrs =
      NList(NList(NList().symbolAtom("Itemset"),
                  NList().symbolAtom(collection::IntSet::BasicType())),
            NList(NList().symbolAtom("Support"),
                  NList().symbolAtom(CcReal::BasicType())));
  ListExpr type = NList().tupleOf(attrs).listExpr();
  return type;
}

// region Implementation of the apriori operator.

ListExpr aprioriTM(ListExpr args) {
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

  string attrName = type.elem(2).str();
  int attrIndex = -1;
  for (int i = 1; i <= (int)attrs.length(); i += 1) {
    NList attr = attrs.elem(i);
    if (attr.elem(1).isSymbol(attrName)) {
      attrIndex = i;
    }
  }

  if (attrIndex == -1) {
    return NList::typeError(
        "The given attribute was not found in the tuple description.");
  }

  NList tupleType = NList(frequentItemsetTupleType());
  return NList(Symbols::APPEND(), NList().intAtom(attrIndex - 1).enclose(),
               NList().streamOf(tupleType))
      .listExpr();
}

class aprioriLI {
public:
  aprioriLI(GenericRelation *relation, double minSupport, int attrIndex) {
    int transactionCount = relation->GetNoTuples();

    vector<set<set<int>>> freqItemsets;

    // Generate the set of frequent 1-Itemsets.
    freqItemsets.resize(2);
    {
      // Count how many transactions contain any given item.
      GenericRelationIterator *rit = relation->MakeScan();
      Tuple *t;
      map<int, int> counts;
      while ((t = rit->GetNextTuple()) != nullptr) {
        auto transaction = (collection::IntSet *)t->GetAttribute(attrIndex);
        for (int i = 0; i < (int)transaction->getSize(); i += 1) {
          counts[transaction->get(i)] += 1;
        }
      }
      // Add any item as an 1-itemset to the frequent itemsets if it satisfies
      // the minimum support.
      for (auto& [item, count] : counts) {
        double support = (double)count / (double)transactionCount;
        if (support >= minSupport) {
          set<int> itemset = {item};
          freqItemsets[1].insert(itemset);
          this->frequentItemsets.emplace_back(itemset, support);
        }
      }
    }

    for (int size = 2; !freqItemsets[size - 1].empty(); size += 1) {
      set<set<int>> candidates =
          aprioriLI::genCandidates(freqItemsets[size - 1]);
      if (candidates.empty()) {
        break;
      }
      // Count how many transactions contain any given candidate.
      GenericRelationIterator *rit = relation->MakeScan();
      Tuple *t;
      map<set<int>, int> counts;
      while ((t = rit->GetNextTuple()) != nullptr) {
        auto transaction = (collection::IntSet *)t->GetAttribute(attrIndex);
        for (const set<int>& candidate : candidates) {
          int containsCandidate = true;
          for (int item : candidate) {
            if (!transaction->contains(item)) {
              containsCandidate = false;
              break;
            }
          }
          if (containsCandidate) {
            counts[candidate] += 1;
          }
        }
      }
      // Add any candidate to the frequent itemsets if it satisfies the minimum
      // support.
      freqItemsets.resize(size + 1);
      for (auto& [itemset, count] : counts) {
        double support = (double)count / (double)transactionCount;
        if (support >= minSupport) {
          freqItemsets[size].insert(itemset);
          this->frequentItemsets.emplace_back(itemset, support);
        }
      }
    }

    this->it = this->frequentItemsets.cbegin();

    // Setup resulting tuple type.
    this->tupleType = new TupleType(
        SecondoSystem::GetCatalog()->NumericType(frequentItemsetTupleType()));
  }

  ~aprioriLI() { this->tupleType->DeleteIfAllowed(); }

  Tuple *getNext() {
    if (this->it != this->frequentItemsets.cend()) {
      auto &[itemset, support] = *this->it;
      auto tuple = new Tuple(this->tupleType);
      tuple->PutAttribute(0, new collection::IntSet(itemset));
      tuple->PutAttribute(1, new CcReal(support));
      this->it++;
      return tuple;
    } else {
      return nullptr;
    }
  }

private:
  vector<pair<set<int>, double> >::const_iterator it;
  vector<pair<set<int>, double> > frequentItemsets;
  TupleType *tupleType;

  static set<set<int>>
  genCandidates(const set<set<int>> &prevFrequentItemsets) {
    if (prevFrequentItemsets.empty()) {
      return {};
    }
    int size = (int)(*prevFrequentItemsets.cbegin()).size() + 1;
    set<set<int>> candidates;
    // join step
    for (const set<int> &itemset1 : prevFrequentItemsets) {
      for (const set<int> &itemset2 : prevFrequentItemsets) {
        if (&itemset1 == &itemset2) {
          continue;
        }
        set<int> itemset;
        itemset.insert(itemset1.cbegin(), itemset1.cend());
        itemset.insert(itemset2.cbegin(), itemset2.cend());
        if ((int)itemset.size() == size) {
          candidates.insert(itemset);
        }
      }
    }
    // prune step
    for (const set<int> &itemset : candidates) {
      for (int item : itemset) {
        set<int> subset(itemset);
        subset.erase(item);
        if (prevFrequentItemsets.count(subset) == 0) {
          candidates.erase(itemset);
          break;
        }
      }
    }
    return candidates;
  }
};

int aprioriVM(Word *args, Word &result, int message, Word &local, Supplier s) {
  auto *li = (aprioriLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    local.addr =
        new aprioriLI((GenericRelation *)args[0].addr,        // relation
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

struct aprioriInfo : OperatorInfo {
  aprioriInfo() : OperatorInfo() {
    this->name = "apriori";
    this->signature = "rel(tuple(...)) attr real -> stream(tuple(Itemset: "
                      "intset, Support: real))";
    this->syntax = "_ apriori[_, _]";
    this->meaning =
        "Discovers the frequent itemsets in the given relation of "
        "transactions. The expected arguments are: the relation that contains "
        "the transactions, the name of the attribute that contains the items "
        "as an intset and the minimum support to look for.";
  }
};

// endregion

class AssociationAnalysisAlgebra : public Algebra {
public:
  AssociationAnalysisAlgebra() : Algebra() {
    AddOperator(genTransactionsInfo(), genTransactionsVM, genTransactionsTM);
    AddOperator(aprioriInfo(), aprioriVM, aprioriTM);
  }
};

} // namespace AssociationAnalysis

extern "C" Algebra *InitializeAssociationAnalysisAlgebra(NestedList *,
                                                         QueryProcessor *) {
  return new AssociationAnalysis::AssociationAnalysisAlgebra();
}