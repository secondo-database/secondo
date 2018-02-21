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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the Cluster Algebra

June, 2006.
Basic functionality, one operator with default values and one
with maximal distance and minimal number of points as values.
Only the type 'points' has been implemented so far.

[TOC]

1 Overview

This implementation file essentially contains the implementation of the
classes ~ClusterAlgebra~ and ~DBccan~ which contains the actual
cluster algorithm.

2 Defines and Includes

Eps is used for the clusteralgorithm as the maximal distance, the
minimum points (MinPts) may be apart. If there are further points
in the Eps-range to one of the points in the cluster, this point
(and further points from this on) belong to the same cluster.

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "LogMsg.h"

#include "MMRTree.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace clusteralg{

#define MINIMUMPTS_DEF 4        // default min points   - MinPts
#define EPS_DEF 400             // default max distance - Eps

class DBscan;

class DBscan
{
public:
  DBscan();
  DBscan(Word*, Word&, int, Word&, Supplier, double**);

  int Parameter_Standard(double**,int);
  int Parameter_UserDefined(double**, int, int, int);//MinPts(int), Eps(int)
  void CopyToResult(Word*, Word&, int, Word&, Supplier, double**);

private:
  int MinPts;//minimum number of points to be a cluster
  int Eps;//max distance for MinPts and further points in cluster
  int FindClusters(double**, int); // main method
  bool ExpandCluster(double**, int,int);
  void Search(double**, int,int, int*);
};
/*
3.1 Type mapping function ~PointsTypeMapA~.

Used for the ~cluster\_a~ operator with one argument (points object).

Type mapping for ~cluster\_a~ is

----  points  [->]  points

----

*/
static ListExpr
PointsTypeMapA( ListExpr args )
{
if ( nl->ListLength(args) == 1 )
{
  ListExpr arg1 = nl->First(args);
  if ( nl->IsEqual(arg1, Points::BasicType()) )
  return nl->SymbolAtom(Points::BasicType());
}
return nl->SymbolAtom(Symbol::TYPEERROR());
}
/*
3.2 Type mapping function ~PointsTypeMapB~.

Used for the ~cluster\_b~ operator with three arguments (points object, Eps).

Type mapping for ~cluster\_b~ is

----  points[MinPts, Eps]  [->]  points

----

*/
static ListExpr
PointsTypeMapB( ListExpr args)
{
ListExpr arg1, arg2, arg3;
if ( nl->ListLength(args) == 3 )
{
  arg1 = nl->First(args);   // points
  arg2 = nl->Second(args); // MinPts --> int
  arg3 = nl->Third(args); // Eps --> int

  if (
      ( nl->IsEqual(arg1, Points::BasicType())) &&
      ( nl->IsEqual(arg2, CcInt::BasicType())) &&
      ( nl->IsEqual(arg3, CcInt::BasicType())))

    return nl->SymbolAtom(Points::BasicType());
}
return nl->SymbolAtom(Symbol::TYPEERROR());
}


static ListExpr cluster_c_TM(ListExpr args){
if(nl->ListLength(args)!=3){
   ErrorReporter::ReportError("points x int x real expected");
   return nl->TypeError();
}
if(nl->IsEqual(nl->First(args),Points::BasicType()) &&
   nl->IsEqual(nl->Second(args),CcInt::BasicType()) &&
   nl->IsEqual(nl->Third(args),CcReal::BasicType())){
   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(Points::BasicType()));

}
ErrorReporter::ReportError("points x int x real expected");
return nl->TypeError();
}


static ListExpr cluster_d_TM(ListExpr args){
if((nl->ListLength(args)==2) &&
   (nl->IsEqual(nl->First(args),Points::BasicType())) &&
   (nl->IsEqual(nl->Second(args),CcReal::BasicType()))){
   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(Points::BasicType()));
}
ErrorReporter::ReportError("points x real expected");
return nl->TypeError();
}



/*
5.1 Value mapping function for operator ~cluster\_a~.

Predefined values for Eps (distance) and MinPts (minimum number of points)
for the cluster algorithm are used.
First an array with four columns is set up, a pointer array is being
used in all the DBscan-class functions for access to this array.

*/
int cluster_aFun (Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
Points* ps = ((Points*)args[0].addr);
int cpoints_size = ps->Size();

double** cpoints;  // pointer-array to cpoints
double* rcpoints;  // real cpoints
int nrows = ps->Size();
int ncols = 4;        // cpoints: 0:x 1:y 2:border point 3:core point
int a;

// DATA array setup
// allocate memory for array 'rcpoints'
rcpoints = (double*) malloc(nrows * ncols * sizeof(double));
if (rcpoints == NULL)
{ printf("\nFailure to allocate room for the array\n");
  exit(0); }

// allocate memory for pointers to rows
cpoints =  (double**) malloc(nrows * sizeof(double *));
if (cpoints == NULL)
{  printf("\nFailure to allocate room for the pointers\n");
   exit(0);}

// point the pointers
for (a = 0; a < nrows; a++)
  cpoints[a] = rcpoints + (a * ncols);

// preset NOISE (0) and CORECHECK (0)
for(a=0; a < nrows; a++){
  cpoints[a][2] =  0.0;
  cpoints[a][3] =  0.0; }

// copy x/y from input into cluster array 'cpoints'
ps->StartBulkLoad();  // relax ordering
if(ps->IsEmpty()) {
  ((Points*)result.addr)->SetDefined(false);
  free(rcpoints);
  free(cpoints);
  return 1;
}

for(int a = 0; a < ps->Size();a++) // transfer x/y-values
{ Point p;                  // to cluster array
  ps->Get(a, p);
  cpoints[a][0] = p.GetX();
  cpoints[a][1] = p.GetY();} // end for

  ps->EndBulkLoad(true, false);

  // for testing copy input to output
  //Points* ps2;
  //ps2 = ps->Clone();
  //(Points*)result.addr = ps2;
  // comment rest of function, if used

/*
Create an instance of DBscan.

*/

DBscan cluster;

/*
Here the no-parameter default setup function is being called, which
itself calls the actual cluster algorithm.

*/
a = cluster.Parameter_Standard(cpoints ,cpoints_size);


// debugging
if ( RTFlag::isActive("ClusterText:Trace") ) {cmsg.file() << "Cluster:"
                                          "                           "
                                "cluster_aFun Ergebnis: " << a << endl;
cmsg.send();}
/*
Copy the result from the internal array 'cpoints' back into the
result 'points' memory location.

*/
cluster.CopyToResult(args,result, message, local, s, cpoints);
free(rcpoints);
return 0;
}
/*
5.2 Value mapping function for operator ~cluster\_b~.

This function receives tweo arguments: Eps and MinPts, which are used
for the cluster algorithm.

The first part ist identical to operator ~cluster\_a~.

*/

