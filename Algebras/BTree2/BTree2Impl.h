/*
----
This file is part of SECONDO.

Copyright (C) 2004-2010, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Fre PARTICULAR PURPOSE.  See thef
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 Template Header File: BTree2Node

Jan. 2010 M.Klocke, O.Feuer, K.Teufel

1.1 Overview

This is the main class of the BTree Algebra.

*/

#ifndef _BTREE2_IMPLCLASS_H_
#define _BTREE2_IMPLCLASS_H_

using namespace std;

#include "BTree2.h"
#include "BTree2Types.h"
#include "BTree2Node.h"
#include "BTree2Entry.h"
#include "BTree2Cache.h"
#include "BTree2Iterator.h"

#include "Attribute.h"
#include "TupleIdentifier.h"

#include "SecondoCatalog.h"

#include <list>
#include <vector>

namespace BTree2Algebra {

template <typename KEYTYPE,typename VALUETYPE>
class BTree2Impl : public BTree2 {
  public:
    typedef InternalNodeClass<KEYTYPE> InternalNode;
    typedef BTreeNode<KEYTYPE, VALUETYPE> LeafNode;

    BTree2Impl(const string& keytype, const string& valuetype);
/*
Constructor. Object must be initialized with Open or CreateNewBTree.
 
*/
    ~BTree2Impl();
/*
Destructor. When a BTree2 Object is destroyed, the cache object is
also destroyed, so that all nodes are updated on disc if necessary.
Also, all BTree parameter in headerS are stored persistently before
closing the files for persistent storage.
 
*/
    bool CreateNewBTree(double _fill, int _recordSize, 
                         multiplicityType u = multiple,
                         bool _temporary = false);
/*
Initializes a new BTree.
 
*/

    bool Open(SmiFileId& sf, SmiRecordId _headerId, int recordSize);
/*
Read an existing BTree from disc.
 
*/

/*
2.3 Methods for getting information on the BTree

*/
    int GetMinEntries(bool internal);
/*
Returns the minimum number of entries for internal or leaf nodes.

*/

    int GetMaxEntries(bool internal);
/*
Returns the maximum number of entries for internal or leaf nodes.

*/

    int GetMinNodeSize();
/*
Returns the smallest possible record size.

*/

    void GetStatistics(NodeId nid, StatisticStruct& result);
/*
Scans the whole tree and collects various information.

*/

/*
2.4 Content related queries

*/

    Attribute* GetEntryKey(NodeId nodeId, int entryNumber);
/*
Returns an Attribute representation of the value of node ~nodeId~ at index 
~entryNumber~. See also BTree2.h

*/
    Attribute* GetEntryKeyInternal(NodeId nodeId, int entryNumber);
/*
Returns an Attribute representation of the key of node ~nodeId~ at index 
~entryNumber~. See also BTree2.h

*/
    Attribute* GetEntryValue(NodeId nodeId, int entryNumber);
/*
Returns an Attribute representation of the value of node ~nodeId~ at index 
~entryNumber~. See also BTree2.h

*/

    NodeId GetEntryValueInternal(NodeId nodeId, int entryNumber);
/*
Returns the NodeId of the child of node node ~nodeId~ at index 
~entryNumber~. nodeId must point to an internal node.

*/

    int GetNodeEntryCount(NodeId nodeId);
/*
Returns the number of entries of a specific node.

*/

    BTreeNode<KEYTYPE,VALUETYPE>* FindLeftmostLeafNode(const KEYTYPE& key, 
                                              std::vector<NodeId>& path);
/*
Returns the leaf node in which one could possibly find the leftmost occurence
of ~key~ in the leafs. 
It returns a pointer to the leaf node and the path through the tree to 
that node. Note, that this method also returns a node, if there is no
entry with ~key~.

*/

    NodeId FindLeftmostLeafNodeId(const KEYTYPE& key,
                                      std::vector<NodeId>& path);
/*
Returns the leaf node in which one could possibly find the leftmost occurence
of ~key~ in the leafs. 
It also returns the path through the tree to that node.
Note, that this method also returns a node, if there is no
entry with ~key~.

*/
    
    BTreeNode<KEYTYPE,VALUETYPE>* FindRightmostLeafNode(const KEYTYPE& key, 
                                              std::vector<NodeId>& path,
                                              bool mustBeSorted = true);
/*
Returns the leaf node in which one could possibly find the rightmost
occurence of ~key~ in the leafs. 
It returns a pointer to the leaf node and the path through the tree to 
that node. Note, that this method also returns a node, if there is no
entry with ~key~.

*/

    NodeId FindRightmostLeafNodeId(const KEYTYPE& key, 
                                      std::vector<NodeId>& path);
/*
Returns the leaf node in which one could possibly find the rightmost
occurence of ~key~ in the leafs. 
It also returns the path through the tree to that node.

*/
    
    bool FindLeafNodeId(const KEYTYPE& key, const VALUETYPE& value,
                          vector<NodeId>& path,
                          const NodeId currentId,
                          NodeId& result,
                          int h = 0);
/*
Returns the leaf node id in which one could possibly find an entry with ~key~
and ~value~ (the leftmost entry which matches). 
This is a recursive method. It should be called with ~currentId~
set to ~header.rootNodeId~. It returns false, if no key/value pair has been
found.

*/
    BTree2Iterator begin();
/*
Returns begin() but stores a marker, where iteration is stopped.

*/

    BTree2Iterator find(Attribute* key);
/*
Returns an iterator to the first (leftmost) occurence of ~key~ or end, if
not found.

*/

    bool GetNext(NodeId& nodeId, int& entry);
    bool GetNext(NodeId& nodeId, int& entry,Attribute*& key,
                                             Attribute*& value);
/*
Moves the iterator to the succeeding entry. If there is no, the method
returns false and the iterator is set to end.

*/

    BTree2Iterator findLeftBoundary(Attribute* key);
/*
Returns begin() but stores a marker, where iteration is stopped.

*/

    BTree2Iterator findRightBoundary(Attribute* key);
/*
Returns the first entry to the right which is not key.

*/

    void findExactmatchBoundary(Attribute* key, BTree2Iterator& startIter, 
                                                BTree2Iterator& finalIter);
/*
The exactmatch.will be different for uniqueKey, uniqueKeyMultiData and
multiple BTree's. To perform this method, we use a local search in this
method, if necessary. The method delivers an iterator as starting point
indicating the first occurence of key and a second iterator to the first 
entry to the right, which is not key.
Therefore, this method should be better then:
startIteratorMarker = findLeftBoundary(key);
finalIteratorMarker = findRightBoundary(key);

*/

    void findRangeBoundary(Attribute* key, Attribute* secondKey, 
                        BTree2Iterator& startIter, BTree2Iterator& finalIter);
/*
The range-operator will be implemented by a search of the smallest sub-B-Tree, 
that includes key and secondKey. The method delivers an iterator as starting 
point indicating the first occurence of key and a second iterator to the first 
entry to the right, which is greater than secondKey.
Therefore, this method should be better then:
startIteratorMarker = findLeftBoundary(key);
finalIteratorMarker = findRightBoundary(key);

*/


/*
2.5 Tree manipulation

*/
    bool Append(const KEYTYPE& key, const VALUETYPE& tid); 
/*
Append a key/value pair to the tree. Returns true, if inserted sucessfully.
If it comes to an overflow in the leaf node to which the new element is added,
it is tried to split the node. For f <= 0.5, the nodes are simply splitted into two. If f > 0.5, this approach would lead to two nodes violating the 
minimum fill degree. Then multiple nodes are used for a split, e.g. for 
f=2/3 two (full) nodes can be split into three nodes which are in agreement
with the minimum fill degree (care! the maximum number of entries must be
a multiple of three, otherwise one or two elements would be missing). If there is an overflow of a node, it is first tried to move the exceeding element to
its siblings until such a multisplit can be applied.
See ~GetExtentForSplitting~, ~SingleSplit~, ~MultiSplit~ for further details.
It is possible, that multiple nodes are violating the minimum fill degree.

*/
 
    bool AppendGeneric(Attribute* key, Attribute* value);
/*
Append a key/value pair to the tree. Returns true, if inserted sucessfully.
This method first converts the Attribute objects according to the types
of the BTree and calls Append afterwards.

*/

    bool DeleteGeneric(Attribute* key);
/*
Delete the first occurence of ~key~. Returns false, if ~key~ is not found.
This method first converts the Attribute objects according to the types
of the BTree and calls Delete afterwards.

*/

    bool DeleteGeneric(Attribute* key, Attribute* value);
/*
Delete the first occurence of given key/value combination. 
Returns false, if this combination is not found.
This method first converts the Attribute objects according to the types
of the BTree and calls Delete afterwards.

*/

    bool Delete(const KEYTYPE& key);
/*
Delete the first occurence of ~key~. Returns false, if ~key~ is not found.

*/

    bool Delete(const KEYTYPE& key, const VALUETYPE& value);
/*
Delete the first occurence of given key/value combination. 
Returns false, if this combination is not found.

*/

    bool UpdateGeneric(Attribute* key, Attribute* value);
    bool Update(const KEYTYPE& key, const VALUETYPE& value);
/*
Function Update inserts a new entry with key/value, if an entry with KEYTYPE 
key does not exist or replaces the value of the first entry with KEYTYPE key 
with VALUETYPE value.

*/

/*
2.6 Cache delegate methods 

All methods are simple delegates to the methods of the BTree2Cache class.
Documentation is available there.

*/

