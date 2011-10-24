/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
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

This class provides methods for loading nodes into memory. Nodes
are kept in memory within certain limits. One can specify, whether
limitation is applied to the number of nodes kept in memory
or the amount of memory which can be used. 

If the cache is full, nodes must be removed from the cache. For this,
the class keeps track of the access to nodes. This is done by storing
the cached nodes in a linear list and each access shifts the specific
entry to the front of that list. The least used node therefore moves
slowly to the back of the list from which it can be deleted if necessary.

This process can be altered - nodes can be pinned which means that they
won't be removed from memory unless the BTree is removed from memory.

*/

#ifndef _BTREE2_CACHECLASS_H_
#define _BTREE2_CACHECLASS_H_

using namespace std;

#include "BTree2.h"
#include "BTree2Types.h"
#include "BTree2Node.h"
#include "BTree2Entry.h"

#include "Attribute.h"
#include "TupleIdentifier.h"

#include "SecondoSMI.h"

#include <map>
#include <vector>
#include <queue>
#include <deque>

namespace BTree2Algebra {


/*
2 Class ~BTreeCache~

*/
template<typename KEYTYPE>
class InternalNodeClass: public BTreeNode<KEYTYPE, NodeId>{

  public:
     InternalNodeClass(const bool internal, NodeId nid, BTree2* parent):
     BTreeNode<KEYTYPE,NodeId>(internal, nid, parent) {}

};



template <typename KEYTYPE,typename VALUETYPE>
class BTree2Cache {
  private:
    typedef InternalNodeClass<KEYTYPE> InternalNode;
    typedef BTreeNode<KEYTYPE, VALUETYPE> LeafNode;

    static const bool operationRelatedHitCounting = false;
/*
If set to true, a cache hit is only counted once per tree operation (e.g.
a call to Append or Delete).

*/
    static const bool historyDelayedHitCounting = false;
/*
If set to true, a cache hit is only counted after it has not beed loaded
for some time (see below).

*/
    static const int cacheHitCounterThreshold = 0; 
/*
The caller might use the fetch methods several times in one routine,
instead of storing and not disposing the pointer accurately. Therefore, a
cache hit is only counted, if there was access to at least 
~cacheHitCounterThreshold~ other nodes before. 
If ~operationRelatedHitCounting~ is set to true, this constant is not used.

*/

  public:
    BTree2Cache(BTree2* p);
/*
Creates a new instance of the cache. 

*/

    ~BTree2Cache();
/*
Destructor: removes all cached nodes from memory. 

*/

    InternalNode* fetchInternalNode(const NodeId nodeId);   
/*
Retrieves an internal node with id ~nodeId~. If the node
is in memory, it will not be loaded from disk.
Caveat! After calling this method, ~dispose~ must be called,
if the node is not used anymore by the caller.

*/

    LeafNode* fetchLeafNode(const NodeId nodeId, bool mustBeSorted = true); 
/*
Retrieves a leaf node with id ~nodeId~. If the node
is in memory, it will not be loaded from disk.
Caveat! After calling this method, ~dispose~ must be called,
if the node is not used anymore by the caller.

*/

    void dispose(const NodeId);
    void dispose(InternalNode*);
    void dispose(LeafNode*);
/*
The dispose method must be called after retrieving a node
with one of the fetch methods. 

*/

    bool IsFixedElementsLimitType() {
      return cacheWithFixedElements;
    }

    void SetMemoryLimitType() {
      cacheWithFixedElements = false;
    }

    void SetFixedElementsLimitType() {
      cacheWithFixedElements = true;
    }

    void SetCacheSize(size_t mem) {
      if (cacheWithFixedElements) {
        cacheElementLimit = mem;
      } else {
        cacheMemoryLimit = mem;
      }
      checkLimits();
    }
/*
Sets the cache size. TODO: this method should be altered.

*/

    size_t GetCacheSize() {
      return (cacheWithFixedElements)?cacheElementLimit:cacheMemoryLimit;
    }
/*
Returns the current cache size. Depending on the current type, it returns
either the maximum number of cache elements or the maximum cache memory usage.

*/

    int GetCacheHitCounter() { return cacheHitCounter; }
/*
Returns the number of events, in which a node was returned from memory instead
of loading it from disc. 

*/

    void resetCacheHitCounter() { cacheHitCounter = 0; }
/*
Set the cache hit counter to zero.

*/

