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

#include "Apriori.h"

#include "Common.h"

#include "NList.h"
#include "StandardTypes.h"

#include <memory>

namespace AssociationAnalysis {

// Implementation of a bitmap which represents an itemset. Each bit corresponds
// to an item.
class ItemsetBitmap {
public:
  // Inserts the given item into the itemset.
  void insert(unsigned long item) {
    unsigned long index = ItemsetBitmap::index(item);
    if (index >= this->bitmap.size()) {
      this->bitmap.resize(index + 1);
    }
    this->bitmap[index] |= ItemsetBitmap::mask(item);
  }

  // Returns true if the itemset contains the the given item.
  [[nodiscard]] bool contains(unsigned long item) const {
    unsigned long index = ItemsetBitmap::index(item);
    if (index < this->bitmap.size()) {
      return this->bitmap[index] & ItemsetBitmap::mask(item);
    } else {
      return false;
    }
  }

  // Resets the itemset but keeps the memory allocated to allow efficient reuse.
  void reset() {
    for (unsigned long &chunk : this->bitmap) {
      chunk = 0;
    }
  }

private:
  // Each item corresponds to a bit in the bitmap. Item k is stored in the k-th
  // bit (counting up from the least significant bit).
  std::vector<unsigned long> bitmap;

  // Computes the index into the bitmap vector for the given item.
  static unsigned long index(unsigned long item) {
    return item / (sizeof(unsigned long) * 8);
  }

