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
#include <random>

/*
Implementation of the N-tree

It is as powerful as the M-tree but supports more efficient range and nearest
neighbor searches.

*/

/*
1 Enums for candidate ordering and pruning method

*/
enum CandOrder {RANDOM, PIVOT2, PIVOT3};
enum PruningMethod {SIMPLE, MINDIST};

template<class T, class DistComp, int variant>
class NTreeInnerNode;

template<class T, class DistComp, int variant>
class NTreeLeafNode;

/*
2 struct PosDistPair

Auxiliary struct for sorting a vector with distances

*/
struct PosDistPair {
  PosDistPair() {}
  
  PosDistPair(const int p) : pos(p) {}
      
  bool operator<(const PosDistPair& pdp) {
    return distEuclid < pdp.distEuclid;
  }
 
  friend bool operator<(const PosDistPair& pair1, const PosDistPair& pair2) {
    return pair1.distEuclid < pair2.distEuclid;
  }
  
  static double dist2d(const std::pair<double, double>& v1,
                       const std::pair<double, double>& v2) {
    return sqrt(pow(v1.first - v2.first, 2) + pow(v1.second - v2.second, 2));
  }
  
  static double dist2d(const std::pair<double, double>& v1,
                       const std::tuple<double, double, double>& t2) {
    return sqrt(pow(std::get<0>(v1) - std::get<0>(t2), 2) +
                pow(std::get<1>(v1) - std::get<1>(t2), 2));
  }
  
  static double dist3d(const std::tuple<double, double, double>& t1,
                       const std::tuple<double, double, double>& t2) {
    return sqrt(pow(std::get<0>(t1) - std::get<0>(t2), 2) +
                pow(std::get<1>(t1) - std::get<1>(t2), 2) +
                pow(std::get<2>(t1) - std::get<2>(t2), 2));
  }
  
  int pos;
//   std::pair<double, double> distmax;
  double distEuclid;
};

/*
1 class NTreeNode

*/
template<class T, class DistComp, int variant>
class NTreeNode {
 public:
  typedef NTreeNode<T, DistComp, variant> node_t;
  typedef NTreeInnerNode<T, DistComp, variant> innernode_t;
  typedef NTreeLeafNode<T, DistComp, variant> leafnode_t;
  
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
  
  virtual void build(std::vector<T>& contents, DistComp& dc, int depth,
                     const int partitionStrategy) = 0;
  
  virtual void clear(const bool deleteContent) = 0;
  
  virtual std::ostream& print(std::ostream& out, DistComp& dc) const = 0;
  
  virtual std::ostream& print(std::ostream& out, const bool printSubtrees,
                              DistComp& di) const = 0;
  
  virtual double evaluateDist(const int i, const T& o, DistComp& dc) const = 0;
                              
  void initAuxStructures(const int size) {
    distMatrix = new double*[size];
    for (int i = 0; i < size; i++) {
      distMatrix[i] = new double[size];
      std::fill_n(distMatrix[i], size, -1.0);
    }
    switch (candOrder) {
      case RANDOM: {
        break;
      }
      case PIVOT2: {
        distances2d = new std::pair<double, double>[size];
        std::get<2>(refDistPos) = -1;
        break;
      }
      case PIVOT3: {
        distances3d = new std::tuple<double, double, double>[size];
        break;
      }
      default: {
        assert(false);
      }
    }
  }
  
  void deleteAuxStructures(const int size) {
    for (int i = 0; i < size; i++) {
      delete[] distMatrix[i];
    }
    delete[] distMatrix;
    if (candOrder == PIVOT2) {
      delete[] distances2d;
    }
    else if (candOrder == PIVOT3) {
      delete[] distances3d;
    }
  }
  
  int getNearestCenterPos(const T& o, DistComp& dc, 
                          std::tuple<double, double, double>& odist3d, 
                          const bool isLeaf, double& minDist) {
    int size = isLeaf ? ((leafnode_t*)this)->getNoEntries() : degree;
    bool cand[size];
    std::fill_n(cand, size, true);
    minDist = DBL_MAX;
    double tempDist, d_ij;
    int result = 0;
    PosDistPair W[size];
    for (int i = 0; i < size; i++) {
      PosDistPair pdp(i);
      if (candOrder == PIVOT2) {
        pdp.distEuclid = PosDistPair::dist2d(distances2d[i], odist3d);
      }
      else if (candOrder == PIVOT3) {
        pdp.distEuclid = PosDistPair::dist3d(distances3d[i], odist3d);
      }
      W[i] = pdp;
    }
    // sort W by dist to odist2d
//       for (int i = 0; i < size; i++) {
//         cout << "(" << W[i].pos << ", " << W[i].distObj2d << "),  ";
//       }
//       cout << endl;
    if (candOrder == PIVOT2 || candOrder == PIVOT3) {
      std::sort(W, W + size);
    }
//       for (int i = 0; i < degree; i++) {
//         cout << "(" << W[i].pos << ", " << W[i].distObj2d << "),  ";
//       }
//       cout << endl << endl;
    double Dq[size];
    std::fill_n(Dq, size, DBL_MAX);
    for (int i = 0; i < size; i++) { // TODO: use ordering of W!
      if (cand[i]) {
        tempDist = isLeaf ? ((leafnode_t*)this)->evaluateDist(W[i].pos, o, dc) :
                            ((innernode_t*)this)->evaluateDist(W[i].pos, o, dc);
//         evaluateDist(W[i].pos, o, dc);
        Dq[W[i].pos] = tempDist;
        if (tempDist < minDist) {
          minDist = tempDist;
          result = W[i].pos;
        }
        for (int j = 0; j < size; j++) {//prune according to pMethod
//           cout << "i = " << i << "; entry (" << j << ", " << W[i].pos;
          d_ij = distMatrix[j][W[i].pos];
//           cout << ") is " << distMatrix[j][W[i].pos] << endl;
          if (pMethod == SIMPLE) {
            if (d_ij > 2 * tempDist) {
//                 cout << "set cand[" << j << "] to FALSE" << endl;
              cand[j] = false;
            }
          }
          else if (pMethod == MINDIST) {
            if (d_ij < tempDist - minDist || d_ij > tempDist + minDist) {
              cand[j] = false;
            }
          }
        }
      }
    }
//       for (int i = 0; i < size; i++) {
//         cout << Dq[i] << "  ";
//       }
    return result;
  }
    
