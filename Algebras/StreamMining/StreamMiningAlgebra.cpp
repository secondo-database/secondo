/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] title: [{\Large \bf ]   [}]
//[->] [$\rightarrow$]



[1] Stream Mining Algebra

October 2021, T. Eschbach implemented this Algebra as part of his Bachelor Thesis


0 Overview 

This algebra can be used to apply different Datamining techniques to Streams. 
It provides the following operators:

  * reservoir: stream x int [->] (stream)
    Creates a reservoir sample of size int for a stream

  * tilted: stream x int [->] (stream)
    Creates a tilted time frame sample for a stream. 
    The type (natural, logarithmic, progressive logarithmic) depends 
    on the provided int. 

  * bloom: stream x real x int [->] bloomfilter
    Creates a Bloomfilter for a Stream with maximum error probability float and size int.  

  * cbloom: bloomfilter x T [->] bool
    Checks whether the provided Argument of Type T is present in the filter

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
#include "ListUtils.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "MurmurHash.h"
#include "BloomFilter.h"

#include <string>
#include <iostream>   
#include <vector>
#include <cmath>


using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;


namespace eschbach {

/*

2 Algebra Implementation

2.1 Data Structure - Class ~BloomFilter~

2.1.1 The Class

*/


ScalableBloomFilter::ScalableBloomFilter
(const float falsePositiveProbability, const long expectedInput) {
  
  defined = true; 
  //Initialisierung der für die C++ Implementierung benötigten Variablen 
  this->falsePositiveProbability = falsePositiveProbability;
  expectedInserts = expectedInput;
  filterSize = optimalSize(expectedInserts, falsePositiveProbability);
  numHashfunctions = optimalHashes(expectedInserts, filterSize);
  //initialize the vector with as many bits as you expect entries; 
  //values are standard initialized which means false for bool values
  filter.resize(expectedInserts);
  assert (numHashfunctions>0);
}

ScalableBloomFilter::ScalableBloomFilter(const ScalableBloomFilter& rhs):
  falsePositiveProbability(rhs.falsePositiveProbability), 
  expectedInserts(rhs.expectedInserts){}

//Setter and Getter
bool
ScalableBloomFilter::getDefined() {
  return defined;
}

void
ScalableBloomFilter::setDefined() {
  defined = true;
}

vector<bool> 
ScalableBloomFilter::getFilter() {
  return filter;
}

bool
ScalableBloomFilter::getElement(size_t index) {
  return filter[index];
} 

int
ScalableBloomFilter::getNumberHashes() {
  return numHashfunctions;
}

size_t
ScalableBloomFilter::getFilterSize() {
  return filterSize;
}

//Auxiliary Functions
void
ScalableBloomFilter::initialize(float fp, size_t entries) {
  defined = true;
  numHashfunctions = optimalHashes(entries, filterSize);
  falsePositiveProbability = fp;
  expectedInserts = entries;
  filterSize = optimalSize(entries, fp);
  filter.resize(filterSize);
}

size_t
ScalableBloomFilter::optimalSize(const long expectedInserts, 
                                const double fPProb) {
  return (size_t) (-expectedInserts*log(fPProb)/ pow(log(2),2));
}

long
ScalableBloomFilter::optimalHashes(const long expectedInserts, 
                                   const long filterSize) {
  return (long) max(1, (int) round((long) filterSize/expectedInserts * log(2)));
}

//Create, Delete, Clone and Close must be implemented;
bool 
ScalableBloomFilter::contains(vector<size_t> hashResults) const {
  bool present = true;
  if (defined) {
    for (size_t index : hashResults) {
      if (!filter[index]) {
        present = false;
        break;
      }
    }
  }
  return present;
}

void 
ScalableBloomFilter::add(vector<size_t> hashResults) {
  if (defined) {
    for (size_t index : hashResults) {
      if (index > 0 && index < filterSize) {
            filter[index] = true;
      }
    }
  }
}

/* Wird erst für die scalable Version benötigt
bool
ScalableBloomFilter::isSaturated(const ScalableBloomFilter& b) {
  return ScalableBloomFilter::getInsertions() > ScalableBloomFilter::getInsertions() ; 
}
*/

/*
~In~/~Out~ Functions
*/

//In-Function to turn List Representation into Class Representation
Word
ScalableBloomFilter::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  ScalableBloomFilter* newBloomfilter;
  newBloomfilter = new ScalableBloomFilter(0.8, 10000);
  return SetWord(newBloomfilter);
}
  //Out-Function to turn List Representation into Class Representation

ListExpr
ScalableBloomFilter::Out(ListExpr typeInfo, Word value) {
  ScalableBloomFilter* bloomfilter = 
                       static_cast<ScalableBloomFilter*> (value.addr);
  if(!bloomfilter -> getDefined()) {
    return listutils::getUndefined();
  }

  ListExpr element; 
  ListExpr list = nl -> TheEmptyList();
  ListExpr last = nl -> TheEmptyList(); 

  for (size_t i = 0; i < bloomfilter -> getFilter().size(); i++) {
    element = bloomfilter -> getElement(i);
    
    if (i == 0) {
      list = nl -> OneElemList(element);
      last = list;
    } else {
      last = nl -> Append(last, element);
    }
  }
  return list;
}

