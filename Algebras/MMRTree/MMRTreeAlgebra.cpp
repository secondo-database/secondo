

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


#include <limits>

#include "MMRTreeAlgebra.h"
#include "MMRTree.h"

//#include "MMRStarTree.h"

#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "FTextAlgebra.h"
#include "StringUtils.h"

#include "Progress.h"
#include "../CostEstimation/MMRTreeAlgebraCostEstimation.h"

#include "Point.h"
#include "MMMTree.h"
#include "TemporalAlgebra.h"

/*
1.1 Auxiliary Functions

~hMem~ 

This function formats  a byte value as a human readable value.

*/
string  hMem(size_t mem){

   stringstream ss; 
   if(mem >= (1u << 30)){
      ss  << (mem / (1u << 30) ) << "GB";
   } else if(mem >= (1u << 20)){
      ss << (mem / (1u << 20)) << "MB" ;
   } else if(mem > (1u << 10)){
      ss << (mem / (1u << 10)) << "kB" ;
   } else  {
      ss << mem << "b";
   }
   return ss.str();
}


/*
Selection function

*/
int realJoinSelect(ListExpr args){

  ListExpr attrList1 = nl->Second(nl->Second(nl->First(args)));
  string name1 = nl->SymbolValue(nl->Third(args));
  ListExpr type;

  int index = listutils::findAttribute(attrList1,name1,type);

  assert(index>0);
  int num1;

  if(listutils::isKind(type,Kind::SPATIAL2D())){
     num1 = 0;
  } else  if(listutils::isKind(type, Kind::SPATIAL3D())){
     num1 = 1;
  } else if(UPoint::checkType(type)){
     num1 = 2;
  } else if(MPoint::checkType(type)){
     num1 = 3;
  } else {
   assert(false);
  }

  ListExpr attrList2 = nl->Second(nl->Second(nl->Second(args)));
  string name2 = nl->SymbolValue(nl->Fourth(args));
  index = listutils::findAttribute(attrList2,name2,type);
  assert(index>0);
  int num2;

  if(listutils::isKind(type,Kind::SPATIAL2D())){
     num2 = 0;
  } else  if(listutils::isKind(type, Kind::SPATIAL3D())){
     num2 = 1;
  } else if(UPoint::checkType(type)){
     num2 = 2;
  } else if(MPoint::checkType(type)){
     num2 = 3;
  } else {
   assert(false);
  }


  return 4 * num1 + num2;

}

/*
1.5 Operator ~realJoinMMRTRee~

1.5.1 Type mapping

Signature is: stream(tuple(A)) x stream(tuple(B)) x a[_]i x b[_]j x 
              int x int [ x int ] -> stream(tuple(A o B))

Meaning is : stream1 x stream2 x attrname1 x attrname2 x min x max x maxMem
where


----
  stream1   : first input stream
  stream2   : second input stream
  attrname1 : attribute name for an attribute of spatial type  in stream1
  attrname2 : attribute name for an attribute of spatial type  in stream2
  min       : minimum number of entries within a node of the used rtree
  max       : maximum number of entries within a node of the used rtree
 [maxMem]   : maximum memory available for storing tuples

----

*/
ListExpr realJoinMMRTreeTM(ListExpr args){



  string err = "stream(tuple(A)) x stream(tuple(B)) x"
               " a_i x b_i x [int x int [x int]] expected";
   int len = nl->ListLength(args);
   if((len!=4) && (len!=6) && (len!=7)){
     return listutils::typeError(err);
   }

   if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)) ||
      !listutils::isSymbol(nl->Third(args)) ||
      !listutils::isSymbol(nl->Fourth(args)) ) {
     return listutils::typeError(err);
   }
   if( (len>4) && 
       (!CcInt::checkType(nl->Fifth(args)) ||
        !CcInt::checkType(nl->Sixth(args)))){
      return listutils::typeError(err + 
                                " ( 5th or 6th element not of type int)");
   }


   if( (len==7) && ! CcInt::checkType(nl->Sixth(nl->Rest((args))))){
     return listutils::typeError(err);
   }

   ListExpr attrList1 = nl->Second(nl->Second(nl->First(args)));
   ListExpr attrList2 = nl->Second(nl->Second(nl->Second(args)));
   string name1 = nl->SymbolValue(nl->Third(args));
   string name2 = nl->SymbolValue(nl->Fourth(args));

   ListExpr type1;
   ListExpr type2;

   int index1 = listutils::findAttribute(attrList1, name1, type1);
   if(index1 == 0){
     return listutils::typeError("Attribute " + name1 + 
                                 " not present in first stream");
   }

   
   int index2 = listutils::findAttribute(attrList2, name2, type2);
   if(index2 == 0){
     return listutils::typeError("Attribute " + name2 + 
                                 " not present in second stream");
   }

   // check for spatial attribute
   if(!listutils::isKind(type1,Kind::SPATIAL2D()) &&
      !listutils::isKind(type1,Kind::SPATIAL3D()) &&
      !UPoint::checkType(type1) &&
      !MPoint::checkType(type1) ){
     string t = " (type is " + nl->ToString(type1)+ ")";  
     return listutils::typeError("Attribute " + name1 + 
                                 " is not in Kind Spatial2D or Spatial3D"
                                 + t);
   }
   if(!listutils::isKind(type2,Kind::SPATIAL2D()) &&
      !listutils::isKind(type2,Kind::SPATIAL3D()) &&
      !UPoint::checkType(type2) &&
      !MPoint::checkType(type2) ){
     string t = " (type is " + nl->ToString(type2)+ ")";  
     return listutils::typeError("Attribute " + name2 + 
                                 " is not in Kind Spatial2D or Spatial3D"
                                 + t);
   }
 
  
   bool rect1 =    Rectangle<2>::checkType(type1) 
                || Rectangle<3>::checkType(type1);
   bool rect2 =    Rectangle<2>::checkType(type2) 
                || Rectangle<3>::checkType(type2);

 
   ListExpr attrList = listutils::concat(attrList1, attrList2);

   if(!listutils::isAttrList(attrList)){
     return listutils::typeError("name conflicts in tuple streams");
   }

   ListExpr resList = nl->TwoElemList( 
                           nl->SymbolAtom(Stream<Tuple>::BasicType()),
                           nl->TwoElemList(
                               nl->SymbolAtom(Tuple::BasicType()),
                               attrList));
   ListExpr appendList;
   if(len==7){
      appendList =   nl->FourElemList(nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1),
                                     nl->BoolAtom(rect1),
                                     nl->BoolAtom(rect2));
   } else if(len==6) {
      appendList =   nl->FiveElemList(
                                     nl->IntAtom(-1),
                                     nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1),
                                     nl->BoolAtom(type1),
                                     nl->BoolAtom(type2));
   } else { // len == 4
      // use (4,8) as default parameter of the rtree
      appendList = listutils::xElemList(7,
                                     nl->IntAtom(4),
                                     nl->IntAtom(8),
                                     nl->IntAtom(-1),
                                     nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1),
                                     nl->BoolAtom(type1),
                                     nl->BoolAtom(type2));
   }

   return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   appendList,
                   resList);

}



/*
1.4.3 Specification

*/
const string realJoinMMRTreeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> stream(tuple(A)) x stream(tuple(B)) x a_i x b_j x int"
    " x int [ x int]-> stream(tuple(X o Y)) </text---> "
    "<text>streamA streamB realJoinMMRTree[attrnameA, attrnameB, min,"
    " max, maxMem]"
    "  </text--->"
    "<text>Performes a spatial join on two streams. "
    "The attributes a_i and b_j must"
    " be of type rect or rect3 in A and B, respectively. "
    "The result is a stream of "
    "tuples with intersecting rectangles build as concatenation"
    " of the source tuples. min and max define the range for the number of"
    " entries within the nodes of the used rtree. maxMem is the maximum cache"
    " size of the tuple store for tuples coming from streamA."
    " If maxMem is omitted, "
    " The default value MaxMemPerOperator (see SecondoConfig.ini) is used."
    " </text--->"
    "<text>query strassen feed extend[B : bbox(.geoData] strassen "
    "feed extend[B : bbox(.geoData)] {a} realJoinMMRTree[B,B_a,8,16, 1024]"
    " count "
    " </text--->"
         ") )";



