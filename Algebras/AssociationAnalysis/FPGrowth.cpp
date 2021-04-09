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

/*
  FPTree data type implementation

*/

FPTreeT::FPTreeT()
    : nodeFile(SmiKey::KeyDataType::Integer, sizeof(Node), false),
      nextNodeId(0),
      headerFile(SmiKey::KeyDataType::Integer, sizeof(Header), false),
      nextHeaderId(0), headerRoot(std::nullopt), transactionCount(0),
      minSupport(0) {
  this->nodeFile.Create();
  this->headerFile.Create();
  this->treeRoot = Node::create(this->nodeFile, this->nextNodeId, {});
}

FPTreeT::FPTreeT(SmiFileId nodeFileId, SmiRecordId nextNodeId,
                 SmiFileId headerFileId, SmiRecordId nextHeaderId,
                 std::optional<SmiRecordId> headerRoot, SmiRecordId treeRoot,
                 int transactionCount, int minSupport)
    : nodeFile(SmiKey::KeyDataType::Integer, sizeof(Node), false),
      nextNodeId(nextNodeId),
      headerFile(SmiKey::KeyDataType::Integer, sizeof(Header), false),
      nextHeaderId(nextHeaderId), headerRoot(headerRoot), treeRoot(treeRoot),
      transactionCount(transactionCount), minSupport(minSupport) {
  this->nodeFile.Open(nodeFileId);
  this->headerFile.Open(headerFileId);
}

FPTreeT::FPTreeT(int transactionCount, int minSupport)
    : nodeFile(SmiKey::KeyDataType::Integer, sizeof(Node), true), nextNodeId(0),
      headerFile(SmiKey::KeyDataType::Integer, sizeof(Header), true),
      nextHeaderId(0), headerRoot(std::nullopt),
      transactionCount(transactionCount), minSupport(minSupport) {
  this->nodeFile.Create();
  this->headerFile.Create();
  this->treeRoot = Node::create(this->nodeFile, this->nextNodeId, {});
}

std::string FPTreeT::BasicType() { return "fptree"; }

ListExpr FPTreeT::Out(ListExpr typeInfo, Word w) {
  auto fpTree = (FPTreeT *)w.addr;
  // Serialize headers.
  NList headers;
  for (SmiRecordId id = 0; id < fpTree->nextHeaderId; id += 1) {
    Header header = Header::read(fpTree->headerFile, id);
    headers.append(NList(
        NList().intAtom(header.item), NList().intAtom((int)header.link),
        NList().intAtom((int)header.left), NList().intAtom((int)header.right)));
  }
  // Serialize nodes.
  NList nodes;
  for (SmiRecordId id = 0; id < fpTree->nextNodeId; id += 1) {
    Node node = Node::read(fpTree->nodeFile, id);
    NList nodeRepr;
    nodeRepr.append(NList().intAtom(node.item));
    nodeRepr.append(NList().intAtom(node.count));
    nodeRepr.append(NList().intAtom((int)node.child));
    nodeRepr.append(NList().intAtom((int)node.left));
    nodeRepr.append(NList().intAtom((int)node.right));
    nodeRepr.append(NList().intAtom((int)node.parent));
    nodeRepr.append(NList().intAtom((int)node.link));
    nodes.append(nodeRepr);
  }
  return NList(NList().intAtom(fpTree->transactionCount),
               NList().intAtom(fpTree->minSupport), headers, nodes)
      .listExpr();
}

