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
#include "../Spatial/SpatialAlgebra.h"
#include "../Temporal/TemporalAlgebra.h"
#include "LogMsg.h"
#include "RelationAlgebra.h"

#include <iostream>
#include <string>

extern NestedList* nl;
extern QueryProcessor* qp;


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
  if ( nl->IsEqual(arg1, "points") )
  return nl->SymbolAtom("points");
}
return nl->SymbolAtom("typeerror");
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
      ( nl->IsEqual(arg1, "points")) &&
      ( nl->IsEqual(arg2, "int")) &&
      ( nl->IsEqual(arg3, "int")))

    return nl->SymbolAtom("points");
}
return nl->SymbolAtom("typeerror");
}


static ListExpr cluster_c_TM(ListExpr args){
if(nl->ListLength(args)!=3){
   ErrorReporter::ReportError("points x int x real expected");
   return nl->TypeError();
}
if(nl->IsEqual(nl->First(args),"points") &&
   nl->IsEqual(nl->Second(args),"int") &&
   nl->IsEqual(nl->Third(args),"real")){
   return nl->TwoElemList(nl->SymbolAtom("stream"),
                          nl->SymbolAtom("points"));

}
ErrorReporter::ReportError("points x int x real expected");
return nl->TypeError();
}


static ListExpr cluster_d_TM(ListExpr args){
if((nl->ListLength(args)==2) &&
   (nl->IsEqual(nl->First(args),"points")) &&
   (nl->IsEqual(nl->Second(args),"real"))){
   return nl->TwoElemList(nl->SymbolAtom("stream"),
                          nl->SymbolAtom("points"));
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
if(ps->IsEmpty())
  {((Points*)result.addr)->SetDefined(false);
  return 1; }

for(int a = 0; a < ps->Size();a++) // transfer x/y-values
{ const Point *p;                  // to cluster array
  ps->Get(a, p);
  cpoints[a][0] = p->GetX();
  cpoints[a][1] = p->GetY();} // end for

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
if(ps->IsEmpty())
  {((Points*)result.addr)->SetDefined(false);
  return 1;}

for(int a = 0; a < ps->Size();a++) // transfer x/y-values
{ const Point *p;                  // to cluster array
  ps->Get(a, p);
  cpoints[a][0] = p->GetX();
  cpoints[a][1] = p->GetY();} // end for

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

return 0;
}

/*
5.3 Value Mapping function for cluster[_]c

*/

class ClusterC_LocalInfo{
public:
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
     env = new set<int>*[size];
     pos = 0;

     // set all points to be  UNCLASSIFIED
     // and clean all sets
     for(int i=0;i<size;i++){
       no[i] = UNCLASSIFIED;
       env[i] = new set<int>();
     }
     computeEnv();
     pos = 0;
     clusterId = 1;
  }

 ~ClusterC_LocalInfo(){
    if(defined){
       for(int i=0;i<size;i++){
          delete env[i];
       }
       delete[] env;
       delete[] no;
     }
  }


 Points* getNext(){
    if(!defined){ // no next cluster available
        return 0;
    }
    // search the next unclassified point
    while(pos<size){
       if(no[pos] >= 0 ){ // point already classified
         pos++;
       } else if ( env[pos]->size()<minPts){
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
  Points* pts; // source points value
  unsigned int minPts;
  double eps;
  double eps2;
  bool  defined;
  int* no;  // cluster number
  set<int>** env; // environments;
  int size;
  int pos;
  int clusterId;

  static const int UNCLASSIFIED = -1;
  static const int NOISE = -2;

  /* computes the epsilon environment for each point.
   *  takes quadratical runtime, should be changed later.
   */
  void computeEnv(){
    // debug::start  :: enforce a sorted input !!
     //   pts->StartBulkLoad();
     //   pts->EndBulkLoad();
    // debug::end
      for(int i=0;i<size;i++){
         computeEnv(i);
      }
  }

  /** computes the square of the distance between two points **/
  double qdist(const Point* p1, const Point* p2){
    double x1 = p1->GetX();
    double x2 = p2->GetX();
    double y1 = p1->GetY();
    double y2 = p2->GetY();
    double dx = x1-x2;
    double dy = y1-y2;
    return dx*dx + dy*dy;

  }

  /** compute the environment for the point at the given position
    * requires quadratic runtime, should be changed later
    */
  void computeEnv(const int pos){
      const Point* p;
      pts->Get(pos,p);


     /**
      // complete naive implementation
      for(int i=0;i<size;i++){
         const Point* p2;
         pts->Get(i,p2);
         if(qdist(p,p2)<=eps2){
            env[pos]->insert(i);
         }
      }
     **/

     // better implementation ; pts must be sorted
     double x = p->GetX();

     bool done = false;
     for(int i=pos; i>=0 && !done; i--){
        const Point* p2;
        pts->Get(i,p2);
        double x2 = p2->GetX();
        if( (x-x2) > eps){
          done = true;
        } else {
          if(qdist(p,p2)<=eps2){
           env[pos]->insert(i);
          }
        }
     }

     done = false;
     for(int i=pos+1; i<size && !done; i++){
        const Point* p2;
        pts->Get(i,p2);
        double x2 = p2->GetX();
        if( (x2-x) > eps){
          done = true;
        } else {
          if(qdist(p,p2)<=eps2){
           env[pos]->insert(i);
          }
        }
     }


  }

  /** expands a cluster */
  Points* expand(int pos){


    Points* result = new Points(minPts);
    result->StartBulkLoad();

    /*
      // Note : the following code does not implement the expand
      // function of dbscan.
      // its just for checking the correct computed environment of each point

      set<int>::iterator it;
      for(it =  env[pos]->begin(); it!=env[pos]->end();it++){
         const Point* p;
         pts->Get(*it,p);
         (*result) += (*p);
      }
    */


     // implementing the dbscan expand
     set<int> seeds = *env[pos];
     no[pos] = clusterId;
     const Point* p;
     pts->Get(pos,p);
     (*result) += (*p);
     seeds.erase(pos);

     while(!seeds.empty()){
       int cpos = *(seeds.begin());
       if(no[cpos]<0){ // not classified by another cluster
          no[cpos] = clusterId;
          pts->Get(cpos,p);
          (*result) += (*p);
          set<int>::iterator it;
          for(it = env[cpos]->begin();it!=env[cpos]->end(); it++){
             if(no[*it]<0){
                if(env[*it]->size()>=minPts){ // a core point
                   seeds.insert(*it);
                } else { // border point
                   no[*it] = clusterId;
                   pts->Get(*it,p);
                   (*result) += *p;
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
        local = SetWord(new ClusterC_LocalInfo(pts,minPts,eps));
        return 0;
    } case REQUEST : {
        if(local.addr==0){
          return CANCEL;
        }
        ClusterC_LocalInfo* linfo =
               static_cast<ClusterC_LocalInfo*>(local.addr);

        Points* hasNext = linfo->getNext();
        result = SetWord(hasNext);
        if(hasNext){
           return YIELD;
        } else {
           return CANCEL;
        }
    } case CLOSE : {
        if(local.addr!=0){
           delete static_cast<ClusterC_LocalInfo*>(local.addr);
           local = SetWord(Address(0));
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


  private:
    void equalize(const Edge& src){
       this->cost = src.cost;
       this->dest = src.dest;
       this->src = src.src; 
    }


};


class ClusterD_LocalInfo{
public:

/*
~Constructor~


Creates a new localinfo for the cluster[_]d operator.
The complte clustering is done here.

*/
  ClusterD_LocalInfo(Points* pts, CcReal* eps):usedPositions(){
    icluster = 0;
    env = 0;
    if(pts->IsDefined() && eps->IsDefined()){
      this->defined = true;
      this->eps = eps->GetRealval();
      this->eps2 = this->eps*this->eps;
      this->pts = pts;
      size = pts->Size();


      computeCluster();
      currentCNum = 0;
      currentPos = 0;
      computeFinalCluster();
      currentCNum = 0;
      currentPos = 0;
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
   }


/*
Returns the next cluster or 0 if no more clusters exist.

*/
  Points* getNext(){
     return getNextFinalCluster();
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

  int currentPos;
  int currentCNum;
  set<int> usedPositions;


  void computeCluster(){
      computeEnv();
      computeInitialCluster();
  }


/*
~computeEnv~

Computes the sets of points which are in a environment of each 
point in __pts__. The indexes of the points are store in the
environments cluster.

*/

  void computeEnv(){
      env = new set<int>*[size];
      for(int i=0;i<size;i++){
         computeEnv(i);
      }
  }

  /** computes the square of the distance between two points **/
  double qdist(const Point* p1, const Point* p2){
    double x1 = p1->GetX();
    double x2 = p2->GetX();
    double y1 = p1->GetY();
    double y2 = p2->GetY();
    double dx = x1-x2;
    double dy = y1-y2;
    return dx*dx + dy*dy;

  }

  /** compute the environment for the point at the given position
    * requires quadratic runtime, should be changed later
    */
  void computeEnv(const int pos){
      env[pos] = new set<int>();
      const Point* p;
      pts->Get(pos,p);

     // naive implementation, should be replaced
     // by using an RTree 
     double x = p->GetX();

     bool done = false;
     for(int i=pos; i>=0 && !done; i--){
        const Point* p2;
        pts->Get(i,p2);
        double x2 = p2->GetX();
        if( (x-x2) > eps){
          done = true;
        } else {
          if(qdist(p,p2)<=eps2){
           env[pos]->insert(i);
          }
        }
     }

     done = false;
     for(int i=pos+1; i<size && !done; i++){
        const Point* p2;
        pts->Get(i,p2);
        double x2 = p2->GetX();
        if( (x2-x) > eps){
          done = true;
        } else {
          if(qdist(p,p2)<=eps2){
           env[pos]->insert(i);
          }
        }
     }
  }


/*
~computeInitialCluster~

This function computes a clusternumber for each point. A cluster consists of
all points which are (indirect) reachable from a point.

*/
  void computeInitialCluster(){
    icluster = new int[size];
    for(int i=0;i<size;i++){
      icluster[i] = -1; // not assigned to a cluster
    }
    int clusterNum = 0;
    for(int i=0;i<size;i++){
      computeInitialCluster(i,clusterNum);
    }
  }

/* 
~computeInitialCluster~

This fucntion compute sthe initial cluster for for the point at position
pos. If the point is already assigned to a cluster, this function does 
nothing. If a new cluster was created, clusternum is incremented
automatically. Note: the environments of the points is partitially
destroyed during this function.

*/
  void computeInitialCluster(const int pos, int& clusterNum){
     if(icluster[pos]>=0){ // point already assigned to a cluster
       return;
     }
     set<int>* s = env[pos];
    
     while(!s->empty()){
       int p = *(s->begin());
       if(icluster[p]<0){ // point is "free"
         icluster[p] = clusterNum;
         for(set<int>::iterator it=env[p]->begin(); it!= env[p]->end(); it++){
             if(icluster[*it]<0){
                s->insert(*it);
             }
         }
       }  
       s->erase(p);
     } 
     clusterNum++;
  }

/*
~getNextInitialCluster~

Return  the next clustera s a points value. If no more clusters 
exist, NULL is returned.  

*/

  Points* getNextInitialCluster(){
    if(!defined){ // one of the arguments was not defined
      return 0;
    } 
    
    while(currentPos<size && icluster[currentPos]!=currentCNum){
       currentPos++;
    }


    if(currentPos>=size){ // set exhausted
      return 0;
    }

    Points* res = new Points(1);
    res->StartBulkLoad();
    int pos = currentPos;
    while(pos<size){
      if(icluster[pos]==currentCNum){
        const Point* pt;
        pts->Get(pos,pt); 
        (*res) += *pt;
      }
      pos++;
    } 
    res->EndBulkLoad(false,false);


    currentCNum++;
    return res;
  }

/*
~getNextFinalCluster~

Return  the next cluster a points value. If no more clusters 
exist, NULL is returned.  

*/

  Points* getNextFinalCluster(){
    if(!defined){ // one of the arguments was not defined
      return 0;
    } 
 
    while(currentPos<size && 
          usedPositions.find(fcluster[currentPos])!=usedPositions.end()){
       currentPos++;
    }

    if(currentPos>=size){ // set exhausted
      return 0;
    }
     
    usedPositions.insert(fcluster[currentPos]);
    currentCNum = fcluster[currentPos];

    cout << "process cluster " << currentCNum << endl;

    Points* res = new Points(1);
    res->StartBulkLoad();
    int pos = currentPos;
    while(pos<size){
      if(fcluster[pos]==currentCNum){
        const Point* pt;
        pts->Get(pos,pt); 
        (*res) += *pt;
      }
      pos++;
    } 
    res->EndBulkLoad(false,false);

    return res;
  }

/*
~computeFInalCluster~

Computes the final cluster numbers.

*/
  void computeFinalCluster(){
     int cnum = 0; // number for final cluster
     Points* pts;
     fcluster = new int[size];
     while((pts=getNextInitialCluster())){
        computeFinalCluster(pts,cnum);
        delete pts;
     }  
     
   // debug::start
     int minc = 1000;
     int maxc = -100;
     set<int> diff;
     for(int i=0;i<size;i++){
        if(fcluster[i]>maxc) maxc = fcluster[i];
        if(fcluster[i]<minc) minc = fcluster[i];
        diff.insert(fcluster[i]);
     }
     cout << "clusters in range " << minc << ", " << maxc << endl; 
     cout << "different" << diff.size() << endl;
   //debug::end
  }


/*
~computeFinalCluster~

Divides the points value ps into a set of clusters. The 
first number is set to cnum. Cnum is increased automatically.

*/
 void computeFinalCluster(const Points* ps, int& cnum){
    const int size = ps->Size();
    // we expect a lot of edges in the graph, so 
    // the implementation is based on a cost matrix

    // step 1 buils the graph, negative costs indicate
    // non-existent  edges   

    const Point* p_i;
    const Point* p_j;

    vector<Edge> edges;
    
    map<int,set<int> > theClusters;
    int tmpfcluster[size];

    for(int i=0;i<size;i++){
       ps->Get(i,p_i);
       tmpfcluster[i] = i; // each point builts its own cluster
 
       set<int> cl;
       cl.insert(i);
       theClusters[i] = cl;
       for(int j=i;j<size;j++){
         ps->Get(j,p_j);
         double dist = qdist(p_i,p_j);
         if(dist <= eps2){
            edges.push_back(Edge(i,j,dist));
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
           set<int> s1 = theClusters[c1];
           set<int> s2 = theClusters[c2];
           double dist = 0.0;
           for(it1=s1.begin(); it1!=s1.end() && dist <= eps2; it1++){
             for(it2=s2.begin();it2!=s2.end() && dist <= eps2; it2++){
                ps->Get(*it1,p_i);
                ps->Get(*it2,p_j);
                dist = max(dist,qdist(p_i,p_j));
             }
           }
           if(dist <=eps2){ // build the union of the clusters
             for(it2=s2.begin();it2!=s2.end();it2++){
                s1.insert(*it2);
                tmpfcluster[*it2] = c1;
             }
             theClusters.erase(c1);
             theClusters.erase(c2);
             theClusters[c1] = s1;
           }
       } 
    }

    // copy cluster numbers into the complete array
    map<int,set<int> >::iterator mit;
    for(mit=theClusters.begin();mit!=theClusters.end();mit++){
      set<int>::iterator sit;
      for(sit=mit->second.begin();sit!=mit->second.end();sit++){
          fcluster[*sit] = cnum;
      }
      cnum++;
    } 
    
 }
};


int cluster_dFun (Word* args, Word& result, int message, Word& local, 
                Supplier s) {     
 switch(message){
      case OPEN : {
        Points* pts = static_cast<Points*>(args[0].addr);
        CcReal* eps = static_cast<CcReal*>(args[1].addr);
        local = SetWord(new ClusterD_LocalInfo(pts,eps));
        return 0;
    } case REQUEST : {
        if(local.addr==0){
          return CANCEL;
        }
        ClusterD_LocalInfo* linfo = 
               static_cast<ClusterD_LocalInfo*>(local.addr);
        
        Points* hasNext = linfo->getNext();
        result = SetWord(hasNext);
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
6.1 Specification Strings for Operator cluster\_a

*/
const string cluster_aSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
      "\"Example\" ) "
      "( <text>points -> points</text--->"
      "<text>cluster_a ( _ )</text--->"
      "<text>Find cluster for"
      " points with standard cluster parameters.</text--->"
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
      " points with parameters MinPts (1) and Eps (2).</text--->"
      "<text>query Kneipen cluster_b[5,200]</text--->"
      ") )";
/*
6.3 Specification string for Operator cluster\_c

*/
const string cluster_cSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
      "\"Example\" ) "
      "( <text>points x int x real -> stream(points)</text--->"
      "<text> _ cluster_c [_, _] </text--->"
      "<text>compute cluster for given minPts"
      " and epsilon. </text--->"
      "<text>query Kneipen cluster_b[5,200.0] count</text--->"
      ") )";
/*
6.4 Specification string for Operator cluster\_d

*/
const string cluster_dSpec = 
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
      "\"Example\" ) "
      "( <text>points x real -> stream(points)</text--->"
      "<text> _ cluster_d [ _] </text--->"
      "<text>compute cluster for given  and epsilon. </text--->"
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

/*
7.4 Operator cluster[_]d

*/
Operator cluster_d (
        "cluster_d",            //name
        cluster_dSpec,          //specification
        cluster_dFun,           //value mapping
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
    //AddOperator ( &cluster_d );
                
    ///// tracefile  /////
   // if ( RTFlag::isActive("ClusterText:Trace") ) {
   //   cmsg.file() << "Cluster: Constructor " << endl;
   //   cmsg.send();
   // }
    ///// tracefile end /////   
  }
  ~ClusterAlgebra() {}; 
};

ClusterAlgebra clusterAlgebra;


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
  
  return (&clusteralg::clusterAlgebra);
}
