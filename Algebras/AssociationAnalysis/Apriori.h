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

#include <functional>
#include <set>
#include <utility>
#include <vector>

namespace AssociationAnalysis {

namespace {
// Implementation of a hash tree as described by Agrawal et al. in Fast
// Algorithms for Mining Association Rules.
//
// It is used to accelerate the support-counting of the candidate itemsets by
// providing a subset of all candidates to test against a given transaction.
class ItemsetHashTree {
public:
  ItemsetHashTree() : nodes(1) {}

  // Inserts the itemset into the hash tree. All inserted itemsets have to be of
  // the same size.
  void insert(const std::vector<int> &itemset);

  // Returns all itemsets that potentially can be part of the given transaction.
  [[nodiscard]] std::vector<std::reference_wrapper<const std::vector<int>>>
  subset(const collection::IntSet *transaction) const;

  // Returns true if the hash-tree is empty.
  [[nodiscard]] bool empty() const;

private:
  // Specifies how many itemsets a node can hold before it is broken up.
  static const size_t ITEMSET_THRESHOLD = 32;

  // Specifies how many children-nodes there are per interior node.
  static const size_t CHILDREN_NUM = 128;

  void insert(const std::vector<int> &itemset, size_t node, size_t depth);

  void subset(const collection::IntSet *transaction, size_t node, size_t depth,
              std::vector<std::reference_wrapper<const std::vector<int>>>
                  &itemsets) const;

  struct Node {
    Node() : isLeaf(true), itemsets(), children() {}

    bool isLeaf;

    // If the node is a leaf node than the itemsets are stored here.
    std::vector<std::vector<int>> itemsets;

    // If the node is an interior node than the indexes of the children nodes
    // are stored here.
    std::vector<size_t> children;
  };

  // Nodes are stored in a vector and point to each other by using indexes into
  // the same vector.
  std::vector<Node> nodes;
};
} // namespace

// Type mapping for the apriori operator.
ListExpr aprioriTM(ListExpr args);

// Local info class for the apriori operator. Contains the implementation of the
// apriori-algorithm.
class aprioriLI {
public:
  // Finds all frequent itemsets that satisfy the support given by minSupport.
  // The itemset of a transaction is extracted from each tuple of the relation
  // by an index given by itemsetAttr.
  aprioriLI(GenericRelation *relation, double minSupport, int itemsetAttr);

  ~aprioriLI() { this->tupleType->DeleteIfAllowed(); }

  // Returns the next frequent itemset as a tuple.
  Tuple *getNext();

private:
  // Used to generate a stream of the frequent itemsets.
  std::vector<std::pair<std::vector<int>, double>>::const_iterator it;

  // Contains the frequent itemsets annotated by their support.
  std::vector<std::pair<std::vector<int>, double>> frequentItemsets;

  // Describes the resulting tuple type: tuple(Itemset: intset, Support: real).
  TupleType *tupleType;
};

// Value mapping for the apriori operator.
int aprioriVM(Word *args, Word &result, int message, Word &local, Supplier s);

// Operator info for the apriori operator.
struct aprioriInfo : OperatorInfo {
  aprioriInfo() : OperatorInfo() {
    this->name = "apriori";
    this->signature = "rel(tuple(...)) attr real -> stream(tuple(Itemset: "
                      "intset, Support: real))";
    this->syntax = "_ apriori[_, _]";
    this->meaning = "Discovers the frequent itemsets in the given relation of "
                    "transactions by using the apriori-algorithm. The expected "
                    "arguments are: the relation that contains the "
                    "transactions, the name of the attribute that contains the "
                    "items as an intset and the minimum support to look for.";
    this->usesArgsInTypeMapping = true;
  }
};
} // namespace AssociationAnalysis