int cluster_bFun (Word* args, Word& result, int message, Word& local,
                Supplier s)
{
Points* ps = ((Points*)args[0].addr);
int cpoints_size = ps->Size();

double** cpoints;  // pointer-array to cpoints
double* rcpoints;  // real cpoints
int nrows = ps->Size();
int ncols = 4;        // cpoints: 0:x 1:y 2:border point 3:core point
int a;

// DATA array setup
// allocate memory for array 'rcpoints'
rcpoints = (double*) malloc(nrows * ncols * sizeof(double));
if (rcpoints == NULL)
{ printf("\nFailure to allocate room for the array\n");
  exit(0); }

// allocate memory for pointers to rows
cpoints =  (double**) malloc(nrows * sizeof(double *));
if (cpoints == NULL)
{ printf("\nFailure to allocate room for the pointers\n");
  exit(0);}

// point the pointers
for (a = 0; a < nrows; a++)
  cpoints[a] = rcpoints + (a * ncols);

// preset NOISE (0) and CORECHECK (0)
for(a=0; a < nrows; a++){
  cpoints[a][2] =  0.0;
  cpoints[a][3] =  0.0;}

// debugging
if ( RTFlag::isActive("ClusterText:B") ) {
  cmsg.file() << "Cluster: cluster_bFun: " << endl;
  cmsg.send();}

// copy x/y from input into cluster array 'cpoints'
ps->StartBulkLoad();  // relax ordering
if(ps->IsEmpty()) {
  ((Points*)result.addr)->SetDefined(false);
  free(rcpoints);
  free(cpoints);
  return 1;
}

for(int a = 0; a < ps->Size();a++) // transfer x/y-values
{ Point p;                  // to cluster array
  ps->Get(a, p);
  cpoints[a][0] = p.GetX();
  cpoints[a][1] = p.GetY();} // end for

  ps->EndBulkLoad(true, false);

// for testing copy input to output
//Points* ps2;
//ps2 = ps->Clone();
//(Points*)result.addr = ps2;
// comment rest of function, if used

DBscan cluster;  // create DBscan object
/*
The following part is different from ~cluster\_a~.
The parameters are eing used for the cluster algorithm.
1: MinPts (int)

2: Eps    (int)

*/
CcInt* i1;
CcInt* i2;

i1 = ((CcInt*)args[1].addr);
i2 = ((CcInt*)args[2].addr);

int cMinPts = i1->GetIntval();
int cEps =    i2->GetIntval();

//debugging
if ( RTFlag::isActive("ClusterText:Trace") ) {
  cmsg.file() << "Cluster: cMinPts: ---------" << cMinPts << endl;
  cmsg.file() << "Cluster: cEps -------------" << cEps << endl;
  cmsg.send();}

a = cluster.Parameter_UserDefined(cpoints, cpoints_size, cMinPts, cEps);

// find cluster-points with user-defined parameters
// returns number of clusters found

if ( RTFlag::isActive("ClusterText:Trace") ) {cmsg.file() <<
                       "Cluster: cluster_bFun Ergebnis: " << a << endl;
       cmsg.send();}

cluster.CopyToResult(args, result, message, local, s, cpoints);
free(rcpoints);
return 0;
}

/*
5.3 Value Mapping function for cluster[_]c

*/

class ClusterC_LocalInfo{
public:
/*
~Constructor~

Creates a new local info from the value coming from the value mapping.

*/

  ClusterC_LocalInfo(Points* pts, CcInt* minPts, CcReal* eps){
     if(!pts->IsDefined() || !minPts->IsDefined() || !eps->IsDefined()){
         defined = false;
         return;
     }
     this->pts = pts;
     this->minPts = max(0,minPts->GetIntval());
     this->eps =  eps->GetRealval();
     this->eps2 = this->eps*this->eps;
     defined = true;
     size = pts->Size();
     no = new int[size];
     env1 = new set<int>*[size];
     pos = 0;
     // set all points to be  UNCLASSIFIED
     // and clean all sets
     for(int i=0;i<size;i++){
       no[i] = UNCLASSIFIED;
       env1[i] = new set<int>();
     }
     computeEnv();
     pos = 0;
     clusterId = 1;
  }

/*
~Destructor~

*/
 ~ClusterC_LocalInfo(){
    if(defined){
       for(int i=0;i<size;i++){
          delete env1[i];
       }
       delete[] env1;
       delete[] no;
     }
  }

/*
~getNext~

Returns the next cluster as points value.

*/
 Points* getNext(){
    if(!defined){ // no next cluster available
        return 0;
    }
    // search the next unclassified point
    while(pos<size){
       if(no[pos] >= 0 ){ // point already classified
         pos++;
       } else if ( env1[pos]->size()<minPts){
         no[pos] = -2; // mark as NOISE
         pos++;
       } else {
         // create a new cluster
         Points* result = expand(pos);
         clusterId++;
         pos++;
         return result;
       }
    }
    return 0;
 }


private:
/*
~Members~

*/

  Points* pts;         // source points value
  unsigned int minPts; // minimum size for core points
  double eps;          // epsilon
  double eps2;         // epsilon * epsilon
  bool  defined;
  int* no;             // cluster number
  set<int>** env1;     // environments;
  int size;            // number of points
  int pos;             // current position
  int clusterId;       // current cluster id

  static const int UNCLASSIFIED = -1;
  static const int NOISE = -2;

/*
~computeEnv~

This function computes the epsilon environment for each point
contained in pts;

*/
  void computeEnv(){
     mmrtree::Rtree<2> tree(10,30);
     Point p;
     double min1[2];
     double max1[2];

     /* insert all contained points into an R- tree */
     for(int i=0;i<pts->Size();i++){
        pts->Get(i,p);
        double x = p.GetX();
        double y = p.GetY();
        min1[0] = x - FACTOR;
        min1[1] = y - FACTOR;
        max1[0] = x + FACTOR;
        max1[1] = y - FACTOR;
        Rectangle<2> box(true,min1,max1);
        tree.insert(box, i);
     }


     /* compute environments using filter /refine */
     for(int i=0;i<pts->Size();i++){
        pts->Get(i,p);
        set<long> cands;
        double x = p.GetX();
        double y = p.GetY();
        min1[0] = x-eps;
        min1[1] = y-eps;
        max1[0] = x+eps;
        max1[1] = y+eps;
        Rectangle<2> searchbox(true, min1,max1);
        tree.findAll(searchbox,cands);
        set<long>::iterator it;
        for(it = cands.begin(); it!=cands.end(); it++){
          Point p2;
          int cand = static_cast<int>(*it);
          pts->Get(cand,p2);
          if(qdist(p,p2)<eps2){
             env1[i]->insert(cand);
          }
        }
     }
  }



/*
~qdist~

This function computes the square of the distance between two point value.


*/
double qdist(Point& p1, Point& p2){
  double x1 = p1.GetX();
  double x2 = p2.GetX();
  double y1 = p1.GetY();
  double y2 = p2.GetY();
  double dx = x1-x2;
  double dy = y1-y2;
  return dx*dx + dy*dy;
}


/*
~expand~

This function implements the expand algorithm of dbscan.

*/

Points* expand(int pos){
  Points* result = new Points(minPts);
  result->StartBulkLoad();

  set<int> seeds = *env1[pos];
  no[pos] = clusterId;
  Point p;
  pts->Get(pos,p);
  (*result) += (p);
  seeds.erase(pos);

  while(!seeds.empty()){
    int cpos = *(seeds.begin());
    if(no[cpos]<0){ // not classified by another cluster
       no[cpos] = clusterId;
       pts->Get(cpos,p);
       (*result) += (p);
       set<int>::iterator it;
       for(it = env1[cpos]->begin();it!=env1[cpos]->end(); it++){
          if(no[*it]<0){
             if(env1[*it]->size()>=minPts){ // a core point
                seeds.insert(*it);
             } else { // border point
                no[*it] = clusterId;
                pts->Get(*it,p);
                (*result) += p;
             }
          }
       }
    }
    seeds.erase(cpos);
  }
  result->EndBulkLoad();
  return result;
}

};