/*
Support Functions for Persistent Sorage
*/
Word
ScalableBloomFilter::Create( const ListExpr typeInfo )
{
  return (SetWord( new ScalableBloomFilter( 0, 0 ) ));
}

void
ScalableBloomFilter::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<ScalableBloomFilter*>( w.addr );
  w.addr = 0;
}

void
ScalableBloomFilter::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<ScalableBloomFilter*>( w.addr );
  w.addr = 0;
}

Word
ScalableBloomFilter::Clone( const ListExpr typeInfo, const Word& w )
{
  ScalableBloomFilter* p = static_cast<ScalableBloomFilter*>( w.addr );
  return SetWord( new ScalableBloomFilter(*p) );
}


/*
Type Description
*/

struct scalableBloomFilterInfo : ConstructorInfo {

  scalableBloomFilterInfo() {

    name         = ScalableBloomFilter::BasicType();
    signature    = "-> " + Kind::SIMPLE();
    typeExample  = ScalableBloomFilter::BasicType();
    listRep      =  "(true, false, false, false, true)";
    valueExample = "(4 12 2 8)";
    remarks      = "";
  }
};

/*
Creation of the Type Constructor Instance
*/

struct scalableBloomFilterFunctions : 
       ConstructorFunctions<ScalableBloomFilter> {

  scalableBloomFilterFunctions()
  {
    create = ScalableBloomFilter::Create;
    in = ScalableBloomFilter::In;
    out = ScalableBloomFilter::Out;
    deletion = ScalableBloomFilter::Delete;
    clone = ScalableBloomFilter::Clone;
    close = ScalableBloomFilter::Close;
  }
};

scalableBloomFilterInfo bi;
scalableBloomFilterFunctions bf;
TypeConstructor scalableBloomFilterTC( bi, bf );

/*
2.2 Type Mapping Functions

These functions check whether the correct argument types are supplied for an
operator; if so, returns a list expression for the result type, otherwise the
symbol ~typeerror~.


Type mapping for ~reservoir~ is

----    (stream T) x int -> (stream T)


Type mapping for ~tilted~ is

----    (stream T) x int -> (stream T)

Type mapping for ~bloom~ is

----    (stream T) x real x int -> scalablebloomfilter

Type mapping for ~cbloom~ is
----    scalablebloomfilter x T -> bool
*/

/*
2.2.1 Operator ~reservoir~
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

  // result is the type of the first argument
  return type.first().listExpr();
}

/*
2.2.2 Operator ~bloom~
*/
ListExpr
bloomTM( ListExpr args ) {
NList type(args);
ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 3){
    return NList::typeError("Operator bloom expects three arguments");
  }

  // test first argument for stream
  if(!(   type.first().hasLength(2)
       && type.first().first().isSymbol(sym.STREAM()))){
    return NList::typeError( "Operator reservoir expects a stream "
                             " as first argument");
  }

  // test second argument for int
  if(type.second() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator reservoir expects a real "
                            "as second argument");
  }

    if(type.third() != NList(CcInt::BasicType())) {
    return NList::typeError("Operator reservoir expects an int "
                            "as third argument");
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



  // result is a bloomfilter
  return nl->OneElemList(nl->SymbolAtom(ScalableBloomFilter::BasicType()));
}