/*
1.4.4 Operator Instance

*/
ValueMapping realJoinMMRTreeVM[] = {
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, 
                StandardSpatialAttribute<2>,2,2,2 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, 
                StandardSpatialAttribute<3>,2,3,2 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, UPoint,2,3,2 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, MPoint,2,3,2 > >,


    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<3>, 
                StandardSpatialAttribute<2>,3,2,2 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                StandardSpatialAttribute<3>, 
                StandardSpatialAttribute<3>,3,3,3 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                StandardSpatialAttribute<3>, UPoint,3,3,3 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                StandardSpatialAttribute<3>, MPoint,3,3,3 > >,

    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,
                UPoint, StandardSpatialAttribute<2>,3,2,2 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                UPoint, StandardSpatialAttribute<3>,3,3,3 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                UPoint, UPoint,3,3,3 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                UPoint, MPoint,3,3,3 > >,

    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,
                MPoint, StandardSpatialAttribute<2>,3,2,2 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                MPoint, StandardSpatialAttribute<3>,3,3,3 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                MPoint, UPoint,3,3,3 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,
                MPoint, MPoint,3,3,3 > >,

  };



Operator realJoinMMRTree(
  "realJoinMMRTree",
  realJoinMMRTreeSpec,
  16,
  realJoinMMRTreeVM,
  realJoinSelect,
  realJoinMMRTreeTM 
 );



ValueMapping realJoinMMRTreeVecVM[] = {
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, 
                StandardSpatialAttribute<2>,2,2,2> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, 
                StandardSpatialAttribute<3>,2,3,2> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, UPoint,2,3,2> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<2>, MPoint,2,3,2> >,

    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<2, TupleId>,
                StandardSpatialAttribute<3>, 
                StandardSpatialAttribute<2>,3,2,2> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<3, TupleId>,
                StandardSpatialAttribute<3>, 
                StandardSpatialAttribute<3>,3,3,3> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<3, TupleId>,
                StandardSpatialAttribute<3>, UPoint,3,3,3> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<3, TupleId>,
                StandardSpatialAttribute<3>, MPoint,3,3,3> >,
    
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<2, TupleId>,
                UPoint, StandardSpatialAttribute<2>,3,2,2> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<3, TupleId>,
                UPoint, StandardSpatialAttribute<3>,3,3,3> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<3, TupleId>,
                UPoint, UPoint,3,3,3> >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<3, TupleId>,
                UPoint, MPoint,3,3,3> >,

  };


Operator realJoinMMRTreeVec(
  "realJoinMMRTreeVec",
  realJoinMMRTreeSpec,
  16,
  realJoinMMRTreeVecVM,
  realJoinSelect,
  realJoinMMRTreeTM 
 );


/*
1.5 insertMMRTree

This operator can be used to check the performance of building an rtree
in main memory.

1.5.1 Type Mapping

Signature is:  stream(rectangle) x int x int

The last two arguments denote the range of number of entries within the
 used rtree.

*/

ListExpr insertMMRTreeTM(ListExpr args){
   string err = "stream(rectangle) x int x int expected";
   if(!nl->HasLength(args,3)){
       return listutils::typeError(err);
   }

   if(!Stream<Rectangle<2> >::checkType(nl->First(args))){
       return listutils::typeError(err);
   }
   if(!CcInt::checkType(nl->Second(args))){
       return listutils::typeError(err);
   }
   if(!CcInt::checkType(nl->Third(args))){
       return listutils::typeError(err);
   }
   return nl->First(args);
};


/*
1.5.2 Value Mapping

*/
int insertMMRTreeVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{

  static long  c = 0;

  mmrtree::Rtree<2>* mmrtree1  = static_cast<mmrtree::Rtree<2>*>(local.addr);

  switch(message){
     case OPEN: { qp->Open(args[0].addr); 
                  if(mmrtree1){
                    delete mmrtree1;
                    local.addr = 0;
                  }
                  CcInt* min1 = static_cast<CcInt*>(args[1].addr);
                  CcInt* max1 = static_cast<CcInt*>(args[2].addr);
                  if(!min1->IsDefined() || !max1->IsDefined()){
                      return 0;
                  }
                  int min2 = min1->GetValue();
                  int max2 = max1->GetValue();
                  if(min2 <1 || max2 < 2* min2){
                    return 0;
                  }
                  local.setAddr(new mmrtree::Rtree<2>(min2,max2));
                  return 0;
                   
                }
     case REQUEST: {
                 if(!mmrtree1){
                    return CANCEL;
                  }  
                  Word w;
                  qp->Request(args[0].addr, w);
                  if(qp->Received(args[0].addr)){
                     result = w;
                     Rectangle<2>* r = (Rectangle<2>*) w.addr;
                     if(mmrtree1){
                       mmrtree1->insert(*r,c++);
                     } 
                     return YIELD;
                  } else {
                     return CANCEL;
                  }
               }

     case CLOSE: {
                    qp->Close(args[0].addr);
                    if(mmrtree1){
                      delete mmrtree1;
                    }
                    local.setAddr(0);
                    return 0;
               }
  };
  assert(false);
  return -1;
}

/*
1.5.3 Specification

*/

const string insertMMRTreeSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(rectangle) x int x int -> stream(rectangle) </text---> "
       "<text> _ insertMMRTRee [min , max] </text--->"
       "<text>Inserts all the rectangles within the "
       "stream into a main memory based rtree. [min, max] is the allowed range"
       " for the numbers of entries within a node of the rtree. min must be"
       " greater than 0 and max must be at least two times min. "
       "  </text--->"
       "<text>query strassen feed extend[B : bbox(.geoData)] "
       "projecttransformstream[B] insertMMRTRee[4,8]  count  </text--->"
             ") )";


/*
1.5.4 Operator instance

*/

Operator insertMMRTree (
  "insertMMRTree",
  insertMMRTreeSpec,
  insertMMRTreeVM,
  Operator::SimpleSelect,
 insertMMRTreeTM);


/*
1.6 Operator ~statMMRTree~

Provides statistical information about an mmrtree build from a tuple stream.


1.6.1 Type Mapping

Signature is:  stream(tuple(A)) x a[_]i x int x int


*/
ListExpr statMMRTreeTM(ListExpr args){
  string err = "stream(Tuple(A)) x a_i x int x int expected";
  if(!nl->HasLength(args,4)){
     return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args)) ||
     !listutils::isSymbol(nl->Second(args)) ||
     !CcInt::checkType(nl->Third(args)) ||
     !CcInt::checkType(nl->Fourth(args))){
     return listutils::typeError(err);
  }
  string name = nl->SymbolValue(nl->Second(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type;
  int index = listutils::findAttribute(attrList, name, type);
  if(index==0){
     return listutils::typeError("Attribute " + name + " unknown in tuple");
  }
  if(!Rectangle<2>::checkType(type) && !Rectangle<3>::checkType(type)){
     return listutils::typeError("Attribute " + name + " not of type" +
                                 Rectangle<2>::BasicType() + " or " + 
                                 Rectangle<3>::BasicType());
  }
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList(nl->IntAtom(index-1)),
           nl->SymbolAtom(FText::BasicType()));
}

