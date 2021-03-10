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

#include "FPGrowth.h"

#include "Common.h"

#include "NList.h"
#include "StandardTypes.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace AssociationAnalysis {

// Type mapping for the fpGrowth operator.
ListExpr fpGrowthTM(ListExpr args) {
  NList type(args);

  bool relativeSupport = false;
  NList attrs;
  if (type.length() == 3) {
    if (!type.elem(1).first().checkRel(attrs)) {
      return NList::typeError(
          "Argument number 1 must be of type rel(tuple(...)).");
    }
    if (!type.elem(2).isSymbol(1)) {
      return NList::typeError("Argument number 2 must name an attribute in the "
                              "relation given as the first argument.");
    }
    if (type.elem(3).first().isSymbol(CcInt::BasicType())) {
      if (type.elem(3).second().intval() <= 0) {
        return NList::typeError("Argument number 3 must be of type int and > 0 "
                                "or of type real and in the interval (0, 1).");
      }
    } else if (type.elem(3).first().isSymbol(CcReal::BasicType())) {
      if (type.elem(3).second().realval() <= 0.0 ||
          type.elem(3).second().realval() >= 1.0) {
        return NList::typeError("Argument number 3 must be of type int and > 0 "
                                "or of type real and in the interval (0, 1).");
      } else {
        relativeSupport = true;
      }
    } else {
      return NList::typeError("Argument number 3 must be of type int and > 0 "
                              "or of type real and in the interval (0, 1).");
    }
    if (!type.elem(3).first().isSymbol(CcInt::BasicType()) ||
        type.elem(3).second().intval() <= 0) {
    }
  } else {
    return NList::typeError("3 arguments expected but " +
                            std::to_string(type.length()) + " received.");
  }

  std::string itemsetAttrName = type.elem(2).first().str();
  int itemsetAttr = -1;
  for (int i = 1; i <= (int)attrs.length(); i += 1) {
    NList attr = attrs.elem(i);
    if (attr.elem(1).isSymbol(itemsetAttrName)) {
      itemsetAttr = i;
    }
  }

  if (itemsetAttr == -1) {
    return NList::typeError("Argument number 2 must name an attribute in the "
                            "relation given as the first argument.");
  }

  NList tupleType = NList(frequentItemsetTupleType());
  return NList(Symbols::APPEND(),
               NList(NList().intAtom(itemsetAttr - 1),
                     NList().boolAtom(relativeSupport)),
               NList().streamOf(tupleType))
      .listExpr();
}

