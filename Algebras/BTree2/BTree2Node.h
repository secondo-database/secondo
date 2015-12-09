/*
----
This file is part of SECONDO.

Copyright (C) 2004-2010, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Fre PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 Template Header File: BTree2Node

Jan. 2010 M.Klocke, O.Feuer, K.Teufel

1.1 Overview

This class represents a single node of the btree. When representing an
internal node, this class should be instantiated with VALUETYPE=NodeId.

*/

#ifndef _BTREE2_NODE_H_
#define _BTREE2_NODE_H_

#include "BTree2Types.h"
#include "BTree2Entry.h"
#include "BTree2.h"

#include <vector>
#include <algorithm>

namespace BTree2Algebra {

/*
2 Class ~BTreeNode~

*/
template <typename KEYTYPE, typename VALUETYPE>
class BTreeNode 
{
  public:
    BTreeNode(const bool internal, NodeId nid, BTree2* parent);
/*
Creates a new node of the btree given by ~parent~. The node is
initialized as an empty node. 

*/

    ~BTreeNode();
/*
The destructor takes care of writing the contents back to disk. 

*/
    static bool ProbeIsInternal(SmiRecordFile* btreefile, NodeId id);
/*
This method opens a node on disk to check, whether it is an internal node.
Should be avoided, if node is in memory.

*/

    inline int GetSizeInRecord() const;
/*
The size of this node in bytes when stored in a record.

*/

    inline size_t GetMemoryUsage() { return memoryUsage; }
/*
The size of this node in bytes in memory.

*/

    static int GetSizeOfEmptyNode();
/*
Gives the overhead of the node's disk usage.

*/

    static int GetEntrySizeInRecord(int maxk, int maxv);
/*
Gives the size of a single entry in bytes (can be used without an
actual instance of a node)

*/
    int GetEntrySizeInRecord() const;
/*
Gives the size of a single entry in bytes.

*/

    static inline int GetMaxEntries(int recSize, int maxk, int maxv);
/*
Returns the maximum number of entries. The node can
hold one entry more in memory, but not store on disk.
This version can be called without an instance of BTreeNode.

*/

    inline int GetMaxEntries() const { return maxEntries; }
/*
Returns the maximum number of entries.

*/

    static inline int GetMinEntries(int recSize, double f, int maxk, int maxv);
/*
Returns the minimum number of entries.
This version can be called without an instance of BTreeNode.

*/

    inline int GetMinEntries() const { return minEntries; }
/*
Returns the minimum number of entries.

*/

    NodeId GetNodeId() { return myNodeId; }
/*
Returns the node id.

*/

    int GetCount() const { return count; }
/*
Returns the current number of entries.

*/

    bool IsOverflow() const { return count == GetMaxEntries()+1; }
/*
Returns true, if this node has one entry more than it can store. 

*/

    bool IsUnderflow() const { return count < GetMinEntries(); }
/*
Returns true, if this node has less entries than it should have.

*/

    bool IsInternal() const { return internal; }
/*
Returns true, if this node is an internal node.

*/

    bool ChildNodeIsLeaf() const { return childNodeIsLeaf; }
/*
Returns true, if this (internal) node's children are leaf nodes.

*/

    void SetChildNodeIsLeaf(bool x) { childNodeIsLeaf = x; }
/*
Set the flag, which indicates, whether this (internal) node's children 
are leaf nodes.

*/

    bool IsLeaf() const { return !internal; }
/*
Returns true, if this node is a leaf node.

*/

    BTreeEntry<KEYTYPE,VALUETYPE> const& GetEntry(int index) { 
      return *entries[index]; 
    }
/*
Returns a reference to the entry at ~index~. The key and the value
cannot be altered, use ~SetKey~, ~SetValue~ or ~SetChildNode~ instead.

*/

    BTreeEntry<KEYTYPE,VALUETYPE> const& GetLastEntry() { 
      return *entries[count - 1]; 
    }
/*
Returns a reference to the rightmost entry. See also ~GetEntry~.

*/

    BTreeEntry<KEYTYPE,VALUETYPE> const* GetEntryPtr(int index) { 
      return entries[index]; 
    }
/*
Returns a pointer to the entry given by ~index~. See also ~GetEntry~.

*/

