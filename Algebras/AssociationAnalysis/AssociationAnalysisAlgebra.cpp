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
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Stream.h"

#include <cmath>
#include <random>
#include <set>

/*

3 Algebra Implementation

*/

namespace AssociationAnalysis
{

int genNonZero(std::mt19937 gen, std::poisson_distribution<int> dist)
{
    int size = dist(gen);
    while (size == 0) {
        size = dist(gen);
    }
    return size;
}

//region Implementation of the genTransactions operator.

ListExpr genTransactionsTM(ListExpr args)
{
    NList type(args);

    if (type.length() == 5) {
        for (int i = 1; i <= 5; i++) {
            if (!type.elem(i).isSymbol(CcInt::BasicType())) {
                return NList::typeError(
                    "Wrong argument type passed. "
                    "The signature of genTransactions is: int int int int int "
                    "-> (stream intset)."
                );
            }
        }
    } else {
        return NList::typeError(
            "Wrong number of arguments passed. "
            "The signature of genTransactions is: int int int int int "
            "-> (stream intset)."
        );
    }

    return Stream<collection::IntSet>::wrap(
        listutils::basicSymbol<collection::IntSet>());
}

class getTransactionsLI
{
public:
    getTransactionsLI(
        int numOfTransactions,
        int transactionSizeMean,
        int frequentItemsetSizeMean,
        int numOfFrequentItemsets,
        int numOfItems
    )
        : numOfTransactions(numOfTransactions),
          t(0),
          genTransactionSize(transactionSizeMean),
          allowOversizedTransaction(false)
    {
        // Prepare the random number distributions.
        std::poisson_distribution<int>
            genFrequentItemsetSize(frequentItemsetSizeMean);
        std::uniform_int_distribution<int> genItem(1, numOfItems);
        std::exponential_distribution<float> genReuseFraction(2);
        std::exponential_distribution<double> genWeight(1);

        this->potentialFrequentItemsets.reserve(numOfFrequentItemsets);

        // Generate the first frequent itemset.
        {
            int size = genNonZero(this->gen, genFrequentItemsetSize);
            std::vector<int> itemset;
            itemset.reserve(size);
            for (int i = 0; i < size; i += 1) {
                itemset.push_back(genItem(this->gen));
            }
            this->potentialFrequentItemsets.push_back(itemset);
        }

        // Generate the rest of the frequent itemsets.
        for (int i = 0; i < numOfFrequentItemsets; i += 1) {
            int size = genNonZero(this->gen, genFrequentItemsetSize);

            std::vector<int> itemset;

            // Reuse items from the previously generated itemset.
            std::vector<int> &prevItemset = this->potentialFrequentItemsets[i];
            std::shuffle(prevItemset.begin(), prevItemset.end(), this->gen);
            int reuseNum = (int) std::round(
                std::min(genReuseFraction(this->gen), 1.0f)
                    * prevItemset.size());
            for (int j = 0; j < reuseNum; j += 1) {
                itemset.push_back(prevItemset[j]);
            }

            // Fill the itemset with random items until the desired size is
            // reached.
            while ((int) itemset.size() < size) {
                itemset.push_back(genItem(this->gen));
            }

            this->potentialFrequentItemsets.push_back(itemset);
        }

        // Setup random number distribution for the itemset selection.
        std::vector<double> itemsetWeights;
        itemsetWeights.reserve(numOfFrequentItemsets);
        for (int i = 0; i < numOfFrequentItemsets; i += 1) {
            itemsetWeights.push_back(genWeight(this->gen));
        }
        this->genPotentialFrequentItemset.~discrete_distribution();
        new(&this->genPotentialFrequentItemset) std::discrete_distribution(
            itemsetWeights.begin(),
            itemsetWeights.end());

        // Setup corruption levels.
        std::normal_distribution randCorruptionLevel(0.5, std::sqrt(0.1));
        this->corruptionLevels.reserve(numOfFrequentItemsets);
        for (int i = 0; i < numOfFrequentItemsets; i += 1) {
            this->corruptionLevels
                .push_back(std::clamp(randCorruptionLevel(this->gen),
                                      0.0,
                                      1.0));
        }
    }

