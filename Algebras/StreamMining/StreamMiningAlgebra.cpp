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
#include "lossyCounter.h"
#include "cPoint.h"
#include "Cluster.h"
#include "kMeans.h"

#include <string>
#include <iostream>   
#include <vector>
#include <cmath>
#include <time.h>
#include <chrono>


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

// Update the parameters and add a new Subfilter to our Scalable Bloom
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
         << " has the following elements: " << endl;;
    for (int count : counter) {
      cout << count;
    }
    cout << endl;
    cout << endl;
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
amsSketch::updateElement(int counterNumber, int index, int value) {
  matrix[counterNumber][index] += value;
}

long
amsSketch::getConstantTwA(int index) {
  return twConstants[index][0];
}

long
amsSketch::getConstantTwB(int index) {
  return twConstants[index][1];
}

void
amsSketch::setConstantsTw(int index, long a, long b) {
  twConstants[index][0] = a;
  twConstants[index][1] = b;
}

long
amsSketch::getConstantFwA(int index) {
  return fwConstants[index][0];
}

long
amsSketch::getConstantFwB(int index) {
  return fwConstants[index][1];
}

long
amsSketch::getConstantFwC(int index) {
  return fwConstants[index][2];
}

long
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
  cout << "FW-Constants generated for Row " << index << " are: " << endl;
  cout << "a: " << a << " b: "  << b << " c: " << c << " d: " << d << endl;
  cout << endl;
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
    updateValue = 2*((long)(fwa*pow(value,3) + (int)fwb*pow(value,2) 
                  + fwc*value + fwd) % LONG_PRIME % 2)- 1;

    //Commit the change
    updateElement(i, hashIndex, updateValue); 
  }
}