    inline BTreeEntry<KEYTYPE,VALUETYPE> const* 
                           GetEntryPtrByValue(const VALUETYPE& n);
/*
Returns a reference of the leftmost entry having a value as given by ~n~.
The key and the value cannot be altered, use ~SetKey~, ~SetValue~ or 
~SetChildNode~ instead.

*/

    inline int GetLeftmostEntryIndexByKey(const KEYTYPE& key);
/*
Returns the index of the leftmost entry which is greater or equals ~key~.
Note that there might be no entry with the speficied key.

*/

    inline int GetRightmostEntryIndexByKey(const KEYTYPE& key);
/*
Returns the index of the rightmost entry which is greater or equals ~key~. 
Note that there might be no entry with the speficied key.

*/

    inline int GetEntryIndexByKeyValue(const KEYTYPE& key,
                                       const VALUETYPE& val);
/*
Returns the index of the leftmost entry which matches ~key~ and ~value~.

*/

    inline int GetEntryIndexByValue(NodeId n);
/*
Returns the index of the leftmost entry which points to node ~n~. This
method must only be called if class is instantiated as internal node
(e.g. VALUETYPE == NodeId).

*/

    KEYTYPE const& GetKey(int index) {
      return entries[index]->GetKey();
    }
/*
Returns a reference to the key at ~index~.

*/

    NodeId GetChildNodeByIndex(int index) {
      return (index == count) ? rightPointer : entries[index]->GetValue();
    }
/*
Returns the node id which is referenced to at ~index~. If ~index~ is
set to the index of the last entry plus one, the right pointer is returned.

*/

    inline NodeId GetLeftmostChildNodeByKey(const KEYTYPE& key);
/*
Returns the child node id according to ~key~ (the leftmost entry
with a key greater or equals to ~key~ will be used).

*/

    inline NodeId GetRightmostChildNodeByKey(const KEYTYPE& key);
/*
Returns the child node id according to ~key~ (the first entry
with a key greater than ~key~ will be used).

*/

    inline void GetLeftmostChildNodeAndIndexByKey(
                                const KEYTYPE& key, NodeId& res, int& entry);
/*
See ~GetLeftmostChildNodeByKey~ but this method also returns the index of the
entry found.

*/

    inline void GetRightmostChildNodeAndIndexByKey(
                                const KEYTYPE& key, NodeId& res, int& entry);
/*
See ~GetRightmostChildNodeByKey~ but this method also returns the index of the
entry found.

*/

    inline bool HasKey(const KEYTYPE& key);
/*
Returns true, if an entry with ~key~ exists.

*/

    inline bool HasEntry(const KEYTYPE& ent, const VALUETYPE& value);
/*
Returns true, if an entry with ~key~ and ~value~ exists.

*/

    bool HasIndex(int i) { return (i >= 0) && (i <= count); }
/*
Returns true, if ~i~ is within valid entry bounds. Care! This includes
the index ~count~, which is out of the entry array bound but stands for
the right pointer in some methods (extended index).

*/

    void SetKey(int index, const KEYTYPE& key) {
      modified = true;
      entries[index]->SetKey(key);
    }
/*
Replaces the key at ~index~ with ~key~. The caller of this method
must ensure, that this will not destroy the order of the array.

*/

    void SetValue(int index, const VALUETYPE& v) {
      modified = true;
      entries[index]->SetValue(v);
    }
/*
Replaces the value at ~index~ with ~value~.

*/

    inline void SetChildNode(int index, const NodeId& v);
/*
Replaces the value at ~index~ with ~value~. This method can be
called with extended indexing (index == count is rightPointer).

*/

    void Read(SmiRecordFile *file, const NodeId pointer);
/*
Read node from disk.

*/

    void ReadInternals(char* buffer, int& offset);
/*
Part 1 of reading from disk: internal flags and values.

*/

    void ReadRecord(SmiRecord& record);
/*
Part 2 of reading from disk: read entries.

*/

    void Write(SmiRecordFile *file, const NodeId pointer );
/*
Write node to disk.

*/

    void WriteRecord(SmiRecord& record );
/*
Write node to disk.

*/
    int GetWaste() { return bytesWasted; }
/*
Returns the difference between used bytes and record size

*/

    inline void InsertLeftmost(const KEYTYPE& key, const VALUETYPE& value);
/*
Insert a new entry with ~key~ and ~value~. 
It will be added at the leftmost position which key is greater or equals 
to ~key~. This method runs in N log N time.

*/

