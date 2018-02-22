/*
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[->] [$\rightarrow$]
//[TOC] [\tableofcontents]
//[_] [\_]

RTree-Class

*/

#ifndef RTREE_H
#define RTREE_H


#include <iostream>
#include <string>
#include <vector>
#include "Cache/LruCache.h"
#include "Cache/CacheBase.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "RTreeHeader.h"
#include "RTreeNode.h"
#include <stack>                    // stack


//Implementation of R-Tree for portable Index

namespace fialgebra{
//File and cache based implementation of a RTree-Index
template <int dim>
class RTree{
public:
  //Tries to create a file based RTree with the passed parameters.
  //Fails if a file with the given path existed in the first place
  //or the system's page-size can't take
  //at least a single entry(rectangle and id).
  //If minEntries is zero or too high its audjusted to the highest valid value.
  static RTree<dim>* Create(const char* fileName, size_t cacheSize,
                            size_t minEntries);
  //Tries to read a file based RTree from its file.
  //Fails if the file doesn't exist or is invalid.
  static RTree<dim>* Open(const char* fileName, size_t cacheSize);
  
  static RTree<dim>* OpenRebuild(const char* fileName, size_t cacheSize);

  //Saves the header and flushes the cache.
  ~RTree();

  int GetTreeHeight();

  //Inserts the passed rectangle and id into the tree.
  void Insert(const Rectangle<dim>& box, size_t id);
  //Deletes one occurence of th specified rectangle and id from the tree.
  //Returns false if no match was found, true otherwise.
  bool Delete(const Rectangle<dim>& box, size_t id);

  unsigned long Search(Rectangle<dim>*);

  void Rebuild(){};
  
  void RebuildR(const char* filename);

  RTreeHeader* GetHeader();

  //Write's the node into the cache
  void WriteNode(RTreeNode<dim>& node);
  //Read's the node with the passed id from the cache
  RTreeNode<dim>* ReadNode( size_t id );

  void WriteNode(RTreeNode<dim>& mynode, size_t page);

  //Returns a simple string representation of this tree.
  //Prints all nodes, values and ids!
  std::string ToString();
  void ToString(RTreeNode<dim>* node);

  // 
  // Bulkload
  // 
  // Start bulkload-mode
  void BeginBulkload( const double maxDist );
  // Insert value in bulkload-mode
  void Bulkload( const Rectangle<dim>& box, const size_t id );
  // End bulkload-mode
  void EndBulkload();
  
  
private:
  //Creates a file based RTree with the given parameters
  RTree(RTreeHeader* header, cache::CacheBase* cache);

  void InsertNode(const RTreeNode<dim>* actualNode);

  //Splits the given node and inserts the additional one into the parent.
  //If the node is the root node the parent should be NULL.
  //To take effect call 'WriteNode' on both, the node and the parent
  //after calling this function!
  void SplitNode( RTreeNode<dim>*& node, RTreeNode<dim>* parent );
  //Puts the node into the 'empty-page-list'
  //To take effect call 'WriteNode' after calling this function!
  void RecycleNode(RTreeNode<dim>& node);
  //Determines the son of the given node best suitable for insertion of the
  //passed rectangle.
  //The node mustn't be a leaf!
  RTreeNode<dim>* BestSonSearch(const RTreeNode<dim>& actualNode,
                                const Rectangle<dim>& rect);

  size_t BestSonIDSearch(const RTreeNode<dim>& actualNode,
                                const Rectangle<dim>& rect);

  void InsertNode(RTreeNode<dim>* parent, RTreeNode<dim>* child);// ERLEDIGT

  void RemoveNodeFromRAM (RTreeNode<dim>* node);

  RTreeNode<dim>* GetParentNode(RTreeNode<dim>* node); //ERLEDIGT

  RTreeNode<dim>*
  GetNodeByID(const Rectangle<dim>& box, const unsigned long id); //ERLEDIGT

  size_t GetChildNodeID(RTreeNode<dim> *parent, RTreeNode<dim> *child);

  RTreeNode<dim>* GetChildNodeByIndex(RTreeNode<dim>* node, size_t index);

  void RemoveNode(RTreeNode<dim> *toDelete);

  void CondenseTree(RTreeNode<dim> *parent);

  void ReinsertNodes(RTreeNode<dim> *node);
  
  RTreeNode<dim>* CreateRTReeNode(bool isLeaf);
  
  

  //This function prints the tree's content recursively(!) and is used by the
  //ToString method.
  void PrintAsTree(std::ostream& o, RTreeNode<dim>& node, size_t depth);

  cache::CacheBase* m_treeCache;
  int rTreeHeight;
  int noNodes;
  std::vector <RTreeNode<dim>* > path;
  RTreeNode<dim>* root;
  std::vector<std::pair<RTreeNode<dim>*, unsigned int> > pathStack;
  
  std::vector<RTreeNode<dim> > AllNodesWithinRTree;

  int MinEntries;

  int numberOfTreeNodes;

  RTreeHeader* m_treeHeader;
  unsigned long m_pageSize;
  
   // gets the use cache
  cache::CacheBase* GetCache();
  
  // 
  // Bulkload
  // 
  // Current bulkload node
  RTreeNode<dim>* _curBulkNode;
  // Path to current bulkload node
  std::stack<RTreeNode<dim>*> _bulkStack;
  // Max. height of tree
  size_t _bulkMaxHeight = 0;
  // Max. distance for bulk-insert
  double _bulkMaxDist = 0;
  
  
};// end of class RTree

}// end of namespace fialgebra


#endif // RTREE_H









