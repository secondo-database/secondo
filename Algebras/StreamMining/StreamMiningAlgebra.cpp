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

  * createbloomfilter: stream(tuple(X)) x ATTR x int x real ->  bloomfilter
    Creates a Bloomfilter for a Stream with maximum error probability float and size int.  

  * bloomcontains: bloomfilter x T [->] bool
    Checks whether the provided Argument of Type T is present in the filter
  
  * createcountmin: stream(tuple(X)) x ATTR x real x real -> countmin
    Creates a Count-Min Sketch for a given Stream

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
#include "CountMinSketch.h"

#include <string>
#include <iostream>   
#include <vector>
#include <bitset>
#include <cmath>


using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

namespace eschbach {

/*

2 Algebra Implementation

2.1 Data Structures

2.1.1 Class ~BloomFilter~

*/


ScalableBloomFilter::ScalableBloomFilter
(const float inputFP, const size_t expectedInput) {
  defined = true; 
  //Initialisierung der für die C++ Implementierung benötigten Variablen 
  falsePositiveProbability = inputFP;
  expectedInserts = expectedInput;
  filterSize = optimalSize(expectedInput, inputFP);
  numHashfunctions = optimalHashes(expectedInput, filterSize);
  //initialize the vector with as many bits as you expect entries; 
  //values are standard initialized which means false for bool values
  filter.resize(filterSize);
  assert (numHashfunctions>0);
}

ScalableBloomFilter::ScalableBloomFilter(const ScalableBloomFilter& rhs) {
  defined = rhs.defined;
  falsePositiveProbability = rhs.falsePositiveProbability;
  expectedInserts = rhs.expectedInserts;
  numHashfunctions = rhs.numHashfunctions;
  filter = rhs.filter;
}

//Setter and Getter
bool
ScalableBloomFilter::getDefined() const {
  return defined;
}

void
ScalableBloomFilter::setDefined() {
  defined = true;
}

size_t
ScalableBloomFilter::getInserts() const{
  return expectedInserts;
}

float
ScalableBloomFilter::getFP() const{
  return falsePositiveProbability;
}

vector<bool> 
ScalableBloomFilter::getFilter() const{
  return filter;
}

void 
ScalableBloomFilter::setFilter(vector<bool> inputFilter) {
  filter = inputFilter;
}

bool
ScalableBloomFilter::getElement(size_t index) const{
  return filter[index];
}

void ScalableBloomFilter::setElement(size_t index, bool value) {
  filter[index] = value;
}

int
ScalableBloomFilter::getNumberHashes() const{
  return numHashfunctions;
}

size_t
ScalableBloomFilter::getFilterSize() const{
  return filterSize;
}



//Auxiliary Functions
void
ScalableBloomFilter::initialize(float fp, size_t entries) {
  defined = true;
  falsePositiveProbability = fp;
  expectedInserts = entries;
  filterSize = optimalSize(entries, fp);
  numHashfunctions = optimalHashes(entries, filterSize);
  filter.resize(filterSize);
}

size_t
ScalableBloomFilter::optimalSize(const long expectedInserts, 
                                const double fPProb) {
  size_t optimalSize = -expectedInserts*log(fPProb)/ pow(log(2),2);
  if (optimalSize < 1) {
    return 1; 
  }
  return optimalSize;
}

long
ScalableBloomFilter::optimalHashes(const long expectedInserts, 
                                   const long filterSize) {
  return (long) max(1, (int) round((long) filterSize/expectedInserts * log(2)));
}

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
  
  Word result = SetWord(Address(0));
  correct = false;
  NList list (instance); 

  if(list.length() != 3){
    cmsg.inFunError("expected three arguments");
    return result; 
  }

  NList first = list.first();
  NList second = list.second();
  NList third = list.third();
  NList index;

  if(!first.isReal() || !second.isInt() || !third.isList()) {
    cmsg.inFunError("expected three numbers");
    return result;
  } 

  if (third.first().isBool()) {
    float fp = first.realval();
    size_t inserts = second.intval();
    ScalableBloomFilter* bloom = new ScalableBloomFilter(fp, inserts);
    for (size_t i = 0; i < bloom->getFilterSize(); i++) {
      index = third.first();
      third.rest();
      bloom -> getFilter()[i] = index.boolval();
    }
  }
  return result;
}

