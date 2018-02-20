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

This class is a collection of two interfaces: first, the secondo-specific
functions (like Open,Save,Create,etc.) are declared here as static methods.
Second, this class serves as an interface for the BTree2Impl class,
allowing access to methods without the burden of having to instantiate
keytype and valuetype.

*/

#ifndef _BTREE2_CLASS_H_
#define _BTREE2_CLASS_H_

#include "BTree2Types.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

#include <string>

namespace BTree2Algebra {

class BTree2Iterator;

/*
2 The BTree2 class

*/
class BTree2 {
  public:
    enum multiplicityType { multiple, stable_multiple, uniqueKeyMultiData,
                             uniqueKey };

/*
2.1 Creation of BTree Objects

*/

    BTree2();
/*
Constructor: initializes some counters; however, the BTree can only be
used, if one initializes it with a call to ~CreateBTree~.

*/
    virtual ~BTree2() { };
/*
Destructor: Basically abstract method - implemented by BTree2Impl.

*/
    static BTree2* Factory(const std::string& keyTypeString,
                           const std::string& valueTypeString);
/*
Factory pattern: this method creates an uninitialized BTree2Impl Object with
given keytype and valuetype.

*/

    virtual bool CreateNewBTree(double fill, int nodesize,
                                 multiplicityType u = multiple,
                                 bool _temporary = false) = 0;
/*
Initialization: this method initializes an uninitialized btree.

*/

/*
2.2 The secondo specific part

*/

    static TypeConstructor typeConstructor;
/*
Secondo Type Information

*/

    static ListExpr Property();
/*
Secondo Property function: returns hints how to create this data structure.

*/

    static ListExpr Out(ListExpr typeInfo, Word value);
/*
Secondo Out function: this is not useful for BTrees.

*/

    static Word In(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct);
/*
Secondo In function: BTrees cannot be initialized directly.

*/

    static ListExpr SaveToList(ListExpr typeInfo, Word value);
/*
Secondo SaveToList function

*/
    static Word RestoreFromList( ListExpr typeInfo, ListExpr value,
                      int errorPos, ListExpr& errorInfo, bool& correct);
/*
Secondo RestoreToList function

*/

    static Word Instantiate(const ListExpr typeInfo);
/*
Secondo function: used to be called "Create", but it would be
ambigious as it does not create an ready-to-use object (one has
to call CreateBTree or Open for this).

*/

    static void Close(const ListExpr typeInfo, Word& w);
/*
Secondo Close function: called after BTree has been used.

*/

    static Word Clone(const ListExpr typeInfo, const Word& w);
/*
Secondo Clone function: duplicate this BTree.

*/

    static void Delete(const ListExpr typeInfo, Word& w);
/*
Secondo Delete function: deletes this BTree.

*/

    static bool KindCheck(ListExpr type, ListExpr& errorInfo);
/*
Secondo KindCheck function: checks if the type identifier is valid.

*/

    static void* Cast(void* addr);
/*
Secondo Cast function: casts a void ptr.

*/

    static int SizeOf();
/*
Secondo SizeOf function: gives the size of this object.

*/

    static bool Open( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word &value);
/*
Secondo Open function: restores an BTree from file.

*/
    static bool Save( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word &value);
/*
Secondo Save function: saves an BTree from file.

*/

/*
2.3 Methods for getting information on the BTree

*/
    const std::string& GetKeyType() { return keytype; }
/*
Returns the name of the keytype. Can only be set by CreateBTree.

*/

    const std::string& GetValueType() { return valuetype; }
/*
Returns the name of the valuetype. Can only be set by CreateBTree.

*/

    const ListExpr& GetKeyTypeListExpr() { return keyTypeListExpr; }
/*
Returns the ListExpr of the keytype.

*/

    const ListExpr& GetValueTypeListExpr() { return valueTypeListExpr; }
/*
Returns the ListExpr of the valuetype.

*/

    int GetRecordSize() { return recordSize; }
/*
Returns the record size of the tree. Can only be set by CreateBTree.

*/

    double GetFill() { return header.fill; }
/*
Returns the fill factor. Currently, it can only be set by CreateBTree
although there would be no principle problem for changing this
value.

*/

