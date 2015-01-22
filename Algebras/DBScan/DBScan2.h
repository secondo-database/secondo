/*

----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Faculty of Mathematics and  Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----



1 DBScan

This class relalizes a DBScan using an r-tree as an index. Since the index
is a main memory representation, this class is restricted to small relations.

*/

#include "Stream.h"
#include "MMRTree.h"
#include "TupleStore1.h"


/*
1.1 Auxiliary class

This class represents the values which should be added to 

*/

class TupleInfo{
  public:
    TupleInfo():visited(false),clusterNo(-1),isSeed(false){}
    TupleInfo(bool v, int c): visited(v),clusterNo(c), isSeed(false) {}

  bool visited;
  int clusterNo; 
  bool isSeed;
};


/*
1.2 Main class

The template parameter represents the dimension of the rectangles which should be 
clustered.

*/
template<int dim>
class DBScanRT2{

public:


/*
1.2.1 Constructor

Within the constructor, the whole work is done.

*/
  DBScanRT2(Word _stream, CcReal* eps, CcInt* minPts, 
           int _CID, ListExpr _tt,size_t mem): 
     stream(_stream), eps(0),minPts(0), CID(_CID), currentCluster(0),
     finished(false), index(0), buffer(0), tupleStates(), tt(0), resIt(0){

     stream.open();
     if(eps->IsDefined() && minPts->IsDefined()){
        this->eps = eps->GetValue();
        this->minPts = minPts->GetValue();
        if(this->eps < 0 || this->minPts<0){
           finished = true;
        } 
     } else {
        finished = true;
     }
     if(!finished){
        tt = new TupleType(_tt);
        initialize(mem);
        computeCluster();
        resIt = buffer->MakeScan();
     }
  }


/*
1.2.3 Destructor

*/  
  ~DBScanRT2(){
    stream.close();
    if(index) delete index;
    if(resIt) delete resIt;
    if(buffer) delete buffer;
    if(tt) tt->DeleteIfAllowed();
  }


/*

1.2.2 Function ~next~

Return the next result tuple or 0 if there are no more tuples.

*/
  Tuple* next(){
     if(finished){
        return 0;
     }
     Tuple* tuple = resIt->GetNextTuple();
     if(!tuple){
        finished=true;
        return 0;
     }
     TupleId id = resIt->GetTupleId();
     Tuple* resTuple = new Tuple(tt);
     int as = tuple->GetNoAttributes();

     for(int i = 0; i<as; i++){
       resTuple->CopyAttribute(i,tuple,i);
     }
     tuple->DeleteIfAllowed();
     resTuple->PutAttribute(as, new CcInt(true, tupleStates[id].clusterNo));
     resTuple->PutAttribute(as+1, new CcBool(true,tupleStates[id].visited));
     return resTuple;
  }




private:
  Stream<Tuple> stream;
  double eps;
  int minPts;
  int CID;
  int currentCluster;
  bool finished;
  mmrtree::RtreeT<dim,TupleId>* index;
  TupleStore1* buffer;
  vector<TupleInfo> tupleStates;
  TupleType* tt;
  GenericRelationIterator* resIt;
  

/*
1.2.4 ~initialize~

This function materialized the incoming stream and builds a main memory r-tree on it.
For each tuple, the initial tuple info (not visited, noise) is stored into a vector.

*/ 
  void initialize(size_t mem){
    Tuple* tuple;
    buffer = new TupleStore1(mem);
    index = new mmrtree::RtreeT<dim,TupleId>(4,8);
    while((tuple = stream.request())){
       TupleId id = buffer->AppendTuple(tuple);
       Rectangle<dim>* rect = (Rectangle<dim>*)tuple->GetAttribute(CID); 
       index->insert(*rect,id);
       TupleInfo info(false,-1);
       tupleStates.push_back(info);
       tuple->DeleteIfAllowed();
    }
  }

/*
1.2.5 ~computeCluster~

This realizes the main function of the DBScan algorithm.

*/ 
  void computeCluster(){

    GenericRelationIterator* it = buffer->MakeScan();
    Tuple* tuple;
    while((tuple = it->GetNextTuple())){
       TupleId id = it->GetTupleId();
       if(!tupleStates[id].visited){
          tupleStates[id].visited=true;
          vector<TupleId> seeds;
          regionQuery(id, seeds);
          if(seeds.size() <  minPts){
             // no core point
             tupleStates[id].clusterNo = -1; 
          } else {
             // a core point
             for(int i=0;i<seeds.size();i++){
                tupleStates[seeds[i]].isSeed =true;
             }
             expandCluster(id,seeds); 
             currentCluster++;
          }
       } 
       tuple->DeleteIfAllowed(); 
    }  
    delete it;
  } 


/*
1.2.6 ~regionQuery~

This function stores all tuple id's of tuples within  an epsilon environment
of the tuple with id ~id~ to ~neighbors~.

*/
  int regionQuery(TupleId id, vector<TupleId>& neighbors){
     Tuple* tuple = buffer->GetTuple(id);
     Rectangle<dim>* rect1 = (Rectangle<dim>*) tuple->GetAttribute(CID);
     Rectangle<dim> rect2 = *rect1;
     rect2.Extend(eps);
     typename mmrtree::RtreeT<dim,TupleId>::iterator* it = index->find(rect2);
     const TupleId* tid;
     int count = 0;
     while( (tid = it->next())){
         if(*tid!=id){
            Tuple* st = buffer->GetTuple(*tid);
            if(((Rectangle<dim>*)st->GetAttribute(CID))->Distance(*rect1) 
                 < eps){
                neighbors.push_back(*tid);
                count++;
            }
            st->DeleteIfAllowed();
         }
     }
     tuple->DeleteIfAllowed();
     delete it;
     return count;
  }


/*
1.2.7 ~expandCluster~

This function expandes a cluster. The ~id~ must be the tuple-id 
of a core point. The list seeds are the neighborrs of this tuple.

*/
  void expandCluster(TupleId id, vector<TupleId>& seeds){
     tupleStates[id].clusterNo = currentCluster;
     tupleStates[id].visited = true;
     while(!seeds.empty()){
       TupleId tid = *(seeds.begin());
       seeds.erase(seeds.begin());
       tupleStates[tid].isSeed = false;
       if(!tupleStates[tid].visited){
          tupleStates[tid].visited = true;
          tupleStates[tid].clusterNo = currentCluster;
          vector<TupleId> n;
          regionQuery(tid,n);
          sort(n.begin(),n.end());
          if(n.size()>=minPts){
            merge(seeds,n);
          }
       } else { // already visited, may be noise
         if(tupleStates[tid].clusterNo <0){
            tupleStates[tid].clusterNo = currentCluster;
         }
       }

     }
  }

  void merge(vector<TupleId>& v1, vector<TupleId>& v2){
     vector<TupleId>::iterator it;
     for(it = v2.begin();it!=v2.end();it++){
       if(!tupleStates[*it].isSeed){
          v1.push_back(*it);
          tupleStates[*it].isSeed = true;
       }
     }
  }

};
