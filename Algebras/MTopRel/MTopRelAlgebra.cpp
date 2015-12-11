/*
----
This file is part of SECONDO.

Copyright (C) 2007,
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



#include "dfa.h"
#include "GenericTC.h"
#include "GenOps.h"
#include "Algebra.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "SpatialAlgebra.h"
#include "Point.h"
#include "MTopRelAlgs.h"
#include "TemporalAlgebra.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "Symbols.h"


using namespace std;
using namespace datetime;

namespace temporalalgebra{

/*
1 Type Constructor

*/

GenTC<TopRelDfa> topreldfa;


/*
2 Operators


1.1 createdfa


Operator for creating a topreldfa from a pridicategroup and a
text.

*/

int
  createDfaVM( Word* args, Word& result, int message,
               Word& local, Supplier s ){

    result = qp->ResultStorage(s);
    toprel::PredicateGroup* pg =
         static_cast<toprel::PredicateGroup*>(args[0].addr);
    FText* regex = static_cast<FText*>(args[1].addr);
    TopRelDfa* res = static_cast<TopRelDfa*>(result.addr);
    if(!pg->IsDefined() || !regex->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
    string r = regex->GetValue();

    res->setTo( r, *pg);
    return 0;
}

const string createDfaSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>predicategroup x text -> mtoprel  </text--->"
       "<text>createDfa(_,_)</text--->"
       "<text>Creates an mtoprel object</text--->"
       "<text>query createDfa(stdpgroup(), 'inside meet disjoint'</text--->"
       ") )";


Operator createDfa(
           "createDfa",
           createDfaSpec,
           createDfaVM,
           Operator::SimpleSelect,
           TypeMap2<toprel::PredicateGroup, FText, TopRelDfa>);


/*
2. Operator toprelseq

This operator takes a spatial type and an moving spatial type and
produces a stream of tuples comptaining an time interval (represented
as attributes Start, End, LC, RC, and an int9m representing the
topological relationship between the arguments within this time
interval.

*/
ListExpr toprelseqTM(ListExpr args){
  string err = "expected point x upoint | "
                " point x mpoint | "
                " upoint x upoint |"
                " upoint x mpoint |"
                " mpoint x mpoint |"
                " region x upoint |"
                " region x mpoint |"
                " upoint x point  |"
                " mpoint x point  |"
                " mpoint x upoint |"
                " upoint x region |"
                " mpoint x region " ;
  int len = nl->ListLength(args);
  if((len!=2) ){
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  bool ok = false;

  ok = ok || (listutils::isSymbol(arg1, Point::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Point::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Region::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Region::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, Point::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, Point::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, Region::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, Region::BasicType()));



  if(ok){
    ListExpr attrList = nl->FiveElemList(
               nl->TwoElemList( nl->SymbolAtom("Start"),
                                nl->SymbolAtom(DateTime::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("End"),
                                nl->SymbolAtom(DateTime::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("LeftClosed"),
                                nl->SymbolAtom(CcBool::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("RightClosed"),
                                nl->SymbolAtom(CcBool::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("TopRel"),
                                nl->SymbolAtom(toprel::Int9M::BasicType())));

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()), attrList));

  }

  return listutils::typeError(err);
}





const string toprelseqSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>point x {upoint, mpoint}  | "
        "        upoint x upoint  | "
        "        upoint x mpoint  | "
        "        mpoint x mpoint  | "
        "        region x upoint  | "
        "        region x point   |"
        "        upoint x point   |"
        "        mpoint x point   |"
        "        mpoint x upoint  |"
        "        upoint x region  |"
        "        mpoint x region  "
        "-> stream(tuple(Start "
        "instant, End instant, "
        "LeftClosed bool, RightClosed bool, TopRel int9m))  </text--->"
       "<text>toprelseq(_,_)</text--->"
       "<text>Returns the stream of topological relationships"
       " together with time intervals</text--->"
       "<text>query toprelseq(p, up) tconsume</text--->"
       ") )";


template<class A1, class A2, class Proc>
class toprelseq_LocalInfo{
  public:

     toprelseq_LocalInfo(const A1* a1, const A2* a2, const ListExpr tt){
        tupleType = new TupleType(tt);
        proc = new Proc(a1,a2);
     }

     toprelseq_LocalInfo(const A1* a1, const A2* a2,
                         const toprel::PredicateGroup* pg, ListExpr tt){
        tupleType = new TupleType(tt);
        proc = new Proc(a1,a2,pg);
     }

     ~toprelseq_LocalInfo(){
        tupleType->DeleteIfAllowed();
        delete proc;
     }

     Tuple* nextTopRelTuple(){
       if(!proc->hasNext()){
          return 0;
       }
       Tuple* tuple = new Tuple(tupleType);
       pair<Interval<Instant>, toprel::Int9M> p = proc->next();
       storeIntervalIntoTuple(tuple, p.first);
       tuple->PutAttribute(4, new toprel::Int9M(p.second));
       return tuple;
     }

     Tuple* nextClusterTuple(){
       if(!proc->hasNextCluster()){
          return 0;
       }
       Tuple* tuple = new Tuple(tupleType);
       pair<Interval<Instant>, toprel::Cluster> p = proc->nextCluster();
       storeIntervalIntoTuple(tuple, p.first);
       tuple->PutAttribute(4, new toprel::Cluster(p.second));
       return tuple;
     }


  private:
    TupleType* tupleType;
    Proc* proc;

/*
~storeIntervalIntoTuple~

stores the interval at the first 4 positions of the tuple.

*/
void storeIntervalIntoTuple(Tuple* tuple, Interval<Instant>& interval){
       tuple->PutAttribute(0, new DateTime(interval.start));
       tuple->PutAttribute(1, new DateTime(interval.end));
       tuple->PutAttribute(2, new CcBool(true,interval.lc));
       tuple->PutAttribute(3, new CcBool(true, interval.rc));
}


};

int toprelseqSelect(ListExpr args){
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(listutils::isSymbol(arg1, Point::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 0;
  }
  if(listutils::isSymbol(arg1, Point::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 1;
  }
  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 2;
  }
  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 3;
  }
  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 4;
  }
  if(listutils::isSymbol(arg1, Region::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 5;
  }
  if(listutils::isSymbol(arg1, Region::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 6;
  }
  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, Point::BasicType())){
     return 7;
  }
  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, Point::BasicType())){
     return 8;
  }

  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 9;
  }

  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, Region::BasicType())){
     return 10;
  }
  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, Region::BasicType())){
     return 11;
  }
  return -1;

}


