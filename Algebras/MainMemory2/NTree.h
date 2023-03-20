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
#include "../FText/FTextAlgebra.h"
#include "Mem.h"
#include "MainMemoryExt.h"
#include "../Picture/hist_hsv.h"

namespace mtreehelper{

  double distance(const Point* p1, const Point* p2, Geoid* geoid) {
     if(!p1->IsDefined() && !p2->IsDefined()){
       return 0;
     }
     if(!p1->IsDefined() || !p2->IsDefined()){
         return std::numeric_limits<double>::max();
     } 
     return p1->Distance(*p2, geoid);
  }

  double distance(const CcString* s1, const CcString* s2, Geoid* geoid) {
     if(!s1->IsDefined() && !s2->IsDefined()){
        return 0;
     }
     if(!s1->IsDefined() || !s2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return stringutils::ld(s1->GetValue() ,s2->GetValue());
  }

  double distance(const CcInt* i1, const CcInt* i2, Geoid* geoid) {
     if(!i1->IsDefined() && !i2->IsDefined()){
        return 0;
     }
     if(!i1->IsDefined() || !i2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return abs(i1->GetValue() - i2->GetValue());
  }
  
  double distance(const CcReal* r1, const CcReal* r2, Geoid* geoid) {
     if(!r1->IsDefined() && !r2->IsDefined()){
        return 0;
     }
     if(!r1->IsDefined() || !r2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return abs(r1->GetValue() - r2->GetValue());
  }

  template<unsigned int dim>
  double distance(const Rectangle<dim>* r1, 
                  const Rectangle<dim>* r2, Geoid* geoid) {
     if(!r1->IsDefined() && !r2->IsDefined()){
        return 0;
     }
     if(!r1->IsDefined() || !r2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return r1->Distance(*r2);
  }

  double distance(const temporalalgebra::MPoint* mp1,
                  const temporalalgebra::MPoint* mp2, Geoid* geoid) {
    if (!mp1->IsDefined() && !mp2->IsDefined()) {
      return 0;
    }
    if (!mp1->IsDefined() || !mp2->IsDefined()) {
      return std::numeric_limits<double>::max();
    }
    datetime::DateTime duration(0, 3600000, datetime::durationtype);
    return temporalalgebra::DistanceComputation<temporalalgebra::MPoint, 
       temporalalgebra::UPoint>::DistanceAvg(*mp1, *mp2, duration, true, geoid);
  }
  
  double distance(const temporalalgebra::CUPoint* cup1,
                  const temporalalgebra::CUPoint* cup2, Geoid* geoid) {
    if (!cup1->IsDefined() && !cup2->IsDefined()) {
      return 0;
    }
    if (!cup1->IsDefined() || !cup2->IsDefined()) {
      return std::numeric_limits<double>::max();
    }
    datetime::DateTime duration(0, 3600000, datetime::durationtype);
    return cup1->DistanceAvg(*cup2, duration, true, geoid); // upper bound dist
  }

  double distance(const temporalalgebra::CMPoint* cmp1,
                  const temporalalgebra::CMPoint* cmp2, Geoid* geoid) {
    if (!cmp1->IsDefined() && !cmp2->IsDefined()) {
      return 0;
    }
    if (!cmp1->IsDefined() || !cmp2->IsDefined()) {
      return std::numeric_limits<double>::max();
    }
    datetime::DateTime duration(0, 3600000, datetime::durationtype);
    return temporalalgebra::DistanceComputation<temporalalgebra::CMPoint, 
                                         temporalalgebra::CUPoint>::DistanceAvg(
                                           *cmp1, *cmp2, duration, true, geoid);
  }
  
  template<unsigned int dim, bool lab>
  double distance(const hist_hsv<dim, lab>* h1, const hist_hsv<dim, lab>* h2,
                  Geoid* geoid) {
    if (!h1->IsDefined() && !h2->IsDefined()) {
      return 0;
    }
    if (!h1->IsDefined() || !h2->IsDefined()) {
      return std::numeric_limits<double>::max();
    }
    bool def;
    double result;
    h1->distance(*h2, def, result);
    return result;
  }
  
  template<class T>
  double distance(const T* o1, const T* o2, double alpha, Geoid* geoid) {
//     cout << "  CALL dist for " << o1->first << " and " << o2->first << endl;
//     auto t1 = chrono::high_resolution_clock::now();
//     double spaDist = o1->first.DistanceAvg(o2->first, geoid);
    double spaDist = distance(&(o1->first), &(o2->first), geoid);
//     auto t2 = chrono::high_resolution_clock::now();
//     auto duration = 
//           chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
//     cout << "  spatial distance: " << spaDist << " after "
//          << (double)duration / 1000.0 << " ms" << endl;
//     cout << "  num of spatial units: (" << o1->first.GetNoComponents()
//          << ", " << o2->first.GetNoComponents() 
//          << "), num of symbolic units: ("
//          << o2->second.GetNoComponents() << ", " 
//          << o2->second.GetNoComponents() << ")" << endl;
    std::set<std::string> allLabels1, allLabels2;
    o1->second.InsertLabels(allLabels1);
    o2->second.InsertLabels(allLabels2);
    double symDist = 1.0 - stj::jaccardSimilarity(allLabels1, allLabels2);
//     auto t3 = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(t3 - t2).count();
//     cout << "  symbolic jaccard sim:   " << symSim << " after "
//          << (double)duration / 1000.0 << " ms" << endl;
//     double symDist = o1->second.Distance_ALL(o2->second, stj::TRIVIAL);
//     auto t4 = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(t4 - t3).count();
//     cout << "  symbolic edit distance: " << symDist << " after " 
//          << (double)duration / 1000.0 << " ms" << endl;
    return spaDist * alpha + symDist * (1.0 - alpha);
  }

/*
   6.4 ~getTypeNo~

   Returns a number for supported types, -1 if not supported.

*/
  typedef Point t1;
  typedef CcString t2;
  typedef CcInt t3;
  typedef CcReal t4;
  typedef Rectangle<1> t5;
  typedef Rectangle<2> t6;
  typedef Rectangle<3> t7;
  typedef Rectangle<4> t8;
  typedef Rectangle<8> t9;
  typedef temporalalgebra::MPoint t10;
  typedef temporalalgebra::CUPoint t11;
  typedef temporalalgebra::CMPoint t12;
  typedef hist_hsv<64, false> t13;
  typedef hist_hsv<128, false> t14;
  typedef hist_hsv<256, false> t15;
  typedef hist_hsv<256, true> t16;

  int getTypeNo(ListExpr type, int expectedNumbers){
     assert(expectedNumbers==16);
     if(nl->ToString(type) == Tuple::BasicType()){return 16;}
     if( t1::checkType(type)){ return 0;}
     if( t2::checkType(type)){ return 1;}
     if( t3::checkType(type) ){ return 2;}
     if( t4::checkType(type)){ return 3; }
     if( t5::checkType(type)){ return 4;}
     if( t6::checkType(type)){ return 5;}
     if( t7::checkType(type)){ return 6;}
     if( t8::checkType(type)){ return 7;}
     if( t9::checkType(type)){ return 8;}
     if(t10::checkType(type)){ return 9;}
     if(t11::checkType(type)){ return 10;}
     if(t12::checkType(type)){ return 11;}
     if(t13::checkType(type)){ return 12;}
     if(t14::checkType(type)){ return 13;}
     if(t15::checkType(type)){ return 14;}
     if(t16::checkType(type)){ return 15;}
     return -1;
  }

  std::string BasicType(){
    return "mtree";
  }

  bool checkType(ListExpr type, ListExpr subtype, std::string basicType) {
    if (!nl->HasLength(type,2)) {
      return false;
    }
    if (!listutils::isSymbol(nl->First(type), basicType)) {
      return false;
    }
    if(getTypeNo(subtype, 16) < 0) {
      return false;
    }
    return nl->Equal(nl->Second(type), subtype);
  }
  
  bool checkType(ListExpr type, ListExpr subtype) {
    return checkType(type, subtype, BasicType());
  }
  
  bool checkTypeN(ListExpr type, ListExpr subtype, int variant) {
    std::string ntreetype = "ntree" +
                                   (variant > 1 ? std::to_string(variant) : "");
    return checkType(type, subtype, ntreetype);
  }

  void increaseCounter(const std::string& objName, const int numberToBeAdded) {
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    Word counterW;
    bool defined = false;
    bool exists = sc->GetObject(objName, counterW, defined);
    ListExpr currentType = sc->GetObjectTypeExpr(objName);
    if (!CcInt::checkType(currentType) || !defined) {
      sc->DeleteObject(objName);
    }
    if (!exists || !CcInt::checkType(currentType) || !defined) {
      counterW.addr = new CcInt(true, numberToBeAdded);
      sc->InsertObject(objName, CcInt::BasicType(),
                       nl->SymbolAtom(CcInt::BasicType()), counterW, true);
    }
    else {
      int counter = ((CcInt*)counterW.addr)->GetValue() + numberToBeAdded;
      ((CcInt*)counterW.addr)->Set(true, counter);
      sc->ModifyObject(objName, counterW);
    }
  }

}

namespace mm2algebra {

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
enum PartitionMethod {FIRSTD, RANDOMONLY, RANDOMOPT};

template<class T, class DistComp, int variant>
class NTreeInnerNode;

template<class T, class DistComp, int variant>
class NTreeLeafNode;

/*
7 Auxiliary Class ~hash\_pair~

*/
struct hash_pair { 
  template <class T1, class T2> 
  size_t operator()(const std::pair<T1, T2>& p) const { 
    auto hash1 = std::hash<T1>{}(p.first); 
    auto hash2 = std::hash<T2>{}(p.second); 
    return hash1 ^ hash2; 
  } 
};

/*
7 Class ~DistStorage~

*/
class DistStorage {
 public: 
  DistStorage() : noDistFunCalls(0) {}
  
  ~DistStorage() {
    clear();
  }
    
  double retrieveDist(const TupleId& tid1, const TupleId& tid2) const {
    assert(tid1 <= tid2);
    auto it = storedDists.find(std::make_pair(tid1, tid2));
    if (it == storedDists.end()) { // not found
//       cout << "NOT FOUND dist(" << tid1 << ", " << tid2 << ")" << endl;
      return -1.0;
    }
//     cout << "FOUND dist(" << tid1 << ", " << tid2 << ") = " << it->second 
//          << endl;
    return it->second;
  }
  
  void storeDist(const TupleId& tid1, const TupleId& tid2, const double dist) {
    storedDists[std::make_pair(tid1, tid2)] = dist;
//     cout << "  COMPUTED and STORED dist(" << tid1 << ", " << tid2 << ") = " 
//          << dist << "  ... # " << storedDists.size() << endl;
  }
  
  size_t size() const {
    return storedDists.size();
  }
  
  void clear() {
    storedDists.clear();
    noDistFunCalls = 0;
  }
  
  void increment() {
    noDistFunCalls++;
  }
  
  int getNoDistFunCalls() const {
    return noDistFunCalls;
  }
  
 private:
  int noDistFunCalls;
  std::unordered_map<std::pair<TupleId,TupleId>, double, hash_pair> storedDists;
};

/*
8 Class ~StdDistComp~

*/
template<class T>
class StdDistComp{
  public:
    StdDistComp(Geoid* _geoid){
      geoid = _geoid?(Geoid*)_geoid->Clone():0; 
    }

    StdDistComp(const StdDistComp& src){
       geoid = src.geoid?(Geoid*)src.geoid->Copy():0;
    }

    StdDistComp& operator=(const StdDistComp&src){
       if(geoid) geoid->DeleteIfAllowed();
       geoid = src.geoid?(Geoid*)src.geoid->Copy():0;
    }

    ~StdDistComp(){
       if(geoid){
         geoid->DeleteIfAllowed();
       }
    }


    double operator()(const MTreeEntry<T>& o1, const MTreeEntry<T>& o2) {
      TupleId tid1 = std::min(o1.getTid(), o2.getTid());
      TupleId tid2 = std::max(o1.getTid(), o2.getTid());
      double dist = distStorage.retrieveDist(tid1, tid2);
      if (dist < 0.0) {
        const T* t1 = o1.getKey();
        const T* t2 = o2.getKey();
        dist = mtreehelper::distance(t1, t2, geoid);
        distStorage.increment();
        if (getNoDistFunCalls() % 10000 == 0) {
          cout << "|";
          std::cout.flush();
        }
        else if (getNoDistFunCalls() % 1000 == 0) {
          std::cerr << ".";
          std::cout.flush();
        }
        distStorage.storeDist(tid1, tid2, dist);
      }
      return dist;
    }
//     double  operator()(const pair<T,TupleId>& o1, 
//                        const pair<T,TupleId>& o2){
//        return mtreehelper::distance(&o1.first,&o2.first, geoid);
//     }

    std::ostream& print( const MTreeEntry<T>& e, std::ostream& o){
       o << "("; e.getKey()->Print(o); o << ", " << e.getTid() << ")";
       return o;
    }
    
    std::ostream& print(const MTreeEntry<T>& e, const bool printContents,
                        std::ostream& o) {
      if (printContents) {
        return print(e, o);
      }
      o << e.getTid();
      return o;
    }
    
    Geoid* getGeoid() const {
      return geoid;
    }
  
    void reset(){} // not sureA
    
    size_t distStorageSize() const {
      return distStorage.size();
    }
    
    int getNoDistFunCalls() const {
      return distStorage.getNoDistFunCalls();
    }

  protected:
    Geoid* geoid;
    DistStorage distStorage;
};

/*
Class ~StdDistCompExt~

*/
template<class T, class U>
class StdDistCompExt : public StdDistComp<std::pair<T, U> > {
 public:
  StdDistCompExt(Geoid* _geoid, double _alpha) :
    StdDistComp<std::pair<T, U> >(_geoid), alpha(_alpha) {}
  
  StdDistCompExt(const StdDistCompExt& src) :
    StdDistComp<std::pair<T, U> >(src), alpha(src.alpha) {}
  
  StdDistCompExt& operator=(const StdDistCompExt& src) {
    if (this->geoid) {
      this->geoid->DeleteIfAllowed();
    }
    ((StdDistComp<std::pair<T, U> >*)this)->geoid = src.geoid ? 
                                                  (Geoid*)src.geoid->Copy() : 0;
    alpha = src.alpha;
  }
  
  ~StdDistCompExt() {}
    
  
  std::ostream& print(const MTreeEntry<std::pair<T, U> >& p, std::ostream& o) {
       o << "< <";
       p.getKey()->first.Print(o);
       o << ", ";
       p.getKey()->second.Print(o);
       o << ">, " << p.getTid() << ">";
       return o;
    }
    
  double operator()(const MTreeEntry<std::pair<T, U> >& o1,
                    const MTreeEntry<std::pair<T, U> >& o2) {
    TupleId tid1 = std::min(o1.getTid(), o2.getTid());
    TupleId tid2 = std::max(o1.getTid(), o2.getTid());
    double dist = this->distStorage.retrieveDist(tid1, tid2);
    if (dist < 0.0) {
      const std::pair<T, U>* p1 = o1.getKey();
      const std::pair<T, U>* p2 = o2.getKey();
      dist = mtreehelper::distance(p1, p2, alpha,
                            ((StdDistComp<std::pair<T, U> >*)this)->getGeoid());
      StdDistComp<std::pair<T, U> >::distStorage.increment();
      this->distStorage.storeDist(tid1, tid2, dist);
    }
    return dist;
  }
  
 private:
  double alpha; 
};

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
  
  int getNodeId() const {
    return nodeId;
  }
  
  void setNodeId(const int id) {
    nodeId = id;
  }
  
  int getCount() const {
    return count;
  }
  
  CandOrder getCandOrder() const {
    return candOrder;
  }
  
  PruningMethod getPruningMethod() const {
    return pMethod;
  }
  
  std::tuple<int, int, int> getRefDistPos() const {
    return refDistPos;
  }
  
  virtual T* getObject(const int pos) = 0;
  
  virtual double getMinDist(const T& q, DistComp& dc) const = 0;

  virtual node_t* clone() = 0;
  
  virtual void build(std::vector<T>& contents, DistComp& dc, int depth,
                     const PartitionMethod partMethod) = 0;
  
  virtual void insert(const T& entry, DistComp& dc,
                      const PartitionMethod partMethod) = 0;
  
  virtual void remove(const T& entry, DistComp& dc,
                      const PartitionMethod partMethod) = 0;
  
  virtual void clear(const bool deleteContent) = 0;
  
  virtual std::ostream& print(std::ostream& out, DistComp& dc) const = 0;
  
  virtual std::ostream& print(std::ostream& out, DistComp& di, 
                           const bool printSubtrees, const bool printDistMatrix,
                           const bool printPivotInfo) const = 0;
  
  virtual double evaluateDist(const int i, const T& o, DistComp& dc) const = 0;
  
  virtual void setCenters(const int nodeId, const std::vector<int>& pos, 
        const std::vector<T*>& objects, const std::vector<double>& maxDist) = 0;
  
  void setDistMatrix(double **dm) {
    distMatrix = dm;
  }
  
  void setPivotInfo(std::vector<int>& rdp, std::pair<double, double>* d2d) {
    if (rdp.size() == 1) {
      std::get<1>(refDistPos) = rdp[0];
    }
    else {
      assert(rdp.size() == 2);
      std::get<1>(refDistPos) = rdp[1];
    }
    std::get<0>(refDistPos) = rdp[0];
    std::get<2>(refDistPos) = -1;
    distances2d = d2d;
  }
        
  void initAuxStructures(const int size) {
    if (distMatrix != 0 || distances2d != 0 || distances3d != 0) {
      return;
    }
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
    distMatrix = 0;
    distances2d = 0;
    distances3d = 0;
  }
  
  int getNearestCenterPos(const T& o, DistComp& dc, const int size,
                          std::tuple<double, double, double>& odist3d, 
                          const bool isLeaf, double& minDist) {
//     int size = isLeaf ? ((leafnode_t*)this)->getNoEntries() : degree;
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
    
    if (candOrder == PIVOT2 || candOrder == PIVOT3) {
      std::sort(W, W + size);
    }
//     for (int i = 0; i < size; i++) {
//       cout << "(" << W[i].pos << ", " << W[i].distEuclid << "),  ";
//     }
//     cout << endl;
    double Dq[size];
    std::fill_n(Dq, size, DBL_MAX);
    for (int i = 0; i < size; i++) {
      if (cand[W[i].pos]) {
        tempDist = isLeaf ? ((leafnode_t*)this)->evaluateDist(W[i].pos, o, dc) :
                            ((innernode_t*)this)->evaluateDist(W[i].pos, o, dc);
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
    
  void precomputeDistances(DistComp& dc, const int size, const bool isLeaf) {
    double maxDist = 0.0;
    refDistPos = std::make_tuple(0, 0, 0);
//     int size = isLeaf ? ((leafnode_t*)this)->getNoEntries() : degree;
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
    if (isLeaf) {
      double leafMaxDist = 0.0;
      for (int i = 0; i < size; i++) {
        if (distMatrix[0][i] > leafMaxDist) {
          leafMaxDist = distMatrix[0][i];
        }
      }
      ((leafnode_t*)this)->setMaxDist(leafMaxDist);
    }
  }
  
  std::vector<double> getPivotDistances(const int pos) const {
    std::vector<double> result;
    switch (candOrder) {
      case PIVOT2: {
        result.push_back(distances2d[pos].first);
        result.push_back(distances2d[pos].second);
        break;
      }
      case PIVOT3: {
        result.push_back(std::get<0>(distances3d[pos]));
        result.push_back(std::get<1>(distances3d[pos]));
        result.push_back(std::get<2>(distances3d[pos]));
        break;
      }
      case RANDOM: {
        break;
      }
      default: {
        assert(false);
      }
    }
    return result;
  }
  
  double getPrecomputedDist(const int pos1, const int pos2, const bool isLeaf)
                                                                         const {
    int size = isLeaf ? ((leafnode_t*)this)->getNoEntries() : degree;
    assert(pos1 >= 0 && pos1 < size && pos2 >= 0 && pos2 < size);
    return distMatrix[pos1][pos2];
  }
  
  
 protected:
  NTreeNode(const int d, const int mls) : node_t(d, mls, RANDOM, SIMPLE) {}
    
  NTreeNode(const int d, const int mls, const CandOrder co, 
            const PruningMethod pm) : degree(d), maxLeafSize(mls), count(0), 
         noDistComp(0), distMatrix(0), distances2d(0), distances3d(0), 
         candOrder(co), pMethod(pm) {}
   
  int nodeId, degree, maxLeafSize, count, noDistComp; 
  
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

template<class T>
struct PartitionStatus {
  PartitionStatus(std::vector<std::vector<T> >& partitions, const int degree,
                  const int noElems) {
    min = INT_MAX;
    max = -1;
    meanError = 0.0;
    meanSquaredError = 0.0;
    double optimalSize = (double)(noElems) / partitions.size();
    for (int i = 0; i < degree; i++) {
      if ((int)partitions[i].size() < min) {
        minPos = i;
        min = partitions[i].size();
      }
      if ((int)partitions[i].size() > max) {
        maxPos = i;
        max = partitions[i].size();
      }
      meanError += std::abs(optimalSize - partitions[i].size());
      meanSquaredError += std::pow(optimalSize - partitions[i].size(), 2);
    }
    minMaxFactor = (double)max / min;
    meanError /= partitions.size();
    meanSquaredError /= partitions.size();
  }
  
  std::set<int> getMaxCenters(std::vector<std::vector<T> >& partitions,
                              const int degree) {
    std::set<std::pair<int, int> > partSizes;
    std::set<int> result;
    for (unsigned int i = 0; i < partitions.size(); i++) {
      std::pair<int, int> partSize((int)partitions[i].size(), (int)i);
      partSizes.insert(partSize);
    }
    auto it = partSizes.end();
    for (int i = 0; i < degree; i++) {
      it--;
    }
    partSizes.erase(partSizes.begin(), it);
    it = partSizes.begin();
    for (int i = 0; i < degree; i++) {
      result.insert(it->second);
      it++;
    }
    return result;
  }
  
  void print() {
    cout << "min: (" << minPos << ", " << min << "), max: (" << maxPos << ", "
         << max << "), Factor = " << minMaxFactor << ", ME = " << meanError 
         << ", MSE = " << meanSquaredError << endl;
  }
  
  int min, max, minPos, maxPos;
  double minMaxFactor, meanError, meanSquaredError;
};

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
    distMatrix = 0;
    distances2d = 0;
    distances3d = 0;
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
      if (children[i] != 0) {
        res += children[i]->memSize();
      }
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
  
  void getContents(std::vector<T>& result) {
    int size;
    for (int i = 0; i < node_t::count; i++) {
      if (children[i]->isLeaf()) {
        size = children[i]->getNoEntries();
        for (int j = 0; j < size; j++) {
          result.push_back(children[i]->getObject(j));
        }
      }
      else {
        ((innernode_t*)children[i])->getContents(result);
      }
    }
  }
  
  int getNearestCenterPos(const T& o, DistComp& dc, const int size,
                          double& minDist) {
    if (variant == 2 || variant >= 6) {
      std::tuple<double, double, double> odist3d;
      if (candOrder == PIVOT2) {
        odist3d = std::make_tuple(dc(o, *(centers[std::get<0>(refDistPos)])),
                                  dc(o, *(centers[std::get<1>(refDistPos)])),
                                  0.0);
      }
      if (candOrder == PIVOT3) {
        std::get<2>(odist3d) = dc(o, *(centers[std::get<2>(refDistPos)]));
      }
      return node_t::getNearestCenterPos(o, dc, size, odist3d, false, minDist);
    }
    else { // N-tree or N-tree5
      double currentDist = std::numeric_limits<double>::max();
      int result = -1;
      for (int i = 0; i < size; i++) {
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
  
  void setCenters(const int nodeId, const std::vector<int>& pos, 
           const std::vector<T*>& objects, const std::vector<double>& maxDist) {
    node_t::setNodeId(nodeId);
    assert(pos.size() == objects.size() && maxDist.size() == pos.size());
    for (unsigned int i = 0; i < pos.size(); i++) {
      centers[pos[i]] = objects[i];
      this->maxDist[pos[i]] = maxDist[i];
      node_t::count++;
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
    assert(i >= 0);
    return dc(*(centers[i]), o);
  }
  
  double evaluateDist(const int i, const int j, DistComp& dc) const {
    assert(i >= 0 && j >= 0);
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
    return print(out, dc, true, true, true);
  }
   
  std::ostream& print(std::ostream& out, DistComp& dc, const bool printSubtrees,
                  const bool printDistMatrix, const bool printPivotInfo) const {
    out << "( \"inner node: ";
    out << " (";
    for (int i = 0; i < node_t::count; i++) {
      out << "center #" << i << " = (";
      centers[i]->getKey()->Print(out);
      out << ", TID " << centers[i]->getTid() << ")";
      if (printSubtrees) {
        out << ", child #" << i << " with node id " << node_t::nodeId << " = ";
        if (children[i] == 0) {
          out << "NULL ";
        }
        else {
          children[i]->print(out, dc);
        }
      }
    }
    out << " )" << endl;
    if (maxDist != 0) {
      out << "maxDist: <";
      for (int i = 0; i < node_t::count; i++) {
        out << maxDist[i] << (i == node_t::count - 1 ? "" : ", ");
      }
      out << ">" << endl;
    }
    if (printDistMatrix && distMatrix != 0) {
      out << "distMatrix:" << endl;
      for (int i = 0; i < node_t::count; i++) {
        for (int j = 0; j <= i; j++) {
          out << distMatrix[i][j] << " ";
        }
        out << endl;
      }
    }
    if (printPivotInfo && (distances2d != 0 || distances3d != 0)) {
      out << "pivot elements: " << std::get<0>(refDistPos) << ", "
          << std::get<1>(refDistPos);
      if (candOrder == PIVOT3) {
        out << ", " << std::get<2>(refDistPos);
      }
      out << endl;
      out << "pivot distances: ";
      for (int i = 0; i < node_t::count; i++) {
        if (candOrder == PIVOT3) {
          out << "(" << std::get<0>(distances3d[i]) << ", "
              << std::get<1>(distances3d[i]) << ", " 
              << std::get<2>(distances3d[i]) << "), ";
        }
        else if (candOrder == PIVOT2) {
          out << "(" << distances2d[i].first << ", " << distances2d[i].second
              << "), ";
        }
      }
      out << endl;
    }
    out << ")";
    return out;
  }

  int getNoLeaves() const {
    int sum = 0;
    for (int i = 0; i < node_t::count; i++) {
      if (children[i] != 0) {
        sum += children[i]->getNoLeaves();
      }
    }
    return sum;
  }
  
  int getNoEntries() const {
    int sum = 0;
    for (int i = 0; i < node_t::count; i++) {
      if (children[i] != 0) {
        sum += children[i]->getNoEntries();
      }
    }
    return sum;
  }

  int getNoNodes() const {
    int sum = 1;
    for (int i = 0; i < node_t::count; i++) {
      if (children[i] != 0) {
        sum += children[i]->getNoNodes();
      }
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
  
  void computeRandomCenters(std::vector<T>& contents) {
    std::vector<int> positions(contents.size());
    for (unsigned int i = 0; i < contents.size(); i++) {
      positions[i] = i;
    }
    std::shuffle(positions.begin(), positions.end(), 
                 std::mt19937(std::random_device()()));
    for (int i = 0; i < node_t::degree; i++) {
      centers[i] = new T(contents[positions[i]]);
    } // node_t::degree random positions between 0 and contents.size() - 1
  }
  
  T** computeTempRandomCenters(std::vector<T>& contents, const int noCenters) {
    T** result = new T*[noCenters];
    for (int i = 0; i < noCenters; i++) {
      result[i] = 0;
    }
    std::vector<int> positions(contents.size());
    for (unsigned int i = 0; i < contents.size(); i++) {
      positions[i] = i;
    }
    std::shuffle(positions.begin(), positions.end(), 
                 std::mt19937(std::random_device()()));
    for (int i = 0; i < noCenters; i++) {
      result[i] = new T(contents[positions[i]]);
    }
    return result;
  }
  
  void computeCenters(std::vector<T>& contents, DistComp& dc,
                      const PartitionMethod partMethod, 
                      std::vector<std::vector<T> >& partitions,
                      std::vector<T>& contentsToRepart) {
    assert(contents.size() >= (unsigned int)node_t::degree);
    switch (partMethod) {
      case FIRSTD : { // first d objects at hand
        for (int i = 0; i < node_t::degree; i++) {
          centers[i] = new T(contents[i]);
        }
        break;
      }
      case RANDOMONLY : { // random
        computeRandomCenters(contents);
        break;
      }
      case RANDOMOPT : { // random centers with optimization ("Strategy 2")
        delete[] centers;
        int m = std::min((int)floor(2.5 * node_t::degree),(int)contents.size());
//         cout << "m = " << m << " | " << contents.size() << " objects for "
//              << node_t::degree << " partitions, optimal size is " 
//              << (double)(contents.size()) / node_t::degree << endl;
        centers = computeTempRandomCenters(contents, m);
//         centers = new T*[m];
//         for (int i = 0; i < m; i++) { // deterministic start
//           centers[i] = new T(contents[i]);
//         }
        node_t::initAuxStructures(m);
        delete[] maxDist;
        maxDist = new double[m];
        std::fill_n(maxDist, m, 0.0);
        node_t::precomputeDistances(dc, m, false);
        std::vector<std::vector<T> > partitionsTemp;
        partitionsTemp.resize(m);
        partition(contents, m, dc, partitionsTemp);
        PartitionStatus<T> status(partitionsTemp, m, contents.size());
        std::set<int> maxCenters = status.getMaxCenters(partitionsTemp, 
                                                           node_t::degree);
        auto it = maxCenters.begin();
        T** newCenters = new T*[node_t::degree];
        for (int i = 0; i < node_t::degree; i++) {
          newCenters[i] = new T(*(centers[*it]));
          it++;
        }
        for (int i = 0; i < m; i++) {
          delete centers[i];
        }
        delete[] centers;
        std::vector<double> maxDistTemp(maxDist, maxDist + m);
        delete[] maxDist;
        node_t::deleteAuxStructures(m);
        maxDist = new double[node_t::degree];
        centers = newCenters;
        for (int i = 0; i < m; i++) {
          if (maxCenters.find(i) != maxCenters.end()) { // one of maxCenters
            maxDist[partitions.size()] = maxDistTemp[i];
            partitions.push_back(partitionsTemp[i]); // keep partition
          }
          else { // not in maxCenters ==> elements must be repartitioned
            for (unsigned int j = 0; j < partitionsTemp[i].size(); j++) {
              contentsToRepart.push_back(partitionsTemp[i][j]);
            }
          }
        }
        node_t::initAuxStructures(node_t::degree);
        node_t::precomputeDistances(dc, node_t::degree, false); 
        break;
      }
//       TODO:
//       case PIVOTDISTORDER : {
//         break;
//       }
      // TODO: add spatially balanced method
      default: {
        assert(false);
      }
    }
  }
  
  void partition(std::vector<T>& contents, const int noCenters,
                 DistComp& dc, std::vector<std::vector<T> >& partitions) {
    double dist(-1.0), centerDist(-1.0);
    int partitionPos = -1;
    if (variant == 2 || variant >= 6) { // NTree2 etc.
//       cout << "sizes BEFORE partition:";
//       for (int i = 0; i < noCenters; i++) {
//         cout << " (" << i << ", " << partitions[i].size() << ")";
//       }
//       cout << endl;
      for (unsigned int i = 0; i < contents.size(); i++) {
        for (int j = 0; j < noCenters; j++) {
          if (contents[i].getTid() == centers[j]->getTid()) {
            partitionPos = j;
            j = noCenters;
          }
        }
        if (partitionPos == -1) {
//           cout << "call gNCP for contents # " << i << endl;
          partitionPos = getNearestCenterPos(contents[i], dc, noCenters,
                                             centerDist);
        }
        partitions[partitionPos].push_back(contents[i]);
        if (centerDist > maxDist[partitionPos]) {
          maxDist[partitionPos] = centerDist;
        }
        partitionPos = -1;
      }
//       PartitionStatus<T> status(partitions, noCenters, contents.size());
//       status.print();
//       cout << "sizes AFTER partition:";
//       for (int i = 0; i < noCenters; i++) {
//         cout << " (" << i << ", " << partitions[i].size() << ")";
//       }
//       cout << endl;
    }
    else { // NTree or NTree5
      for (unsigned int i = 0; i < contents.size(); i++) {
        centerDist = std::numeric_limits<double>::max();
        for (int j = 0; j < noCenters; j++) {
          if (contents[i].getTid() == centers[j]->getTid()) {
            partitionPos = j;
            j = noCenters;
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
             const PartitionMethod partMethod) { 
    // precondition: contents.size > maxLeafSize
//     cout << "start BUILD, " << "depth " << depth << ", " << contents.size()
//          << " elems, counter = " << dc.getNoDistFunCalls() << endl;
    int noDistFunCallsBefore = dc.getNoDistFunCalls();
    depth++;
    std::vector<std::vector<T> > partitions;
    std::vector<T> contentsToRepart; // used for NTree8 efficiency
    computeCenters(contents, dc, partMethod, partitions, contentsToRepart);
    if (variant == 2 || variant >= 6) {
      node_t::initAuxStructures(degree);
      node_t::precomputeDistances(dc, degree, false);
    }
    if (partMethod == RANDOMOPT) {
//       partition(contents, node_t::degree, dc, partitions);
      partition(contentsToRepart, node_t::degree, dc, partitions);
    }
    else {
      partitions.clear();
      partitions.resize(node_t::degree);
      partition(contents, node_t::degree, dc, partitions);
    }
//     printPartitions(contents, partitions, depth, dc, false, cout);
    int noDistFunCallsAfter = dc.getNoDistFunCalls();
    noDistComp = noDistFunCallsAfter - noDistFunCallsBefore;
    for (int i = 0; i < degree; i++) {
      if ((int)partitions[i].size() <= maxLeafSize) {
        children[i] = new leafnode_t(degree, maxLeafSize, candOrder, pMethod,
                                     dc, centers[i]->getTid(), partitions[i]);
      }
      else {
        children[i] = ((variant == 2) || (variant >= 6) ?
             new innernode_t(degree, maxLeafSize, candOrder, pMethod) :
             new innernode_t(degree, maxLeafSize));
        children[i]->build(partitions[i], dc, depth, partMethod);
      }
    }
    node_t::count = degree;
  }
  
  void insert(const T& entry, DistComp& dc, const PartitionMethod partMethod) {
    double minDist;
    int nearestCenter = getNearestCenterPos(entry, dc, this->getDegree(),
                                            minDist);
    node_t *nearestChild = children[nearestCenter];
    if (nearestChild->isLeaf()) {
      int size = ((leafnode_t*)(nearestChild))->getNoEntries();
      if (((leafnode_t*)nearestChild)->contains(entry, dc)) { // nothing to do
        return;
      }
      if (size == node_t::maxLeafSize) { // split required
        std::vector<T> contents;
        for (int i = 0; i < size; i++) {
          contents.push_back(*(((leafnode_t*)nearestChild)->getObject(i)));
        }
        contents.push_back(entry);
        delete nearestChild;
        children[nearestCenter] = new innernode_t(node_t::degree,
                 node_t::maxLeafSize, node_t::candOrder, node_t::pMethod);
        children[nearestCenter]->build(contents, dc, 0, partMethod);
        return;
      }
    }
    nearestChild->insert(entry, dc, partMethod);
  }
  
  void remove(const T& entry, DistComp& dc, const PartitionMethod partMethod) {
    double minDist;
    int nearestCenter = getNearestCenterPos(entry, dc, this->getDegree(),
                                            minDist);
    node_t *nearestChild = children[nearestCenter];
    nearestChild->remove(entry, dc, partMethod);
    bool redistribute = false;
    if (nearestChild->isLeaf()) {
      if (((leafnode_t*)nearestChild)->isEmpty()) {
        redistribute = true;
      }
    }
    else {
      if (((innernode_t*)nearestChild)->getNoEntries() <= node_t::maxLeafSize) {
        redistribute = true;
      }
    }
    if (redistribute) {
      std::vector<T> contents;
      getContents(contents);
      node_t::deleteAuxStructures(node_t::count);
      for (int i = 0; i < node_t::count; i++) {
        delete children[i];
        delete centers[i];
      }
      build(contents, dc, 0, partMethod);
    }
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
  
  NTreeLeafNode(const int d, const int mls, const CandOrder co,
                const PruningMethod pm) : node_t(d, mls, co, pm) {
    entries = new T*[node_t::maxLeafSize];
    for (int i = 0; i < node_t::maxLeafSize; i++) {
      entries[i] = 0;
    }
  }
  
  NTreeLeafNode(const int d, const int mls) : leafnode_t(d, mls, RANDOM, SIMPLE)
    {}
  
  NTreeLeafNode(const int d, const int mls, const CandOrder co, 
                const PruningMethod pm, DistComp& dc, const TupleId centerTid,
                std::vector<T>& contents) :
          NTreeLeafNode(d, mls) {
    candOrder = co;
    pMethod = pm;
    insert(contents, centerTid);
    int noDistFunCallsBefore = dc.getNoDistFunCalls();
    if (variant == 2 || variant >= 6) {
      node_t::initAuxStructures(getNoEntries());
      node_t::precomputeDistances(dc, getNoEntries(), true);
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
  
  bool isEmpty() const {
    return getNoEntries() == 0;
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
  
  void setMaxDist(const double m) {
    maxDist = m;
  }
  
  double getMaxDist() const {
    return maxDist;
  }
  
  void setCenters(const int nodeId, const std::vector<int>& pos, 
           const std::vector<T*>& objects, const std::vector<double>& maxDist) {
    node_t::setNodeId(nodeId);
    assert(pos.size() == objects.size() && maxDist.size() == 1);
    for (unsigned int i = 0; i < pos.size(); i++) {
//       if (entries[pos[i]] == 0) { // TODO: !
//         node_t::count++;
//       }
      entries[pos[i]] = objects[i];
    }
    node_t::count = pos.size();
    this->maxDist = maxDist[0];
  }
  
  bool contains(const T& entry, DistComp& dc) {
    double minDist;
    int nearestEntry = getNearestCenterPos(entry, dc, node_t::count, minDist);
    if (entries[nearestEntry]->getKey() == entry.getKey() &&
        entries[nearestEntry]->getTid() == entry.getTid()) {
      return true;
    }
    return false;
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
  
  int getNearestCenterPos(const T& o, DistComp& dc, const int size,
                          double& minDist) {
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
      return node_t::getNearestCenterPos(o, dc, size, odist3d, true, minDist);
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
             const PartitionMethod partMethod) {
    assert(false);
  }
  
  void insert(const T& entry, DistComp& dc, const PartitionMethod partMethod) {
    assert(node_t::count < node_t::maxLeafSize);
    node_t::deleteAuxStructures(node_t::count);
    entries[node_t::count] = new T(entry);
    node_t::count++;
    node_t::initAuxStructures(node_t::count);
    node_t::precomputeDistances(dc, node_t::count, true);
  }
  
  void remove(const T& entry, DistComp& dc, const PartitionMethod partMethod) {
    assert(node_t::count > 0);
    double minDist;
    bool removed = true;
    while (removed && node_t::count > 0) {
      int nearestEntryPos = getNearestCenterPos(entry, dc, node_t::count, 
                                                minDist);
      T* nearestEntry = entries[nearestEntryPos];
      if (dc(entry, *nearestEntry) == 0.0 && 
          nearestEntry->getTid() == entry.getTid()) {
        delete nearestEntry;
        if (nearestEntryPos < node_t::count - 1) {
          entries[nearestEntryPos] = entries[node_t::count - 1];
          entries[node_t::count - 1] = 0;
        }
        node_t::deleteAuxStructures(node_t::count);
        node_t::count--;
        node_t::initAuxStructures(node_t::count);
        node_t::precomputeDistances(dc, node_t::count, true);
      }
      else {
        removed = false;
      }
    }
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
    out << "[ leaf node, id " << node_t::nodeId << ": \"";
    for (int i = 0; i < node_t::count; i++) {
      if (i > 0) {
        out << ", ";
      }
      dc.print(*entries[i], out);
    }
    out << "\"]" << endl;
    out << "maxDist = " << maxDist << endl;
    if (distMatrix != 0) {
      out << "distMatrix:" << endl;
      for (int i = 0; i < node_t::count; i++) {
        for (int j = 0; j <= i; j++) {
          out << distMatrix[i][j] << " ";
        }
        out << endl;
      }
    }
    return out;
  }
  
  std::ostream& print(std::ostream& out, DistComp &dc, const bool printSubtrees,
                  const bool printDistMatrix, const bool printPivotInfo) const {
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
  double maxDist; // distance between center and farthest entry
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
  
  /*
  The first constructor is used for the mclosestCenterN operator.
  
  */
  RangeIteratorN(node_t* root, const T& q, const DistComp& di) :
                                pos(0), queryObject(q), range(DBL_MAX), dc(di) {
    if (!root) {
      return;
    }
    int noDistFunCallsBefore = dc.getNoDistFunCalls();
    int noCands = (root->isLeaf() ? root->getNoEntries() : root->getDegree());
    double d_min;
    int c_q;
    if (root->isLeaf()) {
      c_q = ((leafnode_t*)root)->getNearestCenterPos(queryObject, dc,
                                                         noCands, d_min);
      addResult(((leafnode_t*)root)->getObject(c_q));
    }
    else {
      c_q = ((innernode_t*)root)->getNearestCenterPos(queryObject, dc,
                                                          noCands, d_min);
      addResult(((innernode_t*)root)->getCenter(c_q));
    }
    
    int noDistFunCallsAfter = dc.getNoDistFunCalls();
    if (root->isLeaf()) {
      stat.noDCLeaves += noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noLeaves++;
    }
    else {
      stat.noDCInnerNodes += noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noInnerNodes++;
    }
  }

  /*
  The second constructor is used for the range search.
  
  */
  RangeIteratorN(node_t* root, const T& q, const double r, const DistComp& di) :
                                      pos(0), queryObject(q), range(r), dc(di) {
    results.clear();
    if (!root) {
      return;
    }
    if (r == 0.0) {
      T* copyQ = new T(q);
      addResult(copyQ);
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
      case 7:
      case 8: {
        collectResultsNtree7(root);
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
    //stat.print(cout, dc.getNoDistFunCalls(), true);
  }
  
  void addResult(T* o) {
    results.push_back(o);
//     cout << "[" << o->getTid() << ", obj=" << *(o->getKey()) << "] ";
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
    int size = (node->isLeaf() ? ((leafnode_t*)node)->getNoEntries() :
                                 ((innernode_t*)node)->getDegree());
    if (node->isLeaf()) {
      int noDistFunCallsBefore = dc.getNoDistFunCalls();
      double distQnnq, distPnnq;
      int nnq = ((leafnode_t*)node)->getNearestCenterPos(queryObject, dc, size,
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
      int nnq = ((innernode_t*)node)->getNearestCenterPos(queryObject, dc, size,
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
      stat.noDCLeaves += noDistFunCallsAfter - noDistFunCallsBefore;
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
      int noDistFunCallsBefore = dc.getNoDistFunCalls();
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
      int noDistFunCallsAfter = dc.getNoDistFunCalls();
      stat.noLeaves++;
      stat.noDCLeaves += noDistFunCallsAfter - noDistFunCallsBefore;
    }
    else { // inner node
      int noDistFunCallsBefore = dc.getNoDistFunCalls();
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
      int noDistFunCallsAfter = dc.getNoDistFunCalls();
      stat.noDCInnerNodes += noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noInnerNodes++;
    }
  }
  
  /*
  function ~prune2~, used for mdistScanN7.
  
  */
  void prune2(node_t* node, bool cand[], bool cand2[], int noCands, int c_i, 
              double u) {
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
      for (int j = 0; j < noCands; j++) {
        if (cand[j] && j != c_i) {
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
    }
    else { // inner node
      for (int j = 0; j < noCands; j++) {
        if (cand[j] && j != c_i) {
          d_ij = node->getPrecomputedDist(c_i, j, false);
          maxDist_j = ((innernode_t*)node)->getMaxDist(j);
          if (range > u + d_ij + maxDist_j) {
            reportEntireSubtree(node->getChild(j));
//             cout << "    report ENTIRE child because " << range << " > "
//                  << u << " + " << d_ij << endl;
            cand[j] = false;
          }
          else if (range < abs(u - d_ij) - maxDist_j) {
            cand[j] = false;
          }
        }
      }
    }
  }
  
  void rangeSearch2(node_t* node) { // used by collectResultsNtree7
    int noDistFunCallsBefore = dc.getNoDistFunCalls();
    int noCands = (node->isLeaf() ? node->getNoEntries() : node->getDegree());
    bool cand[noCands], cand2[noCands]; 
    // cand <=> C, cand2 <=> C'
    std::fill_n(cand, noCands, true);
    std::fill_n(cand2, noCands, false);
    double us[noCands];
    std::fill_n(us, noCands, DBL_MAX);
    double maxDist_i;
    double d_min = DBL_MAX;
    for (int i = 0; i < noCands; i++) {
//       T* obj = (node->isLeaf() ? ((leafnode_t*)node)->getObject(i)
//                                : ((innernode_t*)node)->getCenter(i));
//       cout << "i = " << i << ", cand[tuple " << obj->getTid() << "] (" 
//            << (node->isLeaf()? "leaf node" : "inner node") << ") is " 
//            << (cand[i] ? "TRUE" : "FALSE") << endl;
      if (cand[i]) {
        cand[i] = false;
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
        prune2(node, cand, cand2, noCands, i, us[i]);
        if (node->isLeaf()) {
//           cout << "  RS2, LEAF, i = " << i << ", u = " << us[i] << ", cand ="
//                << (cand[i] ? " TRUE" : " FALSE") << endl;
          if (range > us[i]) {
            addResult(((leafnode_t*)node)->getObject(i));
          }
        }
        else { // inner node
          maxDist_i = ((innernode_t*)node)->getMaxDist(i);
          if (range > us[i] + maxDist_i) {
//             cout << "  report ENTIRE child because " << range << " > "
//                  << us[i] << " + " << maxDist_i << endl;
            reportEntireSubtree(node->getChild(i));
          }
          else if (range > us[i] - maxDist_i) {
            cand2[i] = true;
          }
//           cout << "  RS2, INNER NODE: i = " << i << ", u = " << us[i] 
//                << ", maxDist_i = " << maxDist_i << endl;
        }
      }
    }
    if (!node->isLeaf()) {
      for (int i = 0; i < noCands; i++) {
//         if (cand2[i]) {
//           cout << "  RS2: cand2[" << i << "] = " << cand2[i] << ", us[" << i
//                << "] = " << us[i] << endl;
//         }
        if (cand2[i] && us[i] <= d_min + 2 * range) {
          rangeSearch2(node->getChild(i));
        }
      }
    }
//     cout << "END of RangeSearch2" << endl;
    int noDistFunCallsAfter = dc.getNoDistFunCalls();
    if (node->isLeaf()) {
      stat.noDCLeaves += noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noLeaves++;
    }
    else {
      stat.noDCInnerNodes += noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noInnerNodes++;
    }
  }
  
  // rangeCenters2(C, q, r)
  int rangeCenters2(node_t* node, bool cand[], bool search[]) {
    int noDistFunCallsBefore = dc.getNoDistFunCalls();
    int noCands = (node->isLeaf() ? node->getNoEntries() : node->getDegree());
    std::fill_n(search, noCands, false);
    double d_min, d_iq, maxDist_i;
    int c_q;
    if (node->isLeaf()) {
      c_q = ((leafnode_t*)node)->getNearestCenterPos(queryObject, dc, noCands,
                                                     d_min);
      for (int i = 0; i < node->getNoEntries(); i++) {
        d_iq = node->getPrecomputedDist(c_q, i, true);
//         cout << "  LEAF: d_iq = " << d_iq << ", d_min = " << d_min 
//              << ", dc = "
//              << dc(((leafnode_t*)node)->getObject(i), queryObject) << endl;
        if (d_iq + d_min <= range) {
//           cout << "*add tid " << ((leafnode_t*)node)->getObject(i)->getTid()
//                << " from rc2" << endl;
          addResult(((leafnode_t*)node)->getObject(i));
        }
        else if (d_iq - d_min <= range) {
          T* object_i = ((leafnode_t*)node)->getObject(i);
          if (dc(*object_i, queryObject) <= range) {
//            cout << "#add tid " << ((leafnode_t*)node)->getObject(i)->getTid()
//                << " from rc2" << endl;
            addResult(object_i);
          }
        }
        cand[i] = false;
      }      
    }
    else { // inner node
      c_q = ((innernode_t*)node)->getNearestCenterPos(queryObject, dc, noCands,
                                                      d_min);
      for (int i = 0; i < node->getDegree(); i++) {
        if (i != c_q) {
//           cout << "getPrecomputedDist(" << c_q << ", " << i << ")";
          d_iq = node->getPrecomputedDist(c_q, i, false);
//           cout << "  ..... ok, it's " << d_iq << endl;
          maxDist_i = ((innernode_t*)node)->getMaxDist(i);
          if (d_iq + d_min + maxDist_i <= range) {
            reportEntireSubtree(node->getChild(i));
          }
          else if (d_iq - d_min - maxDist_i <= range &&
                   d_iq <= 2 * d_min + 2 * range) {
            if (d_iq <= 2 * range) {
              search[i] = true;
            }
            else if (dc(*((innernode_t*)node)->getCenter(i), queryObject) <= 
                     d_min + 2 * range) {
              search[i] = true;
            }
          }
        }
        cand[i] = false;
//         cout << "  center #" << i << ", tid = " 
//              << ((innernode_t*)node)->getCenter(i)->getTid() << ", d_iq = " 
//              << (i == c_q ? 0.0 : d_iq) << ", d_min = " << d_min 
//              << ", maxDist_i = " << ((innernode_t*)node)->getMaxDist(i) 
//              << ", dc = " 
//              << dc(*((innernode_t*)node)->getCenter(i), queryObject) << endl;
      }
    }
    int noDistFunCallsAfter = dc.getNoDistFunCalls();
    if (node->isLeaf()) {
      stat.noDCLeaves += noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noLeaves++;
    }
    else {
      stat.noDCInnerNodes += noDistFunCallsAfter - noDistFunCallsBefore;
      stat.noInnerNodes++;
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
    return ((T*)(results[pos - 1]))->getTid();
  }
  
  T* nextObj() {
    assert(pos >= 0);
    if (pos >= (int)results.size()) {
      return 0;
    }
    pos++;
//     cout << "[" << results[pos - 1] << "] ";
    return (T*)(results[pos - 1]);
  }

  size_t noComparisons() const{
      return dc.getCount();
  }
  
  int getNoDistFunCalls() const {
    return dc.getNoDistFunCalls();
  }
  
  NTreeStat getStat() const {
    return stat;
  }

 private:
  std::vector<T*> results;
  int pos;
  T queryObject;
  double range;
  DistComp dc;
  NTreeStat stat; // statistics
};

struct TidDist {
  TidDist(const TupleId id, const double d) : tid(id), dist(d) {}
  
  bool operator<(const TidDist& td) const {
    if (dist != td.dist) {
      return dist < td.dist;
    }
    return tid < td.tid;
  }
  
  TupleId tid;
  double dist;
};

/*
Auxiliary class for a priority queue of objects.

*/
template<class T, class DistComp, int variant>
class NodePQElem {
 public:
  typedef NTreeNode<T, DistComp, variant> node_t;
  
  NodePQElem(node_t* _node, const bool _isNode, const double _dist, 
             const bool _isInside) {
    set(_node, _isNode, _dist, _isInside);
  }
    
  NodePQElem() {
    node = 0;
  }
  
  ~NodePQElem() { // TODO
//     if (!isNode && node != 0) {
//       delete node;
//     }
//     node = 0;
  }
  
  void set(node_t* _node, const bool _isNode, const double _dist, 
           const bool _isInside) {
    node = _node;
    isNode = _isNode;
    dist = _dist;
    isInside = _isInside;
  }
  
  node_t* getNode() {
    return node;
  }
  
  bool getIsNode() {
    return isNode;
  }
  
  double getDist() {
    return dist;
  }
  
  bool getIsInside() {
    return isInside;
  }
  
 private:
  node_t* node;
  bool isNode, isInside;
  double dist;
};

template<class T, class DistComp, int variant>
class NodePQComp {
 public:
  typedef NTreeNode<T, DistComp, variant> node_t;
  typedef NodePQElem<T, DistComp, variant> nodepqelem_t;
   
  bool operator() (nodepqelem_t& p1, nodepqelem_t& p2) {
    return p1.getDist() > p2.getDist();
  }
}; 

template<class T>
class ObjectDistPairComp {
 public:
  bool operator () (std::pair<T, double>& p1, std::pair<T, double>& p2) {
    return p1.second < p2.second;
  }
};

/*
3 class NNIteratorN

*/
template <class T, class DistComp, int variant>
class NNIteratorN {
 public:
  typedef NNIteratorN<T, DistComp, variant> nniterator_t;
  typedef RangeIteratorN<T, DistComp, variant> rangeiterator_t;
  typedef NTreeNode<T, DistComp, variant> node_t;
  typedef NTreeLeafNode<T, DistComp, variant> leafnode_t;
  typedef NTreeInnerNode<T, DistComp, variant> innernode_t;
  typedef NodePQElem<T, DistComp, variant> nodepqelem_t;
  typedef NodePQComp<T, DistComp, variant> nodepqcomp_t;
  
  NNIteratorN(node_t* root, const T& _q, const DistComp& di, const int _k) : 
                                                  q(_q), pos(0), dc(di), k(_k) {
    results.clear();
    if (k == 0) {
      k = INT_MAX;
    }
    collectNN(root);
    //stat.print(cout, dc.getNoDistFunCalls(), true);
  }
  
  void addResult(const TupleId id, const double d) {
    TidDist td(id, d);
    results.push_back(td);
  }
  
  int chooseCenter(node_t* node, const bool isInside, double& d_x) {
    int result = -1;
    if (isInside) {
      if (node->isLeaf()) {
        result = ((leafnode_t*)node)->getNearestCenterPos(q, dc, 
                                      ((leafnode_t*)node)->getNoEntries(), d_x);
      }
      else {
        result = ((innernode_t*)node)->getNearestCenterPos(q, dc, 
                                        ((innernode_t*)node)->getDegree(), d_x);
      }
      cout << "  isInside, " << (node->isLeaf() ? "LEAF" : "INNERNODE")
           << ": c_i = " << result << ", d_x = " << d_x << endl;
    }
    else {
      std::random_device seeder;
      std::mt19937 engine(seeder());
      int maxValue = -1;
      if (node->isLeaf()) {
        maxValue = ((leafnode_t*)node)->getNoEntries() - 1;
      }
      else {
        maxValue = ((innernode_t*)node)->getDegree() - 1;
      }
      std::uniform_int_distribution<int> dist(0, maxValue);
      result = dist(engine);
      if (node->isLeaf()) {
        d_x = dc(q, *(((leafnode_t*)node)->getObject(result)));
      }
      else {
        d_x = dc(q, *(((innernode_t*)node)->getCenter(result)));
      }
      cout << "chooseCenter: call DC" << endl;
    }
    return result;
  }
  
  bool hasEmptyChild(node_t* node, const int pos) const { // node(c_i) != empty
    if (node->isLeaf()) {
      return true;
    }
    node_t* childNode = ((innernode_t*)node)->getChild(pos);
    if (!childNode->isLeaf()) {
      return false;
    }
    return ((leafnode_t*)childNode)->getNoEntries() == 1;
  }
  
  leafnode_t* makeAuxNode(node_t* node, const int pos) {
    std::vector<T> contents;
    if (node->isLeaf()) {
      contents.push_back(*(((leafnode_t*)node)->getObject(pos)));
    }
    else {
      contents.push_back(*(((innernode_t*)node)->getCenter(pos)));
    }
    return new leafnode_t(node->getDegree(), node->getMaxLeafSize(),
                          node->getCandOrder(), node->getPruningMethod(), dc,
                          contents[0].getTid(), contents);
  }
  
/*
Meaning of a pq entry: The first boolean indicates whether the whole node (true)
or only an object (false; in this case, the first and only one) is considered.
The second boolean represents the status variable.
    
*/
  double getApproxRadius(node_t* node) { // getApproxRadius2 from paper
    std::priority_queue<nodepqelem_t, std::vector<nodepqelem_t>, nodepqcomp_t> 
      pq;
    nodepqelem_t newPQElem(node, true, 0.0, true), pqElem;
    pq.push(newPQElem);
    double result = -1.0;
    int pointsVisited = 0;
    node_t* tempNode = 0;
    while (!pq.empty()) {
      pqElem = pq.top();
      cout << "TOP: pqElem = " << (pqElem.getIsNode() ? "NODE, " : "OBJECT, ") 
           << pqElem.getDist() << endl;
      pq.pop();
      tempNode = pqElem.getNode();
      bool isInside = pqElem.getIsInside();
      if (!pqElem.getIsNode()) { // tempNode is only a data object
        result = std::max(result, pqElem.getDist());
        pointsVisited++;
        if (pointsVisited == k) {
          while (!pq.empty()) {
            pq.pop();
          }
          return result;
        }
      }
      else { // internal node or whole leaf
        double d_x = 0.0;
        int c_i = chooseCenter(tempNode, isInside, d_x);
        nodepqelem_t *npqelem = new nodepqelem_t(makeAuxNode(tempNode, c_i), 
                                                 false, d_x, isInside);
        cout << "PQ is " << (pq.empty() ? "" : "NOT ") << "empty" << endl;
        pq.push(*npqelem);
        cout << "PUSH1: " << d_x << endl;
        if (!tempNode->isLeaf()) {
          double r_i = ((innernode_t*)tempNode)->getMaxDist(c_i);
          nodepqelem_t *npqelem = new nodepqelem_t(
            ((innernode_t*)tempNode)->getChild(c_i), true, d_x - r_i, isInside);
          pq.push(*npqelem);
          cout << "PUSH2: " << d_x - r_i << endl;
        }
        int size = (tempNode->isLeaf() ?
                   ((leafnode_t*)tempNode)->getNoEntries() :
                   ((innernode_t*)tempNode)->getDegree());
        for (int j = 0; j < size; j++) {
          if (j != c_i) {
            double d_ij = (tempNode->isLeaf() ? 
                           tempNode->getPrecomputedDist(c_i, j, true) :
                           tempNode->getPrecomputedDist(c_i, j, false));
            nodepqelem_t *npqelem = new nodepqelem_t(makeAuxNode(tempNode, j), 
                                                 false, d_x + d_ij, false);
//            newPQElem.set(makeAuxNode(tempNode, j), false, d_x + d_ij, false);
            pq.push(*npqelem);
            cout << "PUSH3: " << d_x << " + " << d_ij << " = " << d_x + d_ij 
                 << endl;
            if (!tempNode->isLeaf()) {
              double r_j = ((innernode_t*)tempNode)->getMaxDist(j);
              newPQElem.set(((innernode_t*)tempNode)->getChild(j), true,
                            std::max(d_x, d_ij) - r_j, false);
              nodepqelem_t *npqelem = new nodepqelem_t(
                ((innernode_t*)tempNode)->getChild(j), true,
                std::max(d_x, d_ij) - r_j, false);
              pq.push(*npqelem);
              cout << "PUSH4: " << std::max(d_x, d_ij) - r_j << endl;
            }
          }
        }
      }
    }
    while (!pq.empty()) {
      pq.pop();
    }
    return result;
  }

  void collectNN(node_t* node) {
    double approxRadius = getApproxRadius(node);
    cout << "approxRadius is " << approxRadius << endl;
    rangeiterator_t* rit = new rangeiterator_t(node, q, approxRadius, dc);
    T* obj = rit->nextObj();
    double dist_i = 0.0;
    std::vector<TidDist> Res_2;
    while (obj != 0) {
      dist_i = dc(q, *obj);
      cout << "collectNN: call dc(q, " << obj->getTid() << ")" << endl;
      TidDist td(obj->getTid(), dist_i);
      Res_2.push_back(td);
      obj = rit->nextObj();
    }
    delete rit;
    std::sort(Res_2.begin(), Res_2.end());
    for (int i = 0; i < k; i++) {
      addResult(Res_2[i].tid, Res_2[i].dist);
    }
  } 
  
  rangeiterator_t* find1NN(node_t* node, double& radius) { // deprecated
    int noDistFunCallsBefore, noDistFunCallsAfter;
    int c_q;
    double d_min;
    node_t* node_temp = node;
    while (!node_temp->isLeaf()) {
      noDistFunCallsBefore = dc.getNoDistFunCalls();
      c_q = ((innernode_t*)node_temp)->getNearestCenterPos(q, dc, 
                                 ((innernode_t*)node_temp)->getDegree(), d_min);
//       maxDist = ((innernode_t*)node_temp)->getMaxDist(c_q);
      node_temp = node_temp->getChild(c_q);
      noDistFunCallsAfter = dc.getNoDistFunCalls();
      stat.noInnerNodes++;
      stat.noDCInnerNodes += noDistFunCallsAfter - noDistFunCallsBefore;
    }
    noDistFunCallsBefore = dc.getNoDistFunCalls();
    c_q = ((leafnode_t*)node_temp)->getNearestCenterPos(q, dc,
                               ((leafnode_t*)node_temp)->getNoEntries(), d_min);
    noDistFunCallsAfter = dc.getNoDistFunCalls();
    stat.noLeaves++;
    stat.noDCLeaves += noDistFunCallsAfter - noDistFunCallsBefore;
//     cout << "no Entries = " << ((leafnode_t*)node_temp)->getNoEntries()
//          << ", maxDistLeaf = " << ((leafnode_t*)node_temp)->getMaxDist() 
//          << ", distInLeaf = " << d_min << endl;
    radius = ((leafnode_t*)node_temp)->getMaxDist() + d_min;
    if (d_min == 0.0) {
      cout << "radius 0 ==> range query omitted" << endl;
      q.setTid(((leafnode_t*)node_temp)->getObject(c_q)->getTid());
    }
    else {
      cout << "perform range search with radius " << d_min << endl;
    }
    return new rangeiterator_t(node, q, d_min, dc);
  }
  
  void collectNNold(node_t* node) { // deprecated
    double radius;
    rangeiterator_t* rit = find1NN(node, radius);
    T* obj = rit->nextObj();
    assert(obj != 0);
    double dist = dc(q, *obj);
    addResult(obj->getTid(), dist);
    obj = rit->nextObj();
    while (obj != 0) {
      dist = dc(q, *obj);
      addResult(obj->getTid(), dist);
      obj = rit->nextObj();
    }
//     it = results.begin();
    if (k == 1) {
//       pruneResults(node);
      delete rit;
      return;
    }
    obj = rit->nextObj();
    while (obj != 0) {
      double dist = dc(q, *obj);
//       cout << "found object " << *(obj->getKey()) << " with dist " 
//            << dist << endl;
//       if (dist < nnDist) {
//         nnDist = dist;
//         nn = obj;
//       }
      addResult(obj->getTid(), dist);
      obj = rit->nextObj();
    }
//     cout << "NN = " << *(nn->getKey()) << ", dist = " << nnDist << endl;
    while ((int)results.size() < k &&
           (int)results.size() != node->getNoEntries()) { // continue search
      delete rit;
      radius = 2.0 * radius;
      rit = new rangeiterator_t(node, q, radius, dc);
      obj = rit->nextObj();
      while (obj != 0) {
        double dist = dc(q, *obj);
  //       cout << "found object " << *(obj->getKey()) << " with dist " 
  //            << dist << endl;
  //       if (dist < nnDist) {
  //         nnDist = dist;
  //         nn = obj;
  //       }
        addResult(obj->getTid(), dist);
        obj = rit->nextObj();
      }
    }
//     pruneResults(node);
    delete rit;
  }
  
  const TidDist next() {
    TidDist result(0, -1.0);
    assert(pos <= results.size());
    if (pos == results.size()) {
      return result;
    }
    result = results[pos];
    pos++;
    return result;
  }
  
  size_t noComparisons() const{
      return dc.getCount();
  }
  
  int getNoDistFunCalls() const {
    return dc.getNoDistFunCalls();
  }
  
  int getNoDistFunCallsInnerNodes() const {
    return stat.noDCInnerNodes;
  }
  
  int getNoDistFunCallsLeaves() const {
    return stat.noDCLeaves;
  }
  
  NTreeStat getStat() const {
    return stat;
  }
  
 private:
  T q;
  std::vector<TidDist> results;
  unsigned int pos; // pos inside result vector (used for transferring results)
  DistComp dc;
  int k; // number of nearest neighbors
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
  typedef NNIteratorN<T, DistComp, variant> nniterator_t;
  typedef NTreeNode<T, DistComp, variant> node_t;
  typedef NTree<T, DistComp, variant> ntree_t;
  typedef NTreeInnerNode<T, DistComp, variant> innernode_t;
  
  NTree(const int d, const int mls, DistComp& di, PartitionMethod pm,
        const int attr) : 
                degree(d), maxLeafSize(mls), attrNo(attr), root(0), dc(di), 
                partMethod(pm), candOrder(RANDOM), pMethod(SIMPLE) {
    if (variant > 2) {
      candOrder = PIVOT2;
      pMethod = MINDIST;
    }
  }
      
  NTree(const int d, const int mls, const CandOrder c, const PruningMethod pm,
        DistComp& di, const PartitionMethod partm, const int attr) :
      degree(d), maxLeafSize(mls), attrNo(attr), root(0), dc(di),
      partMethod(partm), candOrder(c), pMethod(pm) {
  }
  
  ~NTree() {
    if (root) {
      delete root;
    }
  }
  
  std::string getTypeName() const {
    return "ntree" + (variant > 1 ? std::to_string(variant) : "");
  }
  
  int getVariant() const {
    return variant;
  }
  
  std::string getEntryType() const {
    return T::getBasicType();
  }
  
  int getDegree() const {
    return degree;
  }
  
  int getMaxLeafSize() const {
    return maxLeafSize;
  }
  
  int getAttrNo() const {
    return attrNo;
  }
  
  CandOrder getCandOrder() const {
    return candOrder;
  }
  
  PruningMethod getPruningMethod() const {
    return pMethod;
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
  
  void setRoot(innernode_t* newRoot) {
    root = newRoot;
  }
  
  void build(std::vector<T>& contents) {
    if (root) {
      delete root;
    }
    if (contents.size() <= (unsigned int)maxLeafSize) {
      root = new leafnode_t(degree, maxLeafSize, candOrder, pMethod, dc, 0,
                            contents);
    }
    else {
      root = new innernode_t(degree, maxLeafSize, candOrder, pMethod);
      root->build(contents, dc, -1, partMethod);
    }
    assignNodeIds(0);
    computeStatistics(root);
    cout << endl;
    stat.print(cout);
  }
  
  void insert(const T& entry) {
    int previousSize = root->getNoEntries();
    if (!root) { // empty tree
      std::vector<T> contents;
      contents.push_back(entry);
      root = new leafnode_t(degree, maxLeafSize, candOrder, pMethod, dc, 0,
                            contents);
    }
    else {
      root->insert(entry, dc, partMethod);
    }
    if (root->getNoEntries() > previousSize) {
      int firstNodeId = root->getNodeId();
      assignNodeIds(firstNodeId);
    }
  }
  
  void remove(const T& entry) {
    if (!root) { // empty tree
      return;
    }
    root->remove(entry, dc, partMethod);
    int previousSize = root->getNoEntries();
    if (root->getNoEntries() < previousSize && root->getNoEntries() > 0) {
      int firstNodeId = root->getNodeId();
      assignNodeIds(firstNodeId);
    }
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
    ntree_t* res = new ntree_t(degree, maxLeafSize, dc, partMethod, attrNo);
    if (root) {
      res->root = root->clone();
    }
    return res;
  }
  
  rangeiterator_t* closestCenter(const T& q) const {
    return new rangeiterator_t(root, q, dc);
  }
  
  rangeiterator_t* rangeSearch(const T& q, const double range) const {
    return new rangeiterator_t(root, q, range, dc);
  }
  
  nniterator_t* nnSearch(const T& q, const int k) const {
    return new nniterator_t(root, q, dc, k);
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
  
  NTreeStat getStat() const {
    return stat;
  }
  
  void assignNodeIds(const int firstId = 0) {
    int currentId = firstId;
    std::stack<node_t*> nodeStack;
    nodeStack.push(root);
    node_t* node = 0;
    while (!nodeStack.empty()) {
      node = nodeStack.top();
      nodeStack.pop();
      node->setNodeId(currentId);
      currentId++;
      if (!node->isLeaf()) { // nothing to do for leaves
        innernode_t* inode = (innernode_t*)node;
        for (int i = inode->getCount() - 1; i >= 0; i--) {
          nodeStack.push(inode->getChild(i));
        }
      }
    }
  }
  
  
 protected:
  int degree, maxLeafSize, attrNo;
  node_t* root;
  DistComp dc;
  PartitionMethod partMethod;
  CandOrder candOrder; // random, two ref points, three ref points
  PruningMethod pMethod; // simple, minDist-based
  NTreeStat stat; // statistics
  
  static node_t* insert(node_t* root, const T& o, DistComp& dc,
                        const PartitionMethod partMethod) {
    root->insert(o, dc, partMethod);
    return root;
  }
};

}

#endif
