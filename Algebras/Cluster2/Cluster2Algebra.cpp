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
classes ~ClusterAlgebra~ and ~DBscan~ which contains the actual
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
#include "SpatialAlgebra.h"
#include "LogMsg.h"
#include "Stream.h"
#include "TupleIdentifier.h"

#include "MMRTree.h"
#include "MMRTreeAlgebra.h"
#include "RelationAlgebra.h"
//#include "FTextAlgebra.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

extern NestedList* nl;
extern QueryProcessor* qp;


namespace cl2{

#define MINIMUMPTS_DEF 4        // default min points   - MinPts
#define EPS_DEF 400             // default max distance - Eps

class DBscanC;

/*
3.1 Type Mapping function for operator dbscan

*/

static ListExpr dbscan_TM(ListExpr args){

string err = "stream(tuple(A)) x int x real expected";

if(nl->ListLength(args)!=4){
   return listutils::typeError(err);
}

if(!Stream<Tuple>::checkType(nl->First(args)) || 
   !CcInt::checkType(nl->Third(args)) ||
   !CcReal::checkType(nl->Fourth(args)) ){
   return listutils::typeError(err);
}

//TypeExpression is not finished yet
//to do: include the clusterID computed in algorithm
ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
string name = "geoData";
ListExpr type;

int index = listutils::findAttribute(attrList, name, type);
if (index == 0) {
	return listutils::typeError("Attribute " + name + 
				    "not present in stream");
}

if(!listutils::isKind(type,Kind::SPATIAL2D()) &&
   !listutils::isKind(type,Kind::SPATIAL3D()) ){
	string t = " (type is " + nl->ToString(type) + ")";
	return listutils::typeError("Attribute " + name + "is not "
			            "in Kind Spatial2D or Spatial 3D" +t);
}

int id = 1;

ListExpr appendList = nl->TwoElemList(nl->StringAtom("ClusterID"),id);

return nl->ThreeElemList(
		nl->SymbolAtom(Symbols::APPEND()),
		appendList,
		nl->First(args)
	);


}

/*
3.2 Value Mapping function for operator dbscan

*/

class dbscan_LocalInfo{

/*
~Members~

*/

public:
  mmrtree::Rtree<2>* tree;  	
  Points* pts;			

/*
~Constructor~

Creates a new local info from the value coming from the value mapping.

*/


  dbscan_LocalInfo(CcInt* minPts, CcReal* eps){
     if(!minPts->IsDefined() || !eps->IsDefined()){
         defined = false;
         return;
     }
     this->minPts = max(0,minPts->GetIntval());
     this->eps =  eps->GetRealval();
     this->eps2 = this->eps*this->eps;
     defined = true;

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

/*
~filter(long)~

Compute environments using filter

*/
void filter(long size){
     Point p;
     double min1[2];
     double max1[2];
     this->size = 1000; 
     no = new int[size];
     env1 = new set<int>*[size];
     pos = 0;
     clusterId = 1;
     
     // set all points to be UNCLASSIFIED
     // and clean all sets
     for(int i=0;i<size;i++){
       no[i] = UNCLASSIFIED;
       env1[i] = new set<int>();
     }

     for(int i=0;i<size;i++){
        pts->Get(i,p);
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
             env1[i]->insert(cand);
          }
        }
     }
}	





/*
~Members~

*/


private:	
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
	static long c = 0; 
	CcInt* minPts = static_cast<CcInt*>(args[1].addr);
	CcReal* eps = static_cast<CcReal*>(args[2].addr);
	if(!minPts->IsDefined() || !eps->IsDefined() ){
	  return 0;
	}
	int minPts_val = minPts->GetValue();
	int eps_val = eps->GetValue();
	if(minPts_val < 0 || eps_val < 0){
	  return 0;
	}
	local.setAddr(new dbscan_LocalInfo(minPts,eps));
	dbscan_LocalInfo* linfo = static_cast<dbscan_LocalInfo*>(local.addr);
	linfo->tree  = static_cast<mmrtree::Rtree<2>*>(local.addr);
	result = qp->ResultStorage(s);	
	Points* res = static_cast<Points*>(result.addr);
	res->Clear();
	res->SetDefined(true);
	Word elem;
	res->StartBulkLoad();
	qp->Open(args[0].addr);
	qp->Request(args[0].addr,elem); 
	while(qp->Received(args[0].addr)){
		//insert the stream into the MMRTree
		result = elem;
	  	Rectangle<2>* r = (Rectangle<2>*) result.addr;
		if (linfo->tree){
		   linfo->tree->insert(*r,c++);
		}
		//Save the stream as Points
		Point* p = static_cast<Point*>(elem.addr);
		  if(p->IsDefined()){
		    (*res) += *p;
		  } else {
		    res->EndBulkLoad(false,false,false);
		    res->Clear();
		    res->SetDefined(false);
		    qp->Close(args[0].addr);
		    return 0;
		  }
		p->DeleteIfAllowed();
		qp->Request(args[0].addr,elem);
		}
	qp->Close(args[0].addr);
	res->EndBulkLoad();
	linfo->pts = res;
	linfo->filter(c);
	linfo->getNext();
	return 0;
}



/*
3.3 Specification string for Operator dbscan

*/
const string dbscanSpec =
		"( ( \"Signature\" \"Syntax\" \"Meaning\" "
		"\"Example\" ) "
		"( <text>stream(tuple) x points x int x real -> "
		"stream(tuple)</text--->"
		"<text> _ dbscan [fun, minpts, epsilon ] </text--->"
    		"<text>For a point set given as a points value, "
		"compute the clusters using "
    		"the DBSCAN algorithm with parameters minPts "
		"(minimum number of points "
   		"forming a cluster core) and epsilon (maximum distance "
		"between points in "
    		"a cluster core). "
    		"Returns a stream of tuples representing the clusters. "
		"</text--->"
		"<text>query Kneipen dbscan[.geoData, 5,200.0] "
		"count</text--->"
		") )";

/*
3.4 Operator dbscan

*/
Operator dbscan (
      "dbscan",            //name
      dbscanSpec,          //specification
      dbscanFun,           //value mapping
      Operator::SimpleSelect, //trivial selection function
      dbscan_TM          //type mapping
);




/*
4 Creating the Cluster2 algebra

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


} // end of namespace cl2

/*
5 Initialization (Standard)

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
  return (new cl2::Cluster2Algebra());
}