  // Computes the mask for the given item.
  static unsigned long mask(unsigned long item) {
    return 1ul << (item % (sizeof(unsigned long) * 8));
  }
};

// Inserts the itemset into the hash tree. All inserted itemsets have to be of
// the same size.
void ItemsetHashTree::insert(const std::vector<int> &itemset) {
  this->insert(itemset, 0, 0);
}

// Returns all itemsets that potentially can be part of the given transaction.
std::vector<std::reference_wrapper<const std::vector<int>>>
ItemsetHashTree::subset(const collection::IntSet *transaction) const {
  std::vector<std::reference_wrapper<const std::vector<int>>> itemsets;
  this->subset(transaction, 0, 0, itemsets);
  return itemsets;
}

// Returns true if the hash-tree is empty.
bool ItemsetHashTree::empty() const {
  // The hash-tree starts with an empty leaf node.
  return this->nodes[0].isLeaf && this->nodes[0].itemsets.empty();
}

// Inserts the given itemset into the given node.
//
// If the node is not a leaf the function will recurse down the tree until a
// leaf is reached and the insertion can happen. When recursion occurs the depth
// parameter is incremented, so the depth parameter keeps track of the depth
// at which the node is located in the hash-tree.
void ItemsetHashTree::insert(const std::vector<int> &itemset, size_t node,
                             size_t depth) {
  // The hash-tree can only grow to a depth of itemset size + 1.
  assert(depth <= itemset.size());

  if (!this->nodes[node].isLeaf) {
    // The current node is not a leaf so we can't insert the itemset here. We
    // have to insert the itemset into one of its children. At depth k of the
    // hash-tree we hash the itemsets k-th item to determine the child.
    size_t child = itemset[depth] % CHILDREN_NUM;
    this->insert(itemset, this->nodes[node].children[child], depth + 1);
  } else {
    // The current node is a leaf so we can insert the itemset here.
    this->nodes[node].itemsets.push_back(itemset);

    // If the itemset capacity of the node (specified by ITEMSET_THRESHOLD) is
    // reached we break the node up by moving the contained itemsets into new
    // children-nodes.
    //
    // If the hash-tree reached its maximal depth (itemset size + 1) at the
    // current node no further break-up can occur.
    if (this->nodes[node].itemsets.size() == ITEMSET_THRESHOLD &&
        depth < itemset.size()) {

      // Create new nodes and mark them as children of the current node.
      this->nodes[node].children.resize(CHILDREN_NUM);
      this->nodes.resize(this->nodes.size() + CHILDREN_NUM);
      for (size_t i = 0; i < CHILDREN_NUM; i += 1) {
        this->nodes[node].children[i] = this->nodes.size() - CHILDREN_NUM + i;
      }

      // Before we insert the itemsets into their new nodes, we must safe the
      // itemsets vector. This is necessary to prevent invalidation problems
      // (the nodes vector can grow while insertion and trigger a reallocation)
      // while we iterate over the itemsets vector. We safe the itemsets vector
      // by moving it into a local variable.
      std::vector<std::vector<int>> itemsets(move(this->nodes[node].itemsets));

      // Insert the itemsets into their new homes.
      for (const std::vector<int> &itemset : itemsets) {
        // As we are depth k of the hash-tree we hash the itemsets k-th item
        // to determine the child-node for the itemset.
        size_t child = itemset[depth] % CHILDREN_NUM;
        this->insert(itemset, this->nodes[node].children[child], depth + 1);
      }

      // This node is no longer a leaf now.
      this->nodes[node].isLeaf = false;
    }
  }
}

// Collects all itemsets that potentially can be part of the given transaction.
//
// To do this the hash-tree is traversed top-down and each child that might
// contain an itemset that is part of the given transaction is visited.
void ItemsetHashTree::subset(
    const collection::IntSet *transaction, size_t node, size_t depth,
    std::vector<std::reference_wrapper<const std::vector<int>>> &itemsets)
    const {
  if (this->nodes[node].isLeaf) {
    // The current node is a leaf collect all contained itemsets.
    for (const std::vector<int> &itemset : this->nodes[node].itemsets) {
      itemsets.emplace_back(itemset);
    }
  } else {
    // The current node is not a leaf so we have to visit all children that
    // might contain an itemset that is part of the given transaction.

    // The depth at which we are in the hash-tree determines how many items of
    // the transaction we already handled and therefore can ignore. If we are
    // the depth k we can ignore the first k items of the transaction and have
    // to visit all children that could contain an itemset with one of the
    // transactions remaining items. So we hash the remaining items to figure
    // out which children to visit specifically.
    std::vector<bool> visit(CHILDREN_NUM, false);
    for (size_t i = depth; i < transaction->getSize(); i += 1) {
      visit[transaction->get(i) % CHILDREN_NUM] = true;
    }

    // Visit the children to collect itemsets that potentially can be part of
    // the given transaction.
    for (size_t child = 0; child < CHILDREN_NUM; child += 1) {
      if (visit[child]) {
        this->subset(transaction, this->nodes[node].children[child], depth + 1,
                     itemsets);
      }
    }
  }
}

// Type mapping for the apriori operator.
ListExpr aprioriTM(ListExpr args) {
  NList type(args);

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
    if (!type.elem(3).first().isSymbol(CcReal::BasicType()) ||
        type.elem(3).second().realval() <= 0.0 ||
        type.elem(3).second().realval() >= 1.0) {
      return NList::typeError(
          "Argument number 3 must be of type real and in the interval (0,1).");
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
  return NList(Symbols::APPEND(), NList().intAtom(itemsetAttr - 1).enclose(),
               NList().streamOf(tupleType))
      .listExpr();
}

// Generates candidates of size k+1 given frequent itemsets of size k.
ItemsetHashTree
genCandidates(const std::set<std::vector<int>> &prevFrequentItemsets) {
  std::vector<std::vector<int>> potentialCandidates;
  // join step
  for (const std::vector<int> &itemset1 : prevFrequentItemsets) {
    for (const std::vector<int> &itemset2 : prevFrequentItemsets) {
      if (&itemset1 == &itemset2) {
        continue;
      }
      auto last1 = --itemset1.cend();
      auto last2 = --itemset2.cend();
      if (*last1 < *last2) {
        auto it1 = itemset1.cbegin();
        auto it2 = itemset2.cbegin();
        bool match = true;
        while (it1 != last1 && it2 != last2) {
          if (*it1 != *it2) {
            match = false;
            break;
          }
          it1++;
          it2++;
        }
        if (match) {
          std::vector<int> itemset;
          itemset.reserve(itemset1.size() + 1);
          auto it = itemset1.cbegin();
          while (it != itemset1.cend() && *it < *last2) {
            it++;
          }
          itemset.insert(itemset.cend(), itemset1.cbegin(), it);
          itemset.push_back(*last2);
          itemset.insert(itemset.cend(), it, itemset1.cend());
          potentialCandidates.push_back(itemset);
        }
      }
    }
  }
  // prune step
  ItemsetHashTree candidates;
  for (const std::vector<int> &candidate : potentialCandidates) {
    bool prune = false;
    for (size_t i = 0; i < candidate.size(); i += 1) {
      std::vector<int> subset(candidate.size() - 1);
      for (size_t j = 0; j < subset.size(); j += 1) {
        if (j < i) {
          subset[j] = candidate[j];
        } else {
          subset[j] = candidate[j + 1];
        }
      }
      if (prevFrequentItemsets.count(subset) == 0) {
        prune = true;
        break;
      }
    }
    if (!prune) {
      candidates.insert(candidate);
    }
  }
  return candidates;
}

// Finds all frequent itemsets that satisfy the support given by minSupport.
// The itemset of a transaction is extracted from each tuple of the relation
// by an index given by itemsetAttr.
aprioriLI::aprioriLI(GenericRelation *relation, double minSupport,
                     int itemsetAttr) {
  int transactionCount = relation->GetNoTuples();

  std::vector<std::set<std::vector<int>>> prevFrequentItemsets;
  prevFrequentItemsets.resize(2);

  // Generate the set of frequent 1-Itemsets.
  {
    // Count how many transactions contain any given item.
    std::unique_ptr<GenericRelationIterator> rit(relation->MakeScan());
    Tuple *t;
    std::unordered_map<int, int> counts;
    while ((t = rit->GetNextTuple()) != nullptr) {
      auto transaction = (collection::IntSet *)t->GetAttribute(itemsetAttr);
      for (int i = 0; i < (int)transaction->getSize(); i += 1) {
        counts[transaction->get(i)] += 1;
      }
      t->DeleteIfAllowed();
    }
    // Add any item as an 1-itemset to the frequent itemsets if it satisfies
    // the minimum support.
    for (auto const &[item, count] : counts) {
      double support = (double)count / (double)transactionCount;
      if (support >= minSupport) {
        std::vector<int> itemset = {item};
        prevFrequentItemsets[1].insert(itemset);
        this->frequentItemsets.emplace_back(itemset, support);
      }
    }
  }

  {
    ItemsetBitmap transactionBitmap;
    for (int size = 2; !prevFrequentItemsets[size - 1].empty(); size += 1) {
      ItemsetHashTree candidates =
          genCandidates(prevFrequentItemsets[size - 1]);
      if (candidates.empty()) {
        break;
      }
      // Count how many transactions contain any given candidate.
      std::unique_ptr<GenericRelationIterator> rit(relation->MakeScan());
      Tuple *t;
      std::map<std::vector<int>, int> counts;
      while ((t = rit->GetNextTuple()) != nullptr) {
        auto transaction = (collection::IntSet *)t->GetAttribute(itemsetAttr);
        transactionBitmap.reset();
        for (size_t i = 0; i < transaction->getSize(); i += 1) {
          transactionBitmap.insert(transaction->get(i));
        }
        for (const std::vector<int> &candidate :
             candidates.subset(transaction)) {
          bool containsCandidate = true;
          for (int item : candidate) {
            if (!transactionBitmap.contains(item)) {
              containsCandidate = false;
              break;
            }
          }
          if (containsCandidate) {
            counts[candidate] += 1;
          }
        }
        t->DeleteIfAllowed();
      }
      // Add any candidate to the frequent itemsets if it satisfies the
      // minimum support.
      prevFrequentItemsets.resize(size + 1);
      for (auto const &[itemset, count] : counts) {
        double support = (double)count / (double)transactionCount;
        if (support >= minSupport) {
          prevFrequentItemsets[size].insert(itemset);
          this->frequentItemsets.emplace_back(itemset, support);
        }
      }
    }
  }

  this->it = this->frequentItemsets.cbegin();

  // Setup resulting tuple type.
  this->tupleType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(frequentItemsetTupleType()));
}

// Returns the next frequent itemset as a tuple.
Tuple *aprioriLI::getNext() {
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

// Value mapping for the apriori operator.
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
} // namespace AssociationAnalysis