float 
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
  amsSketch* ams = 
                       static_cast<amsSketch*> (value.addr);
  if(!ams -> getDefined()) {
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
  long constantTwA, constantTwB, constantFwA, 
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
  openAMS -> getConstantsTw().clear(); 
  openAMS -> getConstantsFw().clear();

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

  cout << "For Counter " << i << " the opened constants Vaues are: " << endl; 
  cout << "TwA: " << constantTwA << " TwB: " << constantTwB << endl; 
  cout << "FwA: " << constantFwA << " FwB: " << constantFwB << " FwC: " 
       << constantFwC << " FwD: " << constantFwD << endl;
  }

  for (size_t i = 0; i < depth; i++) {
    cout << "Opened Counter Number " << i << " has the following elements: ";
    cout << endl;
    for (size_t j = 0; j < width; j++) {
        ok = ok && valueRecord.Read (&counterEle, sizeof(int), offset);
        offset+=sizeof(int); 
        openAMS -> updateElement(i,j,counterEle);
        cout << counterEle;   
    }
    cout << endl;
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
      hashConstantFwC, hashConstantFwD; 
      
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

  cout << "For Counter " << i << " the saved constants Values are: " << endl; 
  cout << "TwA: " << hashConstantTwA << " TwB: " << hashConstantTwB << endl; 
  cout << "FwA: " << hashConstantFwA << " FwB: " << hashConstantFwB << " FwC: " 
                  << hashConstantFwC << " FwD: " << hashConstantFwD << endl;


  }
  

  for (size_t i = 0; i < depth; i++) {
    cout << "Saving Counter Values of Counter " << i << endl;
    for (size_t j = 0; j < width; j++) {
        counterEle = ams->getElement(i,j);
        ok = ok && valueRecord.Write(&counterEle, sizeof(int), offset);
        offset+=sizeof(int);
        cout << counterEle;
    }
    cout << endl;
    cout << endl;
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
2.1.4 Class ~lossyCounter~

*/
template<class T> 
lossyCounter<T>::lossyCounter
(const float epsilon) {
  defined = true; 
  this->epsilon = epsilon; 
  eleCounter = 0; 
  windowSize = ceil(1/epsilon);
  windowIndex = 1;
}

//Setter and Getter
template<class T> bool 
lossyCounter<T>::getDefined() {
  return defined;
} 

template<class T> void 
lossyCounter<T>::setDefined(bool value) {
  defined = value;
} 

template<class T> int 
lossyCounter<T>::getEleCounter() {
  return eleCounter;
} 

template<class T> float 
lossyCounter<T>::getEpsilon() {
  return epsilon;
} 

template<class T> long 
lossyCounter<T>::getCurrentWindowIndex() {
  return windowIndex;
} 

template<class T> int 
lossyCounter<T>::getWindowSize() {
  return windowSize;
}

template<class T> T 
lossyCounter<T>::getElement(int index){
  return frequencyList.at(index).getItem();
}

template<class T> std::unordered_map<T, counterPair<T>>
lossyCounter<T>::getFrequencyList() {
  return frequencyList;
}

//Auxiliary Functions
template<class T> void
lossyCounter<T>::initialize(const float epsilon) {
  defined = true; 
  this->epsilon = epsilon; 
  eleCounter = 1; 
  windowSize = ceil(1/epsilon);
  windowIndex = 1;
  frequencyList.insert({0, counterPair(0,0,0)});
}

///Handles incoming Streamelements
template<class T> void 
lossyCounter<T>::addElement(T element) {
  if (elementPresent(element)) {
    incrCount(element);
  } else {
    insertElement(element);
  }

  if (eleCounter % windowSize == 0) {
    reduce();
    updateWindowIndex();
  }
}

//Increase the Frequencycount of a Streamelement which
//was already present
template<class T> void 
lossyCounter<T>::incrCount(T element) {
  frequencyList.at(element).setFrequency();
  eleCounter++;
}

//Inserts previously unencountered Elements into our element list
template<class T> void 
lossyCounter<T>::insertElement(T element) {
  int maxError = windowIndex-1;
  //newly inserted Elements will always have Frequency 1
  counterPair value(element, 1, maxError);

  frequencyList.insert({element, value});
  eleCounter++;
}

//Checks whether a streamelement is already present in the list of elements
template<class T> bool 
lossyCounter<T>::elementPresent(T element) {
  if (frequencyList.find(element) == frequencyList.end()) {
    return false;
  } else {
    return true;
  }
}

//Updates the currently used Window
template<class T> void 
lossyCounter<T>::updateWindowIndex() {
  windowIndex = ceil(eleCounter/windowSize);
}

//Removes the Items below the Frequency Threshold
template<class T> void 
lossyCounter<T>::reduce() {
  vector<T> deletionList;
  for (auto elements : frequencyList) {
    counterPair elem = elements.second; 
    if ((elem.getFrequency() + elem.getMaxError()) < windowIndex) {
      deletionList.push_back(elements.first);
    }
  }
  for (T elem : deletionList) {
    frequencyList.erase(elem);
  }
}

//Get the Frequent items which surpase the minsupport threshold
//Min Frequency to get returned is minSupport*eleCounter
//While max deviation is epsilon*eleCounter
template<class T> vector<T> 
lossyCounter<T>::getFrequentElements(double minSupport) {
  vector<T> resultList; 
  for (auto elements : frequencyList) {
    counterPair elem = elements.second;
    if (elem.frequency >= ((minSupport - epsilon) * eleCounter)) {
      resultList.push_back(elem.getItem());
    }
  }
  return resultList;
}

//Get the frequency of a single Element
template<class T> long
lossyCounter<T>::estimateCount(T elem) {
  if (!(frequencyList.find(elem) == frequencyList.end())) {
    counterPair elemData = frequencyList.at(elem);    
    long elemFrequency = elemData.getFrequency();
    return elemFrequency;
  } else {
    return 0;
  }
}

/*
2.1.4.1 Class ~counterPair~

*/
template<class T>
counterPair<T>::counterPair
(T item, long frequency, long maxError) {
  this -> item = item; 
  this -> frequency = frequency;
  this -> maxError = maxError;
}

template<class T> T
counterPair<T>::getItem() {
  return item; 
}

template<class T> int 
counterPair<T>::getFrequency() {
  return frequency;
}

template<class T> void
counterPair<T>::setFrequency() {
  frequency++;
}

template<class T> int 
counterPair<T>::getMaxError() {
  return maxError;
}

/*
2.1.5 Class ~STREAM~

2.1.5.1 Class ~cPoint~

*/
cPoint::cPoint(int id, int coordinates) {
  pointId = id; 
  values.push_back(coordinates);
  dimensions = 1; 
  //Assign ID=0, because the Point will not have
  //an assigned Cluster at the start
  clusterId = 0; 
}

int
cPoint::getDimensions() {
  return dimensions;
}

int
cPoint::getCluster() {
  return clusterId;
}

int 
cPoint::getId() {
  return pointId;
}

void
cPoint::setCluster(int index) {
  clusterId = index;
}

int
cPoint::getVal(int index) {
  return values[index];
}

/*
2.1.5.2 Class ~Cluster~

*/
Cluster::Cluster(int id, cPoint centroid) {
  clusterId = id; 
  for (int i = 0; i < centroid.getDimensions(); i++) {
    clusterCentroid.push_back(centroid.getVal(i));
  }
  addPoint(centroid);
}

int
Cluster::getId() {
  return clusterId; 
}

cPoint
Cluster::getPoint(int pos) {
  return points[pos];
}

int
Cluster::getSize() {
  return points.size();
}

double
Cluster::getCentroidByPos(int pos) {
  return clusterCentroid[pos];
}

void
Cluster::setCentroidByPos(int pos, double val) {
  clusterCentroid[pos] = val;
}

//Auxilliary Functions
void 
Cluster::addPoint(cPoint p){
  p.setCluster(this->clusterId);
  points.push_back(p);
}

bool
Cluster::removePoint(int pointId) {
  int size = points.size();

  for (int i = 0; i < size; i++) {
    if (points[i].getId() == pointId) {
      points.erase(points.begin()+i);
      return true;
    }
  }
  return false;
}

void
Cluster::removeAllPoints() {
  points.clear();
}

/*
2.1.5.2 Class ~kMeans~

*/
kMeans::kMeans(int k, int iterations) {
  this->k = k; 
  this->iterations = iterations;
}

vector<Cluster>
kMeans::getClusters() {
  return clusters;
}

void
kMeans::clearClusters() {
  for (int i = 0; i < k; i++) {
    clusters[i].removeAllPoints();
  }
}

//Determine the closest Centroid for a given point
int
kMeans::getClosestClusterId(cPoint point) {
  double sum = 0.0; 
  double minDist;
  int closestClusterId; 

  //assign the point to the first cluster and calculate 
  //the distance to its centroid. Only change
  //that assignment if a closer centroid is found
  if(dimensions==1) {
    minDist = abs(clusters[0].getCentroidByPos(0) - point.getVal(0));
  } else {
    for (int i = 0; i < dimensions; i++) {
      sum += pow(clusters[0].getCentroidByPos(i) - point.getVal(i), 2.0);
    }
    minDist = sqrt(sum);
  }
  closestClusterId = clusters[0].getId();

  //check the points distance to other clusters
  for (int i = 1; i < k; i++) {
    double dist;
    sum = 0.0; 

    if (dimensions == 1) {
      dist = abs(clusters[i].getCentroidByPos(0) - point.getVal(0));
    } else {
      for (int j = 0; j < dimensions; j++) {
        sum+=pow(clusters[i].getCentroidByPos(j)-point.getVal(j),2.0);
      }
    dist = sqrt(sum); 
    }

    //if a closer cluster is found save the distance to its centroid
    //and change the Id of the closest cluster
    if (dist < minDist) {
      minDist = dist; 
      closestClusterId = clusters[i].getId();
    }
  }
  return closestClusterId;
}

//We pass a vector of all Points we will work on (i.e. all
//cPoints of the current Streamblock)
void
kMeans::cluster(vector<cPoint> &allPoints) {
  totalNbrPoints = allPoints.size();
  dimensions = allPoints[0].getDimensions();

  vector<int> usedPointIds; 

  //Initialize k Clusters with random centroids
  for (int i = 1; i <= k; i++) {
    while(true) {

      //Generate random index to choose centroids from 
      //all cPoints present in the current Streamblock
      int index = rand() % totalNbrPoints;

      //make sure no potential cPoint is selected twice as centroid
      if(find(usedPointIds.begin(), usedPointIds.end(), index) ==
          usedPointIds.end()) {
            usedPointIds.push_back(index);
            allPoints[index].setCluster(i);
            //The used cPoint at index becomes the centroid of 
            //the i-th cluster
            Cluster cluster(i, allPoints[index]);
            clusters.push_back(cluster);
            break;
      }
    }
  }

  int iteration = 1;
  //Just continue running this loop; Break condition is below 
  while (true) {
    bool done = true; 

    //Add all Points to their respective Clusters
    for (int i = 0; i < totalNbrPoints; i++) {
      int currentClusterId = allPoints[i].getCluster();
      int nearestClusterId = getClosestClusterId(allPoints[i]);

      if (currentClusterId != nearestClusterId) {
        allPoints[i].setCluster(nearestClusterId);
        done = false;
      }
    }

    //Clear current Cluster allocation of points in order to 
    //redistribute Points after finding closer Centroids
    clearClusters();

    //Reassign the Points to their new Clusters
    for(int i = 0; i < totalNbrPoints; i++) {
      //cluster index is Id-1
      clusters[allPoints[i].getCluster()-1].addPoint(allPoints[i]);
    }

    //Calculate the Centroids of each new Cluster
    for (int i = 0; i < k; i++) {
      int clusterSize = clusters[i].getSize();

      for (int j = 0; j < dimensions; j++) {
        double sum = 0.0;
        if (clusterSize > 0) {
          for (int p = 0; p < clusterSize; p++) {
            sum += clusters[i].getPoint(p).getVal(j);
          }
          clusters[i].setCentroidByPos(j, sum/clusterSize);
        }
      }
    }

    if (done || iteration >= iterations) {
      break; 
    }
    iteration++;
  }
}


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
2.2.2 Operator ~tiltedtime~

*/
ListExpr
tiltedtimeTM( ListExpr args ) {
NList type(args);
ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 3){
    return NList::typeError("Operator tiltedtime expects three arguments");
  }

  // test first argument for stream
  if(!(   type.first().hasLength(2)
       && type.first().first().isSymbol(sym.STREAM()))){
    return NList::typeError( "Operator tiltedtime expects a stream "
                             "as first argument");
  }

  // test second argument for int
  if(type.second() != NList(CcInt::BasicType())) {
    return NList::typeError("Operator tiltedtime expects an int "
                            "as second argument");
  }

  // test second argument for int
  if(type.second() != NList(CcInt::BasicType())) {
    return NList::typeError("Operator tiltedtime expects an int "
                            "as third argument");
  }
  
  // stream elements must be in kind DATA or (tuple X)
  NList streamtype = type.first().second();
  if(   !(   streamtype.hasLength(2)
          && streamtype.first().isSymbol(sym.TUPLE())
          && IsTupleDescription(streamtype.second().listExpr())
         )
     && !(am->CheckKind(Kind::DATA(),streamtype.listExpr(),errorInfo))){
      return NList::typeError("Operator tiltedtime expects a "
                              "stream of DATA or TUPLE.");
  }

  // result is the type of the first argument
  return type.first().listExpr();
}

/*
2.2.3 Operator ~createbloomfilter~

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

  /* result is a bloomfilter and we append the index of 
     the attribute of the tuples which will be hashed to create our filter 
  */
  appendList.append(NList().intAtom(attrIndex));

  return NList(Symbols::APPEND(), appendList, 
               ScalableBloomFilter::BasicType()).listExpr();
}

