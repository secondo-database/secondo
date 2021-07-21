/*
----
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
----

*/
 
#ifndef NTREE_H
#define NTREE_H

/*
Implementation of the N-tree

It is as powerful as the M-tree but supports more efficient range and nearest
neighbor searches.

*/

/*
1 class NTreeNode

*/
template<class T, class DistComp>
class NTreeNode {
 public:
  typedef MTreeNode<T,DistComp> node_t;
  
  virtual ~NTreeNode() {}
  
  virtual bool isLeaf() const = 0;
  
  virtual size_t subTreeMemSize() const = 0;
  
  const T* getClusterCenter() const {
    return &clusterCenter;
  }
  
  int getMaxNoChildren() const {
    return maxNoChildren;
  }
  
  virtual int getNoEntries() const = 0;
  
  virtual int getNoLeafs() const = 0;
      
  virtual int getNoNodes() const = 0;

  virtual node_t* clone() = 0;

  void setParent(node_t* p) {
    parent = p;
  }
  
  
 protected:
  NTreeNode(const T& cc, const int mnc, const int mne) :
      clusterCenter(cc), maxNoChildren(mnc), maxNoEntries(mne), parent(0) {}
   
 private:
  T clusterCenter;
  int maxNoChildren;
  int maxNoEntries;
  node_t* parent;
};

/*
2 class NTreeInnerNode

*/
template<class T, class DistComp>
class NTreeInnerNode : public NTreeNode<T, DistComp> {
 public:
  
   
   
 private:
  NTreeNode<T, DistComp> children[]; 
};

/*
3 class NTreeLeafNode

*/
template<class T, class DistComp>
class NTreeLeafNode : public NTreeNode<T, DistComp> {
 public:
  
   
   
 private:
  T entries[];  
};

/*
4 class NTree

This is the main class of this file. It implements a main memory-based N-tree.

*/
template<class T, class DistComp>
class NTree {
 public:
  typedef NTreeLeafNode<T, DistComp> leafnode_t;
//   typedef RangeIterator<T, DistComp>  rangeiterator_t;
//   typedef NNIterator<T, DistComp> nniterator_t;
  typedef NTreeNode<T, DistComp> node_t;
  typedef NTree<T, DistComp> ntree_t;
  typedef NTreeInnerNode<T, DistComp> innernode_t;
  
  NTree(const int mnc, const int mne, DistComp& d) :
      maxNoChildren(mnc), maxNoEntries(mne), root(0), dc(d) {}
  
  ~NTree() {
    if (root) {
      delete root;
    }
  }
  
  int getNoEntries() const {
    if (!root) {
      return 0;
    }
    return root->getNoEntries();
  }
  

 private:
  int maxNoChildren;
  int maxNoEntries;
  node_t* root;
  DistComp dc;
};

#endif