template<class A1, class A2, class Proc, bool sym>
int toprelseq_fun(Word* args, Word& result, int message,
           Word& local, Supplier s){
  switch(message){
    case OPEN: {
      if(local.addr){
          toprelseq_LocalInfo<A1,A2,Proc>* li =
                 static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
          delete li;
       }
       const A1* a1;
       const A2* a2;
       if(!sym){
          a1 = static_cast<const A1*>(args[0].addr);
          a2 = static_cast<const A2*>(args[1].addr);
       } else {
          a1 = static_cast<const A1*>(args[1].addr);
          a2 = static_cast<const A2*>(args[0].addr);
       }
       ListExpr tt = GetTupleResultType(s);
       local.addr = new toprelseq_LocalInfo<A1,A2,Proc>(a1,a2,nl->Second(tt));
       return 0;
    }
    case REQUEST: {
      toprelseq_LocalInfo<A1,A2,Proc>* li =
                static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
      result.addr =  li->nextTopRelTuple();
      return result.addr? YIELD: CANCEL;
    }
    case CLOSE:{
      if(local.addr){
          toprelseq_LocalInfo<A1,A2,Proc>* li =
                 static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
          delete li;
          local.addr = 0;
       }
       return 0;
    }

  }
  return -1;
}



ValueMapping toprelseqVM[] = {
                toprelseq_fun<Point,   UPoint, MTopRelAlg_PUP, false>,
                toprelseq_fun<Point,   MPoint, MTopRelAlg_PMP, false>,
                toprelseq_fun<UPoint,  UPoint, MTopRelAlg_UPUP, false>,
                toprelseq_fun<UPoint,  MPoint, MTopRelAlg_UPMP, false>,
                toprelseq_fun<MPoint,  MPoint, MTopRelAlg_MPMP, false>,
                toprelseq_fun<Region,  UPoint, MTopRelAlg_RUP, false>,
                toprelseq_fun<Region,  MPoint, MTopRelAlg_RMP, false>,
                toprelseq_fun<Point,   UPoint, MTopRelAlg_UPP, true>,
                toprelseq_fun<Point,   MPoint, MTopRelAlg_MPP, true>,
                toprelseq_fun<UPoint,  MPoint, MTopRelAlg_MPUP, true>,
                toprelseq_fun<Region,  UPoint, MTopRelAlg_UPR, true>,
                toprelseq_fun<Region,  MPoint, MTopRelAlg_MPR, true>
            };


