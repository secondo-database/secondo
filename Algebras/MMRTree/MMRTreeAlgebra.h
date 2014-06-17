
/*
----
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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


//[_] [\_]

*/

#ifndef MMRTREEALGEBRA_H
#define MMRTREEALGEBRA_H


#include <sstream>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "RectangleAlgebra.h"


#include "TupleStore1.h" 
#include "TupleStore2.h" 
#include "TupleStore3.h" 


extern NestedList* nl;
extern QueryProcessor* qp;





/*
1 MMRTreeAlgebra

This algebra provides some spatial join algorithms for small datasets.
It uses an rtree which ist hold completely in the main memory. 
The incoming tuples of the first stream are also hold in main memory in
some versions of this operators. Other operators restrict the memory usage 
for tuples but not for the tree.

1.1 Auxiliary Functions

~hMem~ 

This function formats  a byte value as a human readable value.

*/
string  hMem(size_t mem);


/*
1.2 TupleStore Definition


This typedef defines the tuplestore to use. They are several possibilities:

----

 TupleStore1: Uses a vector and after exhausting available memory a relation to 
 store tuples
 TupleStore2: Uses only a vector. All tuples are hold in main memory
 TupleStore3: Uses only a relation. All tuples are stored persistent.

----


*/
typedef TupleStore1 TupleStore;


/*
Auxiliary class ~RealJoinTreeLocalInfo~

The template parameter Tree determines the structure
used as index.


*/
template <class Tree, class Type1, class Type2, int dim1, int dim2, int minDim>
class RealJoinTreeLocalInfo{

  public:

/*
~Constructor~

The parameters are:

----
     _s1 : first stream
     _s2 : second stream
     _i1 : index of an attribute of type Type1 in _s1
     _i2 : index of an attribute of type Type2 in _s2
     _tt : list describing the result tuple type
     _maxMem : maximum cache size for tuples of _s1 in kB
----

The cinstructor is blocking. This means, the first stream is 
processed completely. Each tuple is inserted into a tuple store 
and into an Tree structure.

*/ 

     RealJoinTreeLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, size_t maxMem,
                             bool _rect1 = true, bool _rect2=true):
         ind(), s2(_s2),  i2(_i2), tt(new TupleType(_tt)), lastRes(), 
         currentTuple(0), tb(0) {
         
         Stream<Tuple> s1(_s1);
         init(s1,_i1,maxMem);
     }



/*
~Constructor~

The parameters are:

----
     _s1 : first stream
     _s2 : second stream
     _i1 : index of a rect attribute in _s1
     _i2 : index of a rect attribute in _s2
     _tt : list describing the result tuple type
     min : minimum number of entries within a node of the index
     max : maximum number of entries within a node of the index
     _maxMem : maximum cache size for tuples of _s1 in kB

----



*/

     RealJoinTreeLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, int min, int max,
                             size_t maxMem):
         ind(min,max), s2(_s2),  i2(_i2), tt(new TupleType(_tt)), 
         lastRes(), currentTuple(0), tb(0) {
         Stream<Tuple> s1(_s1);
         init(s1,_i1,maxMem);

     }

/*
~Destructor~

*/

     ~RealJoinTreeLocalInfo(){
         s2.close();
         
         tt->DeleteIfAllowed();
         if(tb){
           delete tb;
         }
         if(currentTuple){
            currentTuple->DeleteIfAllowed();
         }
         
      }

/*
~nextTuple~

Returns the next result tuple or 0 if no more tuples are available.

*/

      Tuple* nextTuple(){
        if(lastRes.empty() && (currentTuple!=0)){
           currentTuple->DeleteIfAllowed();
           currentTuple = 0;
         }        

         while(lastRes.empty()){
            Tuple* t = s2.request();
            if(t==0){
               return 0;
            }
            Rectangle<dim2> 
                  r(((Type2*) t->GetAttribute(i2))->BoundingBox());
            //if(dim2==minDim){
            //   ind.findSimple(r, lastRes); 
            //} else {
               ind.findSimple(r. template project<minDim>(), lastRes);
            //}
            if(lastRes.empty()){
                t->DeleteIfAllowed();
            } else {
               currentTuple = t;
            }
         }

         pair<Rectangle<minDim>,TupleId> p1 = lastRes.back();
         lastRes.pop_back();
         Tuple*result = new Tuple(tt);
         Tuple* t1 = tb->GetTuple(p1.second);
         Concat(t1, currentTuple, result);
         t1->DeleteIfAllowed();         
         return result;
      }

   private:
      Tree ind;
      Stream<Tuple> s2;
      int i2;
      TupleType* tt;
      vector<pair<Rectangle<minDim>,TupleId> > lastRes;
      Tuple* currentTuple;
      TupleStore* tb;


     void init(Stream<Tuple>& s1, int _i1, size_t maxMem){
         // build the index from the first stream
         s1.open();
         Tuple* t = s1.request();
         if(t){
           tb = new TupleStore(maxMem); 
         } else {
           tb = 0;
         }
         while(t){
            TupleId id = tb->AppendTuple(t);
            Rectangle<dim1> r = ((Type1*)
                                 t->GetAttribute(_i1))->BoundingBox();
            //if(dim1==minDim){
            //   ind.insert(r, id);
            //} else {
               ind.insert(r. template project<minDim>(),id);
            //}
            t->DeleteIfAllowed(); 
            t = s1.request();
         }
         s1.close();
         s2.open();
         currentTuple = 0;
    }
};