/*
2.2.4 Operator ~bloomcontains~

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

  // test second argument for DATA
  if(type.second().isAtom()) {
    return NList(CcBool::BasicType()).listExpr(); 
  }    

  return NList::typeError("Operator bloomcontains expects an  " 
                          "ATTRIBUTE as second argument");
}

/*
2.2.5 Operator ~createcountmin~

*/
ListExpr
createcountminTM( ListExpr args ) {
NList type(args);
NList streamtype = type.first().second();
NList appendList;
ListExpr streamType = nl->First(args);
ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 4){
    return NList::typeError("Operator createcountmin expects "
                            "four arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(streamType)){
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
  //so we can modify the way we hash;
  bool isNumeric = listutils::isNumericType(attrType);

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }

  /* result is a Count Min Sketch and we append the index and type of 
     the attribute of the tuples which will be hashed to create our filter 
  */
  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().boolAtom(isNumeric));
  return NList(Symbols::APPEND(), appendList,
               CountMinSketch::BasicType()).listExpr();
}

/*
2.2.6 Operator ~cmscount~

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
               CcInt::BasicType()).listExpr();
  } 

  return NList::typeError("Operator cmscount expects an  " 
                          "ATTRIBUTE as second argument");
}

/*
2.2.7 Operator ~createams~

*/
ListExpr
createamsTM( ListExpr args ) {
NList type(args);
NList streamtype = type.first().second();
NList appendList;
ListExpr streamType = nl->First(args);
ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 4){
    return NList::typeError("Operator createams expects "
                            "four arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(streamType)){
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

  /* result is a AMS Sketch and we append the index and type of 
     the attribute of the tuples which will be hashed to create our filter 
  */
  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().boolAtom(isNumeric));
  return NList(Symbols::APPEND(), appendList, 
               amsSketch::BasicType()).listExpr();
}

/*
2.2.8 Operator ~amsestimate~

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
  return NList(CcReal::BasicType()).listExpr(); 
}

/*
2.2.9 Operator ~createlossycounter~

*/
ListExpr
createlossycounterTM( ListExpr args ) {
NList type(args);
NList streamtype = type.first().second();
NList appendList;
ListExpr outlist = nl->TheEmptyList();
ListExpr streamType = nl->First(args);
ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 3){
    return NList::typeError("Operator createlossycounter expects "
                            "two arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(streamType)){
    return NList::typeError( "Operator createlossycounter expects a "
                             "Tuple Stream as first argument");
  }

  //test second argument for a valid Attribute Name
  if (!type.second().isSymbol()){
    return NList::typeError("Operator createlossycounter expects a valid "
                            "Attribute Name as second argument");
  }

  // test third argument for real
  if(type.third() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator createlossycounter expects a real "
                            "value as third argument");
  }
  
  // stream elements must be in kind tuple (X) with X in DATA
  if(!(streamtype.hasLength(2)
          && streamtype.first().isSymbol(sym.TUPLE())
          && IsTupleDescription(streamtype.second().listExpr())
         )
          && !(am->CheckKind(Kind::DATA(),streamtype.listExpr(),errorInfo))){
      return NList::typeError("Operator createlossycounter can only handle "
                              "Attributetype Tuple values");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.second().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }
  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().stringAtom(nl->ToString(attrType)));
  

  /* Result is a  Tuple Stream consisting of (Item, Frequency, Delta,
     Epsilon,EleCount) and we appended attribute Index and Type
  */
  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->FiveElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(nl->SymbolValue(attrType))
            ),       
            nl->TwoElemList(
              nl->SymbolAtom("Frequency"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Delta"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Epsilon"),
              nl->SymbolAtom(CcReal::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("EleCount"),
              nl->SymbolAtom(CcInt::BasicType())
            ) 
          )
        )
      );

  cout << endl; 
  cout << "Createlossycounter outlist looks like: " << endl; 
  cout << nl->ToString(outlist) << endl;
  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();
}

/*
2.2.10 Operator ~lcfrequent~

*/
ListExpr
lcfrequentTM(ListExpr args) {
  NList type(args);
  NList appendList;
  ListExpr outlist = nl -> TheEmptyList();


  // two arguments must be supplied
  if (type.length() != 2){
    return NList::typeError("Operator lcfrequent expects two arguments");
  }

  // test first argument for stream
  if(!(   type.first().hasLength(2)
       && type.first().first().isSymbol(sym.STREAM()))){
    return NList::typeError( "Operator lcfrequent expects a stream "
                             "as first argument");
  }

  //Every Stream of LossyCounter type will consist of 
  //(Value, Frequency, Delta, Error, EleCount) so we check for that

  string value = 
  nl->ToString((type.first().second().second().first().first()).listExpr());
  string frq = 
  nl->ToString((type.first().second().second().second().first()).listExpr());
  string dlt = 
  nl->ToString((type.first().second().second().third().first()).listExpr());
  string err = 
  nl->ToString((type.first().second().second().fourth().first()).listExpr());
  string cnt = 
  nl->ToString((type.first().second().second().fifth().first()).listExpr());

  cout << "In lcfrequent TM Checking for the Attribute types gave me: " << endl;
  cout << value << " " << frq << " " << dlt << " " << err << " " << cnt << endl;
  cout << endl;

  if(!( value == "Value") && (frq == "Frequency") && (dlt == "Delta") &&
      (err == "Epsilon") && (cnt == "EleCount")){
    return NList::typeError( "Operator lcfrequent expects a Stream generated "
                             "from a Lossy Counter as first argument");
  }

  // test second argument for real
  if(type.second() != NList(CcReal::BasicType())){
    return NList::typeError("Operator lcfrequent expects a "
                            "real value as second argument");
  }

  //extract the attributetype of the "Value" attribute
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = "Value";
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);

  appendList.append(NList().stringAtom(nl->ToString(attrType)));
  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(nl->SymbolValue(attrType))),       
            nl->TwoElemList(
              nl->SymbolAtom("Frequency"),
              nl->SymbolAtom(CcInt::BasicType())),
            nl->TwoElemList(
              nl->SymbolAtom("Delta"),
              nl->SymbolAtom(CcInt::BasicType())
            )           
          )
        )     
      );

        

  /* Result is a  Tuple Stream consisting of (Item, Frequency, Delta) 
     and we appended attribute Index and Type
  */
  cout << endl;
  cout << "Return type of lcfrequent() is: " << endl;
  cout << nl->ToString(NList(Symbols::APPEND(), appendList,
               outlist).listExpr());
  cout << endl;
  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();
}

/*
2.2.11 Operator ~outlier~

*/
ListExpr
outlierTM(ListExpr args) {
  NList type(args);
  NList stream = type.first();
  NList appendList;
  ListExpr outlist = nl -> TheEmptyList();

  // two arguments must be supplied
  if (type.length() != 3){
    return NList::typeError("Operator outlier expects three arguments");
  }


  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(stream.listExpr())){
    return NList::typeError( "Operator outlier expects a "
                             "Tuple Stream as first argument");
  }

  //test second argument for a valid Attribute Name
  if (!type.second().isSymbol()){
    return NList::typeError("Operator outlier expects a valid "
                            "Attribute Name as second argument");
  }


  // test third argument for real
  if(type.third() != NList(CcInt::BasicType())){
    return NList::typeError("Operator outlier expects an"
                            "int value as third argument");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.second().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  } 
  if ((nl->ToString(attrType) != CcInt::BasicType()) &&
       nl->ToString(attrType) != CcReal::BasicType()) {
        return NList::typeError("Operator outlier only works with "
                                "int and real attributes");
  }

  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(nl->SymbolValue(attrType))),       
            nl->TwoElemList(
              nl->SymbolAtom("StreamIndex"),
              nl->SymbolAtom(CcInt::BasicType()))
        )
      )
    );

    cout << endl; 
    cout << "In outlierTM outlist has the form: " << endl; 
    cout << nl->ToString(outlist) << endl;


  /* Result type is a stream of Tuples with the queried attribute value
     and its corresponding index in the stream. We also appended
     the attribute index and type for the Value Mapping
  */
  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().stringAtom(nl->ToString(attrType)));
  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();

}