Operator toprelseq(
           "toprelseq",
           toprelseqSpec,
           12,
           toprelseqVM,
           toprelseqSelect,
           toprelseqTM);


/*
3. Operator clusterseq

3.2 Type Mapping

*/

ListExpr clusterseqTM(ListExpr args){
  string err = "expected point x {upoint, mpoint} x predicategroup  | "
               " upoint x upoint x predicategroup | "
               " upoint x mpoint x predicategroup |"
               " mpoint x mpoint x predicategroup |"
               " region x upoint x predicategroup |"
               " region x mpoint x predicategroup |"
               " upoint x point  x predicategroup |"
               " mpoint x point  x predicategroup |"
               " mpoint x upoint x predicategroup |"
               " upoint x region x predicategroup |"
               " mpoint x region x predicategroup ";
  int len = nl->ListLength(args);
  if((len!=3) ){
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  // the third argument must be a predicategroup
  if(!listutils::isSymbol(arg3,toprel::PredicateGroup::BasicType())){
      return listutils::typeError(err);
  }

  bool ok = false;
  ok = ok || (listutils::isSymbol(arg1, Point::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Point::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Region::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Region::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, Point::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, Point::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, Region::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, Region::BasicType()));

  if(ok){
    ListExpr attrList = nl->FiveElemList(
               nl->TwoElemList( nl->SymbolAtom("Start"),
                                nl->SymbolAtom(DateTime::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("End"),
                                nl->SymbolAtom(DateTime::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("LeftClosed"),
                                nl->SymbolAtom(CcBool::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("RightClosed"),
                                nl->SymbolAtom(CcBool::BasicType())),
               nl->TwoElemList( nl->SymbolAtom("Cluster"),
                                nl->SymbolAtom(toprel::Cluster::BasicType())));

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()), attrList));
  }

  return listutils::typeError(err);
}

/*
3.1 Value Mapping

We use the localInfo of the topredseq value mapping function.

*/

template<class A1, class A2, class Proc, bool sym>
int clusterseq_fun(Word* args, Word& result, int message,
           Word& local, Supplier s){
  switch(message){
    case OPEN: {
      if(local.addr){
          toprelseq_LocalInfo<A1,A2,Proc>* li =
                      static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
          delete li;
       }
       const A1* a1;
       const A2* a2;
       if(!sym){
         a1 = static_cast<const A1*>(args[0].addr);
         a2 = static_cast<const A2*>(args[1].addr);
       } else {
         a1 = static_cast<const A1*>(args[1].addr);
         a2 = static_cast<const A2*>(args[0].addr);
       }
       const toprel::PredicateGroup* pg=
                 static_cast<const toprel::PredicateGroup*>(args[2].addr);
       ListExpr tt = GetTupleResultType(s);
       local.addr = new toprelseq_LocalInfo<A1,A2,Proc>(a1,
                                                     a2,pg,nl->Second(tt));
       return 0;
    }
    case REQUEST: {
      toprelseq_LocalInfo<A1,A2,Proc>* li =
               static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
      result.addr =  li->nextClusterTuple();
      return result.addr? YIELD: CANCEL;
    }
    case CLOSE:{
      if(local.addr){
          toprelseq_LocalInfo<A1,A2,Proc>* li =
               static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
          delete li;
          local.addr = 0;
       }
       return 0;
    }
  }
  return -1;
}

/*
3.3 Array of Value Mapping functions and selection function

As selection function, we use the same as for toprelseq. Thus, only the
value mapping array is defined.

*/

ValueMapping clusterseqVM[] = {
      clusterseq_fun<Point,  UPoint, MTopRelAlg_PUP,  false>,
      clusterseq_fun<Point,  MPoint, MTopRelAlg_PMP,  false>,
      clusterseq_fun<UPoint, UPoint, MTopRelAlg_UPUP, false>,
      clusterseq_fun<UPoint, MPoint, MTopRelAlg_UPMP, false>,
      clusterseq_fun<MPoint, MPoint, MTopRelAlg_MPMP, false>,
      clusterseq_fun<Region, UPoint, MTopRelAlg_RUP,  false>,
      clusterseq_fun<Region, MPoint, MTopRelAlg_RMP,  false>,
      clusterseq_fun<Point,  UPoint, MTopRelAlg_UPP,  true>,
      clusterseq_fun<Point,  MPoint, MTopRelAlg_MPP,  true>,
      clusterseq_fun<UPoint, MPoint, MTopRelAlg_MPUP, true>,
      clusterseq_fun<Region, UPoint, MTopRelAlg_UPR, true>,
      clusterseq_fun<Region, MPoint, MTopRelAlg_MPR, true>,
   };


/*
3.4 Operator Specification


*/

const string clusterseqSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>point x {upoint, mpoint} x predicategroup | "
        "         upoint x upoint x predicategroup  | "
        " upoint x mpoint x predicategroup | "
        " mpoint x mpoint x predicategroup |"
        " region x upoint x predicategroup |"
        " region x mpoint x predicategroup |"
        " upoint x point  x predicategroup |"
        " mpoint x point  x predicategroup |"
        " mpoint x upoint x predicategroup |"
        " upoint x region x predicategroup |"
        " mpoint x region x predicategroup"
        " -> "
        "stream(tuple(Start instant, End instant,"
        " LeftClosed bool, RightClosed bool, TopRel cluster))  </text--->"
       "<text>toprelseq(_,_)</text--->"
       "<text>Returns the stream of topological "
       "relationships together with time intervals</text--->"
       "<text>query clusterseq(p, up) tconsume</text--->"
       ") )";

/*
3.5 Operator instance

*/

Operator clusterseq(
           "clusterseq",
           clusterseqSpec,
           12,
           clusterseqVM,
           toprelseqSelect,
           clusterseqTM);





/*
4 Operator checkTopRel

4.1 Type Mapping

The type mapping is stream(cluster) x dfa -> bool

*/

ListExpr checkTopRelTM(ListExpr args){
  string err = "stream(cluster) x dfa expected";
  if(nl->ListLength(args)!=2){
     return listutils::typeError(err);
  }
  ListExpr stream = nl->First(args);
  ListExpr dfa = nl->Second(args);
  if((nl->ListLength(stream) != 2) ||
     !listutils::isSymbol(nl->First(stream), Symbol::STREAM()) ||
     !listutils::isSymbol(nl->Second(stream), toprel::Cluster::BasicType())){
     return listutils::typeError(err);
  }
  if(!listutils::isSymbol(dfa,TopRelDfa::BasicType())){
     return listutils::typeError(err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
4.2 Value Mapping

*/

int checkTopRelVM(Word* args, Word& result, int message,
           Word& local, Supplier s){

   result = qp->ResultStorage(s);
   CcBool* res = static_cast<CcBool*>(result.addr);

   TopRelDfa* dfa = static_cast<TopRelDfa*>(args[1].addr);

   if(!dfa->isUsuable()){
     res->Set(false,false);
     return 0;
   }

   bool done = false;
   qp->Open(args[0].addr);
   Word clusterW;

   dfa->start();

   qp->Request(args[0].addr,clusterW);
   while(qp->Received(args[0].addr) && !done){
      toprel::Cluster* c =  static_cast<toprel::Cluster*>(clusterW.addr);
      done = !dfa->next(*c);
      if(!done){
         done = dfa->isError() || dfa->acceptsAll();
      }
      c->DeleteIfAllowed();
      if(!done){
        qp->Request(args[0].addr,clusterW);
      }
   }


   res->Set(true, dfa->isFinal());
   qp->Close(args[0].addr);
   return 0;
}

/*
4.3 Specification

*/

const string checkTopRelSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>"
        "   stream(cluster) x dfa -> bool "
        " </text--->"
       "<text> _ checktoprel[_] </text--->"
       "<text>Checks whether the finite automaton  "
       "accepts the stream of clusters</text--->"
       "<text>query clusterseq(thecenter, train7,stdpgroup())"
       " projecttransformstream[Cluster] checkTopRel[ createDfa(stdpgroup(),"
       " 'disjoint meet .* disjoint')]</text--->"
       ") )";

/*
4.4 Operator instance

*/

Operator checkTopRel(
           "checkTopRel",
           checkTopRelSpec,
           checkTopRelVM,
           Operator::SimpleSelect,
           checkTopRelTM);



/*
5 Operator ~mtoppred~

This operator gets two objects (spatial, at least one moving and
an TopRelDfa and checks whether the dfa accepts the topological
relationships between these objects. If one of the arguments is
undefined, also the result is undefined.


5.1 Type Mapping

*/
ListExpr mtoppredTM(ListExpr args){
  string err = "expected "
               " point  x upoint x mtoprel [x bool] | "
               " point  x mpoint x mtoprel [x bool] | "
               " upoint x upoint x mtoprel [x bool] | "
               " upoint x mpoint x mtoprel [x bool] |"
               " mpoint x mpoint x mtoprel [x bool] |"
               " region x upoint x mtoprel [x bool] |"
               " region x mpoint x mtoprel |x bool] |"
               " upoint x point  x mtoprel [x bool] | "
               " mpoint x point  x mtoprel [x bool] | "
               " mpoint x upoint x mtoprel [x bool] | "
               " upoint x region x mtoprel [x bool] | "
               " mpoint x region x mtoprel [x bool] | " ;
  int len = nl->ListLength(args);
  if((len!=3) && (len!=4) ){
      return listutils::typeError(err);
  }

  if((len==4) && !listutils::isSymbol(nl->Fourth(args),CcBool::BasicType())){
      return listutils::typeError(err);
  }


  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  // the third argument must be a predicategroup
  if(!listutils::isSymbol(arg3,TopRelDfa::BasicType())){
      return listutils::typeError(err);
  }

  bool ok = false;
  ok = ok || (listutils::isSymbol(arg1, Point::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Point::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Region::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, Region::BasicType()) &&
              listutils::isSymbol(arg2, MPoint::BasicType()));


  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, Point::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, Point::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, UPoint::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, UPoint::BasicType()) &&
              listutils::isSymbol(arg2, Region::BasicType()));

  ok = ok || (listutils::isSymbol(arg1, MPoint::BasicType()) &&
              listutils::isSymbol(arg2, Region::BasicType()));

  if(ok){
     return nl->SymbolAtom(CcBool::BasicType());
  }

  return listutils::typeError(err);
}


/*
5.2 Value Mapping

5.2.1 Value Mapping function

*/
template<class T1, class T2, class Proc, bool sym>
int mtoppredFun(Word* args, Word& result, int message,
           Word& local, Supplier s){

    result = qp->ResultStorage(s);
    CcBool* res = static_cast<CcBool*>(result.addr);
    const T1* arg1;
    const T2* arg2;
    if(!sym){
      arg1 = static_cast<T1*>(args[0].addr);
      arg2 = static_cast<T2*>(args[1].addr);
    } else {
      arg1 = static_cast<T1*>(args[1].addr);
      arg2 = static_cast<T2*>(args[0].addr);
    }

    TopRelDfa* dfa = static_cast<TopRelDfa*>(args[2].addr);

    if(!arg1->IsDefined() || !arg2->IsDefined() || !dfa->IsDefined()){
       res->SetDefined(false);
       return 0;
    }
    bool iter = false;
    bool step = false;
    if(qp->GetNoSons(s)==4){
      iter = true;
      CcBool* arg4 = static_cast<CcBool*>(args[3].addr);
      if(!arg4->IsDefined()){
        res->SetDefined(false);
        return 0;
      } else {
        step = arg4->GetBoolval();
      }
    }

    if(!dfa->isUsuable()){
       res->SetDefined(false);
       return 0;
    }

    Proc proc(arg1, arg2, dfa->getPredicateGroup());
    pair<Interval<Instant>, toprel::Cluster>
             p(Interval<Instant>(), toprel::Cluster(1));
    if(!iter){
       dfa->start();
       bool done = dfa->acceptsAll() || dfa->isError();
       while(proc.hasNextCluster() && !done){
           p = proc.nextCluster();
           done = ! dfa->next(p.second);
          if(!done){
              done = dfa->acceptsAll() || dfa->isError();
          }
       }
       res->Set(true,dfa->isFinal());
       return 0;
    } else {
      set<int> states;
      states.insert(dfa->getStartState());
      bool done = false;
      while(proc.hasNextCluster() && !done){
        p = proc.nextCluster();
        if(p.second.IsDefined()){ // ignore undefined clusters
          done = ! dfa->next(p.second, states,
                           step && (p.first.start == p.first.end));
        }
       }
       res->Set(true,dfa->isFinal(states));
       return 0;
    }
}

/*
5.2.2 Value Mapping Array

*/

ValueMapping mtoppredVM[] = {
            mtoppredFun<Point,  UPoint, MTopRelAlg_PUP, false>,
            mtoppredFun<Point,  MPoint, MTopRelAlg_PMP, false >,
            mtoppredFun<UPoint, UPoint, MTopRelAlg_UPUP, false >,
            mtoppredFun<UPoint, MPoint, MTopRelAlg_UPMP, false >,
            mtoppredFun<MPoint, MPoint, MTopRelAlg_MPMP, false >,
            mtoppredFun<Region, UPoint, MTopRelAlg_RUP, false>,
            mtoppredFun<Region, MPoint, MTopRelAlg_RMP, false>,
            mtoppredFun<Point,  UPoint, MTopRelAlg_UPP, true>,
            mtoppredFun<Point,  MPoint, MTopRelAlg_MPP, true>,
            mtoppredFun<UPoint, MPoint, MTopRelAlg_MPUP, true>,
            mtoppredFun<Region, UPoint, MTopRelAlg_UPR, true>,
            mtoppredFun<Region, MPoint, MTopRelAlg_MPR, true>
        };

/*
5.3 Selection Function

*/
int mtoppredSelect(ListExpr args){
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(listutils::isSymbol(arg1, Point::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 0;
  }
  if(listutils::isSymbol(arg1, Point::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 1;
  }
  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 2;
  }
  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 3;
  }
  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 4;
  }
  if(listutils::isSymbol(arg1, Region::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 5;
  }
  if(listutils::isSymbol(arg1, Region::BasicType()) &&
     listutils::isSymbol(arg2, MPoint::BasicType())){
     return 6;
  }
  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, Point::BasicType())){
     return 7;
  }
  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, Point::BasicType())){
     return 8;
  }
  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, UPoint::BasicType())){
     return 9;
  }
  if(listutils::isSymbol(arg1, UPoint::BasicType()) &&
     listutils::isSymbol(arg2, Region::BasicType())){
     return 10;
  }
  if(listutils::isSymbol(arg1, MPoint::BasicType()) &&
     listutils::isSymbol(arg2, Region::BasicType())){
     return 11;
  }
  return -1;
}