    multiplicityType GetMultiplicity() { return header.multiplicity; }
/*
Returns the multiplicity type. Can only be set by CreateBTree.
Caveat! Must coincide with the type information after construction.

*/

    int GetMaxKeysize() { return header.maxKeysize; }
/*
Returns the maximum keysize (larger keys will be written into
extended keys file)

*/

    int GetMaxValuesize() { return header.maxValuesize; }
/*
Returns the maximum valuesize (larger values will be written into
extended values file)

*/

    int GetTreeHeight() { return header.treeHeight; }
/*
Returns the maximum valuesize (larger values will be written into
extended values file)

*/

    int GetNodeCount() {return header.internalNodeCount +
                               header.leafNodeCount;}
/*
Returns the total number of nodes in the tree.

*/

    int GetInternalNodeCount() {return header.internalNodeCount; }
/*
Returns the number of internal nodes in the tree.

*/

    int GetLeafNodeCount() {return header.leafNodeCount;}
/*
Returns the number of leaf nodes in the tree.

*/

    int GetEntryCount() {return header.leafEntryCount +
                                header.internalEntryCount;}
/*
Returns the total number of entries.

*/

    int GetLeafEntryCount() {return header.leafEntryCount; }
/*
Returns the number of leaf entries.

*/

    int GetInternalEntryCount(){return header.internalEntryCount;}
/*
Returns the number of internal entries.

*/

    virtual int GetNodeEntryCount(NodeId nodeId) = 0;
/*
Returns the number of entries of a specific node.

*/

    NodeId GetRootNode() { return header.rootNodeId; }
/*
Returns the root node id.

*/

    virtual int GetMinNodeSize() = 0;
/*
Returns the smallest possible record size.

*/

    int GetNodesVisitedCounter() {
       return header.nodesVisitedCounter;
    }
/*
Returns the number of visited nodes.

*/

    void IncNodesVisitedCounter() {
      header.nodesVisitedCounter++;
    }
/*
Increases the counter of visited nodes by 1.

*/

    virtual int GetMinEntries(bool internal) = 0;
/*
Returns the minimum number of entries for internal or leaf nodes.

*/

    virtual int GetMaxEntries(bool internal) = 0;
/*
Returns the maximum number of entries for internal or leaf nodes.

*/

    virtual void GetStatistics(NodeId nid, StatisticStruct& result) = 0;
/*
Scans the whole tree and collects various information.

*/

    bool hasValidBTreeFile() { return file != 0; }
/*
Returns true, if BTree has a valid persistent storage container.

*/

    enum FileEnum { FILE_CORE = 0, FILE_EXTENDED_KEYS, FILE_EXTENDED_VALUES,
                   FILE_CACHE };
    bool GetFileStats( FileEnum ef, SmiStatResultType &result );
/*
Returns information on persistent storage container.

*/

    static const std::string BasicType() { return "btree2"; }

    static const bool checkType(const ListExpr list){
       return listutils::isBTree2Description(list);
    }

/*
2.4 Content related queries

*/
    bool ProbeIsInternal(NodeId nid);
/*
Returns true, if node ~nid~ is an internal node. This method opens the
node on disc but only reads the first byte. This method should be
avoided if performance matters.

*/

    virtual BTree2Iterator begin() = 0;
/*
Returns an iterator to the first/leftmost entry of the whole tree.

*/

    BTree2Iterator end(); // { return BTree2Iterator(this,0,-1); }
/*
Returns an end iterator.

*/

    virtual BTree2Iterator find(Attribute* key) = 0;
/*
Returns an iterator to the first (leftmost) occurence of ~key~ or end, if
not found.

*/

    virtual Attribute* GetEntryKeyInternal(NodeId nodeId, int entryNumber) = 0;
/*
Returns an Attribute representation of the key of node ~nodeId~ at index
~entryNumber~. If keytype is int,bool,string or double, a new Attribute object
is created otherwise a copy is returned. In both cases, the caller have to
call DeleteIfAllowed().

*/

