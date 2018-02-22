/*
 BPTree-Class

*/

#ifndef BPTREE_H
#define BPTREE_H

#include <chrono>
#include <string.h>
#include <stack>

#include "Attribute.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"

#include "Cache/CacheBase.h"
#include "BPTreeNode.h"
#include "BPTreeHeader.h"
#include "BPTreeSearchEnumerator.h" // Suche


//Implementation of B+ Tree for portable Index

namespace fialgebra {
//class definition for BPTree

// circle dependency of BPTree.h and BPTreeSearchEnumerator.h
class BPTreeSearchEnumerator;

class BPTree {
public:
  static BPTree* Create(const char* fileName, unsigned int algebraId,
                        unsigned int typeId, size_t cacheSize);

  static BPTree* Open(const char * fileName, size_t cacheSize);
  ~BPTree();
//Bulkload for a stream of sorted values
  void StartBulkload();
  void InsertBulkload(Attribute& value, TupleId tupleId);
  void EndBulkload();
//insert and delete for unsorted values
  void InsertValue(Attribute& value, TupleId tupleId);
  bool DeleteValue(Attribute& value, TupleId tupleId);
//rebuild of the tree for better performance after many operations
  void Rebuild(const char* filename);
//textual representation of the tree
  std::string ToString();

  int GetAlgebraId();
  int GetTypeId();

  BPTreeHeader& GetHeader();
//gets a node object
  BPTreeNode* GetNodefromPageNumber(size_t page);

  // searches all elements with the given key
  BPTreeSearchEnumerator* SearchKeys( const Attribute* key );
  // searches for all elements within a range, including min & max
  BPTreeSearchEnumerator* SearchKeys(
    const Attribute* minKey,
    const Attribute* maxKey );

  // gets the height of the tree
  size_t GetHeight();

private:
  struct PathEntry{
    BPTreeNode* node;

    size_t index;

    PathEntry(BPTreeNode* node, size_t index);
  };

  cache::CacheBase*    m_treeCache;
  BPTreeHeader* m_treeHeader;
  unsigned long m_pageSize;
  ObjectCast    m_valueCast;

  std::stack<BPTreeNode*>* m_bulkLoadPath = NULL;
  BPTreeNode*   m_bulkLoadLeaf = NULL;

  // ctor
  BPTree( BPTreeHeader* header, cache::CacheBase* cache );
  //gives the node to the cache
  void WriteNode(BPTreeNode& node);
  void WriteNode(BPTreeNode* mynode, size_t page);
  //
  // internal searches
  //
  // returns the position (index) at which the search-value
  // should be placed.
  long LookupSearchIndex( BPTreeNode* node, const Attribute& searchValue );
  //
  // returns the position (index) at which the searched
  // value has to be inserted
  size_t LookupInsertIndex(BPTreeNode& node, const Attribute& insertValue);
  //
  // returns the node which should contain the searched key
  BPTreeNode* SearchNode( const Attribute& key );

  // gets the leftmost leaf of the tree
  BPTreeNode* GetLeftMostLeaf();
  // gets the root or 0, if no root node exists
  BPTreeNode* GetRootNode();

  //creates a new node /(true for leaf, false for inner node or root)
  BPTreeNode* CreateNode(bool isLeaf);
  void DeleteNode(BPTreeNode& node);

  // used by the ToString method
  void PrintAsTree(std::ostream& o, BPTreeNode& node, size_t depth);

  std::stack<PathEntry>* GetInsertPath(const Attribute& attribute);
  std::stack<PathEntry>* GetDeletePath(const Attribute& attribute, 
                                       TupleId tupleId);
   //Splits a node in to almost equal filled nodes
  void SplitNode(BPTreeNode& node, BPTreeNode& parentNode, size_t nodeIndex);
  //merges to nodes, if one is underflowed
  bool MergeNode(BPTreeNode& node,
                 std::pair<BPTreeNode*, BPTreeNode*>& siblings,
                 BPTreeNode& parentNode, size_t nodeIndex);
  //merges with the left brother
  bool MergeWithLeft(BPTreeNode& node, BPTreeNode& leftNode,
                     BPTreeNode& parentNode, size_t nodeIndex);
 //merges with the right brother
  bool MergeWithRight(BPTreeNode& node, BPTreeNode& rightNode,
                      BPTreeNode& parentNode, size_t nodeIndex);
  //balance for under- or overflow
  bool BalanceNode(BPTreeNode* targetNode,
                   std::pair<BPTreeNode*, BPTreeNode*>& siblings,
                   BPTreeNode* parentNode, size_t nodeIndex);

  bool BalanceWithLeft(BPTreeNode& node, BPTreeNode& leftNode,
                       BPTreeNode& parentNode, size_t nodeIndex);

  bool BalanceWithRight(BPTreeNode& node, BPTreeNode& rightNode,
                        BPTreeNode& parentNode, size_t nodeIndex);
//used by balance methods
  void MoveEntriesToLeft(BPTreeNode& node, BPTreeNode& leftNode,
                         BPTreeNode& parentNode, size_t nodeIndex,
                         size_t count);

  void MoveEntriesToRight(BPTreeNode& node, BPTreeNode& rightNode,
                          BPTreeNode& parentNode, size_t nodeIndex,
                          size_t count);
//gets the brothers of a node
  std::pair<BPTreeNode*, BPTreeNode*> GetBrothers(BPTreeNode& node,
                                             BPTreeNode& parent,
                                             size_t nodeIndex);
}; // end of class BPTree
} // end of namespace fialgebra

#endif // BPTREE_H


