    inline void InsertUnsorted(const KEYTYPE& key, const VALUETYPE& value);
/*
Insert a new entry with ~key~ and ~value~ at the end of the entry array.
This method runs in constant time, but it sets the state of the node to
unsorted, so that before writing this node to disc or splitting it, it
is necessary to call a sorting algorithm on the entries (see checkSort).

*/

    inline void InsertRightmost(const KEYTYPE& key, const VALUETYPE& value);
/*
Insert a new entry with ~key~ and ~value~. 
It will be added at the position right to the rightmost entry with ~key~
(or the biggest entry which is smaller than ~key~).
This method runs in N log N time.

*/

    inline void InsertAt(int idx, const KEYTYPE& key, const VALUETYPE& value);
/*
Insert a new entry with ~key~ and ~value~, so that it is at index ~idx~
afterwards. The caller must ensure that the order is not disturbed.
This method runs in linear time.

*/

    inline bool InsertUnique(const KEYTYPE& key, const VALUETYPE& value);
/*
See ~Insert~, but this method rejects inserting if ~key~ already exists (in
this case, false is returned).
This method runs in N log N time.

*/

    typename std::vector<BTreeEntry<KEYTYPE,VALUETYPE>* >::iterator 
            sortedRangeEnd;
/*
Point up to which the entries are sorted, the rest of the entry array is then
supposed to be unsorted. This way, we do not need to sort the whole array but
just the rest and merge it. This sorting/merging process is (currently) not stable.

*/

    struct pred {
      bool operator() (BTreeEntry<KEYTYPE,VALUETYPE>* i,
                         BTreeEntry<KEYTYPE,VALUETYPE>* j) { 
         return i->keyLessThan(j->GetKey());
      }
    } sortPred;
/*
Predicate function object for sorting the entries by key, used by checkSort method.

*/

    inline void checkSort();
/*
Check, if the node is in an unsorted state and sort it if necessary.
Unsorted states can only occur after calls to InsertUnsorted.

*/

    inline int MoveEntryToLeftmost(BTreeNode<KEYTYPE,VALUETYPE>* src, 
                                    int srcidx);
/*
Moves the entry at ~idx~ from source node to this node. 
The moved entry is inserted at the leftmost position which key is
greater or equals to the key of the source node. 

*/

    inline int MoveEntryToRightmost(BTreeNode<KEYTYPE,VALUETYPE>* src, 
                                    int srcidx);
/*
Moves the entry at ~idx~ from source node to this node. 
It will be added at the position right to the rightmost entry with ~key~
(or the biggest entry which is smaller than ~key~).

*/

    inline int MoveEntry(BTreeNode<KEYTYPE,VALUETYPE>* src, int srcidx,
                         int dstidx);
/*
Moves the entry at ~idx~ from source node to this node, inserting it at
index ~dstidx~. The caller must ensure, that this does not disturb the
order of the array.

*/

    void MoveEntries(BTreeNode<KEYTYPE,VALUETYPE>* src, int start, int end);
/*
Moves a range of entries from source node to this node,starting
at index ~start~, ending (including) at index ~end~. Currently, the
implementation is limited: the destination node must be empty.

*/

    inline bool DeleteEntry(int index);
/*
Removes the entry at ~index~ and discards its contents from memory.
 
*/

    inline bool RemoveEntry(int index);
/*
Removes the entry at ~index~. Its contents is not discarded from memory.
 
*/

    inline void RemoveEntries(int startidx, int endidx);
/*
Removes the entries starting at ~startidx~, ending (including) at ~endidx~. 
The contents of the entries are not discarded from memory.
 
*/

    NodeId GetRightPointer() { return rightPointer; }
/*
Returns the node id of the right pointer.
 
*/

    inline void SetRightPointer(NodeId _rightPointer);
/*
Sets the right pointer.
 
*/

    bool HasRightPointer() { return rightPointer != 0; }
/*
Returs true, if the right pointer has been set.
 
*/

    void SetHasRightPointer(bool x) { if (!x) { rightPointer = 0; } }
/*
Set the right pointer set flag to ~x~.
 
*/

    NodeId GetLeftPointer() { return leftPointer; }
/*
Returns the node id of the left pointer.
 
*/

    void SetLeftPointer(NodeId _leftPointer);
/*
Sets the left pointer.
 
*/

    bool HasLeftPointer() { return leftPointer != 0; }
/*
Returs true, if the left pointer has been set.
 
*/

