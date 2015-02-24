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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef OPTICS_SET_OF_OBJECTS_H
#define OPTICS_SET_OF_OBJECTS_H


#include "MMRTree.h"
#include "RelationAlgebra.h"
#include "StandardTypes.h"

/*
1 SetOfObjectsR

This implements the management of objects for optics indexed by an R-tree.
The template parameter are ~D~, a distance function and ~dim~ the 
spatial dimension of the objects.

*/

template<class D, unsigned int dim>
class SetOfObjectsR{

  public:

/*
1.1 Constructor

*/
    SetOfObjectsR( Stream<Tuple>& instream, int _attrPos, D& _distfun, 
                   double _eps, size_t maxMem, ListExpr _resultType):
      buffer(0), attrPos(_attrPos),tree(new mmrtree::RtreeT<dim, TupleId>(4,8)),
      distfun(_distfun), eps(_eps){
      instream.open();
      TupleType* tt = new TupleType(_resultType);
      buffer = new TupleBuffer(maxMem);

      Tuple* inTuple;
      bool first = true;
      while((inTuple = instream.request())){
          Tuple *newTuple = new Tuple(tt);
          //Copy data from given tuple to the new tuple
          int attrCnt = inTuple->GetNoAttributes();
          if(first){
             corePos = attrCnt;
             reachPos = attrCnt + 1;
             procPos = attrCnt + 2;
             epsPos = attrCnt + 3;
             first = false;
           }
          for( int i = 0; i < attrCnt; i++ )  {
             newTuple->CopyAttribute( i, inTuple, i);
           }
           //Initialize the result tuple with default values
           newTuple->PutAttribute( corePos, new CcReal(-1.0));   
           newTuple->PutAttribute( reachPos, new CcReal(-1.0)); 
           newTuple->PutAttribute( procPos, new CcBool(true,false));
           newTuple->PutAttribute( epsPos, new CcReal(true,eps)); // processed
           buffer->AppendTuple(newTuple);
           inTuple->DeleteIfAllowed();
           newTuple->DeleteIfAllowed();
       }
       instream.close();
       tt->DeleteIfAllowed();
 
       GenericRelationIterator* relIt = buffer->MakeScan();
       Tuple* tuple;
       while( (tuple = relIt->GetNextTuple()) ){
          TupleId id = relIt->GetTupleId();
          Rectangle<dim>* rect = (Rectangle<dim>*) tuple->GetAttribute(attrPos);
          if(rect->IsDefined()){
             tree->insert(*rect, id);
          }
          tuple->DeleteIfAllowed();
       }
       delete relIt;
   } 

/*
1.2 Destructor

*/   
   ~SetOfObjectsR(){
      if(buffer){
          delete buffer;
      }
      if(tree){
         delete tree;
      }
   }


/*
1.3 Iterator

*/
   inline GenericRelationIterator* MakeScan(){
      return buffer->MakeScan();
   }

/*
1.4 Random Access

*/
   inline Tuple* GetTuple( TupleId id){
      return buffer->GetTuple(id,true);
   }

/*
1.5 Retrieving neighbors of a tuple.

*/
   list<TupleId>* getNeighbors( TupleId id){
      Tuple* tuple = buffer->GetTuple(id,false);
      Rectangle<dim>* rect = ((Rectangle<dim>*)
           tuple->GetAttribute(attrPos)->Clone());
      rect->Extend(eps);
      set<TupleId> candidates;
      tree->findAll(*rect , candidates);
      list<TupleId>* res = new list<TupleId>();
      set<TupleId>::iterator it;
 
      for(it = candidates.begin(); it!=candidates.end(); it++){
           if(id != *it){
              double dist = distance(tuple, *it);
              if(dist <= eps){
                 res->push_back(*it);
              }
           }
      }
      delete rect;
      tuple->DeleteIfAllowed();
      return res;
    }

/*
1.6 Some getters

*/
    
    inline  double getReachDist(TupleId id){
      Tuple* t = buffer->GetTuple(id,true);
      double res =  ((CcReal*)t->GetAttribute(reachPos))->GetValue();
      t->DeleteIfAllowed();
      return res;
    }
    
    inline  double getCoreDist(TupleId id){
      Tuple* t = buffer->GetTuple(id,true);
      double res =  ((CcReal*)t->GetAttribute(corePos))->GetValue();
      t->DeleteIfAllowed();
      return res;
    }
    
    inline  bool getProcessed(TupleId id){
      Tuple* t = buffer->GetTuple(id,true);
      bool  res =  ((CcBool*)t->GetAttribute(procPos))->GetValue();
      t->DeleteIfAllowed();
      return res;
    }

    inline double distance(Tuple* t1, Tuple* t2){
      Rectangle<dim>* a1 = (Rectangle<dim>*) t1->GetAttribute(attrPos);
      Rectangle<dim>* a2 = (Rectangle<dim>*) t2->GetAttribute(attrPos);
      return distfun(a1,a2); 
    }
     
    double distance(Tuple* t, TupleId id){
        Tuple* t2 = buffer->GetTuple(id,false);
        double res =  distfun( (Rectangle<dim>*) t->GetAttribute(attrPos),
                               (Rectangle<dim>*) t2->GetAttribute(attrPos));
        t2->DeleteIfAllowed();
        return res;
     }


/*
1.7 Some setters

*/

    void updateProcessed(TupleId id, bool value){
       Tuple* t = buffer->GetTuple(id,true);
       t->PutAttribute(procPos, new CcBool(true,value));
       t->DeleteIfAllowed();
    }
    
   void updateReachability(TupleId id, double value){
       Tuple* t = buffer->GetTuple(id,true);
       t->PutAttribute(reachPos, new CcReal(true,value));
       t->DeleteIfAllowed();
    }

   void updateCoreDistance(TupleId id, double value){
       Tuple* t = buffer->GetTuple(id,true);
       t->PutAttribute(corePos, new CcReal(true,value));
       t->DeleteIfAllowed();
    }

/*
1.8 finish

This function must be called after the optics computation is 
finsihed.

*/
    void finish(){
       delete tree;
       tree = 0;
       it = result.begin();
    }


/*
1.9 next

Returns the next Tuple of the result. Note that the finish function must be
called before this function returns meaningful results.

*/
    Tuple* next(){
      if(it==result.end()){
        return 0;
      }
      Tuple* t = buffer->GetTuple(*it,false);
      it++;
      return t;
    }

    void append(TupleId id){
       result.push_back(id);
    }


  private:

/*
1.10 Members

*/
     TupleBuffer* buffer;  // storage for tuples
     int attrPos;          // position of the attribute selected for clustering
     mmrtree::RtreeT<dim, TupleId>* tree; // the index
     D distfun;  // distance function
     double eps; // the epsilon value
     int corePos; // position of the core distance in result tuple
     int reachPos; // position of the reachability distance
     int procPos;  // position of the processed flag        
     int epsPos;   // position of the chosen epsilon value
     vector<TupleId> result; // ordered result
     vector<TupleId>::iterator it;  // result iterator


};

#endif