/*
2.2.12 Operator ~streamcluster~

*/
ListExpr
streamclusterTM(ListExpr args) {
  NList type(args);
  NList stream = type.first();
  NList appendList;
  ListExpr outlist = nl -> TheEmptyList();

  // two arguments must be supplied
  if (type.length() != 4){
    return NList::typeError("Operator streamcluster expects four arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(stream.listExpr())){
    return NList::typeError( "Operator streamcluster expects a "
                             "Tuple Stream as first argument");
  }

  //test second argument for a valid Attribute Name
  if (!type.second().isSymbol()){
    return NList::typeError("Operator streamcluster expects a valid "
                            "Attribute Name as second argument");
  }

  // test third argument for real
  if(type.third() != NList(CcInt::BasicType())){
    return NList::typeError("Operator streamcluster expects an"
                            "int value as third argument");
  }

  // test fourth argument for real
  if(type.fourth() != NList(CcInt::BasicType())){
    return NList::typeError("Operator streamcluster expects an"
                            "int value as fourth argument");
  }


  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.second().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  } 
  if ((nl->ToString(attrType) != CcInt::BasicType()) &&
       nl->ToString(attrType) != CcReal::BasicType()) {
        return NList::typeError("Operator streamcluster only works with "
                                "int and real attributes");
  }

  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("ClusterId"),
              nl->SymbolAtom(CcInt::BasicType())),       
            nl->TwoElemList(
              nl->SymbolAtom("Centroid"),
              nl->SymbolAtom(CcReal::BasicType())),
            nl->TwoElemList(
              nl->SymbolAtom("Centroid Size"),
              nl->SymbolAtom(CcInt::BasicType())
            )
         )
      )
    );

  /* Result type is a stream of Tuples with the queried attribute value
     and its corresponding index in the stream. We also appended
     the attribute index and type for the Value Mapping
  */
  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().stringAtom(nl->ToString(attrType)));
  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();

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
template<class T> 
class reservoirInfo{
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
2.3.2 Operator ~tiltedtime~

*/
template<class T> 
class tiltedtimeInfo{
  public: 
    tiltedtimeInfo(Word inputStream, int inputWindowSize, int type): 
                  stream(inputStream), frameSize(inputWindowSize), 
                  lastOut(0), maxFrameIndex(0), nextFrameIndex(0),
                  timeStamp(0), count(0) {
    stream.open();
    //We denote the starting time of the stream
    startTime = chrono::high_resolution_clock::now();
    init();                               
  }


~tiltedtimeInfo() {
  for(size_t windowIndex = nextFrameIndex; windowIndex < reservoir.size(); 
      windowIndex++) {
    cout << endl;
    cout << "Starting to parse window " << windowIndex << endl;
    if (reservoir[windowIndex].size() != 0) {
      cout << "Window " << windowIndex << " does contain elements" << endl;
      for (size_t eleIndex = lastOut+1; 
           eleIndex < reservoir[windowIndex].size(); eleIndex++) {
        cout << "Trying to delete Item " << eleIndex 
             << " of window " << windowIndex << endl; 
        reservoir[windowIndex][eleIndex]->DeleteIfAllowed(); 
      }
    }
  }
  stream.close();
}

//Returns the Elements in the reservoir in case of requests
T* next() {
  cout << "nextFrameIndex in next() is: " << nextFrameIndex  << endl;
  cout << endl;
  cout << "Number of buckets at the end of the stream is: " 
       << reservoir.size() << " while maxFrameIndex is: " 
       << maxFrameIndex << " and windowSize is: " << reservoir[0].size() 
       << endl;
  cout << endl; 
  cout << "Number of entries in the Buckets are: " << endl; 
  for (auto vector : reservoir) {
    cout << vector.size() << " " << endl;
  }

  //We exhausted all elements in the current window
  if (((lastOut >= (int) reservoir[nextFrameIndex].size()) 
       || (reservoir[nextFrameIndex].size() == 0)) 
       && (nextFrameIndex < (int) reservoir.size())) {
         cout << "Exhausted a bucket" << endl;
    //Theoretically successive Windows could be empty
    //We need to prevent returning items from them
    nextFrameIndex++;
    lastOut = 0;
    while (int i = 0 < reservoir.size()) {
      //Window does not have any entries, we continue increasing
      //the Windowindex
      if (reservoir[nextFrameIndex].size() == 0) {
        cout << "nextBucket.size=0; increasing bucket index" << endl;
        nextFrameIndex++;
        i++;
      } else {
        cout << "nextbucket has Items" << endl;
        break;
      }
    }
  }

  //We passed through the last window; All elements have been passed
  if((lastOut >= (int)reservoir[nextFrameIndex].size()) && 
     (nextFrameIndex >= (int)reservoir.size())) {
      cout << "Exhausted all items; returning 0" << endl;
    return 0;
  }

  T* resElement = reservoir[nextFrameIndex][lastOut];
  reservoir[nextFrameIndex][lastOut] = 0;
  lastOut++;
  cout << "returning tuple" << endl;
  return resElement;
}

private: 
  Stream<T> stream; 
  int frameSize;
  int lastOut;
  int maxFrameIndex;
  int nextFrameIndex;
  vector<vector<T*>> reservoir;
  vector<T*> outputReservoir;
  std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> currentTime;
  std::chrono::duration<double> passedTime;
  size_t timeStamp;
  int count;

  void init() {
    T* data;
    reservoir.resize(1);
    reservoir[0].reserve(frameSize);
    data = stream.request(); 
    reservoir[0].push_back(data);
    cout << "In init reservoir[0].size() = " << reservoir[0].size() << endl;
    //While the Argumentstream can still supply Data/Tuples 
    while ((data = stream.request()) != nullptr) {
      //save the time of receiving the Streamelement
      currentTime = chrono::high_resolution_clock::now();
      //save the difference between the starting time of the stream and 
      //the time of receiving the element in microseconds.
      //A finer time unit (mb nanoseconds) could be chosen; 
      //need to talk about this    
      passedTime = chrono::duration_cast<std::chrono::seconds>
                   (currentTime - startTime);
      insert(data);
    } 
  }

  //Decides whether to include Data/Tuples from the Stream into the reservoir
  void insert(T* data) {
    //int timeStamp = passedTime.count();
    cout << "When starting to insert reservoir size is: " 
         << reservoir.size() << endl; 
    timeStamp = timeStamp + rand() % 10;
    cout << endl; 
    cout << "Timestamp for argument with Index " << count 
         << " is: " << timeStamp << endl;
    cout << endl; 
    
    count++; 
    //We have to fulfill (log2(timestmap)-windowSize <= maxFrameIndex 
    //<= log2(T))
    calculateMaxIndex();
    
    bool inserted = false;
    cout << endl; 
    cout << "Trying to find insert Window Index for T: " 
         << timeStamp << endl; 
    for (int i = maxFrameIndex; i >= 0; i--) {
      if (checkIndex(i)) {
            cout << "Found window index to be: " << i << endl;
        if ((int) reservoir[i].size() < frameSize) {
          cout << "Chosen Window with index " << i << " still has room" << endl;
          cout << endl;
          //We insert, so that the oldest elements are always
          //at the end of the vector
          cout << "Trying to insert Data" << endl;
          cout << "Current size of the Window we insert into is: " 
               << reservoir[i].size() << endl;
          reservoir[i].insert(reservoir[i].begin(), data);
          cout << "Inserted Data" << endl;
          inserted = true;
          break;
        } else {
          cout << endl;
          cout << "Window we want to insert into is full" << endl;
          //remove the last element, because it i s the oldest
          reservoir[i].back()->DeleteIfAllowed();
          cout << "Window size after deleteifallowed() is: " 
               << reservoir[i].size() << endl;
          reservoir[i].pop_back();
          cout << "Poped the element at the back size now: " 
               << reservoir[i].size() << endl; 
          //insert the new element at the front
          reservoir[i].insert(reservoir[i].begin(), data);
          cout << "Inserted new element at the front" << endl;
          cout << "Window size is now: " << reservoir[i].size() << endl;
          inserted = true;
          break;
        }
      }
    }
      //none of the previous i fulfilled our requirement and thus 
      //i > maxFrameNumber so we have to insert the element into 
      //the frame with the highest index 
      if (!inserted) {
        cout << "No i fulfilled the inequality; inserting into maxframe" 
             << endl;
        if ((int) reservoir[maxFrameIndex].size() < frameSize) {
          cout << endl;
          cout << "Window we tried to insert into was not full yet" <<endl;  
          reservoir[maxFrameIndex].insert(
                       reservoir[maxFrameIndex].begin(), data);
        } else {
          cout << endl;
          cout << "Window we tried to insert into was full already" << endl;
          cout << endl;
          //remove the last element, because it is the oldest
          reservoir[maxFrameIndex].back()->DeleteIfAllowed();
          cout << "Window size after deleteifallowed() is: " 
               << reservoir[maxFrameIndex].size() << endl;
          reservoir[maxFrameIndex].pop_back();
          cout << "Poped the element at the back size now: " 
               << reservoir[maxFrameIndex].size() << endl;     
          cout << "Inserted new element at the front" << " ";
          reservoir[maxFrameIndex].insert(
                                   reservoir[maxFrameIndex].begin(), data);
          cout << "Window size is now: " 
               << reservoir[maxFrameIndex].size() << endl;
        }
      }
  }