    bool addPinnedNode(NodeId n, bool internal); 
/*
Mark the node ~n~ as a pinned node, if possible. The node is loaded
if not already in the cache. If the cache is overfull, this method 
returns false and the node is not pinned.

*/

    bool removePinnedNode(NodeId n);
/*
Unmark the node ~n~ as a pinned node. Returns false, if ~n~ is not
in the cache.

*/

    bool hasUndisposedThings();
/*
Returns true, if there are nodes in the cache which have not
yet been freed with a call of ~dispose~. For debugging.

*/

    void printCache();
/*
Writes the contents of the cache to stdout, if the debugPrintCache
flag of the BTree Class is set.

*/

    SmiFileId Create(bool temporary);
/*
Creates a new persistent storage container for cache parameter.

*/

    void Read(SmiFileId fid, bool temporary);
/*
Read cache parameter.

*/

    void Write(SmiFileId fid, bool temporary);
/*
Write cache parameter.

*/

    void DropFile(SmiFileId fid, bool temporary);
/*
Delete the persistent storage container.

*/
   
    std::set<BTree2::PinnedNodeInfo> const* GetPinnedNodes() 
    { return &pinnedNodes; }
/*
Returns the current cache size. Depending on the current type, it returns
either the maximum number of cache elements or the maximum cache memory usage.

*/

    void addInternalNodeToCache(InternalNode* x);                   
/*
Add the node to the list of cached nodes.

*/

    void addLeafNodeToCache(LeafNode* x);
/*
Add the node to the list of cached nodes.

*/

    void checkLimits();     
/*
Checks, if cache memory usage has been exceeded. If so, nodes
will be removed.

*/
    void clear();     
/*
Remove all nodes from memory.

*/

    size_t GetPeakCacheMemoryUsage() { return peakCacheMemoryUsage; }
/*
Returns the peak memory usage.

*/

    int GetPeakCacheElements() { return peakCacheElements;}
/*
Returns the peak number of cached nodes.

*/

    void operationStarts() { cacheHitInThisOperation.clear(); }
/*
Should be called at the beginning of an higher level operation
(e.g. append), so that cache hits are only counted once.

*/

private:
    struct cacheS {  // Structure for local caching of nodes
      NodeId nodeId;
      bool internal;
      int garbageCount;
      bool pinned;
      size_t memoryUsage;
      int historyListCount;
      union {
        InternalNode* internalNode;
        BTreeNode<KEYTYPE,VALUETYPE>* leafNode;
      };
     };


    bool popCache(); 
/*
Add the node to the list of cached nodes.

*/

    void doOperationRelatedHitCounting(const NodeId nodeId, cacheS* s);
/*
Increment hit counter if necessary.

*/

    void doHistoryDelayedHitCounting(const NodeId nodeId, cacheS* s);
/*
Increment hit counter if necessary.

*/


    std::deque<NodeId> historyList;
    std::map<NodeId,cacheS*> nodesInMemory;      // Cache entries

    bool cacheWithFixedElements;          // Cache limit type

    int cacheElements;                    // size of cache (number of nodes)
    int cacheElementLimit;                 // limit of the number of elements 
    size_t cacheMemoryUsage;              // size of cache in memory
    size_t cacheMemoryLimit;           // limit of the memory usage

    size_t peakCacheMemoryUsage;       // For statistics
    int peakCacheElements;             // For statistics

    size_t pinnedNodexMemoryUsage;        // memory usage of all pinned nodes
    
   
    std::set<BTree2::PinnedNodeInfo> pinnedNodes; // List of pinned nodes

    std::set<NodeId> cacheHitInThisOperation;
    unsigned int cacheHitCounter;         // cache hit counter (see above)
                                            
