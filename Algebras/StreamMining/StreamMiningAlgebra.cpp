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

  * createbloomfilter: stream(tuple(X)) x ATTR x real ->  bloomfilter
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
#include "amsSketch.h"

#include <string>
#include <iostream>   
#include <vector>
#include <cmath>
#include <time.h>


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
(const double inputFP) {
  defined = true; 
  currentInserts = 0;
  curFilterIndex = 0;
  falsePositiveProbability = inputFP;
  rollingFP = inputFP;
  
  //Start out with a smaller filter, so not too much space is wasted
  maxInserts = DEFAULT_SIZE;
  filterSize = optimalSize(maxInserts, inputFP);
  numHashfunctions = optimalHashes(maxInserts, filterSize);

  //initialize the vector with as many bits as you expect entries; 
  //values are standard initialized which means false for bool values
  filterList.resize(1); 
  filterList[0].resize(filterSize);
  assert (numHashfunctions>0);
}

ScalableBloomFilter::ScalableBloomFilter(const ScalableBloomFilter& rhs) {
  defined = rhs.defined;
  falsePositiveProbability = rhs.falsePositiveProbability;
  maxInserts = rhs.maxInserts;
  currentInserts = rhs.currentInserts;
  numHashfunctions = rhs.numHashfunctions;
  ithFilterHashes = rhs.ithFilterHashes;
  filterList = rhs.filterList;
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
ScalableBloomFilter::getCurMaxInserts() const{
  return maxInserts;
}

double
ScalableBloomFilter::getFP() const{
  return falsePositiveProbability;
}

double 
ScalableBloomFilter::getRolFP() {
  return rollingFP;
}

vector<bool> 
ScalableBloomFilter::getSubFilter(size_t index){
  return filterList[index];
}

void 
ScalableBloomFilter::setSubFilter(vector<bool> inputSubFilter) {
  filterList.push_back(inputSubFilter);
}

vector<vector<bool>>
ScalableBloomFilter::getFilterList() {
  return filterList;
} 

bool
ScalableBloomFilter::getElement(size_t filterIndex, size_t eleIndex) const{
  return filterList[filterIndex][eleIndex];
}

bool 
ScalableBloomFilter::setElement(size_t filterIndex, 
                                size_t eleIndex, bool value) {
  //Assign the previous Bitstate to use it in determining the fill ratio 
  bool oldValue = filterList[filterIndex][eleIndex];
  filterList[filterIndex][eleIndex] = value;
  return oldValue;
}

void 
ScalableBloomFilter::setElementOpen(size_t filterIndex, 
                                    size_t eleIndex, bool value) {
  filterList[filterIndex][eleIndex] = value;
}

void
ScalableBloomFilter::setFilterHashes(vector<int> nbrHashes) {
  for (int nbr : nbrHashes) {
    ithFilterHashes.push_back(nbr);
  }
}

int
ScalableBloomFilter::getCurNumberHashes() const{
  return numHashfunctions;
}

vector<int> 
ScalableBloomFilter::getFilterHashes() const{
  return ithFilterHashes;
}

size_t
ScalableBloomFilter::getCurFilterSize() const{
  return filterList.back().size();
}

size_t
ScalableBloomFilter::getBloomSize() const {
  return filterList.size(); 
}



//Auxiliary Functions
void
ScalableBloomFilter::initialize(double fp) {
  defined = true;
  falsePositiveProbability = fp;
  rollingFP = fp;
  maxInserts = DEFAULT_SIZE;
  filterSize = optimalSize(maxInserts, fp);
  numHashfunctions = optimalHashes(maxInserts, filterSize);
  filterList.resize(1);
  filterList[0].resize(filterSize);
  ithFilterHashes.push_back(numHashfunctions);
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
ScalableBloomFilter::contains(vector<size_t> hashResults, 
                              int filterIndex) const {
  bool present = true;    
  if (defined) {
    for (size_t index : hashResults) {
      if (!filterList[filterIndex][index]) {
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
  //Use this Value to determine if adding an elements Hashvalues increased
  //the filters Fillrate 
  bool alreadyAdded = true;
    for (size_t eleIndex : hashResults) {
      if (eleIndex > 0 && eleIndex < filterSize) {
        alreadyAdded &= setElement(curFilterIndex, eleIndex, true);
      }
    }
    if (!alreadyAdded) {
      currentInserts++;
    }
  }
}

bool
ScalableBloomFilter::isSaturated() {
  return currentInserts >= maxInserts; 
}

// Update the parameters and and a new Filter to our Scalable Bloom
void
ScalableBloomFilter::updateFilterValues() {
  cout << endl;
  cout << "Updating filter Values; ";
  curFilterIndex++;
  maxInserts *= GROWTH_RATE;
  currentInserts = 0; 
  rollingFP *= TIGHTENING_RATIO;
  filterSize = optimalSize(maxInserts,rollingFP);
  numHashfunctions = optimalHashes(maxInserts, filterSize);
  filterList.resize(curFilterIndex+1);
  filterList.back().resize(filterSize);
  ithFilterHashes.push_back(numHashfunctions);
  cout << "Filter now: " << endl;
  cout << "Current Filter Index: " << curFilterIndex << endl;
  cout << "Subfilters: " << filterList.size() << endl;
  cout << "Current Filter Size: " << filterSize << endl;
  cout << "Current Hashes: " << numHashfunctions << endl;
  cout << "Previous Amount of Hashes in NbrHashesVector: " 
       << ithFilterHashes[curFilterIndex-1] << endl;
  cout << "Total Inserts: " << currentInserts << endl;

}


//~In~/~Out~ Functions

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
    size_t curFilterSize = second.intval();
    ScalableBloomFilter* bloom = new ScalableBloomFilter(fp);
    for (size_t i = 0; i < curFilterSize; i++) {
      index = third.first();
      third.rest();
      bloom -> getSubFilter(0)[i] = index.boolval();
    }
  }
  return result;
}

//Out-Function to turn List Representation into Class Representation
//Currently Dummy

ListExpr
ScalableBloomFilter::Out(ListExpr typeInfo, Word value) {
  ScalableBloomFilter* bloomfilter = 
                       static_cast<ScalableBloomFilter*> (value.addr);
  if(!bloomfilter -> getDefined()) {
    return listutils::getUndefined();
  }

  ListExpr elementList = nl -> OneElemList(nl->BoolAtom(
                              bloomfilter->getElement(0,0)));
  ListExpr last = elementList; 

  for (size_t i = 0; i < bloomfilter -> getSubFilter(0).size(); i++) {    
      last = nl -> Append(last, nl->BoolAtom(bloomfilter->getElement(i,i)));
  }

  ListExpr returnList = nl -> ThreeElemList(
                              nl -> RealAtom(bloomfilter->getFP()),
                              nl -> IntAtom(bloomfilter->getCurFilterSize()),
                              last);

  return returnList;
}


//Support Functions for Persistent Sorage

Word
ScalableBloomFilter::Create( const ListExpr typeInfo )
{
  Word w; 
  w.addr = (new ScalableBloomFilter(0.1));
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
  double fp;
  size_t maxInserts = 8;
  size_t subFilterSize;
  int nbrSubFilters;
  int nbrHashFunctions;
  vector<int> hashFunctionsPerFilter;
  vector<bool> insertionVector;
  bool filterElement;

  bool ok = valueRecord.Read (&fp, sizeof(double), offset);
  offset += sizeof(double);

  cout << "Open FP: " << fp << endl;

  ScalableBloomFilter* openBloom = new ScalableBloomFilter(fp);

  ok = ok && valueRecord.Read (&nbrSubFilters, sizeof(int), offset);
  offset += sizeof(int);

  cout << "Open Nbr Subfilters: " << nbrSubFilters << endl;

  openBloom->getFilterList().reserve(nbrSubFilters);

  hashFunctionsPerFilter.reserve(nbrSubFilters);
  for (int i = 0; i < (nbrSubFilters); i++) {
    ok = ok && valueRecord.Read(&nbrHashFunctions, sizeof(int), offset);
    hashFunctionsPerFilter.push_back(nbrHashFunctions);
    offset += sizeof(int);
  }

  int i = 0; 
  cout << "Open Hashfunctions per filter: " << endl;
  for (int nbr : hashFunctionsPerFilter) {
    cout << "Filter " << i << " has " << nbr << " Hashes" << endl;
    i++;
  }

  openBloom -> getFilterHashes().clear();
  openBloom -> getFilterHashes().reserve(hashFunctionsPerFilter.size());
  openBloom -> setFilterHashes(hashFunctionsPerFilter);
  
  
  cout << "Nbr of Hashfunctions saved per Filter in OpenBloom: " << endl;

  for (int nbr : openBloom -> getFilterHashes()) {
    int i = 0; 
    cout << "Filter " << i << " Hashes: " << nbr << endl;
    i++;
  }
  
  subFilterSize=openBloom->optimalSize(maxInserts, fp);
  for (size_t j = 0; j < subFilterSize; j++) {
    ok = ok && valueRecord.Read (&filterElement, sizeof(bool), offset);
    offset += sizeof(bool);
    openBloom->setElement(0,j, filterElement);
  }

  fp *= 0.8;
  maxInserts*=2;

  cout << endl;
  cout << "Beginning to Copy Subfilter values" << endl;  
  for (int i = 1;  i < nbrSubFilters; i++) {
    cout << endl;
    cout << "Beginning Work on Subfilter " << i << endl;
    cout << endl;
    subFilterSize=openBloom->optimalSize(maxInserts, fp);
    cout << "Size of Subfilter " << i << " determined to be: " 
         << subFilterSize << endl;
    cout << endl;
    insertionVector.reserve(subFilterSize);

    for (size_t j = 0; j < subFilterSize; j++) {
      ok = ok && valueRecord.Read (&filterElement, sizeof(bool), offset);
      offset += sizeof(bool);
      insertionVector.push_back(filterElement);
    }
    
    cout << "Subfilter " << i << " has the form: " << endl;
    for (bool elem  : insertionVector) {
      cout << elem; 
    }
    cout << endl;
    
    cout << "Pushing insertion Vector into FilterList: " << endl;
    openBloom -> setSubFilter(insertionVector);
    cout << endl;

    cout << "FilterList now has " << 
        openBloom -> getFilterList().size() << " SubFilters" << endl; 
    cout << endl;

    insertionVector.clear();
    cout << endl; 
    cout << endl;

    fp *= 0.8;
    maxInserts*=2;

  }

  cout << "The opened Bloomfilter has the values: ";
  int indiz = 0; 
  for (vector<bool> subfilter : openBloom -> getFilterList()) {
    cout << endl;
    cout << "Opened Subfilter " << indiz << " has the form: " << endl;
    cout << endl;
    for (bool filterValue : subfilter) {
      cout << filterValue;
    }
    indiz++;
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
  
  double fp = bloomFilter->getFP();
  int nbrSubFilters = bloomFilter->getFilterList().size();
  vector<int> hashfunctionsPerFilter = bloomFilter -> getFilterHashes();

  cout << endl;
  cout << "Saved FP: " << fp << endl;

  bool ok = valueRecord.Write(&fp, sizeof(double), offset);
  offset+=sizeof(double);

  //The number of Filters is equivalent to the different number of 
  // Hashfunctions we save. Hence we only need to save one of these 
  // updateFilterValues
  ok = ok && valueRecord.Write(&nbrSubFilters, sizeof(int), offset);
  offset+=sizeof(int);

  cout << "Saved Nbr Subfilters: " << nbrSubFilters << endl;

  cout << "Saved Nbr of Hashfunctions per Filter: " << endl;

  int i = 0; 

  //Save the amount of Hashfunctions each Subfilter uses
  for (int nbr : hashfunctionsPerFilter) {
    ok = ok && valueRecord.Write(&nbr, sizeof(int), offset);
    offset+=sizeof(int);
    cout << i << ":" << nbr << endl;
    i++;
  }

  cout << endl;

  
  i = 0;
  for (vector<bool> subFilter : bloomFilter->getFilterList()) {
    cout << "Subfilter " << i << ":" << endl;
    for (bool elem : subFilter) {
      ok = ok && valueRecord.Write(&elem, sizeof(bool), offset);
      offset+=sizeof(bool);
      cout << elem;
    }
    i++; 
    cout <<endl;
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
ScalableBloomFilter::Clone( const ListExpr typeInfo, const Word& w ) {
  ScalableBloomFilter* oldFilter = (ScalableBloomFilter*) w.addr;
  return SetWord( new ScalableBloomFilter(*oldFilter));
}



//Type Description

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


//Creation of the Type Constructor Instance

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

  // resize rows and columns to prevent possible 
  // memory Fragmentation later on, since we already
  // know the required number of counters
  
  matrix.resize(depth);
  for (size_t i = 0; i < depth; i++) {
    matrix[i].resize(width);
  }

  cout << "Hashconstants vector size before resize is " 
       << hashConstants.size() << endl;
  hashConstants.resize(depth);
  cout << "Hashconstants vector size after resize is " 
       << hashConstants.size() << endl;
  for (size_t i = 0; i < depth; i++) {
    hashConstants[i].resize(2);
    generateConstants(i);
  }
} 

CountMinSketch::CountMinSketch
(const CountMinSketch& rhs) {
  defined = rhs.defined;
  eps = rhs.eps;
  delta = rhs.delta; 
  width = rhs.width; 
  depth = rhs.depth;
  matrix = rhs.matrix;
  hashConstants = rhs.hashConstants;
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

int
CountMinSketch::getElement(int counterNumber, int index) {
  return matrix[counterNumber][index];
}

void 
CountMinSketch::setElement(int counterNumber, int index, int value) {
  matrix[counterNumber][index] = value;
}

vector<vector<int>>
CountMinSketch::getMatrix() {
  return matrix;
}

long
CountMinSketch::getConstantA(int index) {
  return hashConstants[index][0];
}

long
CountMinSketch::getConstantB(int index) {
  return hashConstants[index][1];
}

void 
CountMinSketch::setConstants(int counterNumber, long a, long b) {
  hashConstants[counterNumber][0] = a;
  hashConstants[counterNumber][1] = b;
}

vector<vector<long>>
CountMinSketch::getConstants() {
  return hashConstants;
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

  // set seed for the generation of constants 
  // for the choice of hashfunctions from the 
  // pairwise independent family (ax + b % p)
  srand(time(NULL)+getpid());
  cout << "Hashconstants vector size in initialize before resize is " 
       << hashConstants.size() << endl;
  hashConstants.resize(depth);
  cout << "Hashconstants vector size in initialize after resize is " 
       << hashConstants.size() << endl;

  for (size_t i = 0; i < depth; i++) {
    hashConstants[i].resize(2);
    generateConstants(i);
  }

  cout << "Constants in Constant Vector are: " << endl;
  int j = 0; 
  for (vector<long> counterHashes : hashConstants) {
    cout << "For Vector " << j << endl;
    for (long constant : counterHashes) {
      cout << constant << endl; 
    }
    j++;
  }


}

// We use the fact that pairwise independent Hashfunctions are easy
// to generate with h(x) = ax + b % p, with p being a big prime, and a
// b being constants. In this function we generate the constants.

void
CountMinSketch::generateConstants(int index) {
  long a = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  long b = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  cout << "generateConstants() a: " << a << " b: " << b << endl;
  setConstants(index, a, b);
}

void 
CountMinSketch::increaseCount(long hashedEleValue) {
  totalCount++;
  size_t hashValue;
  long a;
  long b;
  // Use our 2wise independent hash function
  // and modulo it additionaly so that our counters
  // are hit
  for (size_t i = 0; i < depth; i++) {
    a = getConstantA(i);
    b = getConstantB(i);
    hashValue = ((a*hashedEleValue+b) % LONG_PRIME) % width;
    matrix[i][hashValue] = matrix[i][hashValue] + 1;
  }
}

int 
CountMinSketch::estimateFrequency(long hashedEleValue) {
  int minVal;
  int compareValue;
  long a;
  long b; 
  
  //use the Hashvalues of the Searchelement as Index
  a = getConstantA(0);
  b = getConstantB(0);
  size_t hashedIndex = ((a*hashedEleValue+b) % LONG_PRIME) % width;

  //Assume that the first value is the amount of times the item appeared
  minVal = getElement(0, hashedIndex);

  for (size_t i = 1; i < depth; i++) {
    a = getConstantA(i);
    b = getConstantB(i);
    hashedIndex = ((a*hashedEleValue+b) % LONG_PRIME) % width;
    compareValue = getElement(i, hashedIndex);
    minVal = minVal < compareValue ? minVal : compareValue;
  } 

  cout << "Found minum value to be " << minVal << endl;
  return minVal;
}

//~In~/~Out~ Functions

//Currently a Dummy
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
    ScalableBloomFilter* bloom = new ScalableBloomFilter(fp);
    for (size_t i = 0; i < inserts; i++) {
      index = third.first();
      third.rest();
      bloom -> getSubFilter(i)[i] = index.boolval();
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


//Support Functions for Persistent Sorage

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

//Save and Open 
bool
CountMinSketch::Open(SmiRecord& valueRecord, size_t& offset, 
                         const ListExpr typeInfo, Word& value) 
{  
  float epsilon;
  float delta; 
  size_t width;
  size_t depth;
  long constantA;
  long constantB;
  int counterEle;

  bool ok = valueRecord.Read (&epsilon, sizeof(float), offset);
  offset += sizeof(float);

  ok = valueRecord.Read (&delta, sizeof(float), offset);
  offset += sizeof(float);

  ok = ok && valueRecord.Read (&width, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ok = ok && valueRecord.Read (&depth, sizeof(size_t), offset);
  offset += sizeof(size_t);

  CountMinSketch* openCMS = new CountMinSketch(epsilon, delta);

  for (size_t i = 0; i < depth; i++) {
    ok = ok && valueRecord.Read (&constantA, sizeof(long), offset);
    offset+=sizeof(long); 
    ok = ok && valueRecord.Read (&constantB, sizeof(long), offset);
    offset+=sizeof(long); 
    openCMS->setConstants(i, constantA, constantB);
  }

  int i = 0;
  cout << "After Opening HashConstants for Counter " << i << " are: ";
  for (vector<long> constants : openCMS -> getConstants()) {
    for (long constant : constants) {
      cout << constant << endl;
    }
    cout << endl;
    i++;
  }


  for (size_t i = 0; i < depth; i++) {
    for (size_t j = 0; j < width; j++) {
        ok = ok && valueRecord.Read (&counterEle, sizeof(int), offset);
        offset+=sizeof(int); 
        openCMS -> setElement(i,j,counterEle);
    }
  }

  i = 0;
  for (vector <int> counter : openCMS -> getMatrix()) {
    cout << "After Opening Counter Number " << i 
         << " has the following elements: ";
    for (int count : counter) {
      cout << count;
    }
    i++;
  }


  if (ok) {
    value.addr = openCMS;
  } else {
    value.addr =  0;
  }
  return true;
} 


bool 
CountMinSketch::Save(SmiRecord & valueRecord , size_t & offset ,
const ListExpr typeInfo , Word & value) {
  CountMinSketch* cms = static_cast<CountMinSketch*>
                                    (value.addr);

  float epsilon = cms->getEpsilon();
  float delta = cms -> getDelta();
  size_t width = cms -> getWidth();
  size_t depth = cms -> getDepth();
  long hashConstantA;
  long hashConstantB;
  int counterEle;                                 

  bool ok = valueRecord.Write(&epsilon, sizeof(float), offset);
  offset+=sizeof(float);

  ok = ok && valueRecord.Write(&delta, sizeof(float), offset);
  offset+=sizeof(float);
  
  ok = ok && valueRecord.Write(&width, sizeof(size_t), offset);
  offset+=sizeof(size_t);

  ok = ok && valueRecord.Write(&depth, sizeof(size_t), offset);
  offset+=sizeof(size_t);

  for (size_t i = 0; i < depth; i++) {
    hashConstantA = cms ->getConstantA(i);
    hashConstantB =  cms -> getConstantB(i);
    ok = ok && valueRecord.Write(&hashConstantA, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Write(&hashConstantB, sizeof(long), offset);
    offset+=sizeof(long);
  }
  
  for (size_t i = 0; i < depth; i++) {
    for (size_t j = 0; j < width; j++) {
        counterEle = cms->getElement(i,j);
        ok = ok && valueRecord.Write(&counterEle, sizeof(int), offset);
        offset+=sizeof(int);
    }
  }

  cout << endl;
  return true;
}


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


//Type Description


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


//Creation of the Type Constructor Instance


struct countMinSketchFunctions : 
       ConstructorFunctions<CountMinSketch> {

  countMinSketchFunctions()
  {
    in = CountMinSketch::In;
    out = CountMinSketch::Out;
    create = CountMinSketch::Create;
    deletion = CountMinSketch::Delete;
    open = CountMinSketch::Open;
    save = CountMinSketch::Save;
    close = CountMinSketch::Close;
    clone = CountMinSketch::Clone;
  }
};

countMinSketchInfo ci;
countMinSketchFunctions cf;
TypeConstructor countMinSketchTC( ci, cf );



/*
2.1.3 Class ~amsSketch~

*/

amsSketch::amsSketch
(const float epsilon, const float delta) {
  defined = true;
  eps = epsilon;
  this->delta = delta;
  width = ceil(exp(1)/eps);
  depth = ceil(log(1/delta));
  
  matrix.resize(depth);
  for (size_t i = 0; i < depth; i++) {
    matrix[i].resize(width);
  }
  
  twConstants.resize(depth);
  for (size_t i = 0; i < depth; i++) {
    twConstants[i].resize(2);
    generateConstants(i);
  }

  // The only difference between CMS and AMS is the
  // requirement for a 4-wise independent Hashfamily
  // the constants required are saved here
  
  fwConstants.resize(depth);
  for (size_t i = 0; i < depth; i++) {
    fwConstants[i].resize(4);
    generateFwConstants(i);
  }
} 

amsSketch::amsSketch
(const amsSketch& rhs) {
  defined = rhs.defined;
  eps = rhs.eps;
  delta = rhs.delta; 
  width = rhs.width; 
  depth = rhs.depth;
  matrix = rhs.matrix;
  twConstants = rhs.twConstants;
  fwConstants = rhs.fwConstants;
  totalCount = rhs.totalCount;
}

//Setter and Getter
bool 
amsSketch::getDefined() {
  return defined;
}

size_t 
amsSketch::getTotalCount() {
  return totalCount;
}

size_t
amsSketch::getWidth() {
  return width;
}

size_t
amsSketch::getDepth() {
  return depth;
}

float
amsSketch::getEpsilon() {
  return eps;
}

float
amsSketch::getDelta() {
  return delta;
}

int
amsSketch::getElement(int counterNumber, int index) {
  return matrix[counterNumber][index];
}

void 
amsSketch::setElement(int counterNumber, int index, int value) {
  matrix[counterNumber][index] = value;
}

int
amsSketch::getConstantTwA(int index) {
  return twConstants[index][0];
}

int
amsSketch::getConstantTwB(int index) {
  return twConstants[index][1];
}

void
amsSketch::setConstantsTw(int index, long a, long b) {
  twConstants[index][0] = a;
  twConstants[index][1] =b;
}

int
amsSketch::getConstantFwA(int index) {
  return fwConstants[index][0];

}int
amsSketch::getConstantFwB(int index) {
  return fwConstants[index][1];

}int
amsSketch::getConstantFwC(int index) {
  return fwConstants[index][2];

}int
amsSketch::getConstantFwD(int index) {
  return fwConstants[index][3];
}

void
amsSketch::setConstantsFw(int index, long a, long b,
                          long c, long d) {
  fwConstants[index][0] = a;
  fwConstants[index][1] = b;
  fwConstants[index][2] = c;
  fwConstants[index][3] = d;
}


vector<vector<long>>
amsSketch::getConstantsFw() {
  return fwConstants;
}

vector<vector<long>>
amsSketch::getConstantsTw() {
  return twConstants;
}

vector<vector<int>>
amsSketch::getMatrix() {
  return matrix;
}

//Auxiliary Functions
void 
amsSketch::initialize(float eps, float delt) {
  defined = true;
  this->eps = eps; 
  this->delta = delt; 
  width = ceil(exp(1)/eps);
  depth = ceil(log(1/delta));
  
  matrix.resize(depth);
  for (size_t i = 0; i < depth; i++)
    matrix[i].resize(width);
  totalCount = 0;

  srand(time(NULL)+getpid());
  twConstants.resize(depth);
  for (size_t i = 0; i < depth; i++) {
    twConstants[i].resize(2);
    generateConstants(i);
  }

  fwConstants.resize(depth);
  for (size_t i = 0; i < depth; i++) {
    fwConstants[i].resize(4);
    generateFwConstants(i);
  }
}

// We use the fact that pairwise independent Hashfunctions are easy
// to generate with h(x) = ax + b % p, with p being a big prime, and a
// b beign constants. In this function we generate the constants.

void
amsSketch::generateConstants(int index) {
  long a = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  long b = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  setConstantsTw(index, a, b);
}

// In contrast to Count-Min we also need a four-wise independent hash
//  Function. These are given by h(x) = ax^3 + bx^2 + cx + d % p.

void 
amsSketch::generateFwConstants(int index) {
  long a = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  long b = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  long c = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  long d = long(float(rand())*float(LONG_PRIME)/float(RAND_MAX));
  setConstantsFw(index, a, b, c, d);
}

// Auxiliary function in the determination of the median of 
// squared sums of row values
void 
amsSketch::swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

int 
amsSketch::partition(int arr[], int l, int r)
{
    int lst = arr[r], i = l, j = l;
    while (j < r) {
        if (arr[j] < lst) {
            swap(&arr[i], &arr[j]);
            i++;
        }
        j++;
    }
    swap(&arr[i], &arr[r]);
    return i;
}

int 
amsSketch::randomPartition(int arr[], int l, int r)
{
    int n = r - l + 1;
    int pivot = rand() % n;
    swap(&arr[l + pivot], &arr[r]);
    return partition(arr, l, r);
}

void
amsSketch::medianDecider(int arr[], int l, int r, int k, int& a, int& b) {
  // if l < r
    if (l <= r) {
        // Find the partition index
        int partitionIndex = randomPartition(arr, l, r);
 
        // If partion index = k, then
        // we found the median of odd
        // number element in medianArray[]
        if (partitionIndex == k) {
            b = arr[partitionIndex];
            if (a != -1)
                return;
        }
 
        // If index = k - 1, then we get
        // a & b as middle element of
        // medianArray[]
        else if (partitionIndex == k - 1) {
            a = arr[partitionIndex];
            if (b != -1)
                return;
        }
 
        // If partitionIndex >= k then
        // find the index in first half
        // of the medianArray[]
        if (partitionIndex >= k)
            return medianDecider(arr, l, partitionIndex - 1,
                              k, a, b);
 
        // If partitionIndex <= k then
        // find the index in second half
        // of the medianArray[]
        else
          return medianDecider(arr, partitionIndex + 1,
                              r, k, a, b);
    }
    return;
}


int
amsSketch::findMedian(int medianArray[]) {
  int a = -1;
  int b = -1;
  int median;
  int n = *(&medianArray + 1) - medianArray;

  if (n % 2 == 1) {
    medianDecider(medianArray, 0, n - 1, n / 2, a, b);
        median = b;
  } else {
    medianDecider(medianArray, 0, n - 1, n / 2, a, b);
    median = (a + b) / 2;
  }
  return median;
}

void 
amsSketch::changeWeight(size_t value) {
  totalCount++;
  int hashIndex;
  int updateValue;
  long twa, twb ,fwa, fwb, fwc, fwd;

  //Extraction of the constants for the Hashfunctions for each (ith) row
  for (size_t i = 0; i < depth; i++) {
    twa = getConstantTwA(i);
    twb = getConstantTwB(i);
    fwa = getConstantFwA(i);
    fwb = getConstantFwB(i);
    fwc = getConstantFwC(i);
    fwd = getConstantFwD(i);
    //Find the Index of the Element whose counter we will change
    hashIndex = ((twa*value+twb) % LONG_PRIME) % width;

    //Compute the Value we will use to update the counter
    updateValue = fwa*pow(value,3) + fwb*pow(value,2) 
                  + fwc*value + fwd % LONG_PRIME % 2 - 1; 

    //Commit the change
    setElement(i, hashIndex, updateValue); 
  }
}

int 
amsSketch::estimateInnerProduct() {
  int medianArray[depth]; 
  int joinSize;
  int sum;

  //Calculate the sum of the squares of all row Elements for each row
  for (size_t i = 0; i < depth; i++) {
    for (size_t j = 0; j < width; j++) {
      sum += pow(getElement(i, j), 2);
    }
    medianArray[i] = sum;
  }

  joinSize = findMedian(medianArray);

  //Return the Median of sum of squares of elements of the rows
  return joinSize;
}


//~In~/~Out~ Functions


//In-Function to turn List Representation into Class Representation
Word
amsSketch::In(const ListExpr typeInfo, const ListExpr instance,
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
    amsSketch* ams = new amsSketch(fp, inserts);
    for (size_t i = 0; i < ams->getDelta(); i++) {
      index = third.first();
      third.rest();
    }
  }
  return result;
}

//Out-Function (Dummy)
ListExpr
amsSketch::Out(ListExpr typeInfo, Word value) {
  amsSketch* cms = 
                       static_cast<amsSketch*> (value.addr);
  if(!cms -> getDefined()) {
    return listutils::getUndefined();
  }

  ListExpr elementList = nl -> OneElemList(nl->BoolAtom(0));
 

  return elementList;
}


//Support Functions for Persistent Sorage

Word
amsSketch::Create( const ListExpr typeInfo )
{
  Word w; 
  w.addr = (new amsSketch(0.1, 0.5));
  return w;
}

void
amsSketch::Delete( const ListExpr typeInfo, Word& w )
{
  delete (amsSketch*) w.addr;
  w.addr = 0;
}

//Save and Open 
bool
amsSketch::Open(SmiRecord& valueRecord, size_t& offset, 
                         const ListExpr typeInfo, Word& value) 
{  
  float epsilon;
  float delta; 
  size_t width;
  size_t depth;
  int constantTwA, constantTwB, constantFwA, 
      constantFwB, constantFwC, constantFwD;
  int counterEle;

  bool ok = valueRecord.Read (&epsilon, sizeof(float), offset);
  offset += sizeof(float);

  ok = valueRecord.Read (&delta, sizeof(float), offset);
  offset += sizeof(float);

  ok = ok && valueRecord.Read (&width, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ok = ok && valueRecord.Read (&depth, sizeof(size_t), offset);
  offset += sizeof(size_t);

  amsSketch* openAMS = new amsSketch(epsilon, delta);

  for (size_t i = 0; i < depth; i++) {
    ok = ok && valueRecord.Read (&constantTwA, sizeof(long), offset);
    offset+=sizeof(long); 
    ok = ok && valueRecord.Read (&constantTwB, sizeof(long), offset);
    offset+=sizeof(long);
    openAMS->setConstantsTw(i, constantTwA, constantTwB);
    ok = ok && valueRecord.Read (&constantFwA, sizeof(long), offset);
    offset+=sizeof(long); 
    ok = ok && valueRecord.Read (&constantFwB, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Read (&constantFwC, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Read (&constantFwD, sizeof(long), offset);
    offset+=sizeof(long);
    openAMS ->setConstantsFw(i, constantFwA,constantFwB,
                             constantFwC,constantFwD);
  }

  for (size_t i = 0; i < depth; i++) {
    cout << "Counter Number " << i << " has the following elements: ";
    cout << endl;
    for (size_t j = 0; j < width; j++) {
        ok = ok && valueRecord.Read (&counterEle, sizeof(int), offset);
        offset+=sizeof(int); 
        openAMS -> setElement(i,j,counterEle);
        cout << counterEle;   
    }
    cout << endl;
  }

  if (ok) {
    value.addr = openAMS;
  } else {
    value.addr =  0;
  }
  return true;
} 


bool 
amsSketch::Save(SmiRecord & valueRecord , size_t & offset ,
const ListExpr typeInfo , Word & value) {
  amsSketch* ams = static_cast<amsSketch*>
                                    (value.addr);

  float epsilon = ams->getEpsilon();
  float delta = ams -> getDelta();
  size_t width = ams -> getWidth();
  size_t depth = ams -> getDepth();
  long hashConstantTwA, hashConstantTwB, hashConstantFwA, hashConstantFwB, 
      hashConstantFwC, hashConstantFwD, counterEle;

  bool ok = valueRecord.Write(&epsilon, sizeof(float), offset);
  offset+=sizeof(float);

  ok = ok && valueRecord.Write(&delta, sizeof(float), offset);
  offset+=sizeof(float);
  
  ok = ok && valueRecord.Write(&width, sizeof(size_t), offset);
  offset+=sizeof(size_t);

  ok = ok && valueRecord.Write(&depth, sizeof(size_t), offset);
  offset+=sizeof(size_t);

  for (size_t i = 0; i < depth; i++) {
    hashConstantTwA = ams ->getConstantTwA(i);
    hashConstantTwB =  ams -> getConstantTwB(i);
    hashConstantFwA = ams -> getConstantFwA(i);
    hashConstantFwB = ams -> getConstantFwB(i);
    hashConstantFwC = ams -> getConstantFwC(i);
    hashConstantFwD = ams -> getConstantFwD(i);
    ok = ok && valueRecord.Write(&hashConstantTwA, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Write(&hashConstantTwB, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Write(&hashConstantFwA, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Write(&hashConstantFwB, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Write(&hashConstantFwC, sizeof(long), offset);
    offset+=sizeof(long);
    ok = ok && valueRecord.Write(&hashConstantFwD, sizeof(long), offset);
    offset+=sizeof(long);
  }
  

  for (size_t i = 0; i < depth; i++) {
    for (size_t j = 0; j < width; j++) {
        counterEle = ams->getElement(i,j);
        ok = ok && valueRecord.Write(&counterEle, sizeof(int), offset);
        offset+=sizeof(int);
        cout << counterEle;
    }
  }

  cout << endl;
  return true;
}


void
amsSketch::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<amsSketch*>( w.addr );
  w.addr = 0;
}


Word
amsSketch::Clone( const ListExpr typeInfo, const Word& w )
{
  amsSketch* oldSketch = (amsSketch*) w.addr;
  return SetWord( new amsSketch(*oldSketch));
}

/*
Type Description

*/

struct amsSketchInfo : ConstructorInfo {

  amsSketchInfo() {

    name         = amsSketch::BasicType();
    signature    = "-> " + Kind::SIMPLE();
    typeExample  = amsSketch::BasicType();
    listRep      =  "()";
    valueExample = "(4 12 2 8)";
    remarks      = "";
  }
};

/*
Creation of the Type Constructor Instance

*/

struct amsSketchFunctions : 
       ConstructorFunctions<amsSketch> {

  amsSketchFunctions()
  {
    in = amsSketch::In;
    out = amsSketch::Out;
    create = amsSketch::Create;
    deletion = amsSketch::Delete;
    open = amsSketch::Open;
    save = amsSketch::Save;
    close = amsSketch::Close;
    clone = amsSketch::Clone;
  }
};

amsSketchInfo ai;
amsSketchFunctions af;
TypeConstructor amsSketchTC( ai, af );


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
  if (type.length() != 3){
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
      return NList::typeError("Operator createcountmin can only handle "
                              "Attributetype Tuple values");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.second().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  //Save whether the Attribute Type we have to hash is a number
  // so we can modify the way we hash;
  bool isNumeric = listutils::isNumericType(attrType);

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }




  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().boolAtom(isNumeric));

  /* result is a Count Min Sketch and we append the index and type of 
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
  NList appendList;

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
    //check if the searchelement is numeric
    bool isNumeric = listutils::isNumericType(type.second().listExpr());
    cout << "cms Count identifies search element as numeric: " 
         << isNumeric << endl;
    appendList.append(NList().boolAtom(isNumeric));
    return NList(Symbols::APPEND(), appendList,
               CountMinSketch::BasicType()).listExpr();
  } 

  return NList::typeError("Operator cmscount expects an  " 
                          "ATTRIBUTE as second argument");
}

/*
2.2.6 Operator ~createams~

*/
ListExpr
createamsTM( ListExpr args ) {
NList type(args);
NList streamtype = type.first().second();
NList appendList;

ListExpr a = nl->First(args);

ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 4){
    return NList::typeError("Operator createams expects "
                            "four arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(a)){
    return NList::typeError( "Operator createams expects a "
                             "Tuple Stream as first argument");
  }

  //test second argument for a valid Attribute Name
  if (!type.second().isSymbol()){
    return NList::typeError("Operator createams expects a valid "
                            "Attribute Name as second argument");
  }

  // test third argument for real
  if(type.third() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator createams expects a real "
                            "value as second argument");
  }

  //test fourth argument for int
  if(type.fourth() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator createams expects a real "
                            "value as third argument");
  }
  
  // stream elements must be in kind tuple (X) with X in DATA
  if(!(streamtype.hasLength(2)
          && streamtype.first().isSymbol(sym.TUPLE())
          && IsTupleDescription(streamtype.second().listExpr())
         )
          && !(am->CheckKind(Kind::DATA(),streamtype.listExpr(),errorInfo))){
      return NList::typeError("Operator createams can only handle "
                              "Attributetype Tuple values");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.second().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  //Save whether the Attribute Type we have to hash is a number
  // so we can modify the way we hash;
  bool isNumeric = listutils::isNumericType(attrType);
  
  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }

  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().boolAtom(isNumeric));

  /* result is a AMS Sketch and we append the index and type of 
     the attribute of the tuples which will be hashed to create our filter 
  */

  return NList(Symbols::APPEND(), appendList, 
               amsSketch::BasicType()).listExpr();
}


/*
2.2.7 Operator ~amsestimate~

*/
ListExpr
amsestimateTM(ListExpr args) {
  NList type(args);
  NList appendList;

  // two arguments must be supplied
  if (type.length() != 1){
    return NList::typeError("Operator amsestimate expects one arguments");
  }

  // test first argument for scalablebloomfilter
  if(type.first() != NList(amsSketch::BasicType())){
    return NList::typeError("Operator amsestimate expects a "
                            "AMS Sketch as argument");
  }

  //check if the searchelement is numeric
  return NList(amsSketch::BasicType()).listExpr(); 
}

/*
2.3 Value Mapping Functions

2.3.1 Operator ~reservoir~

Creates a reservoir Sample (stream) of the passed stream.

*/

/*
Templates are used to deal with the different Types of Streams 
the operator handles

*/
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
  CcInt* attrIndexPointer = (CcInt*) args[3].addr;

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
  bloomFilter->initialize(fpProb->GetValue());

  cout << "After init() Bloom Filter Values are: " << endl;
  cout << "Defined: " << bloomFilter->getDefined() << endl;
  cout << "Nbr of Hashes: " << bloomFilter->getCurNumberHashes() << endl;
  cout << "Hashvector saves: " << bloomFilter->getFilterHashes().size() << endl;
  cout << "FP: " << + bloomFilter->getFP() << endl;
  cout << "Filter currently made for Inserts " << 
           bloomFilter->getCurMaxInserts() << endl;
  cout << "Current Filter Size: " <<  bloomFilter->getCurFilterSize() << endl;
  cout << "Overall having " << bloomFilter->getFilterList().size() 
                            << " Filters" << endl;


  //Get the stream provided by the operator
  Stream<Tuple> stream(args[0]);

  //open the stream 
  stream.open();
  
  //Pointers to stream elements will be saved here for use
  Tuple* streamTuple = (Tuple*) stream.request();

  Attribute* streamElement; 

  //Get the size of the Filter so we can %mod the hash 
  //results to map to an index in the filter
  size_t filterSize = bloomFilter->getCurFilterSize();

  //Get number of Hashfunctions so reserving the hash results
  //vector will be faster
  int nbrHashes = bloomFilter->getCurNumberHashes();
  vector<size_t> hashvalues;

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t mHash[2]; 

  //while the stream can still provide elements:
  while ((streamTuple != 0)) {
    if (bloomFilter->isSaturated()) {
      cout << "Filter " << bloomFilter->getFilterList().size() - 1 
           << " is full" << endl;
      cout << endl;
      bloomFilter->updateFilterValues();
      nbrHashes = bloomFilter->getCurNumberHashes();
      filterSize = bloomFilter ->getCurFilterSize();
    }
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

    streamTuple->DeleteIfAllowed();


    //assign next Element from the Stream
    streamTuple = stream.request();   
  }

  cout << endl;
  cout << "Bloomfilters have the values: " << endl;

  int indiz = 0; 
  for (vector<bool> subfilter : bloomFilter -> getFilterList()) {
    cout << endl;
    cout << "Subfilter " << indiz << " has the form: " << endl;
    for (bool filterValue : subfilter) {
      cout << filterValue;
    }
    indiz++;
    cout << endl;
    cout << endl;
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

  bool included = false;

  //take the parameters values supplied with the operator
  ScalableBloomFilter* bloomFilter = (ScalableBloomFilter*) args[0].addr;
  Attribute* searchEle = (Attribute*) args[1].addr;

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcBool* res = (CcBool*) result.addr;

  //Get the amount of Hashfunctions each subfilter uses
  vector<int> hashIterations = bloomFilter -> getFilterHashes();

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t cmHash[2];

  //Take Size of the bloomFilter
  size_t nbrOfFilters = bloomFilter -> getFilterList().size();

  //prepare a vector to take in the Hashresults
  vector<size_t> hashValues; 
 
  for (size_t i = 0; i < nbrOfFilters; i++) {
    hashValues.clear();
    hashValues.reserve(hashIterations[i]);
    size_t filterSize = bloomFilter->getFilterList()[i].size();

    //hash the Searchelement
    MurmurHash3_x64_128(searchEle, sizeof(*searchEle), 0, cmHash);
  
    size_t h1 = cmHash[0] % filterSize;
    hashValues.push_back(h1);

    size_t h2 = 1;

    if (hashIterations[i] >= 2) {
      h2 = cmHash[1] % filterSize;
      hashValues.push_back(h2);
    }

    for (int j = 2; j < hashIterations[i]; j++) {
      size_t h_i = (h1 + i * h2 + i * i) % filterSize;
      hashValues.push_back(h_i);
    }

    //Element is contained in one of the Subfilters
    if (bloomFilter->contains(hashValues, i)) {
      included = true; 
      break;
    }
  }

  res->Set(true, included);

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
  CcBool* attrIsNumeric = (CcBool*) args[5].addr;

  int attrIndex = attrIndexPointer->GetIntval();
  bool attrNumeric = attrIsNumeric->GetValue();

  cout << "In the VM AttrIsNumeric has value: " << attrNumeric << endl;

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

  /*check if the Attributes we are going to hash is
    any form of text
  */
   

  if (!attrNumeric) {
    cout << "Identified Attribute Type as String" << endl;
    //Prepare buffer for the MurmurHash3 output storage for Strings
    uint64_t mHash[2]; 
    while ((streamTuple != 0)) {
      streamElement = (Attribute*) streamTuple->GetAttribute(attrIndex);
      MurmurHash3_x64_128(streamElement, sizeof(*streamElement), 0, mHash);
      long h1 = mHash[0];
      cms->increaseCount(h1);
      streamTuple->DeleteIfAllowed();
      streamTuple = stream.request();   

    }
  }
  
  //while the stream can still provide elements:
  while ((streamTuple != 0)) {
    CcInt* intElement;
    intElement = (CcInt*) streamTuple->GetAttribute(attrIndex);

    /*Increase element Counter in the 
    Count Min Sketch

    */

    cms->increaseCount(intElement -> GetIntval());
    
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
  cout << "----------------------------------------------" << endl;
  cout << endl;
  
  stream.close();

  result.setAddr(cms);

  return 0;
}

/*
2.3.5 Operator ~cmscount~

*/
int cmscountVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CountMinSketch* cms = (CountMinSketch*) args[0].addr;
  CcInt* searchEle = (CcInt*) args[1].addr;
  CcInt* attrIsNumeric = (CcInt*) args[2].addr;
  bool attrNumeric = attrIsNumeric->GetValue();

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcInt* res = (CcInt*) result.addr;
  
  //prepare the result
  int estimate = 0;

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t cmHash[2];

  //Search element is a string and we have to modulate
  //the way we hash slightly
  if (!attrNumeric) {
    cout << "cmsCount identified the Searchelement as Text" << endl;
      MurmurHash3_x64_128(searchEle, sizeof(*searchEle), 0, cmHash);
      long h1 = cmHash[0];
      estimate = cms->estimateFrequency(h1);    
  } else {
    //hash the Searchelement
    estimate = cms->estimateFrequency(searchEle->GetValue());
  }

  res->Set(true, estimate);

  return 0;
}

/*
2.3.5 Operator ~createams~

*/
int createamsVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CcReal* epsilon = (CcReal*) args[2].addr;
  CcReal* delta = (CcReal*) args[3].addr;
  CcInt* attrIndexPointer = (CcInt*) args[4].addr;
  CcBool* attrIsNumeric = (CcBool*) args[5].addr;

  int attrIndex = attrIndexPointer->GetIntval();
  bool attrNumeric = attrIsNumeric->GetValue();


  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  /*CMS Datastructure is used since CMS and AMS only differ 
    in the way they use Hashfunctions to process updates.
  */
  amsSketch* ams = (amsSketch*) result.addr;

  //initialize the Filter with the values provided by the operator
  ams->initialize(epsilon->GetValue(),delta->GetValue());

  //Get the stream provided by the operator
  Stream<Tuple> stream(args[0]);

  //open the stream 
  stream.open();
  
  //Pointers to stream elements will be saved here for use
  Tuple* streamTuple = (Tuple*) stream.request();

  Attribute* streamElement; 

  if (!attrNumeric) {
    cout << "Identified Attribute Type as String" << endl;
    //Prepare buffer for the MurmurHash3 output storage for Strings
    uint64_t mHash[2]; 
    while ((streamTuple != 0)) {
      streamElement = (Attribute*) streamTuple->GetAttribute(attrIndex);
      MurmurHash3_x64_128(streamElement, sizeof(*streamElement), 0, mHash);
      long h1 = mHash[0];
      ams->changeWeight(h1);
      streamTuple->DeleteIfAllowed();
      streamTuple = stream.request();   
    }
  }


  //while the stream can still provide elements:
  while ((streamTuple != 0)) {
    CcInt* intElement;
    intElement = (CcInt*) streamTuple->GetAttribute(attrIndex);
    ams->changeWeight(intElement -> GetValue());

    streamTuple->DeleteIfAllowed();

    //assign next Element from the Stream
    streamTuple = stream.request();   
    } 

  for (size_t i = 0; i < ams->getDepth(); i++) {
    cout << "Vector " << i << " looks like: " << endl; 
    for (size_t j = 0; j < ams->getWidth(); j++) {
       cout << ams->getMatrix()[i][j];
    }
    cout << endl;
  }
  cout << endl;
  cout << "Total Elements processed: " << ams->getTotalCount();
  cout << "----------------------------------------------" << endl;
  cout << endl;
  
  stream.close();

  result.setAddr(ams);

  return 0;
}

/*
2.3.6 Operator ~amsestimate~

*/
int amsestimateVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  amsSketch* ams = (amsSketch*) args[0].addr;

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcReal* res = (CcReal*) result.addr;
  
  //prepare the result
  int estimate = 0;

  estimate = ams -> estimateInnerProduct();

  res->Set(true, estimate);

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
   "stream(tuple(X)) x ATTR x real ->  bloomfilter",
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
   "_ cmscountSpec [_]",
   "Gives an estimate of how often an Element appeared in the Stream the ",
   "Count Min Sketch was created for"
   "query Kinos feed createcountmin[Name,0.01,0.9] cmscount[\"Astor\"]"
  );

  OperatorSpec createamsSpec(
    "stream(tuple(X)) x ATTR x int x real ->  countminsketch",
   "_ createamsSpec [_,_,_]",
   "Creates an AMS Sketch fpor the supplied stream", 
   "query Kinos feed createams[Name,0.01,0.9] amsestimate"
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

