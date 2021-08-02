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
  
  virtual T* getCenter(const int childPos) const = 0;
  
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
  
//   virtual void insert(const T& o, DistComp& dc, 
//                       const int partitionStrategy = 0) = 0;
  
  int getCount() const {
    return count;
  }

  virtual node_t* clone() = 0;
  
  double centerDist(const T& o, const int childPos, DistComp& dc) const {
    return dc(*getCenter(), o);
  }
  
  virtual void build(std::vector<T>& contents, DistComp& dc,
                     const int partitionStrategy = 0) = 0;
  
  virtual void clear(const bool deleteContent) = 0;
  
  virtual std::ostream& print(std::ostream& out, DistComp& dc) const = 0;
  
  virtual std::ostream& print(std::ostream& out, const bool printSubtrees,
                              DistComp& di) const = 0;
  
  
 protected:
  NTreeNode(const int d, const int mls): degree(d), maxLeafSize(mls), count(0){}
   
  int degree;
  int maxLeafSize;
//   innernode_t* parent;
//   int posInParent;
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
  
  NTreeInnerNode(const int d, const int mls) : node_t(d, mls) {
    centers = new T*[node_t::degree];
    children = new node_t*[node_t::degree];
    for (int i = 0; i < node_t::degree; i++) {
      centers[i] = 0;
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
      if (centers[i] && deleteContent) {
        delete centers[i];
      }
      centers[i] = 0;
      children[i] = 0;
    }
    node_t::count = 0;
    if (deleteContent) {
      delete[] centers;
      delete[] children;
    }
  }
  
  virtual bool isLeaf() const {
    return false;
  }
  
  size_t memSize() const {
    size_t res = sizeof(*this) + sizeof(void*) * node_t::degree;
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
  
  T* getCenter(const int childPos) const {
    assert(childPos >= 0);
    assert(childPos < node_t::degree);
    return centers[childPos];
  }
  
  int getNearestChildPos(const T& o, DistComp& dc) const {
    double currentDist = std::numeric_limits<double>::max();
    int result = -1;
    for (int i = 0; i < node_t::count; i++) {
      const T* c = getCenter(i);
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
  
  void deleteChild(const int pos) {
    assert(pos >= 0);
    assert(pos < node_t::count);
    if (children[pos]) {
      delete children[pos];
      children[pos] = 0;
    }
  }
  
  void addChild(node_t* child) {
    assert(node_t::count < node_t::degree);
    children[node_t::count] = child;
    node_t::count++;
  }
  
//   void insert(const T& o, DistComp& dc, const int partitionStrategy = 0) {
//     if (node_t::count < node_t::degree) { // add new child node
//       children[node_t::count] = new leafnode_t(o, node_t::degree, 
//                                                node_t::maxLeafSize);
//       (children[node_t::count])->insert(o, dc, partitionStrategy);
//       (children[node_t::count])->setParent(this, node_t::count);
//       node_t::count++;
//     }
//     else { // node full; insert into child with nearest center
//       int pos = getNearestChildPos(o, dc);
//       children[pos]->insert(o, dc, partitionStrategy);
//     }
//   }
  
  int findChild(const node_t* child) const {
    for (int i = 0; i < node_t::count; i++) {
      if (children[i] == child) {
        return i;
      }
    }
    return -1;
  }
  
  innernode_t* clone() {
    innernode_t* res;
    res = new innernode_t(node_t::degree, node_t::maxLeafSize);
    res->count = node_t::count;
    for (int i = 0; i < node_t::count; i++) {
      res->centers[i] = new T(*(centers[i]));
      res->children[i] = children[i]->clone();
    }  
    return res;
  }
  
  std::ostream& print(std::ostream& out, DistComp& dc) const {
    return print(out, true, dc);
  }
   
  std::ostream& print(std::ostream& out, const bool printSubtrees,
                      DistComp& dc) const {
    out << "( \"inner node: ";
    if (printSubtrees) {
      out << " (";
      for (int i = 0; i < node_t::count; i++) {
        out << "center #" << i << " = ";
        centers[i]->getKey()->Print(out);
        out << ", child #" << i << " = ";
        children[i]->print(out, dc);
      }
      out << " )" << endl;
    }
    out << ")";
    return out;
  }
  
//   void split(DistComp& dc, const int partitionStrategy = 0) {}

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
    int sum = 1;
    for (int i = 0; i < node_t::count; i++) {
      sum += children[i]->getNoNodes();
    }
    return sum;
  }
  
  bool isOverflow() const {
    return node_t::count > node_t::degree;
  }
  
  void addLeaf(std::vector<T>& contents, T* center) {
    assert((int)contents.size() <= node_t::maxLeafSize);
    centers[node_t::count] = center;
    leafnode_t* newLeaf = new leafnode_t(node_t::degree, node_t::maxLeafSize,
                                         contents);
    children[node_t::count] = newLeaf;
    node_t::count++;
  }
  
  void computeCenters(std::vector<T>& contents, std::vector<int>& result,
                                  const int strategy = 0) {
    assert(contents.size() >= (unsigned int)node_t::degree);
    result.clear();
    result.resize(node_t::degree);
    switch (strategy) {
      case 0: { // random
        std::vector<int> positions;
        for (unsigned int i = 0; i < contents.size(); i++) {
          positions[i] = i;
        }
        std::random_shuffle(positions.begin(), positions.end());
        for (int i = 0; i < node_t::degree; i++) {
          result[i] = positions[i];
        } // node_t::degree random positions between 0 and contents.size() - 1
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
  }
  
  void partition(std::vector<T>& contents, std::vector<int>& centers,
                 DistComp& dc, std::vector<std::vector<T> >& partitions) {
    partitions.clear();
    partitions.resize(node_t::degree);
    double dist;
    int partitionPos;
    for (unsigned int i = 0; i < contents.size(); i++) {
      double minDist = std::numeric_limits<double>::max();
      for (unsigned int j = 0; j < centers.size(); j++) {
        dist = dc(contents[i], contents[centers[j]]);
        if (dist < minDist) {
          minDist = dist;
          partitionPos = j;
        }
      }
      partitions[partitionPos].push_back(contents[i]);
    }
  }
  
  void build(std::vector<T>& contents, DistComp& dc,
             const int partitionStrategy = 0) { // contents.size > maxLeafSize
    std::vector<int> centerPos;
    computeCenters(contents, centerPos, partitionStrategy);
    std::vector<std::vector<T> > partitions;
    partition(contents, centerPos, dc, partitions);
    // TODO: do the important things.


//       for (int j = 0; j < node_t::degree; j++) {
//         if ((int)partitions[j].size() <= node_t::maxLeafSize) {
//           addLeaf(partitions[j], *(centers[j]));
//         }
//         else {
//           innernode_t* newInnerNode = new innernode_t(*(centers[j]), 
//                                         node_t::degree, node_t::maxLeafSize);
//           newInnerNode->build(partitions[j], dc, partitionStrategy);
//           addChild(newInnerNode);
//         }
//       }

  }
   
 private:
  T** centers;
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
  
  NTreeLeafNode(const int d, const int mls) : node_t(d, mls) {
    entries = new T*[node_t::maxLeafSize];
    for (int i = 0; i < node_t::maxLeafSize; i++) {
      entries[i] = 0;
    }
  }
  
  NTreeLeafNode(const int d, const int mls, std::vector<T>& contents) :
                                                         NTreeLeafNode(d, mls) {
    insert(contents);
  }
  
  ~NTreeLeafNode() {
    for (int i = 0; i < node_t::maxLeafSize; i++) {
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
  
  T* getCenter(const int childPos) const {
    return 0;
  }
  
  void insert(std::vector<T>& contents) {
    assert(node_t::count + (int)contents.size() <= node_t::maxLeafSize);
    for (unsigned int i = 0; i < contents.size(); i++) {
      entries[i] = new T(contents[i]);
    }
    node_t::count += contents.size();
  }
  
  void build(std::vector<T>& contents, DistComp& dc,
             const int partitionStrategy = 0) {
    assert(false);
  }
  
  leafnode_t* clone() {
    leafnode_t* res;
    res = new leafnode_t(node_t::degree, node_t::maxLeafSize);
    res->count = node_t::count;
    for (int i = 0; i < node_t::count; i++) {
      res->entries[i] = new T(*entries[i]);
    }  
    return res;
  }
  
  std::ostream& print(std::ostream& out, DistComp& dc) const {
    out << "[ leaf node: \"";
    for (int i = 0; i < node_t::count; i++) {
      if (i > 0) {
        out << ", ";
      }
      dc.print(*entries[i], out);
    }
    out << "\"]" << endl;
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
  
  void build(std::vector<T>& contents, const int partitionStrategy = 0) {
    if (root) {
      delete root;
    }
    if (contents.size() <= (unsigned int)maxLeafSize) {
      root = new leafnode_t(degree, maxLeafSize, contents);
    }
    else {
      root = new innernode_t(degree, maxLeafSize);
      root->build(contents, dc, partitionStrategy);
    }
    print(cout);
  }
  
//   void insert(T& o, const int partitionStrategy = 0) {
//     if (!root) {
//       root = new innernode_t(o, degree, maxLeafSize);
//     }
//     root = insert(root, o, dc, partitionStrategy);
//   }
  
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
  
  DistComp& getDistComp() {
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

#endif