Word FPTreeT::In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct) {
  NList in(instance);
  correct = false;
  if (in.isList() && in.length() == 4) {
    // Unserialize transactionCount.
    if (!in.first().isInt()) {
      return nullptr;
    }
    int transactionCount = in.first().intval();

    // Unserialize minSupport.
    if (!in.second().isInt()) {
      return nullptr;
    }
    int minSupport = in.second().intval();

    // Unserialize headers.
    SmiHashFile headerFile(SmiKey::KeyDataType::Integer, true, false);
    SmiRecordId nextHeaderId = 0;
    headerFile.Create();
    if (in.third().isList() && !in.third().isEmpty()) {
      NList headers = in.third();
      for (std::size_t i = 1; i <= headers.length(); i += 1) {
        if (headers.elem(i).isList() && headers.elem(i).length() == 4 &&
            headers.elem(i).first().isInt() &&
            headers.elem(i).second().isInt() &&
            headers.elem(i).third().isInt() &&
            headers.elem(i).fourth().isInt()) {
          Header::create(
              headerFile, nextHeaderId,
              Header{.item = headers.elem(i).first().intval(),
                     .link = (SmiRecordId)headers.elem(i).second().intval(),
                     .left = (SmiRecordId)headers.elem(i).third().intval(),
                     .right = (SmiRecordId)headers.elem(i).fourth().intval()});
        } else {
          headerFile.Close();
          headerFile.Remove();
          return nullptr;
        }
      }
    } else {
      headerFile.Close();
      headerFile.Remove();
      return nullptr;
    }
    headerFile.Close();
    // Unserialize nodes.
    SmiHashFile nodeFile(SmiKey::KeyDataType::Integer, true, false);
    SmiRecordId nextNodeId = 0;
    nodeFile.Create();
    if (in.fourth().isList() && !in.fourth().isEmpty()) {
      NList nodes = in.fourth();
      for (std::size_t i = 1; i <= nodes.length(); i += 1) {
        if (nodes.elem(i).isList() && nodes.elem(i).length() == 7 &&
            nodes.elem(i).first().isInt() && nodes.elem(i).second().isInt() &&
            nodes.elem(i).third().isInt() && nodes.elem(i).fourth().isInt() &&
            nodes.elem(i).fifth().isInt() && nodes.elem(i).sixth().isInt() &&
            nodes.elem(i).seventh().isInt()) {
          Node::create(
              nodeFile, nextNodeId,
              Node{.item = nodes.elem(i).first().intval(),
                   .count = nodes.elem(i).second().intval(),
                   .child = (SmiRecordId)nodes.elem(i).third().intval(),
                   .left = (SmiRecordId)nodes.elem(i).fourth().intval(),
                   .right = (SmiRecordId)nodes.elem(i).fifth().intval(),
                   .parent = (SmiRecordId)nodes.elem(i).sixth().intval(),
                   .link = (SmiRecordId)nodes.elem(i).seventh().intval()});
        } else {
          nodeFile.Close();
          nodeFile.Remove();
          return nullptr;
        }
      }
    } else {
      nodeFile.Close();
      nodeFile.Remove();
      return nullptr;
    }
    nodeFile.Close();
    // Create FP-tree.
    correct = true;
    return new FPTreeT(nodeFile.GetFileId(), nextNodeId, headerFile.GetFileId(),
                       nextHeaderId, 0, 0, transactionCount, minSupport);
  } else {
    return nullptr;
  }
}

Word FPTreeT::Create(const ListExpr typeInfo) { return new FPTreeT(); }

void FPTreeT::Delete(const ListExpr typeInfo, Word &w) {
  auto fpTree = (FPTreeT *)w.addr;
  if (fpTree->nodeFile.IsOpen()) {
    fpTree->nodeFile.Close();
  }
  fpTree->nodeFile.Drop();
  if (fpTree->headerFile.IsOpen()) {
    fpTree->headerFile.Close();
  }
  fpTree->headerFile.Drop();
  delete fpTree;
  w.addr = nullptr;
}

bool FPTreeT::Open(SmiRecord &valueRecord, std::size_t &offset,
                   const ListExpr typeInfo, Word &value) {
  // Read nodeFileId.
  SmiFileId nodeFileId;
  if (valueRecord.Read(&nodeFileId, sizeof(nodeFileId), offset) !=
      sizeof(nodeFileId)) {
    return false;
  }
  offset += sizeof(nodeFileId);

  // Read nextNodeId.
  int nextNodeId;
  if (valueRecord.Read(&nextNodeId, sizeof(nextNodeId), offset) !=
      sizeof(nextNodeId)) {
    return false;
  }
  offset += sizeof(nextNodeId);

  // Read headerFileId.
  SmiFileId headerFileId;
  if (valueRecord.Read(&headerFileId, sizeof(headerFileId), offset) !=
      sizeof(headerFileId)) {
    return false;
  }
  offset += sizeof(headerFileId);

  // Read nextHeaderId.
  int nextHeaderId;
  if (valueRecord.Read(&nextHeaderId, sizeof(nextHeaderId), offset) !=
      sizeof(nextHeaderId)) {
    return false;
  }
  offset += sizeof(nextHeaderId);

  // Read headerRoot.
  int headerRoot;
  if (valueRecord.Read(&headerRoot, sizeof(headerRoot), offset) !=
      sizeof(headerRoot)) {
    return false;
  }
  offset += sizeof(headerRoot);

  // Read treeRoot.
  SmiRecordId treeRoot;
  if (valueRecord.Read(&treeRoot, sizeof(treeRoot), offset) !=
      sizeof(treeRoot)) {
    return false;
  }
  offset += sizeof(treeRoot);

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

  value.addr = new FPTreeT(nodeFileId, nextNodeId, headerFileId, nextHeaderId,
                           (headerRoot == -1)
                               ? std::nullopt
                               : std::make_optional((SmiRecordId)headerRoot),
                           treeRoot, transactionCount, minSupport);
  return true;
}