/*
2.2.3 Operator ~cbloom~
*/
ListExpr
cbloomTM(ListExpr args) {
  NList type(args); 
  // two arguments must be supplied
  if (type.length() != 2){
    return NList::typeError("Operator cbloom expects two arguments");
  }

  // test first argument for scalablebloomfilter
  if(type.first() != NList(ScalableBloomFilter::BasicType())){
    return NList::typeError( "Operator cbloom expects a "
                            "Bloomfilter as first argument");
  }

  // test second argument for DATA or TUPLE
  if(!(type.second().isAtom()) || 
     listutils::isTupleStream(nl->Second(nl->Second(args)))) { 
    return NList::typeError("Operator reservoir expects a TUPLE " 
                            "or DATA type as second argument");
  }
  return NList(CcBool::BasicType()).listExpr();
}


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
      case REQUEST : result.addr = li?li->next():0;
                     return result.addr?YIELD:CANCEL;
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
2.3.2 Operator ~bloom~
*/
template<class T>
int bloomVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CcReal* fpProb = (CcReal*) args[1].addr;
  CcInt* insertElements = (CcInt*) args[2].addr;

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  ScalableBloomFilter* bloomFilter = (ScalableBloomFilter*) result.addr;

  /* Implementieren, dass Filter Instanz gereinigt wird, wenn er bereits besteht
  if (bloomFilter->getFilter.size() > 0) {
    bloomFilter->Clear();
  }
  */

  //initialize the Filter with the values provided by the operator
  bloomFilter->initialize(fpProb->GetValue(),insertElements->GetValue());

  //Get the stream provided by the operator
  Stream<T> stream(args[0]);

  //open the stream 
  stream.open();
  
  //Stream Elements will be saved here for use
  T* streamElement;

  //Streamelement hashvalues will be stored to be processed in one go
  vector<size_t> hashvalues;
  hashvalues.resize(bloomFilter->getNumberHashes());
  
  /*Get the size of the Filter so we can %mod the hash 
  results to map to an index in the filter*/
  size_t filterSize = bloomFilter->getFilterSize();

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t mHash[2]; 

  
  //while the stream can still provide elements:
  while ((streamElement = stream.request())) {
    
    //Use the secondo Default Hashfunction to calculate h1    
    size_t h1 = streamElement -> HashValue() % filterSize;
    hashvalues.push_back(h1);

    /*64 Bit Version chosen, because of my System. 
    Should we change this 64 bit? */
    MurmurHash3_x64_128(streamElement, sizeof(streamElement), 0, mHash);
    size_t h2 = mHash[0] % filterSize;
    
    //hash the streamelement for the appropriate number of times
    for (int i = 0; i < bloomFilter->getNumberHashes(); i++) {
      /*dereference the pointer of the Streamelement to 
      hash Streamelements themselves and compute h_i according 
      to Mitzenmacher and Kirschbaum*/
      int elemHashValue = (h1 + i * h2 + i * i) % filterSize;
      //order of elements is irrelevant; must only be set in the  filter  
      hashvalues.push_back(elemHashValue);
    }
    
    streamElement->DeleteIfAllowed();
    /*set the bits corresponding to the elements 
    hashed values in the bloomfilter*/
    bloomFilter->add(hashvalues);
  }
  
  stream.close();

  result.setAddr(bloomFilter);

  return 0;
}

//value Mapping Array
ValueMapping bloomVM[] = { 
              bloomVMT<Tuple>,
              bloomVMT<Attribute>
};  

// Selection Function
int bloomSelect(ListExpr args){
   if (Stream<Attribute>::checkType(nl->First(args))) {
     return 1;
   } else if (Stream<Tuple>::checkType(nl->First(args))){
     return 0;
   } else {
     return -1;
   }
}

/*
2.3.3 Operator ~cbloom~
*/

template<class T>
int cbloomVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  ScalableBloomFilter* bloomFilter = (ScalableBloomFilter*) args[0].addr;
  T* searchEle = (T*) args[1].addr;

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcBool* b = (CcBool*) result.addr;

  //prepare a vector to take in the Hashresults
  vector<size_t> hashResults; 

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t mHash[2]; 

  //Take Size of the bloomFilter
  size_t filterSize = bloomFilter -> getFilterSize();

  int h1 = searchEle -> HashValue() % filterSize;
  
  MurmurHash3_x64_128(searchEle, sizeof(searchEle), 0, mHash);
  int h2 = mHash[0] % filterSize;

  for (int i = 0; i < bloomFilter -> getNumberHashes(); i++) {
    int elemHashValue = (h1 + i * h2 + i * i) % filterSize;
    //order of elements is irrelevant; must only be set in the  filter  
    hashResults.push_back(elemHashValue);
  }

  b->Set(true, bloomFilter->contains(hashResults));

  return 0;
}

//value Mapping Array
ValueMapping cbloomVM[] = { 
             cbloomVMT<Tuple>,
             cbloomVMT<Attribute>
};  

// Selection Function
int cbloomSelect(ListExpr args){
  NList type(args);
   if (type.second().isAtom()) {
     return 1;
   } else if (nl->SymbolAtom(sym.TUPLE())){
     return 0;
   } else {
     return -1;
   }
}


/*
2.4 Description of Operators

*/

  OperatorSpec reservoirSpec(
   "stream(X) x int -> stream(T), T = TUPLE or T = DATA",
   "_ reservoir [_] ",
   "Creates a reservoir sample of a supplied stream of a given size ",
   "query intstream(1,1000) reservoir[10] count"
  );

  OperatorSpec bloomSpec(
   "stream(X) x int -> bloom filter",
   "_ bloom [_ , _]",
   "Creates a Bloomfilter of a supplied stream",
   "query intstream(1,10000) bloom[10, 0.001]"
  );

  OperatorSpec cbloomSpec(
   "scalablebloomfilter x T -> bool, T = TUPLE or T = DATA",
   "_ cbloom [_]",
   "Checks for the presence of Element T in a supplied Bloomfilter",
   "query cbloom([const scalablebloomfilter value (10, 0.001)])"
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

Operator bloomOp(
  "bloom",
  bloomSpec.getStr(),
  2,
  bloomVM,
  bloomSelect, 
  bloomTM
);

Operator cbloomOp(
  "cbloom",
  cbloomSpec.getStr(),
  2,
  cbloomVM,
  cbloomSelect, 
  cbloomTM
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
    AddOperator(&bloomOp);
    AddOperator(&cbloomOp);
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