/*
1.6.2 Value Mapping

*/
template <int dim>
int statMMRTreeVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{  
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   CcInt* Min = (CcInt*) args[2].addr;
   CcInt* Max = (CcInt*) args[3].addr;
   if(!Min->IsDefined() || !Max->IsDefined()){
      res->Set(true,"At least one of the range value is undefined");
      return 0;
   } 
   int min = Min->GetValue();
   int max = Max->GetValue();
   if(min<1 || max < min*2){
      res->Set(true, "Invalid range, min must be greater "
                     "than 0, and max >= 2*min");
      return 0;
   }
   size_t tuples = 0;
   Stream<Tuple> stream(args[0]);
   stream.open();
   int index = ((CcInt*)args[4].addr)->GetValue();
   mmrtree::RtreeT<dim,TupleId> tree(min,max);
   Tuple* t = stream.request();
   while(t){
      Rectangle<dim>* r = (Rectangle<dim>*) t->GetAttribute(index);
      tree.insert(*r, t->GetTupleId());
      tuples += t->GetMemSize();
      t->DeleteIfAllowed();
      t = stream.request();
   }
   stream.close();
   stringstream ss;
   tree.printStats(ss);
   ss << " Mem used by tuples = " << tuples  
      << "(" << hMem(tuples) << ")" << endl;
   res->Set(true, ss.str());
   return 0; 
}

/*
1.6.3 Specification

*/
const string statMMRTreeSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(Tuple(A)) x a_i  x int x int -> text </text---> "
       "<text>_ statMMRTRee [ attrname, min , max] </text--->"
       "<text>Inserts all the rectangles within the "
       "stream (pointered by a_i) into a main memory based rtree ."
       " Statistical information about this tree is the result of this "
       "operator</text--->"
       "<text>query strassen feed extend[B : bbox(.geoData)] "
       " statMMRTRee[B,4,8]    </text--->"
             ") )";

/*
1.6.4 Operator instance

*/

int statMMRTreeSelect(ListExpr args){
   ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
   string name = nl->SymbolValue(nl->Second(args));
   ListExpr type;
   int index = listutils::findAttribute(attrList,name,type);
   assert(index >0);
   if(Rectangle<2>::checkType(type)){
      return 0;
   } else if(Rectangle<3>::checkType(type)){
      return 1;
   }
   return -1;
}

ValueMapping statMMRTreevm[] = {
    statMMRTreeVM<2>,
    statMMRTreeVM<3>
};




Operator statMMRTree (
  "statMMRTree",
  statMMRTreeSpec,
  2, 
  statMMRTreevm,
  statMMRTreeSelect,
  statMMRTreeTM);



/*
1.4 Operator joinMMRTreeIt

This operator does the same as the realjoinMMRTree operator. In contrast to 
that operator, here an iterator is used to get all results instead of a 
vector collecting all results within a single scan.

1.4.1 Type Mapping

Because the operator has the same  functionality as the realJoinMMRTree 
operator, we can reuse the type mapping of that operator.

*/

template<class Type1, class Type2, int dim1, int dim2, int minDim>
class joinMMRTreeItLocalInfo{
public:
   joinMMRTreeItLocalInfo(Word& stream1, Word& stream2, 
                          int min, int max, 
                          int maxMem, 
                          const int attrPos1, const int _attrPos2, 
                          ListExpr resultType) : 
                          s2(stream2),attrPos2(_attrPos2),
                          index(min,max), buffer(maxMem){
     Stream<Tuple> s1(stream1);
     s1.open();
     Tuple* tuple = s1.request();
     while(tuple!=0){
        TupleId id = buffer.AppendTuple(tuple);
        Rectangle<dim1> box = ((Type1*) 
                              tuple->GetAttribute(attrPos1))->BoundingBox();
        //if(dim1==minDim){
        //   index.insert(box,id);
        //} else {
           index.insert(box. template project<minDim>(), id);
        //}
        tuple->DeleteIfAllowed();
        tuple = s1.request();
     } 
     s1.close();
     tt = new TupleType(resultType);
     s2.open();
     currentTuple = s2.request(); 
     it = 0;
   }

   ~joinMMRTreeItLocalInfo(){
      tt->DeleteIfAllowed();
      if(it){
        delete it;
      }
      s2.close();
   }

   Tuple* next(){
     while(currentTuple!=0){
        if(it==0){
          Rectangle<dim2> r = ((Type2*)
                        currentTuple->GetAttribute(attrPos2))->BoundingBox();
          //if(dim2==minDim){
          //    it=index.find( r );
          //} else {
              it = index.find(r. template project<minDim>());
          //}
        }
        TupleId const* id = it->next();
        if(id){ // new result found
           Tuple* res = new Tuple(tt);
           Tuple* t1 = buffer.GetTuple(*id);
           Concat(t1,currentTuple,res);
           t1->DeleteIfAllowed();
           return res;
        } else { // iterator exhausted, try next currentTuple
          delete it;
          it = 0;
          currentTuple->DeleteIfAllowed();
          currentTuple = s2.request();
        }
     }
     return 0;
   }


private:
   Stream<Tuple> s2;
   int attrPos2;
   mmrtree::RtreeT<minDim, TupleId> index;
   TupleStore buffer;
   typename mmrtree::RtreeT<minDim, TupleId>::iterator* it;
   Tuple* currentTuple;
   TupleType* tt;
}; // class joinMMRTreeItLocalInfo


template<class Type1, class Type2, int dim1, int dim2, int minDim>
class joinMMRTreeItVecLocalInfo{
public:
   joinMMRTreeItVecLocalInfo(Word& stream1, Word& stream2, 
                          int min, int max, 
                          int maxMem, 
                          const int attrPos1, const int _attrPos2, 
                          ListExpr resultType) : 
                          s2(stream2),attrPos2(_attrPos2),
                          index(min,max), buffer(){
     Stream<Tuple> s1(stream1);
     s1.open();
     Tuple* tuple = s1.request();
     while(tuple!=0){
        TupleId id = (TupleId) buffer.size();
        buffer.push_back(tuple);
        Rectangle<dim1> box = ((Type1*)
                         tuple->GetAttribute(attrPos1))->BoundingBox(); 
        //if(dim1==minDim){
        //   index.insert(box,id);
        //} else {
           //assert(box.Proper());
           index.insert(box.template project<minDim>(), id);
        //}
        tuple = s1.request();
     } 
     s1.close();
     tt = new TupleType(resultType);
     s2.open();
     currentTuple = s2.request(); 
     it = 0;
   }

   ~joinMMRTreeItVecLocalInfo(){
      tt->DeleteIfAllowed();
      if(it){
        delete it;
      }
      s2.close();
      for(size_t i=0;i<buffer.size();i++){
         buffer[i]->DeleteIfAllowed();
      }
      buffer.clear();
   }

   Tuple* next(){
     while(currentTuple!=0){
        if(it==0){
           Rectangle<dim2> r = ((Type2*)
                     currentTuple->GetAttribute(attrPos2))->BoundingBox();
           //if(dim2==minDim){
           //    it = index.find(r);
           //} else {
               it = index.find(r. template project<minDim>());
           //}
        }
        TupleId const* id = it->next();
        if(id){ // new result found
           Tuple* res = new Tuple(tt);
           Tuple* t1 = buffer[*id];
           Concat(t1,currentTuple,res);
           return res;
        } else { // iterator exhausted, try next currentTuple
          delete it;
          it = 0;
          currentTuple->DeleteIfAllowed();
          currentTuple = s2.request();
        }
     }
     return 0;
   }


private:
   Stream<Tuple> s2;
   int attrPos2;
   mmrtree::RtreeT<minDim, TupleId> index;
   vector<Tuple*> buffer;
   typename mmrtree::RtreeT<minDim, TupleId>::iterator* it;
   Tuple* currentTuple;
   TupleType* tt;
}; // class joinMMRTreeItVecLocalInfo