//Out-Function to turn List Representation into Class Representation
ListExpr
ScalableBloomFilter::Out(ListExpr typeInfo, Word value) {
  ScalableBloomFilter* bloomfilter = 
                       static_cast<ScalableBloomFilter*> (value.addr);
  if(!bloomfilter -> getDefined()) {
    return listutils::getUndefined();
  }

  ListExpr elementList = nl -> OneElemList(nl->BoolAtom(
                              bloomfilter->getElement(0)));
  ListExpr last = elementList; 

  for (size_t i = 0; i < bloomfilter -> getFilter().size(); i++) {    
      last = nl -> Append(last, nl->BoolAtom(bloomfilter->getElement(i)));
  }

  ListExpr returnList = nl -> ThreeElemList(
                              nl -> RealAtom(bloomfilter->getFP()),
                              nl -> IntAtom(bloomfilter->getFilterSize()),
                              last);

  return returnList;
}

/*
Support Functions for Persistent Sorage
*/
Word
ScalableBloomFilter::Create( const ListExpr typeInfo )
{
  Word w; 
  w.addr = (new ScalableBloomFilter(0.1, 10));
  return w;
}

void
ScalableBloomFilter::Delete( const ListExpr typeInfo, Word& w )
{
  delete (ScalableBloomFilter*) w.addr;
  w.addr = 0;
}


