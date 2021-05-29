/*
0 Overview

This algebra can be used to apply different Datamining techniques to Streams. 

It provides the following operators:

  * reservoir: stream x int [->] (stream)
    Creates a reservoir sample of size int for a stream

1 Preliminaries

1.1 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "Stream.h"
#include "NList.h"
#include "ListUtils.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"


#include <string>
#include <iostream>    

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace eschbach {

/*

2 Algebra Implementation

2.2 Type Mapping Functions

These functions check whether the correct argument types are supplied for an
operator; if so, returns a list expression for the result type, otherwise the
symbol ~typeerror~.


Type mapping for ~reservoir~ is

----    (stream T) x int -> (stream T)
----

*/

ListExpr
reservoirTM( ListExpr args ) {
NList type(args);
ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // two arguments must be supplied
  if (type.length() != 2){
    return NList::typeError("Operator reservoir expects two arguments");
  }

  // test first argument for stream
  if(!(   type.first().hasLength(2)
       && type.first().first().isSymbol(sym.STREAM()))){
    return NList::typeError( "Operator reservoir expects a stream "
                             " as first argument");
  }

  // test second argument for int
  if(type.second() != NList(CcInt::BasicType())) {
    return NList::typeError("Operator reservoir expects an int "
                            "as second argument");
  }
  
  // stream elements must be in kind DATA or (tuple X)
  NList streamtype = type.first().second();
  if(   !(   streamtype.hasLength(2)
          && streamtype.first().isSymbol(sym.TUPLE())
          && IsTupleDescription(streamtype.second().listExpr())
         )
     && !(am->CheckKind(Kind::DATA(),streamtype.listExpr(),errorInfo))){
      return NList::typeError("Operator reservoir expects a "
                              "stream of DATA or TUPLE.");
  }

  // result is the type of the second argument
  return type.first().listExpr();
}


/** Implemenation with the old NestedList
 * 
 * ListExpr
reservoirStreamType( ListExpr args ) {

  if ( nl->ListLength(args) != 2 ){
    return listutils::typeError("Operator reservoir expects two arguments");
  }

  // test first argument for stream(T)
  if(!(Stream<Attribute>::checkType(nl->First(args))
       || Stream<Tuple>::checkType(nl->First(args)))){
    return listutils::typeError("Operator reservoir expects a (stream T), "
                                "T in kind DATA as its first argument.");
  }

  // test second argument for int
  if(!CcInt::checkType(nl->Second(args))) {
    return listutils::typeError("Operator reservoir expects an int as second argument");
  }

  // result is the type of the second argument
  return nl->First(args);
}*/

/*
2.3 Value Mapping Functions

2.3.1 Operator ~reservoir~

Creates a reservoir Sample (stream) of the passed stream.

*/

/*Templates are used to deal with the different Types of Streams 
the operator handles*/
template<class T> class reservoirInfo{
  public: 
    reservoirInfo(Word inputStream, size_t inputSampleSize): 
                  stream(inputStream), sampleSize(inputSampleSize), 
counter(0),lastOut(-1) {
    stream.open();
    init();                               
}
~reservoirInfo() {
  for(size_t index = lastOut+1; index < reservoir.size(); index++) {
    reservoir[index]->DeleteIfAllowed(); 
  }
  stream.close();
}

//Returns the Elements in the reservoir in case of requests
T* next() {
  lastOut++; 
  if(lastOut >= (int)reservoir.size()) {
    return 0;
  }
  T* resElement = reservoir[lastOut];
  reservoir[lastOut] = 0;
  return resElement;
}

private: 
  Stream<T> stream; 
  size_t sampleSize;
  size_t counter;
  int lastOut;
  std::vector<T*> reservoir;

  void init() {
    T* data;
    //While the Argumentstream can still supply Data/Tuples 
    while ((data = stream.request()) != nullptr) {
      /*increase the counter to keep track of how many Arguments 
      have been passed by the argument stream*/
      counter++;
      //Decide whether to include data in the reservoir
      insert(data);
    } 
  }

  //Decides whether to include Data/Tuples from the Stream into the reservoir
  void insert(T* data) {
    /*Initialize the reservoir with Elements until it is 
    filled for the first time*/
    if(reservoir.size() < sampleSize) {
      reservoir.push_back(data);
      return;
    }
    /*Once the reservoir is filled use Algorithm R to 
    determine Replacement of Elements*/
    size_t rnd = rand() % (counter+1);
    if (rnd < reservoir.size()) {
      reservoir[rnd]->DeleteIfAllowed();
      reservoir[rnd] = data; 
    } else {
      data -> DeleteIfAllowed();
    }
  }
};

template<class T>
int reservoirVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){

  reservoirInfo<T>* li = (reservoirInfo<T>*) local.addr;
  switch(message){
    case OPEN: {
                   if(li) {
                     delete li;
                     local.addr = 0;
                   }
                   CcInt* reservoirSize = (CcInt*) args[1].addr;
                   if(reservoirSize->IsDefined()){
                      int size = reservoirSize->GetValue();
                      if(size>0) {
                        local.addr = new reservoirInfo<T>(args[0], size);
                      }
                   }
                   return 0;
                } 
      case REQUEST : {
        if(li) {
          result.addr = li->next(); 
          return YIELD;
        } else {
          result.addr = 0; 
          return CANCEL;
        }
      }

      case CLOSE : {
                      if(li){
                        delete li;
                        local.addr = 0;
                       }
                       return 0;
                   }

  }
  return -1;
}

//value Mapping Array
ValueMapping reservoirVM[] = { 
              reservoirVMT<Tuple>,
              reservoirVMT<Attribute>
};  

// Selection Function
int reservoirSelect(ListExpr args){
   if (Stream<Attribute>::checkType(nl->First(args))) {
     return 1;
   } else if (Stream<Tuple>::checkType(nl->First(args))){
     return 0;
   } else {
     return -1;
   }
}

/*
2.4 Description of Operators

*/

  OperatorSpec reservoirSpec(
   "stream(X) x int -> stream(X), X = TUPLE or X = DATA",
   "_ reservoir [_] ",
   "Creates a reservoir sample of a supplied stream of a given size ",
   "query intstream(1,1000) reservoir[10] count"
  );


/*
2.5 Operator Instances

*/

Operator reservoirOp(
  "reservoir",
  reservoirSpec.getStr(),
  2,
  reservoirVM,
  reservoirSelect, 
  reservoirTM
);


/*
2.6 The algebra class

*/

class StreamMiningAlgebra : public Algebra
{
 public:
  StreamMiningAlgebra() : Algebra()
  {
    AddOperator(&reservoirOp);
  }
  ~StreamMiningAlgebra() {};
};

} // end of namespace eschbach

/*
3 Initialization

*/

extern "C"
Algebra*
InitializeStreamMiningAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new eschbach::StreamMiningAlgebra);
}