template <class LocalInfo>
int joinMMRTreeItVM( Word* args, Word& result, int message,
                      Word& local, Supplier s ) {


   LocalInfo* li = (LocalInfo*) local.addr;
   switch (message){
     case OPEN : {
                   if(li){
                     delete li;
                     li = 0;
                     local.setAddr(0);
                   }
                   CcInt* Min = (CcInt*) args[4].addr;
                   CcInt* Max = (CcInt*) args[5].addr;
                   CcInt* MaxMem = (CcInt*) args[6].addr;
                   if(!Min->IsDefined() || !Max->IsDefined()){
                      return 0; // invalid option
                   }
                   int min = Min->GetIntval();
                   int max = Max->GetIntval();
                   if(min<2 || max < 2*min){
                       return 0;
                   } 
                   size_t maxMem = (qp->GetMemorySize(s) * 1024); // in kB
                   if(MaxMem->IsDefined() && MaxMem->GetValue() > 0){
                      maxMem = MaxMem->GetValue();
                   }
                   local.setAddr(new LocalInfo(args[0], args[1], min, 
                                               max, maxMem, 
                                      ((CcInt*)args[7].addr)->GetIntval(),
                                      ((CcInt*)args[8].addr)->GetIntval(),
                                       nl->Second(GetTupleResultType(s)) ));
                    return 0;
                 }

      case REQUEST : {
                    if(!li){
                      return CANCEL;
                    }
                    result.addr = li->next();
                    return result.addr?YIELD:CANCEL;

                 }
      case CLOSE :{
                   if(li){
                     delete li;
                     local.setAddr(0);
                   }
                   return 0;
                 }
   }
  return -1;
}


ValueMapping joinMMRTreeItvm[] = {
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<2>, 
                                           StandardSpatialAttribute<2>,2,2,2> >,

    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<2>, 
                                           StandardSpatialAttribute<3>,2,3,2> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<2>, 
                                           UPoint,2,3,2> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<2>, 
                                           MPoint,2,3,2> >,
    
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<3>, 
                                           StandardSpatialAttribute<2>,3,2,2> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<3>, 
                                           StandardSpatialAttribute<3>,3,3,3> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<3>, 
                                           UPoint,3,3,3> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<StandardSpatialAttribute<3>, 
                                           MPoint,3,3,3> >,
    
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<UPoint, 
                                           StandardSpatialAttribute<2>,3,2,2> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<UPoint, 
                                           StandardSpatialAttribute<3>,3,3,3> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<UPoint, 
                                           UPoint,3,3,3> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<UPoint, 
                                           MPoint,3,3,3> >,

    joinMMRTreeItVM<joinMMRTreeItLocalInfo<MPoint, 
                                           StandardSpatialAttribute<2>,3,2,2> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<MPoint, 
                                           StandardSpatialAttribute<3>,3,3,3> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<MPoint, 
                                           UPoint,3,3,3> >,
    joinMMRTreeItVM<joinMMRTreeItLocalInfo<MPoint, 
                                           MPoint,3,3,3> >
 
};




Operator joinMMRTreeIt (
  "joinMMRTreeIt",
  realJoinMMRTreeSpec,
  16, 
  joinMMRTreeItvm,
  realJoinSelect ,
  realJoinMMRTreeTM
  );


ValueMapping joinMMRTreeItVecvm[] = {
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<2>, 
                                              StandardSpatialAttribute<2>,
                                              2,2,2> >,

    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<2>, 
                                              StandardSpatialAttribute<3>,
                                              2,3,2> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<2>, 
                                              UPoint,
                                              2,3,2> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<2>, 
                                              MPoint,
                                              2,3,2> >,


    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<3>, 
                                              StandardSpatialAttribute<2>,
                                              3,2,2> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<3>, 
                                              StandardSpatialAttribute<3>,
                                              3,3,3> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<3>, 
                                              UPoint,
                                              3,3,3> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<StandardSpatialAttribute<3>, 
                                              MPoint,
                                              3,3,3> >,


    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<UPoint, 
                                              StandardSpatialAttribute<2>,
                                              3,2,2> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<UPoint, 
                                              StandardSpatialAttribute<3>,
                                              3,3,3> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<UPoint, 
                                              UPoint,
                                              3,3,3> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<UPoint, 
                                              MPoint,
                                              3,3,3> >,
    

    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<MPoint, 
                                              StandardSpatialAttribute<2>,
                                              3,2,2> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<MPoint, 
                                              StandardSpatialAttribute<3>,
                                              3,3,3> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<MPoint, 
                                              UPoint,
                                              3,3,3> >,
    joinMMRTreeItVM< joinMMRTreeItVecLocalInfo<MPoint, 
                                              MPoint,
                                              3,3,3> >

};




Operator joinMMRTreeItVec (
  "joinMMRTreeItVec",
  realJoinMMRTreeSpec,
  16, 
  joinMMRTreeItVecvm,
  realJoinSelect ,
  realJoinMMRTreeTM
  );



/*
1.5 itSpatialJoin

This version of the SpatialJoin operator uses only the memory 
available for that operator. From the first stream, it collects
all Tuples into a vector and creates an r-tree from the geometries
until the stream or the memory  is exhausted. 
Then, it starts to search on the r-tree using the tuples from the
second stream. If the first stream was not exhausted, all tuples 
of the second stream are collected within a tuple buffer.
While there are elements within the first stream, a partition of
is is used to create a new r-tree and the tuple buffer is scanned
for searching on the r-tree.


1.5.1 Type Mapping

The Type mapping is the same as for other join operations within this
algebra, so we can just use ~realJoinMMRTreeTM~ here.

*/

/*
1.5.2 LocalInfo

*/
template<class Type1, class Type2, int dim1, int dim2, int minDim>
class ItSpatialJoinInfo{
  public:
    
  ItSpatialJoinInfo(Word& _s1, Word& _s2, 
                    const int _min, const int _max,
                    size_t _maxMem, const int _a1,
                    const int _a2, ListExpr ttl, 
                    ItSpatialJoinCostEstimation* _costEstimation):
                    s1(_s1), s2(_s2), min(_min), max(_max),
                    a1(_a1), a2(_a2), costEstimation(_costEstimation)
  {

      s1.open();
      s2.open();
      Tuple* t1 = s1.request();
      tt = new TupleType(ttl);
      tuples2 = 0;
      treeIt = 0;
      bufferIt = 0;
      currentTuple2 =0;

      costEstimation -> setMinRtree(min);
      costEstimation -> setMaxRtree(max);

      // process the first Tuple
      if(t1){
         index = new mmrtree::RtreeT<minDim, TupleId>(min,max);
         int sizePerTuple =  sizeof(void*) + t1->GetMemSize();
         costEstimation -> setSizeOfTupleSt1(sizePerTuple);
         maxTuples = (_maxMem*1024) / sizePerTuple;
         if(maxTuples <= 10){
            maxTuples = 10; // force a minimum of ten tuples
         } else {
            size_t indexSize = index->guessSize(maxTuples, true);
            maxTuples = ((_maxMem*1024)-indexSize)  / sizePerTuple;
            if(maxTuples <=10){
                maxTuples = 10;
            }
         }
         TupleId id = (TupleId) tuples1.size();
         tuples1.push_back(t1);
         Rectangle<dim1> box = ((Type1*)
                         t1->GetAttribute(a1))->BoundingBox(); 
         //if(dim1==minDim){
         //  index->insert(box,id);
         //} else {
           index->insert(box. template project<minDim>(), id);
         //}
      } else {
        index = 0;
        return; 
      }
      costEstimation -> processedTupleInStream1();
      t1 = s1.request();
      int noTuples = 1;
      while( (t1!=0) && (noTuples < maxTuples -1)){
        TupleId id = (TupleId) tuples1.size();
        tuples1.push_back(t1);
        Rectangle<dim1> box = ((Type1*) t1->GetAttribute(a1))->BoundingBox(); 
        //if(dim1==minDim){
        //   index->insert(box,id);
        //} else {
           index->insert(box.template project<minDim>(), id);
        //}
        costEstimation -> processedTupleInStream1();
        t1 = s1.request();
        noTuples++;
      }

      // process the last tuple if present
      if(t1){
        finished = false;
        TupleId id = (TupleId) tuples1.size();
        tuples1.push_back(t1);
        Rectangle<dim1> box = ((Type1*) t1->GetAttribute(a1))->BoundingBox(); 
        //if(dim1==minDim){
        //   index->insert(box,id);
        //} else {
           index->insert(box.template project<minDim>(),id);
        //}
        
      } else {
        costEstimation -> setStream1Exhausted(true);
        finished = true; // stream 1 exhausted
      }
      scans = 1;
      costEstimation -> readPartitionDone();
   }
    