bool
ScalableBloomFilter::Open(SmiRecord& valueRecord, size_t& offset, 
                         const ListExpr typeInfo, Word& value) 
{  
  float fp; 
  size_t inserts;
  size_t filterSize;
  bool filterElement;

  bool ok = valueRecord.Read (&fp, sizeof(float), offset);
  offset += sizeof(float);

  ok = ok && valueRecord.Read (&inserts, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ok = ok && valueRecord.Read (&filterSize, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ScalableBloomFilter* openBloom = new ScalableBloomFilter(fp, inserts);

  for (size_t i = 0;  i < filterSize; i++) {
    ok = ok && valueRecord.Read (&filterElement, sizeof(bool), offset);
    offset += sizeof(bool);
    openBloom -> setElement(i, filterElement);   
  }

  if (ok) {
    value.addr = openBloom;
  } else {
    value.addr =  0;
  }
  return true;
} 


bool 
ScalableBloomFilter::Save(SmiRecord & valueRecord , size_t & offset ,
const ListExpr typeInfo , Word & value) {
  ScalableBloomFilter* bloomFilter = static_cast<ScalableBloomFilter*>
                                    (value.addr);

  float fp = bloomFilter->getFP();
  size_t inserts = bloomFilter -> getInserts();
  size_t filterSize = bloomFilter -> getFilterSize();     
  bool filterElement;                                 

  bool ok = valueRecord.Write(&fp, sizeof(float), offset);
  offset+=sizeof(float);

  ok = ok && valueRecord.Write(&inserts, sizeof(size_t), offset);
  offset+=sizeof(size_t);
  
  ok = ok && valueRecord.Write(&filterSize, sizeof(size_t), offset);
  offset+=sizeof(size_t);
  
  for (size_t i = 0; i < filterSize; i++) {
    filterElement = bloomFilter->getElement(i);
    ok = ok && valueRecord.Write(&filterElement, sizeof(bool), offset);
    offset+=sizeof(bool);
  }
  return true;
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
  ScalableBloomFilter* oldFilter = (ScalableBloomFilter*) w.addr;
  return SetWord( new ScalableBloomFilter(*oldFilter));
}


/*
Type Description
*/

struct scalableBloomFilterInfo : ConstructorInfo {

  scalableBloomFilterInfo() {

    name         = ScalableBloomFilter::BasicType();
    signature    = "-> " + Kind::SIMPLE();
    typeExample  = ScalableBloomFilter::BasicType();
    listRep      =  "()";
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
    in = ScalableBloomFilter::In;
    out = ScalableBloomFilter::Out;
    create = ScalableBloomFilter::Create;
    deletion = ScalableBloomFilter::Delete;
    open = ScalableBloomFilter::Open;
    save = ScalableBloomFilter::Save;
    close = ScalableBloomFilter::Close;
    clone = ScalableBloomFilter::Clone;
  }
};

scalableBloomFilterInfo bi;
scalableBloomFilterFunctions bf;
TypeConstructor scalableBloomFilterTC( bi, bf );


/*
2.1.2 Class ~CountMinSketch~
*/

CountMinSketch::CountMinSketch
(const float epsilon, const float delta) {
  defined = true;
  eps = epsilon;
  this->delta = delta;
  width = ceil(exp(1)/eps);
  depth = ceil(log(1/delta));
  matrix.resize(depth);
  for (size_t i = 0; i < depth; i++)
    matrix[i].resize(width);
} 

CountMinSketch::CountMinSketch
(const CountMinSketch& rhs) {
  defined = rhs.defined;
  eps = rhs.eps;
  delta = rhs.delta; 
  width = rhs.width; 
  depth = rhs.depth;
  matrix = rhs.matrix;
  totalCount = rhs.totalCount;
}

//Setter and Getter
bool 
CountMinSketch::getDefined() {
  return defined;
}

size_t 
CountMinSketch::getTotalCount() {
  return totalCount;
}

size_t
CountMinSketch::getWidth() {
  return width;
}

size_t
CountMinSketch::getDepth() {
  return depth;
}

float
CountMinSketch::getEpsilon() {
  return eps;
}

float
CountMinSketch::getDelta() {
  return delta;
}

std::vector<std::vector<int>>
CountMinSketch::getMatrix() {
  return matrix;
}

//Auxiliary Functions
void 
CountMinSketch::initialize(float eps, float delt) {
  defined = true;
  this->eps = eps; 
  this->delta = delt; 
  width = ceil(exp(1)/eps);
  depth = ceil(log(1/delta));
  matrix.resize(depth);
  for (size_t i = 0; i < depth; i++)
    matrix[i].resize(width);
  totalCount = 0;
}

void 
CountMinSketch::increaseCount(vector<size_t> hashValues) {
  totalCount++;
  int hashValue;
  for (size_t i = 0; i < depth; i++) {
    hashValue = hashValues[i];
    matrix[i][hashValue] = matrix[i][hashValue] + 1;
  }
}

int 
CountMinSketch::estimateFrequency(vector<size_t> hashValues) {
  int minVal;
  int compareValue;

  //use the Hashvalues of the Searchelement as Index
  size_t hashedIndex = hashValues[0];

  //Assume that the first hash is the minimum Frequency
  minVal = matrix[0][hashedIndex];

  for (size_t i = 1; i < depth; i++) {
    hashedIndex = hashValues[i];
    compareValue = matrix[i][hashedIndex];
    minVal = minVal < compareValue ? minVal : compareValue;
  } 

  return minVal;
}

/*
~In~/~Out~ Functions
*/

//In-Function to turn List Representation into Class Representation
Word
CountMinSketch::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  
  Word result = SetWord(Address(0));
  correct = false;
  NList list (instance); 

  if(list.length() != 3){
    cmsg.inFunError("expected three arguments");
    return result; 
  }

  NList first = list.first();
  NList second = list.second();
  NList third = list.third();
  NList index;

  if(!first.isReal() || !second.isInt()) {
    cmsg.inFunError("expected two numbers");
    return result;
  } 

  if (!third.isList()) {
    cmsg.inFunError("Expected a List of Boolvalues");
  }

  if (third.first().isBool()) {
    float fp = first.realval();
    size_t inserts = second.intval();
    ScalableBloomFilter* bloom = new ScalableBloomFilter(fp, inserts);
    for (size_t i = 0; i < bloom->getFilterSize(); i++) {
      index = third.first();
      third.rest();
      bloom -> getFilter()[i] = index.boolval();
    }
  }
  return result;
}

//Out-Function (Dummy)
ListExpr
CountMinSketch::Out(ListExpr typeInfo, Word value) {
  CountMinSketch* cms = 
                       static_cast<CountMinSketch*> (value.addr);
  if(!cms -> getDefined()) {
    return listutils::getUndefined();
  }

  ListExpr elementList = nl -> OneElemList(nl->BoolAtom(0));
 

  return elementList;
}

/*
Support Functions for Persistent Sorage
*/
Word
CountMinSketch::Create( const ListExpr typeInfo )
{
  Word w; 
  w.addr = (new CountMinSketch(0.1, 0.5));
  return w;
}

void
CountMinSketch::Delete( const ListExpr typeInfo, Word& w )
{
  delete (CountMinSketch*) w.addr;
  w.addr = 0;
}

/* Save and Open 
bool
ScalableBloomFilter::Open(SmiRecord& valueRecord, size_t& offset, 
                         const ListExpr typeInfo, Word& value) 
{  
  float fp; 
  size_t inserts;
  size_t filterSize;
  bool filterElement;

  bool ok = valueRecord.Read (&fp, sizeof(float), offset);
  offset += sizeof(float);

  ok = ok && valueRecord.Read (&inserts, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ok = ok && valueRecord.Read (&filterSize, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ScalableBloomFilter* openBloom = new ScalableBloomFilter(fp, inserts);

  for (size_t i = 0;  i < filterSize; i++) {
    ok = ok && valueRecord.Read (&filterElement, sizeof(bool), offset);
    offset += sizeof(bool);
    openBloom -> setElement(i, filterElement);   
  }

  if (ok) {
    value.addr = openBloom;
  } else {
    value.addr =  0;
  }
  return true;
} 


bool 
ScalableBloomFilter::Save(SmiRecord & valueRecord , size_t & offset ,
const ListExpr typeInfo , Word & value) {
  ScalableBloomFilter* bloomFilter = static_cast<ScalableBloomFilter*>
                                    (value.addr);

  float fp = bloomFilter->getFP();
  size_t inserts = bloomFilter -> getInserts();
  size_t filterSize = bloomFilter -> getFilterSize();     
  bool filterElement;                                 

  bool ok = valueRecord.Write(&fp, sizeof(float), offset);
  offset+=sizeof(float);

  ok = ok && valueRecord.Write(&inserts, sizeof(size_t), offset);
  offset+=sizeof(size_t);
  
  ok = ok && valueRecord.Write(&filterSize, sizeof(size_t), offset);
  offset+=sizeof(size_t);
  
  for (size_t i = 0; i < filterSize; i++) {
    filterElement = bloomFilter->getElement(i);
    ok = ok && valueRecord.Write(&filterElement, sizeof(bool), offset);
    offset+=sizeof(bool);
  }
  return true;
}
*/

void
CountMinSketch::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<CountMinSketch*>( w.addr );
  w.addr = 0;
}