int cluster_cFun (Word* args, Word& result, int message, Word& local,
                Supplier s) {
 switch(message){
      case OPEN : {
        Points* pts = static_cast<Points*>(args[0].addr);
        CcInt* minPts = static_cast<CcInt*>(args[1].addr);
        CcReal* eps = static_cast<CcReal*>(args[2].addr);
        local.setAddr(new ClusterC_LocalInfo(pts,minPts,eps));
        return 0;
    } case REQUEST : {
        if(local.addr==0){
          return CANCEL;
        }
        ClusterC_LocalInfo* linfo =
               static_cast<ClusterC_LocalInfo*>(local.addr);

        Points* hasNext = linfo->getNext();
        result.setAddr(hasNext);
        if(hasNext){
           return YIELD;
        } else {
           return CANCEL;
        }
    } case CLOSE : {
        if(local.addr!=0){
           delete static_cast<ClusterC_LocalInfo*>(local.addr);
           local.setAddr(0);
        }
        return 0;
    }
 }
 return -1; // should never be reached
}


class ClusterG_LocalInfo{
public:
/*
~Constructor~

Creates a new local info from the value coming from the value mapping.

*/

  ClusterG_LocalInfo(Points* pts, CcInt* minPts, CcReal* eps){
     if(!pts->IsDefined() || !minPts->IsDefined() || !eps->IsDefined()){
         defined = false;
         return;
     }
     this->pts = pts;
     this->minPts = max(0,minPts->GetIntval());
     this->eps =  eps->GetRealval();
     this->eps2 = this->eps*this->eps;
     defined = true;
     size = pts->Size();
     no = new int[size];
     pos = 0;
     // set all points to be  UNCLASSIFIED
     // and clean all sets
     for(int i=0;i<size;i++){
       no[i] = UNCLASSIFIED;
     }
     createTree();
     pos = 0;
     clusterId = 1;
  }

/*
~Destructor~

*/
 ~ClusterG_LocalInfo(){
    if(defined){
       delete[] no;
       delete tree;
     }
  }

/*
~getNext~

Returns the next cluster as points value.

*/
 Points* getNext(){
    if(!defined){ // no next cluster available
        return 0;
    }
    // search the next unclassified point
    while(pos<size){
       if(no[pos] >= 0 ){ // point already classified
         pos++;
       } else {
          set<int>* env = getEnv(pos);
          unsigned int size = env->size();
          delete env;
          if ( size<minPts){
              no[pos] = -2; // mark as NOISE
              pos++;
          } else {
             // create a new cluster
             Points* result = expand(pos);
             clusterId++;
             pos++;
             return result;
         }
       }
    }
    return 0;
 }


private:
/*
~Members~

*/

  Points* pts;         // source points value
  unsigned int minPts; // minimum size for core points
  double eps;          // epsilon
  double eps2;         // epsilon * epsilon
  bool  defined;
  int* no;             // cluster number
  int size;            // number of points
  int pos;             // current position
  int clusterId;       // current cluster id
  mmrtree::Rtree<2>* tree;

  static const int UNCLASSIFIED = -1;
  static const int NOISE = -2;

  void createTree(){
     tree = new mmrtree::Rtree<2>(10,30);
     Point p;
     double min1[2];
     double max1[2];

     /* insert all contained points into an R- tree */
     for(int i=0;i<pts->Size();i++){
        pts->Get(i,p);
        double x = p.GetX();
        double y = p.GetY();
        min1[0] = x - FACTOR;
        min1[1] = y - FACTOR;
        max1[0] = x + FACTOR;
        max1[1] = y - FACTOR;
        Rectangle<2> box(true,min1,max1);
        tree->insert(box, i);
     }
  }

  set<int>* getEnv(int pos){
     set<int>* res = new set<int>();
     Point p;
     pts->Get(pos,p);
     set<long> cands;
     double x = p.GetX();
     double y = p.GetY();
     double min1[2];
     double max1[2];
     min1[0] = x-eps;
     min1[1] = y-eps;
     max1[0] = x+eps;
     max1[1] = y+eps;
     Rectangle<2> searchbox(true, min1,max1);
     tree->findAll(searchbox,cands);
     set<long>::iterator it;
     for(it = cands.begin(); it!=cands.end(); it++){
        Point p2;
        int cand = static_cast<int>(*it);
        pts->Get(cand,p2);
        if(qdist(p,p2)<eps2){
           res->insert(cand);
        }
    }
    return res;
  }


  unsigned int getEnvSize(int pos){
     int res = 0;
     Point p;
     double min1[2];
     double max1[2];
     pts->Get(pos,p);
     set<long> cands;
     double x = p.GetX();
     double y = p.GetY();
     min1[0] = x-eps;
     min1[1] = y-eps;
     max1[0] = x+eps;
     max1[1] = y+eps;
     Rectangle<2> searchbox(true, min1,max1);
     tree->findAll(searchbox,cands);
     set<long>::iterator it;
     for(it = cands.begin(); it!=cands.end(); it++){
        Point p2;
        int cand = static_cast<int>(*it);
        pts->Get(cand,p2);
        if(qdist(p,p2)<eps2){
           res++;
        }
    }
    return res;
  }

/*
~qdist~

This function computes the square of the distance between two point value.


*/
double qdist(Point& p1, Point& p2){
  double x1 = p1.GetX();
  double x2 = p2.GetX();
  double y1 = p1.GetY();
  double y2 = p2.GetY();
  double dx = x1-x2;
  double dy = y1-y2;
  return dx*dx + dy*dy;
}


/*
~expand~

This function implements the expand algorithm of dbscan.

*/

Points* expand(int pos){


  Points* result = new Points(minPts);
  result->StartBulkLoad();

  set<int>* seeds = getEnv(pos);
  no[pos] = clusterId;
  Point p;
  pts->Get(pos,p);
  (*result) += (p);
  seeds->erase(pos);
  while(!seeds->empty()){
    int cpos = *(seeds->begin());
    if(no[cpos]<0){ // not classified by another cluster
       no[cpos] = clusterId;
       pts->Get(cpos,p);
       (*result) += (p);
       tree->erase(p.BoundingBox(),cpos);
       set<int>::iterator it;
       set<int>* env = getEnv(cpos);
       for(it = env->begin();it!=env->end(); it++){
          if(no[*it]<0){
             if(getEnvSize(*it)>=minPts){ // a core point
                seeds->insert(*it);
             } else { // border point
                no[*it] = clusterId;
                pts->Get(*it,p);
                (*result) += p;
                tree->erase(p.BoundingBox(),*it);
             }
          }
       }
       delete env;
    }
    seeds->erase(cpos);
  }
  result->EndBulkLoad();
  delete seeds;
  return result;
}

};

int cluster_gFun (Word* args, Word& result, int message, Word& local,
                Supplier s) {
 switch(message){
      case OPEN : {
        Points* pts = static_cast<Points*>(args[0].addr);
        CcInt* minPts = static_cast<CcInt*>(args[1].addr);
        CcReal* eps = static_cast<CcReal*>(args[2].addr);
        local.setAddr(new ClusterG_LocalInfo(pts,minPts,eps));
        return 0;
    } case REQUEST : {
        if(local.addr==0){
          return CANCEL;
        }
        ClusterG_LocalInfo* linfo =
               static_cast<ClusterG_LocalInfo*>(local.addr);

        Points* hasNext = linfo->getNext();
        result.setAddr(hasNext);
        if(hasNext){
           return YIELD;
        } else {
           return CANCEL;
        }
    } case CLOSE : {
        if(local.addr!=0){
           delete static_cast<ClusterG_LocalInfo*>(local.addr);
           local.setAddr(0);
        }
        return 0;
    }
 }
 return -1; // should never be reached
}


/*
ClusterD ValueMapping

*/

class Edge{
public:
   int src;
   int dest;
   double cost;

  Edge(int src, int dest, double cost){
    this->src = src;
    this->dest = dest;
    this->cost = cost;
  }