    void SetHasLeftPointer(bool x) { if (!x) { leftPointer = 0; } }
/*
Set the left pointer set flag to ~x~.
 
*/

  private:
    bool internal;   // Flag, if this is an internal node
    NodeId myNodeId; // Id of this node
    int recordSize;  // RecordSize of the btree
    int count;       // number of entries
    double fill;     // Fill factor of the btree
    int maxEntries;  
    int minEntries;  

    int maxKeysize;   // max. keysize for direct storage
    int maxValuesize; // max. valuesize for direct storage

    ListExpr keyTypeListExpr;  // copy of the type description (for creating
                               // Attributes)

    ListExpr valueTypeListExpr;  // copy of the type description (for creating
                                 // Attributes)

    SmiRecordFile* btreeFile;         // BTree Main File, where node is stored 
    SmiRecordFile* extendedKeysFile;  // Extended Keys File
    SmiRecordFile* extendedValuesFile;// Extended Values File

    std::vector<BTreeEntry<KEYTYPE,VALUETYPE>*> entries;  // Entries

    NodeId rightPointer;       // Right pointer 
    NodeId leftPointer;        // Left pointer
    bool childNodeIsLeaf;      // Flag, if children are leaf nodes

    bool modified;             // Indicates, that changes must be written back
    bool dbgPrintNodeLoading;  // Debug-flag: prints loading/unloading msg.
    size_t memoryUsage;        // Memory usage of this node

    int bytesWasted;           // Number of wasted bytes

