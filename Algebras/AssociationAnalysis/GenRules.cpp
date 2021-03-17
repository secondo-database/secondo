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

#include "GenRules.h"

#include "Common.h"

#include "StandardTypes.h"

#include <memory>

namespace AssociationAnalysis {
ListExpr strongRuleTupleType() {
  NList attrs =
      NList(NList(NList().symbolAtom("Antecedent"),
                  NList().symbolAtom(collection::IntSet::BasicType())),
            NList(NList().symbolAtom("Consequent"),
                  NList().symbolAtom(collection::IntSet::BasicType())),
            NList(NList().symbolAtom("Support"),
                  NList().symbolAtom(CcReal::BasicType())),
            NList(NList().symbolAtom("Confidence"),
                  NList().symbolAtom(CcReal::BasicType())));
  ListExpr type = NList().tupleOf(attrs).listExpr();
  return type;
}

// Generates association rules from the given stream of frequent itemsets.
genStrongRulesLI::genStrongRulesLI(Stream<Tuple> *frequentItemsets,
                                   double minConfidence) {
  std::map<std::vector<int>, double> supports;

  frequentItemsets->open();
  Tuple *t;
  while ((t = frequentItemsets->request())) {
    auto itemset = (collection::IntSet *)t->GetAttribute(0);
    auto support = (CcReal *)t->GetAttribute(1);
    std::vector<int> itemsetv;
    itemsetv.resize(itemset->getSize());
    for (std::size_t i = 0; i < itemset->getSize(); i += 1) {
      itemsetv[i] = itemset->get(i);
    }
    supports[itemsetv] = support->GetRealval();
  }
  frequentItemsets->close();

  for (const auto &[itemset, support] : supports) {
    if (itemset.size() > 1) {
      std::vector<bool> include(itemset.size());
      while (increment(include)) {
        std::vector<int> antecedent;
        std::vector<int> consequent;
        for (std::size_t i = 0; i < itemset.size(); i += 1) {
          if (include[i]) {
            antecedent.push_back(itemset[i]);
          } else {
            consequent.push_back(itemset[i]);
          }
        }
        if (!antecedent.empty() && !consequent.empty()) {
          double confidence = support / supports[antecedent];
          if (confidence >= minConfidence) {
            this->rules.push_back(
                {antecedent, consequent, support, confidence});
          }
        }
      }
    }
  }

  this->it = this->rules.cbegin();

  // Setup resulting tuple type.
  this->tupleType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(strongRuleTupleType()));
}

// Returns the next frequent itemset as a tuple.
Tuple *genStrongRulesLI::getNext() {
  if (this->it != this->rules.cend()) {
    auto &rule = *this->it;
    auto tuple = new Tuple(this->tupleType);
    tuple->PutAttribute(
        0, new collection::IntSet(std::set<int>(rule.antecedent.cbegin(),
                                                rule.antecedent.cend())));
    tuple->PutAttribute(
        1, new collection::IntSet(std::set<int>(rule.consequent.cbegin(),
                                                rule.consequent.cend())));
    tuple->PutAttribute(2, new CcReal(rule.support));
    tuple->PutAttribute(3, new CcReal(rule.confidence));
    this->it++;
    return tuple;
  } else {
    return nullptr;
  }
}

// Type mapping for the genStrongRules operator.
ListExpr genStrongRulesTM(ListExpr args) {
  NList type(args);

  NList attrs;
  if (type.length() == 2) {
    NList tupleType = NList(frequentItemsetTupleType());
    if (!type.elem(1).first().checkStream(tupleType)) {
      return NList::typeError("Argument number 1 must of of type "
                              "tuple(Itemset: intset, Support: real).");
    }
    if (type.elem(2).first().isSymbol(CcReal::BasicType())) {
      if (type.elem(2).second().realval() <= 0.0 ||
          type.elem(2).second().realval() >= 1.0) {
        return NList::typeError("Argument number 2 must be of type real and in "
                                "the interval (0, 1).");
      }
    } else {
      return NList::typeError("Argument number 2 must be of type int and > 0 "
                              "or of type real and in the interval (0, 1).");
    }
  } else {
    return NList::typeError("2 arguments expected but " +
                            std::to_string(type.length()) + " received.");
  }

  NList tupleType = NList(strongRuleTupleType());
  return NList().streamOf(tupleType).listExpr();
}

// Value mapping for the genStrongRules operator.
int genStrongRulesVM(Word *args, Word &result, int message, Word &local,
                     Supplier s) {
  auto *li = (genStrongRulesLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    auto frequentItemsets = new Stream<Tuple>(args[0]);
    double minConfidence = ((CcReal *)args[1].addr)->GetRealval();
    local.addr = new genStrongRulesLI(frequentItemsets, minConfidence);
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
