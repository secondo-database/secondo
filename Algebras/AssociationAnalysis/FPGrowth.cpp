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
class FPTreeInMemory : public FPTreeImpl<FPTreeInMemory, std::size_t> {
public:
  explicit FPTreeInMemory(int transactionCount, int minSupport)
      : transactionCount(transactionCount), minSupport(minSupport), nodes(1) {}

private:
  // Represents a node of the FP-Tree.
  struct Node {
    // The item that the node is holding.
    int item;

    // The support count of the itemset that is built of the path from to this
    // node.
    int count;

    // Children indexes.
    std::vector<std::size_t> children;

    // Parent index.
    std::size_t parent;

    // Index to the next node that holds the same item.
    std::size_t link;
  };

  // Represents a row in the header table.
  struct Header {
    int item;

    // Index to the first node that holds the item.
    std::size_t link;
  };

  // The transaction count and the minSupport with which this FP-Tree was
  // created. This numbers will be used while mining to check if a given itemset
  // is frequent.
  int transactionCount;
  int minSupport;

  // Nodes are stored in a vector and point to each other by using indexes into
  // the same vector.
  std::vector<Node> nodes;

  // A header table that is used to find nodes that hold a specific item.
  std::vector<Header> headerTable;

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

  // Returns handle of the root node.
  std::size_t root() { return 0; }

  // Returns handle of the child node with the given item.
  std::optional<std::size_t> findChild(std::size_t node, int item) {
    assert(node < this->nodes.size());
    for (std::size_t child : this->nodes[node].children) {
      if (this->nodes[child].item == item) {
        return child;
      }
    }
    return std::nullopt;
  }

  // Adds the given count to the given node.
  void addCount(std::size_t node, int count) {
    assert(node < this->nodes.size());
    this->nodes[node].count += count;
  }

  // Creates a new child with the given item and count and returns its handle.
  std::size_t createChild(std::size_t node, int item, int count) {
    assert(node < this->nodes.size());
    // Create a new node.
    std::size_t child = this->nodes.size();
    this->nodes.push_back({.item = item, .count = count, .parent = node});

    // Update the header table.
    Header &header = this->header(item);
    this->nodes[child].link = header.link;
    header.link = child;

    // Append the child on the given node.
    this->nodes[node].children.push_back(child);

    return child;
  }

  // Returns the number of entries in the header table.
  std::size_t headerTableSize() { return this->headerTable.size(); }

  // Looks up the link for the given item in the header table.
  std::size_t headerLinkByItem(int item) {
    std::size_t link = this->header(item).link;
    assert(link != 0);
    return link;
  }

  // Returns the item of the entry in the header table with the given index.
  int headerItem(std::size_t index) {
    assert(index < this->headerTable.size());
    return this->headerTable[index].item;
  }

  // Returns the link handle of the entry in the header table with the given
  // index.
  std::size_t headerLink(std::size_t index) {
    assert(index < this->headerTable.size());
    return this->headerTable[index].link;
  }

  // Returns the item of the given node.
  int nodeItem(std::size_t node) {
    assert(node < this->nodes.size());
    return this->nodes[node].item;
  }

  // Returns the count of the given node.
  int nodeCount(std::size_t node) {
    assert(node < this->nodes.size());
    return this->nodes[node].count;
  }

  // Returns the link handle of the given node.
  std::optional<std::size_t> nodeLink(std::size_t node) {
    assert(node < this->nodes.size());
    return this->nodes[node].link == 0
               ? std::nullopt
               : std::make_optional(this->nodes[node].link);
  }

  // Returns the parent handle of the given node.
  std::optional<std::size_t> nodeParent(std::size_t node) {
    assert(node < this->nodes.size());
    if (node == 0) {
      return std::nullopt;
    } else {
      assert(node < this->nodes.size());
      return this->nodes[node].parent == 0
                 ? std::nullopt
                 : std::make_optional(this->nodes[node].parent);
    }
  }

  // Returns the child handles of the given node.
  std::vector<std::size_t> nodeChildren(std::size_t node) {
    assert(node < this->nodes.size());
    return this->nodes[node].children;
  }