    bool unsorted;             // Node is in unsorted state
    bool multipleUnsorted;     // For speed up: false, if there is just one
                               //  entry unsorted
};

/*
3 Class ~BTreeNode~: Implementation

*/
template<class KEYTYPE,typename VALUETYPE>
BTreeNode<KEYTYPE,VALUETYPE>::BTreeNode(const bool _internal, 
                                        NodeId nid,
                                        BTree2* parent) 
    : entries(
          GetMaxEntries(
                parent->GetRecordSize(),
                parent->GetMaxKeysize(),
                parent->GetMaxValuesize())
            +1)
// Initialization of the entries array: add MaxEntries+1 null pointers
{
  myNodeId = nid;
  internal = _internal;
  unsorted = false;
  multipleUnsorted = false;
//  entries = new BTreeEntry<KEYTYPE,VALUETYPE>*[
//          GetMaxEntries(
//                parent->GetRecordSize(),
//                parent->GetMaxKeysize(),
//                parent->GetMaxValuesize())+1];

  // Store information of btree locally
  recordSize = parent->GetRecordSize();
  fill = parent->GetFill();
  maxKeysize = parent->GetMaxKeysize();
  maxValuesize = parent->GetMaxValuesize();
  extendedKeysFile = parent->GetExtendedKeysFile();
  extendedValuesFile = parent->GetExtendedValuesFile();
  btreeFile = parent->GetBTreeFile();
  keyTypeListExpr = parent->GetKeyTypeListExpr();
  valueTypeListExpr = parent->GetValueTypeListExpr();
  dbgPrintNodeLoading = parent->dbgPrintNodeLoading();
  maxEntries = GetMaxEntries(recordSize,maxKeysize,maxValuesize);
  minEntries = GetMinEntries(recordSize,fill,maxKeysize,maxValuesize);

  // Initialize memory usage with sizeof BTreeNode
  memoryUsage = sizeof(*this);
  count = 0;
  modified = true;    // store at least the node structure
  bytesWasted = recordSize - GetSizeOfEmptyNode();

  // Initialize left and right pointer

  rightPointer = 0;
  leftPointer = 0;

  childNodeIsLeaf = false;  // must default to false
}

template<class KEYTYPE,typename VALUETYPE>
BTreeNode<KEYTYPE,VALUETYPE>::~BTreeNode()
{
  // Write back, if necessary
  Write(btreeFile, myNodeId);

  // Remove all entries from memory

  for (int i = 0; i < count; i++) {
    delete entries[i];
  }
  if (dbgPrintNodeLoading) {
    cout << "Removed node " << myNodeId << endl;
  }
}

template<class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetSizeInRecord() const
{
  int size = GetSizeOfEmptyNode();

  size += GetEntrySizeInRecord() * GetMaxEntries();

  return size;
}

template<class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetSizeOfEmptyNode()
{
  // This method is bad, I know...
  // Here, the sizes of all internal storage entries must be added
  
  return sizeof( bool ) +    // internal
         sizeof( int ) +     // count
         sizeof( NodeId ) +  // rightPointer
         std::max(sizeof(NodeId),sizeof(bool));  // leftpointer (for leafs), 
                                            // children are leafs (for internl)
}

template<typename KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetEntrySizeInRecord() const {
  return BTreeEntry<KEYTYPE,VALUETYPE>::GetKeySizeInRecord(maxKeysize)
         + BTreeEntry<KEYTYPE,VALUETYPE>::GetValueSizeInRecord(maxValuesize);
}

template<typename KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetEntrySizeInRecord(int maxk, int maxv) {
  return BTreeEntry<KEYTYPE,VALUETYPE>::GetKeySizeInRecord(maxk)
         + BTreeEntry<KEYTYPE,VALUETYPE>::GetValueSizeInRecord(maxv);
}

//template<class KEYTYPE,typename VALUETYPE>
//int BTreeNode<KEYTYPE,VALUETYPE>::GetMaxEntries() const {
//  return GetMaxEntries(recordSize,maxKeysize,maxValuesize);
//}

template<class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetMaxEntries(int recsize, 
                                                int maxk, int maxv) {
  return (int) floor((recsize - GetSizeOfEmptyNode()) / 
                        GetEntrySizeInRecord(maxk,maxv));
}

//template<class KEYTYPE,typename VALUETYPE>
//int BTreeNode<KEYTYPE,VALUETYPE>::GetMinEntries() const {
//  return GetMinEntries(recordSize,fill,maxKeysize,maxValuesize);
//}

template<class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetMinEntries(int recsize,double fillf,
                                               int maxk, int maxv) {
  // At least one entry must be in the node
  //  (= internal nodes have minimum degree of 2)
  int minEntries = (int) ceil(fillf * GetMaxEntries(recsize,maxk,maxv));
  return (minEntries == 0) ? 1 : minEntries;
}

template<class KEYTYPE,typename VALUETYPE>
NodeId BTreeNode<KEYTYPE,VALUETYPE>::GetLeftmostChildNodeByKey(
                                                   const KEYTYPE& key) {
  int idx = GetLeftmostEntryIndexByKey(key); 
  return GetChildNodeByIndex(idx);
}

template<class KEYTYPE,typename VALUETYPE>
NodeId BTreeNode<KEYTYPE,VALUETYPE>::GetRightmostChildNodeByKey(
                                                   const KEYTYPE& key) {
  int idx = GetRightmostEntryIndexByKey(key); 
  return GetChildNodeByIndex(idx);
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::GetLeftmostChildNodeAndIndexByKey(
                                const KEYTYPE& key, NodeId& res, int& entry) {
  entry = GetLeftmostEntryIndexByKey(key); 
  res = GetChildNodeByIndex(entry);
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::GetRightmostChildNodeAndIndexByKey(
                                const KEYTYPE& key, NodeId& res, int& entry) {
  entry = GetRightmostEntryIndexByKey(key); 
  res = GetChildNodeByIndex(entry);
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::SetChildNode(int index, const NodeId& v) {
  modified = true;
  if (index == count) {    // extended index: if index is out of entry bounds
    rightPointer = v; 
  } else {
    entries[index]->SetValue(v);
  }
}

template<class KEYTYPE,typename VALUETYPE>
bool BTreeNode<KEYTYPE,VALUETYPE>::RemoveEntry(int index) {
  assert( index >= 0 && index < count );  // Extended index not valid here

  memoryUsage -= entries[index]->GetValueSizeInMemory() + 
                    entries[index]->GetKeySizeInMemory();

  // Shift entries linearly
  for (int i = index; i < count - 1; i++) {
    entries[i] = entries[i+1];
  }

  count -= 1;
  modified = true;
  return (count >= GetMinEntries());
}

template<class KEYTYPE,typename VALUETYPE>
bool BTreeNode<KEYTYPE,VALUETYPE>::DeleteEntry(int index) {
  assert( index >= 0 && index < count );
  BTreeEntry<KEYTYPE,VALUETYPE>* todel = entries[index];

  memoryUsage -= entries[index]->GetValueSizeInMemory() + 
                    entries[index]->GetKeySizeInMemory();

  // Shift entries linearly
  for (int i = index; i < count - 1; i++) {
    entries[i] = entries[i+1];
  }

  count -= 1;
  delete todel;

  modified = true;
  return (count >= GetMinEntries());
}

template <class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::RemoveEntries(int start, int end) {
  // Shift entries linearly
  for (int i = 0; i < count-end-1; i++) {
    memoryUsage -= entries[i+start]->GetValueSizeInMemory() + 
                        entries[i+start]->GetKeySizeInMemory();
    entries[start+i] = entries[end+i+1];
  }
  count -= (end-start+1);
  modified = true;
}

template<class KEYTYPE,typename VALUETYPE>
bool BTreeNode<KEYTYPE,VALUETYPE>::HasKey(const KEYTYPE& ent) {
  int i = GetLeftmostEntryIndexByKey(ent);
  // i is extended index (i==count possible) 
  return ((i < count) && (entries[i]->keyEquals(ent)));
}

template<class KEYTYPE,typename VALUETYPE>
bool BTreeNode<KEYTYPE,VALUETYPE>::HasEntry(const KEYTYPE& ent, 
                                              const VALUETYPE& val) {
  int i;
  // Simple sweep
  for (i = 0; i < count; i++) {
    if ((entries[i]->keyEquals(ent)) && (entries[i]->valueEquals(val))) {
      return true;
    }
  }
  return false;
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::InsertAt(int idx, const KEYTYPE& key, 
                                           const VALUETYPE& value) {
  // Insert space for new entry at index findA

//  memmove(entries+idx+1,entries+idx,count-idx);
  for (int i = count; i > idx; i--) {
    entries[i] = entries[i-1];
  }

  // Create a new entry

  BTreeEntry<KEYTYPE,VALUETYPE>* newentry = new BTreeEntry<KEYTYPE,VALUETYPE>();
  newentry->SetKey(key);
  newentry->SetValue(value);
  entries[idx] = newentry;

  // Update counter 
 
  count++;
  memoryUsage += newentry->GetValueSizeInMemory() + 
                   newentry->GetKeySizeInMemory();
  modified = true;
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::InsertLeftmost(const KEYTYPE& key, 
                                           const VALUETYPE& value) {
  // Find suited position
  int findA = GetLeftmostEntryIndexByKey(key);

  InsertAt(findA,key,value);
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::InsertRightmost(const KEYTYPE& key, 
                                           const VALUETYPE& value) {
  // Find suited position
  int findA = GetRightmostEntryIndexByKey(key);

  InsertAt(findA,key,value);
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::InsertUnsorted(const KEYTYPE& key, 
                                           const VALUETYPE& value) {
  if (!unsorted) {
    sortedRangeEnd = entries.begin() + count;
    unsorted = true;
  } else {
    multipleUnsorted = true;
  }
  InsertAt(count,key,value);
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::checkSort() {
  if (multipleUnsorted) {
    typename std::vector<BTreeEntry<KEYTYPE,VALUETYPE>* >::iterator it = 
              entries.begin() + count;
    std::sort(sortedRangeEnd, it,sortPred);
    std::inplace_merge(entries.begin(),sortedRangeEnd,it,sortPred);
    unsorted = false;
    multipleUnsorted = false;
  } else if (unsorted) {
    if (count > 1) {
      BTreeEntry<KEYTYPE,VALUETYPE>* lastX = entries[count-1];
      count--;
      int idx = GetRightmostEntryIndexByKey(lastX->GetKey());
      count++;
      for (int i = count-1; i > idx; i--) {
        entries[i] = entries[i-1];
      }
      entries[idx] = lastX;
    }
    unsorted = false;
  }
}

template<class KEYTYPE,typename VALUETYPE>
bool BTreeNode<KEYTYPE,VALUETYPE>::InsertUnique(const KEYTYPE& key, 
                                           const VALUETYPE& value) {
  int findA = GetLeftmostEntryIndexByKey(key);

  // Do not add, if key already exists

  if ((findA < count) && (entries[findA]->keyEquals(key))) {
    return false;
  } else {
    InsertAt(findA,key,value);
    return true;
  }
}

template<class KEYTYPE,typename VALUETYPE>
bool BTreeNode<KEYTYPE,VALUETYPE>::ProbeIsInternal(SmiRecordFile *file,
                      const NodeId pointer)
{
  SmiRecord record;
  int RecordSelected = file->SelectRecord(pointer, record, SmiFile::ReadOnly );
  assert(RecordSelected);
  bool result;
  
  // Just read the first byte, which contains the internal flag

  int RecordRead = record.Read(&result, sizeof(bool), 0);
  assert( RecordRead );
  return result;
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::Read(SmiRecordFile *file,
                      const NodeId pointer)
{
  SmiRecord record;
  int RecordSelected = file->SelectRecord(pointer, record, SmiFile::ReadOnly );
  assert(RecordSelected);
  ReadRecord(record);
  if (dbgPrintNodeLoading) {
    cout << "Loaded node " << myNodeId << endl;
  }
}

template <class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::ReadInternals(char* buffer, int& offset)
{
  // Caveat! ProbeIsInternal depends on the order here
  // Caveat! SizeOfEmptyNode() depends on entries here
  bool in;
  memcpy( &in, buffer + offset, sizeof( internal ) );
  offset += sizeof( internal );
  assert(in == internal);
  memcpy( &count, buffer + offset, sizeof( count ) );
  offset += sizeof( count );
  memcpy( &rightPointer, buffer + offset, sizeof( NodeId ) );
  offset += sizeof( NodeId );
  if (internal) {
    memcpy( &childNodeIsLeaf, buffer + offset, sizeof( bool ) );
    offset += sizeof( bool );
    leftPointer = 0;
  } else {
    memcpy( &leftPointer, buffer + offset, sizeof( NodeId ) );
    offset += sizeof( NodeId );
    childNodeIsLeaf = false;  // but has basically no meaning
  }
  assert(count >= 0);
  assert(count <= GetMaxEntries() );
}

template <class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::ReadRecord(SmiRecord& record)
{
  int offset = 0;
  int size = GetSizeInRecord();
  char buffer[size + 1];

  int RecordRead = record.Read(buffer, size, offset);
  assert(RecordRead == size);

  ReadInternals(buffer, offset);

  for( int i = 0; i < count; i++ ) {
    entries[i] = new BTreeEntry<KEYTYPE,VALUETYPE>();
    entries[i]->Read(buffer,offset,keyTypeListExpr,valueTypeListExpr,
                      extendedKeysFile, extendedValuesFile);
    memoryUsage += entries[i]->GetValueSizeInMemory() + 
                     entries[i]->GetKeySizeInMemory();
  }
  bytesWasted = recordSize - offset;
  modified = false;
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::Write( SmiRecordFile *file,
                        const SmiRecordId pointer )
{
  if (modified) {
    SmiRecord record;
    int RecordSelected = file->SelectRecord( pointer, record, SmiFile::Update );
    assert( RecordSelected );
    WriteRecord( record );
    modified = false;
  }
}

template<class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::WriteRecord( SmiRecord& record ) {
  if( modified ) {
    int offset = 0;
    int size = GetSizeInRecord();
    char buffer[size + 1];
    memset( buffer, 0, size + 1 );

    // Caveat! ProbeIsInternal depends on the order here
    // Caveat! SizeOfEmptyNode() depends on entries here
   
    // Writes internals
    memcpy( buffer + offset, &internal, sizeof( internal) );
    offset += sizeof( internal );
    memcpy( buffer + offset, &count, sizeof( count ) );
    offset += sizeof( count );
    memcpy( buffer + offset, &rightPointer, sizeof( NodeId ) );
    offset += sizeof( NodeId );
    if (internal) {
      memcpy( buffer + offset, &childNodeIsLeaf, sizeof( bool ) );
      offset += sizeof( bool );
    } else {
      memcpy( buffer + offset, &leftPointer, sizeof( NodeId ) );
      offset += sizeof( NodeId );
    }

    assert(offset <= GetSizeOfEmptyNode());
    assert(count <= GetMaxEntries() );

    // Now write the entry array.
    checkSort();
    for( int i = 0; i < count; i++ ) {
      entries[i]->Write(buffer, offset, keyTypeListExpr,valueTypeListExpr, 
                         extendedKeysFile, extendedValuesFile,
                         maxKeysize, maxValuesize);
    }

    bytesWasted = recordSize - offset;
    assert(offset <= size);
    assert(size <= recordSize);
    int RecordWritten = record.Write( buffer, size, 0 );
    assert( RecordWritten == size );
    modified = false;
  }
}

template <class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::MoveEntries(
                    BTreeNode<KEYTYPE,VALUETYPE>* src, int start, int end) {
  if (count == 0) { // Easy: just copy
    for (int i = start; i <= end; i++) {
      entries[i-start] = src->entries[i];
      memoryUsage += entries[i-start]->GetValueSizeInMemory() + 
                     entries[i-start]->GetKeySizeInMemory();
    }

    count = end-start+1;
    modified = true;

    src->RemoveEntries(start,end);

  } else {
    // TODO: insert and sort entries array
    const bool TODO = false;
    assert(TODO);
  }
}

template <class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::MoveEntryToLeftmost(
                 BTreeNode<KEYTYPE,VALUETYPE>* src, int srcidx) {
  return MoveEntry(src,srcidx,-1);
}

template <class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::MoveEntryToRightmost(
                 BTreeNode<KEYTYPE,VALUETYPE>* src, int srcidx) {
  return MoveEntry(src,srcidx,-2);
}

template <class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::MoveEntry(
                                 BTreeNode<KEYTYPE,VALUETYPE>* src, int srcidx,
                                 int dstidx) {
  int i;
  BTreeEntry<KEYTYPE,VALUETYPE> const& ent = src->GetEntry(srcidx);
  int findA;

  if (dstidx == -1) {
    findA = GetLeftmostEntryIndexByKey(ent.GetKey()); 
  } else if (dstidx == -2) {
    findA = GetRightmostEntryIndexByKey(ent.GetKey()); 
  } else {
    findA = dstidx;
  }

  for (i = count; i > findA; i--) {
    entries[i] = entries[i-1];
  }

  entries[i] = src->entries[srcidx];

  src->RemoveEntry(srcidx);

  count++;
  modified = true;


  memoryUsage += entries[findA]->GetValueSizeInMemory() + 
                 entries[findA]->GetKeySizeInMemory();

  return i;
}

template <class KEYTYPE,typename VALUETYPE>
BTreeEntry<KEYTYPE,VALUETYPE> const* 
      BTreeNode<KEYTYPE,VALUETYPE>::GetEntryPtrByValue(const VALUETYPE& ns) {
  for (int i = 0; i < count; i++) {
    if (entries[i]->valueEquals(ns)) {
      return entries[i];
    } 
  }
  return 0;
}

template <class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetEntryIndexByValue(NodeId nid) {
  // Using extended index
  if (rightPointer == nid) {
    return count;
  } else {
    for (int i = 0; i < count; i++) {
      if (entries[i]->valueEquals(nid)) {
        return i;
      } 
    }
  }
  assert(false);
  return -1;
}

template <class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetLeftmostEntryIndexByKey(
                                         const KEYTYPE& ent) {
  int i;
  int findA = 0;
  int findB = count - 1;

  // Fast find of right index: 
  //   after this loop, findA == findB+1
  //   and findA is the first/leftmost entry where all keys left are smaller

  while (findB >= findA) {
    i = (findB+findA)/2;
    if (entries[i]->keyLessThan(ent)) {
      findA = i+1;
    } else {
      findB = i-1;
    }
  }

  return findA;
}

template <class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetRightmostEntryIndexByKey(
                                               const KEYTYPE& ent) {
  int i;
  int findA = 0;
  int findB = count - 1;
  // Fast find of right index: 
  //   after this loop, findA == findB+1
  //   and findA is the first/leftmost entry where all keys left are smaller
  //   or equal

  while (findB >= findA) {
    i = (findB+findA)/2;
    if (entries[i]->keyGreaterThan(ent)) {
      findB = i-1;
    } else {
      findA = i+1;
    }
  }

  return findA;
}

template <class KEYTYPE,typename VALUETYPE>
int BTreeNode<KEYTYPE,VALUETYPE>::GetEntryIndexByKeyValue(const KEYTYPE& key,
                                                const VALUETYPE& value) {
  int i;
  for (i = 0; i < count; i++) {
    if (entries[i]->keyEquals(key)) {
      if (entries[i]->valueEquals(value)) {
        return i;
      }
    }
  }
  return -1;
}

template <class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::SetRightPointer(NodeId _rightPointer) { 
  assert(_rightPointer != myNodeId); 
  rightPointer = _rightPointer; 
  modified = true; 
}

template <class KEYTYPE,typename VALUETYPE>
void BTreeNode<KEYTYPE,VALUETYPE>::SetLeftPointer(NodeId _leftPointer) { 
  assert(_leftPointer != myNodeId); 
  assert(!internal);
  leftPointer = _leftPointer; 
  modified = true; 
}


} // end namespace 
#endif