  //nothing implemented for negatives, cause the exponent cant be < 0 
  int
  squareExp(long base, long exp) {
    if (exp == 0) {
      return 1;
    } else if (exp == 1) {
      return base; 
    } else if ((exp % 2) == 0) {
      return squareExp(base*base, exp/2);
    } else {
      return base*(squareExp(base*base, (exp-1)/2));
    }
  }
 
  bool
  checkIndex(int i) {
    long condition = squareExp(2,i);     
    bool equality = (timeStamp % condition) == 0;
    bool inequality = (timeStamp % (2*condition)) != 0;
    return equality && inequality;
  }

  void
  calculateMaxIndex() {
    cout << endl;
    cout << "Recalculating maxFrameIndex" << endl;
    float lowerBound = log2(timeStamp)-frameSize;
    float upperBound = log2(timeStamp);
    if (lowerBound > maxFrameIndex) {
      //This way we always fullfil frameIndex <= log2(timeStamp)
      maxFrameIndex = floor(upperBound);
      cout << endl;
      cout << "maxFrameIndex was too small and was readjusted to: " 
           << maxFrameIndex << endl;
      //resize the reservoir so that we can access reservoir[maxFrameIndex]
      if (maxFrameIndex > (int) reservoir.size()) {
        cout <<  "maxFrameIndex is bigger then reservoir size" << endl;
        reservoir.resize(maxFrameIndex+1);
        cout << "Resized reservoir to size: " << reservoir.size() << endl; 
      }
    }
  }
};

template<class T>
int tiltedtimeVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){

  tiltedtimeInfo<T>* tti = (tiltedtimeInfo<T>*) local.addr;
  switch(message){
    case OPEN: {
                   if(tti) {
                     delete tti;
                     local.addr = 0;
                   }
                   CcInt* reservoirSize = (CcInt*) args[1].addr;
                   CcInt* ttType = (CcInt*) args[2].addr; 
                   if(reservoirSize->IsDefined()){
                      int size = reservoirSize->GetValue();
                      int type = ttType->GetValue();
                      if(size>0) {
                        local.addr = new tiltedtimeInfo<T>(args[0], size, type);
                      }
                   }
                   return 0;
                } 
      case REQUEST : result.addr = tti?tti->next():0;
                     return result.addr?YIELD:CANCEL;
      case CLOSE : {
                      if(tti){
                        delete tti;
                        local.addr = 0;
                       }
                       return 0;
                   }

  }
  return -1;
}

//value Mapping Array
ValueMapping tiltedtimeVM[] = { 
              tiltedtimeVMT<Tuple>,
              tiltedtimeVMT<Attribute>,
};  

// Selection Function
int tiltedtimeSelect(ListExpr args){
   if (Stream<Tuple>::checkType(nl->First(args))){
     return 0;
   } else if (Stream<Attribute>::checkType(nl->First(args))){
     return 1;
   } else {
     return -1;
   }
}


/*
2.3.3 Operator ~createbloomfilter~

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
2.3.4 Operator ~bloomcontains~

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
2.3.5 Operator ~createcountmin~

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
2.3.6 Operator ~cmscount~

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
2.3.7 Operator ~createams~

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

  cout << "After init() AMS Values are: " << endl;
  cout << endl;
  cout << "Defined: " + ams->getDefined() << endl;
  cout << "Epsilon: " << + ams->getEpsilon() << endl;
  cout << "Delta: " << + ams->getDelta() << endl;
  cout << "Width: "  << endl;
  cout <<  ams->getWidth();
  cout << endl;
  cout << "Depth: " << + ams->getDepth() << endl;
  cout << "Total Count " << + ams->getTotalCount() << endl;

  cout << "VV enthält: " << ams->getMatrix().size() << endl;

  for (size_t i = 0; i < ams->getDepth(); i++) {
    cout << "V" << i << " length is: " << ams->getMatrix()[i].size() << endl;
  }


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
2.3.8 Operator ~amsestimate~

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
  float estimate = ams -> estimateInnerProduct();

  res->Set(true, estimate);

  return 0;
}

/*
2.3.9 Operator ~createlossycounter~

*/
template<class T, class S> 
class createlossycounterInfo{
    public: 
    createlossycounterInfo(Word inputStream, float epsilon, int index, 
                           string type):
    stream(inputStream), error(epsilon), attrIndex(index), attrType(type), 
    lastOut(-1), counter(epsilon){
      stream.open();
      typeList = nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->FiveElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(attrType)
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Frequency"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Delta"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Error"),
              nl->SymbolAtom(CcReal::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Element Count"),
              nl->SymbolAtom(CcInt::BasicType())
            )            
          )
        );
      SecondoCatalog* sc = SecondoSystem::GetCatalog();
      numTypeList = sc->NumericType(typeList);
      tupleType = new TupleType(numTypeList);
      init();
    }

    ~createlossycounterInfo() {
      for(size_t index = lastOut+1; index < frequencyList.size(); index++) {
        frequencyList[index]->DeleteIfAllowed();
      }
      tupleType -> DeleteIfAllowed();
      stream.close();
    }

    Tuple* next() {
      lastOut++; 
      if (lastOut >= (int) frequencyList.size()) {
        return 0;
      }
      Tuple* frequentItem = frequencyList[lastOut];
      frequencyList[lastOut] = 0;
      return frequentItem;
    }    

    private: 
      Stream<Tuple> stream;
      float error;
      int attrIndex;
      string attrType;
      int lastOut;
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;
      lossyCounter<S> counter;
      vector<Tuple*> frequencyList; 

    void init() {
      Tuple* oldTuple;
      while ((oldTuple = stream.request()) != nullptr) {
          T* attrValue = (T*) oldTuple->GetAttribute(attrIndex);
          S value = attrValue ->GetValue();
          counter.addElement(value);
          oldTuple -> DeleteIfAllowed();
      }
      output();
    }

    void output() {
      cout << endl; 
      cout << "Created Counter holds the values: " << endl;
      for (auto elements : counter.getFrequencyList()) {
        counterPair<S> elem = elements.second;
        Tuple* newTuple = new Tuple(tupleType);
        S value = elem.getItem();
        T* attrValue = new T(true, value);
        int frequency = elem.getFrequency();
        CcInt* attrFrequency = new CcInt(true, frequency);
        int delta = elem.getMaxError();
        CcInt* attrDelta = new CcInt(true, delta);
        CcReal* attrError = new CcReal(true, error);
        int eleCount = counter.getEleCounter();
        CcInt* attrCount = new CcInt(true, eleCount);
        cout << "Value: " << value << " Frequency: " << frequency 
             << " Delta: " << delta << " Error:  " << error 
             << " Element Count: " << eleCount << endl;
        //Generate the output tuple 
        newTuple->PutAttribute(0, attrValue);
        newTuple->PutAttribute(1, attrFrequency);
        newTuple->PutAttribute(2, attrDelta);
        newTuple->PutAttribute(3, attrError);
        newTuple->PutAttribute(4, attrCount);
        frequencyList.push_back(newTuple);
      }
    }   
};