    void SetCacheSize(size_t mem) { cache.SetCacheSize(mem); }
    size_t GetCacheSize() { return cache.GetCacheSize(); }
    std::set<PinnedNodeInfo> const* GetPinnedNodes() { return 
                                              cache.GetPinnedNodes(); }
    int GetCacheHitCounter() { return cache.GetCacheHitCounter(); }
    void resetCacheHitCounter() { return cache.resetCacheHitCounter(); }
    bool addCachePinnedNode(NodeId n) { 
       return cache.addPinnedNode(n,ProbeIsInternal(n)); 
    }
    bool removeCachePinnedNode(NodeId n) { return cache.removePinnedNode(n); }
    bool IsFixedElementsLimitType() { return cache.IsFixedElementsLimitType(); }
    void SetFixedElementsLimitType() { cache.SetFixedElementsLimitType(); }
    void SetMemoryLimitType() { cache.SetMemoryLimitType(); }
    void DropCacheFile() { cache.DropFile(header.cacheFileId, temporary); } 

    size_t GetPeakCacheMemoryUsage() { return cache.GetPeakCacheMemoryUsage(); }
    int GetPeakCacheElements() { return cache.GetPeakCacheElements();}

    BTree2Cache<KEYTYPE,VALUETYPE> cache;

    void printNodeInfos();  // For debugging...
    void printNodeInfo(NodeId id,int height);  // For debugging...

  private:
/*
2.8 Private methods for node information retrieval

Internally, it is often useful to reference to a specific node by
giving the entry number of the parental node (which does not work
for the root node - however, the root node must often be
treated specially nevertheless).

*/

    int GetCount(int entry, InternalNode* parent);
/*
Get the number of entries of a node.

*/

    bool Overflow(int entry, InternalNode* parent);
/*
Returns true, if the node has too many entries.

*/

    bool Underflow(int entry, InternalNode* parent);
/*
Returns true, if the node has too few entries.

*/

    int GetMaxCountOfChildren(InternalNode* parent);
/*
Returns the maximum number of entries for children of node ~parent~.
While parent is always an internal node, its children can be internal
or leaf nodes with different values for this quantity.

*/

    int GetMinCountOfChildren(InternalNode* parent);
/*
Returns the maximum number of entries for children of node ~parent~.
While parent is always an internal node, its children can be internal
or leaf nodes with different values for this quantity.

*/

    bool HasLeafChildren(InternalNode* p);
/*
Returns true, if children of ~p~ are leaf nodes.

*/

    bool ScanForEntryLeft(LeafNode* startLeafNode, const KEYTYPE& key, 
                              const VALUETYPE& value); 
/*
Returns true, if combination of key/value exists in btree. 
All leaf nodes left to (and including) ~startLeafNode~ are scanned until
a node is reached in which the first entry has a key smaller than ~key~.

*/
    bool ScanForEntryRight(LeafNode* startLeafNode, const KEYTYPE& key, 
                              const VALUETYPE& value); 
/*
Returns true, if combination of key/value exists in btree. 
~startLeafNode~ must be a node, which contains at least one
entry with ~key~.

*/

    LeafNode* fetchLeafNode(int entry, InternalNode* parent);  
/*
Same as cache.fetchLeafNode but with different referencing scheme.

*/

    InternalNode* fetchInternalNode(int entry, InternalNode* parent);  
/*
Same as cache.fetchInternalNode but with different referencing scheme.

*/

/*
2.9 Private methods for tree manipulation

*/

    BTreeNode<KEYTYPE,VALUETYPE>* createLeafNode();
/*
Creates an empty leaf node. It is also moved into the cache,
so the caller must free resources with a call to cache.dispose.

*/

    InternalNode* createInternalNode();
/*
Creates an empty internal node. It is also moved into the cache,
so the caller must free resources with a call to cache.dispose.

*/

    void GetExtentForBalanceUnderflow(int entry, InternalNode* parent, 
                             int count, 
                             int& right_siblings, int&left_siblings);
/*
Returns the number of sibling to the left and to the right which should
be used for balancing underflows of node entry@parent.
The number of siblings checked is by default the same as the extent
for splitting (see below), but it can be raised to get better results.

*/

    void GetExtentForSplitting(int entry, InternalNode* parent, 
                               int& right_siblings, int&left_siblings);
/*
Returns the number of sibling to the left and to the right which should
be used for splitting, assuming that node entry@parent is overflow.

*/

    void ShiftOneElement(int src, InternalNode* parent, int dir);
/*
Shifts one element from src@parent to the left (dir=-1) or to the
right (dir=1) sibling. The caller must ensure, that a) such an sibling
exists and b) the sibling can hold another entry.

*/

    bool ShiftExceed(int entry, InternalNode* parent, int extent, int dir);
/*
Shift an element, if possible, to the sibling given by dir (dir=-1 left,
dir=1 right). If the sibling is already full, it is attempted to shift one
element away of the sibling in the same direction. ~extent~ limits the
number of siblings which are taken into account.

*/

    bool BalanceUnderflow(int entry, InternalNode* parent);
/*
Tries to correct an underflow of node entry@parent. If this method
is called on a node without underflow, it simply returns true. If there
is an underflow, it is tried to fill the node up with elements from its
siblings. The number of siblings taken into account is limited by 
~GetExtentForBalance~. If the underflow is successfully resolved, this
method returns true, otherwise false.

*/

    inline bool BalanceOverflow(int entry, InternalNode* parent);
/*
Tries to correct an overflow of node entry@parent. If this method
is called on a node without overflow, it simply returns true. If there
is an overflow, it is tried to shift one element to its
siblings. The number of siblings taken into account is limited by 
~GetExtentForSplitting~, so that there is no further balancing, if all
nodes are prepared for a split. If the overflow is successfully resolved, 
this method returns true, otherwise false.

*/

    bool CollectItems(int dst, InternalNode* parent, int dir, int depth, 
                       int maxdepth, int& maxLoad);
/*
This recusive methods tries to shift ~maxLoad~ elements to the node dst@parent.
Items are collected from left (dir=-1) or right (dir=1) siblings. 
The maximum number of siblings taken into account is limited by maxdepth.
The method first seeks for a node with more entries than the minimum number
of entries. If found, it shifts away entries, but only such that a) the
node does not go below the minimum number of entries and b) while shifting
elements over multiple siblings, a node temporarily holds not more elements
than possible (maxEntries+1). The number of elements actually shifted is
written back to maxLoad. If the caller intends to fill up a node with a
certain number of entries, the method must be called multiple times.
If no entry can be moved, the method returns false otherwise it returns
true. However, it also returns true, if the resulting maxLoad is lower than
the initial maxLoad.

*/

    bool RemoveNodeByMerging(int entry, InternalNode* parent);
/*
Try to remove the node entry@parent. This is done by shifting all elements
to the node's siblings. If this cannot be done, the method returns false.

*/
 
    InternalNode* SingleSplit(InternalNode* n,InternalNode* parent);
/*
Split the node ~n~. The median entry is used as splitting element, which
is then moved to the parent node. If the root node is splitted,
~parent~ must be NULL. In this case, a second node (the new root node)
is created. The methods returns a pointer to the new node, which holds
the right part of the original one.

*/

    LeafNode* SingleSplit(LeafNode* n,InternalNode* parent);
/*
Split the node ~n~. The median entry is used as splitting element, which
is copied to the parent node. If the root node is splitted, ~parent~ 
must be NULL. In this case, a second node (the new root node)
is created. The methods returns a pointer to the new node, which holds
the right part of the original one.

*/

    void MultiSplit(LeafNode* n, InternalNode* parent, int entry = -1);
/*
Split the node ~n~ using parts of multiple nodes. The split is done such
that first the node ~n~ is splitted into two nodes using SingleSplit and
then entries of the siblings are shifted to the two nodes until they
do not violate the minimum number of entries criterion. Note, that this
might not be possible, so that after this operation, the BTree might have
two nodes with insufficient number of entries. This can even occur for
f=0.5, when maxCount is odd (not possible for classical btrees). 
If the caller knows the entry of the node ~n~ with respect to ~parent~,
it should be given additionally to ~n~, otherwise it is looked up.

*/

    void MultiSplit(InternalNode* n, InternalNode* parent, int entry = -1);
/*
Split the node ~n~ using parts of multiple nodes. The split is done such
that first the node ~n~ is splitted into two nodes using SingleSplit and
then entries of the siblings are shifted to the two nodes until they
do not violate the minimum number of entries criterion. Note, that this
might not be possible, so that after this operation, the BTree might have
two nodes with insufficient number of entries. This can even occur for
f=0.5, when maxCount is odd (not possible for classical btrees). 
If the caller knows the entry of the node ~n~ with respect to ~parent~,
it should be given additionally to ~n~, otherwise it is looked up.

*/

    void Delete(const KEYTYPE& key, int entry, vector<NodeId>& path);
/*
Delete a specific entry. The caller must ensure, that the entry has
indeed the given ~key~. The path must be a complete path from the
root node to (including) a leaf node. The entry in the leaf node is
removed. If this leads to an underflow, it is first tried to shift
elements from siblings. If this is not possible, it is tries to
merge the node with its siblings. Merging is actually done by 
depopulating the node (all elements are shifted to the siblings).
If this is not possible (e.g. would lead to siblings with overflow),
the node remains in an underflow state. Otherwise, the entry of the
parent node is removed with the same procedure as just described.

If the removed element is the last element of a leaf node, its key
should also occur in an internal node. This key is then replaced
with the key of the left entry (with respect to the removed one).

*/