  Edge(const Edge& edge){
     equalize(edge);
  }

  ~Edge(){}

  Edge& operator=(const Edge& src){
     equalize(src);
     return *this;
  }


  bool operator<(const Edge& e)const{
     if(cost < e.cost) return true;
     if(cost > e.cost) return false;
     if(src < e.src) return true;
     if(src > e.src) return false;
     return dest < e.dest;
  }

  bool operator>(const Edge& e) const{
     if(cost > e.cost) return true;
     if(cost < e.cost) return false;
     if(src > e.src) return true;
     if(src < e.src) return false;
     return dest > e.dest;
  }

  bool operator==(const Edge& e)const{
     return cost == e.cost &&
            src == e.src &&
            dest == e.dest;
  }


  ostream& printTo(ostream& o)const{
      return (o << "(" << src << " -> " << dest << ", " << cost <<")");
  }

  private:
    void equalize(const Edge& src){
       this->cost = src.cost;
       this->dest = src.dest;
       this->src = src.src;
    }


};


ostream& operator<<(ostream& o, const Edge& e){
    return e.printTo(o);
}


ostream& operator<<(ostream& o, const set<int>& e){
  set<int>::iterator it;
  o << "{";
  for(it = e.begin(); it!=e.end();it++){
     if(it!=e.begin()){
       o << ", ";
     }
     o << *it;
  }
  o << "}";
  return o;
}

struct intset{
  intset():member(),refs(1){}

  void deleteIfAllowed(){
     refs--;
     if(refs<1){
        delete this;
     }
  }

  set<int> member;
  int refs;
};


struct cCluster{
    cCluster(){
      cx = 0.0;
      cy = 0.0;
      member = new intset();
      forbidden = false;
    }

    cCluster(const cCluster& c){
       cx = c.cx;
       cy = c.cy;
       forbidden = c.forbidden;
       member = c.member;
       member->refs++;
    }

    cCluster& operator=(const cCluster& c){
       cx = c.cx;
       cy = c.cy;
       forbidden = c.forbidden;
       member = c.member;
       member->refs++;
       return *this;
    }

    ~cCluster(){
      member->deleteIfAllowed();
    }

    set<int>::iterator begin(){
      return member->member.begin();
    }

    set<int>::iterator end(){
      return member->member.end();
    }

    size_t size(){
      return member->member.size();
    }

    void insert(int i){
      member->member.insert(i);
    }

    void erase(int i){
       member->member.erase(i);
    }

    void clear(){
       member->member.clear();
    }


    double cx;
    double cy;
    intset* member; // avoid copying of this set !!!!
    bool forbidden;
};



class ClusterD_LocalInfo{
public:

/*
~Constructor~

Creates a new localinfo for the cluster[_]d operator.
The complete clustering is done here.

*/
  ClusterD_LocalInfo(Points* pts, CcReal* eps){
    env = 0;
    currentInitialCluster = 0;
    currentInitialPos = 0;
    pts->Copy();
    if(pts->IsDefined() && eps->IsDefined()){
      this->defined = true;
      this->eps = eps->GetRealval();
      this->eps2 = this->eps*this->eps;
      this->pts = pts;
      size = pts->Size();
      icluster = new int[size];
      for(int i=0;i<size;i++){
         icluster[i] = -1; // not assigned to an initial cluster
      }

      fcluster = 0;
      computeEnv();
      currentCNum = 0;
      origPos = 0;
    } else {
       this->defined = 0;
       this->pts = 0;
       this->eps = 0.0;
    }

  }

/*
~Destructor~


Destroys this instance.

*/

  ~ClusterD_LocalInfo(){
      pts->DeleteIfAllowed();
      if(icluster){
         delete[] icluster;
         icluster=0;
      }
      if(fcluster){
         delete[] fcluster;
         fcluster=0;
      }
      if(env){
         for(int i=0;i<size;i++){
           if(env[i] ){
             delete env[i];
             env[i] = 0;
           }
         }
         delete[] env;
         env = 0;
      }
      if(currentInitialCluster){
         delete currentInitialCluster;
      }
      if(origPos){
         delete [] origPos;
         origPos = 0;
      }
   }


/*
Returns the next cluster or 0 if no more clusters exist.

*/
  Points* getNext(int i){
     return getNextFinalCluster(i);
  }



private:
  bool defined;
  Points* pts;
  double eps;
  double eps2;  // the square of eps
  int* icluster; // initial cluster
  int* fcluster; // final cluster
  int size;

  set<int>** env;

  int currentInitialPos;
  int currentCNum;
  set<int>* currentInitialCluster;

  map<int, set<int> > currentFinalCluster;
  map<int, set<int> >::iterator currentFinalPos;

  int* origPos;


/*
~computeEnv~

This function computes the epsilon environment for each point
contained in pts;

*/
  void computeEnv(){

     env = new set<int>*[size];

     mmrtree::Rtree<2> tree(10,30);
     Point p;
     double min1[2];
     double max1[2];

     /* insert all contained points into an R- tree */
     for(int i=0;i<pts->Size();i++){
        pts->Get(i,p);
        double x = p.GetX();
        double y = p.GetY();
        min1[0] = x - FACTOR;
        min1[1] = y - FACTOR;
        max1[0] = x + FACTOR;
        max1[1] = y - FACTOR;
        Rectangle<2> box(true,min1,max1);
        tree.insert(box, i);
     }


     /* compute environments using filter /refine */
     for(int i=0;i<pts->Size();i++){
        env[i] = new set<int>();
        pts->Get(i,p);
        set<long> cands;
        double x = p.GetX();
        double y = p.GetY();
        min1[0] = x-eps;
        min1[1] = y-eps;
        max1[0] = x+eps;
        max1[1] = y+eps;
        Rectangle<2> searchbox(true, min1,max1);
        tree.findAll(searchbox,cands);
        set<long>::iterator it;
        for(it = cands.begin(); it!=cands.end(); it++){
          Point p2;
          int cand = static_cast<int>(*it);
          pts->Get(cand,p2);
          if(qdist(p,p2)<eps2){
             env[i]->insert(cand);
          }
        }
     }
  }
/*
~qdist~

~qdist~  computes the square of the distance between two points

*/
  double qdist(const Point& p1, const Point& p2){
    double x1 = p1.GetX();
    double x2 = p2.GetX();
    double y1 = p1.GetY();
    double y2 = p2.GetY();
    double dx = x1-x2;
    double dy = y1-y2;
    return dx*dx + dy*dy;

  }

