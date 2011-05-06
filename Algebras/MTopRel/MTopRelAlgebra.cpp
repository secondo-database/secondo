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
                " mpoint x mpoint"; 
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

    return nl->TwoElemList(nl->SymbolAtom("stream"),
                nl->TwoElemList( nl->SymbolAtom("tuple"), attrList));
                           
  }

  return listutils::typeError(err);
}





const string toprelseqSpec = 
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>point x {upoint, mpoint}  | "
        "        upoint x upoint  | "
        "        upoint x mpoint  | "
        "        mpoint x mpoint   "
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

  return -1; 

}


template<class A1, class A2, class Proc>
int toprelseq_fun(Word* args, Word& result, int message,
           Word& local, Supplier s){
  switch(message){
    case OPEN: {
      if(local.addr){
          toprelseq_LocalInfo<A1,A2,Proc>* li = 
                 static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
          delete li;
       }
       const A1* a1 = static_cast<const A1*>(args[0].addr);
       const A2* a2 = static_cast<const A2*>(args[1].addr);
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



ValueMapping toprelseqVM[] = {toprelseq_fun<Point,  UPoint, MTopRelAlg_PUP>,
                              toprelseq_fun<Point,  MPoint, MTopRelAlg_PMP>, 
                              toprelseq_fun<UPoint, UPoint, MTopRelAlg_UPUP>,
                              toprelseq_fun<UPoint, MPoint, MTopRelAlg_UPMP>,
                              toprelseq_fun<MPoint, MPoint, MTopRelAlg_MPMP> };


Operator toprelseq(
           "toprelseq",
           toprelseqSpec,
           5,
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
               " mpoint x mpoint x predicategroup";
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

    return nl->TwoElemList(nl->SymbolAtom("stream"),
                nl->TwoElemList( nl->SymbolAtom("tuple"), attrList));
  }

  return listutils::typeError(err);
}

/*
3.1 Value Mapping

We use the localInfo of the topredseq value mapping function.

*/

template<class A1, class A2, class Proc>
int clusterseq_fun(Word* args, Word& result, int message,
           Word& local, Supplier s){
  switch(message){
    case OPEN: {
      if(local.addr){
          toprelseq_LocalInfo<A1,A2,Proc>* li = 
                      static_cast<toprelseq_LocalInfo<A1,A2,Proc>*>(local.addr);
          delete li;
       }
       const A1* a1 = static_cast<const A1*>(args[0].addr);
       const A2* a2 = static_cast<const A2*>(args[1].addr);
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
      clusterseq_fun<Point, UPoint, MTopRelAlg_PUP>,
      clusterseq_fun<Point, MPoint, MTopRelAlg_PMP>,
      clusterseq_fun<UPoint, UPoint, MTopRelAlg_UPUP>, 
      clusterseq_fun<UPoint, MPoint, MTopRelAlg_UPMP>,
      clusterseq_fun<MPoint, MPoint, MTopRelAlg_MPMP> };


/*
3.4 Operator Specification


*/

const string clusterseqSpec = 
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>point x {upoint, mpoint} x predicategroup | "
        "         upoint x upoint x predicategroup  | "
        " upoint x mpoint x predicategroup | "
        " mpoint x mpoint x predicategroup"
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
           4,
           clusterseqVM,
           toprelseqSelect,
           clusterseqTM);




/*
3 Algebra definition

*/
class MTopRelAlgebra: public Algebra{

  public:

    MTopRelAlgebra(){
       AddTypeConstructor( &topreldfa );
       topreldfa.AssociateKind("DATA");

       // add operators
       AddOperator(&createDfa);
       AddOperator(&toprelseq);
       AddOperator(&clusterseq);

    }
    
    ~MTopRelAlgebra() {};

};

/*
3 Initialization of the Algebra


*/
extern "C"
Algebra*
InitializeMTopRelAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new MTopRelAlgebra());
}