bool FPTreeT::Save(SmiRecord &valueRecord, std::size_t &offset,
                   const ListExpr typeInfo, Word &w) {
  offset = 0;
  auto fpTree = (FPTreeT *)w.addr;

  // Write nodeFileId.
  SmiFileId nodeFileId = fpTree->nodeFile.GetFileId();
  if (valueRecord.Write(&nodeFileId, sizeof(nodeFileId), offset) !=
      sizeof(nodeFileId)) {
    return false;
  }
  offset += sizeof(nodeFileId);

  // Write nextNodeId.
  if (valueRecord.Write(&fpTree->nextNodeId, sizeof(fpTree->nextNodeId),
                        offset) != sizeof(fpTree->nextNodeId)) {
    return false;
  }
  offset += sizeof(fpTree->nextNodeId);

  // Write headerFileId.
  SmiFileId headerFileId = fpTree->headerFile.GetFileId();
  if (valueRecord.Write(&headerFileId, sizeof(headerFileId), offset) !=
      sizeof(headerFileId)) {
    return false;
  }
  offset += sizeof(headerFileId);

  // Write nextHeaderId.
  if (valueRecord.Write(&fpTree->nextHeaderId, sizeof(fpTree->nextHeaderId),
                        offset) != sizeof(fpTree->nextHeaderId)) {
    return false;
  }
  offset += sizeof(fpTree->nextHeaderId);

  // Write headerRoot.
  int headerRoot = fpTree->headerRoot ? (int)*fpTree->headerRoot : -1;
  if (valueRecord.Write(&headerRoot, sizeof(headerRoot), offset) !=
      sizeof(headerRoot)) {
    return false;
  }
  offset += sizeof(headerRoot);

  // Write treeRoot.
  if (valueRecord.Write(&fpTree->treeRoot, sizeof(fpTree->treeRoot), offset) !=
      sizeof(fpTree->treeRoot)) {
    return false;
  }
  offset += sizeof(fpTree->treeRoot);

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

  return true;
}

void FPTreeT::Close(const ListExpr typeInfo, Word &w) {
  delete (FPTreeT *)w.addr;
  w.addr = nullptr;
}

Word FPTreeT::Clone(const ListExpr typeInfo, const Word &w) {
  Word result;
  auto source = (FPTreeT *)w.addr;
  // Copy nodes.
  SmiHashFile nodeFile(SmiKey::KeyDataType::Integer, true, false);
  nodeFile.Create();
  SmiRecordId nextNodeId = 0;
  for (SmiRecordId i = 0; i < source->nextNodeId; i += 1) {
    Node::create(nodeFile, nextNodeId, Node::read(source->nodeFile, i));
  }
  nodeFile.Close();
  // Copy headers.
  SmiHashFile headerFile(SmiKey::KeyDataType::Integer, true, false);
  headerFile.Create();
  SmiRecordId nextHeaderId = 0;
  for (SmiRecordId i = 0; i < source->nextHeaderId; i += 1) {
    Header::create(headerFile, nextHeaderId,
                   Header::read(source->headerFile, i));
  }
  headerFile.Close();
  // Create FP-tree clone.
  return new FPTreeT(nodeFile.GetFileId(), nextNodeId, headerFile.GetFileId(),
                     nextHeaderId, 0, 0, source->transactionCount,
                     source->minSupport);
}

void *FPTreeT::Cast(void *addr) { return (new (addr) FPTreeT); }

int FPTreeT::SizeOf() { return sizeof(FPTreeT); }

bool FPTreeT::KindCheck(ListExpr type, ListExpr &errorInfo) {
  return listutils::isSymbol(type, BasicType());
}

void FPTreeT::reset(int transactionCount, int minSupport) {
  if (this->nodeFile.IsOpen()) {
    this->nodeFile.Truncate();
  } else {
    this->nodeFile.Create();
  }
  this->nextNodeId = 0;
  if (this->headerFile.IsOpen()) {
    this->headerFile.Truncate();
  } else {
    this->headerFile.Create();
  }
  this->nextHeaderId = 0;
  this->transactionCount = transactionCount;
  this->minSupport = minSupport;
  this->headerRoot = std::nullopt;
  this->treeRoot = Node::create(this->nodeFile, this->nextNodeId, {});
}

