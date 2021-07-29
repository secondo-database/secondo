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

#include <limits>

/*
Implementation of the N-tree

It is as powerful as the M-tree but supports more efficient range and nearest
neighbor searches.

*/
template<class T, class DistComp>
class NTreeAux;

template<class T, class DistComp>
class NTreeInnerNode;

/*
1 class NTreeNode

*/
template<class T, class DistComp>
class NTreeNode {
 public:
  typedef NTreeNode<T, DistComp> node_t;
  typedef NTreeInnerNode<T, DistComp> innernode_t;
  
  virtual ~NTreeNode() {}
  
  virtual bool isLeaf() const = 0;
  
  virtual size_t memSize() const = 0;
  
  const T* getCenter() const {
    return &center;
  }
  
  int getDegree() const {
    return degree;
  }
  
  int getMaxLeafSize() const {
    return maxLeafSize;
  }
  
  virtual int getNoEntries() const = 0;
  
  virtual int getNoLeaves() const = 0;
      
  virtual int getNoNodes() const = 0;
  
  virtual bool isOverflow() const = 0;
  
  virtual void insert(const T& o, DistComp& dc, const int partitionStrategy = 0)
    = 0;
  
  int getCount() const {
    return count;
  }

  virtual node_t* clone() = 0;

  innernode_t* getParent() {
    return parent;
  }
  
  int getPosInParent() {
    return posInParent;
  }
  
  void setParent(innernode_t* p, const int pos) {
    parent = p;
    posInParent = pos;
  }
  
  double centerDist(const T& o, DistComp& dc) const {
    return dc(center, o);
  }
  
  double distance(node_t* node, DistComp& dc) const {
    return dc(center, node->center);
  }
    
  virtual void split(DistComp& dc, const int partitionStrategy = 0) = 0;
  
  virtual void clear(const bool deleteContent) = 0;
  
  virtual std::ostream& print(std::ostream& out, DistComp& dc) const = 0;
  
  virtual std::ostream& print(std::ostream& out, const bool printSubtrees,
                              DistComp& di) const = 0;
  
  
 protected:
  NTreeNode(const T& c, const int d, const int mls) :
    center(c), degree(d), maxLeafSize(mls), parent(0), posInParent(-1), count(0)
    {}
   
  T center;
  int degree;
  int maxLeafSize;
  innernode_t* parent;
  int posInParent;
  int count;
};

template<class T, class DistComp>
class NTreeLeafNode;

/*
2 class NTreeInnerNode

*/
template<class T, class DistComp>
class NTreeInnerNode : public NTreeNode<T, DistComp> {
 public:
  typedef NTreeInnerNode<T, DistComp> innernode_t;
  typedef NTreeNode<T, DistComp> node_t;
  typedef NTreeLeafNode<T, DistComp> leafnode_t;
  
  NTreeInnerNode(const T& o, const int d, const int mls) : node_t(o, d, mls) {
    children = new node_t*[node_t::degree];
    for (int i = 0; i < node_t::degree; i++) {
      children[i] = 0;
    }
  }
  
  virtual ~NTreeInnerNode() {
    clear(true);
  }
  
  virtual void clear(const bool deleteContent) {
    for (int i = 0; i < node_t::degree; i++) {
      if (children[i] && deleteContent) {
        delete children[i];
      }
      children[i] = 0;
    }
    node_t::count = 0;
    if (deleteContent) {
      delete[] children;
    }
  }
  
  virtual bool isLeaf() const {
    return false;
  }
  
  size_t memSize() const {
    size_t res = sizeof(*this) + sizeof(void*) * node_t::degree + 1;
    for (int i = 0; i < node_t::count; i++) {
      res += children[i]->memSize();
    }
    return res;
  }
  
  node_t* getChild(const int i) {
    assert(i >= 0);
    assert(i < node_t::degree);
    return children[i];
  }
  
  int getNearestChildPos(const T& o, DistComp& dc) const {
    double currentDist = std::numeric_limits<double>::max();
    int result = -1;
    for (int i = 0; i < node_t::count; i++) {
      const T* c = (children[i])->getCenter();
      double dist = dc(*c, o);
      if (dist < currentDist) {
        result = i;
        currentDist = dist;
      }
    }
    return result;
  }
  
  void setChild(const int pos, node_t* child, const bool deleteCurrent) {
    assert(pos >= 0);
    assert(pos < node_t::degree);
    if (children[pos] && deleteCurrent) {
      delete children[pos];
    }
    children[pos] = child;
  }
  
  node_t* getNearestChild(const T& o, DistComp& dc) const {
    int pos = getNearestChildPos(o, dc);
    return children[pos];
  }
  