  double qdist(double x1, double y1, double x2, double y2){
    double dx = x1-x2;
    double dy = y1-y2;
    return dx*dx + dy*dy;

  }

/*
~getNextInitialCluster~

This function extracts the next initial clsuter from the original point.
If the poinst value is exhausted, null is returned. The caller of this
function has to delete the returned value.

*/
  set<int>* getNextInitialCluster(){

    while(currentInitialPos<size &&
          icluster[currentInitialPos]>=0){
       currentInitialPos++;
    }


    if(currentInitialPos>=size){ // set exhausted
      return 0;
    }


    set<int>* res = new set<int>();


    set<int> seed(*env[currentInitialPos]);

     while(!seed.empty()){
       int p = *(seed.begin());
       if(icluster[p]<0){ // point is "free"
         icluster[p] = 1; // mark as assigned
         res->insert(p);
         for(set<int>::iterator it=env[p]->begin(); it!= env[p]->end(); it++){
             if(icluster[*it]<0){
                seed.insert(*it);
           }
       }
     }
     seed.erase(p);
   }

   return res;
  }

/*
~computeFinalCluster0~

Divides the points value ps into a set of clusters. The
first number is set to cnum. Cnum is increased automatically.

*/
 void computeFinalCluster0(){

    currentFinalCluster.clear();

    const int size = currentInitialCluster->size();

    // store all edges into an vector and
    // build a single cluster for each point
    Point p_i;
    Point p_j;

    vector<Edge> edges;

    int tmpfcluster[size];
    if(origPos){
      delete[] origPos;
    }

    origPos = new int[size];

    // initialize origPos
    for(int i=0;i<size;i++){
      origPos[i] = -1;
    }

    map<int,int> rev;


   int pos = 0;
   set<int>::iterator it1;


   for(it1=currentInitialCluster->begin();
       it1!=currentInitialCluster->end();
       it1++){
      rev[*it1] = pos;
      pos++;
   }

   pos = 0;
   for(it1=currentInitialCluster->begin();
       it1!=currentInitialCluster->end();
       it1++){
       origPos[pos] = *it1;
       tmpfcluster[pos] = pos; //each point builds its own cluster
       set<int> s;
       s.insert(pos);
       currentFinalCluster[pos] = s;
       pos++;
       set<int>* e = env[*it1];
       set<int>::iterator it2;
       Point p1;
       pts->Get(*it1,p1);
       for(it2=e->begin();it2!=e->end();it2++){
          Point p2;
          pts->Get(*it2,p2);
          double dist = qdist(p1,p2);
          int src = rev[*it1];
          int dest = rev[*it2];
          if(src<dest){
             edges.push_back(Edge(src,dest,dist));
          }
       }
   }

    // sort the vector of edges
    sort(edges.begin(),edges.end());



    vector<Edge>::iterator it;
    // insert edges and connect clusters
    for(it = edges.begin(); it!=edges.end();it++){
       Edge e = *it;
       int c1 = tmpfcluster[e.src];
       int c2 = tmpfcluster[e.dest];
       if(c1!=c2){ // otherwise the points are already in the same cluster
           // compute the maximum distance between points of c1
           // and points of c2
           set<int>::iterator it1,it2;
           set<int> s1 = currentFinalCluster[c1];
           set<int> s2 = currentFinalCluster[c2];
           double dist = 0.0;
           for(it1=s1.begin(); it1!=s1.end() && dist <= eps2; it1++){
             pts->Get(origPos[*it1],p_i);
             for(it2=s2.begin();it2!=s2.end() && dist <= eps2; it2++){
                pts->Get(origPos[*it2],p_j);
                dist = max(dist,qdist(p_i,p_j));
             }
           }
           if(dist <=eps2){ // build the union of the clusters
             for(it2=s2.begin();it2!=s2.end();it2++){
                s1.insert(*it2);
                tmpfcluster[*it2] = c1;
             }
             currentFinalCluster.erase(c1);
             currentFinalCluster.erase(c2);
             currentFinalCluster[c1] = s1;
           }

       }
    }

    currentFinalPos = currentFinalCluster.begin();


  // at this


    currentFinalPos = currentFinalCluster.begin();

 }

/*
Another method for dividing a group into several subgroups.

*/
void insertPoint(vector<cCluster>& clusters, int pos){

  Point p;
  pts->Get(pos,p);
  double x = p.GetX();
  double y = p.GetY();

  // first cluster
  if(clusters.empty()){
     cCluster cl;
     cl.cx = x;
     cl.cy = y;
     cl.forbidden = false;
     cl.insert(pos);
     clusters.push_back(cl);
     return;
  }

  int index = -1;
  double bestDist=eps2+10;

  for(unsigned int i=0;i<clusters.size();i++){
    if(!clusters[i].forbidden){
       double dist = qdist(x,y,clusters[i].cx,clusters[i].cy);
       if(index < 0 ||
          ((dist < bestDist) ||
           ((dist == bestDist) &&
            (clusters[index].size() > clusters[i].size())))){
          index = i;
          bestDist = dist;
       }
    }
  }

  if((index < 0) || (bestDist > eps2)){ // no cluster found, produce a new one
     cCluster cl;
     cl.cx = x;
     cl.cy = y;
     cl.forbidden = false;
     cl.insert(pos);
     clusters.push_back(cl);
     return;
  }

  // insert the point into the best cluster
  clusters[index].cx = (clusters[index].cx *
                        clusters[index].size() + x ) /
                       (clusters[index].size() +1);

  clusters[index].cy = (clusters[index].cy *
                        clusters[index].size() + y ) /
                       (clusters[index].size() +1 );

  clusters[index].insert(pos);

  // check whether some points are outside the cluster
  Point pc(true,clusters[index].cx, clusters[index].cy);

  set<int> removed;
  set<int>::iterator it;
  Point p2;
  for(it = clusters[index].begin();
      it != clusters[index].end();
      it++){
      pts->Get(*it,p2);
      if(qdist(pc,p2) > eps2){
         removed.insert(*it);
      }
  }

  // remove 'bad' points from the cluster
  double sx = 0.0;
  double sy = 0.0;
  for(it = removed.begin(); it!=removed.end();it++){
     Point p3;
     pts->Get(*it,p3);
     sx += p3.GetX();
     sy += p3.GetY();
     clusters[index].erase(*it);
  }

  /*
  // we avoid to correct the center again because of this correction
  // further points may go out from the cluster
  // thiy may lead to long running times

  // correct the center
  clusters[index].cx = ((clusters[index].cx *
                        (clusters[index].member.size() + removed.size())) -
                       sx) / clusters[index].member.size();
  clusters[index].cy = ((clusters[index].cy *
                        (clusters[index].member.size() + removed.size())) -
                       sy) / clusters[index].member.size();

  */


  // inserts the points again
  clusters[index].forbidden = true;
  for(it = removed.begin(); it!=removed.end();it++){
    insertPoint(clusters,*it);
  }
  clusters[index].forbidden = false;

}

void insertPointSimple(vector<cCluster>& clusters, int pos){

  Point p;
  pts->Get(pos,p);
  double x = p.GetX();
  double y = p.GetY();

  // clusters has to be non-empty

  int index = 0;
  int size = clusters.size();
  double bestDist = qdist(x,y,clusters[index].cx,clusters[index].cy);

  for(int i=1;i<size;i++){
     double dist = qdist(x,y,clusters[i].cx,clusters[i].cy);
     if( (dist < bestDist) ||
         ((dist == bestDist) &&
          (clusters[index].size() > clusters[i].size()))){
        index = i;
        bestDist = dist;
     }
  }

  if(!(bestDist <= eps2)){
     cout << "Error a point was not assigned to a cluster " << endl;
     cout << "The position of the point was " << pos << endl;
     cout << "The dist is " << bestDist << endl;
     cout << "Allowed dist " << eps2 << endl;
     cout << "Best cluster " << index << endl;
     cout << "#cluster " << size << endl;
     assert(false);

  }

  clusters[index].insert(pos);
}

void computeFinalCluster1(){

  currentFinalCluster.clear();

  vector<cCluster> currentCluster;

  // insert the points
  set<int>::iterator it1;
  for( it1 = currentInitialCluster->begin();
       it1 != currentInitialCluster->end();
       it1++){
     insertPoint(currentCluster,*it1);
  }


  // by the movement of the center, some
  // clusters may have unhandsome overlappings
  // we will redistribute the points located in such
  // overlappings

 // redistribute the points
  vector<cCluster>::iterator it3;
  for(it3 = currentCluster.begin(); it3 != currentCluster.end(); it3++){
      ((*it3)).clear();
  }

  for( it1 = currentInitialCluster->begin();
       it1 != currentInitialCluster->end();
       it1++){
     insertPointSimple(currentCluster,*it1);
  }

  // copy the result into currentFinalCluster
  int i = 0;
  vector<cCluster>::iterator it2;
  for(it2 = currentCluster.begin();
      it2 != currentCluster.end();
      it2++){
      currentFinalCluster[i++] = (*it2).member->member;
  }

  currentFinalPos = currentFinalCluster.begin();

}




/*
Returns the next cluster

*/


