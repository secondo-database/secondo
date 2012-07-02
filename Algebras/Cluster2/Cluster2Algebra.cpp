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

[1] Implementation of the DBScan Algorithm.

2012, Implementation of operator dbscan.

[TOC]

1 Overview

This implementation file essentially contains the implementation of the
class ~DBscanC~.

2 Defines and Includes

Eps is used for the DBScan Algorithm as the maximal distance, the
minimum points (MinPts) may be apart. If there are further points
in the Eps-range to one of the points in the cluster, this point
(and further points from this on) belong to the same cluster.

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "LogMsg.h"
#include "TupleIdentifier.h"

#include "MMRTree.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

extern NestedList* nl;
extern QueryProcessor* qp;


namespace dbsalg{

#define MINIMUMPTS_DEF 4        // default min points   - MinPts
#define EPS_DEF 400             // default max distance - Eps

class DBscanC;

class DBscanC
{
public:
  DBscanC();
  DBscanC(Word*, Word&, int, Word&, Supplier, double**);

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

3.1 Type Mapping for operator dbscan

*/

static ListExpr dbscan_TM(ListExpr args){
  cout << "DBScan: ";
  string test;
  nl->WriteToString(test, args);
  cout << test;
  cout << endl;

if(nl->ListLength(args)!=3){
   ErrorReporter::ReportError("stream x int x real expected");
   return nl->TypeError();
}

if(!nl->IsEqual(nl->Second(args),CcInt::BasicType()) &&
   !nl->IsEqual(nl->Third(args),CcReal::BasicType())){
   ErrorReporter::ReportError("int x real expected");
   return nl->TypeError();
}

if(!listutils::isTupleStream(nl->First(args))){
   ErrorReporter::ReportError("first argument is not a tuple stream");
   return nl->TypeError();
}

return nl->First(args);
}


/*

3.2 Value Mapping function for operator dbscan

*/

class dbscan_LocalInfo{
public:

/*
~Constructor~

Creates a new local info from the value coming from the value mapping.

*/

  dbscan_LocalInfo(Points* pts, CcInt* minPts, CcReal* eps){
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
 ~dbscan_LocalInfo(){
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


int dbscanFun (Word* args, Word& result, int message, Word& local,
                Supplier s) {
Word elem;
dbscan_LocalInfo *dli = static_cast<dbscan_LocalInfo*>(local.addr);

 switch(message){
      case OPEN : {
	//folgender Teil funktioniert nicht
	ListExpr* lex = static_cast<ListExpr*>(args[0].addr);
	string attname;	
	int searchpos = 1;
	int attpos = listutils::findType(lex,
		nl->SymbolAtom(Points::BasicType()), attname, searchpos);
	Points* pts = static_cast<Points*>(args[attpos].addr);
        CcInt* minPts = static_cast<CcInt*>(args[1].addr);
        CcReal* eps = static_cast<CcReal*>(args[2].addr);
	dbscan_LocalInfo *dli = new dbscan_LocalInfo(pts, minPts, eps);
	local.setAddr(dli);
	qp->Open(args[0].addr);
        return 0;
    } case REQUEST : {
        if(local.addr==0){
          return CANCEL;
        }
	qp->Request(args[0].addr, elem);
	while (qp->Received(args[0].addr) ) {
	        Points* hasNext = dli->getNext();
        	result.setAddr(hasNext);
        	if(hasNext){
           	return YIELD;
        	} else {
           	return CANCEL;
        	}
	}
    } case CLOSE : {
	qp->Close(args[0].addr);
        return 0;
    }
 }
 return -1; // should never be reached
}


/*
6.3 Specification string for Operator dbscan

*/
const string dbscanSpec =
		"( ( \"Signature\" \"Syntax\" \"Meaning\" "
		"\"Example\" ) "
		"( <text>points x int x real -> stream(points)</text--->"
		"<text> _ dbscan [ minpts, epsilon ] </text--->"
    "<text>For a point set given as a points value, compute the clusters using "
    "the DBSCAN algorithm with parameters minPts (minimum number of points "
    "forming a cluster core) and epsilon (maximum distance between points in "
    "a cluster core). "
    "Returns a stream of points values (point sets) representing the clusters. "
		"</text--->"
		"<text>query Kneipen dbscan[5,200.0] count</text--->"
		") )";

/*
7.3 Operator dbscan

*/
Operator dbscan (
      "dbscan",            //name
      dbscanSpec,          //specification
      dbscanFun,           //value mapping
      Operator::SimpleSelect, //trivial selection function
      dbscan_TM          //type mapping
);



/*
8.1 Creating the cluster algebra

*/
class Cluster2Algebra : public Algebra
{
public:
  Cluster2Algebra() : Algebra()
  {
    AddOperator ( &dbscan );
  }
  ~Cluster2Algebra() {};
};


} // end of namespace dbs

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
InitializeCluster2Algebra(NestedList* nlRef, QueryProcessor* qpRef)
                           
{
  nl = nlRef;
  qp = qpRef;
  return (new dbsalg::Cluster2Algebra());
}