/*

5.4 Specification

*/
const string mtoppredSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>"
        " point  x upoint x mtoprel [x bool] | "
        " point  x mpoint x mtoprel [x bool] | "
        " upoint x upoint x mtoprel [x bool] | "
        " upoint x mpoint x mtoprel [x bool] | "
        " mpoint x mpoint x mtoprel [x bool] |"
        " region x upoint x mtoprel [x bool] |"
        " region x mpoint x mtoprel [x bool] |"
        " upoint x point  x mtoprel [x bool] |"
        " mpoint x point  x mtoprel [x bool] |"
        " mpoint x upoint x mtoprel [x bool] |"
        " upoint x region x mtoprel [x bool] |"
        " mpoint x region x mtoprel [x bool]"
        " ->  bool "
        "</text--->"
       "<text> mtoppred(_,_,_) </text--->"
       "<text>Checks whether the first two parameters "
       " have one of  moving topological relationship as "
       " defined in the third argument."
       "If the boolean parameter is omittet, the automaton works like an "
       "usual deterministic automaton. So a moving point defined for "
       "a connected interval witch is disjoint over its lifespan, will "
       "not be accepted by an automaton defined by 'disjoint disjoint'."
       "If the boolean parameter is present and set to be FALSE, "
       "The automaton works like an non-deterministic automaton. "
       "For an transition all states are the new states which are reachable "
       "by a sequence of simple transitions using the current symbol."
       "If the boolean parameter is set to be TRUE, the automaton works "
       "similar to a FALSE value of this argument. The only difference is "
       "that such sequence transistions are only made, if the interval of "
       "where the topological relationship holds is longer than a single "
       "instant."
       "</text--->"
       "<text> query mtoppred(thecenter, train7, "
       "createDfa(stdpgroup(), 'disjoint meet .*'))</text--->"
       ") )";


/*
3.5 Operator instance

*/

Operator mtoppred(
           "mtoppred",
           mtoppredSpec,
           12,
           mtoppredVM,
           mtoppredSelect,
           mtoppredTM);





/*
3 Algebra definition

*/
class MTopRelAlgebra: public Algebra{

  public:

    MTopRelAlgebra(){
       AddTypeConstructor( &topreldfa );
       topreldfa.AssociateKind(Kind::DATA());

       // add operators
       AddOperator(&createDfa);
       AddOperator(&toprelseq);
       AddOperator(&clusterseq);
       AddOperator(&checkTopRel);
       AddOperator(&mtoppred);

    }

    ~MTopRelAlgebra() {};

};

} // end of namepsace

/*
3 Initialization of the Algebra


*/
extern "C"
Algebra*
InitializeMTopRelAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new temporalalgebra::MTopRelAlgebra());
}