    virtual NodeId GetEntryValueInternal(NodeId nodeId, int entryNumber) = 0;
/*
Returns the NodeId of the child of node node ~nodeId~ at index
~entryNumber~.

*/

    virtual Attribute* GetEntryKey(NodeId nodeId, int entryNumber) = 0;
/*
Returns an Attribute representation of the value of node ~nodeId~ at index
~entryNumber~. If keytype is int,bool,string or double, a new Attribute object
is created otherwise a copy is returned. In both cases, the caller have to
call DeleteIfAllowed().

*/

    virtual Attribute* GetEntryValue(NodeId nodeId, int entryNumber) = 0;
/*
Returns an Attribute representation of the value of node ~nodeId~ at index
~entryNumber~. If valuetype is int,bool,string or double, a new Attribute object
is created otherwise a copy is returned. In both cases, the caller have to
call DeleteIfAllowed().

*/

    virtual bool GetNext(NodeId& nodeId, int& entry) = 0;
/*
Move to the next entry. If there is no, the method
returns false and the nodeId/entry is set to 0/-1.

*/

    virtual bool GetNext(NodeId& nodeId,int& entry,Attribute*&,Attribute*&)=0;
/*
Move to the next entry. If there is no, the method
returns false and the nodeId/entry is set to 0/-1.
Key and value is instantiated as Attribute objects.

*/

    virtual BTree2Iterator findLeftBoundary(Attribute* key) = 0;
/*
Returns begin() but stores a marker, where iteration is stopped.

*/

    virtual BTree2Iterator findRightBoundary(Attribute* key) = 0;
/*
Returns the first entry to the right which is not key.

*/

    virtual void findExactmatchBoundary(Attribute* key, BTree2Iterator&,
                                                        BTree2Iterator&) = 0;
/*
The exactmatch will be different for uniqueKey, uniqueKeyMultiData and
multiple BTree's. To perform this method, we use a local search in this
method, if necessary. The method delivers an iterator as starting point
indicating the first occurence of key and a second iterator to the first
entry to the right, which is not key.
Therefore, this method should be better then:
startIteratorMarker = findLeftBoundary(key);
finalIteratorMarker = findRightBoundary(key);

*/

    virtual void findRangeBoundary(Attribute* key, Attribute* secondKey,
                   BTree2Iterator& startIter, BTree2Iterator& finalIter) = 0;
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
    virtual bool AppendGeneric(Attribute* key, Attribute* value) = 0;
/*
Append a key/value pair to the tree. Returns true, if inserted sucessfully.

*/

    virtual bool DeleteGeneric(Attribute* key) = 0;
/*
Delete the first occurence of ~key~. Returns false, if ~key~ is not found.

*/

    virtual bool DeleteGeneric(Attribute*, Attribute*) = 0;
/*
Delete the first occurence of given key/value combination.
Returns false, if this combination is not found.

*/

    virtual bool UpdateGeneric(Attribute*, Attribute*) = 0;


/*
2.6 Cache delegate methods

All methods are simple delegates to the methods of the BTree2Cache class.
Documentation is available there.

*/

    virtual void SetCacheSize(size_t mem) = 0;
    virtual size_t GetCacheSize() = 0;
    virtual int GetCacheHitCounter() = 0;
    virtual void resetCacheHitCounter() = 0;
    virtual bool addCachePinnedNode(NodeId n) = 0;
    virtual bool removeCachePinnedNode(NodeId n) = 0;
    virtual bool IsFixedElementsLimitType() = 0;
    virtual void SetFixedElementsLimitType() = 0;
    virtual void SetMemoryLimitType() = 0;
    virtual void DropCacheFile() = 0;
    virtual size_t GetPeakCacheMemoryUsage() = 0;
    virtual int GetPeakCacheElements() = 0;
    struct PinnedNodeInfo{
      NodeId id;
      size_t memoryUsage;
      PinnedNodeInfo(){}
      PinnedNodeInfo(NodeId i, size_t m) : id(i), memoryUsage(m) {}
      bool operator < (const PinnedNodeInfo& p) const
      {
        return id < p.id;
      }
    };
/*
Stores information about pinned nodes.

*/
    virtual std::set<PinnedNodeInfo> const* GetPinnedNodes() = 0;

/*
2.7 Global parameter settings

Here, methods are provided for getting and setting default parameters
for all BTrees.

*/