  // FPTreeImpl needs access to the private FP-Tree access/manipulation methods:
  // root, addCount, createChild, etc.
  friend class FPTreeImpl<FPTreeInMemory, std::size_t>;
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
      for (std::size_t i = 0; i < transaction->getSize(); i += 1) {
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
    FPTreeInMemory fpTree(transactionCount, minSupport);

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

    fpTree.mine(this->frequentItemsets);
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
      for (std::size_t i = 1; i < headers.length(); i += 1) {
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
      for (std::size_t i = 1; i < nodes.length(); i += 1) {
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

bool FPTreeT::Open(SmiRecord &valueRecord, std::size_t &offset,
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
    std::size_t bufferSize = decltype(fpTree->headerTable)::headerSize();
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
    std::size_t bufferSize = decltype(fpTree->nodes)::headerSize();
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

bool FPTreeT::Save(SmiRecord &valueRecord, std::size_t &offset,
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

    std::size_t bufferSize = decltype(fpTree->headerTable)::headerSize();
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

    std::size_t bufferSize = decltype(fpTree->nodes)::headerSize();
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

// Returns handle of the root node.
int FPTreeT::root() { return 0; }

// Returns handle of the child node with the given item.
std::optional<int> FPTreeT::findChild(int nodeIndex, int item) {
  assert(nodeIndex >= 0 && nodeIndex < this->nodes.Size());
  Node node{};
  this->nodes.Get(nodeIndex, node);
  int childIndex = node.child;
  while (childIndex != 0) {
    Node child{};
    this->nodes.Get(childIndex, child);
    if (child.item == item) {
      return childIndex;
    }
    childIndex = child.nextChild;
  }
  return std::nullopt;
}

// Adds the given count to the given node.
void FPTreeT::addCount(int nodeIndex, int count) {
  assert(nodeIndex >= 0 && nodeIndex < this->nodes.Size());
  Node node{};
  this->nodes.Get(nodeIndex, node);
  node.count += count;
  this->nodes.Put(nodeIndex, node);
}

// Creates a new child with the given item and count and returns its handle.
int FPTreeT::createChild(int nodeIndex, int item, int count) {
  assert(nodeIndex >= 0 && nodeIndex < this->nodes.Size());

  int childId = this->nodes.Size();

  // Find the entry for the given item in the header table.
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
    // Header entry for the given item already exists -> update the link to the
    // new child node.
    Header header{};
    this->headerTable.Get(*headerIndex, header);
    childLink = header.link;
    header.link = childId;
    this->headerTable.Put(*headerIndex, header);
  } else {
    // Header entry for the given item does not exist yet -> create a new entry.
    this->headerTable.Append((Header){.item = item, .link = childId});
  }

  // Create child node.
  Node child{
      .item = item, .count = count, .parent = nodeIndex, .link = childLink};
  this->nodes.Append(child);

  // Append the child node on the given node.
  Node node{};
  this->nodes.Get(nodeIndex, node);
  if (node.child != 0) {
    child.nextChild = node.child;
    this->nodes.Put(childId, child);
  }
  node.child = childId;
  this->nodes.Put(nodeIndex, node);

  return childId;
}

// Returns the number of entries in the header table.
std::size_t FPTreeT::headerTableSize() { return this->headerTable.Size(); }

// Looks up the link for the given item in the header table.
std::size_t FPTreeT::headerLinkByItem(int item) {
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

// Returns the item of the entry in the header table with the given index.
int FPTreeT::headerItem(std::size_t index) {
  assert(index < (std::size_t)this->headerTable.Size());
  Header header{};
  this->headerTable.Get(index, header);
  return header.item;
}

// Returns the link handle of the entry in the header table with the given
// index.
int FPTreeT::headerLink(std::size_t index) {
  assert(index < (std::size_t)this->headerTable.Size());
  Header header{};
  this->headerTable.Get(index, header);
  return header.link;
}

// Returns the item of the given node.
int FPTreeT::nodeItem(int nodeIndex) {
  assert(nodeIndex < this->nodes.Size());
  Node node{};
  this->nodes.Get(nodeIndex, node);
  return node.item;
}

// Returns the count of the given node.
int FPTreeT::nodeCount(int nodeIndex) {
  assert(nodeIndex < this->nodes.Size());
  Node node{};
  this->nodes.Get(nodeIndex, node);
  return node.count;
}

// Returns the link handle of the given node.
std::optional<int> FPTreeT::nodeLink(int nodeIndex) {
  assert(nodeIndex < this->nodes.Size());
  Node node{};
  this->nodes.Get(nodeIndex, node);
  return node.link == 0 ? std::nullopt : std::make_optional(node.link);
}

// Returns the parent handle of the given node.
std::optional<int> FPTreeT::nodeParent(int nodeIndex) {
  assert(nodeIndex < this->nodes.Size());
  if (nodeIndex == 0) {
    return std::nullopt;
  } else {
    Node node{};
    this->nodes.Get(nodeIndex, node);
    return node.parent == 0 ? std::nullopt : std::make_optional(node.parent);
  }
}

// Returns the child handles of the given node.
std::vector<int> FPTreeT::nodeChildren(int nodeIndex) {
  assert(nodeIndex < this->nodes.Size());
  Node node{};
  this->nodes.Get(nodeIndex, node);
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
      for (std::size_t i = 0; i < transaction->getSize(); i += 1) {
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