Word
CountMinSketch::Clone( const ListExpr typeInfo, const Word& w )
{
  CountMinSketch* oldSketch = (CountMinSketch*) w.addr;
  return SetWord( new CountMinSketch(*oldSketch));
}

/*
Type Description
*/

struct countMinSketchInfo : ConstructorInfo {

  countMinSketchInfo() {

    name         = CountMinSketch::BasicType();
    signature    = "-> " + Kind::SIMPLE();
    typeExample  = CountMinSketch::BasicType();
    listRep      =  "()";
    valueExample = "(4 12 2 8)";
    remarks      = "";
  }
};

/*
Creation of the Type Constructor Instance
*/

struct countMinSketchFunctions : 
       ConstructorFunctions<CountMinSketch> {

  countMinSketchFunctions()
  {
    in = CountMinSketch::In;
    out = CountMinSketch::Out;
    create = CountMinSketch::Create;
    deletion = CountMinSketch::Delete;
    //open = CountMinSketch::Open;
    //save = CountMinSketch::Save;
    close = CountMinSketch::Close;
    clone = CountMinSketch::Clone;
  }
};

countMinSketchInfo ci;
countMinSketchFunctions cf;
TypeConstructor countMinSketchTC( ci, cf );



/*
2.2 Type Mapping Functions

These functions check whether the correct argument types are supplied for an
operator; if so, returns a elementList expression for the result type, otherwise the
symbol ~typeerror~.


Type mapping for ~reservoir~ is

----    (stream T) x int -> (stream T)


Type mapping for ~tilted~ is

----    (stream T) x int -> (stream T)

Type mapping for ~bloom~ is

----    (stream(tuple(X)) x ATTR) x int x real -> bloomfilter

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
                             "as first argument");
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
2.2.2 Operator ~createbloomfilter~
*/
ListExpr
createbloomfilterTM( ListExpr args ) {
NList type(args);
NList streamtype = type.first().second();
NList appendList;

ListExpr a = nl->First(args);

ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 4){
    return NList::typeError("Operator createbloomfilter expects "
                            "four arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(a)){
    return NList::typeError( "Operator createbloomfilter expects a "
                             "Tuple Stream as first argument");
  }

  //test second argument for a valid Attribute Name
  if (!type.second().isSymbol()){
    return NList::typeError("Operator createbloomfilter expects a valid "
                            "Attribute Name as second argument");
  }

  // test third argument for real
  if(type.third() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator createbloomfilter expects a real "
                            "value as second argument");
  }

  //test fourth argument for int
  if(type.fourth() != NList(CcInt::BasicType())) {
    return NList::typeError("Operator createbloomfilter expects an int "
                            "value as third argument");
  }
  
  // stream elements must be in kind tuple (X) with X in DATA
  if(!(streamtype.hasLength(2)
          && streamtype.first().isSymbol(sym.TUPLE())
          && IsTupleDescription(streamtype.second().listExpr())
         )
          && !(am->CheckKind(Kind::DATA(),streamtype.listExpr(),errorInfo))){
      return NList::typeError("Operator createbloomfilter can only handle "
                              "Attributetype Tuple values");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr type2;
  string attrName = type.second().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, type2) - 1;

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }


  appendList.append(NList().intAtom(attrIndex));

  /* result is a bloomfilter and we append the index of 
     the attribute of the tuples which will be hashed to create our filter 
  */
  return NList(Symbols::APPEND(), appendList, 
               ScalableBloomFilter::BasicType()).listExpr();
}