template<class T, class S>
int createlossycounterVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){


  createlossycounterInfo<T,S>* lc = (createlossycounterInfo<T,S>*) local.addr;
  switch(message){
    case OPEN: {
                   if(lc) {
                     delete lc;
                     local.addr = 0;
                   }
                   CcReal* epsilon = (CcReal*) args[2].addr;
                   CcInt* attrIndex = (CcInt*) args[3].addr;
                   CcString* attrType = (CcString*) args[4].addr;
                   if(epsilon->IsDefined()){
                      float error = epsilon->GetValue();
                      int index = attrIndex -> GetValue();
                      string type = attrType ->GetValue();
                                            cout << endl;
                      if((error < 1) && (error > 0)) {
                        local.addr = new createlossycounterInfo<T,S>(args[0], 
                                     error, index, type);
                      }
                   }
                   return 0;
                } 
      case REQUEST :
      result.addr = lc?lc->next():0;
                     return result.addr?YIELD:CANCEL;
      case CLOSE : {
                      if(lc){
                        delete lc;
                        local.addr = 0;
                       }
                       return 0;
                   }

  }
  return -1;
}

//value Mapping Array
ValueMapping  createlossycounterVM[] = { 
              createlossycounterVMT<CcInt,int>,
              createlossycounterVMT<CcReal,float>,
              createlossycounterVMT<CcString,string>,
              createlossycounterVMT<CcBool,bool>
};  

// Selection Function
int createlossycounterSelect(ListExpr args){
  NList type(args);
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.second().str();
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);
  
  cout << endl;
  cout << "LossyCounterSelection identified AttrType as: " 
       << nl->ToString(attrType) << endl;
  cout << endl;
  
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    cout << "Returned Int" << endl;
    cout << endl;
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    cout << "Returned Real" << endl;
    cout << endl;
    return 1;
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    cout << "Returned String" << endl;
    cout << endl;
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    cout << "Returned bool" << endl;
    cout << endl;
    return 3;
  } else {
    return -1;
  }
}


/*
2.3.10 Operator ~lcfrequent~

*/
template<class T>
class lcFrequentInfo{
  public: 
    lcFrequentInfo(Word inputStream, float minSup, string type): 
                   stream(inputStream), minSupport(minSup), attrType (type),
                   lastOut(-1){

      stream.open();
      typeList = nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(attrType)
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Frequency"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Delta"),
              nl->SymbolAtom(CcInt::BasicType())
            )            
          )
        );
      cout << endl; 
      cout << "TupleType generated in lcfrequentinfo() looks like: " << endl;
      cout << nl->ToString(typeList) << endl; 
      cout << endl; 
      cout << "AttrType in lcfrequentinfo() is: " << attrType << endl;

      SecondoCatalog* sc = SecondoSystem::GetCatalog();
      numTypeList = sc->NumericType(typeList);
      tupleType = new TupleType(numTypeList);
      init();
    }

    ~lcFrequentInfo() {
      for(size_t index = lastOut+1; index < aboveMinSup.size(); index++) {
        aboveMinSup[index]->DeleteIfAllowed();
      }
      tupleType -> DeleteIfAllowed();
      stream.close();
    }

    Tuple* getNext() {
      lastOut++; 
      if(lastOut >= (int)aboveMinSup.size()) {
        return 0;
      }
      Tuple* frequentElement = aboveMinSup[lastOut];
      aboveMinSup[lastOut] = 0;
      return frequentElement;     
    }

    private:
      Stream<Tuple> stream;
      float minSupport;
      string attrType; 
      int lastOut;
      float epsilon; 
      int nbrElements;
      vector<Tuple*> aboveMinSup;
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;

    void init() {
      Tuple* oldTuple = stream.request();
      epsilon = ((CcReal*) oldTuple->GetAttribute(3))->GetValue();
      nbrElements = ((CcInt*) oldTuple->GetAttribute(4))->GetValue();
      int frequency = ((CcInt*) oldTuple->GetAttribute(1))->GetValue();
       if (isFrequent(frequency)) {
          Tuple* newTuple = new Tuple(tupleType);
          T* attrValue = (T*) oldTuple->GetAttribute(0);
          //Since we are going to delete the old tuple we have to make sure 
          //the reference count of the Attributes we want to carry over into 
          //the new tuple does not go to 0.
          attrValue -> Copy();
          CcInt* attrFrequency = (CcInt*)oldTuple->GetAttribute(1);
          attrFrequency -> Copy();
          CcInt* attrDelta = (CcInt*) oldTuple->GetAttribute(2);
          attrDelta -> Copy();
          newTuple->PutAttribute(0, attrValue);
          newTuple->PutAttribute(1, attrFrequency);
          newTuple->PutAttribute(2, attrDelta);
          aboveMinSup.push_back(newTuple);
          oldTuple->DeleteIfAllowed();
        } else {
          oldTuple->DeleteIfAllowed();
        }
      while ((oldTuple = stream.request()) != nullptr) {
        int frequency = ((CcInt*) oldTuple->GetAttribute(1))->GetValue();
        if (isFrequent(frequency)) {
          Tuple* newTuple = new Tuple(tupleType);
          T* attrValue = (T*) oldTuple->GetAttribute(0);
          attrValue -> Copy();
          CcInt* attrFrequency = (CcInt*) oldTuple->GetAttribute(1);
          attrFrequency -> Copy();
          CcInt* attrDelta = (CcInt*) oldTuple->GetAttribute(2);
          attrDelta -> Copy();
          newTuple->PutAttribute(0, attrValue);
          newTuple->PutAttribute(1, attrFrequency);
          newTuple->PutAttribute(2, attrDelta);
          aboveMinSup.push_back(newTuple);
          oldTuple->DeleteIfAllowed();
        } else {
          oldTuple->DeleteIfAllowed();
        }
      }
    }

      bool isFrequent(int frequency) {
        return (((minSupport-epsilon)*nbrElements) <= frequency);
      }
};

template<class T>
int lcfrequentVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){

    lcFrequentInfo<T>* freqInf = (lcFrequentInfo<T>*) local.addr;
    switch(message){
      case OPEN: {
                  if(freqInf) {
                    delete freqInf;
                    local.addr = 0;
                  }
                  CcReal* minSupport = (CcReal*) args[1].addr;
                  CcString* attrType = (CcString*) args[2].addr;
                  if (minSupport -> IsDefined()) {
                    float minSup = minSupport->GetValue();
                    string type = attrType->GetValue();
                    cout << "LcFrequentVMT AttrType: " << type << endl;
                    if ((minSup > 0) && (minSup <= 1)) {
                      local.addr = new 
                      lcFrequentInfo<T>(args[0].addr, minSup, type);
                    }
                  }
                  return 0;
                }
      case REQUEST: result.addr = freqInf?freqInf->getNext():0;
                    return result.addr?YIELD:CANCEL;
      case CLOSE: {
                    if(freqInf){
                      delete freqInf;
                      local.addr = 0;
                    }
                   return 0;
                  }
  }
    return -1;
}

//value Mapping Array
ValueMapping  lcfrequentVM[] = { 
              lcfrequentVMT<CcInt>,
              lcfrequentVMT<CcReal>,
              lcfrequentVMT<CcString>,
              lcfrequentVMT<CcBool>
};  

