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

#include "StandardTypes.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <unordered_map>
#include <vector>

namespace AssociationAnalysis {
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

int mineFpTreeVM(Word *args, Word &result, int message, Word &local,
                 Supplier s) {
  auto *li = (frequentItemsetStreamLI *)local.addr;
  switch (message) {
  case OPEN: {
    delete li;
    std::vector<std::pair<std::vector<int>, double>> frequentItemsets;
    auto fpTree = (FPTreeT *)args[0].addr;
    fpTree->mine(frequentItemsets);
    local.addr = new frequentItemsetStreamLI(std::move(frequentItemsets));
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

FPTreeT::FPTreeT(int transactionCount, int minSupport)
    : headerTable(0), nodes(1), transactionCount(transactionCount),
      minSupport(minSupport) {
  Node root{};
  this->nodes.Append(root);
}

std::string FPTreeT::BasicType() { return "fptree"; }

ListExpr FPTreeT::Out(ListExpr typeInfo, Word w) {
  auto fpTree = (FPTreeT *)w.addr;
  // Serialize headers.
  NList headers;
  for (int i = 0; i < fpTree->headerTable.Size(); i += 1) {
    Header header{};
    assert(fpTree->headerTable.Get(i, header));
    headers.append(
        NList(NList().intAtom(header.item), NList().intAtom(header.link)));
  }
  // Serialize nodes.
  NList nodes;
  for (int i = 0; i < fpTree->nodes.Size(); i += 1) {
    Node node{};
    assert(fpTree->nodes.Get(i, node));
    nodes.append(
        NList(NList().intAtom(node.item), NList().intAtom(node.count),
              NList().intAtom(node.child), NList().intAtom(node.nextChild),
              NList().intAtom(node.parent), NList().intAtom(node.link)));
  }
  return NList(NList().intAtom(fpTree->transactionCount),
               NList().intAtom(fpTree->minSupport), headers, nodes)
      .listExpr();
}

Word FPTreeT::In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct) {
  std::unique_ptr<FPTreeT> fpTree;
  NList in(instance);
  if (in.isList() && in.length() == 4) {
    // Unserialize transactionCount.
    if (!in.first().isSymbol(CcInt::BasicType())) {
      correct = false;
      return nullptr;
    }
    int transactionCount = in.first().intval();

    // Unserialize minSupport.
    if (!in.second().isSymbol(CcInt::BasicType())) {
      correct = false;
      return nullptr;
    }
    int minSupport = in.second().intval();

    fpTree = std::make_unique<FPTreeT>(transactionCount, minSupport);

    // Unserialize headers.
    if (in.third().isList()) {
      NList headers = in.third();
      for (size_t i = 1; i < headers.length(); i += 1) {
        if (headers.elem(i).isList() && headers.elem(i).length() == 2 &&
            headers.elem(i).first().isInt() &&
            headers.elem(i).second().isInt()) {
          fpTree->headerTable.Append(
              Header{.item = headers.elem(i).first().intval(),
                     .link = headers.elem(i).second().intval()});
        } else {
          correct = false;
          return nullptr;
        }
      }
    } else {
      correct = false;
      return nullptr;
    }
    // Unserialize nodes.
    if (in.fourth().isList()) {
      NList nodes = in.fourth();
      for (size_t i = 1; i < nodes.length(); i += 1) {
        if (nodes.elem(i).isList() && nodes.elem(i).length() == 6 &&
            nodes.elem(i).first().isInt() && nodes.elem(i).second().isInt() &&
            nodes.elem(i).third().isInt() && nodes.elem(i).fourth().isInt() &&
            nodes.elem(i).fifth().isInt() && nodes.elem(i).sixth().isInt()) {
          fpTree->nodes.Append(
              Node{.item = nodes.elem(i).first().intval(),
                   .count = nodes.elem(i).second().intval(),
                   .child = nodes.elem(i).third().intval(),
                   .nextChild = nodes.elem(i).fourth().intval(),
                   .parent = nodes.elem(i).fifth().intval(),
                   .link = nodes.elem(i).sixth().intval()});
        } else {
          correct = false;
          return nullptr;
        }
      }
    } else {
      correct = false;
      return nullptr;
    }
  } else {
    correct = false;
    return nullptr;
  }
  correct = true;
  return fpTree.release();
}

Word FPTreeT::Create(const ListExpr typeInfo) { return new FPTreeT(0, 0); }

void FPTreeT::Delete(const ListExpr typeInfo, Word &w) {
  auto fpTree = (FPTreeT *)w.addr;
  fpTree->headerTable.Destroy();
  fpTree->nodes.Destroy();
  delete fpTree;
  w.addr = nullptr;
}

bool FPTreeT::Open(SmiRecord &valueRecord, size_t &offset,
                   const ListExpr typeInfo, Word &value) {
  // Read transactionCount.
  int transactionCount;
  if (valueRecord.Read(&transactionCount, sizeof(transactionCount), offset) !=
      sizeof(transactionCount)) {
    return false;
  }
  offset += sizeof(transactionCount);

  // Read minSupport.
  int minSupport;
  if (valueRecord.Read(&minSupport, sizeof(minSupport), offset) !=
      sizeof(minSupport)) {
    return false;
  }
  offset += sizeof(minSupport);

  std::unique_ptr<FPTreeT> fpTree =
      std::make_unique<FPTreeT>(transactionCount, minSupport);

  // Read headerTable DbArray.
  {
    size_t bufferSize = decltype(fpTree->headerTable)::headerSize();
    auto buffer = std::make_unique<char[]>(bufferSize);
    if (valueRecord.Read(buffer.get(), bufferSize, offset) != bufferSize) {
      return false;
    }
    offset += bufferSize;
    SmiSize headerOffset = 0;
    fpTree->headerTable.restoreHeader(buffer.get(), headerOffset);
    if (bufferSize != headerOffset) {
      return false;
    }
  }

  // Read nodes DbArray.
  {
    size_t bufferSize = decltype(fpTree->nodes)::headerSize();
    auto buffer = std::make_unique<char[]>(bufferSize);
    if (valueRecord.Read(buffer.get(), bufferSize, offset) != bufferSize) {
      return false;
    }
    offset += bufferSize;
    SmiSize headerOffset = 0;
    fpTree->nodes.restoreHeader(buffer.get(), headerOffset);
    if (bufferSize != headerOffset) {
      return false;
    }
  }

  value.addr = fpTree.release();
  return true;
}

bool FPTreeT::Save(SmiRecord &valueRecord, size_t &offset,
                   const ListExpr typeInfo, Word &w) {
  offset = 0;
  auto fpTree = (FPTreeT *)w.addr;

  // Write transactionCount.
  if (valueRecord.Write(&fpTree->transactionCount,
                        sizeof(fpTree->transactionCount),
                        offset) != sizeof(fpTree->transactionCount)) {
    return false;
  }
  offset += sizeof(fpTree->transactionCount);

  // Write minSupport.
  if (valueRecord.Write(&fpTree->minSupport, sizeof(fpTree->minSupport),
                        offset) != sizeof(fpTree->minSupport)) {
    return false;
  }
  offset += sizeof(fpTree->minSupport);

  // Write headerTable DbArray.
  {
    SecondoCatalog *catalog = SecondoSystem::GetCatalog();
    SmiRecordFile *file = catalog->GetFlobFile();
    fpTree->headerTable.saveToFile(file, fpTree->headerTable);

    size_t bufferSize = decltype(fpTree->headerTable)::headerSize();
    auto buffer = std::make_unique<char[]>(bufferSize);
    SmiSize headerOffset = 0;
    fpTree->headerTable.serializeHeader(buffer.get(), headerOffset);
    if (bufferSize != headerOffset) {
      return false;
    }
    if (valueRecord.Write(buffer.get(), bufferSize, offset) != bufferSize) {
      return false;
    }
    offset += headerOffset;
  }

  // Write nodes DbArray.
  {
    SecondoCatalog *catalog = SecondoSystem::GetCatalog();
    SmiRecordFile *file = catalog->GetFlobFile();
    fpTree->nodes.saveToFile(file, fpTree->nodes);

    size_t bufferSize = decltype(fpTree->nodes)::headerSize();
    auto buffer = std::make_unique<char[]>(bufferSize);
    SmiSize headerOffset = 0;
    fpTree->nodes.serializeHeader(buffer.get(), headerOffset);
    if (bufferSize != headerOffset) {
      return false;
    }
    if (valueRecord.Write(buffer.get(), bufferSize, offset) != bufferSize) {
      return false;
    }
    offset += headerOffset;
  }

  return true;
}

void FPTreeT::Close(const ListExpr typeInfo, Word &w) {
  delete (FPTreeT *)w.addr;
}

Word FPTreeT::Clone(const ListExpr typeInfo, const Word &w) {
  Word result;
  auto source = (FPTreeT *)w.addr;
  auto clone = new FPTreeT(source->transactionCount, source->minSupport);
  clone->headerTable.copyFrom(source->headerTable);
  clone->nodes.copyFrom(source->nodes);
  return clone;
}

void *FPTreeT::Cast(void *addr) { return (new (addr) FPTreeT); }

int FPTreeT::SizeOf() { return sizeof(FPTreeT); }

bool FPTreeT::KindCheck(ListExpr type, ListExpr &errorInfo) {
  return listutils::isSymbol(type, BasicType());
}

void FPTreeT::reset(int transactionCount, int minSupport) {
  this->transactionCount = transactionCount;
  this->minSupport = minSupport;
  this->nodes.clean();
  Node root{};
  this->nodes.Append(root);
  this->headerTable.clean();
}

struct fptreeInfo : ConstructorInfo {
  fptreeInfo() : ConstructorInfo() {
    this->name = FPTreeT::BasicType();
    this->signature = "-> " + Kind::SIMPLE();
    this->typeExample = FPTreeT::BasicType();
    this->listRep = "(((<item> <link>)*) ((<item> <count> <child> "
                    "<nextChild> <parent> <link>)*))";
    this->valueExample =
        "(((1 1) (2 2)) ((0 0 1 0 0 0) (1 3 2 0 0 0) (2 3 0 0 0 0)))";
    this->remarks =
        "The first list represents the header table of the FP-Tree. The second "
        "list are the nodes of the FP-Tree. All values are integers.";
  }
};

struct fptreeFunctions : ConstructorFunctions<FPTreeT> {
  fptreeFunctions() : ConstructorFunctions<FPTreeT>() {
    this->in = FPTreeT::In;
    this->out = FPTreeT::Out;
    this->create = FPTreeT::Create;
    this->deletion = FPTreeT::Delete;
    this->open = FPTreeT::Open;
    this->save = FPTreeT::Save;
    this->close = FPTreeT::Close;
    this->clone = FPTreeT::Clone;
    this->cast = FPTreeT::Cast;
    this->sizeOf = FPTreeT::SizeOf;
    this->kindCheck = FPTreeT::KindCheck;
  }
};

TypeConstructor fptreeTC = TypeConstructor(fptreeInfo(), fptreeFunctions());

ListExpr createFpTreeTM(ListExpr args) {
  return mineTM(args, NList().symbolAtom(FPTreeT::BasicType()).listExpr());
}

int createFpTreeVM(Word *args, Word &result, int message, Word &local,
                   Supplier s) {
  auto relation = (GenericRelation *)args[0].addr;
  bool relativeSupport = ((CcBool *)args[4].addr)->GetBoolval();
  int minSupport = 0;
  int transactionCount = relation->GetNoTuples();
  if (relativeSupport) {
    double support = ((CcReal *)args[2].addr)->GetRealval();
    minSupport = (int)(std::ceil(support * (double)transactionCount));
  } else {
    minSupport = ((CcInt *)args[2].addr)->GetIntval();
  }
  int itemsetAttr = ((CcInt *)args[3].addr)->GetIntval();

  result = qp->ResultStorage(s);
  auto fpTree = (FPTreeT *)result.addr;
  fpTree->reset(transactionCount, minSupport);

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
      fpTree->insert(itemset);
      t->DeleteIfAllowed();
    }
  }

  return 0;
}

ListExpr mineFpTreeTM(ListExpr args) {
  NList type(args);
  if (type.isList() && type.length() == 1) {
    if (!type.elem(1).first().isSymbol(FPTreeT::BasicType())) {
      return NList::typeError("Argument must be of type fptree.");
    }
    NList tupleType = NList(frequentItemsetTupleType());
    return NList().streamOf(tupleType).listExpr();
  } else {
    return NList::typeError("1 argument expected but " +
                            std::to_string(type.length()) + " received.");
  }
}

} // namespace AssociationAnalysis