    static bool dbgPrintTree() { return debugPrintTree; }
/*
True, if printTree debug output is active. (Output of the
complete btree)

*/

    static bool dbgPrintCache() { return debugPrintCache; }
/*
True, if printCache debug output is active. (output of the
complete cache)

*/

    static bool dbgPrintNodeLoading() { return debugPrintNodeLoading; }
/*
True, if printNodeLoading debug output is active. (output of
small messages on loading and unloading of nodes)

*/

    static void SetDbgPrintTree(bool x) { debugPrintTree = x; }
/*
Set or unset the debug printTree output.

*/

    static void SetDbgPrintCache(bool x) { debugPrintCache = x; }
/*
Set or unset the debug printCache output.

*/

    static void SetDbgPrintNodeLoading(bool x) { debugPrintNodeLoading = x; }
/*
Set or unset the debug printNodeLoading output.

*/

    static int GetDefaultMaxKeysize() { return defaultMaxKeysize; }
/*
Returns the current default maxKeysize. (New trees will be initialized
with this value).

*/

    static int GetDefaultMaxValuesize() { return defaultMaxValuesize; }
/*
Returns the current default maxValuesize.
(New trees will be initialized with this value).

*/

    static bool SetDefaultMaxKeysize(int i);
/*
Sets the default maxKeysize

*/

    static bool SetDefaultMaxValuesize(int i);
/*
Sets the default maxValuesize

*/

/*
2.8 Miscellaneous

*/

    virtual bool Open(SmiFileId& sf, SmiRecordId _headerId, int recordSize) = 0;
/*
Reads btree from disc.

*/

    SmiRecordFile* GetExtendedKeysFile() { return extendedKeysFile; }
/*
Returns handle to extendedKeysFile

*/

    SmiRecordFile* GetExtendedValuesFile() { return extendedValuesFile; }
/*
Returns handle to extendedValuesFile

*/

    SmiRecordFile* GetBTreeFile() { return file; }
/*
Returns handle to btree's persistent storage container.

*/

    SmiRecordId GetHeaderId() { return headerId; }
/*
Returns the record id where btree parameter are stored.

*/

    SmiFileId GetBTreeFileId() { return file->GetFileId(); }
/*
Returns the file id of btree's persistent storage container.

*/

    virtual void printNodeInfo(NodeId id,int height) = 0;
/*
For debugging: print btree, starting at node ~id~, to stdout.

*/

    virtual void printNodeInfos() = 0;
/*
For debugging: print whole btree to stdout.

*/

    void ReadHeader();
/*
Read btree's parameter.

*/

    void WriteHeader();
/*
Write btree's parameter.

*/

    void resetNodesVisitedCounter() {
       header.nodesVisitedCounter = 0;
    }

/*
Reset the nodes visited counter.

*/



 protected:
    std::string keytype;
    std::string valuetype;
    int recordSize;
    SmiRecordFile* file;
    SmiRecordId headerId;
    unsigned int maxNodesInMemory;
    SmiRecordFile* extendedKeysFile;
    SmiRecordFile* extendedValuesFile;
    ListExpr keyTypeListExpr;
    ListExpr valueTypeListExpr;

    struct headerS {            // these values will be stored persistently
      NodeId rootNodeId;
      int treeHeight;
      int internalNodeCount;
      int leafNodeCount;
      int leafEntryCount;
      int internalEntryCount;
      double fill;
      SmiFileId extendedKeysFileId;
      SmiFileId extendedValuesFileId;
      SmiFileId cacheFileId;
      int nodesVisitedCounter;
      int maxKeysize;
      int maxValuesize;
      multiplicityType multiplicity;
    } header;

//    BTree2Iterator startIterate;
//    BTree2Iterator finalIterate;

    static bool debugPrintTree;
    static bool debugPrintCache;
    static bool debugPrintNodeLoading;

    static int defaultMaxKeysize;
    static int defaultMaxValuesize;
};

} // end namespace btree2algebra

#endif