/*
2.2.3 Operator ~bloomcontains~
*/
ListExpr
bloomcontainsTM(ListExpr args) {
  NList type(args); 
  // two arguments must be supplied
  if (type.length() != 2){
    return NList::typeError("Operator bloomcontains expects two arguments");
  }

  // test first argument for scalablebloomfilter
  if(type.first() != NList(ScalableBloomFilter::BasicType())){
    return NList::typeError("Operator bloomcontains expects a "
                            "Bloomfilter as first argument");
  }

  // test second argument for DATA or TUPLE
  if(type.second().isAtom()) {
    return NList(CcBool::BasicType()).listExpr(); 
  }    

  return NList::typeError("Operator bloomcontains expects an  " 
                          "ATTRIBUTE as second argument");
}


/*
2.2.4 Operator ~createcountmin~
*/
ListExpr
createcountminTM( ListExpr args ) {
NList type(args);
NList streamtype = type.first().second();
NList appendList;

ListExpr a = nl->First(args);

ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 4){
    return NList::typeError("Operator createcountmin expects "
                            "four arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(a)){
    return NList::typeError( "Operator createcountmin expects a "
                             "Tuple Stream as first argument");
  }

  //test second argument for a valid Attribute Name
  if (!type.second().isSymbol()){
    return NList::typeError("Operator createcountmin expects a valid "
                            "Attribute Name as second argument");
  }

  // test third argument for real
  if(type.third() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator createcountmin expects a real "
                            "value as second argument");
  }

  //test fourth argument for int
  if(type.fourth() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator createcountmin expects a real "
                            "value as third argument");
  }
  
  // stream elements must be in kind tuple (X) with X in DATA
  if(!(streamtype.hasLength(2)
          && streamtype.first().isSymbol(sym.TUPLE())
          && IsTupleDescription(streamtype.second().listExpr())
         )
          && !(am->CheckKind(Kind::DATA(),streamtype.listExpr(),errorInfo))){
      return NList::typeError("Operator createbloomfilter can only handle "
                              "Attributetype Tuple values");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr type2;
  string attrName = type.second().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, type2) - 1;

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }


  appendList.append(NList().intAtom(attrIndex));

  /* result is a bloomfilter and we append the index of 
     the attribute of the tuples which will be hashed to create our filter 
  */
  return NList(Symbols::APPEND(), appendList, 
               CountMinSketch::BasicType()).listExpr();
}

