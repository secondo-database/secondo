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
template<class T, class DistComp, bool opt>
class NTreeInnerNode;

/*
1 class NTreeNode

*/
template<class T, class DistComp, bool opt>
class NTreeNode {
 public:
  typedef NTreeNode<T, DistComp, opt> node_t;
//   typedef NTreeInnerNode<T, DistComp> innernode_t;
  
  virtual ~NTreeNode() {}
  
  virtual bool isLeaf() const = 0;
  
  virtual size_t memSize() const = 0;
  
  virtual node_t* getChild(const int i) = 0;
  
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
  
  int getCount() const {
    return count;
  }
  
  virtual T* getObject(const int pos) = 0;
  
  virtual double getMinDist(const T& q, DistComp& dc) const = 0;

  virtual node_t* clone() = 0;
  
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
  int count;
};

template<class T, class DistComp, bool opt>
class NTreeLeafNode;

/*
2 struct DistVectorContents

Auxiliary struct for sorting a vector with distances

*/
struct DistVectorContents {
  DistVectorContents(const int p, const std::pair<double, double>& dm) :
      pos(p), distmax(dm) {}
      
  bool operator<(const DistVectorContents& dvc) {
    return distObj2d < dvc.distObj2d;
  }
  
  static double dist2d(const std::pair<double, double>& v1,
                       const std::pair<double, double>& v2) {
    return sqrt(pow(v1.first - v2.first, 2) + pow(v1.second - v2.second, 2));
  }
  
  int pos;
  std::pair<double, double> distmax;
  double distObj2d;
};

/*
2 class NTreeInnerNode

*/
template<class T, class DistComp, bool opt>
class NTreeInnerNode : public NTreeNode<T, DistComp, opt> {
 public:
  typedef NTreeInnerNode<T, DistComp, opt> innernode_t;
  typedef NTreeNode<T, DistComp, opt> node_t;
  typedef NTreeLeafNode<T, DistComp, opt> leafnode_t;
  
  using node_t::degree;
  using node_t::maxLeafSize;
  
  NTreeInnerNode(const int d, const int mls) : node_t(d, mls) {
    centers = new T*[node_t::degree];
    children = new node_t*[node_t::degree];
    for (int i = 0; i < node_t::degree; i++) {
      centers[i] = 0;
      children[i] = 0;
    }
  }
  