  void precomputeDistances(DistComp& dc, const bool isLeaf) {
    double maxDist = 0.0;
    refDistPos = std::make_tuple(0, 0, 0);
    int size = isLeaf ? ((leafnode_t*)this)->getNoEntries() : degree;
    for (int i = 0; i < size; i++) {
      distMatrix[i][i] = 0.0;
      for (int j = 0; j < i; j++) {
        distMatrix[i][j] = isLeaf ? ((leafnode_t*)this)->evaluateDist(i, j, dc):
                                   ((innernode_t*)this)->evaluateDist(i, j, dc);
//           dc(*(centers[i]), *(centers[j]));
        distMatrix[j][i] = distMatrix[i][j]; // fill right upper triangle
        // select points with maximum distance between each other
        if (distMatrix[i][j] > maxDist) { // update refDistPos
          std::get<0>(refDistPos) = i;
          std::get<1>(refDistPos) = j;
          maxDist = distMatrix[i][j];
        }
      }
    }
    if (candOrder == PIVOT2) {
      for (int i = 0; i < size; i++) { // compute 2d distance vector
//           cout << "i = " << i << ", std::get<0>(refDistPos) = " 
//                << std::get<0>(refDistPos) << ", dist = " 
//                << distMatrix[std::get<0>(refDistPos)][i];
        distances2d[i].first = distMatrix[std::get<0>(refDistPos)][i];
//           cout << ", std::get<1>(refDistPos) = " << std::get<1>(refDistPos) 
//                << ", dist = " << distMatrix[std::get<1>(refDistPos)][i] 
//                << endl;
        distances2d[i].second = distMatrix[std::get<1>(refDistPos)][i];
      }
    }
    else if (candOrder == PIVOT3) { // find point with max dist to first two pts
      maxDist = 0.0;
      double distSum;
      for (int i = 0; i < size; i++) {
        if (i != std::get<0>(refDistPos) && i != std::get<1>(refDistPos)) {
          distSum = distMatrix[i][std::get<0>(refDistPos)] + 
                    distMatrix[i][std::get<1>(refDistPos)];
          if (distSum > maxDist) {
            std::get<2>(refDistPos) = i;
            maxDist = distSum;
          }
        }
      }
      for (int i = 0; i < size; i++) { // compute 3d distance vector
        std::get<0>(distances3d[i]) = distMatrix[std::get<0>(refDistPos)][i];
        std::get<1>(distances3d[i]) = distMatrix[std::get<1>(refDistPos)][i];
        std::get<2>(distances3d[i]) = distMatrix[std::get<2>(refDistPos)][i];
      }
    }
  }
  
  double getPrecomputedDist(const int pos1, const int pos2, const bool isLeaf)
                                                                         const {
    int size = isLeaf ? ((leafnode_t*)this)->getNoEntries() : degree;
    assert(pos1 >= 0 && pos1 < size && pos2 >= 0 && pos2 < size);
    return distMatrix[pos1][pos2];
  }
  
  
 protected:
  NTreeNode(const int d, const int mls) :
    degree(d), maxLeafSize(mls), count(0), noDistComp(0), distMatrix(0),
    distances2d(0), distances3d(0), candOrder(RANDOM), pMethod(SIMPLE) {}
   
  int degree, maxLeafSize, count, noDistComp; 
  
  // only used for N-tree2, i.e., variant == 2
  double** distMatrix; // matrix of pairwise center distances
  std::tuple<int, int, int> refDistPos; // 2 or 3 reference center positions
  std::pair<double, double>* distances2d; // 2d distances to refDistPos
  std::tuple<double, double, double>* distances3d; // 3d distances to refDistPos
  CandOrder candOrder; // random, two pivots, three pivots
  PruningMethod pMethod; // simple, minDist-based
};

template<class T, class DistComp, int variant>
class NTreeLeafNode;

/*
2 class NTreeInnerNode

*/
template<class T, class DistComp, int variant>
class NTreeInnerNode : public NTreeNode<T, DistComp, variant> {
 public:
  typedef NTreeInnerNode<T, DistComp, variant> innernode_t;
  typedef NTreeNode<T, DistComp, variant> node_t;
  typedef NTreeLeafNode<T, DistComp, variant> leafnode_t;
  
  using node_t::degree;
  using node_t::maxLeafSize;
  using node_t::noDistComp;
  using node_t::distMatrix;
  using node_t::refDistPos;
  using node_t::distances2d;
  using node_t::distances3d;
  using node_t::candOrder;
  using node_t::pMethod;
  