   ~ItSpatialJoinInfo(){
      s1.close();
      s2.close();
      tt->DeleteIfAllowed();
      for(unsigned int i=0;i< tuples1.size();i++){
         if(tuples1[i]){
           tuples1[i]->DeleteIfAllowed();
           tuples1[i] = 0;
         }
      }
      tuples1.clear();
      if(bufferIt){
         delete bufferIt;
      }
      if(tuples2){
         tuples2->DeleteAndTruncate();
         //delete tuples2;
      } 
      if(treeIt){
        delete treeIt;
      }
      if(index){
        delete index;
      }
    }



    Tuple* next(){
       if(!index){
         return 0;
       }
       while(true){ // use return for finishing this loop 
          if(!treeIt){ // try to create a new tree iterator
             createNewTreeIt();
             if(treeIt==0){
               costEstimation -> setStream2Exhausted(true);
               cout << "Join finished with " << scans 
                    << " scans for stream 2" << endl;
               cout << "there are " << maxTuples 
                    << " maximum stored tuples within the index" << endl;
               return 0;
             }
          }
          TupleId const* id = treeIt->next();    
          if(!id){ //treeIt exhausted
             delete treeIt;
             treeIt = 0;
             currentTuple2->DeleteIfAllowed();
             currentTuple2=0;
          } else { // new result found
            Tuple* res = new Tuple(tt);
            Tuple* t1 = tuples1[*id];
            Concat(t1,currentTuple2,res);
            return res;
          }        
       } 
    }


    

  private:
      Stream<Tuple> s1;
      Stream<Tuple> s2;
      int min;
      int max;
      int a1;
      int a2;
      TupleType* tt;
      int maxTuples;
      vector<Tuple*> tuples1;
      Relation* tuples2;
      bool finished;  // all tuples from stream1 read 
      typename mmrtree::RtreeT<minDim, TupleId>::iterator* treeIt; 
      Tuple* currentTuple2;  // tuple from stream 2
      GenericRelationIterator* bufferIt;
      mmrtree::RtreeT<minDim, TupleId>* index;
      int scans;
      ItSpatialJoinCostEstimation* costEstimation;

      void createNewTreeIt(){
         assert(!treeIt);
         if(!currentTuple2){
            getCurrentTuple(); 
         } 
         if(!currentTuple2){
             return;
         }
         Rectangle<dim2> r = ((Type2*)
                    currentTuple2->GetAttribute(a2))->BoundingBox();
         //if(dim2==minDim){
         //   treeIt = index->find(r);
         //} else {
            treeIt = index->find(r.template project<minDim>());
         //}
      }

      void getCurrentTuple(){
         if(currentTuple2){
           currentTuple2->DeleteIfAllowed();
           currentTuple2=0;
         }
         if(bufferIt){ // read from Buffer
            currentTuple2 = bufferIt->GetNextTuple();
            costEstimation -> incReadInIteration();
            if(!currentTuple2){
               if(finished){
                 return;
               } else { 
                 rebuildIndex();
                 resetBuffer();
               }
            } else {
              return; // next current tuple found
            }
         } else {
           currentTuple2 = s2.request();
           costEstimation -> processedTupleInStream2();
           costEstimation -> incReadInIteration();
           if(!currentTuple2){
              if(finished){
                return; 
              } else {
                rebuildIndex();
                resetBuffer();
              }
           } else {
             if(!finished){ //insert Tuple into buffer
               if(!tuples2){
                  tuples2 = new Relation(currentTuple2->GetTupleType(),true);
               }
               currentTuple2->PinAttributes();
               tuples2->AppendTupleNoLOBs(currentTuple2);
               costEstimation -> incTuplesInTupleFile();
             }
             return;
           }
         }
      }

      void rebuildIndex(){
         if(treeIt){
           delete treeIt;
           treeIt=0;
         }
         if(index){
           delete index;
         }
         for(unsigned int i=0;i<tuples1.size();i++){
            tuples1[i]->DeleteIfAllowed();
            tuples1[i]=0;
         }
         tuples1.clear();
         index = new mmrtree::RtreeT<minDim, TupleId>(min,max);
         Tuple* t1 = s1.request();
         costEstimation -> processedTupleInStream1();
         int noTuples = 0;
         while( (t1!=0) && (noTuples < maxTuples -1)){
             TupleId id = (TupleId) tuples1.size();
             tuples1.push_back(t1);
             Rectangle<dim1> box = ((Type1*)
                           t1->GetAttribute(a1))->BoundingBox(); 
             //if(dim1==minDim){
             //   index->insert(box,id);
             //} else {
                index->insert(box.template project<minDim>(),id);
             //}
             t1 = s1.request();
             costEstimation -> processedTupleInStream1();
             noTuples++;
         }
         // process the last tuple if present
        if(t1){
           finished = false;
           TupleId id = (TupleId) tuples1.size();
           tuples1.push_back(t1);
           Rectangle<dim1> box = ((Type1*)
                          t1->GetAttribute(a1))->BoundingBox(); 
           //if(dim1==minDim){
           //   index->insert(box,id);
           //} else {
              index-> insert(box. template project<minDim>(), id);
           //}
        } else {
          costEstimation -> setStream1Exhausted(true);
          finished = true; // stream 1 exhausted
        }

        costEstimation -> readPartitionDone();
      }

      void resetBuffer(){
         assert(tuples2);
         if(bufferIt){
           delete bufferIt;
         }
         costEstimation -> resetReadInIteration();
         costEstimation -> setTupleFileWritten(true);
         bufferIt = tuples2->MakeScan();
         assert(currentTuple2==0);
         currentTuple2 = bufferIt->GetNextTuple();
         costEstimation -> incReadInIteration();
         
         // Next iteration
         scans++;
         costEstimation -> setIteration(scans);
      }


};

/*
1.5.3 Value Mapping

*/
template <class Type1, class Type2, int dim1, int dim2, int minDim>
int itSpatialJoinVM( Word* args, Word& result, int message,
                      Word& local, Supplier s ) {


   ItSpatialJoinInfo<Type1, Type2, dim1, dim2, minDim>* li =
            (ItSpatialJoinInfo<Type1, Type2, dim1, dim2, minDim>*) local.addr;
   switch (message){
     case OPEN : {
                   if(li){
                     delete li;
                     li = 0;
                     local.setAddr(0);
                   }
                   CcInt* Min = (CcInt*) args[4].addr;
                   CcInt* Max = (CcInt*) args[5].addr;
                   CcInt* MaxMem = (CcInt*) args[6].addr;
                   if(!Min->IsDefined() || !Max->IsDefined()){
                      return 0; // invalid option
                   }
                   int min = Min->GetIntval();
                   int max = Max->GetIntval();
                   if(min<2 || max < 2*min){
                       return 0;
                   } 
                   size_t maxMem = (qp->GetMemorySize(s) * 1024); // in kB
                   if(MaxMem->IsDefined() && MaxMem->GetValue() > 0){
                      maxMem = MaxMem->GetValue();
                   }
                   
                  ItSpatialJoinCostEstimation* costEstimation =
                    (ItSpatialJoinCostEstimation*) qp->getCostEstimation(s);

                  costEstimation -> init(NULL, NULL);
 
                  local.setAddr(new 
                      ItSpatialJoinInfo<Type1,Type2,dim1,dim2, minDim>( 
                                      args[0], args[1],
                                       min, max, maxMem, 
                                      ((CcInt*)args[7].addr)->GetIntval(),
                                      ((CcInt*)args[8].addr)->GetIntval(),
                                       nl->Second(GetTupleResultType(s)),
                                       costEstimation ));
                    return 0;
                 }

      case REQUEST : {
                    if(!li){
                      return CANCEL;
                    }
                    result.addr = li->next();
                    return result.addr?YIELD:CANCEL;

                 }
      case CLOSE :{
                   if(li){
                     delete li;
                     local.setAddr(0);
                   }
                   return 0;
                 }
   }
  return -1;
}