  NTreeInnerNode(const int d, const int mls, const int m) : innernode_t(d, mls){
    distances = new double*[degree];
    for (int i = 0; i < degree; i++) {
      distances[i] = new double[degree];
    }
    distances2d = new std::pair<double, double>[degree];
    refPtsMethod = m;
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
    if (opt) {
      for (int i = 0; i < degree; i++) {
        delete[] distances[i];
      }
      delete[] distances;
      delete[] distances2d;
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
  
  T* getObject(const int pos) { // only for leaves
    assert(false); 
  }
  
  T* getCenter(const int childPos) const {
    assert(childPos >= 0);
    assert(childPos < node_t::degree);
    return centers[childPos];
  }
  
  int getNearestCenterPos(const T& o, DistComp& dc) const {
    if (opt) { // N-tree2
      bool cand[node_t::degree] = {true};
      std::pair<double, double> odist2d = 
                   make_pair(dc(o, *(innernode_t::centers[refDistPos.first])),
                             dc(o, *(innernode_t::centers[refDistPos.second])));
      DistVectorContents W[node_t::degree];
      for (int i = 0; i < node_t::degree; i++) {
        DistVectorContents dvc(i, distances2d[i]);
        dvc.distObj2d = DistVectorContents::dist2d(distances2d[i], odist2d);
        W[i] = dvc;
      }
      // sort W by dist to odist2d
      std::sort(W.begin(), W.end());
      double Dq[node_t::degree] = {DBL_MAX};
      for (int i = 0; i < node_t::degree; i++) {
        if (cand[i]) {
          Dq[W[i]] = dc(*(innernode_t::centers[W[i].pos]), o);
          for (int j = 0; j < node_t::degree; j++) {
            if (distances[i][W[i].pos] > 2 * Dq[W[i].pos]) {
              cand[i] = false;
            }
          }
        }
      }
      auto it = std::min_element(std::begin(Dq), std::end(Dq));
      return std::distance(std::begin(Dq), it);
    }
    else { // N-tree
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
    int pos = getNearestCenterPos(o, dc);
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
  
  int findChild(const node_t* child) const {
    for (int i = 0; i < node_t::count; i++) {
      if (children[i] == child) {
        return i;
      }
    }
    return -1;
  }
  
  double getMinDist(const T& q, DistComp& dc) const {
    double result = std::numeric_limits<double>::max();
    double dist;
    for (int i = 0; i < node_t::count; i++) {
      dist = dc(*(centers[i]), q);
      if (dist < result) {
        result = dist;
      }
    }
    return result;
  }
  
  innernode_t* clone() {
    innernode_t* res;
    res = new innernode_t(node_t::degree, node_t::maxLeafSize);
    res->count = node_t::count;
    for (int i = 0; i < node_t::count; i++) {
      res->centers[i] = new T(*(centers[i]));
      res->children[i] = children[i]->clone();
    }
    if (opt) {
      res->distances = this->distances;
      res->refDistPos = this->refDistPos;
      res->distances2d = this->distances2d;
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
  
  void addLeaf(std::vector<T>& contents) {
    assert((int)contents.size() <= node_t::maxLeafSize);
    leafnode_t* newLeaf = new leafnode_t(node_t::degree, node_t::maxLeafSize,
                                         contents);
    children[node_t::count] = newLeaf;
    node_t::count++;
  }
  
  void addLeaf(std::vector<T>& contents, T* center) {
    centers[node_t::count] = center;
    addLeaf(contents);
  }
  
  void computeCenters(std::vector<T>& contents, const int strategy = 0) {
    assert(contents.size() >= (unsigned int)node_t::degree);
    switch (strategy) {
      case 0: { // random
        std::vector<int> positions(contents.size());
        for (unsigned int i = 0; i < contents.size(); i++) {
          positions[i] = i;
        }
        std::random_shuffle(positions.begin(), positions.end());
        for (int i = 0; i < node_t::degree; i++) {
          centers[i] = new T(contents[positions[i]]);
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
  
  void partition(std::vector<T>& contents, DistComp& dc,
                 std::vector<std::vector<T> >& partitions) {
    partitions.clear();
    partitions.resize(node_t::degree);
    double dist;
    int partitionPos;
    for (unsigned int i = 0; i < contents.size(); i++) {
      double minDist = std::numeric_limits<double>::max();
      for (int j = 0; j < node_t::degree; j++) {
        dist = dc(contents[i], *centers[j]);
        if (dist < minDist) {
          minDist = dist;
          partitionPos = j;
        }
      }
      partitions[partitionPos].push_back(contents[i]);
    }
  }
  
  void printPartitions(std::vector<T>& contents,
                       std::vector<std::vector<T> >& partitions, DistComp& dc, 
                       const bool printContents, std::ostream& out) {
    out << "Centers: ";
    for (int i = 0; i < node_t::degree; i++) {
      dc.print(*centers[i], printContents, out);
      out << ", ";
    }
    out << endl << endl;
    for (unsigned int i = 0; i < partitions.size(); i++) {
      out << "Partition #" << i << " with " << partitions[i].size()
          << " elems: {";
      for (unsigned int j = 0; j < partitions[i].size(); j++) {
        dc.print(partitions[i][j], printContents, out);
        out << ", ";
      }
      out << "}" << endl;
    }
    out << endl;
  }
  
  void precomputeDistances(DistComp& dc) {
    double maxDist = 0.0;
    refDistPos = std::make_pair(0, 0);
    for (int i = 0; i < degree; i++) {
      for (int j = 0; j < i; j++) {
        distances[i][j] = dc(*(centers[i]), *(centers[j]));
        distances[j][i] = distances[i][j]; // copy to right upper triangle
        if (refPtsMethod == 0) { // select points with maximum distance
          if (distances[i][j] > maxDist) { // update maxDistPos
            refDistPos = std::make_pair(i, j);
            maxDist = distances[i][j];
          }
        }
      }
    }
    for (int i = 0; i < degree; i++) { // compute 2d distance vector
      distances2d[i] = std::make_pair(distances[refDistPos.first][i],
                                      distances[refDistPos.second][i]);
    }
  }
  
  void build(std::vector<T>& contents, DistComp& dc,
             const int partitionStrategy = 0) { // contents.size > maxLeafSize
    computeCenters(contents, partitionStrategy);
    if (opt) {
      precomputeDistances(dc);
    }
    std::vector<std::vector<T> > partitions;
    partition(contents, dc, partitions);
//     printPartitions(contents, partitions, dc, true, cout);
    for (int i = 0; i < degree; i++) {
      if ((int)partitions[i].size() <= maxLeafSize) {
        leafnode_t* newLeaf = new leafnode_t(degree,maxLeafSize, partitions[i]);
        children[i] = newLeaf;
      }
      else {
        if (opt) {
          children[i] = new innernode_t(degree, maxLeafSize, refPtsMethod);
        }
        else {
          children[i] = new innernode_t(degree, maxLeafSize);
        }
        children[i]->build(partitions[i], dc, partitionStrategy);
      }
    }
    node_t::count = degree;
  }
   
 protected:
  T** centers;
  node_t** children;
  
  // only used for N-tree2, i.e., opt = true
  double** distances; // matrix of pairwise center distances
  std::pair<int, int> refDistPos; // two reference center positions
  std::pair<double, double>* distances2d; // distances to maxDistPos
  int refPtsMethod; // 0: maxDist, 1: random, 2: median dist
};

/*
3 class NTreeLeafNode

*/
template<class T, class DistComp, bool opt>
class NTreeLeafNode : public NTreeNode<T, DistComp, opt> {
 public:
  typedef NTreeLeafNode<T, DistComp, opt> leafnode_t;
  typedef NTreeNode<T, DistComp, opt> node_t;
//   typedef NTreeInnerNode<T, DistComp> innernode_t;
  
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
  
  node_t* getChild(const int i) { // only for inner nodes
    assert(false);
  }
  
  T* getCenter(const int childPos) const { // only for inner nodes
    assert(false);
  }
  
  T* getObject(const int pos) {
    return entries[pos];
  }
  
  double getMinDist(const T& q, DistComp& dc) const { // only for inner nodes
    assert(false);
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

template <class T, class DistComp, bool opt>
class RangeIteratorN {
 public:
  typedef RangeIteratorN<T, DistComp, opt> rangeiterator_t;
  typedef NTreeNode<T, DistComp, opt> node_t;
  typedef NTreeLeafNode<T, DistComp, opt> leafnode_t;
  typedef NTreeInnerNode<T, DistComp, opt> innernode_t;

  RangeIteratorN(node_t* root, const T& q, const double r, 
                 const DistComp& di) : pos(0), queryObject(q), range(r), dc(di){
    results.clear();
    if (!root) {
      return;
    }
    collectResults(root);
  }
  
  void collectResults(node_t* node) {
    if (node->isLeaf()) {
      for (int i = 0; i < node->getCount(); i++) {
        if (dc(*(node->getObject(i)), queryObject) <= range) {
          results.push_back(node->getObject(i)->getTid());
        }
      }
    }
    else { // inner node
      double minDist = node->getMinDist(queryObject, dc);
      for (int i = 0; i < node->getCount(); i++) {
        T* c = node->getCenter(i);
//         cout << "  " << *(queryObject.getKey()) << *(c->getKey()) << " § " 
//              << dc(*c, queryObject) << " $ " 
//              << minDist + 2 * range << "  ?" << endl;
        if (dc(*c, queryObject) <= minDist + 2 * range) {
          collectResults(node->getChild(i));
        } 
      }
    }
  }

  const TupleId next() {
    assert(pos >= 0);
    if (pos >= (int)results.size()) {
      return -1;
    }
    pos++;
    return results[pos - 1];
  }

  size_t noComparisons() const{
      return dc.getCount();
  }
  
  int getNoDistFunCalls() const {
    return dc.getNoDistFunCalls();
  }

 private:
  std::vector<TupleId> results;
  int pos;
  T queryObject;
  double range;
  DistComp dc;
};

/*
4 class NTree

This is the main class of this file. It implements a main memory-based N-tree
(opt = false) and an N-tree2 (opt = true).

*/
template<class T, class DistComp, bool opt>
class NTree {
 public:
  typedef NTreeLeafNode<T, DistComp, opt> leafnode_t;
  typedef RangeIteratorN<T, DistComp, opt> rangeiterator_t;
//   typedef NNIterator<T, DistComp> nniterator_t;
  typedef NTreeNode<T, DistComp, opt> node_t;
  typedef NTree<T, DistComp, opt> ntree_t;
  typedef NTreeInnerNode<T, DistComp, opt> innernode_t;
  
  NTree(const int d, const int mls, DistComp& di) :
      degree(d), maxLeafSize(mls), root(0), dc(di), partitionStrategy(0),
      refPtsMethod(0) {}
      
  NTree(const int d, const int mls, const int m, DistComp& di) :
      degree(d), maxLeafSize(mls), root(0), dc(di), partitionStrategy(0),
      refPtsMethod(m) {}
  
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
      if (opt) {
        root = new innernode_t(degree, maxLeafSize, refPtsMethod);
      }
      else {
        root = new innernode_t(degree, maxLeafSize);
      }
      root->build(contents, dc, partitionStrategy);
    }
//     print(cout);
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
  
  rangeiterator_t* rangeSearch(const T& q, double range) const {
    return new rangeiterator_t(root, q, range, dc);
  }
  
  
 protected:
  int degree;
  int maxLeafSize;
  node_t* root;
  DistComp dc;
  int partitionStrategy;
  int refPtsMethod; // 0: maxDist, 1: random, 2: median dist
  
  static node_t* insert(node_t* root, const T& o, DistComp& dc,
                        const int partitionStrategy = 0) {
    root->insert(o, dc, partitionStrategy);
    return root;
  }
};

#endif