// Value mapping for the fpGrowth operator.
int fpGrowthVM(Word *args, Word &result, int message, Word &local, Supplier s) {
  auto *li = (fpGrowthLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    auto relation = (GenericRelation *)args[0].addr;
    bool relativeSupport = ((CcBool *)args[4].addr)->GetBoolval();
    int minSupport = 0;
    if (relativeSupport) {
      double support = ((CcReal *)args[2].addr)->GetRealval();
      minSupport = (int)(std::ceil(support * (double)relation->GetNoTuples()));
    } else {
      minSupport = ((CcInt *)args[2].addr)->GetIntval();
    }
    int attrIndex = ((CcInt *)args[3].addr)->GetIntval();
    local.addr = new fpGrowthLI(relation, minSupport, attrIndex);
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

// Implementation of an FP-Tree. It is used to efficiently mine frequent
// itemsets.
class FPTree {
public:
  explicit FPTree() : nodes(1) {}

  // Inserts the given itemset into the FP-Tree.
  void insert(const std::vector<int> &itemset) {
    this->insert(0, 1, itemset.cbegin(), itemset.cend());
  }

  // Mines frequent itemsets with the given minSupport. The frequent itemsets
  // are appended to the collect vector.
  void mine(std::vector<std::pair<std::vector<int>, double>> &collect,
            int transactionCount, int minSupport) {
    this->mine(collect, transactionCount, minSupport, {});
  }

private:
  // Represents a node of the FP-Tree.
  struct Node {
    // The item that the node is holding.
    int item;

    // The support count of the itemset that is built of the path from to this
    // node.
    int count;

    // Children indexes.
    std::vector<size_t> children;

    // Parent index.
    size_t parent;

    // Index to the next node that holds the same item.
    size_t link;
  };

  // Represents a row in the header table.
  struct Header {
    int item;

    // Index to the first node that holds the item.
    size_t link;
  };

  // Nodes are stored in a vector and point to each other by using indexes into
  // the same vector.
  std::vector<Node> nodes;

  // A header table that is used to find nodes that hold a specific item.
  std::vector<Header> headerTable;

  // Inserts the given itemset with the given count. Used to built conditional
  // FP-Trees without need to inserting the same itemset repeatedly.
  void insert(const std::vector<int> &itemset, int count) {
    this->insert(0, count, itemset.cbegin(), itemset.cend());
  }

  // Inserts the head item of the given itemset into the appropriate child of
  // the given node and calls the insert function recursively with the child and
  // the remaining tail of the itemset. This process is repeated until the end
  // of the itemset is reached.
  void insert(size_t node, int count, std::vector<int>::const_iterator begin,
              std::vector<int>::const_iterator end) {
    if (begin != end) {
      int item = *begin;

      size_t child = 0;

      // Try to find a child node for the item we want to insert.
      for (size_t child_ : this->nodes[node].children) {
        if (this->nodes[child_].item == item) {
          child = child_;
          break;
        }
      }

      if (child != 0) {
        // A child node already exists, so we just need to update the count.
        this->nodes[child].count += count;
      } else {
        // No child node was found for the item we want to insert, so we create
        // a new child node.
        child = this->nodes.size();
        nodes.push_back({.item = item, .count = count, .parent = node});

        // Update the header table.
        Header &header = this->header(item);
        this->nodes[child].link = header.link;
        header.link = child;

        // Append the child on the current node.
        this->nodes[node].children.push_back(child);
      }

      // Proceed with the rest of the itemset.
      begin++;
      insert(child, count, begin, end);
    }
  }

  // Returns the count of the given item.
  int count(int item) {
    int count = 0;
    size_t node = this->header(item).link;
    while (node != 0) {
      count += this->nodes[node].count;
      node = this->nodes[node].link;
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
    size_t node = this->header(item).link;
    while (node != 0) {
      // Built the prefix itemset by climbing up towards the root from the node
      // and collecting the items of the visited node.
      std::vector<int> prefix;
      size_t parent = this->nodes[node].parent;
      while (parent != 0) {
        counts[this->nodes[parent].item] += this->nodes[node].count;
        prefix.push_back(this->nodes[parent].item);
        parent = this->nodes[parent].parent;
      }
      if (!prefix.empty()) {
        // The items in the prefix itemset are ordered descendingly by their
        // support count. As the base will be used to create an FP-Tree later,
        // we reverse the itemset here before including it in the conditional
        // base.
        base.emplace_back(this->nodes[node].count,
                          std::vector<int>(prefix.crbegin(), prefix.crend()));
      }

      // Visit the next node with the same item.
      node = this->nodes[node].link;
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

  // Returns the header that contains the given item. If a header with the given
  // item was not found a new header for this item is created and returned.
  Header &header(int item) {
    for (auto &header : this->headerTable) {
      if (header.item == item) {
        return header;
      }
    }
    this->headerTable.push_back({.item = item});
    return this->headerTable[this->headerTable.size() - 1];
  }

  // Increments the integer that the given vector of booleans/bits represents.
  // Returns true on overflow, false otherwise. This function is used as helper
  // to built all subsets of an another vector of the same size.
  static bool increment(std::vector<bool> &bs) {
    for (size_t i = 0; i < bs.size(); i += 1) {
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
      std::vector<bool> include(this->headerTable.size(), false);
      while (FPTree::increment(include)) {
        int minCount = -1;

        // Build the itemset.
        std::vector<int> itemset;
        assert(include.size() == headerTable.size());
        for (size_t i = 0; i < include.size(); i += 1) {
          if (include[i]) {
            if (minCount == -1 ||
                this->nodes[headerTable[i].link].count < minCount) {
              // We are keeping track of the lowest count here, as that will
              // be the support count of the itemset we are building.
              minCount = this->nodes[headerTable[i].link].count;
            }
            itemset.push_back(this->headerTable[i].item);
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
      for (const auto &header : this->headerTable) {
        int count = this->count(header.item);

        if (count >= minSupport) {
          // Build the itemset.
          std::vector<int> itemset;
          itemset.reserve(suffix.size() + 1);
          itemset.push_back(header.item);
          itemset.insert(itemset.end(), suffix.cbegin(), suffix.cend());

          // We found a frequent itemset -> collect it.
          double support = (double)count / (double)transactionCount;
          collect.emplace_back(itemset, support);

          // Proceed the mining within the FP-Tree conditioned by the current
          // header item.
          FPTree fpTreeConditioned;
          for (auto &[count, itemset] :
               this->computeConditionalBase(header.item, minSupport)) {
            fpTreeConditioned.insert(itemset, count);
          }
          fpTreeConditioned.mine(collect, transactionCount, minSupport,
                                 itemset);
        }
      }
    }
  }

  // Returns true if the FP-Tree consists out of single path.
  bool containsSinglePath() {
    size_t node = 0;
    while (this->nodes[node].children.size() == 1) {
      node = this->nodes[node].children[0];
    }
    return this->nodes[node].children.empty();
  }
};

// Finds all frequent itemsets that satisfy the support given by minSupport.
// The itemset of a transaction is extracted from each tuple of the relation
// by an index given by itemsetAttr.
fpGrowthLI::fpGrowthLI(GenericRelation *relation, int minSupport,
                       int itemsetAttr) {
  int transactionCount = relation->GetNoTuples();

  std::vector<int> frequentItems;

  // Scan the database to find all frequent items.
  {
    // Mapping from an item to its count.
    std::unordered_map<int, int> counts;

    // Database scan.
    std::unique_ptr<GenericRelationIterator> rit(relation->MakeScan());
    Tuple *t;
    while ((t = rit->GetNextTuple()) != nullptr) {
      auto transaction = (collection::IntSet *)t->GetAttribute(itemsetAttr);
      for (size_t i = 0; i < transaction->getSize(); i += 1) {
        counts[transaction->get(i)] += 1;
      }
      t->DeleteIfAllowed();
    }

    // Find the frequent items and sort them descendingly by their support count
    // to reduce the size of the FP-Tree by using the most common prefixes.
    std::vector<std::pair<int, int>> frequentItemSupportPairs;
    for (auto const &[item, count] : counts) {
      if (count >= minSupport) {
        frequentItemSupportPairs.emplace_back(item, count);
      }
    }
    std::sort(frequentItemSupportPairs.begin(), frequentItemSupportPairs.end(),
              [](auto &a, auto &b) -> bool { return a.second > b.second; });

    frequentItems.reserve(frequentItemSupportPairs.size());
    for (const auto &[item, _] : frequentItemSupportPairs) {
      frequentItems.push_back(item);
    }
  }

  // Scan database to create the FP-Tree and mine the frequent itemsets.
  {
    FPTree fpTree;

    // Database scan.
    std::unique_ptr<GenericRelationIterator> rit(relation->MakeScan());
    Tuple *t;
    while ((t = rit->GetNextTuple()) != nullptr) {
      // Create an itemset out of the frequent items that
      auto transaction = (collection::IntSet *)t->GetAttribute(itemsetAttr);
      std::vector<int> itemset;
      for (int item : frequentItems) {
        if (transaction->contains(item)) {
          itemset.push_back(item);
        }
      }
      fpTree.insert(itemset);
      t->DeleteIfAllowed();
    }

    fpTree.mine(this->frequentItemsets, transactionCount, minSupport);
  }

  // Setup iterator for the result stream.
  this->it = this->frequentItemsets.cbegin();

  // Setup resulting tuple type.
  this->tupleType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(frequentItemsetTupleType()));
}

// Returns the next frequent itemset as a tuple.
Tuple *fpGrowthLI::getNext() {
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