void FPTreeT::Header::write(SmiRecord &record) const {
  size_t offset = 0;
  record.Write(&this->item, sizeof(this->item), offset);
  offset += sizeof(this->item);
  record.Write(&this->link, sizeof(this->link), offset);
  offset += sizeof(this->link);
  record.Write(&this->left, sizeof(this->left), offset);
  offset += sizeof(this->left);
  record.Write(&this->right, sizeof(this->right), offset);
  offset += sizeof(this->right);
}

FPTreeT::Header FPTreeT::Header::read(SmiHashFile &file, SmiRecordId id) {
  Header header{};
  SmiRecord record;
  file.SelectRecord(id, record);
  size_t offset = 0;
  record.Read(&header.item, sizeof(header.item), offset);
  offset += sizeof(header.item);
  record.Read(&header.link, sizeof(header.link), offset);
  offset += sizeof(header.link);
  record.Read(&header.left, sizeof(header.left), offset);
  offset += sizeof(header.left);
  record.Read(&header.right, sizeof(header.right), offset);
  offset += sizeof(header.right);
  record.Finish();
  return header;
}

void FPTreeT::Header::write(SmiHashFile &file, SmiRecordId id,
                            const Header &header) {
  SmiRecord record;
  file.SelectRecord(id, record, SmiFile::Update);
  header.write(record);
  record.Finish();
}

SmiRecordId FPTreeT::Header::create(SmiHashFile &file, SmiRecordId &nextId,
                                    const Header &header) {
  SmiRecordId id = nextId;
  nextId += 1;
  SmiRecord record;
  file.InsertRecord(id, record);
  header.write(record);
  record.Finish();
  return id;
}

void FPTreeT::Node::write(SmiRecord &record) const {
  size_t offset = 0;
  record.Write(&this->item, sizeof(this->item), offset);
  offset += sizeof(this->item);
  record.Write(&this->count, sizeof(this->count), offset);
  offset += sizeof(this->count);
  record.Write(&this->child, sizeof(this->child), offset);
  offset += sizeof(this->child);
  record.Write(&this->left, sizeof(this->left), offset);
  offset += sizeof(this->left);
  record.Write(&this->right, sizeof(this->right), offset);
  offset += sizeof(this->right);
  record.Write(&this->parent, sizeof(this->parent), offset);
  offset += sizeof(this->parent);
  record.Write(&this->link, sizeof(this->link), offset);
  offset += sizeof(this->link);
}

FPTreeT::Node FPTreeT::Node::read(SmiHashFile &file, SmiRecordId id) {
  Node node{};
  SmiRecord record;
  file.SelectRecord(id, record);
  size_t offset = 0;
  record.Read(&node.item, sizeof(node.item), offset);
  offset += sizeof(node.item);
  record.Read(&node.count, sizeof(node.count), offset);
  offset += sizeof(node.count);
  record.Read(&node.child, sizeof(node.child), offset);
  offset += sizeof(node.child);
  record.Read(&node.left, sizeof(node.left), offset);
  offset += sizeof(node.left);
  record.Read(&node.right, sizeof(node.right), offset);
  offset += sizeof(node.right);
  record.Read(&node.parent, sizeof(node.parent), offset);
  offset += sizeof(node.parent);
  record.Read(&node.link, sizeof(node.link), offset);
  offset += sizeof(node.link);
  record.Finish();
  return node;
}

void FPTreeT::Node::write(SmiHashFile &file, SmiRecordId id, const Node &node) {
  SmiRecord record;
  file.SelectRecord(id, record, SmiFile::Update);
  node.write(record);
  record.Finish();
}

SmiRecordId FPTreeT::Node::create(SmiHashFile &file, SmiRecordId &nextId,
                                  const Node &node) {
  SmiRecord record;
  SmiRecordId id = nextId;
  nextId += 1;
  file.InsertRecord(id, record);
  node.write(record);
  record.Finish();
  return id;
}

// Returns handle of the root node.
SmiRecordId FPTreeT::root() { return this->treeRoot; }

// Returns handle of the child node with the given item.
std::optional<SmiRecordId> FPTreeT::findChild(SmiRecordId nodeId, int item) {
  Node node = Node::read(this->nodeFile, nodeId);
  if (node.child == 0) {
    return std::nullopt;
  } else {
    SmiRecordId ignore;
    return binaryFind<Node>(this->nodeFile, node.child, item, ignore);
  }
}