 Points* getNextFinalCluster(int method=0){
    if(!defined) return 0;
    if(!currentInitialCluster){
       currentInitialCluster = getNextInitialCluster();
       if(currentInitialCluster){
         switch(method){
           case 0:  computeFinalCluster0(); break;
           case 1:  computeFinalCluster1(); break;
           default: assert(false);
         }
       } else {
         return 0;
       }

    } else if(currentFinalPos == currentFinalCluster.end()){
       delete currentInitialCluster;
       currentInitialCluster = getNextInitialCluster();
       if(currentInitialCluster){
         switch(method){
           case 0:  computeFinalCluster0(); break;
           case 1:  computeFinalCluster1(); break;
           default: assert(false);
         }
       } else{
          return 0;
       }
    }

    set<int> cs = (*currentFinalPos).second;
    Points* res = new Points(cs.size());
    res->StartBulkLoad();
    Point p;
    set<int>::iterator it;
    for(it = cs.begin();it!=cs.end();it++){
        switch(method){
          case 0 : pts->Get(origPos[*it],p); break;
          case 1 : pts->Get(*it,p); break;
          default : assert(false);
        }
        (*res) += p;
    }
    res->EndBulkLoad();
    currentFinalPos++;

    return res;

 }


};

template<int i>
int cluster_dFun (Word* args, Word& result, int message, Word& local,
                Supplier s) {
 switch(message){
      case OPEN : {
        Points* pts = static_cast<Points*>(args[0].addr);
        CcReal* eps = static_cast<CcReal*>(args[1].addr);
        local.setAddr(new ClusterD_LocalInfo(pts,eps));
        return 0;
    } case REQUEST : {
        if(local.addr==0){
          return CANCEL;
        }
        ClusterD_LocalInfo* linfo =
               static_cast<ClusterD_LocalInfo*>(local.addr);

        Points* hasNext = linfo->getNext(i);
        result.setAddr(hasNext);
        if(hasNext){
           return YIELD;
        } else {
           return CANCEL;
        }
    } case CLOSE : {
        if(local.addr!=0){
           delete static_cast<ClusterD_LocalInfo*>(local.addr);
           local.addr = 0;
        }
        return 0;
    }
 }
 return -1; // should never be reached

}

/*
1.3 Value Mapping for cluster[_]f

Cluster[_]f implements the same slgoithms as cluster[_]e.
The difference is that this algoritms avoids the preprocessing step
to avoid large allocations of memory. Instead of that, an R-tree is
used to manage the centers.

1.3.1 LocalInfo

*/

class ClusterF_LocalInfo{

public:
/*
~Constructor~

Here, the complete work is done.

*/
  ClusterF_LocalInfo(Points* pts, CcReal* eps){
    this->pts = static_cast<Points*>(pts->Copy());
    if(!pts->IsDefined() || !eps->IsDefined()){
      defined = false;
      cluster = 0;
      size = 0;
    } else {
      defined = true;
      size = pts->Size();
      this->eps = max(FACTOR,eps->GetRealval());
      this->eps2 = this->eps * this->eps;
      cluster = new vector<cCluster>();
      computeCluster();
      pos=0;
      no_cluster = cluster->size();
    }
  }

/*
~Destructor~

Destroys this structure.

*/
  ~ClusterF_LocalInfo(){
      pts->DeleteIfAllowed();
      if(cluster){
        delete cluster;
      }
   }

/*
~getNext~

This function returns the next cluster as a points value.

*/
   Points* getNext(){
     if(!defined){
       return 0;
     } else if (pos>= no_cluster){
       return 0;
     } else {
       Points* res = new Points((*cluster)[pos].size());
       res->StartBulkLoad();
       set<int>::iterator it;
       Point p;
       for(it = (*cluster)[pos].begin();
           it != (*cluster)[pos].end(); it++){
         pts->Get(*it,p);
         (*res) += p;
       }
       res->EndBulkLoad();
       pos++;
       return res;
     }
   }

/*
2 Private Part


*/
private:
/*
~data members~

*/

 Points* pts;     // source points value
  double eps;     // maximum deviation
  double eps2;    // = eps * eps
  bool defined;   // true if the inputs are correct
  int size;       // = pts->Size()
  int pos;        // the current cluster
  int no_cluster; // number of clusters
  vector<cCluster >* cluster; // the clusters

/*
~qdist~

Returns the square of the Euclidean distance between the points defined
by (x1, y1) and (x2,y2).

*/
double qdist(const double x1,const  double y1,
             const double x2, const double y2) const{
  double dx = x2-x1;
  double dy = y2-y1;
  return dx*dx + dy * dy;
}

/*
~indexOfNearestCluster~

Computes the index of the cluster whose center is closest to p within
the cluster vector. If all clusters have a minimum distance larger than
eps, -1 will be returned. The r-tree is used as index and has to contain
all cluster centers.

*/
int indexOfNearestCluster(const mmrtree::Rtree<2>& tree, const Point& p) const{
    int res = -1;
    double bestDist = eps2 + 10.0;
    double min[2];
    double max[2];
    double x = p.GetX();
    double y = p.GetY();
    min[0] = x - eps - FACTOR;
    min[1] = y - eps - FACTOR;
    max[0] = x + eps + FACTOR;
    max[1] = y + eps + FACTOR;
    Rectangle<2> searchbox(true,min,max);
    set<long> cands;
    tree.findAll(searchbox,cands);
    set<long>::iterator it;
    for(it = cands.begin(); it != cands.end(); it++){
      cCluster c = cluster->at(*it);
      double d = qdist(c.cx,c.cy,x,y);
      if(d <= eps2 && d < bestDist && !c.forbidden){
        bestDist = d;
        res = *it;
      }
    }
    return res;
}

/*
~insertPointSimple~

This function assigns the point at position ~pos~ in the ~pts~ member
variable to the nearest cluster w.r.t. its center. The cluster itself remains unchanged, i.e.
the center is not moved.

*/
void insertPointSimple(const mmrtree::Rtree<2>& tree, const int pos){
   Point p;
   pts->Get(pos,p);
   int index = indexOfNearestCluster(tree,p);
    assert(index >= 0);
    (*cluster)[index].insert(pos);
}

/*
~insertPoint~

Inserts a point to the nearest cluster. If no appropriate cluster is found, a new one
is created. The center of the cluster is changed to be the center of all points
within the cluster including that one at positon ~pos~. Thereby, some points of the
cluster may exceed the maximum distance to the cluster's center. Such points are
reinserted recursively but the source cluster is locked.

*/
void insertPoint(mmrtree::Rtree<2>& tree, const int pos){
  Point p;
  pts->Get(pos,p);
  int index = indexOfNearestCluster(tree,p);
  double min[2];
  double max[2];
  double x = p.GetX();
  double y = p.GetY();
  if(index <0){ // no appropriate cluster found, build a new one
     cCluster c;
     c.cx = x;
     c.cy = y;
     c.insert(pos);
     c.forbidden = false;
     cluster->push_back(c);
     min[0] = x - FACTOR;
     min[1] = y - FACTOR;
     max[0] = x + FACTOR;
     max[1] = y + FACTOR;
     Rectangle<2> box(true,min,max);
     tree.insert(box,cluster->size()-1);
     return;
  }

  (*cluster)[index].insert(pos);
  double cx = (*cluster)[index].cx;
  double cy = (*cluster)[index].cy;
  int s = (*cluster)[index].size();
  (*cluster)[index].cx = ( (cx * (s - 1.0) + x) / s);
  (*cluster)[index].cy = ( (cy * (s - 1.0) + y) / s);

  min[0] = cx - FACTOR;
  min[1] = cy - FACTOR;
  max[0] = cx + FACTOR;
  max[1] = cy + FACTOR;
  Rectangle<2> erasebox(true,min,max);
  tree.erase(erasebox, index);

  min[0] = (*cluster)[index].cx - FACTOR;
  min[1] = (*cluster)[index].cy - FACTOR;
  max[0] = (*cluster)[index].cx + FACTOR;
  max[1] = (*cluster)[index].cy + FACTOR;
  Rectangle<2> newCenter(true,min,max);
  tree.insert(newCenter,index);

  repairClusterAt(index,tree);
}

/*
~repairClusterAt~

Removes all points exceeding the maximum allowed distance to the cluster's
center from the cluster at ~index~. Such points are reinserted.

*/
void repairClusterAt(const int index, mmrtree::Rtree<2>& tree){
  (*cluster)[index].forbidden = true;
  double cx = (*cluster)[index].cx;
  double cy = (*cluster)[index].cy;
  set<int> wrong;
  set<int>::iterator it;
  Point  p;
  for(it = (*cluster)[index].begin();
      it!= (*cluster)[index].end(); it++){
      pts->Get(*it,p);
      double d = qdist(cx,cy, p.GetX(),p.GetY());
      if(d>eps2){
        wrong.insert(*it);
      }
  }

  for( it=wrong.begin(); it!=wrong.end(); it++){
     (*cluster)[index].erase(*it);
  }

  for( it=wrong.begin(); it!=wrong.end(); it++){
     insertPoint(tree,*it);
  }


  (*cluster)[index].forbidden = false;


}

/*
~computeCluster~

This function divides a points value into a set of clusters.


*/
void computeCluster(){
   mmrtree::Rtree<2> tree(3,6);
   for(int i=0;i<size;i++){
      insertPoint(tree, i);
   }

   // redistribute the points
   for(unsigned int i=0;i<cluster->size();i++){
      (*cluster)[i].clear();
   }

   for(int i=0;i<size;i++){
      insertPointSimple(tree, i);
   }
}

};


