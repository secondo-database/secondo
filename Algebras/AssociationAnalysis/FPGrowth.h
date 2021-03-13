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

#pragma once

#include "Common.h"

#include "Algebras/Collection/IntSet.h"
#include "Algebras/Relation-C++/RelationAlgebra.h" // rel, trel, tuple
#include "NestedList.h"
#include "Operator.h"
#include "Tools/Flob/DbArray.h"

#include <string>
#include <utility>
#include <vector>

namespace AssociationAnalysis {
template <class FPTree, typename Id> class FPTreeImpl {
public:
  // Inserts the given itemset into the FP-Tree.
  void insert(const std::vector<int> &itemset) {
    insert(this->fpTree().root(), 1, itemset.cbegin(), itemset.cend());
  }

  // Mines frequent itemsets with the given minSupport. The frequent itemsets
  // are appended to the collect vector.
  void mine(std::vector<std::pair<std::vector<int>, double>> &collect) {
    this->mine(collect, this->fpTree().transactionCount,
               this->fpTree().minSupport, {});
  }

private:
  // This class should not be instantiable by itself.
  FPTreeImpl() = default;

  FPTree &fpTree() { return (FPTree &)*this; }

  // Inserts the head item of the given itemset into the appropriate child of
  // the given node and calls the insert function recursively with the child and
  // the remaining tail of the itemset. This process is repeated until the end
  // of the itemset is reached.
  void insert(Id nodeId, int count, std::vector<int>::const_iterator begin,
              std::vector<int>::const_iterator end) {
    if (begin != end) {
      int item = *begin;

      // Try to find a child node for the item we want to insert.
      std::optional<Id> childId = this->fpTree().findChild(nodeId, item);

      if (childId) {
        // A child node already exists, so we just need to update the count.
        this->fpTree().addCount(*childId, count);
      } else {
        // No child node was found for the item we want to insert, so we create
        // a new child node.
        childId = this->fpTree().createChild(nodeId, item, count);
      }

      // Proceed with the rest of the itemset.
      begin++;
      this->insert(*childId, count, begin, end);
    }
  }

  // Returns the count of the given item.
  int count(int item) {
    int count = 0;
    std::optional<Id> node = this->fpTree().headerLinkByItem(item);
    while (node) {
      count += this->fpTree().nodeCount(*node);
      node = this->fpTree().nodeLink(*node);
    }
    return count;
  }

  // Computes the conditional base of the given item. The conditional base of
  // an item consists out of all prefix itemsets of the item.
  std::vector<std::pair<int, std::vector<int>>>
  computeConditionalBase(int item, int minSupport) {
    std::vector<std::pair<int, std::vector<int>>> base;

    // Mapping from an item to its count.
    std::unordered_map<int, int> counts;

    // Visit all nodes with the given item and collect the prefix itemsets into
    // the base vector.
    std::optional<Id> node = this->fpTree().headerLinkByItem(item);
    while (node) {
      int count = this->fpTree().nodeCount(*node);
      // Built the prefix itemset by climbing up towards the root from the node
      // and collecting the items of the visited node.
      std::vector<int> prefix;
      std::optional<Id> parent = this->fpTree().nodeParent(*node);
      while (parent) {
        int item = this->fpTree().nodeItem(*parent);
        counts[item] += count;
        prefix.push_back(item);
        parent = this->fpTree().nodeParent(*parent);
      }
      if (!prefix.empty()) {
        // The items in the prefix itemset are ordered descendingly by their
        // support count. As the base will be used to create an FP-Tree later,
        // we reverse the itemset here before including it in the conditional
        // base.
        base.emplace_back(count,
                          std::vector<int>(prefix.crbegin(), prefix.crend()));
      }

      // Visit the next node with the same item.
      node = this->fpTree().nodeLink(*node);
    }

    // Not all items we collected to create the conditional base are necessarily
    // frequent. Those that are not frequent will be filtered out here.
    std::vector<std::pair<int, std::vector<int>>> cleanBase;
    for (const auto &[count, itemset] : base) {
      std::vector<int> cleanItemset;
      for (int item : itemset) {
        if (counts[item] >= minSupport) {
          cleanItemset.push_back(item);
        }
      }
      if (!cleanItemset.empty()) {
        cleanBase.emplace_back(count, cleanItemset);
      }
    }
    return cleanBase;
  }

  // Increments the integer that the given vector of booleans/bits represents.
  // Returns true on overflow, false otherwise. This function is used as helper
  // to built all subsets of an another vector of the same size.
  static bool increment(std::vector<bool> &bs) {
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

  // Mines frequent itemsets with the given minSupport. The frequent itemsets
  // are appended to the collect vector.
  void mine(std::vector<std::pair<std::vector<int>, double>> &collect,
            int transactionCount, int minSupport,
            const std::vector<int> &suffix) {
    if (this->containsSinglePath()) {
      // The FP-Tree is a single path. To obtain the frequent itemsets we have
      // to generate all itemsets that result from concatenating all
      // combinations of the items in this path with the given suffix. Not all
      // itemsets generated in this way will be frequent, the support count of
      // such an itemset is the lowest count of all the nodes that were used.
      //
      // As the items in the path are the only items in this FP-Tree we use the
      // header table to get the items instead of traversing the nodes.

      // We use a vector (named `include`) that represents an integer that is
      // incremented with each step to help with the enumeration of all
      // combinations of the items in the path. The indexes where the vector
      // `include` is true are the indexes of items in the header table
      // that will be included in the itemset we are building.
      std::vector<bool> include(this->fpTree().headerTableSize(), false);
      while (FPTreeImpl<FPTree, Id>::increment(include)) {
        int minCount = -1;

        // Build the itemset.
        std::vector<int> itemset;
        assert(include.size() == this->fpTree().headerTableSize());
        for (std::size_t i = 0; i < include.size(); i += 1) {
          if (include[i]) {
            Id link = this->fpTree().headerLink(i);
            int count = this->fpTree().nodeCount(link);
            if (minCount == -1 || count < minCount) {
              // We are keeping track of the lowest count here, as that will
              // be the support count of the itemset we are building.
              minCount = count;
            }
            itemset.push_back(this->fpTree().headerItem(i));
          }
        }
        itemset.insert(itemset.begin(), suffix.cbegin(), suffix.cend());

        if (minCount >= minSupport) {
          // We found a frequent itemset -> collect it.
          double support = (double)minSupport / (double)transactionCount;
          collect.emplace_back(itemset, support);
        }
      }
    } else {
      // The FP-Tree is not a single path. In this case we generate an itemset
      // for each item in the header table by appending the given suffix to it.
      // The support count of the resulting itemset is the count of all the
      // nodes that contain the header item that was used to generate the
      // itemset.
      //
      // If the resulting itemset is frequent proceed the mining with a FP-Tree
      // conditioned on the header item that was used to generate the itemset.
      // The frequent itemset is passed a the new suffix for the mining in the
      // conditional FP-Tree.
      for (std::size_t i = 0; i < this->fpTree().headerTableSize(); i += 1) {
        int headerItem = this->fpTree().headerItem(i);
        int count = this->count(headerItem);

        if (count >= minSupport) {
          // Build the itemset.
          std::vector<int> itemset;
          itemset.reserve(suffix.size() + 1);
          itemset.push_back(headerItem);
          itemset.insert(itemset.end(), suffix.cbegin(), suffix.cend());

          // We found a frequent itemset -> collect it.
          double support = (double)count / (double)transactionCount;
          collect.emplace_back(itemset, support);

          // Proceed the mining within the FP-Tree conditioned by the current
          // header item.
          FPTree fpTreeConditioned(transactionCount, minSupport);
          for (auto &[count, itemset] :
               this->computeConditionalBase(headerItem, minSupport)) {
            // TODO: implement faster insert here
            for (int i = 0; i < count; i += 1) {
              fpTreeConditioned.insert(itemset);
            }
          }
          fpTreeConditioned.mine(collect, transactionCount, minSupport,
                                 itemset);
        }
      }
    }
  }

  // Returns true if the FP-Tree consists out of single path.
  bool containsSinglePath() {
    Id node = this->fpTree().root();
    std::vector<Id> children = this->fpTree().nodeChildren(node);
    while (children.size() == 1) {
      node = children[0];
      children = this->fpTree().nodeChildren(node);
    }
    return children.empty();
  }

  // The deriving class needs access to the private constructor of this class.
  friend FPTree;
};

// Local info class for the fpGrowth operator. Contains the implementation of
// the fp-growth-algorithm.
class fpGrowthLI {
public:
  // Finds all frequent itemsets that satisfy the support given by minSupport.
  // The itemset of a transaction is extracted from each tuple of the relation
  // by an index given by itemsetAttr.
  fpGrowthLI(GenericRelation *relation, int minSupport, int itemsetAttr);

  ~fpGrowthLI() { this->tupleType->DeleteIfAllowed(); }

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

// Operator info for the fpGrowth operator.
struct fpGrowthInfo : OperatorInfo {
  fpGrowthInfo() : OperatorInfo() {
    this->name = "fpGrowth";
    this->signature = "rel(tuple(...)) attr int -> stream(tuple(Itemset: "
                      "intset, Support: real))";
    this->syntax = "_ fpGrowth[_, _]";
    this->meaning = "Discovers the frequent itemsets in the given relation of "
                    "transactions by using the fp-growth-algorithm. The "
                    "expected arguments are: the relation that contains the "
                    "transactions, the name of the attribute that contains the "
                    "items as an intset and the minimum support to look for.";
    this->usesArgsInTypeMapping = true;
  }
};

class FPTreeT : public FPTreeImpl<FPTreeT, int> {
public:
  FPTreeT(int minSupport, int transactionCount);

  static std::string BasicType();

  static ListExpr Out(ListExpr typeInfo, Word w);

  static Word In(ListExpr typeInfo, ListExpr instance, int errorPos,
                 ListExpr &errorInfo, bool &correct);

  static Word Create(ListExpr typeInfo);

  static void Delete(ListExpr typeInfo, Word &w);

  static bool Open(SmiRecord &valueRecord, std::size_t &offset,
                   ListExpr typeInfo, Word &value);

  static bool Save(SmiRecord &valueRecord, std::size_t &offset,
                   ListExpr typeInfo, Word &w);

  static void Close(ListExpr typeInfo, Word &w);

  static Word Clone(ListExpr typeInfo, const Word &w);

  static void *Cast(void *addr);

  static int SizeOf();

  static bool KindCheck(ListExpr type, ListExpr &errorInfo);

  void reset(int transactionCount, int minSupport);

private:
  // Represents a row in the header table.
  struct Header {
    int item;

    // Index to the first node that holds the item.
    int link;
  };

  // Represents a node of the FP-Tree.
  struct Node {
    // The item that the node is holding.
    int item;

    // The support count of the itemset that is built of the path from to this
    // node.
    int count;

    // First child index.
    int child;

    // Next child index.
    int nextChild;

    // Parent index.
    int parent;

    // Index to the next node that holds the same item.
    int link;
  };

  // A header table that is used to find nodes that hold a specific item.
  DbArray<Header> headerTable;

  // Nodes are stored in a DbArray and point to each other by using indexes into
  // the same vector.
  DbArray<Node> nodes;

  // The transaction count and the minSupport with which this FP-Tree was
  // created. This numbers will be used while mining to check if a given itemset
  // is frequent.
  int transactionCount;
  int minSupport;

  // Default constructor should only be accessible via the
  FPTreeT() = default;

  int root() { return 0; }

  std::optional<int> findChild(int nodeId, int item) {
    assert(nodeId >= 0 && nodeId < this->nodes.Size());
    Node node{};
    this->nodes.Get(nodeId, node);
    int childId = node.child;
    while (childId != 0) {
      Node child{};
      this->nodes.Get(childId, child);
      if (child.item == item) {
        return childId;
      }
      childId = child.nextChild;
    }
    return std::nullopt;
  }

  void addCount(int nodeId, int count) {
    assert(nodeId >= 0 && nodeId < this->nodes.Size());
    Node node{};
    this->nodes.Get(nodeId, node);
    node.count += count;
    this->nodes.Put(nodeId, node);
  }

  int createChild(int nodeId, int item, int count) {
    assert(nodeId >= 0 && nodeId < this->nodes.Size());

    int childId = this->nodes.Size();

    std::optional<int> headerIndex = std::nullopt;
    for (int i = 0; i < this->headerTable.Size(); i += 1) {
      Header header{};
      this->headerTable.Get(i, header);
      if (header.item == item) {
        headerIndex = i;
        break;
      }
    }
    int childLink = 0;
    if (headerIndex) {
      Header header{};
      this->headerTable.Get(*headerIndex, header);
      childLink = header.link;
      header.link = childId;
      this->headerTable.Put(*headerIndex, header);
    } else {
      this->headerTable.Append((Header){.item = item, .link = childId});
    }

    Node child{
        .item = item, .count = count, .parent = nodeId, .link = childLink};
    this->nodes.Append(child);

    Node node{};
    this->nodes.Get(nodeId, node);
    if (node.child != 0) {
      child.nextChild = node.child;
      this->nodes.Put(childId, child);
    }
    node.child = childId;
    this->nodes.Put(nodeId, node);

    return childId;
  }

  std::size_t headerTableSize() { return this->headerTable.Size(); }

  std::size_t headerLinkByItem(int item) {
    for (int i = 0; i < this->headerTable.Size(); i += 1) {
      Header header{};
      this->headerTable.Get(i, header);
      if (header.item == item) {
        assert(header.link != 0);
        return header.link;
      }
    }
    assert(false);
  }

  int headerItem(std::size_t index) {
    assert(index < (std::size_t)this->headerTable.Size());
    Header header{};
    this->headerTable.Get(index, header);
    return header.item;
  }

  int headerLink(std::size_t index) {
    assert(index < (std::size_t)this->headerTable.Size());
    Header header{};
    this->headerTable.Get(index, header);
    return header.link;
  }

  int nodeItem(int nodeId) {
    assert(nodeId < this->nodes.Size());
    Node node{};
    this->nodes.Get(nodeId, node);
    return node.item;
  }

  int nodeCount(int nodeId) {
    assert(nodeId < this->nodes.Size());
    Node node{};
    this->nodes.Get(nodeId, node);
    return node.count;
  }

  std::optional<int> nodeLink(int nodeId) {
    assert(nodeId < this->nodes.Size());
    Node node{};
    this->nodes.Get(nodeId, node);
    return node.link == 0 ? std::nullopt : std::make_optional(node.link);
  }

  std::optional<int> nodeParent(int nodeId) {
    assert(nodeId < this->nodes.Size());
    if (nodeId == 0) {
      return std::nullopt;
    } else {
      Node node{};
      this->nodes.Get(nodeId, node);
      return node.parent == 0 ? std::nullopt : std::make_optional(node.parent);
    }
  }

  std::vector<int> nodeChildren(int nodeId) {
    assert(nodeId < this->nodes.Size());
    Node node{};
    this->nodes.Get(nodeId, node);
    std::vector<int> children;
    if (node.child != 0) {
      int childId = node.child;
      while (childId != 0) {
        children.push_back(childId);
        Node child{};
        this->nodes.Get(childId, child);
        childId = child.nextChild;
      }
    }
    return children;
  }

  // FPTreeImpl needs access to the private FP-Tree access/manipulation methods:
  // root, addCount, createChild, etc.
  friend class FPTreeImpl<FPTreeT, int>;

  // ConstructorFunctions needs access to the private constructor of this class.
  friend struct ConstructorFunctions<FPTreeT>;
};

extern TypeConstructor fptreeTC;

ListExpr createFpTreeTM(ListExpr args);

int createFpTreeVM(Word *args, Word &result, int message, Word &local,
                   Supplier s);

struct createFpTreeInfo : OperatorInfo {
  createFpTreeInfo() : OperatorInfo() {
    this->name = "createFpTree";
    this->signature = "rel(tuple(...)) attr int -> fptree";
    this->syntax = "_ createFpTree[_, _]";
    this->meaning =
        "Creates an FP-Tree out of the transactions in the given relation. The "
        "expected arguments are: the relation that contains transactions, the "
        "name of the attribute that contains the items as an intset and the "
        "minimum support to look for.";
    this->usesArgsInTypeMapping = true;
  }
};

ListExpr mineFpTreeTM(ListExpr args);

int mineFpTreeVM(Word *args, Word &result, int message, Word &local,
                 Supplier s);

struct mineFpTreeInfo : OperatorInfo {
  mineFpTreeInfo() : OperatorInfo() {
    this->name = "mineFpTree";
    this->signature = "fptree -> stream(tuple(Itemset: intset, Support: real))";
    this->syntax = "_ mineFpTree[_, _]";
    this->meaning = "Discovers the frequent itemsets in the given FP-Tree";
    this->usesArgsInTypeMapping = true;
  }
};
} // namespace AssociationAnalysis