/*
1.5.4 Value Mapping Array

*/
ValueMapping ItSpatialJoinVM[] = {
         itSpatialJoinVM<StandardSpatialAttribute<2>, 
                         StandardSpatialAttribute<2>,2, 2, 2>,

         itSpatialJoinVM<StandardSpatialAttribute<2>, 
                         StandardSpatialAttribute<3>, 2, 3, 2>,

         itSpatialJoinVM<StandardSpatialAttribute<2>, 
                         UPoint,2,3,2>,
         itSpatialJoinVM<StandardSpatialAttribute<2>, 
                         MPoint,2,3,2>,


         itSpatialJoinVM<StandardSpatialAttribute<3>, 
                         StandardSpatialAttribute<2>,3,2,2>,
         itSpatialJoinVM<StandardSpatialAttribute<3>, 
                         StandardSpatialAttribute<3>,3,3,3>,
         itSpatialJoinVM<StandardSpatialAttribute<3>, 
                         UPoint,3,3,3>,
         itSpatialJoinVM<StandardSpatialAttribute<3>, 
                         MPoint,3,3,3>,

         
         itSpatialJoinVM<UPoint, 
                         StandardSpatialAttribute<2>,3,2,2>,
         itSpatialJoinVM<UPoint, 
                         StandardSpatialAttribute<3>,3,3,3>,
         itSpatialJoinVM<UPoint, 
                         UPoint,3,3,3>,
         itSpatialJoinVM<UPoint, 
                         MPoint,3,3,3>,


         itSpatialJoinVM<MPoint, 
                         StandardSpatialAttribute<2>,3,2,2>,
         itSpatialJoinVM<MPoint, 
                         StandardSpatialAttribute<3>,3,3,3>,
         itSpatialJoinVM<MPoint, 
                         UPoint,3,3,3>,
         itSpatialJoinVM<MPoint, 
                         MPoint,3,3,3>,

      };

/*
1.5.5 Selection function

We use realJoinSelect here.

*/

/*
1.5.6 Cost Estimation

*/
CostEstimation* ItSpatialJoinCostEstimationFunc() {
  return new ItSpatialJoinCostEstimation();
}

CreateCostEstimation ItSpatialJoinCostEstimationList[] 
  = {ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc,
     ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc,
     ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc,
     ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc,
     ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc,
     ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc,
     ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc,
     ItSpatialJoinCostEstimationFunc, ItSpatialJoinCostEstimationFunc};


/*
1.5.7 Operator instance

*/
Operator itSpatialJoin(
      "itSpatialJoin",
       realJoinMMRTreeSpec,
       16, 
       ItSpatialJoinVM,
       realJoinSelect ,
       realJoinMMRTreeTM,
       ItSpatialJoinCostEstimationList     
);



/*
1.6 Similarity Join

1.6.1  Type Mapping

  stream(tuple(A)) x stream(tuple(B)) x a[_]i x b[_]j x real x  [ x  int x int]

*/
template<bool usefun>
ListExpr SimJoinTM(ListExpr args){

   string err = "stream(tuple(A)) x stream(tuple(B)) "
                "x a_i x b_j x real [x int x int] expected";
   
   int len = nl->ListLength(args);
   int reqargs = usefun?6:5;

   if( (len !=reqargs) && (len != (reqargs+2))){
     return listutils::typeError(err);
   }
   ListExpr stream1 = nl->First(args);
   if(!Stream<Tuple>::checkType(stream1)){
     return listutils::typeError("first arg is not a tuple stream");
   }  
   ListExpr stream2 = nl->Second(args);
   if(!Stream<Tuple>::checkType(stream2)){
     return listutils::typeError("second arg is not a tuple stream");
   }  
   ListExpr aname1 = nl->Third(args);
   if(!listutils::isSymbol(aname1)){
     return listutils::typeError("third arg is not an attribute name");
   }
   ListExpr aname2 = nl->Fourth(args);
   if(!listutils::isSymbol(aname2)){
     return listutils::typeError("fourth arg is not an attribute name");
   }
   
   if(!CcReal::checkType(nl->Fifth(args))){
     return listutils::typeError("ffth arg is not a real");
   }


   ListExpr attrList1 = nl->Second(nl->Second(stream1));
   ListExpr attrType1;
   int attrindex1 = listutils::findAttribute(attrList1, 
                                  nl->SymbolValue(aname1),attrType1);
   if(attrindex1 == 0){
      return listutils::typeError("Attribute " + nl->SymbolValue(aname1) +
                                  " not known in first stream");
   }
   ListExpr attrList2 = nl->Second(nl->Second(stream2));
   ListExpr attrType2;
   int attrindex2 = listutils::findAttribute(attrList2, 
                                  nl->SymbolValue(aname2),attrType2);
   if(attrindex2 == 0){
      return listutils::typeError("Attribute " + nl->SymbolValue(aname2) +
                                  " not known in second stream");
   }

   ListExpr resAttrList = listutils::concat(attrList1 , attrList2);


   if(!listutils::isAttrList(resAttrList)){
     return listutils::typeError("naming conflicts detected ");
   }

   if(usefun){ // the 6th argument must be a function 
      ListExpr funList = nl->Sixth(args);
      args = nl->Rest(nl->Rest(nl->Rest(nl->Rest(nl->Rest(nl->Rest(args))))));
      if(!listutils::isMap<2>(funList)){
          return listutils::typeError("sixth element must be a function");
      }
      ListExpr farg1 = nl->Second(funList);
      ListExpr farg2 = nl->Third(funList);
      ListExpr fres = nl->Fourth(funList);
      if(!CcReal::checkType(fres)){
         return listutils::typeError("Function result not of type double");
      }
      if(!nl->Equal(farg1,farg2)){
         return listutils::typeError("Function arguments differ");
      }
      if(!nl->Equal(attrType1,attrType2)){
         return listutils::typeError("Type confliuct in stream attributes");
      }
      if(!nl->Equal(attrType1,farg1)){
         return listutils::typeError("Type conflict between stream "
                                     "attribute and function argument");
      }
      // map is ok
   } else {
      args = nl->Rest(nl->Rest(nl->Rest(nl->Rest(nl->Rest(args)))));
   }


   if(len==(reqargs+2)){
      if(!CcInt::checkType(nl->First(args))){
          return listutils::typeError("next to last element is not an int");
      }
      if(!CcInt::checkType(nl->Second(args))){
          return listutils::typeError("last element is not an int");
      }
   }
   // build result
   ListExpr appendList;
   if(len==(reqargs+2)){
      appendList = nl->TwoElemList(nl->IntAtom(attrindex1-1), 
                                   nl->IntAtom(attrindex2-1));
   } else { // use standard values for min and max
      appendList = nl->FourElemList(nl->IntAtom(4),
                                   nl->IntAtom(8),
                                   nl->IntAtom(attrindex1-1),
                                   nl->IntAtom(attrindex2-1));
   }
   ListExpr resType = nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                                       nl->TwoElemList(
                                           listutils::basicSymbol<Tuple>(),
                                           resAttrList));
   ListExpr result = nl->ThreeElemList(
                             nl->SymbolAtom(Symbols::APPEND()),
                             appendList,
                             resType);

   if(usefun){
        return result;
   }
 
   // check for valid attribute types
   if(CcInt::checkType(attrType1) && CcInt::checkType(attrType2)){
      return result;
   } 
   if(Point::checkType(attrType1) && Point::checkType(attrType2)){
      return result;
   }
   if(CcString::checkType(attrType1) && CcString::checkType(attrType2)){
      return result;
   }
   return listutils::typeError("Unsupported attributes");
}

/*
1.6.2 Some distance functors

*/