/*
2.2.5 Operator ~cmscount~
*/
ListExpr
cmscountTM(ListExpr args) {
  NList type(args); 
  // two arguments must be supplied
  if (type.length() != 2){
    return NList::typeError("Operator cmscount expects two arguments");
  }

  // test first argument for scalablebloomfilter
  if(type.first() != NList(CountMinSketch::BasicType())){
    return NList::typeError("Operator cmscount expects a "
                            "Count Min Sketch as first argument");
  }

  // test second argument for DATA or TUPLE
  if(type.second().isAtom()) {
    return NList(CcInt::BasicType()).listExpr(); 
  }    

  return NList::typeError("Operator cmscount expects an  " 
                          "ATTRIBUTE as second argument");
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
2.3.2 Operator ~createbloomfilter~
*/
int createbloomfilterVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CcReal* fpProb = (CcReal*) args[2].addr;
  CcInt* insertElements = (CcInt*) args[3].addr;
  CcInt* attrIndexPointer = (CcInt*) args[4].addr;

  int attrIndex = attrIndexPointer->GetIntval();

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

  cout << "After init() Bloom Filter Values are: " << endl;
  cout << "Defined: " + bloomFilter->getDefined() << endl;
  cout << "Nbr of Hashes: " + bloomFilter->getNumberHashes() << endl;
  cout << "FP: " << + bloomFilter->getFP() << endl;
  cout << "Expected Inserts " << + bloomFilter->getInserts() << endl;
  cout << "Filter Size: " << + bloomFilter->getFilterSize() << endl;


  //Get the stream provided by the operator
  Stream<Tuple> stream(args[0]);

  //open the stream 
  stream.open();
  
  //Pointers to stream elements will be saved here for use
  Tuple* streamTuple = (Tuple*) stream.request();

  Attribute* streamElement; 

  /*Get the size of the Filter so we can %mod the hash 
  results to map to an index in the filter*/
  size_t filterSize = bloomFilter->getFilterSize();

  /*Get number of Hashfunctions so reserving the hash results
  vector will be faster*/
  int nbrHashes = bloomFilter->getNumberHashes();
  vector<size_t> hashvalues;

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t mHash[2]; 

  //while the stream can still provide elements:
  while ((streamTuple != 0)) {
    hashvalues.reserve(nbrHashes);

    streamElement = (Attribute*) streamTuple->GetAttribute(attrIndex);
    
    /*64 Bit Version chosen, because of my System. 
    Should we change this 64 bit? */    
    MurmurHash3_x64_128(streamElement, sizeof(*streamElement), 0, mHash);
    size_t h1 = mHash[0] % filterSize;
    hashvalues.push_back(h1);

    //more than 1 Hash is needed (probably always the case)
    if (nbrHashes > 1) {
      size_t h2 = mHash[1] % filterSize;
      hashvalues.push_back(h2);
    
      //hash the streamelement for the appropriate number of times
      for (int i = 2; i < nbrHashes; i++) {
          size_t h_i = (h1 + i * h2 + i * i) % filterSize;
          //order of elements is irrelevant; must only be set in the  filter 
          hashvalues.push_back(h_i);
      }
    } 
    
    /*set the bits corresponding to the elements 
    hashed values in the bloomfilter*/
    bloomFilter->add(hashvalues);

    //delete old hashvalues from the vector
    hashvalues.clear();

    streamElement->DeleteIfAllowed();
    streamTuple->DeleteIfAllowed();


    //assign next Element from the Stream
    streamTuple = stream.request();   
  }
  
  stream.close();

  result.setAddr(bloomFilter);

  return 0;
}