    BTree2* btree;
};

template <typename KEYTYPE,typename VALUETYPE>
BTree2Cache<KEYTYPE,VALUETYPE>::BTree2Cache(BTree2* _btree) {
  cacheElements = 0;
  cacheMemoryUsage = 0;

  // Using fixed elements can be useful for speed up, if key/values have
  // fixed sizes (do not need to call getMem etc.) Maybe useless.
  cacheWithFixedElements = false;
  cacheElementLimit = 300;
  cacheMemoryLimit = qp->FixedMemory();
  cacheHitCounter = 0;
  peakCacheMemoryUsage = 0;
  peakCacheElements = 0;
  btree = _btree;
}

template <typename KEYTYPE,typename VALUETYPE>
BTree2Cache<KEYTYPE,VALUETYPE>::~BTree2Cache() {
}

template <typename KEYTYPE,typename VALUETYPE>
SmiFileId BTree2Cache<KEYTYPE,VALUETYPE>::Create(bool temporary) {
  // Just create the parameter file here
  SmiRecordFile* f = new SmiRecordFile(false,0,temporary);
  f->Create();
  SmiFileId fid = f->GetFileId();
  f->Close();
  delete f;
  return fid;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::DropFile(SmiFileId fid,bool temporary) {
  SmiRecordFile* f = new SmiRecordFile(false,0,temporary);
  f->Open(fid);
  f->Drop();
  delete f;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::Read(SmiFileId fid, bool temporary) {
  SmiRecordFile* f = new SmiRecordFile(false,0,temporary);
  int offset = 0;
  f->Open(fid);
  SmiRecord record;
  SmiRecordFileIterator fit;
  SmiRecordId recno = 1;

  bool RecordSelected = f->SelectRecord(recno, record, SmiFile::ReadOnly);

  // If the record is not there, we can just skip the reading (using defaults)
  if (RecordSelected) {
    record.Read(&cacheWithFixedElements, sizeof(bool), offset);
    offset += sizeof(bool);
    
    record.Read(&cacheElementLimit, sizeof(int), offset);
    offset += sizeof(int);
    
    record.Read(&cacheMemoryLimit, sizeof(size_t), offset);
    offset += sizeof(size_t);
   
    record.Read(&cacheHitCounter, sizeof(unsigned int), offset);
    offset += sizeof(unsigned int);

    record.Read(&peakCacheMemoryUsage, sizeof(size_t), offset);
    offset += sizeof(size_t);
   
    record.Read(&peakCacheElements, sizeof(int), offset);
    offset += sizeof(int);

    // Get list of pinned nodes
    
    int size;
    NodeId id;
    size_t mem;
    record.Read( &size, sizeof(int), offset);
    offset += sizeof(int);
    for (int i = 0; i < size; i++){
      record.Read( &id, sizeof(NodeId), offset);
      offset += sizeof(NodeId);
      record.Read( &mem, sizeof(size_t), offset);
      offset += sizeof(size_t);
      BTree2::PinnedNodeInfo pni(id, mem);
      pinnedNodes.insert(pni);
    }
  }

  f->Close();
  delete f;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::Write(SmiFileId fid, bool temporary) {
  SmiRecordFile* f = new SmiRecordFile(false,0,temporary);
  int offset = 0;
  f->Open(fid);
  SmiRecord record;
  SmiRecordFileIterator fit;

  bool RecordSelected = f->SelectRecord(1, record, SmiFile::Update);
  
  if (!RecordSelected) {  // Create record if necessary
    SmiRecordId recno = 1;
    f->AppendRecord(recno,record);
  }

  record.Write(&cacheWithFixedElements, sizeof(bool), offset);
  offset += sizeof(bool);
  
  record.Write(&cacheElementLimit, sizeof(int), offset);
  offset += sizeof(int);
  
  record.Write(&cacheMemoryLimit, sizeof(size_t), offset);
  offset += sizeof(size_t);
 
  record.Write(&cacheHitCounter, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int); 

  record.Write(&peakCacheMemoryUsage, sizeof(size_t), offset);
  offset += sizeof(size_t);
 
  record.Write(&peakCacheElements, sizeof(int), offset);
  offset += sizeof(int); 

  // Write pinned nodes
  typename std::set<BTree2::PinnedNodeInfo>::iterator it;
  BTree2::PinnedNodeInfo pni;
  int size = pinnedNodes.size();
  NodeId id;
  size_t mem;
  record.Write( &size, sizeof(int), offset);
  offset += sizeof(int);
  int i = 0;
  for (it = pinnedNodes.begin(); it != pinnedNodes.end(); it++){
    pni = *it;
    id = pni.id;
    record.Write( &id, sizeof(NodeId), offset);
    offset += sizeof(NodeId);
    mem = pni.memoryUsage;
    record.Write( &mem, sizeof(size_t), offset);
    offset += sizeof(size_t);
    i++;
  }

  f->Close();

  delete f;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::clear() {
  // Clean up cache: Remove everything
  pinnedNodes.clear();
  typename std::map<NodeId,cacheS*>::iterator it;

  for (it = nodesInMemory.begin(); it != nodesInMemory.end(); it++) {
    cacheS* s = it->second;
    if (s->garbageCount > 0) {
      cerr << "Internal software error: Cache memory leakage. Maybe you " <<
              "forgot a dispose? [" << s->nodeId << "/" 
           << s->garbageCount << "]" << endl;
    }
    if (s->internal) {
      delete s->internalNode;
    } else {
      delete s->leafNode;
    }
    delete s;
  }
  // historyList.clear();
  cacheElements = 0;
  cacheMemoryUsage = 0;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::addInternalNodeToCache(InternalNode* n) {

   cacheS* s = new cacheS();
   s->nodeId = n->GetNodeId();
   s->internal = true;
   s->internalNode = n;
   s->garbageCount = 1;
   s->memoryUsage = n->GetMemoryUsage();
   s->historyListCount = 1;
   historyList.push_back(s->nodeId);
   //historyList.push(s->nodeId);
   BTree2::PinnedNodeInfo pni(s->nodeId, s->memoryUsage);
   s->pinned = (pinnedNodes.find(pni) != pinnedNodes.end());
   nodesInMemory.insert(pair<NodeId,cacheS*>(s->nodeId,s));
   cacheElements++;
   cacheMemoryUsage += s->memoryUsage;
   if (operationRelatedHitCounting) {
     cacheHitInThisOperation.insert(n->GetNodeId());
   }
   assert(n->IsInternal());
   checkLimits();
   printCache();
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::addLeafNodeToCache(LeafNode* n) {
 
   cacheS* s = new cacheS();
   s->nodeId = n->GetNodeId();
   s->internal = false;
   s->garbageCount = 1;
   s->leafNode = n;
   s->memoryUsage = n->GetMemoryUsage();
   s->historyListCount = 1;
   historyList.push_back(s->nodeId);
   //historyList.push(s->nodeId);
   BTree2::PinnedNodeInfo pni(s->nodeId, s->memoryUsage);
   s->pinned = (pinnedNodes.find(pni) != pinnedNodes.end());
   nodesInMemory.insert(pair<NodeId,cacheS*>(s->nodeId,s));
   cacheElements++;
   cacheMemoryUsage += s->memoryUsage;
   if (operationRelatedHitCounting) {
     cacheHitInThisOperation.insert(n->GetNodeId());
   }
   assert(!n->IsInternal());
   checkLimits();
   printCache();
}

template <typename KEYTYPE,typename VALUETYPE>
bool BTree2Cache<KEYTYPE,VALUETYPE>::popCache() {
  typename std::map<NodeId,cacheS*>::iterator it;
  int counter = 0;
  int historySize = historyList.size();

  while (counter < historySize) {
    NodeId n = historyList.front();
    historyList.pop_front();
    //historyList.pop();
    counter++;
   
    it = nodesInMemory.find(n);
//    assert(it != nodesInMemory.end());

    cacheS* s = it->second;

    s->historyListCount--;
    if (s->historyListCount == 0) {
      if ((s->garbageCount > 0) || s->pinned) {
        historyList.push_back(n);
        //historyList.push(n);
        s->historyListCount++;
      } else {
        if (s->internal) {
          cacheMemoryUsage -= s->memoryUsage;
          delete s->internalNode;
        } else {
          cacheMemoryUsage -= s->memoryUsage;
          delete s->leafNode;
        }
        cacheElements--;
        nodesInMemory.erase(it);
        delete s;
        return true;
      }
    }
  } 
  return false;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::checkLimits() {
  if (cacheWithFixedElements) {
    if (cacheElements > cacheElementLimit) {
      popCache();
    }
  } else {
    bool res = true; 
    // Remove elements until memory usage is small enough or
    // no element can be removed (e.g. all pinned)
    while ((cacheMemoryUsage > cacheMemoryLimit) && (res)) {
      res = popCache();
    }
  }
  
  // For statistics

  if (cacheMemoryUsage > peakCacheMemoryUsage) {
    peakCacheMemoryUsage = cacheMemoryUsage;
  }
  if (cacheElements > peakCacheElements) {
    peakCacheElements = cacheElements;
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Cache<KEYTYPE,VALUETYPE>::doOperationRelatedHitCounting(
                                                         const NodeId nodeId,
                                                         cacheS* s) {
  if (cacheHitInThisOperation.find(nodeId) == 
                             cacheHitInThisOperation.end()) {
    cacheHitCounter++;
    cacheHitInThisOperation.insert(nodeId);
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Cache<KEYTYPE,VALUETYPE>::doHistoryDelayedHitCounting(
                                                         const NodeId nodeId,
                                                         cacheS* s) {
  // Check history list, if the last entries have the
  // same node id (if so, do not count it as cache
  // hit)
  
  static NodeId last[cacheHitCounterThreshold];
  static bool init = false;
  static int idx = 0;

  if (!init) {
    for (unsigned int i = 0; i < cacheHitCounterThreshold; i++) {
      last[i] = 0;
    }
  }

  bool found = false;
  for (int i = 0; i < cacheHitCounterThreshold; i++) {
    if (last[i] == nodeId) {
      found = true;
    }
  }
  last[idx] = nodeId;
  idx++;
  if (idx >= cacheHitCounterThreshold) {
    idx++;
  }
  if (!found) {
    cacheHitCounter++;
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
BTreeNode<KEYTYPE,VALUETYPE>* 
        BTree2Cache<KEYTYPE,VALUETYPE>::fetchLeafNode(const NodeId nodeId,
                                                      bool mustBeSorted) {
  
  btree->IncNodesVisitedCounter();
  typename std::map<NodeId,cacheS*>::iterator it;

  it = nodesInMemory.find(nodeId);

  if (it != nodesInMemory.end()) {
    cacheS* s = it->second;
    // We will return a pointer to that node, so inc. reference count
    s->garbageCount++;
    
    // Node might have altered its size while kept in cache: update it
    cacheMemoryUsage -= s->memoryUsage;
    s->memoryUsage = s->leafNode->GetMemoryUsage();
    cacheMemoryUsage += s->memoryUsage;

    // Usage list:
    historyList.push_back(nodeId);
    s->historyListCount++;

    // Complicated cache hit counting
    if (operationRelatedHitCounting) {
      doOperationRelatedHitCounting(nodeId,s);
    } else if (historyDelayedHitCounting) {
      doHistoryDelayedHitCounting(nodeId,s);
    } else {
      cacheHitCounter++;
    }
   
    if (mustBeSorted) s->leafNode->checkSort();

    return s->leafNode;
  }

  
  BTreeNode<KEYTYPE,VALUETYPE>* n = 
                
  new BTreeNode<KEYTYPE,VALUETYPE>(false, nodeId, btree);

  n->Read(btree->GetBTreeFile(),nodeId);

  addLeafNodeToCache(n); 

  checkLimits();

  assert(!n->IsInternal());
  return n;
}

template <typename KEYTYPE,typename VALUETYPE> 
InternalNodeClass<KEYTYPE>* 
        BTree2Cache<KEYTYPE,VALUETYPE>::fetchInternalNode(const NodeId nodeId) {
  btree->IncNodesVisitedCounter();

  typename std::map<NodeId,cacheS*>::iterator it;

  it = nodesInMemory.find(nodeId);

  if (it != nodesInMemory.end()) {
    cacheS* s = it->second;
    s->garbageCount++;
    cacheMemoryUsage -= s->memoryUsage;
    s->memoryUsage = s->leafNode->GetMemoryUsage();
    cacheMemoryUsage += s->memoryUsage;
    s->historyListCount++;
    historyList.push_back(nodeId);

    // Complicated cache hit counting

    // Complicated cache hit counting
    if (operationRelatedHitCounting) {
      doOperationRelatedHitCounting(nodeId,s);
    } else if (historyDelayedHitCounting) {
      doHistoryDelayedHitCounting(nodeId,s);
    } else {
      cacheHitCounter++;
    }
   
    return s->internalNode;
  }

  // Not found: fetch from disc
  
  InternalNodeClass<KEYTYPE>* n = 
                  new InternalNodeClass<KEYTYPE>(true, nodeId, btree);

  n->Read(btree->GetBTreeFile(), nodeId);

  addInternalNodeToCache(n); 

  checkLimits();

  assert(n->IsInternal());

  return n;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Cache<KEYTYPE,VALUETYPE>::addPinnedNode(NodeId n, bool internal) {
  // Query required node, put it into cache
  BTree2::PinnedNodeInfo pni(n, 0);
  InternalNode* nodeI = 0;
  LeafNode* nodeL = 0;
  if (internal) {
    nodeI = fetchInternalNode(n);
    pni.memoryUsage = nodeI->GetMemoryUsage(); 
  } else {
    nodeL = fetchLeafNode(n);
    pni.memoryUsage = nodeL->GetMemoryUsage();
  }

  // Check, if that leads to an overflow
 
  if (cacheWithFixedElements) {
    if (cacheElements >= cacheElementLimit) {
      popCache();
      if (internal)
        dispose(nodeI);
      else
        dispose(nodeL);
      return false;
    }
  } else {
    if (cacheMemoryUsage >= cacheMemoryLimit) {
      popCache();
      if (internal)
        dispose(nodeI);
      else
        dispose(nodeL);
      return false;
    }
  }

  // Mark the cache entry as pinned  
  pinnedNodes.insert(pni);

  typename std::map<NodeId,cacheS*>::iterator it;

  for (it = nodesInMemory.begin(); it != nodesInMemory.end(); it++) {
    if ((*it).second->nodeId == n) {
      (*it).second->pinned = true;
    }
  }
  if (internal)
    dispose(nodeI);
  else
    dispose(nodeL);
  return true;
}

template <typename KEYTYPE,typename VALUETYPE> 
bool BTree2Cache<KEYTYPE,VALUETYPE>::removePinnedNode(NodeId n) {
  BTree2::PinnedNodeInfo pni(n, 0);
  typename std::set<BTree2::PinnedNodeInfo>::iterator in = 
                                            pinnedNodes.find(pni);
  if (in == pinnedNodes.end()) {

    return false;
  } else {
    pinnedNodes.erase(in);
    // Find cache element and unset pinned flag
    typename std::map<NodeId,cacheS*>::iterator it;

    for (it = nodesInMemory.begin(); it != nodesInMemory.end(); it++) {
      if ((*it).second->nodeId == n) {
        (*it).second->pinned = false;
      }
    }
  }

  return true;
} 

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Cache<KEYTYPE,VALUETYPE>::dispose(const NodeId nodeId) {
  // decrease garbageCount
  typename std::map<NodeId,cacheS*>::iterator it;

  it = nodesInMemory.find(nodeId);
  if (it != nodesInMemory.end()) {
    it->second->garbageCount--;
  }
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Cache<KEYTYPE,VALUETYPE>::dispose(InternalNode* n) {
  // decrease garbageCount
  typename std::map<NodeId,cacheS*>::iterator it;

  it = nodesInMemory.find(n->GetNodeId());
//  assert(it != nodesInMemory.end());
  it->second->garbageCount--;
}

template <typename KEYTYPE,typename VALUETYPE> 
void BTree2Cache<KEYTYPE,VALUETYPE>::dispose(LeafNode* n) {
  // decrease garbageCount
  typename std::map<NodeId,cacheS*>::iterator it;

  it = nodesInMemory.find(n->GetNodeId());
//  assert(it != nodesInMemory.end());
  it->second->garbageCount--;
}

template <typename KEYTYPE,typename VALUETYPE>
bool BTree2Cache<KEYTYPE,VALUETYPE>::hasUndisposedThings() {
  // Debug routine
  typename std::map<NodeId,cacheS*>::reverse_iterator it;
  int idx = cacheElements;
  bool x = false;

  for (it = nodesInMemory.rbegin(); it != nodesInMemory.rend(); ++it, --idx) {
    if ((*it).second->garbageCount != 0) {
      cerr << "Warning! Cache memory leakage. Maybe you forgot a dispose? [" 
           << (*it)->nodeId << "/" << (*it)->garbageCount << "]" << endl;
      x = true;
    }
  }
  return x;
}

template <typename KEYTYPE,typename VALUETYPE>
void BTree2Cache<KEYTYPE,VALUETYPE>::printCache() {
  // Debug routine

  if (btree->dbgPrintCache()) {
    typename std::map<NodeId,cacheS*>::iterator it;
    size_t totalMem = 0;
  
    cerr << "Cache has " << cacheElements << " entries = " 
         << nodesInMemory.size() << endl;
    for (it = nodesInMemory.begin(); it != nodesInMemory.end(); ++it) {
      cacheS* s = (*it).second;
      if (s->pinned) {
        cerr << s->nodeId << "/" << s->garbageCount << "P ";
      } else {
        cerr << s->nodeId << "/" << s->garbageCount << " ";
      }
      if (s->internal) {
        InternalNode* n = s->internalNode;
        totalMem += n->GetMemoryUsage();
      } else {
        LeafNode* n = s->leafNode;
        totalMem += n->GetMemoryUsage();
      }
    }
    cerr << endl;
    cerr << "Memory usage: " << totalMem << endl;
    cerr << "Approx: " << cacheMemoryUsage << endl;
    if (cacheWithFixedElements) {
      cerr << "Cache elements limit: " << cacheElementLimit << endl;
    } else {
      cerr << "Cache memory limit: " << cacheMemoryLimit << endl;
    }
  }
}

} // end namespace

#endif