// Selection Function
int lcfrequentSelect(ListExpr args){
  NList type(args);
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = "Value";
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);
  
  cout << endl;
  cout << "lcfrequent identified AttrType as: " 
       << nl->ToString(attrType) << endl;
  cout << endl;
  
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    cout << "Returned Int" << endl;
    cout << endl;
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    cout << "Returned Real" << endl;
    cout << endl;
    return 1;
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    cout << "Returned String" << endl;
    cout << endl;
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    cout << "Returned bool" << endl;
    cout << endl;
    return 3;
  } else {
    return -1;
  }
}

/*
2.3.11 Operator ~outlier~

*/
template<class T, class S>
class outlierInfo{
  public:
    outlierInfo(Word inputStream, int zScoreInput, int index, string type):
      stream(inputStream), zThreshhold(zScoreInput), attrIndex(index),  
      attrType(type), mean(0), variance(0), lastOut(-1), counter(0){
        stream.open();

        //We appended the attribute Type in the TM and use it to create our
        //output tuple
        typeList = nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(attrType)
            ),
            nl->TwoElemList(
              nl->SymbolAtom("StreamIndex"),
              nl->SymbolAtom(CcInt::BasicType())
            )
          )
        );

        SecondoCatalog* sc = SecondoSystem::GetCatalog();
        numTypeList = sc->NumericType(typeList);
        tupleType = new TupleType(numTypeList);
        init();
      }

    ~outlierInfo() {
      for(size_t index = lastOut+1; index < outlierHistory.size(); index++) {
        outlierHistory[index]->DeleteIfAllowed();
        tupleType -> DeleteIfAllowed();
      }
      stream.close();
    }

    Tuple* next() {
      lastOut++; 
      if (lastOut >= (int) outlierHistory.size()) {
        return 0;
      }
      Tuple* outlier = outlierHistory[lastOut];
      outlierHistory[lastOut] = 0;
      return outlier;
    }

    private:
      Stream<Tuple> stream; 
      int zThreshhold;
      int attrIndex;
      string attrType;
      double mean; 
      double variance;
      int lastOut;
      size_t counter;
      vector<Tuple*> outlierHistory;
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;


    void init() {
      Tuple* oldTuple;
      Tuple* newTuple = new Tuple(tupleType); 
      oldTuple = stream.request(); 
      T* attrValue = (T*) oldTuple->GetAttribute(attrIndex);
      CcInt* index = new CcInt(true, counter);

      //We will consider the first Tuple Value to be an outlier since we 
      //have no way of knowing what the stream will actually look like
      newTuple -> PutAttribute(0, attrValue);
      newTuple -> PutAttribute(1, index);
      outlierHistory.push_back(newTuple);

      //We use the second template type to be able to handle the values
      S value = (S) attrValue->GetValue();
      updateData(value);
      while ((oldTuple = stream.request()) != nullptr) {
          Tuple* newTuple = new Tuple(tupleType);
          T* attrValue = (T*) oldTuple->GetAttribute(attrIndex);
          CcInt* index = new CcInt(true, counter);
          S value = attrValue->GetValue();
          if (checkData(value)) {
            newTuple -> PutAttribute(0, attrValue);
            newTuple -> PutAttribute(1, index);
            outlierHistory.push_back(newTuple);
            if (outlierHistory.size() == 1589) {
              cout << endl;
              cout << "Outlier history now contains: " << endl; 
              for (auto elem : outlierHistory) {
               T* attrValue = (T*) elem->GetAttribute(0);
                CcInt* attrIndex = (CcInt*) elem->GetAttribute(1);
                S attr = attrValue -> GetValue();
                int index = attrIndex -> GetValue();
                cout << "Value: " << attr << " Index: " << index << endl;
                cout << endl;
              }
            }
          } else {
            oldTuple -> DeleteIfAllowed();
          }
          updateData(value);
        }
    }

    /*Check whether the currently handled Streamelements
      z-Score surpases our treshholds and save it if it does
    */
    bool checkData(S data) {
      int zscore;
      if (data == 0) {
        zscore = 0;
      } else {
        zscore = (data - mean)/sqrt(variance);
      }
      if((zscore < (-1*zThreshhold)) || (zscore > zThreshhold)) {
        return true; 
      }
      return false;
    }

  //Update mean, variance and the counter
  void updateData(S data) {    
    //Calculate the first part of the variance for the i+1th element
    variance = ((variance + pow(mean, 2)) * counter + 
                pow(data, 2))/(counter+1);

    //Calculate the mean for the i+1th element
    mean = ((mean * counter) + data)/(counter+1);
  
    //finish the calculation of the variance
    variance = variance - pow(mean,2);

    counter++;
  }
};

template<class T, class S> int
outlierVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){

  outlierInfo<T,S>* outlier = (outlierInfo<T,S>*) local.addr;
  switch(message){
    case OPEN: {
                   if(outlier) {
                     delete outlier;
                     local.addr = 0;
                   }
                   CcInt* threshold = (CcInt*) args[2].addr;
                   CcInt* attrIndex = (CcInt*) args[3].addr;
                   CcString* attrType = (CcString*) args[4].addr;
                   if(threshold->IsDefined()){
                      int zThreshold = threshold->GetValue();
                      int index = attrIndex -> GetValue();
                      string type = attrType ->GetValue();
                                            cout << endl;
                      if(zThreshold>1) {
                        local.addr = new outlierInfo<T,S>(args[0], zThreshold, 
                                                     index, type);
                      }
                   }
                   return 0;
                } 
      case REQUEST :
      result.addr = outlier?outlier->next():0;
                     return result.addr?YIELD:CANCEL;
      case CLOSE : {
                      if(outlier){
                        delete outlier;
                        local.addr = 0;
                       }
                       return 0;
                   }

  }
  return -1;
}

//value Mapping Array
ValueMapping outlierVM[] = { 
              outlierVMT<CcInt, int>,
              outlierVMT<CcReal, float>
};  

// Selection Function
int outlierSelect(ListExpr args){
  NList type(args);
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.second().str();
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);
  
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    return 1;
  } else {
    return -1;
  }
}