/*
2.3.3 Operator ~bloomcontains~
*/
int bloomcontainsVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  ScalableBloomFilter* bloomFilter = (ScalableBloomFilter*) args[0].addr;
  Attribute* searchEle = (Attribute*) args[1].addr;

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcBool* b = (CcBool*) result.addr;

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t cmHash[2];

  //Take Size of the bloomFilter
  size_t filterSize = bloomFilter -> getFilterSize();

  //Take number of hashfunctions used by the bloomFilter
  int nbrHashes = bloomFilter -> getNumberHashes();

  //prepare a vector to take in the Hashresults
  vector<size_t> hashValues; 
  hashValues.reserve(nbrHashes);

  //hash the Searchelement
  MurmurHash3_x64_128(searchEle, sizeof(*searchEle), 0, cmHash);
  
  size_t h1 = cmHash[0]% filterSize;
  hashValues.push_back(h1);
  
  size_t h2 = cmHash[1] % filterSize;
  hashValues.push_back(h2);

  for (int i = 2; i < nbrHashes; i++) {
    size_t h_i = (h1 + i * h2 + i * i) % filterSize;
    //order of elements is irrelevant; must only be set in the  filter  
    hashValues.push_back(h_i);
  }

  bool contains = bloomFilter->contains(hashValues);
  b->Set(true, contains);

  return 0;
}

/*
2.3.4 Operator ~createcountmin~
*/
int createcountminVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CcReal* epsilon = (CcReal*) args[2].addr;
  CcReal* delta = (CcReal*) args[3].addr;
  CcInt* attrIndexPointer = (CcInt*) args[4].addr;

  int attrIndex = attrIndexPointer->GetIntval();

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CountMinSketch* cms = (CountMinSketch*) result.addr;

  //initialize the Filter with the values provided by the operator
  cms->initialize(epsilon->GetValue(),delta->GetValue());

  cout << "After init() CMS Values are: " << endl;
  cout << endl;
  cout << "Defined: " + cms->getDefined() << endl;
  cout << "Epsilon: " << + cms->getEpsilon() << endl;
  cout << "Delta: " << + cms->getDelta() << endl;
  cout << "Width: "  << endl;
  cout <<  cms->getWidth();
  cout << endl;
  cout << "Depth: " << + cms->getDepth() << endl;
  cout << "Total Count " << + cms->getTotalCount() << endl;

  cout << "VV enthält: " << cms->getMatrix().size() << endl;

  for (size_t i = 0; i < cms->getDepth(); i++) {
    cout << "V" << i << " length is: " << cms->getMatrix()[i].size() << endl;
  }


  //Get the stream provided by the operator
  Stream<Tuple> stream(args[0]);

  //open the stream 
  stream.open();
  
  //Pointers to stream elements will be saved here for use
  Tuple* streamTuple = (Tuple*) stream.request();

  Attribute* streamElement; 

  /*Get the size of the Filter so we can %mod the hash 
  results to map to an index in the filter*/
  size_t width = cms->getWidth();

  /*Get number of Hashfunctions so reserving the hash results
  vector will be faster*/
  int depth = cms->getDepth();

  //prepare a vector to take in the Hashresults
  vector<size_t> hashValues; 

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t mHash[2]; 

  //while the stream can still provide elements:
  while ((streamTuple != 0)) {

    streamElement = (Attribute*) streamTuple->GetAttribute(attrIndex);
    
    /*64 Bit Version chosen, because of my System. 
    Should we change this 64 bit? */    
    MurmurHash3_x64_128(streamElement, sizeof(*streamElement), 0, mHash);
    size_t h1 = mHash[0] % width;
    hashValues.push_back(h1);

    //more than 1 Hash is needed (probably always the case)
    if (depth > 1) {
      size_t h2 = mHash[1] % width;
      hashValues.push_back(h2);
      //hash the streamelement for the appropriate number of times
      for (int i = 2; i < depth; i++) {
          size_t h_i = (h1 + i * h2 + i * i) % width;
          hashValues.push_back(h_i);
      }
    } 

    /*Increase element Counter in the 
    Count Min Sketch*/
    cms->increaseCount(hashValues);
    
    //delete old hashvalues from the vector
    hashValues.clear();;

    streamElement->DeleteIfAllowed();
    streamTuple->DeleteIfAllowed();

    //assign next Element from the Stream
    streamTuple = stream.request();   
  }


  for (size_t i = 0; i < cms->getDepth(); i++) {
    cout << "Vector " << i << " looks like: " << endl; 
    for (size_t j = 0; j < cms->getWidth(); j++) {
       cout << cms->getMatrix()[i][j];
    }
    cout << endl;
  }
  cout << endl;
  cout << "Total Elements processed: " << cms->getTotalCount();
  
  stream.close();

  result.setAddr(cms);

  return 0;
}