  NTreeInnerNode(const int d, const int mls) : node_t(d, mls) {
    centers = new T*[node_t::degree];
    children = new node_t*[node_t::degree];
    for (int i = 0; i < node_t::degree; i++) {
      centers[i] = 0;
      children[i] = 0;
    }
    maxDist = 0;
    if (variant > 1) {
      maxDist = new double[degree];
      std::fill_n(maxDist, degree, 0.0);
    }
  }
  
  NTreeInnerNode(const int d, const int mls, const CandOrder c, 
                 const PruningMethod pm) : innernode_t(d, mls) {
    candOrder = c;
    pMethod = pm;
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
      if (variant > 1) {
        delete[] maxDist;
      }
    }
    if (variant == 2 || variant >= 6) {
      node_t::deleteAuxStructures(degree);
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
    if (variant == 2) {
      res += node_t::degree * node_t::degree * sizeof(double); // distMatrix
      res += 3 * sizeof(int);
      if (candOrder == PIVOT2) {
        res += node_t::degree * 2 * sizeof(double);
      }
      else if (candOrder == PIVOT3) {
        res += node_t::degree * 3 * sizeof(double);
      }
    }
    if (variant > 1) {
      res += node_t::degree * sizeof(double); // maxDist
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
  
  int getNearestCenterPos(const T& o, DistComp& dc, double& minDist) {
    if (variant == 2 || variant >= 6) { // N-tree2
      std::tuple<double, double, double> odist3d;
      if (candOrder == PIVOT2) {
        odist3d = std::make_tuple(dc(o, *(centers[std::get<0>(refDistPos)])),
                                  dc(o, *(centers[std::get<1>(refDistPos)])),
                                  0.0);
      }
      if (candOrder == PIVOT3) {
        std::get<2>(odist3d) = dc(o, *(centers[std::get<2>(refDistPos)]));
      }
      return node_t::getNearestCenterPos(o, dc, odist3d, false, minDist);
    }
    else { // N-tree or N-tree5
      double currentDist = std::numeric_limits<double>::max();
      int result = -1;
      for (int i = 0; i < node_t::degree; i++) {
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
    double dummy;
    int pos = getNearestCenterPos(o, dc, dummy);
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
  
  double evaluateDist(const int i, const T& o, DistComp& dc) const {
    assert(i >= 0 && i < degree);
    return dc(*(centers[i]), o);
  }
  
  double evaluateDist(const int i, const int j, DistComp& dc) const {
    assert(i >= 0 && i < node_t::degree && j >= 0 && j < node_t::degree);
    return dc(*(centers[i]), *(centers[j]));
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
    if (variant == 2 || variant >= 6) {
      res->initAuxStructures(node_t::degree);
      res->distMatrix = this->distMatrix;
      res->refDistPos = this->refDistPos;
      res->distances2d = this->distances2d;
      // TODO: ...
    }
    if (variant > 1) {
      res->maxDist = new double[node_t::degree];
      memcpy(res->maxDist, maxDist, node_t::degree * sizeof(double));
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
        std::shuffle(positions.begin(), positions.end(), 
                     std::mt19937(std::random_device()()));
        for (int i = 0; i < node_t::degree; i++) {
          centers[i] = new T(contents[positions[i]]);
        } // node_t::degree random positions between 0 and contents.size() - 1
        break;
      }
      case 1: { // spatially balanced
        // TODO
        break;
      }
      case 2: { // distance-constrained
        // TODO
        break;
      }
      case 3: { // fixed
        for (int i = 0; i < node_t::degree; i++) {
          centers[i] = new T(contents[i]);
        }
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
    double dist(-1.0), centerDist(-1.0);
    int partitionPos = -1;
    if (variant == 2 || variant >= 6) { // NTree2
      for (unsigned int i = 0; i < contents.size(); i++) {
        for (int j = 0; j < degree; j++) {
          if (contents[i].getTid() == centers[j]->getTid()) {
            partitionPos = j;
            j = degree;
          }
        }
        if (partitionPos == -1) {
//           cout << "call gNCP for contents # " << i << endl;
          partitionPos = getNearestCenterPos(contents[i], dc, centerDist);
        }
        partitions[partitionPos].push_back(contents[i]);
        if (centerDist > maxDist[partitionPos]) {
          maxDist[partitionPos] = centerDist;
        }
        partitionPos = -1;
      }
    }
    else { // NTree or NTree5
      for (unsigned int i = 0; i < contents.size(); i++) {
        centerDist = std::numeric_limits<double>::max();
        for (int j = 0; j < node_t::degree; j++) {
          if (contents[i].getTid() == centers[j]->getTid()) {
            partitionPos = j;
            j = degree;
          }
          else {
            dist = dc(contents[i], *centers[j]);
            if (dist < centerDist) {
              centerDist = dist;
              partitionPos = j;
            }
          }
        }
        partitions[partitionPos].push_back(contents[i]);
        if (variant == 5) {
          if (centerDist > maxDist[partitionPos]) {
            maxDist[partitionPos] = centerDist;
          }
        }
      }
    }
  }
  
  void printPartitions(std::vector<T>& contents,
              std::vector<std::vector<T> >& partitions, int depth, DistComp& dc,
                       const bool printContents, std::ostream& out) {
    std::string spaces;
    for (int i = 0; i < depth; i++) {
      spaces += "  ";
    }
    out << spaces << "Centers: ";
    for (int i = 0; i < node_t::degree; i++) {
      dc.print(*centers[i], printContents, out);
      out << ", ";
    }
    out << endl << endl;
    for (unsigned int i = 0; i < partitions.size(); i++) {
      out << spaces << "Partition #" << i << " with " << partitions[i].size()
          << " elems: {";
      for (unsigned int j = 0; j < partitions[i].size(); j++) {
        dc.print(partitions[i][j], printContents, out);
        out << ", ";
      }
      out << "}" << endl;
    }
    out << endl;
  }
  
  void build(std::vector<T>& contents, DistComp& dc, int depth,
             const int partitionStrategy) { 
    // precondition: contents.size > maxLeafSize
//     cout << "start BUILD, " << "depth " << depth << ", " << contents.size()
//          << " elems, counter = " << dc.getNoDistFunCalls() << endl;
    int noDistFunCallsBefore = dc.getNoDistFunCalls();
    depth++;
    computeCenters(contents, partitionStrategy);
    if (variant == 2 || variant >= 6) {
      node_t::initAuxStructures(degree);
      node_t::precomputeDistances(dc, false);
    }
    std::vector<std::vector<T> > partitions;
    partition(contents, dc, partitions);
//     printPartitions(contents, partitions, depth, dc, false, cout);
    int noDistFunCallsAfter = dc.getNoDistFunCalls();
    noDistComp = noDistFunCallsAfter - noDistFunCallsBefore;
    for (int i = 0; i < degree; i++) {
      if ((int)partitions[i].size() <= maxLeafSize) {
        children[i] = new leafnode_t(degree, maxLeafSize, dc, 
                                     centers[i]->getTid(), partitions[i]);
      }
      else {
        children[i] = (variant == 2 ? new innernode_t(degree, maxLeafSize, 
                candOrder, pMethod) : new innernode_t(degree, maxLeafSize));
        children[i]->build(partitions[i], dc, depth, partitionStrategy);
      }
    }
    node_t::count = degree;
  }
  
  double getMaxDist(const int i) const {
    assert(i >= 0 && i < degree);
    return maxDist[i];
  }
   
 protected:
  T** centers;
  node_t** children;
  double* maxDist;
};

/*
3 class NTreeLeafNode

*/
template<class T, class DistComp, int variant>
class NTreeLeafNode : public NTreeNode<T, DistComp, variant> {
 public:
  typedef NTreeLeafNode<T, DistComp, variant> leafnode_t;
  typedef NTreeNode<T, DistComp, variant> node_t;
//   typedef NTreeInnerNode<T, DistComp, variant> innernode_t;
  
  using node_t::degree;
  using node_t::maxLeafSize;
  using node_t::noDistComp;
  using node_t::distMatrix;
  using node_t::refDistPos;
  using node_t::distances2d;
  using node_t::distances3d;
  using node_t::candOrder;
  using node_t::pMethod;
  
  NTreeLeafNode(const int d, const int mls) : node_t(d, mls) {
    entries = new T*[node_t::maxLeafSize];
    for (int i = 0; i < node_t::maxLeafSize; i++) {
      entries[i] = 0;
    }
  }
  
  NTreeLeafNode(const int d, const int mls, DistComp& dc, 
                const TupleId centerTid, std::vector<T>& contents) : 
                                                         NTreeLeafNode(d, mls) {
    insert(contents, centerTid);
    int noDistFunCallsBefore = dc.getNoDistFunCalls();
    if (variant == 2 || variant >= 6) {
      node_t::initAuxStructures(getNoEntries());
      node_t::precomputeDistances(dc, true);
    }
    int noDistFunCallsAfter = dc.getNoDistFunCalls();
    noDistComp = noDistFunCallsAfter - noDistFunCallsBefore;
  }
  
  ~NTreeLeafNode() {
    for (int i = 0; i < node_t::maxLeafSize; i++) {
      if (entries[i]) {
        delete entries[i];
      }
    }
    delete[] entries;
    if (variant == 2 || variant >= 6) {
      node_t::deleteAuxStructures(getNoEntries());
    }
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
  
  double evaluateDist(const int i, const T& o, DistComp& dc) const {
    assert(i >= 0 && i < getNoEntries());
    return dc(*(entries[i]), o);
  }
  
  double evaluateDist(const int i, const int j, DistComp& dc) const {
    assert(i >= 0 && i < getNoEntries() && j >= 0 && j < getNoEntries());
    return dc(*(entries[i]), *(entries[j]));
  }
  
  double getMinDist(const T& q, DistComp& dc) const { // only for inner nodes
    assert(false);
  }
  
  void insert(std::vector<T>& contents, const TupleId centerTid) {
    assert(node_t::count + (int)contents.size() <= node_t::maxLeafSize);
    for (unsigned int i = 0; i < contents.size(); i++) {
      if (contents[i].getTid() == centerTid && i > 0) {
        entries[i] = entries[0];
        entries[0] = new T(contents[i]);
      }
      else {
        entries[i] = new T(contents[i]);
      }
    }
    node_t::count += contents.size();
  }
  
  int getNearestCenterPos(const T& o, DistComp& dc, double& minDist) {
    if (variant == 2 || variant >= 6) {
      std::tuple<double, double, double> odist3d;
      if (node_t::candOrder == PIVOT2) {
        odist3d = std::make_tuple(dc(o, *(entries[std::get<0>(refDistPos)])),
                                  dc(o, *(entries[std::get<1>(refDistPos)])),
                                  0.0);
      }
      if (node_t::candOrder == PIVOT3) {
        std::get<2>(odist3d) = dc(o, *(entries[std::get<2>(refDistPos)]));
      }
      return node_t::getNearestCenterPos(o, dc, odist3d, true, minDist);
    }
    else { // N-tree or N-tree5
      double currentDist = std::numeric_limits<double>::max();
      int result = -1;
      for (int i = 0; i < node_t::degree; i++) {
        const T* c = getObject(i);
        double dist = dc(*c, o);
        if (dist < currentDist) {
          result = i;
          currentDist = dist;
        }
      }
      return result;
    }
  }
  
  void build(std::vector<T>& contents, DistComp& dc, int depth,
             const int partitionStrategy) {
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
9 struct NTreeStat

This type is applied for counting (expensive) distance computations per inner 
node and per leaf.

*/
struct NTreeStat {
  NTreeStat() : noInnerNodes(0), noLeaves(0), noDCInnerNodes(0), noDCLeaves(0){}
  
  std::ostream& print(std::ostream& out, const int noDCTotal = 0,
                      const bool isSearch = false) {
    if (noDCTotal > 0) {
      noDCInnerNodes = noDCTotal - noDCLeaves;
    }
    out << (isSearch ? "SEARCH" : "TREE") << " STATISTICS:" << endl;
    out << (isSearch ? "------" : "----") << "-----------" << endl;
    out << (isSearch ? "visited: " : "created: ") << noInnerNodes 
        << " inner nodes, " << noLeaves << " leaves" << endl;
    double avgDCInnerNode = (double)noDCInnerNodes / noInnerNodes;
    double avgDCLeaves = (double)noDCLeaves / noLeaves;
    out << "number of distance computations: " << noDCInnerNodes
         << " at inner nodes (avg. = " << avgDCInnerNode << "), " 
         << noDCLeaves << " at leaves (avg. = " << avgDCLeaves << ")." << endl;
    return out;
  }
  
  int noInnerNodes, noLeaves;
  int noDCInnerNodes, noDCLeaves;
};

template <class T, class DistComp, int variant>
class RangeIteratorN {
 public:
  typedef RangeIteratorN<T, DistComp, variant> rangeiterator_t;
  typedef NTreeNode<T, DistComp, variant> node_t;
  typedef NTreeLeafNode<T, DistComp, variant> leafnode_t;
  typedef NTreeInnerNode<T, DistComp, variant> innernode_t;

  RangeIteratorN(node_t* root, const T& q, const double r, const DistComp& di) :
                                      pos(0), queryObject(q), range(r), dc(di) {
    results.clear();
    if (!root) {
      return;
    }
    switch (variant) {
      case 1: {
        collectResultsNtree(root);
        break;
      }
      case 2: {
        collectResultsNtree2(root);
        break;
      }
      case 5: {
        collectResultsNtree5(root);
        break;
      }
      case 6: {
        collectResultsNtree6(root);
        break;
      }
      case 7: {
        collectResultsNtree7(root);
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
    stat.print(cout, dc.getNoDistFunCalls(), true);
  }
  
  void addResult(const T* o) {
    results.push_back(o->getTid());
//     cout << "[" << o->getTid() << "] ";
//          << ": " << *(o->getKey()) << "] ";
  }
  
  void reportEntireSubtree(node_t* node) {
    if (node->isLeaf()) {
      for (int i = 0; i < ((leafnode_t*)node)->getNoEntries(); i++) {
        addResult(((leafnode_t*)node)->getObject(i));
      }
      stat.noLeaves++;
    }
    else {
      for (int i = 0; i < ((innernode_t*)node)->getCount(); i++) {
        reportEntireSubtree(((innernode_t*)node)->getChild(i));
      }
      stat.noInnerNodes++;
    }
  }
  
  void collectResultsNtree(node_t* node) {
    if (node->isLeaf()) {
      int noDistFunCallsBefore = dc.getNoDistFunCalls();
      for (int i = 0; i < node->getCount(); i++) {
        if (dc(*(node->getObject(i)), queryObject) <= range) {
          addResult(node->getObject(i));
        }
      }
      int noDistFunCallsAfter = dc.getNoDistFunCalls();
      stat.noLeaves++;
      stat.noDCLeaves+= noDistFunCallsAfter - noDistFunCallsBefore;
    }
    else { // inner node
      double minDist = node->getMinDist(queryObject, dc);
      for (int i = 0; i < node->getCount(); i++) {
        T* c = node->getCenter(i);
//         cout << "  " << *(queryObject.getKey()) << *(c->getKey()) << " § "
//              << dc(*c, queryObject) << " $ " 
//              << minDist + 2 * range << "  ?" << endl;
        if (dc(*c, queryObject) <= minDist + 2 * range) {
          collectResultsNtree(node->getChild(i));
        } 
      }
      stat.noInnerNodes++;
    }
  }
  
  void collectResultsNtree2(node_t* node) {
    if (node->isLeaf()) {
      int noDistFunCallsBefore = dc.getNoDistFunCalls();
      double distQnnq, distPnnq;
      int nnq = ((leafnode_t*)node)->getNearestCenterPos(queryObject, dc,
                                                          distQnnq);
      for (int i = 0; i < node->getCount(); i++) {
        distPnnq = node->getPrecomputedDist(nnq, i, true);
        if (distPnnq <= range - distQnnq) { // report
//             cout << "  report object" << endl;
          addResult(((leafnode_t*)node)->getObject(i));
        }
        else if (distPnnq <= range + distQnnq) { // evaluate
          if (dc(*(node->getObject(i)), queryObject) <= range) {
//               cout << "  evaluate object dist" << endl;
            addResult(((leafnode_t*)node)->getObject(i));
          }
        } // else: ignore entry
      }
      int noDistFunCallsAfter = dc.getNoDistFunCalls();
      stat.noLeaves++;
      stat.noDCLeaves+= noDistFunCallsAfter - noDistFunCallsBefore;
    }
    else { // inner node
      double distQnnq, distPnnq, maxDist;
      int nnq = ((innernode_t*)node)->getNearestCenterPos(queryObject, dc,
                                                          distQnnq);
      for (int i = 0; i < node->getCount(); i++) {
        distPnnq = node->getPrecomputedDist(i, nnq, false);
        maxDist = ((innernode_t*)node)->getMaxDist(i);
        if (distPnnq <= range - distQnnq - maxDist) {
//             cout << "report entire subtree" << endl;
          reportEntireSubtree(node->getChild(i));
        }
        else if (distPnnq <= range + distQnnq + maxDist) { // evaluate subtree
//             cout << "evaluate subtree" << endl;
          if (distPnnq <= 2 * distQnnq + 2 * range) {
            if (distPnnq <= 2 * range ||
                dc(*(node->getCenter(i)), queryObject) <= distQnnq + 2*range) {
              collectResultsNtree2(node->getChild(i));
            }
          }
        } // else: ignore subtree
      }
      stat.noInnerNodes++;
    }
  }
  
  void collectResultsNtree5(node_t* node) {
    if (node->isLeaf()) {
      int noDistFunCallsBefore = dc.getNoDistFunCalls();
      for (int i = 0; i < node->getNoEntries(); i++) {
        if (dc(queryObject, *(node->getObject(i))) <= range) {
          addResult(((leafnode_t*)node)->getObject(i));
        }
      }
      int noDistFunCallsAfter = dc.getNoDistFunCalls();
      stat.noLeaves++;
      stat.noDCLeaves+= noDistFunCallsAfter - noDistFunCallsBefore;
    }
    else { // inner node
      for (int i = 0; i < node->getDegree(); i++) {
        double dist = dc(queryObject, *(node->getCenter(i)));
        if (dist + ((innernode_t*)node)->getMaxDist(i) <= range) {
          reportEntireSubtree(node->getChild(i));
        }
        else if (dist <= ((innernode_t*)node)->getMaxDist(i) + range) {
          collectResultsNtree5(node->getChild(i));
        }
      }
      stat.noInnerNodes++;
    }
  }
  
  void collectResultsNtree6(node_t* node) {
    int noCands = (node->isLeaf() ? node->getNoEntries() : node->getDegree());
    bool cand[noCands];
    std::fill_n(cand, noCands, true);
    double u, d_ij;
    if (node->isLeaf()) {
      for (int i = 0; i < noCands; i++) {
        if (cand[i]) {
          u = dc(queryObject, *((leafnode_t*)node)->getObject(i));
          for (int j = i + 1; j < noCands; j++) { // prune2
            if (cand[j]) {
              d_ij = node->getPrecomputedDist(i, j, true);
              if (range > u + d_ij) {
                addResult(((leafnode_t*)node)->getObject(j));
                cand[j] = false;
              }
              else if (range < abs(u - d_ij)) {
                cand[j] = false;
              }
            }
          }
          if (cand[i] && range > u) {
            addResult(((leafnode_t*)node)->getObject(i));
            cand[i] = false;
          }
        }
      }
    }
    else { // inner node
      for (int i = 0; i < noCands; i++) {
        if (cand[i]) {
          u = dc(queryObject, *((innernode_t*)node)->getCenter(i));
          for (int j = i + 1; j < noCands; j++) { // prune2
            if (cand[j]) {
              d_ij = node->getPrecomputedDist(i, j, true);
              if (range > u + d_ij + ((innernode_t*)node)->getMaxDist(j)) {
                reportEntireSubtree(node->getChild(j));
                cand[j] = false;
              }
              else if (range < abs(u - d_ij) - 
                      ((innernode_t*)node)->getMaxDist(j)) {
                cand[j] = false;
              }
            }
          }
          if (cand[i]) {
            if (range > u + ((innernode_t*)node)->getMaxDist(i)) {
              reportEntireSubtree(node->getChild(i));
            }
            else if (range > u - ((innernode_t*)node)->getMaxDist(i)) {
              collectResultsNtree5(node->getChild(i));
            }
          }
        }
      }
      stat.noInnerNodes++;
    }
  }
  
  /*
  function ~prune2~, used for mdistScanN7.
  
  */
  void prune2(node_t* node, bool cand[], bool cand2[], int noCands, int c_i, 
              double u, bool reported[]) {
//     cout << "  Prune2, " << (node->isLeaf() ? "LEAF" : "INNER NODE") 
//          << ", cand at start: ";
//     for (int j = 0; j < noCands; j++) {
//       cout << cand[j] << " ";
//     }
//     cout << ", cand2 at start: ";
//     for (int j = 0; j < noCands; j++) {
//       cout << cand2[j] << " ";
//     }
//     cout << endl;
    double d_ij, maxDist_j;
    if (node->isLeaf()) {
      int noDistFunCallsBefore = dc.getNoDistFunCalls();
      for (int j = 0; j < noCands; j++) {
        if (cand[j]) {
          d_ij = node->getPrecomputedDist(c_i, j, true);
          if (range > u + d_ij) {
            addResult(((leafnode_t*)node)->getObject(j));
            cand[j] = false;
          }
          else if (range < abs(u - d_ij)) {
            cand[j] = false;
          }
        }
      }
      int noDistFunCallsAfter = dc.getNoDistFunCalls();
      stat.noDCLeaves+= noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noLeaves++;
    }
    else { // inner node
      for (int j = 0; j < noCands; j++) {
        if (cand[j]) {
          d_ij = node->getPrecomputedDist(c_i, j, false);
          maxDist_j = ((innernode_t*)node)->getMaxDist(j);
          if (range > u + d_ij + maxDist_j) {
            reportEntireSubtree(node->getChild(j));
            reported[j] = true;
//             cout << "    report ENTIRE child because " << range << " > "
//                  << u << " + " << d_ij << endl;
            cand[j] = false;
          }
          else if (range < abs(u - d_ij) - maxDist_j) {
            cand[j] = false;
          }
        }
      }
      stat.noInnerNodes++;
    }
//     cout << "    ===> cand at end: ";
//     for (int j = 0; j < noCands; j++) {
//       cout << cand[j];
//     }
//     cout << ", cand2 at end: ";
//     for (int j = 0; j < noCands; j++) {
//       cout << cand2[j] << " ";
//     }
//     cout << endl;
  }
  
  void rangeSearch2(node_t* node) { // used by collectResultsNtree7
    int noCands = (node->isLeaf() ? node->getNoEntries() : node->getDegree());
//     cout << "START rangeSearch2, " << (node->isLeaf() ? "LEAF": "INNER NODE")
//          << ",  size " << noCands << endl;
    bool cand[noCands], cand2[noCands], fullyReported[noCands]; 
    // cand <=> C, cand2 <=> C'
    std::fill_n(cand, noCands, true);
    std::fill_n(cand2, noCands, false);
    std::fill_n(fullyReported, noCands, false);
    double us[noCands];
    std::fill_n(us, noCands, DBL_MAX);
    double maxDist_i;
    double d_min = DBL_MAX;
    for (int i = 0; i < noCands; i++) {
      if (cand[i]) {
        if (node->isLeaf()) {
          us[i] = dc(queryObject, *((leafnode_t*)node)->getObject(i));
        }
        else { // inner node
          us[i] = dc(queryObject, *((innernode_t*)node)->getCenter(i));
        }
        if (us[i] < d_min) {
  //         nn_q = i;
          d_min = us[i];
        }
        prune2(node, cand, cand2, noCands, i, us[i], fullyReported);
        if (node->isLeaf()) {
//           cout << "  RS2, LEAF, i = " << i << ", u = " << us[i] << ", cand ="
//                << (cand[i] ? " TRUE" : " FALSE") << endl;
          if (cand[i] && range > us[i]) {
            addResult(((leafnode_t*)node)->getObject(i));
            cand[i] = false;
          }
        }
        else { // inner node
          maxDist_i = ((innernode_t*)node)->getMaxDist(i);
          if (cand[i] && range > us[i] + maxDist_i) {
//             cout << "  report ENTIRE child because " << range << " > "
//                  << us[i] << " + " << maxDist_i << endl;
            reportEntireSubtree(node->getChild(i));
          }
          else if (!fullyReported[i] && range > us[i] - maxDist_i) {
            cand2[i] = true;
          }
//           cout << "  RS2, INNER NODE: i = " << i << ", u = " << us[i] 
//                << ", maxDist_i = " << maxDist_i << endl;
        }
        cand[i] = false;
      }
    }
    if (!node->isLeaf()) {
      for (int i = 0; i < noCands; i++) {
        if (cand2[i]) {
//           cout << "  RS2: cand2[" << i << "] = " << cand2[i] << ", us[" << i
//                << "] = " << us[i] << endl;
        }
        if (cand2[i] && us[i] <= d_min + 2 * range) {
          rangeSearch2(node->getChild(i));
        }
      }
    }
//     cout << "END of RangeSearch2" << endl;
  }
  
  // rangeCenters2(C, q, r)
  int rangeCenters2(node_t* node, bool cand[], bool search[]) { 
    double d_min, d_iq, maxDist_i;
    int c_q;
    if (node->isLeaf()) {
      c_q = ((leafnode_t*)node)->getNearestCenterPos(queryObject, dc, d_min);
      for (int i = 0; i < node->getCount(); i++) {
        d_iq = node->getPrecomputedDist(c_q, i, true);
//         cout << "  LEAF: d_iq = " << d_iq << ", d_min = " << d_min 
//              << ", dc = "
//              << dc(((leafnode_t*)node)->getObject(i), queryObject) << endl;
        if (d_iq + d_min <= range) {
          addResult(((leafnode_t*)node)->getObject(i));
        }
        else if (d_iq - d_min <= range) {
          auto object_i = ((leafnode_t*)node)->getObject(i);
          if (dc(object_i, queryObject) <= range) {
            addResult(object_i);
          }
        }
        cand[i] = false;
      }
    }
    else { // inner node
      c_q = ((innernode_t*)node)->getNearestCenterPos(queryObject, dc, d_min);
      for (int i = 0; i < node->getDegree(); i++) {
        if (i != c_q) {
//           cout << "getPrecomputedDist(" << c_q << ", " << i << ")";
          d_iq = node->getPrecomputedDist(c_q, i, false);
//           cout << "  ..... ok, it's " << d_iq << endl;
          maxDist_i = ((innernode_t*)node)->getMaxDist(i);
//           cout << "  INNER NODE: d_iq = " << d_iq << ", d_min = " << d_min 
//                << ", maxDist_i = " << maxDist_i << ",  dc = " 
//                << dc(*((innernode_t*)node)->getCenter(i), queryObject)
//                << endl;
          if (d_iq + d_min + maxDist_i <= range) {
            reportEntireSubtree(node->getChild(i));
          }
          else if (d_iq - d_min - maxDist_i <= range &&
                   d_iq <= 2 * d_min + 2 * range) {
            if (d_iq <= 2 * range) {
              search[i] = true;
//               rangeSearch2(node->getChild(i));
            }
            else if (dc(*((innernode_t*)node)->getCenter(i), queryObject) <= 
                     d_min + 2 * range) {
              search[i] = true;
//               rangeSearch2(node->getChild(i));
            }
          }
        }
        cand[i] = false;
      }
    }
    return c_q;
  }
  
  // rangeSearch(q, r, p)
  void collectResultsNtree7(node_t* node) {
    int noCands = (node->isLeaf() ? node->getNoEntries() : node->getDegree());
    bool cand[noCands], search[noCands]; // cand <=> C
    std::fill_n(cand, noCands, true);
    std::fill_n(search, noCands, false);
    int nn_q = rangeCenters2(node, cand, search);
    if (!node->isLeaf()) {
      collectResultsNtree7(node->getChild(nn_q));
      for (int i = 0; i < noCands; i++) {
        if (search[i]) {
          rangeSearch2(node->getChild(i));
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
//     cout << "[" << results[pos - 1] << "] ";
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
  NTreeStat stat; // statistics
};

/*
4 class NTree

This is the main class of this file. It implements a main memory-based N-tree,
N-tree2, N-tree5, and N-tree6 (controlled by ~variant~).

*/
template<class T, class DistComp, int variant>
class NTree {
 public:
  typedef NTreeLeafNode<T, DistComp, variant> leafnode_t;
  typedef RangeIteratorN<T, DistComp, variant> rangeiterator_t;
//   typedef NNIterator<T, DistComp, variant> nniterator_t;
  typedef NTreeNode<T, DistComp, variant> node_t;
  typedef NTree<T, DistComp, variant> ntree_t;
  typedef NTreeInnerNode<T, DistComp, variant> innernode_t;
  
  NTree(const int d, const int mls, DistComp& di, int ps) :
      degree(d), maxLeafSize(mls), root(0), dc(di), partitionStrategy(ps),
      candOrder(RANDOM), pMethod(SIMPLE) {
    if (variant > 2) {
      candOrder = PIVOT2;
      pMethod = MINDIST;
    }
  }
      
  NTree(const int d, const int mls, const CandOrder c, const PruningMethod pm,
        DistComp& di, int ps) :
      degree(d), maxLeafSize(mls), root(0), dc(di), partitionStrategy(ps),
      candOrder(c), pMethod(pm) {
  }
  
  ~NTree() {
    if (root) {
      delete root;
    }
  }
  
  std::string getTypeName() const {
    return "ntree" + (variant > 1 ? std::to_string(variant) : "");
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
  
  void build(std::vector<T>& contents) {
    if (root) {
      delete root;
    }
    if (contents.size() <= (unsigned int)maxLeafSize) {
      root = new leafnode_t(degree, maxLeafSize, dc, 0, contents);
    }
    else {
      root = new innernode_t(degree, maxLeafSize, candOrder, pMethod);
      root->build(contents, dc, -1, partitionStrategy);
    }
//     print(cout);
    computeStatistics(root);
    cout << endl;
    stat.print(cout);
  }
  
  std::ostream& print(std::ostream& out) {
    out << "{" << getTypeName() << ", degree = " << degree << ", maxLeafSize = "
        << maxLeafSize << "}" << endl;
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
    ntree_t* res = new ntree_t(degree, maxLeafSize, dc, partitionStrategy);
    if (root) {
      res->root = root->clone();
    }
    return res;
  }
  
  rangeiterator_t* rangeSearch(const T& q, const double range) const {
    return new rangeiterator_t(root, q, range, dc);
  }
  
  void computeStatistics(node_t* ptr) {
    if (ptr->isLeaf()) {
      stat.noLeaves++;
      stat.noDCLeaves += ((leafnode_t*)ptr)->noDistComp;
    }
    else {
      stat.noInnerNodes++;
      stat.noDCInnerNodes += ((innernode_t*)ptr)->noDistComp;
      for (int i = 0; i < degree; i++) {
        computeStatistics(ptr->getChild(i));
      }
    }
  }
  
  
 protected:
  int degree;
  int maxLeafSize;
  node_t* root;
  DistComp dc;
  int partitionStrategy;
  CandOrder candOrder; // random, two ref points, three ref points
  PruningMethod pMethod; // simple, minDist-based
  NTreeStat stat; // statistics
  
  static node_t* insert(node_t* root, const T& o, DistComp& dc,
                        const int partitionStrategy = 0) {
    root->insert(o, dc, partitionStrategy);
    return root;
  }
};

#endif