/*
2.3.12 Operator ~streamcluster~

*/
class streamclusterInfo{
  public:
    streamclusterInfo(Word inputStream, int index, string type,
                      int nbrClusters, int iter):
      stream(inputStream), attrIndex(index), attrType(type), lastOut(-1),
      k(nbrClusters), iterations(iter){
        
        cout << endl; 
        cout << "Created new StreamClusterInfo" << endl;
        stream.open();
        //We appended the attribute Type in the TM and use it to create our
        //output tuple
        typeList = nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("ClusterId"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Centroid"),
              nl->SymbolAtom(CcReal::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Centroid Size"),
              nl->SymbolAtom(CcInt::BasicType())
            )
          )
        );
        SecondoCatalog* sc = SecondoSystem::GetCatalog();
        numTypeList = sc->NumericType(typeList);
        tupleType = new TupleType(numTypeList);
        cout << "Return Tuple Type from Info has the form: " << endl; 
        cout << nl->ToString(typeList) << endl;
        init();
      }

    ~streamclusterInfo() {
      for(size_t index = lastOut+1; index < clusterInformation.size(); 
          index++) {
        clusterInformation[index]->DeleteIfAllowed();
        tupleType -> DeleteIfAllowed();
      }
      stream.close();
    }

    Tuple* next() {
      lastOut++; 
      if (lastOut >= (int) clusterInformation.size()) {
        return 0;
      }
      Tuple* cluster = clusterInformation[lastOut];
      clusterInformation[lastOut] = 0;
      return cluster;
    }

    private:
      Stream<Tuple> stream; 
      int attrIndex;
      string attrType;
      int lastOut;
      int k;
      int iterations;
      vector<Tuple*> clusterInformation;
      vector<cPoint> readData;  
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;


    void init() {
      cout << endl;
      cout << "Init() streamClusterInfo" << endl;
      Tuple* oldTuple;
      int id = 0;
      while ((oldTuple = stream.request()) != nullptr) {
        CcInt* attrValue = (CcInt*) oldTuple->GetAttribute(attrIndex);
        int value = (int) attrValue->GetValue();
        cout << "Stream Tuple Attr Value is: " << value << endl;
        cPoint point(id, value);
        cout << "test" << endl;
        cout << "Created new point with ID " << point.getId() 
             << " and Value " << point.getVal(0) << endl;
        readData.push_back(point);
        id++;
        oldTuple->DeleteIfAllowed();
      }
      compute();
    }

    void compute() {
      cout << endl;
      cout << "Starting compute()" << endl;
      kMeans clusteringTest(k,iterations);
      cout << "Created new kMeans Structure" << endl; 
      cout << "Points handed over for Clustering are: " << endl; 
      for (auto point : readData) {
        cout << "ID: " << point.getId() << " Value: " 
                       << point.getVal(0) << endl; 
      }
      clusteringTest.cluster(readData);
      for (Cluster currentCluster : clusteringTest.getClusters()) {
        Tuple* newTuple = new Tuple(tupleType);
        int clusterId = currentCluster.getId();
        CcInt* attrId = new CcInt(true, clusterId);
        int clusterSize = currentCluster.getSize();
        CcInt* attrSize = new CcInt(true, clusterSize);
        double clusterCentroid = currentCluster.getCentroidByPos(0);
        CcReal* attrCentroid = new CcReal(true, clusterCentroid);
        newTuple -> PutAttribute(0, attrId);
        newTuple -> PutAttribute(1, attrCentroid);
        newTuple -> PutAttribute(2, attrSize);
        cout << endl;
        cout << "Pushing back Cluster: " << clusterId << " with Centroid: " 
             << clusterCentroid << " and size: " << clusterSize << endl;
        cout << endl; 
        clusterInformation.push_back(newTuple);        
      }
      cout << "At the end of compute() the Tuple Vector has size: " 
           << clusterInformation.size() << endl;
    }
};

int
streamclusterVM(Word* args, Word& result,
           int message, Word& local, Supplier s){

  streamclusterInfo* cluster = (streamclusterInfo*) local.addr;
  switch(message){
    case OPEN: {
                   if(cluster) {
                     delete cluster;
                     local.addr = 0;
                   }
                   CcInt* nbrOfClusters = (CcInt*) args[2].addr;
                   CcInt* iterations = (CcInt*) args[3].addr;
                   CcInt* attrIndex = (CcInt*) args[4].addr;
                   CcString* attrType = (CcString*) args[5].addr;
                   if(iterations->IsDefined() && nbrOfClusters->IsDefined()){
                      int iter = iterations->GetValue();
                      int k = nbrOfClusters -> GetValue();
                      int index = attrIndex -> GetValue();
                      string type = attrType ->GetValue();
                                            cout << endl;
                      if(k>1) {
                        local.addr = new streamclusterInfo(args[0], index, 
                                                           type, k, iter);
                      }
                   }
                   return 0;
                } 
      case REQUEST :
      result.addr = cluster?cluster->next():0;
                     return result.addr?YIELD:CANCEL;
      case CLOSE : {
                      if(cluster){
                        delete cluster;
                        local.addr = 0;
                       }
                       return 0;
                   }

  }
  return -1;
}

/*
2.4 Description of Operators

*/

  OperatorSpec reservoirSpec(
    "stream(T) x int -> stream(T), T = TUPLE or T = DATA",
    "_ reservoir [_] ",
    "Creates a reservoir sample of a supplied stream of a given size ",
    "query intstream(1,1000) reservoir[10] count"
  );

   OperatorSpec tiltedtimeSpec(
    "stream(T) x int x int -> stream(T), T = TUPLE or T = DATA",
    "_ tiltedtime [_,_] ",
    "Creates either a natural, logarithmic or progressive logarthmic",
    " sample of a stream, depending on the third argument",
    "query intstream(1,1000) tiltedtime[3,10] count"
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
    "stream(tuple(X)) x ATTR x int x real ->  amssketch",
    "_ createamsSpec [_,_,_]",
    "Creates an AMS Sketch fpor the supplied stream", 
     "query Kinos feed createams[Name,0.01,0.9] amsestimate"
  );

  OperatorSpec amsestimateSpec(
    "amssketch -> real",
    "_ amsestimate ",
    "Creates and estimate of the F_2 Moment of the ",
    "given AMS-Sketch"
  );

  OperatorSpec createlossycounterSpec(
    "stream(tuple(X)) x ATTR x real ->  lossycounter",
    "_ createlossycounter [_,_]",
    "Creates a lossy Counter the supplied stream",
    "query intstream(0,100) createlossycounter lcfrequent"
  );

  OperatorSpec lcfrequentSpec(
    "lossycounter(X) -> stream(ATTR)",
    "_ lcfrequent [_]",
    "Displays the items determined to be frequent by the lossy Counter",
    "query intstream(0,100) createlossycounter lcfrequent"
  );

  OperatorSpec outlierSpec(
    "stream(T) x ATTR x real -> stream(T), T = int or T = real",
    "_ outlier [_,_]",
    "Determines outliers for an int/real stream according to the ",
    "passed Z-Score Threshold.",
    "query intstream(0,100) outliers[Elem, 3] consume"
  );

  OperatorSpec streamclusterSpec(
    "stream(T) x ATTR x int x int -> stream(T), T = int or T = real",
    "_ streamcluster [_,_,_]",
    "Determines Cluster for an int/real stream using ",
    "the number of clusters and iterations passed",
    "query intstream(0,100) outliers[Elem,2,3] consume"
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

Operator tiltedtimeOp(
  "tiltedtime",
  tiltedtimeSpec.getStr(),
  2,
  tiltedtimeVM,
  tiltedtimeSelect, 
  tiltedtimeTM
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

Operator createamsOp(
  "createams",
  createamsSpec.getStr(),
  createamsVM,
  Operator::SimpleSelect,
  createamsTM
);

Operator amsestimateOp(
  "amsestimate",
  amsestimateSpec.getStr(),
  amsestimateVM,
  Operator::SimpleSelect,
  amsestimateTM
);

Operator createlossycounterOp(
  "createlossycounter",
  createlossycounterSpec.getStr(),
  4,
  createlossycounterVM,
  createlossycounterSelect,
  createlossycounterTM
);


Operator lcfrequentOp(
  "lcfrequent",
  lcfrequentSpec.getStr(),
  4,
  lcfrequentVM,
  lcfrequentSelect,
  lcfrequentTM
);


Operator outlierOp(
  "outlier",
  outlierSpec.getStr(),
  2,
  outlierVM,
  outlierSelect,
  outlierTM
);

Operator streamclusterOp(
  "streamcluster",
  streamclusterSpec.getStr(),
  streamclusterVM,
  Operator::SimpleSelect,
  streamclusterTM
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
    AddTypeConstructor(&amsSketchTC);

    //Usage possibilities of the Types
    scalableBloomFilterTC.AssociateKind(Kind::SIMPLE());
    countMinSketchTC.AssociateKind(Kind::SIMPLE());
    amsSketchTC.AssociateKind(Kind::SIMPLE());


    //Registration of Operators
    AddOperator(&reservoirOp);
    AddOperator(&tiltedtimeOp);
    AddOperator(&createbloomfilterOp);
    AddOperator(&bloomcontainsOp);
    AddOperator(&createcountminOp);
    AddOperator(&cmscountOp);
    AddOperator(&createamsOp);
    AddOperator(&amsestimateOp);
    AddOperator(&createlossycounterOp);
    AddOperator(&lcfrequentOp);
    AddOperator(&outlierOp);
    AddOperator(&streamclusterOp);
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

