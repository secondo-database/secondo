

/*
----
This file is part of SECONDO.

Copyright (C) 2015, 
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
----


1 DBScanGen

This is a generic implemention of the DBScan algorithm.

Template parameters are S, the realizatrion od SetOfObjects and D, a distance 
function.

*/


#ifndef DBSCANGEN_H
#define DBSCANGEN_H

#include "AlgebraTypes.h"
#include "RelationAlgebra.h"


namespace dbscan{

template<class S, class D>
class DBScanGen{
  public:

/*
1.1 Constructor

If the constructor called, the complete db scan algorith is performed on the
stream referred by s. This means each tuple is assigned to a cluster id.

*/
    DBScanGen(Word s, ListExpr tupleResultType, double eps, int _minPts, 
              size_t _maxMem, int _attrPos, D _distfun):
     setOfObjects(s, tupleResultType, eps, _minPts, 
                 _maxMem, _attrPos, _distfun), 
     minPts(_minPts), clusterid(0){
       dbScan();
    }


/*
1.2 ~next~

Calling this function returns the next result tuple. If no more tuples are 
available, 0 is returned.

*/
    Tuple* next(){
       return setOfObjects.next(); 
    }

  private:
     S setOfObjects;
     int minPts;
     int clusterid;


/*
1.3 ~dbscan~

Main Algorithm

*/
  void dbScan(){



     GenericRelationIterator* it = setOfObjects.makeScan();

     Tuple* tuple;
     while((tuple=it->GetNextTuple())!=0){
        TupleId id = it->GetTupleId();
        if(!setOfObjects.getProcessed(id)){
            setOfObjects.setProcessed(id, true);
            list<TupleId>* n = setOfObjects.getNeighbors(id);
            if(n->size() < minPts){
                setOfObjects.setCluster(id,-2);
                delete n;              
            } else {
                clusterid++;
                list<TupleId>::iterator it2;
                for(it2=n->begin(); it2!=n->end() ; it2++){
                  if(setOfObjects.getCluster(*it2)<=0){
                     setOfObjects.setSeed(*it2,true);
                  }
                }
                expandCluster(id, n);
                delete n; 
            }
        }
        tuple->DeleteIfAllowed();
     }
     delete it;
     setOfObjects.initOutput();
  }

/*
1.4 ~expandCluster~

This expands a cluster starting at id having neighbors n.

*/

  void expandCluster(TupleId id, list<TupleId>* n){

     setOfObjects.setCluster(id,clusterid);


     while(!n->empty()){
       TupleId id2 = n->front();
       n->pop_front();
       setOfObjects.setSeed(id2,false);
       if(!setOfObjects.getProcessed(id2)){
          list<TupleId>* n2 = setOfObjects.getNeighbors(id2);
          setOfObjects.setProcessed(id2,true);
          if(n2->size()>= minPts){
              list<TupleId>::iterator it;
              for(it=n2->begin(); it!=n2->end() ; it++){
                if(!setOfObjects.isSeed(*it) ){
                   setOfObjects.setSeed(*it,true);
                   n->push_back(*it);
                }
              }              
          }
          delete n2;
       }
       if(setOfObjects.getCluster(id2) < 0){
          setOfObjects.setCluster(id2, clusterid);
       } 
     }
  }

};

}

#endif

