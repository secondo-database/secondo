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

namespace {
enum Deoptimize { NoSinglePathCheck = 1 << 0 };
}

template <class FPTree, typename Handle> class FPTreeImpl {
public:
  // Inserts the given itemset into the FP-Tree.
  void insert(const std::vector<int> &itemset) {
    insert(this->fpTree().root(), 1, itemset.cbegin(), itemset.cend());
  }

  // Inserts the given itemset with the given count into the FP-Tree.
  void insert(const std::vector<int> &itemset, int count) {
    insert(this->fpTree().root(), count, itemset.cbegin(), itemset.cend());
  }

  // Mines frequent itemsets with the given minSupport. The frequent itemsets
  // are appended to the collect vector.
  void mine(std::vector<std::pair<std::vector<int>, double>> &collect,
            int minSupport, int deoptimize) {
    this->mine(collect, {}, minSupport, deoptimize);
  }

private:
  // This class should not be instantiable by itself.
  FPTreeImpl() = default;

  FPTree &fpTree() { return (FPTree &)*this; }

  // Inserts the head item of the given itemset into the appropriate child of
  // the given node and calls the insert function recursively with the child and
  // the remaining tail of the itemset. This process is repeated until the end
  // of the itemset is reached.
  void insert(Handle node, int count, std::vector<int>::const_iterator begin,
              std::vector<int>::const_iterator end) {
    if (begin != end) {
      int item = *begin;

      // Try to find a child node for the item we want to insert.
      std::optional<Handle> child = this->fpTree().findChild(node, item);

      if (child) {
        // A child node already exists, so we just need to update the count.
        this->fpTree().addCount(*child, count);
      } else {
        // No child node was found for the item we want to insert, so we create
        // a new child node.
        child = this->fpTree().createChild(node, item, count);
      }

      // Proceed with the rest of the itemset.
      begin++;
      this->insert(*child, count, begin, end);
    }
  }

  // Returns the count of the given item.
  int count(int item) {
    int count = 0;
    std::optional<Handle> node = this->fpTree().headerLinkByItem(item);
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
    std::optional<Handle> node = this->fpTree().headerLinkByItem(item);
    while (node) {
      int count = this->fpTree().nodeCount(*node);
      // Built the prefix itemset by climbing up towards the root from the node
      // and collecting the items of the visited node.
      std::vector<int> prefix;
      std::optional<Handle> parent = this->fpTree().nodeParent(*node);
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

  // Mines frequent itemsets with the given minSupport. The frequent itemsets
  // are appended to the collect vector.
  void mine(std::vector<std::pair<std::vector<int>, double>> &collect,
            const std::vector<int> &suffix, int minSupport, int deoptimize) {
    int transactionCount = this->fpTree().transactionCount();
    if (!(deoptimize & Deoptimize::NoSinglePathCheck) &&
        this->containsSinglePath()) {
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
      while (increment(include)) {
        int minCount = -1;

        // Build the itemset.
        std::vector<int> itemset;
        for (std::size_t i = 0; i < include.size(); i += 1) {
          if (include[i]) {
            Handle link = this->fpTree().headerLink((Handle)i);
            int count = this->fpTree().nodeCount(link);
            if (minCount == -1 || count < minCount) {
              // We are keeping track of the lowest count here, as that will
              // be the support count of the itemset we are building.
              minCount = count;
            }
            itemset.push_back(this->fpTree().headerItem((Handle)i));
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
        int headerItem = this->fpTree().headerItem((Handle)i);
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
          std::vector<std::pair<int, std::vector<int>>> base =
              this->computeConditionalBase(headerItem, minSupport);
          if (!base.empty()) {
            FPTree fpTreeConditioned(transactionCount);
            for (auto &[count, itemset] : base) {
              fpTreeConditioned.insert(itemset, count);
            }
            fpTreeConditioned.mine(collect, itemset, minSupport, deoptimize);
          }
        }
      }
    }
  }

  // Returns true if the FP-Tree consists out of single path.
  bool containsSinglePath() {
    Handle node = this->fpTree().root();
    std::vector<Handle> children = this->fpTree().nodeChildren(node);
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
  fpGrowthLI(GenericRelation *relation, int minSupport, int itemsetAttr,
             int deoptimize);

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

class FPTreeT : public FPTreeImpl<FPTreeT, SmiRecordId> {
public:
  FPTreeT();

  FPTreeT(int transactionCount);

  FPTreeT(SmiFileId nodeFileId, SmiRecordId nextNodeId, SmiFileId headerFileId,
          SmiRecordId treeRoot, int transactionCount);

  ~FPTreeT() {
    if (this->nodeFile.IsOpen()) {
      this->nodeFile.Close();
    }
    if (this->headerFile.IsOpen()) {
      this->headerFile.Close();
    }
  }

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

  void reset(int transactionCount);

  int transactionCount() { return this->_transactionCount; }

private:
  // Represents a row in the header table.
  struct Header {
    int item;

    // Id of the first node that holds the item.
    SmiRecordId link;

    // Ids (of other headers) used to look up specific items by binary
    // search.
    SmiRecordId left, right;

    void write(SmiRecord &record) const;
    static Header read(SmiHashFile &file, SmiRecordId id);
    static void write(SmiHashFile &file, SmiRecordId id, const Header &header);
    static SmiRecordId create(SmiHashFile &file, SmiRecordId &nextId,
                              const Header &header);
  };

  // Represents a node of the FP-Tree.
  struct Node {
    // The item that the node is holding.
    int item;

    // The support count of the itemset that is built of the path from to this
    // node.
    int count;

    // First child id.
    SmiRecordId child;

    // Ids (of other nodes) used to look up specific child nodes by binary
    // search.
    SmiRecordId left, right;

    // Parent id.
    SmiRecordId parent;

    // Id of the next node that holds the same item.
    SmiRecordId link;

    void write(SmiRecord &record) const;
    static Node read(SmiHashFile &file, SmiRecordId id);
    static void write(SmiHashFile &file, SmiRecordId id, const Node &node);
    static SmiRecordId create(SmiHashFile &file, SmiRecordId &nextId,
                              const Node &node);
  };

  // This file holds all the nodes of the FP-Tree.
  SmiHashFile nodeFile;
  SmiRecordId nextNodeId;

  // This file holds all the headers of the FP-Tree.
  SmiHashFile headerFile;
  SmiRecordId nextHeaderId;

  // The transaction count with which this FP-Tree was created. This number to
  // determine the support of a given itemset.
  int _transactionCount;

  // Returns handle of the root node.
  SmiRecordId root();

  // Returns handle of the root header.
  SmiRecordId headerRoot();

  // Returns handle of the child node with the given item.
  std::optional<SmiRecordId> findChild(SmiRecordId nodeId, int item);

  // Adds the given count to the given node.
  void addCount(SmiRecordId nodeId, int count);

  // Creates a new child with the given item and count and returns its id.
  SmiRecordId createChild(SmiRecordId nodeId, int item, int count);

  // Returns the number of entries in the header table.
  std::size_t headerTableSize();

  // Looks up the link for the given item in the header table.
  SmiRecordId headerLinkByItem(int item);

  // Returns the item of the entry in the header table with the given handle.
  int headerItem(SmiRecordId index);

  // Returns the link id of the entry in the header table with the given handle.
  SmiRecordId headerLink(SmiRecordId index);

  // Returns the item of the given node.
  int nodeItem(SmiRecordId nodeId);

  // Returns the count of the given node.
  int nodeCount(SmiRecordId nodeId);

  // Returns the link id of the given node.
  std::optional<SmiRecordId> nodeLink(SmiRecordId nodeId);

  // Returns the parent id of the given node.
  std::optional<SmiRecordId> nodeParent(SmiRecordId nodeId);

  // Returns the child id of the given node.
  std::vector<SmiRecordId> nodeChildren(SmiRecordId nodeId);

  template <class T>
  static std::optional<SmiRecordId> binaryFind(SmiHashFile &file,
                                               SmiRecordId nodeId, int item,
                                               SmiRecordId &lastId) {
    lastId = nodeId;
    T node = T::read(file, nodeId);
    if (item == node.item) {
      return nodeId;
    } else {
      if (item < node.item) {
        if (node.left == 0) {
          return std::nullopt;
        } else {
          return binaryFind<T>(file, node.left, item, lastId);
        }
      } else {
        if (node.right == 0) {
          return std::nullopt;
        } else {
          return binaryFind<T>(file, node.right, item, lastId);
        }
      }
    }
  }

  template <class T>
  static void binaryInsert(SmiHashFile &file, SmiRecordId targetId, int item,
                           SmiRecordId insertId) {
    T target = T::read(file, targetId);
    if (target.item == item) {
      assert(false);
    } else {
      if (item < target.item) {
        if (target.left == 0) {
          target.left = insertId;
          T::write(file, targetId, target);
        } else {
          return binaryInsert<T>(file, target.left, item, insertId);
        }
      } else {
        if (target.right == 0) {
          target.right = insertId;
          T::write(file, targetId, target);
        } else {
          return binaryInsert<T>(file, target.right, item, insertId);
        }
      }
    }
  }

  // FPTreeImpl needs access to the private FP-Tree access/manipulation methods:
  // root, addCount, createChild, etc.
  friend class FPTreeImpl<FPTreeT, SmiRecordId>;

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
    this->signature = "rel(tuple(...)) attr real -> fptree";
    this->appendSignature("rel(tuple(...)) attr int -> fptree");
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
    this->signature =
        "fptree real -> stream(tuple(Itemset: intset, Support: real))";
    this->appendSignature(
        "fptree int -> stream(tuple(Itemset: intset, Support: real))");
    this->syntax = "_ mineFpTree[_]";
    this->meaning = "Discovers the frequent itemsets in the given FP-Tree";
    this->usesArgsInTypeMapping = true;
  }
};
} // namespace AssociationAnalysis