/*
2.3.5 Operator ~cmsCount~
*/
int cmscountVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CountMinSketch* cms = (CountMinSketch*) args[0].addr;
  Attribute* searchEle = (Attribute*) args[1].addr;

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcInt* b = (CcInt*) result.addr;

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t cmHash[2];

  //Take Size of the bloomFilter
  size_t width = cms -> getWidth();

  //Take number of hashfunctions used by the bloomFilter
  int depth = cms -> getDepth();

  //prepare a vector to take in the Hashresults
  vector<size_t> hashValues; 
  hashValues.reserve(depth);

  //hash the Searchelement
  MurmurHash3_x64_128(searchEle, sizeof(*searchEle), 0, cmHash);
  
  size_t h1 = cmHash[0]% width;
  hashValues.push_back(h1);
  
  size_t h2 = cmHash[1] % width;
  hashValues.push_back(h2);

  for (int i = 2; i < depth; i++) {
    size_t h_i = (h1 + i * h2 + i * i) % width;
    //order of elements is irrelevant; must only be set in the  filter  
    hashValues.push_back(h_i);
  }
  
  int estimate = cms->estimateFrequency(hashValues);

  b->Set(true, estimate);

  return 0;
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

  OperatorSpec createbloomfilterSpec(
   "stream(tuple(X)) x ATTR x int x real ->  bloomfilter",
   "_ createbloomfilter [_,_,_]",
   "Creates a Bloomfilter of a supplied stream with the given ",
   "False Positive rate for the expected number of inserts",
   "query Kinos feed createbloomfilter[Name,0.01,100] bloomcontains[\"Astor\"]"
  );

  OperatorSpec bloomcontainsSpec(
   "scalablebloomfilter x T -> bool, T = TUPLE or T = DATA",
   "_ bloomcontains [_]",
   "Checks for the presence of Element T in a supplied Bloomfilter",
   "query Kinos feed createbloomfilter[Name,0.01,100] bloomcontains[\"Astor\"]"
  );

  OperatorSpec createcountminSpec(
   "stream(tuple(X)) x ATTR x int x real ->  countminsketch",
   "_ createcountminSpec [_,_,_]",
   "Creates Count Mint Sketch for the supplied stream",
   "query Kinos feed createcountmin[Name,0.01,0.9] cmscount[\"Astor\"]"
  );

  OperatorSpec cmscountSpec(
   "countminsketch x T -> bool, T = TUPLE or T = DATA",
   "_ bloomcontains [_]",
   "Gives an estimate of how often an Element appeared in the Stream the ",
   "Count Min Sketch was created for"
   "query Kinos feed createcountmin[Name,0.01,0.9] cmscount[\"Astor\"]"
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

Operator createbloomfilterOp(
  "createbloomfilter",
  createbloomfilterSpec.getStr(),
  createbloomfilterVM,
  Operator::SimpleSelect,
  createbloomfilterTM
);

Operator bloomcontainsOp(
  "bloomcontains",
  bloomcontainsSpec.getStr(),
  bloomcontainsVM,
  Operator::SimpleSelect, 
  bloomcontainsTM
);

Operator createcountminOp(
  "createcountmin",
  createcountminSpec.getStr(),
  createcountminVM,
  Operator::SimpleSelect, 
  createcountminTM
);


Operator cmscountOp(
  "cmscount",
  cmscountSpec.getStr(),
  cmscountVM,
  Operator::SimpleSelect, 
  cmscountTM
);


/*
2.6 The algebra class

*/

class StreamMiningAlgebra : public Algebra
{
 public:
  StreamMiningAlgebra() : Algebra()
  {

    //Reigstration of Types
    AddTypeConstructor(&scalableBloomFilterTC);
    AddTypeConstructor(&countMinSketchTC);

    //Usage possibilities of the Types
    scalableBloomFilterTC.AssociateKind(Kind::SIMPLE());
    countMinSketchTC.AssociateKind(Kind::SIMPLE());

    //Registration of Operators
    AddOperator(&reservoirOp);
    AddOperator(&createbloomfilterOp);
    AddOperator(&bloomcontainsOp);
    AddOperator(&createcountminOp);
    AddOperator(&cmscountOp);
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