/*
1.3.2  The actual Value Mapping

*/

int cluster_fFun (Word* args, Word& result, int message, Word& local,
                Supplier s) {
 switch(message){
      case OPEN : {
        Points* pts = static_cast<Points*>(args[0].addr);
        CcReal* eps = static_cast<CcReal*>(args[1].addr);
        local.setAddr(new ClusterF_LocalInfo(pts,eps));
        return 0;
    } case REQUEST : {
        if(local.addr==0){
          return CANCEL;
        }
        ClusterF_LocalInfo* linfo =
               static_cast<ClusterF_LocalInfo*>(local.addr);

        Points* hasNext = linfo->getNext();
        result.setAddr(hasNext);
        if(hasNext){
           return YIELD;
        } else {
           return CANCEL;
        }
    } case CLOSE : {
        if(local.addr!=0){
           delete static_cast<ClusterF_LocalInfo*>(local.addr);
           local.addr = 0;
        }
        return 0;
    }
 }
 return -1; // should never be reached

}



/*
6.1 Specification Strings for Operator cluster\_a

*/
const string cluster_aSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
      "\"Example\" ) "
      "( <text>points -> points</text--->"
      "<text>cluster_a ( _ )</text--->"
      "<text>Find cluster for"
      " points with standard cluster parameters. "
      "[ADVICE: Do not use.]</text--->"
      "<text>query cluster_a (Kneipen)</text--->"
      ") )";
/*
6.2 Specification Strings for Operator cluster\_b

*/
const string cluster_bSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
      "\"Example\" ) "
      "( <text>points -> points</text--->"
      "<text>_ cluster_b [_, _] </text--->"
      "<text>Find cluster for"
      " points with parameters MinPts (1) and Eps (2). "
      "[ADVICE: Do not use.]</text--->"
      "<text>query Kneipen cluster_b[5,200]</text--->"
      ") )";
/*
6.3 Specification string for Operator cluster\_c

*/
const string cluster_cSpec =
		"( ( \"Signature\" \"Syntax\" \"Meaning\" "
		"\"Example\" ) "
		"( <text>points x int x real -> stream(points)</text--->"
		"<text> _ cluster_c [ minpts, epsilon ] </text--->"
    "<text>For a point set given as a points value, compute the clusters using "
    "the DBSCAN algorithm with parameters minPts (minimum number of points "
    "forming a cluster core) and epsilon (maximum distance between points in "
    "a cluster core). "
    "Returns a stream of points values (point sets) representing the clusters. "
		"</text--->"
		"<text>query Kneipen cluster_c[5,200.0] count</text--->"
		") )";

const string cluster_gSpec =
		"( ( \"Signature\" \"Syntax\" \"Meaning\" "
		"\"Example\" ) "
		"( <text>points x int x real -> stream(points)</text--->"
		"<text> _ cluster_g [ minpts, epsilon] </text--->"
    "<text>For a point set given as a points value, compute the clusters using "
    "the DBSCAN algorithm with parameters minPts (minimum number of points "
    "forming a cluster core) and epsilon (maximum distance between points in "
    "a cluster core). [Alternative implementation?]."
    "Returns a stream of points values (point sets) representing the clusters. "
		"<text>query Kneipen cluster_g[5,200.0] count</text--->"
		") )";
/*
6.4 Specification string for Operator cluster\_d

*/
const string cluster_dSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
      "\"Example\" ) "
      "( <text>points x real -> stream(points)</text--->"
      "<text> _ cluster_d [ maxdist ] </text--->"
      "<text>For a point set given as a points value, compute the clusters "
      "using the a distance-based clustering algorithm with parameter maxdist "
      "(maximum distance between points within a cluster. Returns a stream of "
      "points values (point sets) representing the clusters. "
      "<text>query Kneipen cluster_b[200.0] count</text--->"
      ") )";
/*
7.1 Operator cluster\_a

*/
Operator cluster_a (
      "cluster_a",            //name
      cluster_aSpec,          //specification
      cluster_aFun,           //value mapping
      Operator::SimpleSelect, //trivial selection function
      PointsTypeMapA          //type mapping
);


/*
7.2 Operator cluster\_b

*/
Operator cluster_b (
      "cluster_b",            //name
      cluster_bSpec,          //specification
      cluster_bFun,           //value mapping
      Operator::SimpleSelect, //trivial selection function
      PointsTypeMapB          //type mapping
);


/*
7.3 Operator cluster[_]c

*/
Operator cluster_c (
      "cluster_c",            //name
      cluster_cSpec,          //specification
      cluster_cFun,           //value mapping
      Operator::SimpleSelect, //trivial selection function
      cluster_c_TM          //type mapping
);

Operator cluster_g (
      "cluster_g",            //name
      cluster_gSpec,          //specification
      cluster_gFun,           //value mapping
      Operator::SimpleSelect, //trivial selection function
      cluster_c_TM          //type mapping // equals to c
);

/*
7.4 Operator cluster[_]d

*/
Operator cluster_d (
        "cluster_d",            //name
        cluster_dSpec,          //specification
        cluster_dFun<0>,           //value mapping
        Operator::SimpleSelect, //trivial selection function
        cluster_d_TM          //type mapping
);