  void insert(const T& o, DistComp& dc, const int partitionStrategy = 0) {
    if (node_t::count == 0) { // add new child node
      children[node_t::count] = new leafnode_t(o, node_t::degree, 
                                               node_t::maxLeafSize);
      (children[node_t::count])->insert(o, dc, partitionStrategy);
      (children[node_t::count])->setParent(this, node_t::count);
      node_t::count++;
    }
    else { // insert into child with nearest center
      int pos = getNearestChildPos(o, dc);
      children[pos]->insert(o, dc, partitionStrategy);
    }
  }
  
  int getChildPos(const node_t* child) const {
    for (int i = 0; i < node_t::count; i++) {
      if (children[i] == child) {
        return i;
      }
    }
    return -1;
  }
  
  innernode_t* clone() {
    innernode_t* res;
    res = new innernode_t(node_t::center, node_t::degree, node_t::maxLeafSize);
    res->count = node_t::count;
    for (int i = 0; i < node_t::count; i++) {
      res->children[i] = children[i]->clone();
      res->children[i]->setParent(res, i);
    }  
    return res;
  }
  
  std::ostream& print(std::ostream& out, DistComp& dc) const {
    return print(out, true, dc);
  }
   
  std::ostream& print(std::ostream& out, const bool printSubtrees,
                      DistComp& dc) const {
    out << "( \"inner node: center = " ;
    dc.print(node_t::center, out) << "\"";
    if (printSubtrees) {
      out << " (";
      for (int i = 0; i < node_t::count; i++) {
        children[i]->print(out, dc);
      }
      out << " )";
    }
    out << ")";
    return out;
  }
  
  void split(DistComp& dc, const int partitionStrategy = 0) {} //only for leaves

  int getNoLeaves() const {
    int sum = 0;
    for (int i = 0; i < node_t::count; i++) {
      sum += children[i]->getNoLeaves();
    }
    return sum;
  }
  
  int getNoEntries() const {
    int sum = 0;
    for (int i = 0; i < node_t::count; i++) {
      sum += children[i]->getNoEntries();
    }
    return sum;
  }

  int getNoNodes() const {
    int sum = 0;
    for (int i = 0; i < node_t::count; i++) {
      sum += children[i]->getNoNodes();
    }
    return sum + 1;
  }
  
  bool isOverflow() const {
    return node_t::count > node_t::degree;
  }
   
 private:
  node_t** children;
};

/*
3 class NTreeLeafNode

*/
template<class T, class DistComp>
class NTreeLeafNode : public NTreeNode<T, DistComp> {
 public:
  typedef NTreeLeafNode<T, DistComp> leafnode_t;
  typedef NTreeNode<T, DistComp> node_t;
  typedef NTreeInnerNode<T, DistComp> innernode_t;
  
  NTreeLeafNode(const T& c, const int d, const int mls) : node_t(c, d, mls) {
    entries = new T*[node_t::maxLeafSize + 1];
    for (int i = 0; i < node_t::maxLeafSize + 1; i++) {
      entries[i] = 0;
    }
  }
  
  ~NTreeLeafNode() {
    for (int i = 0; i < node_t::maxLeafSize + 1; i++) {
      if (entries[i]) {
        delete entries[i];
      }
    }
    delete[] entries;
  }
  
  void clear(const bool deleteContent) {
    for (int i = 0; i < node_t::count; i++) {
      if (deleteContent) {
        delete entries[i];
      }
      entries[i] = 0;
    }
    node_t::count = 0;
  }
  
  virtual bool isLeaf() const {
    return true;
  }
  
  int getNoLeaves() const {
    return 1;
  }
  
  int getNoEntries() const {
    return node_t::count;
  }
  
  int getNoNodes() const {
    return 1;
  }
  
  bool isOverflow() const {
    return node_t::count > node_t::maxLeafSize;
  }
  
  void insert(const T& o, DistComp& dc, const int partitionStrategy = 0) {
    entries[node_t::count] = new T(o);
    node_t::count++;
    if (node_t::count > node_t::maxLeafSize) {
      split(dc, partitionStrategy);
    }
  }
  
  leafnode_t* clone() {
    leafnode_t* res;
    res = new leafnode_t(node_t::center, node_t::degree, node_t::maxLeafSize);
    res->count = node_t::count;
    for (int i = 0; i < node_t::count; i++) {
      res->entries[i] = new T(*entries[i]);
    }  
    return res;
  }
  
  std::ostream& print(std::ostream& out, DistComp& dc) const {
    out << "[ leaf node: center = ";
    dc.print(node_t::center, out) << ", content = \"";
    for (int i = 0; i < node_t::count; i++) {
      if (i > 0) {
        out << ", ";
      }
      dc.print(*entries[i], out);
    }
    out << "\"]";
    return out;
  }
  