template <class Tree, class Type1, class Type2, int dim1, int dim2, int minDim >
class RealJoinTreeVecLocalInfo{

  public:

/*
~Constructor~

The parameters are:

----
     _s1 : first stream
     _s2 : second stream
     _i1 : index of an attribute of type Type1 in _s1
     _i2 : index of an attribute of type Type2 in _s2
     _tt : list describing the result tuple type
     _maxMem : maximum cache size for tuples of _s1 in kB

----

The cinstructor is blocking. This means, the first stream is 
processed completely. Each tuple is inserted into a vector 
and into an Tree structure.

*/ 

     RealJoinTreeVecLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, size_t maxMem):
         ind(), s2(_s2),  i2(_i2) {
         
         tt = new TupleType(_tt);
         Stream<Tuple> s1(_s1);
         init(s1,_i1,maxMem);
     }



/*
~Constructor~

The parameters are:

----
     _s1 : first stream
     _s2 : second stream
     _i1 : index of a rect attribute in _s1
     _i2 : index of a rect attribute in _s2
     _tt : list describing the result tuple type
     min : minimum number of entries within a node of the index
     max : maximum number of entries within a node of the index
     _maxMem : maximum cache size for tuples of _s1 in kB

----



*/

     RealJoinTreeVecLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, int min, int max,
                             size_t maxMem):
         ind(min,max), s2(_s2),  i2(_i2) {
         tt = new TupleType(_tt);
         Stream<Tuple> s1(_s1);

         init(s1,_i1,maxMem);
     }





/*
~Destructor~

*/

     ~RealJoinTreeVecLocalInfo(){
         s2.close();
         
         tt->DeleteIfAllowed();
         vector<Tuple*>::iterator it;
         for(it=vec.begin(); it!=vec.end(); it++){
            (*it)->DeleteIfAllowed();
         }
         vec.clear();
         if(currentTuple){
            currentTuple->DeleteIfAllowed();
         }
         
      }

/*
~nextTuple~

Returns the next result tuple or 0 if no more tuples are available.

*/

      Tuple* nextTuple(){
        if(lastRes.empty() && (currentTuple!=0)){
           currentTuple->DeleteIfAllowed();
           currentTuple = 0;
         }        

         while(lastRes.empty()){
            Tuple* t = s2.request();
            if(t==0){
               return 0;
            }
            Rectangle<dim2> r = ((Type2*) 
                                 t->GetAttribute(i2))->BoundingBox();
            //if(dim2==minDim){
            //    ind.findSimple(r, lastRes); 
            //} else {
                ind.findSimple(r. template project<minDim>(), lastRes);
            //}
            if(lastRes.empty()){
                t->DeleteIfAllowed();
            } else {
               currentTuple = t;
            }
         }

         pair<Rectangle<minDim>,TupleId> p1 = lastRes.back();
         lastRes.pop_back();
         Tuple*result = new Tuple(tt);
         Tuple* t1 = vec[p1.second];
         Concat(t1, currentTuple, result);
         return result;
      }

   private:
      Tree ind;
      Stream<Tuple> s2;
      int i2;
      TupleType* tt;
      vector<pair<Rectangle<minDim>,TupleId> > lastRes;
      Tuple* currentTuple;
      vector<Tuple*> vec;


     void init(Stream<Tuple>& s1, int _i1, size_t maxMem){
         // build the index from the first stream
         s1.open();
         Tuple* t = s1.request();
         while(t){
            TupleId id = vec.size();
            vec.push_back(t);
            Rectangle<dim1> r = ((Type1*)
                                 t->GetAttribute(_i1))->BoundingBox();
            //if(dim1==minDim){
            //    ind.insert(r, id);
            //} else {
                ind.insert(r. template project<minDim>(), id);
            //}
            t = s1.request();
         }
         s1.close();
         s2.open();
         currentTuple = 0;
     }
};



/*
Selection function

*/
int realJoinSelect(ListExpr args);



/*
1.4.2 Value Mapping

*/

template<class JLI>
int joinRTreeVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   JLI* li = static_cast<JLI*>(local.addr);

   switch(message){
        case OPEN: {
             if(li){
               delete li;
               local.addr = 0;
               li = 0;
             }
             CcInt* MaxMem = (CcInt*) args[6].addr;
             size_t maxMem = (qp->GetMemorySize(s) * 1024); // in kB
             if(MaxMem->IsDefined() && MaxMem->GetValue() > 0){
                maxMem = MaxMem->GetValue();
             }
             CcInt* Min = (CcInt*)(args[4].addr);
             CcInt* Max = (CcInt*)(args[5].addr);
             if(!Min->IsDefined() || !Max->IsDefined()){
                return 0;
             }
             int min = Min->GetValue();
             int max = Max->GetValue();
             if(min<1 || max< (2*min)){
                  return 0;
             }
             int i1 = ((CcInt*)(args[7].addr))->GetValue();
             int i2 = ((CcInt*)(args[8].addr))->GetValue();

             local.addr = new JLI(args[0], args[1],i1,i2,
                                  nl->Second(GetTupleResultType(s)), 
                                  min, max, maxMem);
             return 0; 
        }
        case REQUEST: {
             if(!li){
                return CANCEL;
             }
             result.addr = li->nextTuple();
             return result.addr?YIELD:CANCEL;
        }
        case CLOSE:{
             if(li){
               delete li;
               local.addr = 0;
             }
             return 0;
        }

   }
   return -1;
}





 #endif