    collection::IntSet *getNext()
    {
        std::uniform_real_distribution<double> randCorruptionLevel(0, 1);
        if (this->t < this->numOfTransactions) {
            int transactionSize =
                genNonZero(this->gen, this->genTransactionSize);
            collection::IntSet transaction(true);

            while ((int) transaction.getSize() < transactionSize) {
                int index = this->genPotentialFrequentItemset(this->gen);

                std::vector<int>
                    &itemset = this->potentialFrequentItemsets[index];

                // Create a corrupted itemset.
                std::shuffle(itemset.begin(), itemset.end(), this->gen);
                auto end = itemset.cend();
                while (end != itemset.cbegin() && (
                    randCorruptionLevel(this->gen)
                        < this->corruptionLevels[index]
                )) {
                    end -= 1;
                }
                collection::IntSet
                    corruptedItemset(std::set<int>(itemset.cbegin(), end));

                if ((int) transaction.add(corruptedItemset).getSize()
                    > transactionSize) {
                    if (transaction.getSize() == 0
                        && transactionSize < (int) corruptedItemset.getSize()
                        && !this->allowOversizedTransaction) {
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
                        this->allowOversizedTransaction =
                            !this->allowOversizedTransaction;
                    }
                    break;
                } else {
                    transaction = transaction.add(corruptedItemset);
                }
            }

            this->t += 1;
            return new collection::IntSet(transaction);
        } else {
            return nullptr;
        }
    }

private:
    int numOfTransactions;
    int t;
    std::mt19937 gen;
    std::poisson_distribution<int> genTransactionSize;
    std::discrete_distribution<int> genPotentialFrequentItemset;
    std::vector<std::vector<int>> potentialFrequentItemsets;
    std::vector<double> corruptionLevels;
    bool allowOversizedTransaction;
};

int genTransactionsVM(Word *args,
                      Word &result,
                      int message,
                      Word &local,
                      Supplier s)
{
    auto *li = (getTransactionsLI *) local.addr;
    switch (message) {
        case OPEN: {
            delete li;
            local.addr = new getTransactionsLI(
                ((CcInt *) args[0].addr)->GetIntval(), // numOfTransactions
                ((CcInt *) args[1].addr)->GetIntval(), // transactionSizeMean
                ((CcInt *) args[2].addr)
                    ->GetIntval(), // frequentItemsetSizeMean
                ((CcInt *) args[3].addr)->GetIntval(), // numOfFrequentItemsets
                ((CcInt *) args[4].addr)->GetIntval()  // numOfItems
            );
            return 0;
        }
        case REQUEST:result.addr = li ? li->getNext() : nullptr;
            return result.addr ? YIELD : CANCEL;
        case CLOSE:delete li;
            local.addr = nullptr;
            return 0;
        default:return 0;
    }
}

struct genTransactionsInfo: OperatorInfo
{
    genTransactionsInfo()
        : OperatorInfo()
    {
        this->name = "genTransactions";
        this->signature = "int int int int int -> (stream intset)";
        this->syntax = "genTransaction(numOfTransactions, transactionSizeMean, "
                       "frequentItemsetSizeMean, numOfFrequentItemsets, "
                       "numOfItems)";
        this->meaning = "Generates a stream of transactions.";
    }
};

//endregion

class AssociationAnalysisAlgebra: public Algebra
{
public:
    AssociationAnalysisAlgebra()
        : Algebra()
    {
        AddOperator(genTransactionsInfo(),
                    genTransactionsVM,
                    genTransactionsTM);
    }
};

} // end of AssociationAnalysisAlgebra namespace

extern "C" Algebra *InitializeAssociationAnalysisAlgebra(NestedList *,
                                                         QueryProcessor *)
{
    return new AssociationAnalysis::AssociationAnalysisAlgebra();
}