// Adds the given count to the given node.
void FPTreeT::addCount(SmiRecordId nodeId, int count) {
  Node node = Node::read(this->nodeFile, nodeId);
  node.count += count;
  Node::write(this->nodeFile, nodeId, node);
}

// Creates a new child with the given item and count and returns its handle.
SmiRecordId FPTreeT::createChild(SmiRecordId nodeId, int item, int count) {
  // Create child node.
  Node child{.item = item, .count = count, .parent = nodeId};
  SmiRecordId childId = Node::create(this->nodeFile, this->nextNodeId, child);

  // Find the entry for the given item in the header table.
  if (!this->headerRoot) {
    this->headerRoot = Header::create(this->headerFile, this->nextHeaderId,
                                      {.item = item, .link = childId});
  } else {
    SmiRecordId lastVisitedHeaderId = 0;
    std::optional<SmiRecordId> headerId = binaryFind<Header>(
        this->headerFile, *this->headerRoot, item, lastVisitedHeaderId);
    if (headerId) {
      // Header entry for the given item already exists -> update the link to
      // the new child node.
      Header header = Header::read(this->headerFile, *headerId);
      child.link = header.link;
      Node::write(this->nodeFile, childId, child);
      header.link = childId;
      Header::write(this->headerFile, *headerId, header);
    } else {
      // Header entry for the given item does not exist yet -> create a new
      // entry.
      binaryInsert<Header>(this->headerFile, lastVisitedHeaderId, item,
                           Header::create(this->headerFile, this->nextHeaderId,
                                          {.item = item, .link = childId}));
    }
  }

  // Append the child node on the given node.
  Node node = Node::read(this->nodeFile, nodeId);
  if (node.child == 0) {
    node.child = childId;
    Node::write(this->nodeFile, nodeId, node);
  } else {
    binaryInsert<Node>(this->nodeFile, node.child, item, childId);
  }

  return childId;
}

// Returns the number of entries in the header table.
std::size_t FPTreeT::headerTableSize() {
  return (std::size_t)this->nextHeaderId;
}

// Looks up the link for the given item in the header table.
SmiRecordId FPTreeT::headerLinkByItem(int item) {
  SmiRecordId ignore = 0;
  std::optional<SmiRecordId> headerId =
      binaryFind<Header>(this->headerFile, *this->headerRoot, item, ignore);
  if (headerId) {
    return Header::read(this->headerFile, *headerId).link;
  }
  assert(false);
}

// Returns the item of the entry in the header table with the given handle.
int FPTreeT::headerItem(SmiRecordId id) {
  return Header::read(this->headerFile, id).item;
}

// Returns the link handle of the entry in the header table with the given
// index.
SmiRecordId FPTreeT::headerLink(SmiRecordId id) {
  return Header::read(this->headerFile, id).link;
}

// Returns the item of the given node.
int FPTreeT::nodeItem(SmiRecordId id) {
  return Node::read(this->nodeFile, id).item;
}

// Returns the count of the given node.
int FPTreeT::nodeCount(SmiRecordId id) {
  return Node::read(this->nodeFile, id).count;
}

// Returns the link handle of the given node.
std::optional<SmiRecordId> FPTreeT::nodeLink(SmiRecordId id) {
  Node node = Node::read(this->nodeFile, id);
  return node.link == 0 ? std::nullopt : std::make_optional(node.link);
}

// Returns the parent handle of the given node.
std::optional<SmiRecordId> FPTreeT::nodeParent(SmiRecordId id) {
  Node node = Node::read(this->nodeFile, id);
  if (node.parent == this->treeRoot) {
    return std::nullopt;
  } else {
    return std::make_optional(node.parent);
  }
}

// Returns the child handles of the given node.
std::vector<SmiRecordId> FPTreeT::nodeChildren(SmiRecordId nodeId) {
  std::vector<SmiRecordId> ids;
  Node node = Node::read(this->nodeFile, nodeId);
  if (node.child != 0) {
    std::vector<SmiRecordId> visit = {node.child};
    while (!visit.empty()) {
      SmiRecordId id = visit.back();
      visit.pop_back();
      ids.push_back(id);
      Node child = Node::read(this->nodeFile, id);
      if (child.left != 0) {
        visit.push_back(child.left);
      }
      if (child.right != 0) {
        visit.push_back(child.right);
      }
    }
  }
  return ids;
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
