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

#include <vector>

namespace AssociationAnalysis {

// Local info class for the genStrongRules operator.
class genStrongRulesLI {
public:
  // Generates association rules from the given stream of frequent itemsets.
  genStrongRulesLI(Stream<Tuple> *frequentItemsets, double minConfidence);

  ~genStrongRulesLI() { this->tupleType->DeleteIfAllowed(); }

  // Returns the next rule as a tuple.
  Tuple *getNext();

private:
  struct Rule {
    std::vector<int> antecedent;
    std::vector<int> consequent;
    double support;
    double confidence;
  };

  // Used to generate a stream of the rules.
  std::vector<Rule>::const_iterator it;

  // Contains the strong rules annotated by their support and confidence.
  std::vector<Rule> rules;

  // Describes the resulting tuple type: tuple(Antecedent: intset, Consequent:
  // intset, Support: real, Confidence: real).
  TupleType *tupleType;
};

// Type mapping for the genStrongRules operator.
ListExpr genStrongRulesTM(ListExpr args);

// Value mapping for the genStrongRules operator.
int genStrongRulesVM(Word *args, Word &result, int message, Word &local,
                     Supplier s);

// Operator info for the genStrongRules operator.
struct genStrongRulesInfo : OperatorInfo {
  genStrongRulesInfo() : OperatorInfo() {
    this->name = "genStrongRules";
    this->signature = "stream(tuple(Itemset: intset, Support: real)) real -> "
                      "stream(tuple(Antecedent: intset, Consequent: intset, "
                      "Support: real, Confidence: real))";
    this->syntax = "_ genStrongRules[_, _]";
    this->meaning =
        "Generates strong association rules from the given stream of "
        "frequent itemsets. The expected arguments are: a stream "
        "of frequent itemsets and a confidence value.";
    this->usesArgsInTypeMapping = true;
  }
};
} // namespace AssociationAnalysis