  std::ostream& print(std::ostream& out, const bool printSubtrees,
                      DistComp &dc) const {
    return print(out, dc);
  }
  
  size_t memSize() const {
    size_t res = sizeof(*this) + node_t::maxLeafSize * sizeof(void*);
    for (int i = 0; i < node_t::count; i++) {
      res += sizeof(*entries[i]);
    }
    return res;
  }
  
  void split(DistComp& dc, const int partitionStrategy = 0) {
//     leafnode_t* thisLeafToDelete = this;
//     std::pair<T*, T*> newCenters = 
//        NTreeAux<T,DistComp>::computeCenters(entries, partitionStrategy);
//     leafnode_t* newLeaf1 = new leafnode_t(*(newCenters.first), 
//         node_t::degree, node_t::maxLeafSize);
//     leafnode_t* newLeaf2 = new leafnode_t(*(newCenters.second), 
//         node_t::degree, node_t::maxLeafSize);
//     for (int i = 0; i < node_t::count; i++) {
//       double dist1 = dc(*entries[i], *(newCenters.first));
//       double dist2 = dc(*entries[i], *(newCenters.second));
//       if (dist1 <= dist2) {
//         newLeaf1->insert(entries[i]->clone());
//       }
//       else {
//         newLeaf2->insert(entries[i]->clone());
//       }
//       this->getParent()->deleteChild(this->getPosInParent());
//       this->getParent()->addLeaf(newLeaf1);
//       this->getParent()->addLeaf(newLeaf2);
//     }
  }
  
 private:
  T** entries;
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
  
  NTree(const int d, const int mls, DistComp& di) :
      degree(d), maxLeafSize(mls), root(0), dc(di), partitionStrategy(0) {}
  
  ~NTree() {
    if (root) {
      delete root;
    }
  }
  
  int getNoLeaves() const {
    if (!root) {
      return 0;
    }
    return root->getNoLeaves();
  }
  
  int getNoEntries() const {
    if (!root) {
      return 0;
    }
    return root->getNoEntries();
  }
  
  int getNoNodes() const {
    if (!root) {
      return 0;
    }
    return root->getNoNodes();
  }
  
  size_t noComparisons() const {
    return dc.getCount();
  }
  
  size_t memSize() const {
    size_t res = sizeof(*this);
    if (root) {
      res += root->memSize();
    }
    return res;
  }
  
  void insert(T& o, const int partitionStrategy = 0) {
    if (!root) {
      root = new innernode_t(o, degree, maxLeafSize);
    }
    root = insert(root, o, dc, partitionStrategy);
  }
  
  std::ostream& print(std::ostream& out) {
    out << "{N-tree, degree = " << degree << ", maxLeafSize = " << maxLeafSize
        << ", partition strategy = " << partitionStrategy << "}" << endl;
    if (!root) {
      out << "empty";
    }
    else {
      root->print(out, dc);
    }
    return out;
  }
  
  DistComp& getDistComp() const {
    return dc;
  }
  
  node_t* getRoot() {
    return root;
  }
  
  ntree_t* clone() {
    ntree_t* res = new ntree_t(degree, maxLeafSize, dc);
    if (root) {
      res->root = root->clone();
    }
    return res;
  }
  
  
 private:
  int degree;
  int maxLeafSize;
  node_t* root;
  DistComp dc;
  int partitionStrategy;

  
  static node_t* insert(node_t* root, const T& o, DistComp& dc,
                        const int partitionStrategy = 0) {
    root->insert(o, dc, partitionStrategy);
    return root;
  }
};

template<class T, class DistComp>
class NTreeAux {
 public:
//   typedef NTreeLeafNode<T, DistComp> leafnode_t;
  typedef NTreeNode<T, DistComp> node_t;
//   typedef NTree<T, DistComp> ntree_t;
//   typedef NTreeInnerNode<T, DistComp> innernode_t; 
   
  static T** computeCenters(T** entries, const int size, 
                            const int strategy = 0) {
    T** result = new T*[size];
    switch (strategy) {
      case 0: { // random
        std::vector<int> positions;
        for (int i = 0; i < size; i++) {
          positions.push_back(i);
        }
        std::random_shuffle(positions.begin(), positions.end());
        for (int i = 0; i < size; i++) {
          result[i] = new T(*(entries[positions[i]]));
        }
        return result;
        break;
      }
      case 1: { // spatially balanced
        
        break;
      }
      case 2: { // distance-constrained
        
        break;
      }
      default: {
        assert(false);
      }
    }
    return result;
  }
  
};

#endif