Operator cluster_e (
        "cluster_e",            //name
        cluster_dSpec,          //specification
        cluster_dFun<1>,           //value mapping
        Operator::SimpleSelect, //trivial selection function
        cluster_d_TM          //type mapping
);


Operator cluster_f (
        "cluster_f",            //name
        cluster_dSpec,          //specification
        cluster_fFun,           //value mapping
        Operator::SimpleSelect, //trivial selection function
        cluster_d_TM          //type mapping
);

/*
10.1    class DBscan (cluster algorithm)

*/
DBscan::DBscan() // Constructor
{
        //Default Constructor - does nothing
        return;
}


/*
10.2    Function FindClusters

This function is being called through the 'Parameter-' functions,
which set up Eps and MinPts.

It loops through each point and passes it on to the 'ExpandCluster' function
if the point has not been classified as a cluster member yet.

*/
int DBscan::FindClusters(double** cpoints, int cpoints_size){
  int point; // counter
  float percentage = 0.0;
  int anzahl = 0;


  // iterate all cpoints
  for(point=0; point < cpoints_size; point++)
    if (cpoints[point][2] == 0.0) // not yet classified as cluster member
      if(!ExpandCluster(cpoints, cpoints_size, point))
        if ( RTFlag::isActive("ClusterText:Trace") ) {
          cmsg.file() << "Cluster: Problem with ExpandCluster " << endl;
          cmsg.send();}

  // calculate percentage of cluster-cpoints
  for(point=0; point < cpoints_size; point++)
    if(cpoints[point][2] > 0)   {
      percentage++;
      anzahl++;
    }

  percentage = percentage/(float)point;

  if ( RTFlag::isActive("ClusterText:Trace") ) {
    cmsg.file() << "Cluster: Percentage: " << (percentage*100) << endl
     << "   EPS: " << Eps << endl << "   MinPts: " << MinPts << endl;
    cmsg.send();}

    return (int)(percentage*100); // return percentage of points in cluster
}

/*
10.3    Function Parameter\_Standard

This function only sets MinPts and Eps to the \#DEFINE values and
calls the function 'FindClusters'.

*/

int DBscan::Parameter_Standard(double** cpoints,int cpoints_size) {
  MinPts = MINIMUMPTS_DEF;
  Eps = EPS_DEF;

  int res;
  // call FindClusters
  res = FindClusters(cpoints,cpoints_size);
  return res;
}

/*
10.4    Function Parameter\_UserDefined

Similar to the function 'Parameter\_Standard', but sets MinPts and
Eps to the parameter values.

*/

int DBscan::Parameter_UserDefined(double** cpoints,int cpoints_size,
                                   int MinPts_user, int Eps_user){
  MinPts = MinPts_user;
  Eps = Eps_user;

  int res;

  res = FindClusters(cpoints,cpoints_size);

  return res;
}
/*
10.5    Function ExpandCluster

This function checks, if the passed point is member of a cluster and - if so -
checks for further members. For this, the function 'Search' is being used.

*/

bool DBscan::ExpandCluster(double** cpoints,int cpoints_size,int point)
{
  int* seeds;
  int a = 0;

  seeds = (int*) malloc((cpoints_size) * sizeof(int));
  seeds[0]=0; // none yet

  Search(cpoints, cpoints_size, point, seeds);

  // seeds: seeds[0] = number of seeds,
  // seeds[1...] = ('cpoints'-) numbers of Eps-Points

  if(seeds[0] < MinPts) // no core point - seeds[0]
                        // contains number of points in Eps
  { cpoints[point][3] = -1.0;   // no core point
    free(seeds);
    return true;}

    else // core point
    { while(a < seeds[0])
      { a++;
        point = seeds[a];
        if (cpoints[point][3] < 1.0) // no core point
          Search(cpoints, cpoints_size, point, seeds);
      } // end while

    for(a=1; a<seeds[0]+1; a++) //all seeds are member of cluster
      cpoints[seeds[a]][2]=1.0;

    free(seeds);
    return true;
  } // end if
}
/*
10.6    Function CopyToResult

This function copies the resulting cluster members back into the 'points'
value provided by the QueryProcessor.

*/
void DBscan::CopyToResult(Word* args, Word& result, int message, Word& local,
                          Supplier s, double** cpoints)
{
  Points* ps = ((Points*)args[0].addr);
  result = qp->ResultStorage( s ); // Query Processor provided Points
                                   //instance for the result
  // copy x/y from cluster array back into result (only cluster members)
  ((Points*)result.addr)->Clear();
  ((Points*)result.addr)->StartBulkLoad();
  for(int a=0; a < ps->Size(); a++)
    if(cpoints[a][2] > 0)       // cluster member
    {  Point p(true, cpoints[a][0], cpoints[a][1]);
       //((Points*)result.addr)->InsertPt(p);
       (*((Points*)result.addr)) += p;
  }  // end if / end for
  ((Points*)result.addr)->EndBulkLoad();
  // clean up, go home
  free(cpoints);
  return;
}
/*
10.7    Function Search

This function searches for all points in the 'Eps'-area of each given
point and returns these.
This function has so far been implemented only as a SLOW each-by-each search.
Alternative methods (R[*]-Tree, ...) should be implemented.

*/
void DBscan::Search(double** cpoints,int cpoints_size, int point, int* seeds){
// return EPS-environment of point in seeds
// ... could be implemented as an EFFICIENT r*-tree

  int a;
  //int b = seeds[0]+1;
  int c;
  int seedcounter = 0;
  bool check = true;
  double min1, min2, dist;

  for(a=0; a < cpoints_size; a++){
    min1 = (double)cpoints[point][0]-(double)cpoints[a][0];
    min2 = (double)cpoints[point][1]-(double)cpoints[a][1];
    dist = sqrt(pow(min1, 2.0) + pow(min2, 2.0));

    if(dist <= (double)Eps && point != a)
    {
      check = true;
      for(c=1; c < seeds[0]+1 && check == true; c++)
      {
        if(seeds[c] == a)
          check = false;
        // don't put the same point into seeds more than once
        else
          check = true;
      } // end for
      if(check) {
        // add a (in Eps) ... seed not yet included
        seeds[0]++;
        seeds[seeds[0]] = a;
      } // end if(check...
      seedcounter++;  // used for core-point classification
    } // end if(dist ...
  } // end for
  if (seedcounter > Eps)
    cpoints[point][3] = 1.0; // core-point classification
  return;
}



/*
8.1 Creating the cluster algebra

*/
class ClusterAlgebra : public Algebra
{
public:
  ClusterAlgebra() : Algebra()
  {
    AddOperator ( &cluster_a );
    AddOperator ( &cluster_b );
    AddOperator ( &cluster_c );
    AddOperator ( &cluster_g );
    AddOperator ( &cluster_d );
    AddOperator ( &cluster_e );
    AddOperator ( &cluster_f );

    ///// tracefile  /////
   // if ( RTFlag::isActive("ClusterText:Trace") ) {
   //   cmsg.file() << "Cluster: Constructor " << endl;
   //   cmsg.send();
   // }
    ///// tracefile end /////
  }
  ~ClusterAlgebra() {};
};


} // end of namespace clusteralg

/*
9.1 Initialization (Standard)

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeClusterAlgebra(  NestedList* nlRef,
                           QueryProcessor* qpRef
                           )
{
  nl = nlRef;
  qp = qpRef;

  ///// tracefile ////
 // if ( RTFlag::isActive("ClusterText:Trace") ) {
 //    cmsg.file() << "Cluster: InitializeClusterAlgebra "
 //                << endl; cmsg.send(); }

  return (new clusteralg::ClusterAlgebra());
}