class DistCount{
  public: 
     DistCount(){
         cnt = 0;
     }
     void reset(){
        cnt =0;
     }

     size_t getCount() {
        return cnt;
     }

  protected:
     size_t cnt;

};


class IntDist: public DistCount{
  public:

    double operator()(const pair<CcInt*,int>& p1, const pair<CcInt*,int>& p2){
        DistCount::cnt++;
        assert(p1.first);
        assert(p2.first);
        if(!p1.first->IsDefined() && !p2.first->IsDefined()){
          return 0;
        }
        if(!p1.first->IsDefined() || !p2.first->IsDefined()){
           return numeric_limits<double>::max();
        }
        int i1 = p1.first->GetValue();
        int i2 = p2.first->GetValue();
        int c = i1-i2;
        return c<0?-c:c;
    }

    ostream& print(const pair<CcInt*,int>& p, ostream& o){
         o << *(p.first);
         return o;
    }
};

class PointDist: public DistCount{
  public:
    double operator()(const pair<Point*,int>& p1, const pair<Point*,int>& p2){
        cnt++;
        assert(p1.first);
        assert(p2.first);
        if(!p1.first->IsDefined() && !p2.first->IsDefined()){
          return 0;
        }
        if(!p1.first->IsDefined() || !p2.first->IsDefined()){
           return numeric_limits<double>::max();
        }
        return p1.first->Distance(*(p2.first));
    }
    
    ostream& print(const pair<Point*,int>& p, ostream& o){
        o << *(p.first);
        return o;
    }
};

class StringDist: public DistCount{
  public:
    double operator()(const pair<CcString*,int>& p1, 
                      const pair<CcString*,int>& p2){
        cnt++;
        assert(p1.first);
        assert(p2.first);
        if(!p1.first->IsDefined() && !p2.first->IsDefined()){
          return 0;
        }
        if(!p1.first->IsDefined() || !p2.first->IsDefined()){
           return numeric_limits<double>::max();
        }
        return stringutils::ld(p1.first->GetValue(), p2.first->GetValue());
    }
    
    ostream& print(const pair<CcString*,int>& p, ostream& o){
        o << *(p.first);
        return o;
    }
};



/*
1.6.2 LocalInfo Class

*/
template<class T, class DistComp>
class SimJoinLocalInfo{
public:
   SimJoinLocalInfo( Word _stream1, Word _stream2,
                     const double _epsilon,
                     const int _min, const int _max, // parameters for the tree
                     const int _a1, const int _a2, // attribute indexes
                     const size_t _maxMem, ListExpr resType,
                     DistComp& _dc):
                     stream1(_stream1), stream2(_stream2),
                     epsilon(_epsilon),
                     min(_min), max(_max),
                     a1(_a1), a2(_a2), 
                     tt(0), tree(0), buffer_stream1(0),
                     it1(0), stream1Finished(false), stream2Finished(false),
                     maxMem(_maxMem), buffer_stream2(0), it2(0), current(0),
                     dc(_dc), 
                     partitions(0),max_elems(0), comparisons(0){

         tt = new TupleType(resType);
         stream1.open();
         stream2.open();
         readNextPartition();
         current = getNextTuple();
         if(current){ // case of empty second stream
             T key = (T) current->GetAttribute(a2);
             pair<T,int> p(key,0);
             it1 = tree->rangeSearch(p, epsilon);
         }
   }

   ~SimJoinLocalInfo(){

       cout << "Join finsihed with " << partitions 
            << " scans of the second stream" << endl;
       cout << "The maximum number of elements within the index was " 
            << max_elems << endl;

       if(it1){
          comparisons += it1->noComparisons();
       }

       cout << "no Comparisons : " << comparisons << endl;

       tt->DeleteIfAllowed();
       stream1.close();
       stream2.close();
       removePartition();
       if(it1){
          //cout << "Comparisons for scan : " << it1->noComparisons() << endl;
          //cout << "Processed stream1 : " << stream1.getCount() << endl;
          //cout << "Processed stream2 : " << stream2.getCount() << endl;
          delete it1;
       }
       if(it2){
         delete it2;
       }
       if(buffer_stream2){
          delete buffer_stream2;
       }
    }
                      
    Tuple* next(){
        while(it1){ // range iterator initialized
          if(it1->hasNext()){
            pair<T,int> p = *(it1->next());
            Tuple* t1 = (*buffer_stream1)[p.second];
            Tuple* res = new Tuple(tt);
            Concat(t1,current,res);
            return res;  
          } else { // range iterator exhausted
            current->DeleteIfAllowed();
            current = getNextTuple();
            //cout << "Comparisons for scan : " << it1->noComparisons() << endl;
            //cout << "Processed stream1 : " << stream1.getCount() << endl;
            //cout << "Processed stream2 : " << stream2.getCount() << endl;
            comparisons += it1->noComparisons();
            delete it1;
            it1 = 0;
            if(!current){
                if(stream1Finished){
                  return 0;
                } else {
                  readNextPartition();
                  if(it2){
                    delete it2;
                    it2 = 0;
                  } 
                  current = getNextTuple();
                }
            }
            if(!current){
               return 0;
            }
            T key = (T) current->GetAttribute(a2);
            pair<T,int> p(key,0);

            it1 = tree->rangeSearch(p, epsilon);
          }
        }
        return 0;
    }
   

private:
    Stream<Tuple> stream1;
    Stream<Tuple> stream2;
    double epsilon;
    int min;
    int max;
    int a1;
    int a2;
    TupleType* tt; 
    MMMTree<pair<T,int>, DistComp >* tree;
    vector<Tuple*>*  buffer_stream1;
    RangeIterator<pair<T,int>,DistComp >*  it1;
    bool stream1Finished;
    bool stream2Finished;
    size_t maxMem;
    Relation* buffer_stream2;
    GenericRelationIterator* it2;
    Tuple* current; // current tuple of stream 2
    DistComp dc;
    size_t partitions;
    size_t max_elems;
    size_t comparisons;


    // replaces the current partition by the next one
    void readNextPartition(){
      partitions++;
      removePartition();
      if(stream1Finished){
         return;
      }

      buffer_stream1 = new vector<Tuple*>();
      tree = new MMMTree<pair<T,int>,DistComp> (min,max,dc);
      size_t usedSize = 0;
      Tuple* t = stream1.request();

      stream1Finished = t==0;
      size_t nodesize = sizeof(MTreeNode<pair<T,int>,DistComp >);

      size_t cnt = 0;

      while(t){
         cnt++;
         T key = (T)  t->GetAttribute(a1);
         int pos = buffer_stream1->size();
         buffer_stream1->push_back(t);
         pair<T,int> p(key,pos);
         tree->insert(p);
         usedSize = usedSize + nodesize + t->GetMemSize();
         if(usedSize < maxMem){
            t = stream1.request();
            if(t==0){
               stream1Finished = true;
            }
         } else {
            t = 0;
         }
      }

      //cout << "tree created" << endl;
      //cout << "comparisons required : " << tree->noComparisons() << endl;
      //cout << cnt << " elements inserted" << endl;
      comparisons += tree->noComparisons();

      if(cnt > max_elems){
          max_elems = cnt;
      }
    }

    // removes the currently buffered  partition
    void removePartition(){
       if(tree==0){
         return;
       }
       delete tree;
       for(size_t i=0;i<buffer_stream1->size();i++){
          (*buffer_stream1)[i]->DeleteIfAllowed();
       }
       delete buffer_stream1;
       buffer_stream1 = 0;
       tree = 0;
    }


   
    // returns the next tuple of stream 2
    Tuple* getNextTuple(){
        Tuple* res = 0;
        if(!stream2Finished){
           // read from stream
           res = stream2.request(); 
           if(res){
              if(!stream1Finished){
                 if(!buffer_stream2){
                     buffer_stream2 = new Relation(res->GetTupleType(),true);
                 }                 
                 buffer_stream2->AppendTuple(res);
              } 
              return res;
           } else {
              stream2Finished = true;
              return 0;
           }
        }
        assert(buffer_stream2);
        if(!it2){
           it2 = buffer_stream2->MakeScan();
        }
        return it2->GetNextTuple(); 
    }
};


