

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


#include "MMRTreeAlgebra.h"
#include "MMRTree.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "FTextAlgebra.h"


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
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string name = nl->SymbolValue(nl->Third(args));

  ListExpr type;

  int index = listutils::findAttribute(attrList,name,type);
  assert(index>0);
  if(listutils::isKind(type,Kind::SPATIAL2D())){
     return 0;
  }
  if(listutils::isKind(type, Kind::SPATIAL3D())){
     return 1;
   }
   assert(false);
   return -1;
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
 [maxMem]    : maximum memory available for storing tuples

----

*/
ListExpr realJoinMMRTreeTM(ListExpr args){



  string err = "stream(tuple(A)) x stream(tuple(B)) x"
               " a_i x b_i x int x int [x int] expected";
   int len = nl->ListLength(args);
   if((len!=6) && (len!=7)){
     return listutils::typeError(err);
   }

   if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)) ||
      !listutils::isSymbol(nl->Third(args)) ||
      !listutils::isSymbol(nl->Fourth(args))  ||
      !CcInt::checkType(nl->Fifth(args)) ||
      !CcInt::checkType(nl->Sixth(args))){
     return listutils::typeError(err);
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
      !listutils::isKind(type1,Kind::SPATIAL3D())  ){
     return listutils::typeError("Attribute " + name1 + 
                                 " is not in Kind Spatial2D or Spatial3D");
   }
   if(!listutils::isKind(type2,Kind::SPATIAL2D()) &&
      !listutils::isKind(type2,Kind::SPATIAL3D())  ){
     return listutils::typeError("Attribute " + name1 + 
                                 " is not in Kind Spatial2D or Spatial3D");
   }
 
  // check for same dimension
  if(listutils::isKind(type1,Kind::SPATIAL2D()) &&
     !listutils::isKind(type2,Kind::SPATIAL2D())){
    return listutils::typeError("Selected attributes have "
                                "different dimensions"); 
  }
  if(listutils::isKind(type1,Kind::SPATIAL3D()) &&
     !listutils::isKind(type2,Kind::SPATIAL3D())){
    return listutils::typeError("Selected attributes have"
                                " different dimensions"); 
  }

  bool rect1 = Rectangle<2>::checkType(type1) || Rectangle<3>::checkType(type1);
  bool rect2 = Rectangle<2>::checkType(type2) || Rectangle<3>::checkType(type2);

 
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
   } else {
      appendList =   nl->FiveElemList(
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
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId>,2 > >,
    joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<3, TupleId>,3 > >
  };



Operator realJoinMMRTree(
  "realJoinMMRTree",
  realJoinMMRTreeSpec,
  2,
  realJoinMMRTreeVM,
  realJoinSelect,
  realJoinMMRTreeTM 
 );



ValueMapping realJoinMMRTreeVecVM[] = {
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<2, TupleId>,2 > >,
    joinRTreeVM<RealJoinTreeVecLocalInfo<mmrtree::RtreeT<3, TupleId>,3 > >
  };


Operator realJoinMMRTreeVec(
  "realJoinMMRTreeVec",
  realJoinMMRTreeSpec,
  2,
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

Becaus the operator has the same  functionality as the realJoinMMRTree 
operator, we can reuse the type mapping of that operator.

*/

template<int dim>
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
        Rectangle<dim> box = ((StandardSpatialAttribute<dim>*) 
                              tuple->GetAttribute(attrPos1))->BoundingBox();
        index.insert(box,id);
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
          Rectangle<dim> r = ((StandardSpatialAttribute<dim>*)
                        currentTuple->GetAttribute(attrPos2))->BoundingBox();
          it=index.find( r );
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
   mmrtree::RtreeT<dim, TupleId> index;
   TupleStore buffer;
   typename mmrtree::RtreeT<dim, TupleId>::iterator* it;
   Tuple* currentTuple;
   TupleType* tt;
}; // class joinMMRTreeItLocalInfo


template<int dim>
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
        Rectangle<dim> box = ((StandardSpatialAttribute<dim>*)
                         tuple->GetAttribute(attrPos1))->BoundingBox(); 
        index.insert(box,id);
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
           Rectangle<dim> r = ((StandardSpatialAttribute<dim>*)
                     currentTuple->GetAttribute(attrPos2))->BoundingBox();
           it = index.find(r);
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
   mmrtree::RtreeT<dim, TupleId> index;
   vector<Tuple*> buffer;
   typename mmrtree::RtreeT<dim, TupleId>::iterator* it;
   Tuple* currentTuple;
   TupleType* tt;
}; // class joinMMRTreeItVecLocalInfo



template <int dim, class LocalInfo>
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
    joinMMRTreeItVM<2, joinMMRTreeItLocalInfo<2> >,
    joinMMRTreeItVM<3, joinMMRTreeItLocalInfo<3> >
};




Operator joinMMRTreeIt (
  "joinMMRTreeIt",
  realJoinMMRTreeSpec,
  2, 
  joinMMRTreeItvm,
  realJoinSelect ,
  realJoinMMRTreeTM
  );


ValueMapping joinMMRTreeItVecvm[] = {
    joinMMRTreeItVM<2, joinMMRTreeItVecLocalInfo<2> >,
    joinMMRTreeItVM<3, joinMMRTreeItVecLocalInfo<3> >
};




Operator joinMMRTreeItVec (
  "joinMMRTreeItVec",
  realJoinMMRTreeSpec,
  2, 
  joinMMRTreeItVecvm,
  realJoinSelect ,
  realJoinMMRTreeTM
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