    bool temporary;  // ??? TODO: understand this parameter
    SmiFileId extendedKeysFileId;         
    SmiFileId extendedValuesFileId;
};

/*
3 Class ~BTreeImpl~: Implementation

3.1 Creation of BTree Objects

*/
template <typename KEYTYPE,typename VALUETYPE>
BTree2Impl<KEYTYPE,VALUETYPE>::BTree2Impl(const string& _keytype, 
                             const string& _valuetype) : cache(this) {
  keytype = _keytype;
  valuetype = _valuetype;
  // Get the ListExpr description of the given types 
  //   (needed for instantiation of Attribute in BTreeEntry)
  ListExpr nlkey = nl->SymbolAtom(_keytype);
  keyTypeListExpr = SecondoSystem::GetCatalog()->NumericType(nlkey);
  ListExpr nlvalue = nl->SymbolAtom(_valuetype);
  valueTypeListExpr = SecondoSystem::GetCatalog()->NumericType(nlvalue);
  temporary = false;
  extendedKeysFileId=0;
  extendedValuesFileId=0;
}

template <typename KEYTYPE,typename VALUETYPE>
BTree2Impl<KEYTYPE,VALUETYPE>::~BTree2Impl() {
  // Write information about cache
  if(file){
    cache.Write(header.cacheFileId,temporary);
  }
  // Clean up remaining nodes (write changes of nodes to disk)
  cache.clear();

  // Close or delete BTree-Files

  if (hasValidBTreeFile()) {
    if (temporary) {
      if(file){
        if (file->IsOpen()) {
          file->Drop();
        }
        cache.DropFile(header.cacheFileId,temporary);  
      }
      if ((extendedKeysFile != 0) && (extendedKeysFile->IsOpen())) {
        extendedKeysFile->Drop();
      }
      if ((extendedValuesFile != 0) && (extendedValuesFile->IsOpen())) {
        extendedValuesFile->Drop();
      }
    } else { 
      if(file){
        if (file->IsOpen()) {
          WriteHeader();
          file->Close();
        }
      }
      if ((extendedKeysFile != 0) && (extendedKeysFile->IsOpen())) {
        extendedKeysFile->Close();
      }
      if ((extendedValuesFile != 0) && (extendedValuesFile->IsOpen())) {
        extendedValuesFile->Close();
      }
    }
    if(file){
      delete file;
    }
    if (extendedKeysFile != 0) delete extendedKeysFile;
    if (extendedValuesFile != 0) delete extendedValuesFile;
  }
}

template <typename KEYTYPE,typename VALUETYPE>
bool BTree2Impl<KEYTYPE,VALUETYPE>::CreateNewBTree(double _fill, 
                                        int _recordSize, 
                                        multiplicityType u,
                                        bool _temporary) {
  // Set necessary parameter
  temporary = _temporary;
  header.fill = _fill;
  recordSize = _recordSize;
  header.multiplicity = u;
  header.maxKeysize = defaultMaxKeysize;
  header.maxValuesize = defaultMaxValuesize;

  // Create the main BTree record file with fixed recordSize
 
  file = new SmiRecordFile(true,recordSize,temporary);
  file->Create();

  // Setup persistent storage for the cache
  header.cacheFileId = cache.Create(temporary);

  // Setup persistent storage for extended keys and values
  // One could skip this, if it is known that it is of no
  // use (e.g. KEYTYPE==int).

  if (BTreeEntry<KEYTYPE,VALUETYPE>::needExtendedKeyFile()) {
    extendedKeysFile = new SmiRecordFile(false,0,temporary);
    extendedKeysFile->Create();
    header.extendedKeysFileId = extendedKeysFile->GetFileId();
  } else {
    header.extendedKeysFileId = 0;
    extendedKeysFile = 0;
  }

  if (BTreeEntry<KEYTYPE,VALUETYPE>::needExtendedValueFile()) {
    extendedValuesFile = new SmiRecordFile(false,0,temporary);
    extendedValuesFile->Create();
    header.extendedValuesFileId = extendedValuesFile->GetFileId();
  } else {
    header.extendedValuesFileId = 0;
    extendedValuesFile = 0;
  }

  // Create the record for the BTree parameter (header's data
  // will be stored, when BTree is destoryed)

  SmiRecord record;
  int RecordSelected = file->AppendRecord( headerId, record);
  assert( RecordSelected );

  if (file->IsOpen()) {
// && (extendedKeysFile->IsOpen()) &&
//       (extendedValuesFile->IsOpen())) {

    // Create a new empty leaf node
    BTreeNode<KEYTYPE,VALUETYPE>* rootNode = createLeafNode();
    
    // which is the new root node 
    header.rootNodeId = rootNode->GetNodeId();

    cache.dispose(rootNode);
    return true;
  } else {
    return false;
  }
} 

template <typename KEYTYPE,typename VALUETYPE>
bool BTree2Impl<KEYTYPE,VALUETYPE>::Open(SmiFileId& f, SmiRecordId _headerId,
                                         int _recordSize) {
  temporary = false; 
  recordSize = _recordSize;
  file = new SmiRecordFile(true,recordSize,temporary);
  if (f != 0) {
    file->Open(f);
  }
  if (file->IsOpen()) {
    // get parameter for BTree
    headerId = _headerId;
    ReadHeader();
    cache.Read(header.cacheFileId, temporary);
    // open the extended files
    extendedKeysFile = 0;
    extendedValuesFile = 0;
    if (header.extendedKeysFileId != 0) {
      extendedKeysFile = new SmiRecordFile(false,0,temporary);
      extendedKeysFile->Open(header.extendedKeysFileId);
    }
    if (header.extendedValuesFileId != 0) {
      extendedValuesFile = new SmiRecordFile(false,0,temporary);
      extendedValuesFile->Open(header.extendedValuesFileId);
    }
  }
  return file->IsOpen();
} 

/*
3.3 Information on the BTree

*/
template <typename KEYTYPE,typename VALUETYPE> 
int BTree2Impl<KEYTYPE,VALUETYPE>::GetMinEntries(bool internal) {
  if (internal) {
    return InternalNode::GetMinEntries(GetRecordSize(),
                              GetFill(),header.maxKeysize,header.maxValuesize);
  } else {
    return LeafNode::GetMinEntries(GetRecordSize(),GetFill()
                                ,header.maxKeysize,header.maxValuesize);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
int BTree2Impl<KEYTYPE,VALUETYPE>::GetMaxEntries(bool internal) {
  if (internal) {
    return InternalNode::GetMaxEntries(recordSize,
                                       header.maxKeysize,header.maxValuesize);
  } else {
    return LeafNode::GetMaxEntries(recordSize,
                                   header.maxKeysize,header.maxValuesize);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
int BTree2Impl<KEYTYPE,VALUETYPE>::GetMinNodeSize()
{
  int minLeafNodeSize = 
      LeafNode::GetSizeOfEmptyNode() +
      LeafNode::GetEntrySizeInRecord(header.maxKeysize,header.maxValuesize);
  int minInternalNodeSize = 
      InternalNode::GetSizeOfEmptyNode() +
      InternalNode::GetEntrySizeInRecord(header.maxKeysize,header.maxValuesize);
  size_t minNodeSize = max(minLeafNodeSize,minInternalNodeSize);
 
  return max(minNodeSize,sizeof(headerS));
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::GetStatistics(NodeId nid,
                                          StatisticStruct& result) {
  StatisticStruct tmp;

  result.Internal.NumberOfNodes = 0;
  result.Leaf.NumberOfNodes = 0;
  result.Internal.UnderflowNodes = 0;
  result.Leaf.UnderflowNodes = 0;
  result.Internal.Entries = 0;
  result.Leaf.Entries = 0;
  result.Internal.MissingEntries = 0;
  result.Leaf.MissingEntries = 0;
  result.Internal.BytesWasted = 0;
  result.Leaf.BytesWasted = 0;

  if (ProbeIsInternal(nid)) {
    InternalNode* n = cache.fetchInternalNode(nid);

    result.Internal.NumberOfNodes++;
    if (n->IsUnderflow()) {
      result.Internal.UnderflowNodes++;
      result.Internal.MissingEntries += n->GetMinEntries() - n->GetCount();
    }
    result.Internal.BytesWasted += n->GetWaste();
    result.Internal.Entries += n->GetCount();

    for (int i = 0; i <= n->GetCount(); i++) { 
      // using extended index: including right pointer
      GetStatistics(n->GetChildNodeByIndex(i), tmp);
      result.Internal.NumberOfNodes += tmp.Internal.NumberOfNodes;
      result.Leaf.NumberOfNodes += tmp.Leaf.NumberOfNodes;
      result.Internal.UnderflowNodes += tmp.Internal.UnderflowNodes;
      result.Leaf.UnderflowNodes += tmp.Leaf.UnderflowNodes;
      result.Internal.Entries += tmp.Internal.Entries;
      result.Leaf.Entries += tmp.Leaf.Entries;
      result.Internal.MissingEntries += tmp.Internal.MissingEntries;
      result.Leaf.MissingEntries += tmp.Leaf.MissingEntries;
      result.Internal.BytesWasted += tmp.Internal.BytesWasted;
      result.Leaf.BytesWasted += tmp.Leaf.BytesWasted;
    }
    cache.dispose(n);
  } else {
    LeafNode* n = cache.fetchLeafNode(nid);

    result.Leaf.NumberOfNodes++;
    if (n->IsUnderflow()) {
      result.Leaf.UnderflowNodes++;
      result.Leaf.MissingEntries += n->GetMinEntries() - n->GetCount();
    }
    result.Leaf.BytesWasted += n->GetWaste();
    result.Leaf.Entries += n->GetCount();
    cache.dispose(n);
  }
}

/*
3.4 Content related queries

*/

template <typename KEYTYPE,typename VALUETYPE> 
Attribute* BTree2Impl<KEYTYPE,VALUETYPE>::GetEntryKey(NodeId nodeId,
                                                          int entryNumber) {

  BTreeNode<KEYTYPE,VALUETYPE>* n = cache.fetchLeafNode(nodeId);
  if (entryNumber >= n->GetCount()) {
    cache.dispose(n);
    return 0;
  }

  BTreeEntry<KEYTYPE,VALUETYPE> const& ent = n->GetEntry(entryNumber);

  cache.dispose(n);

  return entry2Attribute<KEYTYPE>(ent.GetKey());
}

template <typename KEYTYPE,typename VALUETYPE> 
Attribute* BTree2Impl<KEYTYPE,VALUETYPE>::GetEntryKeyInternal(NodeId nodeId,
                                                           int entryNumber) {

  InternalNodeClass<KEYTYPE>* n = cache.fetchInternalNode(nodeId);
  if (entryNumber >= n->GetCount()) {
    cache.dispose(n);
    return 0;
  }

  BTreeEntry<KEYTYPE,NodeId> const& ent = n->GetEntry(entryNumber);

  cache.dispose(n);

  return entry2Attribute<KEYTYPE>(ent.GetKey());
}

template <typename KEYTYPE,typename VALUETYPE> 
Attribute* BTree2Impl<KEYTYPE,VALUETYPE>::GetEntryValue(NodeId nodeId,
                                                          int entryNumber) {

  BTreeNode<KEYTYPE,VALUETYPE>* n = cache.fetchLeafNode(nodeId);
 
  if (entryNumber >= n->GetCount()) {
    cache.dispose(n);
    return 0;
  }

  BTreeEntry<KEYTYPE,VALUETYPE> const& ent = n->GetEntry(entryNumber);

  cache.dispose(n);

  return entry2Attribute<VALUETYPE>(ent.GetValue());
}

template <typename KEYTYPE,typename VALUETYPE> 
NodeId BTree2Impl<KEYTYPE,VALUETYPE>::GetEntryValueInternal(NodeId nodeId,
                                                          int entryNumber) {

  InternalNodeClass<KEYTYPE>* n = cache.fetchInternalNode(nodeId);

  if (entryNumber > n->GetCount()) {
    cache.dispose(n);
    return 0;
  }

  if (entryNumber == n->GetCount()) {
    cache.dispose(n);
    if (n->HasRightPointer()) {
      return n->GetRightPointer();
    }
    return 0;
  }

  BTreeEntry<KEYTYPE,NodeId> const& ent = n->GetEntry(entryNumber);

  cache.dispose(n);

  return ent.GetValue();
}

template <typename KEYTYPE,typename VALUETYPE> 
int BTree2Impl<KEYTYPE,VALUETYPE>::GetNodeEntryCount(NodeId nodeId) 
{
  if (ProbeIsInternal(nodeId))
  {
    InternalNodeClass<KEYTYPE>* n = cache.fetchInternalNode(nodeId); 
    cache.dispose(n);
    return n->GetCount();
  }
  else
  {
    BTreeNode<KEYTYPE,VALUETYPE>* n = cache.fetchLeafNode(nodeId);
    cache.dispose(n);
    return n->GetCount();
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
BTreeNode<KEYTYPE,VALUETYPE>*
  BTree2Impl<KEYTYPE,VALUETYPE>::FindLeftmostLeafNode(const KEYTYPE& key, 
                                              vector<NodeId>& path) {
  NodeId i = FindLeftmostLeafNodeId(key, path);
  return cache.fetchLeafNode(i);
}

template <typename KEYTYPE,typename VALUETYPE> 
BTreeNode<KEYTYPE,VALUETYPE>*
  BTree2Impl<KEYTYPE,VALUETYPE>::FindRightmostLeafNode(const KEYTYPE& key, 
                                     vector<NodeId>& path, bool mustBeSorted) {
  NodeId i = FindRightmostLeafNodeId(key, path);
  return cache.fetchLeafNode(i,mustBeSorted);
}

template <typename KEYTYPE,typename VALUETYPE> 
NodeId BTree2Impl<KEYTYPE,VALUETYPE>::FindLeftmostLeafNodeId(
                                                    const KEYTYPE& key, 
                                                    vector<NodeId>& path) {
  InternalNodeClass<KEYTYPE>* n;
  path.resize(header.treeHeight+1);
  NodeId currentId = header.rootNodeId;
  path[0] = currentId;

  // Descend until leaf is reached

  for (int h = 1; h <= header.treeHeight; h++) {
    n = cache.fetchInternalNode(currentId); 
    assert(n->GetCount() > 0);
    currentId = n->GetLeftmostChildNodeByKey(key); // Next node
    path[h] = currentId;
    cache.dispose(n);
  }

  return currentId;
}

template <typename KEYTYPE,typename VALUETYPE> 
NodeId BTree2Impl<KEYTYPE,VALUETYPE>::FindRightmostLeafNodeId(
                                                    const KEYTYPE& key,
                                                    vector<NodeId>& path) {
  InternalNodeClass<KEYTYPE>* n;
  path.resize(header.treeHeight+1);
  NodeId currentId = header.rootNodeId;
  path[0] = currentId;

  // Descend until leaf is reached

  for (int h = 1; h <= header.treeHeight; h++) {
    n = cache.fetchInternalNode(currentId); 
    currentId = n->GetRightmostChildNodeByKey(key); // Next node
    path[h] = currentId;
    cache.dispose(n);
  }

  return currentId;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::FindLeafNodeId(const KEYTYPE& key, 
                                                    const VALUETYPE& value,
                                                    vector<NodeId>& path,
                                                    const NodeId currentId,
                                                    NodeId& result,
                                                    int h) {
  path[h] = currentId;

  // Use recursion to get the right leaf node id
 
  if (h == header.treeHeight) {
    // Test, if key/value pair is indeed in the current node
    LeafNode* n = cache.fetchLeafNode(currentId);
    cache.dispose(n);
    if (n->HasEntry(key,value)) {
      result = currentId;
      return true;
    }
    // ... is not, so remove this node from the path and track back
    return false;
  } else {
    InternalNode* n = cache.fetchInternalNode(currentId);
    int entry;
    NodeId nextId;

    n->GetLeftmostChildNodeAndIndexByKey(key,nextId,entry);

    // If there are multple entries with the same key, we have
    // to descend at all entries with the same key as well as
    // the first entry with a key which is greater than the key.
   
    int maxvalidentry = entry + 1;   // used for preventing to descend at
                                     // entries, where the key cannot be
                                     // found.
 
    while (!FindLeafNodeId(key,value,path,nextId,result,h+1)) {
      // check if next entry has same key, otherwise return
      entry++;
      if ((entry > maxvalidentry) || (!n->HasIndex(entry))) {
        cache.dispose(n);
        return false;
      } 
      if ((entry < n->GetCount()) && (n->GetEntry(entry).keyEquals(key))) {
        maxvalidentry++;
      }

      nextId = n->GetChildNodeByIndex(entry);
    }
    cache.dispose(n);
    return true;
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
BTree2Iterator BTree2Impl<KEYTYPE,VALUETYPE>::begin() {
  InternalNodeClass<KEYTYPE>* n;

  NodeId currentId = header.rootNodeId;
  
  for (int h = 0; h < header.treeHeight; h++) {
    n = cache.fetchInternalNode(currentId);
    if (n->GetCount() == 0) {
      currentId = n->GetRightPointer();
    } else {
      BTreeEntry<KEYTYPE,NodeId> const& ent = n->GetEntry(0);
      currentId = ent.GetValue();
    }
    cache.dispose(n);
  }

  if (currentId == header.rootNodeId) {
    // If root is leaf, the tree might be empty
    LeafNode* n = cache.fetchLeafNode(currentId);
    int count = n->GetCount();
    cache.dispose(n);
    if (count == 0) {
      return end();
    }
  }

  return BTree2Iterator(this,currentId,0);
}

template <typename KEYTYPE,typename VALUETYPE> 
BTree2Iterator BTree2Impl<KEYTYPE,VALUETYPE>::find(Attribute* key) {
  vector<NodeId> path; 
  KEYTYPE const& k = Attribute2Entry<KEYTYPE>(key);
  BTreeNode<KEYTYPE,VALUETYPE>* n = FindLeftmostLeafNode(k,path);
  cache.dispose(n);
  if (!n->HasKey(k)) {
    return end();
  } else {
    return BTree2Iterator(this,n->GetNodeId(),n->GetLeftmostEntryIndexByKey(k));
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::GetNext(NodeId& nodeId, int& entryNumber) {
  BTreeNode<KEYTYPE,VALUETYPE>* n = cache.fetchLeafNode(nodeId);
  cache.dispose(n);
  entryNumber++;
  if (entryNumber >= n->GetCount()) {
    if (!n->HasRightPointer()) {
      nodeId = 0;
      entryNumber = -1;
      return false;
    } else {
      nodeId = n->GetRightPointer();
      entryNumber = 0;
    }
  }
  return true;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::GetNext(NodeId& nodeId, int& entryNumber,
                                             Attribute*& key,
                                             Attribute*& value) {
  BTreeNode<KEYTYPE,VALUETYPE>* n = cache.fetchLeafNode(nodeId);
  cache.dispose(n);
  entryNumber++;
  if (entryNumber >= n->GetCount()) {
    if (!n->HasRightPointer()) {
      nodeId = 0;
      entryNumber = -1;
      return false;
    } else {
      nodeId = n->GetRightPointer();
      entryNumber = 0;
      n = cache.fetchLeafNode(nodeId);
      cache.dispose(n);
    }
  }
  
  BTreeEntry<KEYTYPE,VALUETYPE> const& ent = n->GetEntry(entryNumber);
  key = entry2Attribute<KEYTYPE>(ent.GetKey());
  value = entry2Attribute<VALUETYPE>(ent.GetValue());
  return true;
}


template <typename KEYTYPE,typename VALUETYPE> 
BTree2Iterator BTree2Impl<KEYTYPE,VALUETYPE>::findLeftBoundary(Attribute* key)
{
  vector<NodeId> path; 
  KEYTYPE const& k = Attribute2Entry<KEYTYPE>(key);
  BTreeNode<KEYTYPE,VALUETYPE>* n = FindLeftmostLeafNode(k,path);
  cache.dispose(n);

  int lIndex = n->GetLeftmostEntryIndexByKey(k);
  if (lIndex==n->GetCount())
    return end();
  else
    return BTree2Iterator(this,n->GetNodeId(), lIndex);
}

template <typename KEYTYPE,typename VALUETYPE> 
BTree2Iterator BTree2Impl<KEYTYPE,VALUETYPE>::findRightBoundary(Attribute* key)
{
  vector<NodeId> path; 
  KEYTYPE const& k = Attribute2Entry<KEYTYPE>(key);
  BTreeNode<KEYTYPE,VALUETYPE>* n = FindRightmostLeafNode(k,path);
  cache.dispose(n);
  int rIndex = n->GetRightmostEntryIndexByKey(k);
  if (rIndex==n->GetCount()) {
    return end();
  } else {
    return BTree2Iterator(this,n->GetNodeId(), rIndex);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::findExactmatchBoundary(Attribute* key, 
                                                BTree2Iterator& startIter, 
                                                BTree2Iterator& finalIter)
{
  vector<NodeId> path; 
  KEYTYPE const& k = Attribute2Entry<KEYTYPE>(key);
  BTreeNode<KEYTYPE,VALUETYPE>* n = FindLeftmostLeafNode(k,path);
  cache.dispose(n);

  int lIndex = n->GetLeftmostEntryIndexByKey(k);
  if ((lIndex==n->GetCount())||(!((n->GetEntry(lIndex)).keyEquals(k))))
  // key not found in btree2
  {
    startIter = end();
    finalIter = end();
  }
  else
  {
  // now find the first key greater then key
    startIter = BTree2Iterator(this,n->GetNodeId(), lIndex);

    int rIndex = lIndex+1;
    if (rIndex==n->GetCount())
    // end of node reached...
    {
      rIndex = 0;
      if (n->HasRightPointer())
      {
        n = cache.fetchLeafNode(n->GetRightPointer());
        cache.dispose(n);
      }
      else
      // end of btree2 reached...
        n = 0;
    }

    if ( (n != 0) && ((header.multiplicity)!=uniqueKey)
      &&((n->GetEntry(rIndex)).keyEquals(k)))
    // more than one key possible and the next one on the right is also equal
    // the key, so find the first key greater then key
    {
      if (n->GetLastEntry().keyEquals(k))
      // the greater key is in another node on the right
      {
        while ((n->HasRightPointer())&&(n->GetLastEntry().keyEquals(k)))
        {
          n = cache.fetchLeafNode(n->GetRightPointer());
          cache.dispose(n);
        }
        rIndex = n->GetRightmostEntryIndexByKey(k);
      }
      else
      // the greater key is inside the node n
      {
        while ((n->GetEntry(rIndex)).keyEquals(k))
          rIndex++;
      }
    }

    if (n!=0)
      finalIter = BTree2Iterator(this,n->GetNodeId(), rIndex);
    else 
      finalIter = end();
  }
}


template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::findRangeBoundary(Attribute* key, 
                                                Attribute* secondKey, 
                                                BTree2Iterator& startIter, 
                                                BTree2Iterator& finalIter)
{
  startIter = end();
  finalIter = end();
  KEYTYPE const& k1 = Attribute2Entry<KEYTYPE>(key);
  KEYTYPE const& k2 = Attribute2Entry<KEYTYPE>(secondKey);

  if (k1<=k2) {
  // range-search not necessary, if secondKey > key!!
    NodeId currentId1 = header.rootNodeId;
    NodeId currentId2 = header.rootNodeId;

    InternalNodeClass<KEYTYPE>* ni;
    int index = 0;
    int h = 0;
    // find internal node, where currentId(key) is different from
    // currentId(secondKey), resp. key and secondKey splits to
    // different son-nodes
    // found internal node is root node of the smallest sub-B-tree
    // that includes key and secondKey
    for (h = 0; (h < header.treeHeight)&&(currentId1==currentId2); h++) {
      ni = cache.fetchInternalNode(currentId1); 
      assert(ni->GetCount() > 0);
      ni->GetLeftmostChildNodeAndIndexByKey(k1, currentId1, index);
      if ((index==ni->GetCount())||((ni->GetEntry(index)).keyGreaterThan(k2)))
        currentId2 = currentId1;
      else
        currentId2 = ni->GetRightmostChildNodeByKey(k2);
      cache.dispose(ni);
    }

    int h1 = h;
    // path to key in the smallest sub-B-tree
    // currentId1 is the leafnode including key
    while (h1 < header.treeHeight)
    {
      ni = cache.fetchInternalNode(currentId1); 
      assert(ni->GetCount() > 0);
      currentId1 = ni->GetLeftmostChildNodeByKey(k1); // Next node
      cache.dispose(ni);
      h1++;
    }

    // path to secondKey in the smallest sub-B-tree
    // currentId2 is the leafnode including secondKey
    while (h < header.treeHeight)
    {
      ni = cache.fetchInternalNode(currentId2); 
      assert(ni->GetCount() > 0);
      currentId2 = ni->GetRightmostChildNodeByKey(k2); // Next node
      cache.dispose(ni);
      h++;
    }


    BTreeNode<KEYTYPE,VALUETYPE>* n;
    n = cache.fetchLeafNode(currentId2); 
    cache.dispose(n);
    // find leafnode-entry of secondKey as endmarking point
    int rIndex = n->GetRightmostEntryIndexByKey(k2);
    if (rIndex==n->GetCount()) {
      finalIter = end();
    } else {
      finalIter = BTree2Iterator(this,n->GetNodeId(), rIndex);
    }
    if (currentId1!=currentId2)
    // load leafnode only if necessary...
    {
      n = cache.fetchLeafNode(currentId1); 
      cache.dispose(n);
    }
    // find leafnode-entry of key as starting point
    int lIndex = n->GetLeftmostEntryIndexByKey(k1);
    if (lIndex==n->GetCount())
      startIter = end();
    else
      startIter = BTree2Iterator(this,n->GetNodeId(), lIndex);
  }
}




/*
3.5 Tree manipulation - Implementation

*/

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::Append(const KEYTYPE& key,
                                               const VALUETYPE& tid) {
  // First, find the leaf node where to put the new key
  // Store the path to the leaf node, 
  //  as necessary changes might propagate backwards along this path
  //
  cache.operationStarts();

  vector<NodeId> path(header.treeHeight+1); 
  int pathpos;     
  BTreeNode<KEYTYPE,VALUETYPE>* n = 0;

  if (header.multiplicity == uniqueKeyMultiData) { 
    n = FindRightmostLeafNode(key,path);
    if (ScanForEntryLeft(n, key, tid)) {
      cache.dispose(n);
      return false;
    }
    n->InsertRightmost(key, tid); 
  } else if (header.multiplicity == uniqueKey) {  
    n = FindLeftmostLeafNode(key,path);
    bool res = n->InsertUnique(key, tid); 
    if (!res) {
      cache.dispose(n);
      return false;
    }
  } else {
    n = FindRightmostLeafNode(key,path,false);
    if (header.multiplicity == stable_multiple) {
      n->InsertRightmost(key,tid);
    } else {
      n->InsertUnsorted(key,tid);
    }
  }

  header.leafEntryCount++; 

  pathpos = header.treeHeight;

  InternalNode* pnode = 0;

  if (n->IsOverflow()) {
    n->checkSort();
    pnode = (pathpos > 0)?cache.fetchInternalNode(path[pathpos-1]):0;
    if (pnode == 0) {
      LeafNode* nnew = SingleSplit(n,0);
      cache.dispose(nnew);
    } else {
      int entry = pnode->GetEntryIndexByValue(n->GetNodeId());
      if (!BalanceOverflow(entry,pnode)) {
        MultiSplit(n,pnode,entry);
      }
    }

    pathpos--;
    InternalNode* m = pnode;
  
    while ((pathpos >= 0) && ((m != 0) && (m->IsOverflow()))) {
      InternalNode* parent = (pathpos > 0) ?
                                    cache.fetchInternalNode(path[pathpos-1]):0;
      if (parent == 0) {
        InternalNode* nnew = SingleSplit(m,0);
        cache.dispose(nnew);
      } else {
        int entry = parent->GetEntryIndexByValue(m->GetNodeId());
        if (!BalanceOverflow(entry,parent)) {
          MultiSplit(m,parent,entry);
        }
      }
      pathpos--;
      cache.dispose(m);
      m = parent;
    }
    if (m != 0) {
      cache.dispose(m);
    }
  }

  cache.dispose(n);

  if (debugPrintTree) { printNodeInfos(); }

  return true;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::AppendGeneric(Attribute* key, 
                                                   Attribute* value) {
 return Append(Attribute2Entry<KEYTYPE>(key),Attribute2Entry<VALUETYPE>(value));
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::DeleteGeneric(Attribute* key) {
 return Delete(Attribute2Entry<KEYTYPE>(key));
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::DeleteGeneric(Attribute* key, 
                                                   Attribute* value) {
 return Delete(Attribute2Entry<KEYTYPE>(key),Attribute2Entry<VALUETYPE>(value));
}

template <typename KEYTYPE, typename VALUETYPE>
bool BTree2Impl<KEYTYPE, VALUETYPE>::UpdateGeneric(Attribute* key, 
                                                    Attribute* value){
  return Update(Attribute2Entry<KEYTYPE>(key), Attribute2Entry<VALUETYPE>
                                                                  (value));
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::Delete(const KEYTYPE& key) {
  vector<NodeId> path;
  BTreeNode<KEYTYPE,VALUETYPE>* n = FindLeftmostLeafNode(key,path);

  if (!n->HasKey(key)) {
    cache.dispose(n);
    return false;
  }
  int leafentry = n->GetLeftmostEntryIndexByKey(key);

  cache.dispose(n);
  Delete(key,leafentry,path);

  return true;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::Delete(const KEYTYPE& key, 
                                           const VALUETYPE& value) {
  
  cache.operationStarts();
  vector<NodeId> path(header.treeHeight+1);
  NodeId leafId;
  bool res = FindLeafNodeId(key,value,path,header.rootNodeId,leafId);

  if (!res) {
    return false;
  }

  LeafNode* n = cache.fetchLeafNode(leafId);

  int leafentry = n->GetEntryIndexByKeyValue(key,value);

  cache.dispose(n);
  Delete(key,leafentry,path);

  return true;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::Update(const KEYTYPE& key, 
                                           const VALUETYPE& value) {
  vector<NodeId> path;
  BTreeNode<KEYTYPE,VALUETYPE>* n = FindLeftmostLeafNode(key,path);

  if (!n->HasKey(key)) {
    cache.dispose(n);
    return Append (key, value);
  }
  else {
    int leafentry = n->GetLeftmostEntryIndexByKey(key); 
    n->SetValue(leafentry, value); 
    cache.dispose(n);
    return true;
  }
  cache.dispose(n);
  return false;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Impl<KEYTYPE,VALUETYPE>::printNodeInfo(NodeId id, int height) {
  if (ProbeIsInternal(id)) {
    InternalNodeClass<KEYTYPE>* n = cache.fetchInternalNode(id);
    printf("Node %d ", n->GetNodeId());
    printf("(#=%d) h=%d ", n->GetCount(), height);
    vector<NodeId> subs;
    printf("Internal");
    for (int i = 0; i < n->GetCount(); i++) {
      BTreeEntry<KEYTYPE,NodeId> const& x = n->GetEntry(i);
      cout << "[" << x.GetKey() << "] ";
      subs.push_back(n->GetEntry(i).GetValue());
    }
    subs.push_back(n->GetRightPointer());
    printf("\n");
    cache.dispose(n);
    for (unsigned int i = 0; i < subs.size(); i++) {
      printf("Descending entry %d\n",i);
      printNodeInfo(subs[i],height+1);
    }
  } else {
    BTreeNode<KEYTYPE,VALUETYPE>* n = cache.fetchLeafNode(id);
    printf("Node %d ", n->GetNodeId());
    printf("(#=%d) ", n->GetCount());
    printf("Leaf: ");
    for (int i = 0; i < n->GetCount(); i++) {
      BTreeEntry<KEYTYPE,VALUETYPE> const& x = n->GetEntry(i);
      cout << x.GetKey() << " ";
    }
    printf("\n");
    cache.dispose(n);
  }
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Impl<KEYTYPE,VALUETYPE>::printNodeInfos() {
  if (dbgPrintTree()) {
    std::cout << "BTree2File" << endl
              << "Filename = " << file->GetContext().c_str() << endl
              << "Filecontext = %s" <<  file->GetContext().c_str() << endl
              << "RecordLength = " <<  file->GetRecordLength() << endl;

    printNodeInfo(header.rootNodeId,1);
  }
}

/*
3.8 Private methods for node information retrieval

*/

template <typename KEYTYPE,typename VALUETYPE> 
int BTree2Impl<KEYTYPE,VALUETYPE>::GetCount(int entry, InternalNode* parent) {
  NodeId nid = parent->GetChildNodeByIndex(entry);

  if (parent->ChildNodeIsLeaf()) {
    LeafNode* n = cache.fetchLeafNode(nid);
    cache.dispose(n);
    return n->GetCount();
  } else {
    InternalNode* n = cache.fetchInternalNode(nid);
    cache.dispose(n);
    return n->GetCount();
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::Overflow(int entry, InternalNode* parent) {
  int c = GetCount(entry,parent);
  return (c > GetMaxCountOfChildren(parent));
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::Underflow(int entry, InternalNode* parent) {
  int c = GetCount(entry,parent);
  return (c < GetMinCountOfChildren(parent));
}

template <typename KEYTYPE,typename VALUETYPE> 
int BTree2Impl<KEYTYPE,VALUETYPE>::GetMaxCountOfChildren(InternalNode* parent) {
  if (parent->ChildNodeIsLeaf()) {
    return LeafNode::GetMaxEntries(recordSize,header.maxKeysize,
                                    header.maxValuesize);
  } else {
    return InternalNode::GetMaxEntries(recordSize,header.maxKeysize,
                                         header.maxValuesize);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
int BTree2Impl<KEYTYPE,VALUETYPE>::GetMinCountOfChildren(
                                             InternalNode* parent) {
  
  if (HasLeafChildren(parent)) {
    return LeafNode::GetMinEntries(GetRecordSize(),GetFill()
                                ,header.maxKeysize,header.maxValuesize);
  } else {
    return InternalNode::GetMinEntries(GetRecordSize(),
                              GetFill(),header.maxKeysize,header.maxValuesize);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::HasLeafChildren(InternalNode* p) {
  return p->ChildNodeIsLeaf();
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::ScanForEntryLeft(LeafNode* n, 
                                  const KEYTYPE& key, const VALUETYPE& tid) {
  do {
    if (n->HasEntry(key, tid)) {
      return true;
    }
    if (n->GetCount() == 0) {
      return false;
    }

    // If the first element has also the requested key, we have to look
    // at preceeding leaf nodes as well

    BTreeEntry<KEYTYPE, VALUETYPE> const& lastEntry = n->GetEntry(0);
    
    if (!lastEntry.keyLessThan(key)) {
      if (!n->HasLeftPointer()) {
        return false;
      } else {
        n = cache.fetchLeafNode(n->GetLeftPointer());
        cache.dispose(n);
      }
    } else {
      return false;
    }
  } while (1 == 1);

  return false;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::ScanForEntryRight(LeafNode* n, 
                                  const KEYTYPE& key, const VALUETYPE& tid) {
  do {
    if (n->HasEntry(key, tid)) {
      return true;
    }
    if (n->GetCount() == 0) {
      return false;
    }

    // If the last element has also the requested key, we have to look
    // at successive leaf nodes as well

    BTreeEntry<KEYTYPE, VALUETYPE> const& lastEntry = n->GetLastEntry();
    
    if (lastEntry.keyEquals(key)) {
      if (!n->HasRightPointer()) {
        return false;
      } else {
        n = cache.fetchLeafNode(n->GetRightPointer());
        cache.dispose(n);
      }
    } else {
      return false;
    }
  } while (1 == 1);

  return false;
}

template <typename KEYTYPE,typename VALUETYPE> 
BTreeNode<KEYTYPE,VALUETYPE>* 
    BTree2Impl<KEYTYPE,VALUETYPE>::fetchLeafNode(int pos,
                                              InternalNode* parent) {
  NodeId i = parent->GetChildNodeByIndex(pos);
  return cache.fetchLeafNode(i);
}

template <typename KEYTYPE,typename VALUETYPE> 
InternalNodeClass<KEYTYPE>* 
   BTree2Impl<KEYTYPE,VALUETYPE>::fetchInternalNode(int pos, 
                                              InternalNode* parent) {
  NodeId i = parent->GetChildNodeByIndex(pos);
  return cache.fetchInternalNode(i);
}

/*
3.9 Private methods for tree manipulation

*/

template <typename KEYTYPE,typename VALUETYPE>
BTreeNode<KEYTYPE,VALUETYPE>* BTree2Impl<KEYTYPE,VALUETYPE>::createLeafNode() {
  SmiRecord record;
  SmiRecordId recno;

  // We create the record to get a valid record number
  // but the record is written when the node is removed
  // from the cache.
 
  file->AppendRecord(recno, record);

  LeafNode* node = new LeafNode(false,(NodeId) recno,this);
  node->WriteRecord(record);        // Necessary to write it here, 
                                    // as ProbeIsInternal might access it.
  cache.addLeafNodeToCache(node);
  header.leafNodeCount++;
  return node;
}

template <typename KEYTYPE,typename VALUETYPE>
InternalNodeClass<KEYTYPE>* BTree2Impl<KEYTYPE,VALUETYPE>::createInternalNode(){
  SmiRecord record;
  SmiRecordId recno;

  // We create the record to get a valid record number
  // but the record is written when the node is removed
  // from the cache.
 
  file->AppendRecord(recno, record);

  InternalNode* node = new InternalNode(true,(NodeId) recno,this);
  node->WriteRecord(record);        // Necessary to write it here, 
                                    // as ProbeIsInternal might access it.

  cache.addInternalNodeToCache(node);
  header.internalNodeCount++;
  return node;
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::GetExtentForBalanceUnderflow(
                        int entry, InternalNode* parent, int count, 
                        int& right_siblings, int&left_siblings) {
  right_siblings = 0;
  left_siblings = 0;
  int maxExtentForBalanceUnderflow = parent->GetMaxEntries(); // No limit

  // We check the siblings of entry@parent (alternating left/right),
  // if they have entries to shift away.
 
  for (int i = 1; 
       (count > 0) && (i < maxExtentForBalanceUnderflow) &&
       (parent->HasIndex(entry+i) || 
       (parent->HasIndex(entry-i))); i++) {
    if (parent->HasIndex(entry+i)) {
      count -= GetCount(entry+i,parent);
      right_siblings = i;
    }
    if (parent->HasIndex(entry-i)) {
      count -= GetCount(entry-i,parent);
      left_siblings = i;
    }
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::GetExtentForSplitting(
                        int entry, InternalNode* parent, int& right_siblings, 
                        int&left_siblings) {
  int maxCount = GetMaxCountOfChildren(parent);
  int minCount = GetMinCountOfChildren(parent);
  int splittingNodes = (maxCount - minCount == 0)?2*maxCount+1:
                         (int) (ceil((minCount-1)/(maxCount-minCount)));

  int maxdepth_r = (int) floor(splittingNodes/2.0);
  int maxdepth_l = (int) floor(splittingNodes/2.0);

  if (entry+maxdepth_r > parent->GetCount()) {
    int overflow = (entry+maxdepth_r - parent->GetCount());
    maxdepth_r -= overflow;
    maxdepth_l += overflow;
    if (entry-maxdepth_l < 0) {
      maxdepth_l -= -(entry-maxdepth_l);
    }
  }

  if (entry-maxdepth_l < 0) {
    int overflow = -(entry-maxdepth_r);
    maxdepth_r += overflow;
    maxdepth_l -= overflow;
    if (entry+maxdepth_r > parent->GetCount()) {
      maxdepth_r -= (entry+maxdepth_r - parent->GetCount());
    }
  }
  right_siblings = maxdepth_r;
  left_siblings = maxdepth_l;
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::ShiftOneElement(int srcpos, 
                                     InternalNode* parent, int dir) {
  int dstpos = srcpos + dir;
  assert(dstpos <= parent->GetCount());
  assert(dstpos >= 0);
  if (HasLeafChildren(parent)) {
    LeafNode* ndst = fetchLeafNode(dstpos,parent);
    LeafNode* nsrc = fetchLeafNode(srcpos,parent);
    if (dir == 1) {
      ndst->MoveEntryToLeftmost(nsrc,nsrc->GetCount()-1);
      if (nsrc->GetCount() > 0) { 
        parent->SetKey(srcpos,nsrc->GetKey(nsrc->GetCount()-1));
      } 
    } else {  
      ndst->MoveEntryToRightmost(nsrc,0);
      parent->SetKey(dstpos,ndst->GetKey(ndst->GetCount()-1));
    }
    cache.dispose(ndst);
    cache.dispose(nsrc);
  } else {
    InternalNode* ndst = fetchInternalNode(dstpos,parent);
    InternalNode* nsrc = fetchInternalNode(srcpos,parent);
    if (dir == 1) {
      ndst->InsertLeftmost(parent->GetKey(srcpos),nsrc->GetRightPointer());
      assert(parent->GetKey(srcpos) == ndst->GetKey(0));
      parent->SetKey(srcpos,nsrc->GetKey(nsrc->GetCount()-1));
      nsrc->SetRightPointer(nsrc->GetChildNodeByIndex(nsrc->GetCount()-1));
      nsrc->DeleteEntry(nsrc->GetCount()-1);
    } else {
      ndst->InsertRightmost(parent->GetKey(dstpos),ndst->GetRightPointer());
      assert(parent->GetKey(dstpos) == ndst->GetKey(ndst->GetCount()-1));
      ndst->SetRightPointer(nsrc->GetChildNodeByIndex(0));
      parent->SetKey(dstpos,nsrc->GetKey(0));
      nsrc->DeleteEntry(0);
    }
    cache.dispose(ndst);
    cache.dispose(nsrc);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::ShiftExceed(int srcpos, 
                          InternalNode* parent, int extent, int dir) {
  int dstpos = srcpos + dir;
  if (extent == 0) {
    return false;
  }

  // If destination node is already full, try if one can shift away 
  // one element from there to the next node.
 
  if (GetCount(dstpos,parent) == GetMaxCountOfChildren(parent)) {
    if (!ShiftExceed(dstpos,parent,extent-1,dir)) {
      return false;
    }
    assert(GetCount(dstpos,parent) == GetMaxCountOfChildren(parent) - 1);
  }

  ShiftOneElement(srcpos,parent,dir);

  return true;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::BalanceOverflow(int entry, 
                                                     InternalNode* parent) {
  if (parent == 0) {         // Root node cannot be balanced
    return false;
  }
  if (header.fill <= 0.5) {  // Do not try to balance for classical btrees
    return false;            //  (split them)
  }

  if (!Overflow(entry,parent)) {  // Do not balance without overflow
    return true;
  }

  int maxdepth_r, maxdepth_l;
  GetExtentForSplitting(entry,parent,maxdepth_r,maxdepth_l);

  // Now we know the remaining capacity of the left and right
  // siblings, try to resolve with right siblings first.

  bool res = ShiftExceed(entry,parent,maxdepth_r,1);
  if (!res) {
    // If not sufficient, try left siblings.
    res = ShiftExceed(entry,parent,maxdepth_l,-1);
  }
  return res;
}


template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::BalanceUnderflow(int entry, 
                                                      InternalNode* parent) {
  int right_siblings;
  int left_siblings;

  int minCount = GetMinCountOfChildren(parent);
  int underflow = minCount - GetCount(entry,parent);
  
  if (underflow < 0) {
    return true;
  }

  GetExtentForBalanceUnderflow(entry,parent,underflow,right_siblings,
                                 left_siblings);

  // Now we know, from how many siblings we have to collect items.
  // underflow gives us the number of missing items

  int maxLoad = underflow;
  while ((underflow > 0) && 
         (CollectItems(entry,parent,1,0,right_siblings,maxLoad))) {
    // After calling collectItems, we requested maxLoad = underflow
    // items, but we might just got a smaller amount of items which is
    // now stored in maxLoad. So we subtract the shifted elements
    // from the number of missing items (underflow) and try again,
    // but just try to grab not more than underflow elements.
    underflow -= maxLoad;
    maxLoad = underflow;
  }

  maxLoad = minCount - GetCount(entry,parent);
  while ((underflow > 0) && 
         (CollectItems(entry,parent,-1,0,left_siblings,maxLoad))) {
    underflow -= maxLoad;
    maxLoad = underflow;
  }

  // Successful, if underflow has been resolved.

  return (!Underflow(entry,parent));
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::CollectItems(int dst, 
         InternalNode* parent, int dir, int depth, int maxdepth, int& maxLoad) {
  // This is a recursive function!
  if (depth >= maxdepth) {
    return false;
  }
  int minCount = GetMinCountOfChildren(parent);

  int srcCount = GetCount(dst+dir,parent);
  if (srcCount > minCount) {
    for (int i = 0; (i < maxLoad) && (i < srcCount - minCount); i++) {
      ShiftOneElement(dst+dir,parent,-dir);
    }
    return true;
  } else {
    int maxCount = GetMaxCountOfChildren(parent);
    maxLoad = max(maxLoad, maxCount - srcCount + 1);
    return CollectItems(dst+dir,parent,dir,depth+1,maxdepth,maxLoad);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Impl<KEYTYPE,VALUETYPE>::RemoveNodeByMerging(int entry, 
                                               InternalNode* parent) {
  int maxdepth_r, maxdepth_l;
  bool isLeaf = HasLeafChildren(parent);
  int maxCount = GetMaxCountOfChildren(parent);

  // How many elements have to go?
  int elems = (isLeaf)?GetCount(entry,parent):GetCount(entry,parent)+1;

  GetExtentForSplitting(entry,parent,maxdepth_r,maxdepth_l);

  // Increase splitting extent by one, if possible
  if (entry+maxdepth_r < parent->GetCount()) {
    maxdepth_r++;
  }
  if (entry-maxdepth_l > 0) {
    maxdepth_l++;
  }

  // Determine how many elements we need to shift
  int elr = 0;
  int ell = 0;

  int shiftsr[maxdepth_r+1];
  int shiftsl[maxdepth_l+1];

  for (int i = 0; i < maxdepth_r; i++) {
    int nc = GetCount(entry+i+1,parent);
    shiftsr[i] = (maxCount - nc);
    elr += (maxCount - nc);
  }

  if (elr < elems) {
    for (int i = entry-1; i >= entry-maxdepth_l; i--) {
      int nc = GetCount(i,parent);
      shiftsl[(entry-1)-i] = (maxCount - nc);
      ell += (maxCount - nc);
    }
  }

  if (elr+ell < elems) {
    // Cannot remove node, would lead to overflow
    return false; 
  } else {

    // Shift away all elements of node to be deleted
   
    for (int i = maxdepth_r; i >= 1; i--) {
      for (int j = 0; j < shiftsr[i-1]; j++) {
        if (elems > 0) {
          // for internal nodes, we have to consider the parental entry
          // we shift away entries of the neighboring nodes (k=1), so that
          // at least one free entry is available for the parental entry
          int kbeg = ((elems == 1) && (!isLeaf))?1:0;
          for (int k = kbeg; k < i; k++) {
            ShiftOneElement(entry+k,parent,1);
          }
          elems--;
        }
      }
    }
    for (int i = maxdepth_l; i >= 1; i--) {
      for (int j = 0; j < shiftsl[i-1]; j++) {
        if (elems > 0) {
          int kbeg = ((elems == 1) && (!isLeaf))?1:0; 
          for (int k = kbeg; k < i; k++) {
            ShiftOneElement(entry-k,parent,-1);
          }
          elems--;
        }
      }
    }

    // Correct left pointer 
    
    if (isLeaf) { 
      LeafNode* n = cache.fetchLeafNode(parent->GetChildNodeByIndex(entry));
      if (n->HasLeftPointer()) {
        LeafNode* npred = cache.fetchLeafNode(n->GetLeftPointer());
        npred->SetRightPointer(n->GetRightPointer());
        npred->SetHasRightPointer(n->HasRightPointer());
        cache.dispose(npred);
      }
      if (n->HasRightPointer()) {
        LeafNode* nsucc = cache.fetchLeafNode(n->GetRightPointer());
        nsucc->SetLeftPointer(n->GetLeftPointer());
        nsucc->SetHasLeftPointer(n->HasLeftPointer());
        cache.dispose(nsucc);
      }
      cache.dispose(n);
    }

    if (!isLeaf) {
      // Find out, whether we have to add parental entry to left or right 
      int dir = 0;
    
      if (parent->HasIndex(entry+1)) {
        if (GetCount(entry+1,parent) < maxCount) {
          dir = 1;
        }
      }

      if (dir == 0) {
        if (parent->HasIndex(entry-1)) {
          if (GetCount(entry-1,parent) < maxCount) {
            dir = -1;
          }
        }
      }

      // Copy parental entry 
      //
      
      InternalNode* x = 
              cache.fetchInternalNode(parent->GetChildNodeByIndex(entry+dir));
      InternalNode* n = 
              cache.fetchInternalNode(parent->GetChildNodeByIndex(entry));
   
      if (entry == parent->GetCount()) {
        if (dir == -1) {
          x->InsertRightmost(parent->GetKey(entry-1),x->GetRightPointer());
        } else {
          x->InsertLeftmost(parent->GetKey(entry-1),x->GetRightPointer());
        }
        header.internalEntryCount++;
        x->SetRightPointer(n->GetRightPointer());
      } else {
        if (dir == -1) {
          x->InsertRightmost(parent->GetKey(entry-1),x->GetRightPointer());
          x->SetRightPointer(n->GetRightPointer());
          header.internalEntryCount++;
          parent->SetKey(entry-1, parent->GetKey(entry)); // entry will be rem.
        } else {
          x->InsertLeftmost(parent->GetKey(entry),n->GetRightPointer());
          header.internalEntryCount++;
        }
      }
      cache.dispose(x);
      cache.dispose(n);
    }

    // Remove parental entry 
    
    if (entry == parent->GetCount()) {
      parent->SetRightPointer(
            parent->GetChildNodeByIndex(parent->GetCount()-1));
      parent->DeleteEntry(entry-1);
      header.internalEntryCount--;  
    } else {
      parent->DeleteEntry(entry);
      header.internalEntryCount--;
    }

    // Check, if root node is empty and discard it if necessary

    if (parent->GetCount() == 0) {
      if (parent->GetNodeId() == header.rootNodeId) {
        header.rootNodeId = parent->GetRightPointer();
        // header.nodeCount--;
        header.internalNodeCount--;
        header.treeHeight--;
      }
    }

  }
  if (isLeaf){
    header.leafNodeCount--;
  }
  else{
    header.internalNodeCount--;
  }
  return true;
   
}

template <typename KEYTYPE,typename VALUETYPE> 
InternalNodeClass<KEYTYPE>*
  BTree2Impl<KEYTYPE,VALUETYPE>::SingleSplit(InternalNodeClass<KEYTYPE>* m,
                                        InternalNodeClass<KEYTYPE>* parent){
  int e = (m->GetCount()/2);   // select splitting entry
  InternalNode* m2 = createInternalNode();
  m2->SetChildNodeIsLeaf(m->ChildNodeIsLeaf());
  m2->MoveEntries(m,e+1,m->GetMaxEntries());  // e+1 goes to new node
  if (m->HasRightPointer()) {
    m2->SetRightPointer(m->GetRightPointer());  // including right pointer
  }

  // now add the element e to the parent node (or create new root node)

  if (parent == 0) { // rootNodeId == n->GetNodeId()) 
    InternalNode* pnode = createInternalNode();  // new root 
    header.rootNodeId = pnode->GetNodeId();
    int entidx = pnode->MoveEntryToLeftmost(m,e);  // move entry

    m->SetRightPointer(pnode->GetChildNodeByIndex(entidx)); 
                                                     // moved entry is now
                                                     // the right pointer
                                                      // of the splitted node
    pnode->SetChildNode(entidx,m->GetNodeId());         // the entry points to
                                                     // the left splitted node
    pnode->SetRightPointer(m2->GetNodeId());        // right pointer to new
                                                      // node
    header.treeHeight++;
    cache.dispose(pnode);

  } else {
                                           // we might need to modify the entry,
                                           // which points to the splitted
                                           // node
    int modfidx = parent->GetEntryIndexByValue(m->GetNodeId());
    parent->SetChildNode(modfidx,m2->GetNodeId());

    BTreeEntry<KEYTYPE,NodeId> const& ent = m->GetEntry(e);
    m->SetRightPointer(ent.GetValue());    // new right pointer is pointer
                                             // of splitted entry

    int en2 = parent->MoveEntry(m,e,modfidx);  // move up
    parent->SetChildNode(en2,m->GetNodeId());  // set pointer to splitted node
  } 
  return m2;
}

template <typename KEYTYPE,typename VALUETYPE> 
BTreeNode<KEYTYPE,VALUETYPE>*
      BTree2Impl<KEYTYPE,VALUETYPE>::SingleSplit(LeafNode* n,
                                                  InternalNode* parent) {
  int e = (n->GetCount()/2)-1;
  LeafNode* nnew = createLeafNode(); 

  nnew->MoveEntries(n,e+1,n->GetMaxEntries());

  // Update all right and left pointers

  if (n->HasRightPointer()) {  
    LeafNode* nr = cache.fetchLeafNode(n->GetRightPointer());
    nr->SetLeftPointer(nnew->GetNodeId());
    nnew->SetRightPointer(n->GetRightPointer());
    cache.dispose(nr);
  }
  n->SetRightPointer(nnew->GetNodeId());
  nnew->SetLeftPointer(n->GetNodeId());

  if (header.rootNodeId == n->GetNodeId()) {  // if leaf is root node
    InternalNodeClass<KEYTYPE>* pnode = createInternalNode(); // new root node
    header.rootNodeId = pnode->GetNodeId();
    BTreeEntry<KEYTYPE,VALUETYPE> const& ent = n->GetEntry(e);
    pnode->InsertLeftmost(ent.GetKey(), n->GetNodeId());
    header.internalEntryCount++;
    pnode->SetRightPointer(nnew->GetNodeId());
    pnode->SetChildNodeIsLeaf(true);
    header.treeHeight++;                         // adding new root node is
                                                 // only operation which
                                                   // increases tree height
    cache.dispose(pnode);
  } else {                                       // add key to parent node
    int modfidx = parent->GetEntryIndexByValue(n->GetNodeId());
    parent->SetChildNode(modfidx,nnew->GetNodeId());

    BTreeEntry<KEYTYPE,VALUETYPE> const& ent = n->GetEntry(e);
    parent->InsertAt(modfidx,ent.GetKey(), n->GetNodeId()); 
    header.internalEntryCount++;
  }

  return nnew;
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::MultiSplit(InternalNode* node, 
                                           InternalNode* parent, int entry) {
  InternalNode* nnew = SingleSplit(node,parent);
  cache.dispose(nnew);
  
  if (parent != 0) {
    if (header.fill > 0.5) {
      int newentry = parent->GetEntryIndexByValue(nnew->GetNodeId());
      if (entry == -1) {
        entry = parent->GetEntryIndexByValue(node->GetNodeId());
      }
      BalanceUnderflow(entry,parent);
      BalanceUnderflow(newentry,parent); 
    }
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::MultiSplit(LeafNode* node, 
                                        InternalNode* parent, int entry) {
  LeafNode* nnew = SingleSplit(node,parent);
  cache.dispose(nnew);
  
  if (parent != 0) {
    if (header.fill > 0.5) {
      if (entry == -1) {
        entry = parent->GetEntryIndexByValue(node->GetNodeId());
      }
      BalanceUnderflow(entry,parent);
      BalanceUnderflow(entry+1,parent);
    }
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Impl<KEYTYPE,VALUETYPE>::Delete(const KEYTYPE& key, int entry,
                                                        vector<NodeId>& path) {
  int pathpos = path.size()-1; 
  LeafNode* n = cache.fetchLeafNode(path[pathpos]);

  BTreeEntry<KEYTYPE,VALUETYPE> const* nreplckey = 0;

  if (entry == n->GetCount()-1) { 
    // if entry is the last entry of a leaf, there should be an entry
    // in the internal nodes with that key which must be replaced by
    // the key of the left neighbor
    if (entry == 0) {
      // This might not be correct here. We would have to remove the
      // node by merging and then delete the entry. 
      if (n->HasLeftPointer()) {
        BTreeNode<KEYTYPE,VALUETYPE>* nx = 
                                  cache.fetchLeafNode(n->GetLeftPointer());
        nreplckey = nx->GetEntryPtr(nx->GetCount()-1);
        cache.dispose(nx);
      } 
    } else {
      nreplckey = n->GetEntryPtr(entry-1);
    }
  }

  n->DeleteEntry(entry); 
  header.leafEntryCount--; 

  InternalNode* pnode = 0;

  pnode = (pathpos > 0)?cache.fetchInternalNode(path[pathpos-1]):0;

  if ((pnode != 0) && (nreplckey != 0)) {
    int entry = pnode->GetEntryIndexByValue(n->GetNodeId());
    if (entry < pnode->GetCount()) {
      if (pnode->GetEntry(entry).keyEquals(key)) {
        pnode->SetKey(entry,nreplckey->GetKey());
        nreplckey = 0;  // only replace once
      }
    }
  }

  cache.dispose(n);

  if ((pnode != 0) && (n->IsUnderflow())) {
    int entry = pnode->GetEntryIndexByValue(n->GetNodeId());
    if (!BalanceUnderflow(entry,pnode)) {
      RemoveNodeByMerging(entry,pnode);
    }
  }

  pathpos--;
  InternalNode* m = pnode;

  // must go up until root or replaced key if nreplckey != 0
  // I guess, loop is pretty complicated, he? 

  while ((pathpos >= 0) && (m != 0) && 
      ((m->IsUnderflow() || (nreplckey != 0)))) {
    pnode = (pathpos > 0)?cache.fetchInternalNode(path[pathpos-1]):0;
    if (pnode != 0) {
      int entry = pnode->GetEntryIndexByValue(m->GetNodeId());
      if ((nreplckey != 0) && (entry < pnode->GetCount())) {
        if (pnode->GetEntry(entry).keyEquals(key)) {
          pnode->SetKey(entry,nreplckey->GetKey());
          nreplckey = 0;  // only replace once
        }
      }
      if (m->IsUnderflow()) {
        if (!BalanceUnderflow(entry,pnode)) {
          RemoveNodeByMerging(entry,pnode);
        }
      }
    }
    pathpos--;
    cache.dispose(m);
    m = pnode;
  }
  if (m != 0) { cache.dispose(m); }
}


} // end namespace btree2algebra

#endif