/*
1.6.3 Value Mapping


*/

template <class T, class DistComp>
int SimJoinVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ) {

  SimJoinLocalInfo<T,DistComp>* li = (SimJoinLocalInfo<T,DistComp>*) local.addr;

   switch(message){
      case OPEN: {
            if(li){
               delete li;
               local.addr = 0;
            }
            CcInt* Min = (CcInt*) args[5].addr;
            CcInt* Max = (CcInt*) args[6].addr;
            int min = 4;
            int max = 8;
            if(Min->IsDefined() && Max->IsDefined()){
                int m1 = Min->GetValue();  
                int m2 = Max->GetValue();
                if((m1 >= 2) && (m2>= 2* m1)){
                   min = m1;
                   max = m2;
                }
            } 
            CcReal* eps = (CcReal*) args[4].addr;
            if(!eps->IsDefined()){
               return 0;
            }
            double epsilon = eps->GetValue();
            if(epsilon < 0){
               return 0;
            }
            int a1 = ((CcInt*)args[7].addr)->GetValue();
            int a2 = ((CcInt*)args[8].addr)->GetValue();
            size_t memSize = qp->GetMemorySize(s) * 1024 * 1024;
            ListExpr resType = nl->Second(GetTupleResultType(s));
            DistComp dc;
            local.addr = new SimJoinLocalInfo<T,DistComp>(args[0], args[1],
                                              epsilon, min, max, a1, a2,
                                              memSize,
                                              resType, dc);
            return 0;
      }
      case REQUEST: {
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;
      }
      case CLOSE: {
          if(li){
             delete li;
             local.addr = 0;
          }
          return 0;

      }

   }
   return 0;
}

/*
1.6.4 Specification

*/

OperatorSpec SimJoinSpec(
  " stream(tuple(A)) x stream(tuple(B)) x a_i x b_j "
  "x real  [ x int x int ] -> stream (tuple(AB))",
  " _ _ simJoin [ a_i, b_j, epsilon, min, max] ",
  "Similariry join based on predefined distance functions",
  " query Orte feed Orte feed {o} simJoin[ Name, Name_o, 4.0] count" 
);


/*
1.6.5 Value Mapping array and selection function

*/
ValueMapping SimJoinVM[] = {
     SimJoinVMT<CcInt*,IntDist>,
     SimJoinVMT<Point*,PointDist>,
     SimJoinVMT<CcString*,StringDist>
};

int SimJoinSelect(ListExpr args){
   ListExpr s1 = nl->First(args);
   string aname = nl->SymbolValue(nl->Third(args));
   ListExpr type;
   listutils::findAttribute(nl->Second(nl->Second(s1)), aname, type);
   if(CcInt::checkType(type)){
     return 0;
   }
   if(Point::checkType(type)){
      return 1;
   }
   if(CcString::checkType(type)){
      return 2;
   }
   return -1; 
};

/*
1.6.6 Operator instance

*/
Operator SimJoinOp(
  "simJoin",
  SimJoinSpec.getStr(),
  3,
  SimJoinVM,
  SimJoinSelect,
  SimJoinTM<false>
);


class FunDistComp: public DistCount{
  public:
    FunDistComp(ArgVectorPointer _funargs, QueryProcessor* _qp, Word _fun):
      funargs(_funargs), qp(_qp), fun(_fun) { }


    double operator()(const pair<Attribute*, int>& p1, 
                      const pair<Attribute*, int>& p2){
        cnt++;
        assert(p1.first);
        assert(p2.first);

        (*funargs)[0] = p1.first;
        (*funargs)[1] = p2.first;
        Word res;
        qp->Request(fun.addr,res);
        CcReal* r = (CcReal*) res.addr;
        if(!r->IsDefined()){
           return numeric_limits<double>::max();
        }     
        double rd = r->GetValue();
        return rd<0?-rd:rd;  
    }
    
    ostream& print(const pair<Word,int>& p, ostream& o){
        return o;
    }

   private:
     ArgVectorPointer funargs;
     QueryProcessor* qp;
     Word fun;
};

int SimJoinFunVM( Word* args, Word& result, int message,
               Word& local, Supplier s ) {

  SimJoinLocalInfo<Attribute*,FunDistComp>* li = 
            (SimJoinLocalInfo<Attribute*, FunDistComp>*) local.addr;

   switch(message){
      case OPEN: {
            if(li){
               delete li;
               local.addr = 0;
            }
            CcInt* Min = (CcInt*) args[6].addr;
            CcInt* Max = (CcInt*) args[7].addr;
            int min = 4;
            int max = 8;
            if(Min->IsDefined() && Max->IsDefined()){
                int m1 = Min->GetValue();  
                int m2 = Max->GetValue();
                if((m1 >= 2) && (m2>= 2* m1)){
                   min = m1;
                   max = m2;
                }
            } 
            CcReal* eps = (CcReal*) args[4].addr;
            if(!eps->IsDefined()){
               return 0;
            }
            double epsilon = eps->GetValue();
            if(epsilon < 0){
               return 0;
            }
            int a1 = ((CcInt*)args[8].addr)->GetValue();
            int a2 = ((CcInt*)args[9].addr)->GetValue();
            size_t memSize = qp->GetMemorySize(s) * 1024 * 1024;
            ListExpr resType = nl->Second(GetTupleResultType(s));
            // create distcomp from function (arg[6])
            ArgVectorPointer funargs = qp->Argument(args[5].addr);
            Word fun = args[5];
            FunDistComp dc(funargs,qp,fun);
            local.addr = new SimJoinLocalInfo<Attribute*, FunDistComp>(
                            args[0], args[1],
                            epsilon, min, max, a1, a2,
                            memSize,
                            resType, dc);
            return 0;
      }
      case REQUEST: {
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;
      }
      case CLOSE: {
          if(li){
             delete li;
             local.addr = 0;
          }
          return 0;

      }

   }
   return 0;
}


OperatorSpec SimJoinFunSpec(
  " stream(tuple(A)) x stream(tuple(B)) x a_i x b_j x "
  "real x (fun: t_i x t_j -> real) [ x int x int ] -> stream (tuple(AB))",
  " _ _ simJoin [ a_i, b_j, epsilon, distfun, min, max] ",
  "Similariry join based on user defined distance functions",
  " query Orte feed Orte feed {o} simJoin[ Name, Name_o, 4.0, "
  "fun(a:string, b: string) 1.0*(length(a)-length(b) ] count" 
);

Operator SimJoinFunOp(
  "simjoinfun",
  SimJoinFunSpec.getStr(),
  SimJoinFunVM,
  Operator::SimpleSelect,
  SimJoinTM<true>
);



/*
2 Algebra Definition

2.1 Algebra Class

*/

class MMRTreeAlgebra : public Algebra {
  public:
   MMRTreeAlgebra() : Algebra()
   {
      // operators using mmrtrees
      AddOperator(&insertMMRTree);
      AddOperator(&statMMRTree);
      AddOperator(&realJoinMMRTree);
        realJoinMMRTree.SetUsesMemory();
      AddOperator(&realJoinMMRTreeVec);
        realJoinMMRTreeVec.SetUsesMemory();
      AddOperator(&joinMMRTreeIt);
        joinMMRTreeIt.SetUsesMemory();
      AddOperator(&joinMMRTreeItVec);
        joinMMRTreeItVec.SetUsesMemory();
      AddOperator(&itSpatialJoin);
        itSpatialJoin.SetUsesMemory();


      AddOperator(&SimJoinOp);
      SimJoinOp.SetUsesMemory();

      AddOperator(&SimJoinFunOp);
      SimJoinFunOp.SetUsesMemory();
   }
};

/*
2.2 Algebra Initialization

*/

extern "C"
Algebra*
InitializeMMRTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new MMRTreeAlgebra());
}


