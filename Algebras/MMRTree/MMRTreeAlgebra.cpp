

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
#include "rstartree.h"
#include "storage.h"
#include "util.h"


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
  if(Rectangle<2>::checkType(type)){
     return 0;
  }
  if(Rectangle<3>::checkType(type)){
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
  attrname1 : attribute name for an attribute of type rect in stream1
  attrname2 : attribute name for an attribute of type rect in stream2
  min       : minimum number of entries within a node of the used rtree
  max       : maximum number of entries within a node of the used rtree

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
   if(!Rectangle<2>::checkType(type1) && !Rectangle<3>::checkType(type1)){
     return listutils::typeError("Attribute " + name1 + 
                                 " not of type " + Rectangle<2>::BasicType());
   }
   
   int index2 = listutils::findAttribute(attrList2, name2, type2);
   if(index2 == 0){
     return listutils::typeError("Attribute " + name2 + 
                                 " not present in second stream");
   }
   if(!nl->Equal(type1,type2)){
     return listutils::typeError("Attribute type in the second stream differs"
                                 " from the attribute type in the first "
                                 "stream" );
   }
 
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
      appendList =   nl->TwoElemList(nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1));
   } else {
      appendList =   nl->ThreeElemList(
                                     nl->IntAtom(-1),
                                     nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1));
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
3 Operators using r-star implementation


3.1 Operator insertRStar

3.1.1 Type Mapping

Signature is: stream(rectangle) x int x double -> stream(rectangle)

*/
ListExpr insertRStarTM(ListExpr args){

  string err = " stream(rectangle) x int x double expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err);
  }
  if(!Stream<Rectangle<2> >::checkType(nl->First(args)) ||
     !CcInt::checkType(nl->Second(args)) ||
     !CcReal::checkType(nl->Third(args))){
     return listutils::typeError(err);
  }
  return nl->First(args);
}

/*
3.1.2 Value Mapping


~copy~

Auxiliary function to copy the value of  a Secondo Rectangle<2> into
an rstartree::Rectangle.

*/
void copy(rstartree::Rectangle& target, const Rectangle<2>& src){

   target.min.x = src.MinD(0);
   target.min.y = src.MinD(1);
   target.max.x = src.MaxD(0);
   target.max.y = src.MaxD(1);
}

/*
~InsertRStarLI~

Auxiliary class. 

*/

class InsertRStarLI{
  public:
    InsertRStarLI(Word& s, int b, double& f):
         stream(s), store(), tree(0), count(0) {
         
        tree = new rstartree::RTree(&store);
        tree->create(b,f);
        stream.open();
    }

    ~InsertRStarLI(){
       stream.close();
       delete tree;
    }

    Rectangle<2>* next(){
       Rectangle<2>* res = stream.request();
       if(!res){
         return 0;
       }
       if(!res->IsDefined()){
          return res;
       }
       rstartree::Object obj;
       obj.id = count++;
       copy(obj.mbr, *res);
       tree->insertData(obj);
       return res;
    }

   

  private:
    Stream<Rectangle<2> > stream;
    rstartree::Storage store;
    rstartree::RTree*   tree;
    size_t count;


};



int insertRStarVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{  
  
  InsertRStarLI* li = (InsertRStarLI*) local.addr;

  switch(message){
     case OPEN: {
          if(li){
             delete li;
             local.addr = 0;
             li = 0;
          }
          CcInt* branches = (CcInt*) args[1].addr;
          CcReal* fill = (CcReal*) args[2].addr;
          if(!branches->IsDefined() || !fill->IsDefined()){
            return 0;
          }
          int b = branches->GetValue();
          double f = fill->GetValue();

          if( (b<2) || (f < 0.5) || (f>=1) ){
             return 0;
          }
          // values are ok, 
          local.addr = new InsertRStarLI(args[0], b,f);
          return 0;
     }
     case REQUEST: {
          if(!li){
            return CANCEL;
          }
          result.addr = li->next();
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
  return -1;
}

/*
3.1.3 Specification

*/


const string insertRStarSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(rectangle) x int x double -> stream(rectangle) "
       "</text---> "
       "<text>_ insertRStar[branches, filling]</text--->"
       "<text>Inserts all the rectangles within the "
       "stream into an Rstar tree.The argument branches is the maximul number"
       " of nodes within the tree. It must be greater than 1."
       " The argument filling is the fill degree of the tree. "
       "It must be in ([0.5,1)</text--->"
       "<text>"
       "query strassen feed extend[B : bbox(.geoData)] insertRStar[B] count"
       "</text--->"
     ") )";




/*
3.1.4 Operator instance

*/
Operator insertRStar (
  "insertRStar",
  insertRStarSpec,
  insertRStarVM,
  Operator::SimpleSelect,
  insertRStarTM);


/*
3.2 Operator ~realJoinRStar~


3.2.1 Type Mapping

Signature is: stream(tuple(A)) x stream(tuple(b)) x a[_]i x b[_]j 
              x int x real [+ int] -> stream(tuple(A o B))
 
*/
ListExpr realJoinRStarTM(ListExpr args){
   string err = "stream(tuple(A)) x stream(tuple(b)) x a[_]i x b[_]j "
                " x int x real [+ int] expected";
   int len = nl->ListLength(args);
   if((len!=6)  && (len!=7)){
      return listutils::typeError(err);
   }
   if((len==7) && !CcInt::checkType(nl->Sixth(nl->Rest(args)))){
      return listutils::typeError(err);
   }
   if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)) ||
      !listutils::isSymbol(nl->Third(args)) ||
      !listutils::isSymbol(nl->Fourth(args)) ||
      !CcInt::checkType(nl->Fifth(args)) ||
      !CcReal::checkType(nl->Sixth(args))){
      return listutils::typeError(err);
   }
   ListExpr attrList1 = nl->Second(nl->Second(nl->First(args)));
   ListExpr attrList2 = nl->Second(nl->Second(nl->Second(args)));
   string name1 = nl->SymbolValue(nl->Third(args));
   string name2 = nl->SymbolValue(nl->Fourth(args));
   ListExpr type;

   int index1 = listutils::findAttribute(attrList1, name1, type);
   if(index1==0){
      return listutils::typeError("Attribute " + name1 + 
                                  " unknown in stream1");
   }
   if(!Rectangle<2>::checkType(type)){
      return listutils::typeError("Attribute " + name1 + " not of type " + 
                                  Rectangle<2>::BasicType());
   }
   int index2 = listutils::findAttribute(attrList2, name2, type);
   if(index2==0){
      return listutils::typeError("Attribute " + name2 + 
                                  " unknown in stream2");
   }
   if(!Rectangle<2>::checkType(type)){
      return listutils::typeError("Attribute " + name2 + " not of type " + 
                                  Rectangle<2>::BasicType());
   }

  ListExpr attrList = listutils::concat(attrList1, attrList2);
  if(!listutils::isAttrList(attrList)){
     return listutils::typeError("attributes of the input streams "
                        "have name conflicts");
  }
  ListExpr appendList = len==7 
              ?   nl->TwoElemList(
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1))
              :   nl->ThreeElemList(
                         nl->IntAtom(-1),
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1));
                         
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           appendList,
                           nl->TwoElemList( 
                               nl->SymbolAtom(Stream<Tuple>::BasicType()),
                               nl->TwoElemList(
                                   nl->SymbolAtom(Tuple::BasicType()),
                                   attrList)));
}



class RealJoinRStarLocalInfo{

  public:

/*
~Constructor~

The parameters are:

----
     _s1 : first stream
     _s2 : second stream
     _i1 : index of a rect attribute in _s1
     _i2 : index of a rect attribute in _s2
     _tt : list describing the result tuple type
     max : maximum number of entries within a node of the index
     fill: fill degree of nodes
    _maxMem : maximum cache size for tuples of _s1 in kB

----

*/

     RealJoinRStarLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, int max, double fill,
                             size_t maxMem):
         storage(), s2(_s2),  i2(_i2) {
         ind = new rstartree::RTree(&storage);
         ind->create(max,fill);
         // build the index from the first stream
         tt = new TupleType(_tt);
         Stream<Tuple> s1(_s1);
         s1.open();
         Tuple* t = s1.request();
         if(t){
           tb = new TupleStore(maxMem); 
         } else {
           tb = 0;
         }
         rstartree::Object obj;
         while(t){
           obj.id = tb->AppendTuple(t);
           Rectangle<2>* r = (Rectangle<2>*)t->GetAttribute(_i1);
           copy(obj.mbr, *r); 
           ind->insertData(obj);
           t->DeleteIfAllowed(); 
           t = s1.request();
         }
         s1.close();
         s2.open();
         currentTuple = 0;
     }


/*
~Destructor~

*/

     ~RealJoinRStarLocalInfo(){
         s2.close();
         
         tt->DeleteIfAllowed();
         if(tb){
           delete tb;
         }
         if(currentTuple){
            currentTuple->DeleteIfAllowed();
         }
         delete ind;
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
            Rectangle<2>* r = (Rectangle<2>*) t->GetAttribute(i2);
            rstartree::Rectangle mbr;
            copy(mbr, *r);
            ind->rangeQuery(lastRes, mbr); 
            if(lastRes.empty()){
                t->DeleteIfAllowed();
            } else {
               currentTuple = t;
            }
         }

         rstartree::Object  obj = lastRes.back();
         lastRes.pop_back();
         Tuple*result = new Tuple(tt);
         Tuple* t1 = tb->GetTuple(obj.id);
         Concat(t1, currentTuple, result);
         t1->DeleteIfAllowed();         
         return result;
      }

   private:
      rstartree::Storage storage;
      rstartree::RTree* ind;
      Stream<Tuple> s2;
      int i2;
      TupleType* tt;
      vector<rstartree::Object > lastRes;
      Tuple* currentTuple;
      TupleStore* tb;


};



int realJoinRStarVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{

   RealJoinRStarLocalInfo* li  = (RealJoinRStarLocalInfo*) local.addr;
   switch(message){
        case OPEN: {
              if(li){
                delete li;
                local.addr = 0;
                li = 0;
              }
              CcInt*  Max = (CcInt*) args[4].addr;
              CcReal* Fill = (CcReal*) args[5].addr;
              CcInt*  MaxMem = (CcInt*) args[6].addr;
              if(!Max->IsDefined() || !Fill->IsDefined() ||
                 !MaxMem->IsDefined()){
                  return 0;
              }
              int max = Max->GetValue();
              double fill = Fill->GetValue();
              int maxMem = MaxMem->GetValue();
              if(maxMem<0){
                maxMem = qp->MemoryAvailableForOperator()/1024;
              }
              if(max < 2 || fill < 0.5 || fill >=1){
                 return 0;
              }
              int i1 = ((CcInt*)args[7].addr)->GetValue();
              int i2 = ((CcInt*)args[8].addr)->GetValue();
              local.addr = new RealJoinRStarLocalInfo(
                                  args[0], args[1],
                                  i1, i2,
                                  nl->Second(GetTupleResultType(s)),
                                  max, fill,
                                  maxMem);
              return 0;
        }
      case REQUEST: {
            if(!li){
               return CANCEL;
            }
            result.addr = li->nextTuple();
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
   return -1;

}

/*
3.2.3 Specification

*/
const string realJoinRStarSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(tuple(A)) stream(tuple(B)) x a_i x b_j x int x "
       "double [x int] -> stream(tuple( A o B)) </text---> "
       "<text>_ _ insertRStar[a_i, b_j, max, fill [, maxMem]]</text--->"
       "<text>Performs a spatial join on the input streams. "
       "It first inserts all entries from stream A into an r*tree build"
       " by parameters max (maximum degree of a single node) and fill "
       "(filling degree).."
       " The tuples of the stream a collected into a tuple store with cache "
       "size maxMem (in kB). For each tuple from the second stream a search"
       " is performed on the r*tree to get the intersecting tuples. "
       " </text--->"
       "<text>"
       "query strassen feed extend[B : bbox(.geoData)] {a} "
       "strassen feed extend[B : bbox(.geoData)] {b} "
       "realJoinRStar[B_a, B_b, 16, 0.7, 512] count"
       "</text--->"
     ") )";

/*
3.2.4 Operator instance

*/

Operator realJoinRStar (
  "realJoinRStar",
  realJoinRStarSpec,
  realJoinRStarVM,
  Operator::SimpleSelect,
  realJoinRStarTM);



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
      AddOperator(&realJoinMMRTreeVec);

      // R* operators
      AddOperator(&insertRStar);
      AddOperator(&realJoinRStar);
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


