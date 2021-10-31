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

  * reservoir: stream x int x real [->] (stream)
    Creates a reservoir sample of size int for a stream

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
#include "Algebras/Spatial/Point.h"

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
#include <algorithm>
#include <cmath>
#include <random>
#include <cmath>
#include <chrono>

using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

//Make sure that we are using the correct Hashing algorithm
//depending on the Systemarchitecture. Only works for 
//32/64 Bit Systems. 
#if INTPTR_MAX == INT64_MAX
  #define murmur(a,b,c,d) MurmurHash3_x64_128(a, b, c, d)
#elif INTPTR_MAX == INT32_MAX
  #define murmur(a,b,c,d) MurmurHash3_x86_128(a, b, c, d)
// 32-bit
#else
#error Unknown pointer size or missing size macros!
#endif

namespace eschbach {

/*
2 Algebra Implementation

2.1 Data Structures

2.1.1 Class ~BloomFilter~

*/
//Effectively the way I construct this cannot be correct
//When the QP storage is assigned with e.g. " ScalableBloomFilter* bloomFilter 
//= (ScalableBloomFilter*) result.addr;" the call goes to this constructor first
//but then Create() function is called, and I have to use initialize() after. 
//There has to be a right way to do this, but I have not figured it out. 
//The way this is, either the assignments in initialize, or those made here
//are superflous, but deleting either results in Problems. 
//This is the case for every Object. 
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

//Implemented because of the open function
void 
ScalableBloomFilter::setSubFilter(vector<bool> inputSubFilter) {
  filterList.push_back(inputSubFilter);
}

vector<vector<bool>>
ScalableBloomFilter::getFilterList() {
  return filterList;
} 

//Implemented because of the open function
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

//Calculate the Number of bits needed for a given FP
//considering the Number of items we build our Filter 
//for. This happens according to 
size_t
ScalableBloomFilter::optimalSize(const long expectedInserts, 
                                const double fPProb) {
  //we use the ceiling of the calculated number of bits. At worst this
  //adds one bit to our subfilter size.
  size_t optimalSize = ceil(-expectedInserts*log(fPProb)/ pow(log(2),2));
  if (optimalSize < 1) {
    return 1; 
  }
  return optimalSize;
}

//Calculate the number of Hashfunctions needed to guarantee our 
//FP considering the Number of items and the calculated filter size
long
ScalableBloomFilter::optimalHashes(const long expectedInserts, 
                                   const long filterSize) {
  return (long) max(1, (int) round((long) filterSize/expectedInserts * log(2)));
}

//Akwardly constructed, should be refactored: 
//Receives a vector of the calculated Hashvalues of an element and the
//index of the subfilter these Hashvalues should be present in if
//the element is actually 
bool 
ScalableBloomFilter::contains(vector<size_t> hashResults, 
                              int filterIndex) const {
  bool present = true;    
  if (defined) {
    for (size_t index : hashResults) {
      //If even one bit is not set, the element can not be present
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
  //Use this Value to determine if adding an elements Hashvalues increases
  //the amount of items in the filter. We do this since the filter is 
  //constructed for a certain number of distinct items. 
  bool alreadyAdded = true;
    for (size_t eleIndex : hashResults) {
      //Check should be superflous, but the whole class should be refactured 
      //and time is too short to do this now.
      if ((eleIndex >= 0) && (eleIndex < filterSize)) {
        //This sets all bits corresponding to the hashes of an element 
        //if any were unset the bool becomes false and 
        //we added a new distinct element
        alreadyAdded &= setElement(curFilterIndex, eleIndex, true);
      }
    }
    //New distinct element added. Counter has to be increased
    if (!alreadyAdded) {
      currentInserts++;
    }
  }
}

//Checks if the current Filter has processed as many distinct 
//Elements as it was constructed for
bool
ScalableBloomFilter::isSaturated() {
  return currentInserts >= maxInserts; 
}

// Update the parameter vaöies and add a new Subfilter to our Scalable Bloom
void
ScalableBloomFilter::updateFilterValues() {
  curFilterIndex++;
  maxInserts *= GROWTH_RATE;
  //Inserts start by 0 again for the new subfilter
  currentInserts = 0; 
  rollingFP *= TIGHTENING_RATIO;
  filterSize = optimalSize(maxInserts,rollingFP);
  numHashfunctions = optimalHashes(maxInserts, filterSize);
  //resize the filter to accomodate the new subfilter
  filterList.resize(curFilterIndex+1);
  //resize the newly added subfilter, so that 
  //push_back is faster
  filterList.back().resize(filterSize);
  ithFilterHashes.push_back(numHashfunctions);
}

//~In~/~Out~ Functions

//As we discussed: In and Out Functions should only be dummies, since 
//Listrepresentations of the Sketches would not make any sense
Word
ScalableBloomFilter::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  
  Word result = SetWord(Address(0));
  cmsg.inFunError("The Bloomfilter Datastructure "
                  "does not Support In-Functions");
  return result;
}

ListExpr
ScalableBloomFilter::Out(ListExpr typeInfo, Word value) {
  ListExpr returnList 
           = nl -> TwoElemList(
                    nl -> StringAtom("Bloomfilter operation successfull, but "),
                    nl -> StringAtom("console output is not supported"));

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
  size_t maxInserts = 4096;
  size_t subFilterSize;
  int nbrSubFilters;
  int nbrHashFunctions;
  vector<int> hashFunctionsPerFilter;
  vector<bool> insertionVector;
  bool filterElement;

  
  bool ok = valueRecord.Read (&fp, sizeof(double), offset);
  offset += sizeof(double);

  //By constructing a new Bloomfilter with the saved fp 
  //the new Filter will already have a subfilter with the 
  //correct size. 
  ScalableBloomFilter* openBloom = new ScalableBloomFilter(fp);

  ok = ok && valueRecord.Read (&nbrSubFilters, sizeof(int), offset);
  offset += sizeof(int);

  //reserve the amount of Subfilters we know we will have
  //in the FilterList
  openBloom->getFilterList().reserve(nbrSubFilters);

  //We will have as many "layers" of hashfunctions as filters
  hashFunctionsPerFilter.reserve(nbrSubFilters);

  //See how many hashfunctions each filter uses
  for (int i = 0; i < (nbrSubFilters); i++) {
    ok = ok && valueRecord.Read(&nbrHashFunctions, sizeof(int), offset);
    hashFunctionsPerFilter.push_back(nbrHashFunctions);
    offset += sizeof(int);
  }

  //clear out the number of Hashes that were determined in the 
  //constructor
  openBloom -> getFilterHashes().clear();
  openBloom -> getFilterHashes().reserve(hashFunctionsPerFilter.size());
  openBloom -> setFilterHashes(hashFunctionsPerFilter);
   
  //Read the values of the first saved subfilter
  subFilterSize=openBloom->optimalSize(maxInserts, fp);
  for (size_t j = 0; j < subFilterSize; j++) {
    ok = ok && valueRecord.Read (&filterElement, sizeof(bool), offset);
    offset += sizeof(bool);
    openBloom->setElement(0,j, filterElement);
  }

  //Update the parameters after inserting the first subfilter
  fp *= 0.8;
  maxInserts*=2;

  //Read the values of the following subfilters
  for (int i = 1;  i < nbrSubFilters; i++) {
    subFilterSize=openBloom->optimalSize(maxInserts, fp);
    insertionVector.reserve(subFilterSize);
    
    for (size_t j = 0; j < subFilterSize; j++) {
      ok = ok && valueRecord.Read (&filterElement, sizeof(bool), offset);
      offset += sizeof(bool);
      insertionVector.push_back(filterElement);
    }
    
    openBloom -> setSubFilter(insertionVector);
    insertionVector.clear();
    fp *= 0.8;
    maxInserts*=2;

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

  bool ok = valueRecord.Write(&fp, sizeof(double), offset);
  offset+=sizeof(double);

  //The number of Filters is equivalent to the different number of 
  // Hashfunctions we save. Hence we only need to save one of these 
  // updateFilterValues
  ok = ok && valueRecord.Write(&nbrSubFilters, sizeof(int), offset);
  offset+=sizeof(int);
  

  //Save the amount of Hashfunctions each Subfilter uses
  for (int nbr : hashfunctionsPerFilter) {
    ok = ok && valueRecord.Write(&nbr, sizeof(int), offset);
    offset+=sizeof(int);
  }

  //Save the subfilter Values. This method of saving is highly 
  //ineffective. Even for just 4096 we are reading >39000 bool
  //values. This should be changed asap.
  for (vector<bool> subFilter : bloomFilter->getFilterList()) {
    for (bool elem : subFilter) {
      ok = ok && valueRecord.Write(&elem, sizeof(bool), offset);
      offset+=sizeof(bool);
    }
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

//Will not return the correct size (with or without) *this
//See below.
int
ScalableBloomFilter::SizeOfObj()
{
  return sizeof(ScalableBloomFilter);
}

//These were attempts to get an exact measure of a Bloomfilters 
//size before our discussion to use /usr/bin/time -f "%M" . 
//Since it is a dynamic structure none will work anyhow. 
size_t 
ScalableBloomFilter::Sizeof() const {
  size_t filterSizeByte;
  size_t bytes;
  for (int i = 0; i < (int) filterList.capacity(); i++) {
    filterSizeByte = (filterList[i].capacity()+7)/8;
    bytes += filterSizeByte; 
  }
    /*struct rusage r_usage;
  getrusage(RUSAGE_SELF, &r_usage);
  printf("%ld\n", r_usage.ru_maxrss);
  */
  printf("%ld\n", bytes);
  return 0; 
}

//Type Description
struct scalableBloomFilterInfo : ConstructorInfo {

  scalableBloomFilterInfo() {

    name         = ScalableBloomFilter::BasicType();
    signature    = "-> " + Kind::SIMPLE();
    typeExample  = ScalableBloomFilter::BasicType();
    listRep      =  "Listrepresentation does not exist";
    valueExample = "(0 1 1 0)";
    remarks      = "In- and Out-Functions are dummies";
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
    sizeOf= ScalableBloomFilter::SizeOfObj;
  }
};

scalableBloomFilterInfo bi;
scalableBloomFilterFunctions bf;
TypeConstructor scalableBloomFilterTC( bi, bf );



/*
2.1.2 Class ~CountMinSketch~

*/
//Comment Re: constructor made in Bloomfilter 
//also applies here
CountMinSketch::CountMinSketch
(const double epsilon, const double delta) {
  defined = true;
  eps = epsilon;
  this->delta = delta;
  width = ceil(exp(1)/eps);
  depth = ceil(log(1/delta));
  //This matrix could be made into an array
  //should be better re: memory consumption
  //Since we are using a vector, we resize everything
  //at the start, so that no memory reallocations will 
  //have to happen later on. This should negate
  //the disadvantages of the vector.
  matrix.resize(depth);
  for (size_t i = 0; i < depth; i++)
    matrix[i].resize(width);
  totalCount = 0;

  //We need as many hashfunctions as we have rows. 
  hashConstants.resize(depth);

  for (size_t i = 0; i < depth; i++) {
    hashConstants[i].resize(2);
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

void
CountMinSketch::setTotalCount(size_t count){
  totalCount = count;
}

size_t
CountMinSketch::getWidth() {
  return width;
}

size_t
CountMinSketch::getDepth() {
  return depth;
}

double
CountMinSketch::getEpsilon() {
  return eps;
}

double
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
CountMinSketch::initialize(double eps, double delt) {
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
  // Only called here, since initialize will
  // always be called after the vector. 
  srand(time(NULL)+getpid());

  hashConstants.resize(depth);

  for (size_t i = 0; i < depth; i++) {
    hashConstants[i].resize(2);
    generateConstants(i);
  }
}

// We use the fact that pairwise independent Hashfunctions are easy
// to generate with h(x) = ax + b % p, with p being a big prime, and a
// b being constants. In this function we generate the constants.
void
CountMinSketch::generateConstants(int index) {
  long a = (rand() % LONG_PRIME)-1;
  long b = (rand() % LONG_PRIME)-1;
  setConstants(index, a, b);
}

void 
CountMinSketch::increaseCount(long hashedEleValue) {

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

  return minVal;
}

//~In~/~Out~ Functions

//Not needed, as discussed
Word
CountMinSketch::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  
  Word result = SetWord(Address(0));
  cmsg.inFunError("The CountMinSketch Datastructure "
                  "does not Support In-Functions");
  return result;
}

//Out-Function (Dummy)
ListExpr
CountMinSketch::Out(ListExpr typeInfo, Word value) {
  ListExpr returnList 
           = nl -> TwoElemList(
                    nl -> StringAtom("CMS operation successfull, "),
                    nl -> StringAtom("but console output is not supported"));

  return returnList;
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
  double epsilon;
  double delta; 
  size_t width;
  size_t depth;
  size_t totalEleCount;
  long constantA;
  long constantB;
  int counterEle;

  bool ok = valueRecord.Read (&epsilon, sizeof(double), offset);
  offset += sizeof(double);

  ok = valueRecord.Read (&delta, sizeof(double), offset);
  offset += sizeof(double);

  ok = ok && valueRecord.Read (&width, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ok = ok && valueRecord.Read (&depth, sizeof(size_t), offset);
  offset += sizeof(size_t);

  ok = ok && valueRecord.Read (&totalEleCount, sizeof(size_t), offset);
  offset += sizeof(size_t);

  CountMinSketch* openCMS = new CountMinSketch(epsilon, delta);

  //total Count is required for our estimate. 
  openCMS -> setTotalCount(totalEleCount);

  //Constants are read in and stored pairwise(1 pair on each "level")
  //of our saving vector so the allocation to the proper matrix 
  //row is easy
  for (size_t i = 0; i < depth; i++) {
    ok = ok && valueRecord.Read (&constantA, sizeof(long), offset);
    offset+=sizeof(long); 
    ok = ok && valueRecord.Read (&constantB, sizeof(long), offset);
    offset+=sizeof(long); 
    openCMS->setConstants(i, constantA, constantB);
  }

  //Reading the counter values is the same as with the boolfilter
  //Needs an urgent rework; because otherwise saving the synopsis
  //is really impractical. 
  for (size_t i = 0; i < depth; i++) {
    for (size_t j = 0; j < width; j++) {
        ok = ok && valueRecord.Read (&counterEle, sizeof(int), offset);
        offset+=sizeof(int); 
        openCMS -> setElement(i,j,counterEle);
    }
  }

  if (ok) {
    value.addr = openCMS;
  } else {
    value.addr =  0;
  }
  return true;
} 

//Saving is exactly identical to Bloom
bool 
CountMinSketch::Save(SmiRecord & valueRecord , size_t & offset ,
const ListExpr typeInfo , Word & value) {
  CountMinSketch* cms = static_cast<CountMinSketch*>
                                    (value.addr);

  double epsilon = cms->getEpsilon();
  double delta = cms -> getDelta();
  size_t width = cms -> getWidth();
  size_t depth = cms -> getDepth();
  size_t totalEleCount = cms -> getTotalCount();   
  long hashConstantA;
  long hashConstantB;
  int counterEle;
                         
  bool ok = valueRecord.Write(&epsilon, sizeof(double), offset);
  offset+=sizeof(double);

  ok = ok && valueRecord.Write(&delta, sizeof(double), offset);
  offset+=sizeof(double);
  
  ok = ok && valueRecord.Write(&width, sizeof(size_t), offset);
  offset+=sizeof(size_t);

  ok = ok && valueRecord.Write(&depth, sizeof(size_t), offset);
  offset+=sizeof(size_t);

  ok = ok && valueRecord.Write(&totalEleCount, sizeof(size_t), offset);
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
    listRep      =  "Listrepresentation does not exist";
    valueExample = "(4 12 2 8)";
    remarks      = "In- and Out-Functions are dummies";
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
//Previous comments re: Constructor also apply here. 
//Constructor is identical to CMS constructor except
//for the generation of fourwise independent hashing
//constants.
amsSketch::amsSketch
(const double epsilon, const double delta) {
  defined = true;
  eps = epsilon;
  this->delta = delta;
  //The width of the matrix has to be initialized differently
  //in comparison with the CMS
  width = ceil(1/pow(eps,2));
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

double
amsSketch::getEpsilon() {
  return eps;
}

double
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
amsSketch::initialize(double eps, double delt) {
  defined = true;
  this->eps = eps; 
  this->delta = delt; 
  width = ceil(1/pow(eps,2));
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
  long a = (rand() % LONG_PRIME)-1;
  long b = (rand() % LONG_PRIME)-1;
  setConstantsTw(index, a, b);
}

// In contrast to Count-Min we also need a four-wise independent hash
//  Function. These are given by h(x) = ax^3 + bx^2 + cx + d % p.
void 
amsSketch::generateFwConstants(int index) {
  long a = (rand() % LONG_PRIME)-1;
  long b = (rand() % LONG_PRIME)-1;
  long c = (rand() % LONG_PRIME)-1;
  long d = (rand() % LONG_PRIME)-1;
  setConstantsFw(index, a, b, c, d);
}

// Auxiliary function in the determination of the median of 
// squared sums of row values. Swaps the two elements 
//so that we receive a furhter sorted array
void 
amsSketch::swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

//Sort the array according to the pivot
int 
amsSketch::partition(int arr[], int lIndex, int rIndex)
{
    int lst = arr[rIndex];
    int i = lIndex;
    int j = lIndex;
    while (j < rIndex) {
        if (arr[j] < lst) {
            swap(&arr[i], &arr[j]);
            i++;
        }
        j++;
    }
    swap(&arr[i], &arr[rIndex]);
    return i;
}

int 
amsSketch::randomPartition(int arr[], int lIndex, int rIndex)
{
    int n = rIndex - lIndex + 1;
    int pivot = rand() % n;
    swap(&arr[lIndex + pivot], &arr[rIndex]);
    return partition(arr, lIndex, rIndex);
}

//Implementation of quickselect to sort the array and find the median
void
amsSketch::medianDecider(int arr[], int lIndex, int rIndex, 
                         int mIndex, int& a, int& b) {
  // if lIndex < mIndex
    if (lIndex <= rIndex) {
        // Find the pivotElement index
        int partitionIndex = randomPartition(arr, lIndex, rIndex);
 
        // If partion index = mIndex, then
        // we found the median of odd
        // number element in medianArray[]
        if (partitionIndex == mIndex) {
            b = arr[partitionIndex];
            if (a != -1)
                return;
        }
 
        // If index = mIndex - 1, then we get
        // a & b as middle element of
        // medianArray[]
        else if (partitionIndex == mIndex - 1) {
            a = arr[partitionIndex];
            if (b != -1)
                return;
        }
 
        // If partitionIndex >= mIndex then
        // find the index in first half
        // of the medianArray[]
        if (partitionIndex >= mIndex)
            return medianDecider(arr, lIndex, partitionIndex - 1,
                              mIndex, a, b);
 
        // If partitionIndex <= k then
        // find the index in second half
        // of the medianArray[]
        else
          return medianDecider(arr, partitionIndex + 1,
                              rIndex, mIndex, a, b);
    }
    return;
}

//function used to find the median of the sum of squares array we build
int
amsSketch::findMedian(int medianArray[], int arraySize) {
  int a = -1;
  int b = -1;
  int median;

  //Median position differs between arrays with an uneven vs even
  //amount of elements.
  if (arraySize % 2 == 1) {
    medianDecider(medianArray, 0, arraySize - 1, arraySize / 2, a, b);
    median = b;
  } else {
    medianDecider(medianArray, 0, arraySize - 1, arraySize / 2, a, b);
    median = (a + b) / 2;
  }
  return median;
}

//Function used to compute the effect of an element arriving
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
    updateValue = 2*((long)((fwa*pow(value,3)) + (int)(fwb*pow(value,2)) 
                  + fwc*value + fwd) % LONG_PRIME % 2)-1;

    //Commit the change
    updateElement(i, hashIndex, updateValue); 
  }
}

//Function to calculate the self-join size of the monitored stream
int 
amsSketch::estimateInnerProduct() {
  int medianArray[depth]; 
  int joinSize;
  int sum = 0;
  int arraySize;

  //Calculate the sum of the squares of all row Elements for each row
  for (size_t i = 0; i < depth; i++) {
    for (size_t j = 0; j < width; j++) {
      sum += pow(getElement(i, j), 2);
    }
    medianArray[i] = sum;
    sum = 0;
  }

  //Use pointer arithmetic to find the arraysize
  //We are substracting the adress of the start of the array
  //from the end address to get the length
  arraySize = *(&medianArray + 1) - medianArray;
  joinSize = findMedian(medianArray,arraySize);

  //Return the Median of sum of squares of elements of the rows
  return joinSize;
}


//~In~/~Out~ Functions

//In- and Out- Functions are as discussed.
Word
amsSketch::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word result = SetWord(Address(0));
  cmsg.inFunError("The AMS-Sketch Datastructure "
                  "does not Support In-Functions");
  return result;
}

ListExpr
amsSketch::Out(ListExpr typeInfo, Word value) {
  ListExpr returnList 
           = nl -> TwoElemList(
                    nl -> StringAtom("AMS Sketch operation succesfull,"),
                    nl -> StringAtom("but console output is not supported"));

  return returnList;
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
  double epsilon;
  double delta; 
  size_t width;
  size_t depth;
  long constantTwA, constantTwB, constantFwA, 
      constantFwB, constantFwC, constantFwD;
  int counterEle;

  bool ok = valueRecord.Read (&epsilon, sizeof(double), offset);
  offset += sizeof(double);

  ok = valueRecord.Read (&delta, sizeof(double), offset);
  offset += sizeof(double);

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
  }

  for (size_t i = 0; i < depth; i++) {
    for (size_t j = 0; j < width; j++) {
        ok = ok && valueRecord.Read (&counterEle, sizeof(int), offset);
        offset+=sizeof(int); 
        openAMS -> updateElement(i,j,counterEle);
    }
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

  double epsilon = ams->getEpsilon();
  double delta = ams -> getDelta();
  size_t width = ams -> getWidth();
  size_t depth = ams -> getDepth();
  long hashConstantTwA, hashConstantTwB, hashConstantFwA, hashConstantFwB, 
      hashConstantFwC, hashConstantFwD; 
      
  int counterEle;

  bool ok = valueRecord.Write(&epsilon, sizeof(double), offset);
  offset+=sizeof(double);

  ok = ok && valueRecord.Write(&delta, sizeof(double), offset);
  offset+=sizeof(double);
  
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
    }
  }
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
    listRep      =  "Listrepresentation does not exist";
    valueExample = "(4 12 2 8)";
    remarks      = "In- and Out-Functions are dummies";
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
(const double eps) {
  defined = true; 
  epsilon = eps; 
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

template<class T> double 
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
lossyCounter<T>::initialize(const double eps) {
  defined = true; 
  epsilon = eps; 
  eleCounter = 0; 
  windowSize = ceil(1/epsilon);
  windowIndex = 1;
  frequencyList.insert({0, counterPair(0,0,0)});
}

///Handles incoming Streamelements
template<class T> void 
lossyCounter<T>::addElement(T element) {
  if (elementPresent(element)) {
    //The element is already present and we 
    //only need to increase its counter
    incrCount(element);
  } else {
    //We create a new counter in the map for an element
    //since it was not present
    insertElement(element);
  }
  //After handling the element our current bucket/window 
  //is full and we have to remove all counters 
  //which no longer fulfill our threshold. 
  if (eleCounter % windowSize == 0) {
    //remove the elements
    reduce();
    //increase the current bucket/window index
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

//Insert previously unencountered Element into our element list
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
  windowIndex++;
}

//Removes the Items below the Frequency Threshold. The usage of the 
//Second vector to save the elements which we mark for deletion is
//probably easily improveable (mb. by just saving the keys)
template<class T> void 
lossyCounter<T>::reduce() {
  vector<T> deletionList;
  for (auto elements : frequencyList) {
    counterPair elem = elements.second;
    //item does no longer pass the threshold
    if ((elem.getFrequency() + elem.getMaxError()) < windowIndex) {
      //mark the element for deletion
      deletionList.push_back(elements.first);
    }
  }
  //remove all elements which were marked for deletion
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

//Get the frequency of a single Element. This has no Operator
//part, but would be very easy to include
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
//This class is only auxiliary for the lossy counter
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
//This Implementation is fully working for int and real
//values. To create 
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

vector<cPoint> 
Cluster::getAllPoints() {
  return points;
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
        //A Cluster assignment changed and we have to continue
        //Checking whether this has any cascading effects.
        //The loop will run again, in case the maximum number 
        //of iterations has not yet been reached
        done = false;
      }
    }

    //Clear current Cluster allocation of points in order to 
    //redistribute Points after finding closer Centroids
    clearClusters();

    //Reassign the Points to their new Clusters
    for(int i = 0; i < totalNbrPoints; i++) {
      //cluster index in cluster is Id-1,  due to Cluster id starting
      //at 1
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

    //Break the loop if no changes to point/cluster assignments
    //occured OR the maximum number of iterations according to 
    //operator usage is reached
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
  if (type.length() != 3){
    return NList::typeError("Operator reservoir expects three arguments");
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

  //test third argument for int
  if(type.third() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator reservoir expects an real "
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
  if (type.length() != 2){
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

ListExpr stream = nl->First(args);

ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  // three arguments must be supplied
  if (type.length() != 3){
    return NList::typeError("Operator createbloomfilter expects "
                            "three arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(stream)){
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
  ListExpr attrType;
  string attrName = type.second().str();

  //Already modifying the attrIndex to point to the correct
  //index here
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  //Attribute name is not present in the Tuple
  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }

  /* result is a bloomfilter and we append the index of 
     the attribute in the tuples, so that we can hash 
     the correct values
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
    return NList(CcString::BasicType()).listExpr(); 
  }    

  return NList::typeError("Operator bloomcontains expects an argument of " 
                          " type DATA as second argument");
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
                            "value as third argument");
  }

  //test fourth argument for int
  if(type.fourth() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator createcountmin expects a real "
                            "value as fourth argument");
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

  //Already modifying the attrIndex to point to the correct
  //index here
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  //Attribute Name is not present in the Tuples of the Stream
  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }

  /* result is a Count Min Sketch and we append the index of the 
     attribute of the tuples which will be hashed to create our sketch 
  */
  appendList.append(NList().intAtom(attrIndex));
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

  // test second argument for DATA 
  if(type.second().isAtom()) {
    return NList(Symbols::APPEND(), appendList,
               CcInt::BasicType()).listExpr();
  } 

  return NList::typeError("Operator cmscount expects an argument" 
                          " of Type DATA as second argument");
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
                            "value as third argument");
  }

  //test fourth argument for int
  if(type.fourth() != NList(CcReal::BasicType())) {
    return NList::typeError("Operator fourth expects a real "
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
  //Already modifying the attrIndex to point to the correct
  //index here
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;
  //Attribute Name is not present in the Tuples of the Stream
  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }

  /* result is a AMS Sketch and we append the index of 
     the attribute of the tuples which will be hashed to create our filter 
  */
  appendList.append(NList().intAtom(attrIndex));
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
    return NList::typeError("Operator amsestimate expects one argument");
  }

  // test first argument for scalablebloomfilter
  if(type.first() != NList(amsSketch::BasicType())){
    return NList::typeError("Operator amsestimate expects an "
                            "AMS Sketch as argument");
  }

  //Only the selfjoin Size will be returned, this is a simple int
  return NList(CcInt::BasicType()).listExpr(); 
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
                            "three arguments");
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
  

  /* Result is a Tuple Stream consisting of (Item, Frequency, Delta,
     Epsilon,EleCount) and the appended attribute Index and Type.
     Epsilon and Elementcount are appended, because we need the information
     in the ~lcfrequent~ Operator. 
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

  //Every Stream generated by the ~createlossycounter~ Operator will consist of 
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

  if(!( value == "Value") && (frq == "Frequency") && (dlt == "Delta") &&
      (err == "Epsilon") && (cnt == "EleCount")){
    return NList::typeError( "Operator lcfrequent expects a Stream generated "
                             "from ~createlossycounter~ as first argument");
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
  
  //Generate the type of the Stream we will create
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

//Typelist for use with ~lossycompare~. 
//outlist = 
//    nl->TwoElemList(
//     nl->SymbolAtom(Symbol::STREAM()),
//      nl->TwoElemList(
//          nl->SymbolAtom(Tuple::BasicType()),
//          nl->TwoElemList(
//            nl->TwoElemList(
//              nl->SymbolAtom("Value"),
//              nl->SymbolAtom(nl->SymbolValue(attrType))),       
//            nl->TwoElemList(
//              nl->SymbolAtom("Frequency"),
//              nl->SymbolAtom(CcInt::BasicType()))         
//          )
//        )     
//      );
//  Result is a  Tuple Stream consisting of (Item, Frequency, Delta) 
//  and we appended attribute Index and Type    

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

  // test third argument for int
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

  /* This was the Typemap created when trying to simply append a 
     new clusterID Attribute to the existing Tuple. The detection
     of the Index and creation of the new tupletype in the list 
     work fine and might be helpful at some point

  Our intention is to add every Tuples Cluster ID - in relation
  to the Attribute we are clustering for - as Attribute to the Tuple
  Therefore, we append a new Attribute to the existing Tuple
  Attribute List.
  
  type.first().second().second().append(
                                nl->TwoElemList(
                                  nl->SymbolAtom("ClusterID"), 
                                  nl->SymbolAtom(CcInt::BasicType())));

  Save the number of Tuple Attributes so we can add 
  the clusterID in the VM at the right index
  
  int newAttrIndex = type.first().second().second().length()-1;


  Result type is a stream of Tuples with the queried attribute value
     and its corresponding index in the stream. We also appended
     the attribute index, -type and amount of ATtributes for the Value
     Mapping
  
  appendList.append(NList().intAtom(newAttrIndex));

     */
  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().stringAtom(nl->ToString(attrType)));
  
  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
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
      )
    );
  
  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();  
  
  /* Return type if you want to append the 
  ID to as new attribute to an existing tuple
  
  return NList(Symbols::APPEND(), appendList,
               type.first()).listExpr();

  */
}


/*
2.13 Generator Operator

2.13.1 Operator ~pointgen~ 

*/
//Test generator to receive random point attributes
//Only basic checks implemented
ListExpr
pointgenTM(ListExpr args)  {
  if(!nl->HasLength(args,2)){
   return NList::typeError("Operator pointGen expects "
                           "two arguments");
  }
  
  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator pointGen expects an int "
                            "as first argument");
  }

  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator pointGen expects an int "
                            "as first argument");
  }

  return nl->TwoElemList(nl->SymbolAtom(Stream<Point>::BasicType()),
                         nl->SymbolAtom(Point::BasicType()));

}

/*
2.13.2 Operator ~stringgen~ 

*/
ListExpr
stringgenTM(ListExpr args)  {
  if(!nl->HasLength(args,2)){
   return NList::typeError("Operator stringgen expects "
                           "two arguments");
  }
  
  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator stringgen expects "
                           " an int argument as first argument");
  }

  if(!listutils::isSymbol(nl->Second(args), CcInt::BasicType())) {
    return NList::typeError("Operator stringgen expects "
                           " an int argument as seconds argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcString>>(),
                          listutils::basicSymbol<CcString>());
}

/*
2.13.3 Operator ~intgen~ 

*/
ListExpr
intgenTM(ListExpr args)  {
  if(!nl->HasLength(args,2)){
   return NList::typeError("Operator intgen expects "
                           "two argumente");
  }
  
  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator intgen expects "
                           " an int as first argument");
  }

  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator intgen expects "
                           " an int as second argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcInt>>(),
                          listutils::basicSymbol<CcInt>());
}

/*
2.13.4 Operator ~realgen~ 

*/
ListExpr
realgenTM(ListExpr args)  {
  if(!nl->HasLength(args,4)){
   return NList::typeError("Operator realgen expects "
                           "four arguments");
  }
  
  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator realgen expects "
                           " an int argument as first argument");
  }

  if(!listutils::isSymbol(nl->Second(args), CcReal::BasicType())) {
    return NList::typeError("Operator realgen expects "
                           " an real argument as second argument");
  }

  if(!listutils::isSymbol(nl->Second(args), CcReal::BasicType())) {
    return NList::typeError("Operator realgen expects "
                           " an real argument as third argument");
  }

  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator realgen expects "
                           " an int argument as fourth argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcReal>>(),
                          listutils::basicSymbol<CcReal>());
}


/*
2.13.5 Operator ~massquerybloom~ 

*/
ListExpr
massquerybloomTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();
  NList appendList;
  ListExpr outlist = nl->TheEmptyList();

  if(!nl->HasLength(args,3)){
   return NList::typeError("Operator massquerybloom expects "
                           "three arguments");
  }
  
  if(!Stream<Tuple>::checkType(stream.listExpr())){
    return NList::typeError( "Operator massquerybloom expects a "
                             "Tuple Stream as first argument");
  }

  if(type.second() != NList(ScalableBloomFilter::BasicType())){
    return NList::typeError("Operator massquerybloom expects a "
                            "Bloomfilter as second argument");
  }

  //test third argument for a valid Attribute Name
  if (!type.third().isSymbol()){
    return NList::typeError("Operator massquerybloom expects a valid "
                            "Attribute Name as third argument");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.third().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  } 

  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(nl->SymbolValue(attrType))
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Found"),
              nl->SymbolAtom(CcBool::BasicType())
            )
        )
      )
    );

  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().stringAtom(nl->ToString(attrType)));

  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();
}

/*
2.13.4 Operator ~inttuplegen~ 

*/
ListExpr
inttuplegenTM(ListExpr args)  {
  ListExpr outlist = nl -> TheEmptyList();

  if(!nl->HasLength(args,1)){
   return NList::typeError("Operator inttuplegen expects "
                           "only one argument");
  }
  
  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator inttuplegen expects "
                           " an int argument");
  }

  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->OneElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Elem"),
              nl->SymbolAtom(CcInt::BasicType()))
          )
      )
    );

  return NList(outlist).listExpr();
}

/*
2.13.6 Operator ~stringtuplegenVM~

*/
ListExpr
stringtuplegenTM(ListExpr args)  {
  ListExpr outlist = nl -> TheEmptyList();

  if(!nl->HasLength(args,2)){
   return NList::typeError("Operator stringtuplegen expects "
                           "two argument");
  }
  
  if(!listutils::isSymbol(nl->First(args), CcInt::BasicType())) {
    return NList::typeError("Operator stringtuplegen expects "
                           " an int as first argument");
  }

  if(!listutils::isSymbol(nl->Second(args), CcInt::BasicType())) {
    return NList::typeError("Operator stringtuplegen expects "
                           " an int as second argument");
  }

  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->OneElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Elem"),
              nl->SymbolAtom(CcString::BasicType()))
          )
      )
    );

  return NList(outlist).listExpr();
}

/*
2.13.7 Operator ~bloomfalsepositive~

*/
ListExpr
bloomfalsepositiveTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();

  if(!nl->HasLength(args,4)){
   return NList::typeError("Operator bloomfalsepositive expects "
                           "four arguments");
  }

  if(type.first() != NList(ScalableBloomFilter::BasicType())){
    return NList::typeError("Operator bloomfalsepositive expects a "
                            "Bloomfilter as first argument");
  }

  //test third argument for a valid Attribute Name
  if (type.second() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator bloomfalsepositive expects an "
                            "int value as second argument");
  }

  if (type.third() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator bloomfalsepositive expects an "
                            "int value as third argument");
  }

  if (type.fourth() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator bloomfalsepositive expects an "
                            "int value as fourth argument");
  }
  return NList(CcInt::BasicType()).listExpr(); 
}

/*
2.13.8 Operator ~geometricdistTM~

*/
ListExpr
geometricdistTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();

  if(!nl->HasLength(args,2)){
   return NList::typeError("Operator geometricdist expects "
                           "two arguments");
  }

  if(type.first() != NList(CcInt::BasicType())){
    return NList::typeError("Operator geometricdist expects an "
                            "int value as first argument");
  }

  if (type.second() !=  NList(CcReal::BasicType())){
    return NList::typeError("Operator geometricdist expects an "
                            "real value as second argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcInt>>(),
                          listutils::basicSymbol<CcInt>());
}

/*
2.13.9 Operator ~uniformdist~

*/
ListExpr
uniformdistTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();

  if(!nl->HasLength(args,3)){
   return NList::typeError("Operator uniformdist expects "
                           "three arguments");
  }

  if(type.first() != NList(CcInt::BasicType())){
    return NList::typeError("Operator uniformdist expects an "
                            "int value as first argument");
  }

  if (type.second() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator uniformdist expects an "
                            "int value as second argument");
  }

  if (type.third() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator uniformdist expects an "
                            "int value as third argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcInt>>(),
                          listutils::basicSymbol<CcInt>());
}

/*
2.13.10 Operator ~normaldist~

*/
ListExpr
normaldistTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();

  if(!nl->HasLength(args,3)){
   return NList::typeError("Operator normaldist expects "
                           "three arguments");
  }

  if(type.first() != NList(CcInt::BasicType())){
    return NList::typeError("Operator normaldist expects an "
                            "int value as first argument");
  }

  if (type.second() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator normaldist expects an "
                            "int value as second argument");
  }

    if (type.third() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator normaldist expects an "
                            "int value as third argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcInt>>(),
                          listutils::basicSymbol<CcInt>());
}

/*
2.13.11 Operator ~normaldistreal~

*/
ListExpr
normaldistrealTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();

  if(!nl->HasLength(args,3)){
   return NList::typeError("Operator normaldistreal expects "
                           "three arguments");
  }

  if(type.first() != NList(CcInt::BasicType())){
    return NList::typeError("Operator normaldistreal expects an "
                            "int value as first argument");
  }

  if (type.second() !=  NList(CcReal::BasicType())){
    return NList::typeError("Operator normaldistreal expects an "
                            "real value as second argument");
  }

    if (type.third() !=  NList(CcReal::BasicType())){
    return NList::typeError("Operator normaldistreal expects an "
                            "real value as third argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcReal>>(),
                          listutils::basicSymbol<CcReal>());
}

/*
2.13.12 Operator ~distinctcountTM~

*/
ListExpr
distinctcountTM(ListExpr args)  {
  NList type(args);
  NList streamtype = type.first().second();
  NList appendList;
  ListExpr outlist = nl->TheEmptyList();
  ListExpr streamType = nl->First(args);

  // three arguments must be supplied
  if (type.length() != 2){
    return NList::typeError("Operator distinctcount expects "
                            "two arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(streamType)){
    return NList::typeError( "Operator distinctcount expects a "
                             "Tuple Stream as first argument");
  }

  //test second argument for a valid Attribute Name
  if (!type.second().isSymbol()){
    return NList::typeError("Operator distinctcount expects a valid "
                            "Attribute Name as second argument");
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
  

  /* Result is a  Tuple Stream consisting of (Item, Frequency) and 
     we appended attribute Index and Type
  */
  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(nl->SymbolValue(attrType))
            ),       
            nl->TwoElemList(
              nl->SymbolAtom("Frequency"),
              nl->SymbolAtom(CcInt::BasicType())
            )
          )
        )
      );

  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();
}

/*
2.13.13 Operator ~cmsoverreport~

*/
ListExpr
cmsoverreportTM(ListExpr args)  {
  NList type(args);
  NList appendList;
  ListExpr outlist = nl->TheEmptyList();
  ListExpr streamType = nl->First(args);

  // three arguments must be supplied
  if (type.length() != 5){
    return NList::typeError("Operator cmsoverreport expects "
                            "five arguments");
  }

  // test first argument for being a tuple stream
  if(!Stream<Tuple>::checkType(streamType)){
    return NList::typeError( "Operator cmsoverreport expects a "
                             "Tuple Stream as first argument");
  }


  //test second argument for a valid Attribute Name
  if (type.second() != NList(CountMinSketch::BasicType())){
    return NList::typeError("Operator cmsoverreport expects a  "
                            "Count Min Sketch as second argument");
  }

  //test third argument for a valid Attribute Name
  if (!type.third().isSymbol()){
    return NList::typeError("Operator cmsoverreport expects a valid "
                            "Attribute Name as third argument");
  }

  if(type.fourth() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator cmsoverreport expects an "
                            "int value as fourth argument");
  }


  if(type.fifth() !=  NList(CcReal::BasicType())){
    return NList::typeError("Operator cmsoverreport expects a "
                            "real value as fifth argument");
  }




  //We ensure, that the Tuplestream is one created by ~distinctcount~
  //All of these have the form (Value Frequency)
  string value = 
  nl->ToString((type.first().second().second().first().first()).listExpr());
  string frq = 
  nl->ToString((type.first().second().second().second().first()).listExpr());

  if(!( value == "Value") && (frq == "Frequency")){
    return NList::typeError( "Operator cmsoverreport expects a Stream "
                             "generated from distinctcount as first argument");
  }

  //extract index of the attribute we intend to hash
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.third().str();
  int attrIndex = listutils::findAttribute(attrList.listExpr(), 
                                           attrName, attrType) - 1;

  if (attrIndex < 0) {
    return NList::typeError("Attribute " + attrName + " "
                            "not found in tuple");
  }
  appendList.append(NList().intAtom(attrIndex));
  appendList.append(NList().stringAtom(nl->ToString(attrType)));
  

  /* Result is a  Tuple Stream consisting of (Item, Frequency) and 
     we appended attribute Index and Type
  */
  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList
          (
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(nl->SymbolValue(attrType))
            ),      
            nl->TwoElemList(
              nl->SymbolAtom("Overcount"),
              nl->SymbolAtom(CcReal::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Error"),
              nl->SymbolAtom(CcInt::BasicType())
            )
          )
        )
      );

  return NList(Symbols::APPEND(), appendList,
               outlist).listExpr();
}

/*
2.13.14 Operator ~switchingdist~

*/
ListExpr
switchingdistTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();

  if(!nl->HasLength(args,4)){
   return NList::typeError("Operator switchingdist expects "
                           "four arguments");
  }

  if(type.first() != NList(CcInt::BasicType())){
    return NList::typeError("Operator switchingdist expects an "
                            "int value as first argument");
  }

  if (type.second() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator switchingdist expects an "
                            "int value as second argument");
  }

  if (type.third() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator switchingdist expects an "
                            "int value as third argument");
  }

  if (type.fourth() !=  NList(CcReal::BasicType())){
    return NList::typeError("Operator switchingdist expects an "
                            "real value as fourth argument");
  }

  return nl->TwoElemList( listutils::basicSymbol<Stream<CcInt>>(),
                          listutils::basicSymbol<CcInt>());
}

/*
2.13.15 Operator ~switchingdist~

*/
ListExpr
samplegenTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();
  ListExpr outlist = nl -> TheEmptyList();


  if(!nl->HasLength(args,2)){
   return NList::typeError("Operator samplegen expects "
                           "two arguments");
  }

  if(type.first() != NList(CcInt::BasicType())){
    return NList::typeError("Operator samplegen expects an "
                            "int value as first argument");
  }


  if(type.second() != NList(CcInt::BasicType())){
    return NList::typeError("Operator samplegen expects an "
                            "int value as second argument");
  }

  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Elem"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Timestamp"),
              nl->SymbolAtom(CcInt::BasicType())
            )
          )
      )
    );

  return NList(outlist).listExpr();
}
/*
2.13.16 Operator ~lossycompare~

*/
ListExpr
lossycompareTM(ListExpr args)  {
  NList type(args);
  NList stream = type.first();
  ListExpr outlist = nl->TheEmptyList();
  NList appendList;


  if(!nl->HasLength(args,5)){
   return NList::typeError("Operator lossycompare expects "
                           "five arguments");
  }

  if(!listutils::isTupleStream(nl->First(args)) ||
     !listutils::isTupleStream(nl->Second(args))){
    ErrorReporter::ReportError("Operator lossycompare expects two "
                              " tuple streams as first arguments");
    return nl->TypeError();
  }

  if (type.third() !=  NList(CcInt::BasicType())){
    return NList::typeError("Operator lossycompare expects an "
                            "int value as third argument");
  }

  if (type.fourth() !=  NList(CcReal::BasicType())){
    return NList::typeError("Operator lossycompare expects an "
                            "real value as fourth argument");
  }

  if (type.fifth() !=  NList(CcReal::BasicType())){
    return NList::typeError("Operator lossycompare expects an "
                            "real value as fifth argument");
  }


  string attrType =  nl->ToString(type.first().second().second().first().
                                  second().listExpr());
  appendList.append(NList().stringAtom(attrType));

  outlist = 
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(attrType)
            ),  
            nl->TwoElemList(
              nl->SymbolAtom("ErrorCode"),
              nl->SymbolAtom(CcInt::BasicType())
            )
          )
        )
      );

  return NList(Symbols::APPEND(), appendList, 
               outlist).listExpr();
}

/*
2.3 Value Mapping Functions

2.3.1 Operator ~reservoir~

Creates a biased reservoir Sample (stream) of the passed stream.

*/
//Templates are used to deal with the different Types of Streams 
//the operator handles
template<class T> 
class reservoirInfo{
  public: 
    reservoirInfo(Word inputStream, size_t inputSampleSize, double bias): 
                  stream(inputStream), sampleSize(inputSampleSize), bias(bias), 
                  lastOut(-1), generator{std::random_device{}()},
                  distr(std::uniform_real_distribution<> (0.0,1.0)){
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
  double bias;
  int lastOut;
  std::vector<T*> reservoir;
  std::mt19937 generator;
  std::uniform_real_distribution<> distr;

  void init() {
    T* data;
    //While the Argumentstream can still supply Data/Tuples 
    while ((data = stream.request()) != nullptr) {
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
    /*Once the reservoir is filled we Check 
    if an Element fullfill the bias function
    */
    if (bias > distr(generator)) {
      /*If it does we insert it and remove another item 
      */
      size_t rnd = generator() % sampleSize;
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
                   CcReal* bias = (CcReal*) args[2].addr;
                   if(reservoirSize->IsDefined()){
                      int size = reservoirSize->GetValue();
                      double biasF = bias ->GetValue();
                      if(size>0 && ((biasF > 0) && (biasF < 1))) {
                        local.addr = new reservoirInfo<T>(args[0], size, biasF);
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
//This Operator creates a progressive logarithmic TTF for 
//Tuples.
template<class T> 
class tiltedtimeInfo{
  public: 
    tiltedtimeInfo(Word inputStream, int inputWindowSize): 
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
    double lowerBound = log2(timeStamp)-frameSize;
    double upperBound = log2(timeStamp);
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
                   if(reservoirSize->IsDefined()){
                      int size = reservoirSize->GetValue();
                      if(size>0) {
                        local.addr = new tiltedtimeInfo<T>(args[0], size);
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
//This Operator creates a Bloom Filter regarding a certain 
//Attributes values in a Tuplestream
int createbloomfilterVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CcReal* fpProb = (CcReal*) args[2].addr;
  CcInt* attrIndexPointer = (CcInt*) args[3].addr;
  Attribute* streamElement;

  int attrIndex = attrIndexPointer->GetIntval();

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  ScalableBloomFilter* bloomFilter = (ScalableBloomFilter*) result.addr;

  //initialize the Filter with the values provided by the operator
  bloomFilter->initialize(fpProb->GetValue());

  //Get the stream provided by the operator
  Stream<Tuple> stream(args[0]);

  //open the stream 
  stream.open();
  
  //Pointers to stream elements will be saved here for use
  Tuple* streamTuple = (Tuple*) stream.request();

  //Get the size of the Filter so we can mod the hash 
  //results to map to an index in the filter
  size_t filterSize = bloomFilter->getCurFilterSize();

  //Get number of Hashfunctions so using the hash results
  //vector will be faster
  int nbrHashes = bloomFilter->getCurNumberHashes();
  vector<size_t> hashvalues;

  //Prepare buffer for the MurmurHash3 output storage
  uint64_t mHash[2]; 

  //while the stream can still provide elements:
  while ((streamTuple != 0)) {
    //If we have added the number of elements the Filter was made for 
    //update the parameters (fp, filtersize) and add a new subfilter
    //Also update the variables relevant for hashing.
    //
    if (bloomFilter->isSaturated()) {
      bloomFilter->updateFilterValues();
      nbrHashes = bloomFilter->getCurNumberHashes();
      filterSize = bloomFilter ->getCurFilterSize();
    }
    hashvalues.reserve(nbrHashes);

    streamElement = (Attribute*) streamTuple->GetAttribute(attrIndex);

    //We make use of the HashValue() function so that we can use Elements of
    //any type. Using MurmurHash straight on the Attribute does not deliver 
    //deterministic results. 
    size_t secondoHash = streamElement->HashValue();
    size_t* hashPointer = &secondoHash;
    
    /*murmur type used is depending on the systemarchitecture
      the way it is used is defined in the Macro at the start
     */    
    murmur(hashPointer, sizeof(secondoHash), 0, mHash);
    size_t h1 = mHash[0] % filterSize;
    hashvalues.push_back(h1);

    //more than 1 Hash is needed (probably always the case)
    if (nbrHashes > 1) {
      size_t h2 = mHash[1] % filterSize;
      hashvalues.push_back(h2);
    
      //hash the streamelement for the appropriate number of times
      for (int i = 2; i < nbrHashes; i++) {
          size_t h_i = (h1 + i * h2 + (i * i)) % filterSize;
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
  
  stream.close();

  result.setAddr(bloomFilter);

  return 0;
}

/*
2.3.4 Operator ~bloomcontains~

*/
//This Operator allows us to make membership queries regarding a search element
//and a bloomfilter
int bloomcontainsVM(Word* args, Word& result,
           int message, Word& local, Supplier s){

  bool included = false;

  //take the parameters values supplied with the operator
  ScalableBloomFilter* bloomFilter = (ScalableBloomFilter*) args[0].addr;
  Attribute* searchEle = (Attribute*) args[1].addr;
  
  size_t secondoHash = searchEle->HashValue();
  size_t* hashpointer = &secondoHash; 

  cout << "SecondoHash of the Searchele is in bloomcontains is : " 
       << secondoHash << endl;

  string resultString = "The Search Element is not present in the Filter";

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcString* res = (CcString*) result.addr;

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
    cout << "FilterSize of Filter: " << i << "determined by bloomcontains is: " 
         << filterSize << endl;

    //hash the Searchelement
    murmur(hashpointer, sizeof(secondoHash), 0, cmHash);
  
    size_t h1 = cmHash[0] % filterSize;
    hashValues.push_back(h1);

    if (hashIterations[i] > 1) {
      size_t h2 = cmHash[1] % filterSize;
      hashValues.push_back(h2);

        for (int j = 2; j < hashIterations[i]; j++) {
          size_t h_i = (h1 + j * h2 + (j * j)) % filterSize;
          hashValues.push_back(h_i);
        }
    }
    //Element is contained in one of the Subfilters
    if (bloomFilter->contains(hashValues, i)) {
      included = true; 
      break;
    }
  }

  if (!included) {
    res->Set(true, resultString);
  } else {
    resultString = "The Search Element is present in the Filter";
    res->Set(true, resultString);
  }

  return 0;
}

/*
2.3.5 Operator ~createcountmin~

*/
//This Operator creates a Count-Min-Sketch regarding a certain 
//Attributes values in a Tuplestream
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

  //Get the stream provided by the operator
  Stream<Tuple> stream(args[0]);

  //open the stream 
  stream.open();
  
  //Pointers to stream elements will be saved here for use
  Tuple* streamTuple = (Tuple*) stream.request();

  Attribute* streamElement;

  while ((streamTuple != 0)) {
    streamElement = (Attribute*) streamTuple->GetAttribute(attrIndex);
    size_t secondoHash = streamElement->HashValue();
    cms->increaseCount(secondoHash);
    streamTuple->DeleteIfAllowed();
    streamTuple = stream.request();   
  }

  stream.close();

  result.setAddr(cms);

  return 0;
}

/*
2.3.6 Operator ~cmscount~

*/
//This Operator allows us to get the estimated frequency of an
//Element in a Stream for which we created a CMS
int cmscountVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CountMinSketch* cms = (CountMinSketch*) args[0].addr;
  Attribute* searchEle = (Attribute*) args[1].addr;
  size_t secondoHash = searchEle->HashValue();

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcInt* res = (CcInt*) result.addr;
  
  //prepare the result
  int estimate = 0;

  estimate = cms->estimateFrequency(secondoHash);   
  res->Set(true, estimate);

  return 0;
}

/*
2.3.7 Operator ~createams~

*/
//This Operator creates an AMS Sketch to track the F_2
//Moment of a Stream given an Attribute
int createamsVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  CcReal* epsilon = (CcReal*) args[2].addr;
  CcReal* delta = (CcReal*) args[3].addr;
  CcInt* attrIndexPointer = (CcInt*) args[4].addr;

  int attrIndex = attrIndexPointer->GetIntval();

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

  while ((streamTuple != 0)) {
    streamElement = (Attribute*) streamTuple->GetAttribute(attrIndex);
    size_t secondoHash = streamElement->HashValue();
    ams->changeWeight(secondoHash);
    streamTuple->DeleteIfAllowed();
    streamTuple = stream.request();   
  }

  stream.close();

  result.setAddr(ams);

  return 0;
}

/*
2.3.8 Operator ~amsestimate~

*/
//This Operator allows us to estimate the self join Size 
//of a Stream for which we created an AMS
int amsestimateVM(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  //take the parameters values supplied with the operator
  amsSketch* ams = (amsSketch*) args[0].addr;

  //Get the Resultstorage provided by the Query Processor
  result = qp -> ResultStorage(s);

  //Make the Storage provided by QP easily usable
  CcInt* res = (CcInt*) result.addr;
  
  //prepare the result
  int estimate = ams -> estimateInnerProduct();

  res->Set(true, estimate);

  return 0;
}

/*
2.3.9 Operator ~createlossycounter~

*/
//This Operator allows us to create a Lossy Counter 
//for a certain Attribute in a Tuplestream
template<class T, class S> 
class createlossycounterInfo{
    public: 
    createlossycounterInfo(Word inputStream, double epsilon, int index, 
                           string type):
    stream(inputStream), error(epsilon), attrIndex(index), attrType(type), 
    lastOut(-1), counter(epsilon){
      stream.open();
      
      //Create the TupleType so we can output the values we 
      //want with our tuple stream
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

      //Registering the Tupletype and making it useable for 
      //new tuples
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
      double error;
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
          T* tupleAttr = (T*) oldTuple->GetAttribute(attrIndex);
          S value = tupleAttr ->GetValue();
          //the counter structure will handle the data
          counter.addElement(value);
          oldTuple -> DeleteIfAllowed();
      }
      output();
    }

    void output() {
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
                      double error = epsilon->GetValue();
                      int index = attrIndex -> GetValue();
                      string type = attrType ->GetValue();
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
              createlossycounterVMT<CcReal,double>,
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
  
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    return 1;
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    return 3;
  } else {
    return -1;
  }
}


/*
2.3.10 Operator ~lcfrequent~

*/
//This Operator allows us to retrieve the frequent Items
//surpassing the minsupport threshold from a lossy counter
template<class T>
class lcFrequentInfo{
  public: 
    lcFrequentInfo(Word inputStream, double minSup, string type): 
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

       //Typelist for use with the ~lossycompare~ operator
       /*
       typeList = nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(attrType)
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Frequency"),
              nl->SymbolAtom(CcInt::BasicType())
            )          
          )
        ); 
      */
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
      double minSupport;
      string attrType; 
      int lastOut;
      double epsilon; 
      size_t nbrElements;
      vector<Tuple*> aboveMinSup;
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;

    void init() {
      //We get the first tuple while not in the loop so that 
      //we have access to the epsilon and nbrOfElements value
      //which do not change for any of the tuples
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

    //Determines whether an item is frequent when considering 
    //the parameters
    bool isFrequent(int frequency) {
      return ((minSupport-epsilon)*nbrElements) <= frequency;
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
                    double minSup = minSupport->GetValue();
                    string type = attrType->GetValue();
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
              lcfrequentVMT<CcBool>,
};  

// Selection Function
int lcfrequentSelect(ListExpr args){
  NList type(args);
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = "Value";
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);
  
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    return 1;
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    return 3;
  } else {
    return -1;
  }
}

/*
2.3.11 Operator ~outlier~

*/
//This Operator tries to Implement a form of anomaly detection
//for certain Attribute in a Tuplestream
template<class T, class S>
class outlierInfo{
  public:
    outlierInfo(Word inputStream, int zScoreInput, int index, 
    string type): 
      stream(inputStream), zThreshhold(zScoreInput), attrIndex(index), 
      attrType(type), oldMean(0), newMean(0), oldSsq(0),  newSsq(0), 
      variance(0), stdDev(0), lastOut(-1), counter(0){
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
        }
        tupleType -> DeleteIfAllowed();
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
        double oldMean;
        double newMean;
        double oldSsq;
        double newSsq;
        double variance;
        double stdDev;
        int lastOut;
        size_t counter;
        vector<Tuple*> outlierHistory;
      
        TupleType* tupleType;
        ListExpr typeList;
        ListExpr numTypeList;


      void init() {
        Tuple* oldTuple = stream.request();
        Tuple* newTuple = new Tuple(tupleType);
        //We will consider the first Tuple Value to be an outlier since we 
        //have no way of knowing what the stream will actually look like
        //We use the second template type to be able to handle the values
        oldMean = ((T*) oldTuple->GetAttribute(attrIndex))->GetValue();
        counter++;
        oldTuple->DeleteIfAllowed();
        while ((oldTuple = stream.request()) != nullptr) {
          S value = ((T*) oldTuple->GetAttribute(attrIndex))->GetValue();
          updateData(value);
          if (checkData(value)) {
            newTuple = new Tuple(tupleType);
            CcInt* index = new CcInt(true, counter);
            newTuple -> PutAttribute(0, oldTuple->
                        GetAttribute(attrIndex)->Copy());
            newTuple -> PutAttribute(1, index);
            oldTuple -> DeleteIfAllowed();
            outlierHistory.push_back(newTuple);
          } else {
            oldTuple -> DeleteIfAllowed();
          }
        }
      }

      /*Check whether the currently handled Streamelements
      z-Score surpases our treshholds and save it if it does
      */
      bool checkData(S data) {
        int zscore;
        //Variance can only be zero if the stream consists
        //of a single element
        if ((counter <= 2) || (variance==0)) {
          zscore = 0;
          return false;
        }
        zscore = (data - newMean)/stdDev;
        if((zscore < (-1*zThreshhold)) || (zscore > zThreshhold)) {
          return true; 
        }
        return false;
      }

      //Update mean, variance, counter and stdDeviation
      void updateData(S data) {
        counter++;
        newMean = oldMean + ((data - oldMean)/counter);
        newSsq = oldSsq + (data - oldMean)*(data - newMean);
        oldMean = newMean;
        oldSsq = newSsq;
        variance = newSsq/(counter-1);
        stdDev = sqrt(variance);
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
                      if(zThreshold>=1) {
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
              outlierVMT<CcInt,int>,
              outlierVMT<CcReal,double>
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
//This Operator uses k-mean clustering to return the 
//centroids and their size for a certain Attribute of a
//Tuplestream
class streamclusterInfo{
  public:
    streamclusterInfo(Word inputStream, size_t _maxMem, int index, string type,
                      int nbrClusters, int iter):
      stream(inputStream), maxMem(_maxMem), attrIndex(index), attrType(type),
      k(nbrClusters), 
      iterations(iter), lastOut(-1){
        
        stream.open();
      
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

        init();
      }

    ~streamclusterInfo() {
      for(size_t index = lastOut+1; index < clusterInformation.size(); 
      index++) {
        clusterInformation[index]->DeleteIfAllowed();
      }
      tupleType -> DeleteIfAllowed();
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
      size_t maxMem;
      int attrIndex;
      string attrType;
      int newAttrIndex;
      int k;
      int iterations;
      int lastOut;
      vector<Tuple*> clusterInformation;
      vector<cPoint> readData;  
      vector<Tuple*> buffer;
      
      
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;


    void init() {
      Tuple* streamTuple;
      int id = 0;
      while ((streamTuple = stream.request()) != nullptr) {
        CcInt* attrValue = (CcInt*) streamTuple->GetAttribute(attrIndex);
        int value = (int) attrValue->GetValue();
        cPoint point(id, value);
        readData.push_back(point);
        id++;
        buffer.push_back(streamTuple);
      }
      compute();
    }

    void compute() {
      //create the environment of cluster and points
      kMeans kMeansEnv(k,iterations);
      //Cluster the points we created from the stream tuples we received so far
      //Currently this will only call when the stream is terminated/results are 
      //queried. If this is supposed to be implemented in the STREAM Algorithm 
      //This should be called after the end of each block
      kMeansEnv.cluster(readData);
      //Create  the Tuples from the Clusters we generated
      for (Cluster currentCluster : kMeansEnv.getClusters()) {        
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
        clusterInformation.push_back(newTuple);     
      }
      //We talked about creating a Tuplestream with all original values present
      //and only appending their Cluster ID. This Method was supposed to do this
      //sadly i was unable to fix the errors this throws
      // appendIDs(kMeansEnv);
    }

  /*Only needed if we want to try to append the IDs
  void appendIDs(kMeans env) {
    cout << endl;
    cout << "In appendIDs() we are" << endl;
    for (Cluster currentCluster : env.getClusters()) {
      cout << endl;
      cout << "Gathering Data from Cluster: " << currentCluster.getId() << endl;
      for (cPoint point : currentCluster.getAllPoints()) {
        cout << "Creating new Attribute" << endl; 
        CcInt* clusterIndex = new CcInt(true, currentCluster.getId());
        clusterIndex ->Copy(); 
        cout << "Created Attribute has value: " 
             << clusterIndex->GetValue() << endl;
        cout << "Trying to append new Attribute to the tuple with value: " 
        << ((CcInt*) buffer[point.getId()]->
           GetAttribute(attrIndex))->
           GetValue() 
        << endl;
        buffer[point.getId()]->PutAttribute(newAttrIndex,clusterIndex);
        cout << "Appended Attribute has Value: " 
        << ((CcInt*) buffer[point.getId()]->
           GetAttribute(newAttrIndex))->
           GetValue()
        << endl;
        cout << "Elem Value is: " 
        << ((CcInt*) buffer[point.getId()]->
        GetAttribute(attrIndex))->
        GetValue() 
        << endl;

      }
    }
  }
  */
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
                   //CcInt* putAttrIndex = (CcInt*) args[6].addr;
                   if(iterations->IsDefined() && nbrOfClusters->IsDefined()){
                      int iter = iterations->GetValue();
                      int k = nbrOfClusters -> GetValue();
                      int index = attrIndex -> GetValue();
                      //int putIndex = putAttrIndex -> GetValue();
                      string type = attrType ->GetValue();
                      if(k>1) {
                        local.addr = new streamclusterInfo(args[0], 
                                     qp->GetMemorySize(s)*1024*1024*64,
                                     index, type, k, iter);
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
2.3.13 Operator ~pointgen~

*/
//Test Generator Implemented to create two dimensional 
//random point instances with set maximum x/y-coordinates
int
pointgenVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {
  
  struct Range {
    int current;
    int last;
    double* attrSize;
    double* attrSizeExt;

    Range(CcInt* amount):
    attrSize(0), attrSizeExt(0)
    {
      if (amount->IsDefined()) {
        current = 1; 
        last = amount -> GetIntval();
      } else {
          current = 1;
          last = 0;
      }
      //Implemented this before reading up on the overhaul 
      //of random numbers in c++11. Operator is just used for
      //test data gen, so was left as is.
      srand(time(NULL)+getpid());
    }

    ~Range() {
    delete[] attrSize;
    delete[] attrSizeExt;
    }
  };

  Range* range = static_cast<Range*>(local.addr);
  CcInt* attrLimit =  static_cast<CcInt*>( args[1].addr );
  int limit = attrLimit -> GetIntval();

  switch( message )
  {
    case OPEN: {
      CcInt* amount = static_cast<CcInt*>( args[0].addr );
      if(range){
        delete range;
      }
      range = new Range(amount);
      local.addr = range;
      
      return 0;
    }
    case REQUEST: { 
      if(!range) {
        return CANCEL;
      } else if ( range->current <= range->last ) {
        //See struct comment
        int x = rand() % limit; 
        int y = rand() % limit;    
        Point* elem = new Point(true, x, y);
        result.addr = elem;
        range->current++;
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (range != 0) {
        delete range;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

/*
2.3.14 Operator ~stringgen~

*/
//Test Generator Implemented to create random string
//instances. Modifications have to be done in the 
//class and not via paramater
class stringgenInfo{
  public:
  stringgenInfo(int amount, int strLength): 
  max(amount), length(strLength), 
  start(0), current(0), generator{std::random_device{}()},
  normaldist(std::normal_distribution<> (0,1))
  {}

  ~stringgenInfo(){}


  //Modify the amount of char in the array for 
  //less variety 
  string strGen() {
    
    string tempString;
    static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";    
    
    tempString.reserve(length); 

    //Modify the distribution by e.g. exchanging normaldist
    //with another distribution type
    for (int i = 0; i < length; i++) {
      int index = normaldist(generator);
      tempString += alphanum[index % (sizeof(alphanum) - 1)];
    }

    return tempString;
  }

  void inc() {
    current++;
  }

  bool done() {
    return current >= max; 
  }

  private:
    int max;
    int length;
    int start;  
    int current;
    std::mt19937 generator;
    std::normal_distribution<> normaldist;
};

int
stringgenVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

  stringgenInfo* strInfo = static_cast<stringgenInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(strInfo){
        delete strInfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcInt* attrLength = static_cast<CcInt*>( args[1].addr);
      int length = attrLength -> GetIntval();
      int amount = attrAmount -> GetIntval(); 
      if ((amount > 0) && (length > 0)) {
        local.addr = new stringgenInfo(amount, length);
      }
      return 0;

    }
    case REQUEST: { 
      if(!strInfo) {
        return CANCEL;
      } else if (!strInfo->done()) {
        string attr = strInfo->strGen();
        CcString* elem = new CcString(true, attr);
        result.addr = elem;
        strInfo->inc();
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (strInfo != 0) {
        delete strInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

/*
2.3.14 Operator ~intgen~

*/
//Test Generator Implemented to create random integer
//instances. 
class intgenInfo{
  public:
  intgenInfo(int amount, int max): 
  amount(amount), max(max), current(0), 
  generator{std::random_device{}()}
   {
  }

  ~intgenInfo(){}

  bool done() {
    return current >= amount; 
  }

  void inc() {
    current++;
  }

  int draw() {
   return generator() % max;
  }

  private:
    int amount;
    int max;
    int current;
    std::mt19937 generator;
};


int
intgenVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

  intgenInfo* intInfo = static_cast<intgenInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(intInfo){
        delete intInfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcInt* attrMax = static_cast<CcInt*>( args[1].addr);
      int amount = attrAmount -> GetIntval(); 
      int max = attrMax -> GetIntval();
      if ((amount > 0) && (max > 0)) {
        local.addr = new intgenInfo(amount, max);
      }
      return 0;

    }
    case REQUEST: { 
      if(!intInfo) {
        return CANCEL;
      } else if (!intInfo->done()) {
        intInfo->inc();
        result.addr = new CcInt(true, intInfo->draw());
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (intInfo != 0) {
        delete intInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

/*
2.3.14 Operator ~realgen~

*/
//Test Generator Implemented to create random real
//instances. 
class realgenInfo{
  public:
  realgenInfo(int amount, double min, double max, int decimals): 
  max(amount), start(0), current(0), generator{std::random_device{}()},
  distr(std::uniform_real_distribution<double> (min, max))
  {
    factor = (int)pow(10,decimals);
  }

  ~realgenInfo(){}

  void inc() {
    current++;
  }

  bool done() {
    return current >= max; 
  }
double draw() {

   double base = distr(generator);
   //Used to ensure we get the requested amount of decimals
   double result = round(base*factor)/factor;
   return result;
  }

  private:
    int max;
    int length;
    int start;  
    int current;
    int factor;
    std::mt19937 generator;
    std::uniform_real_distribution<double> distr;
};

int
realgenVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

  realgenInfo* realInfo = static_cast<realgenInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(realInfo){
        delete realInfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcReal* attrMin = static_cast<CcReal*>( args[1].addr);
      CcReal* attrMax = static_cast<CcReal*>( args[2].addr);
      CcInt* attrDecimals = static_cast<CcInt*>( args[3].addr);
      int amount = attrAmount -> GetIntval(); 
      double min = attrMin -> GetValue();
      double max = attrMax -> GetValue();
      int decimals = attrDecimals->GetIntval();
      if ((amount > 0) && (min > 0) && (max > min)) {
        local.addr = new realgenInfo(amount, min, max, decimals);
      }
      return 0;

    }
    case REQUEST: { 
      if(!realInfo) {
        return CANCEL;
      } else if (!realInfo->done()) {
        realInfo->inc();
        result.addr = new CcReal(true,realInfo->draw());
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (realInfo != 0) {
        delete realInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}


/*
2.3.15 Operator ~massquerybloom~ 

*/
//This Operator is used to send several membership queries to the Bloom
//Filter. It takes a Tuplestream, a Bloomfilter and an Attribute Name 
//and will query every Tuples Value of the Attribute with Attribute Name
//for membership in the Bloomfilter
template <class T, class S>
class massquerybloomInfo{
  public: 
    massquerybloomInfo(Word inputStream, ScalableBloomFilter* bloom, 
                       int index, string type): 
                   stream(inputStream), attrType (type), 
                   attrIndex(index), lastOut(-1), filter(bloom){

      stream.open();

      typeList = nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(attrType)
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Found"),
              nl->SymbolAtom(CcBool::BasicType())
            )         
          )
        );

      SecondoCatalog* sc = SecondoSystem::GetCatalog();
      numTypeList = sc->NumericType(typeList);
      tupleType = new TupleType(numTypeList);
      init();
    }

    ~massquerybloomInfo() {
      for(size_t index = lastOut+1; index < annotated.size(); index++) {
        annotated[index]->DeleteIfAllowed();
      }
      tupleType -> DeleteIfAllowed();
      stream.close();
    }

    Tuple* getNext() {
      lastOut++; 
      if(lastOut >= (int)annotated.size()) {
        return 0;
      }
      Tuple* t = annotated[lastOut];
      annotated[lastOut] = 0;
      return t;     
    }

    private:
      Stream<Tuple> stream;
      string attrType;
      int attrIndex;
      int lastOut;
      ScalableBloomFilter* filter;
      int nbrOfFilter = filter->getFilterList().size();
      vector<int> hashIterations = filter -> getFilterHashes();
      vector<Tuple*> annotated;
      
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;

    void init() {
      Tuple* oldTuple;
      Attribute* streamElement;
      vector<size_t> hashValues;
      //The method for querying is the exact same as in ~bloomcontains~
      while ((oldTuple = stream.request()) != nullptr) {
        Tuple* newTuple = new Tuple(tupleType);
        streamElement = (Attribute*) oldTuple->GetAttribute(attrIndex);
        size_t secondoHash = streamElement->HashValue();
        size_t* hashpointer = &secondoHash;
        for (int i = 0; i < nbrOfFilter; i++) {
          uint64_t cmHash[2];
          hashValues.clear();
          hashValues.reserve(hashIterations[i]);
          size_t filterSize = filter->getFilterList()[i].size();
          murmur(hashpointer, sizeof(secondoHash), 0, cmHash);
          size_t h1 = cmHash[0] % filterSize;
          hashValues.push_back(h1);

          if (hashIterations[i] > 1) {
            size_t h2 = cmHash[1] % filterSize;
            hashValues.push_back(h2);

            for (int j = 2; j < hashIterations[i]; j++) {
              size_t h_i = (h1 + j * h2 + j * j) % filterSize;
              hashValues.push_back(h_i); 
            }
          }
          //If the Elements hashes are all present in one 
          //subfilter the element is present and we can 
          //search for the next element
          if (filter->contains(hashValues, i)) {
            oldTuple -> DeleteIfAllowed();
            break;
          }
          //we only reach this, if the element is not present in any filter
          //The Element is not present and we create a new tuple
          if (i == (nbrOfFilter-1)) {
            T* attrValue = (T*) oldTuple->GetAttribute(attrIndex);
            attrValue->Copy();
            newTuple->PutAttribute(0, attrValue);
            newTuple->PutAttribute(1, new CcBool(true, false));
            annotated.push_back(newTuple);
            oldTuple->DeleteIfAllowed();
          }
        }
      }
    }
};

template <class T, class S>
int massquerybloomVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  massquerybloomInfo<T,S>* massq = (massquerybloomInfo<T,S>*) local.addr;
  switch(message){
    case OPEN: {
                  if(massq) {
                    delete massq;
                    local.addr = 0;
                  }
                  ScalableBloomFilter* filter = 
                  (ScalableBloomFilter*) args[1].addr;

                  CcInt* attrIndex = (CcInt*) args[3].addr;
                  CcString* attrType = (CcString*) args[4].addr;

                  if (filter -> getDefined()) {
                    int index = attrIndex->GetValue();
                    string type = attrType->GetValue();
                    local.addr = new 
                    massquerybloomInfo<T,S>(args[0], filter,
                                       index, type);
                  }
                  return 0;
                }
    case REQUEST: result.addr = massq?massq->getNext():0;
                  return result.addr?YIELD:CANCEL;
    case CLOSE: {
                    if(massq){
                      delete massq;
                      local.addr = 0;
                    }
                    return 0;
                  }
    }
    return -1;
}

//value Mapping Array
ValueMapping  massquerybloomVM[] = { 
              massquerybloomVMT<CcInt,int>,
              massquerybloomVMT<CcReal,double>,
              massquerybloomVMT<CcString,string>,
              massquerybloomVMT<CcBool,bool>
};  

// Selection Function
int massquerybloomSelect(ListExpr args){
  NList type(args);
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.third().str();
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    return 1;
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    return 3;
  } else {
    return -1;
  }
}


/*
2.3.16 Operator ~inttuplegenVM~

*/
//A Generator for random int tuples. Only Implemented
//to have a comparison how fast the operators can work
//without a ~transformstream~ or feed from a relation
class inttuplegenInfo{
  public:
  inttuplegenInfo(int amount): 
    max(amount), start(1), current(1)
   {
    typeList = nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
      nl->OneElemList(
        nl->TwoElemList(
          nl->SymbolAtom("Elem"),
          nl->SymbolAtom(CcInt::BasicType())
        )
      )
    );

    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    numTypeList = sc->NumericType(typeList);
    tupleType = new TupleType(numTypeList);
  }

  ~inttuplegenInfo(){
    tupleType -> DeleteIfAllowed(); 
  }


  Tuple* nextTuple() {
    Tuple* generatedTuple = new Tuple(tupleType); 
    CcInt* attrValue = new CcInt(true, current);
    generatedTuple -> PutAttribute(0, attrValue);
    current++;
    return generatedTuple;
  }

  int getCurrent() {
    return current; 
  }

  int getMax() {
    return max;
  }

  private:
    int max;
    int start;  
    int current;

    TupleType* tupleType;
    ListExpr typeList;
    ListExpr numTypeList;
};


int
inttuplegenVM(Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
  inttuplegenInfo* tupgen = static_cast<inttuplegenInfo*>(local.addr);

  switch( message )
  {
    case OPEN: { // initialize the local storage
      CcInt* i1 = (CcInt*) args[0].addr;
      int max = i1 -> GetIntval(); 
      if(tupgen){
        delete tupgen;
      }
      tupgen = new inttuplegenInfo(max);
      local.addr = tupgen;

      return 0;
    }
    case REQUEST: { // return the next stream element
      if(!tupgen) {
        return CANCEL;
      } else if ( tupgen->getCurrent() <= tupgen->getMax() ) {
        result.addr = tupgen->nextTuple();
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (tupgen != 0) {
        delete tupgen;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

/*
2.3.17 Operator ~stringtuplegenVM~

*/


class stringtuplegenInfo{
  public:
  stringtuplegenInfo(int amount, int strlength): 
    max(amount), length(strlength), 
    start(1), current(1)
   {
    
    srand(time(NULL)+getpid());

    typeList = nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
      nl->OneElemList(
        nl->TwoElemList(
          nl->SymbolAtom("Elem"),
          nl->SymbolAtom(CcString::BasicType())
        )
      )
    );

    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    numTypeList = sc->NumericType(typeList);
    tupleType = new TupleType(numTypeList);
  }

  ~stringtuplegenInfo(){
    tupleType -> DeleteIfAllowed();
  }


  Tuple* nextTuple() {
    Tuple* generatedTuple = new Tuple(tupleType); 
    string value = strGen();
    CcString* attrValue = new CcString(true, value);
    generatedTuple -> PutAttribute(0, attrValue);
    current++;
    return generatedTuple;
  }

  string strGen() {
    
    string tempString;
    static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";    
    
    tempString.reserve(length); 

    for (int i = 0; i < length; i++) {
      tempString += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tempString;
  }

  int getCurrent() {
    return current; 
  }

  int getMax() {
    return max;
  }

  private:
    int max;
    int length;
    int start;  
    int current;

    TupleType* tupleType;
    ListExpr typeList;
    ListExpr numTypeList;
};

int
//A Memory Leak is present within this Operator
stringtuplegenVM(Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
  stringtuplegenInfo* strtuplegen = 
              static_cast<stringtuplegenInfo*>(local.addr);

  switch( message )
  {
    case OPEN: { // initialize the local storage
      CcInt* i1 = (CcInt*) args[0].addr;
      int max = i1 -> GetIntval(); 
      CcInt* i2 = (CcInt*) args[1].addr;
      int length = i2 -> GetIntval(); 

      if(strtuplegen){
        delete strtuplegen;
      }
      strtuplegen = new stringtuplegenInfo(max, length);
      local.addr = strtuplegen;

      return 0;
    }
    case REQUEST: { // return the next stream element
      if(!strtuplegen) {
        return CANCEL;
      } else if ( strtuplegen->getCurrent() <= strtuplegen->getMax() ) {
        result.addr = strtuplegen->nextTuple();
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (strtuplegen != 0) {
        delete strtuplegen;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

/*
2.3.18 Operator ~bloomfalsepositive~

*/
//Operator is solely suitable to check for false positives. 
//It will generate random elements of int, string or point type
//which will then be checked for membership in a Bloom Filter. 
//Argument bounds should be chosen so that the Elements cannot 
//Be present in the filter, in order to test FP probability. 
//The User has to take care that the filter passed and the 
//attrIdentifier (1 = int, 2 = string, 3 = point) are congruent
int
bloomfalsepositiveVM(Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
  ScalableBloomFilter* filter = (ScalableBloomFilter*) args[0].addr;
  CcInt* i1 = (CcInt*) args[1].addr;
  int elementsToGen = i1 -> GetIntval(); 
  CcInt* i2 = (CcInt*) args[2].addr;
  int typeToGen = i2 -> GetIntval(); 
  CcInt* i3 = (CcInt*) args[3].addr;
  int lowerIntLimit = i3 -> GetIntval(); 
  int falselyPresent = 0;

  result = qp -> ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;

  int current = 0;
  size_t nbrOfFilters = filter -> getFilterList().size();
  vector<int> hashIterations = filter -> getFilterHashes();
  uint64_t cmHash[2];
  vector<size_t> hashValues;

  //Lambda for string generation
  //We use the characters to make sure strings that will result in false
  //positives are created. Admittedly a few "regular" hits can be 
  //generated this way
  auto string_gen = []() {    
    string tempString;
    static const char alphanum[] =
      "0123456789"
      "#/()!=,.-";    
    
    tempString.reserve(15); 

    for (int i = 0; i < 5; i++) {
      tempString += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tempString;
  };

  //Generate int values
  if (typeToGen == 1) {
    while (current < elementsToGen) {
      size_t intValue = lowerIntLimit + rand() / 
                      (RAND_MAX / (RAND_MAX - lowerIntLimit+1)+1);
      size_t* intPointer = &intValue; 

      //no wrapping in CcInt is necessary;
      //Secondo HashValue() for CcInt is the int itself
      //Afterwards the hashing is taking place just like in
      //~bloomcontains~
      for (size_t i = 0; i < nbrOfFilters; i++) {
        hashValues.clear();
        hashValues.reserve(hashIterations[i]);
        size_t filterSize = filter->getFilterList()[i].size();
        murmur(intPointer, sizeof(intValue), 0, cmHash);
        size_t h1 = cmHash[0] % filterSize;
        hashValues.push_back(h1);
          if (hashIterations[i] > 1) {
            size_t h2 = cmHash[1] % filterSize;
            hashValues.push_back(h2);
            for (int j = 2; j < hashIterations[i]; j++) {
              size_t h_i = (h1 + j * h2 + (j * j)) % filterSize;
              hashValues.push_back(h_i);
            }
          }
          //If the filter contains the value it has to be a false
          //positive since we should purposely generate values that 
          //cannot be present
          if (filter->contains(hashValues, i)) {
            falselyPresent++; 
          }
          current++;
      }
    } 
  //generate string values
  } else if (typeToGen == 2) {
      while (current < elementsToGen) {
        string stringValue = string_gen(); 
        //We have to generate a CcString to  use its
        //HashValue() function
        CcString* stringAttr= new CcString(true, stringValue);
        size_t secondoHash = stringAttr->HashValue();
        size_t* hashpointer = &secondoHash;
        for (size_t i = 0; i < nbrOfFilters; i++) {
          hashValues.clear();
          hashValues.reserve(hashIterations[i]);
          size_t filterSize = filter->getFilterList()[i].size();
          murmur(hashpointer, sizeof(secondoHash), 0, cmHash);
          size_t h1 = cmHash[0] % filterSize;
          hashValues.push_back(h1);
          if (hashIterations[i] > 1) {
            size_t h2 = cmHash[1] % filterSize;
            hashValues.push_back(h2);
            for (int j = 2; j < hashIterations[i]; j++) {
              size_t h_i = (h1 + j * h2 + (j * j)) % filterSize;
              hashValues.push_back(h_i);
            }
          }
          stringAttr -> DeleteIfAllowed();
          if (filter->contains(hashValues, i)) {
            falselyPresent++; 
          }
          current++;
        }
      }    
    } else {
      while (current < elementsToGen) {
        int x = lowerIntLimit + rand() / 
                    (RAND_MAX / (RAND_MAX - lowerIntLimit+1)+1);
        int y = lowerIntLimit + rand() / 
                    (RAND_MAX / (RAND_MAX - lowerIntLimit+1)+1);
        //Wrap in Point, to use HashValue()
        Point* point = new Point(true, x, y);
        size_t secondoHash = point->HashValue();
        size_t* hashpointer = &secondoHash;
        for (size_t i = 0; i < nbrOfFilters; i++) {
          hashValues.clear();
          hashValues.reserve(hashIterations[i]);
          size_t filterSize = filter->getFilterList()[i].size();
          murmur(hashpointer, sizeof(secondoHash), 0, cmHash);
          size_t h1 = cmHash[0] % filterSize;
          hashValues.push_back(h1);
          if (hashIterations[i] > 1) {
            size_t h2 = cmHash[1] % filterSize;
            hashValues.push_back(h2);
            for (int j = 2; j < hashIterations[i]; j++) {
              size_t h_i = (h1 + j * h2 + (j * j)) % filterSize;
              hashValues.push_back(h_i);
            }
          }
          point -> DeleteIfAllowed();
          if (filter->contains(hashValues, i)) {
            falselyPresent++; 
          }
          current++;
        }    
      }
    }
  res->Set(true, falselyPresent);

  return 0;
}

/*
2.3.19 Operator ~geometricdist~

*/
//One of several operators for the creation of 
//randomly distributed data for Data Structure Tests
class geometricdistInfo{
  public:
  geometricdistInfo(int samples, double probability ): 
  maxElements(samples), current(0), 
  generator{std::random_device{}()},
  geoDist(std::geometric_distribution(probability))
  {}

  ~geometricdistInfo(){}

  int draw() {
    if (current < maxElements) {
      int test = geoDist(generator);
      current++;
      return test;
    }
          current++;
    return 0;
  }

  bool done() {
    return (current >= maxElements);
  }

  private:
    int maxElements;
    double prob;
    int current;
    std::mt19937 generator;
    std::geometric_distribution<> geoDist;
};

int
geometricdistVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

  geometricdistInfo* geoInfo = static_cast<geometricdistInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(geoInfo){
        delete geoInfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcReal* attrProb = static_cast<CcReal*>( args[1].addr);
      int amount = attrAmount -> GetIntval(); 
      double prob = attrProb -> GetValue();
      if ((amount > 0) && (prob > 0 && prob < 1)) {
        local.addr = new geometricdistInfo(amount, prob);
      }
      return 0;
    }
    case REQUEST: { 
      if(!geoInfo) {
        return CANCEL;
      } else if (!geoInfo->done()) {
        result.addr = new CcInt(true, geoInfo->draw());
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (geoInfo != 0) {
        delete geoInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}


/*
2.3.20 Operator ~uniformdist~

*/
//Generates uniformly distributed numbers within an 
//Interval determined by lowerL and upperL
//Can easily be used for real value generation 
//by changing the types
class uniformdistInfo{
  public:
  uniformdistInfo(int samples, int lowerL, int upperL): 
  maxElements(samples), current(0), 
  generator{std::random_device{}()},
  unidist(std::uniform_int_distribution<> (lowerL,upperL))
  {}

  ~uniformdistInfo(){}

  int draw() {
    if (current < maxElements) {
      int test = unidist(generator);
      current++;
      return test;
    }
    current++;
    return 0;
  }

  bool done() {
    return (current >= maxElements);
  }

  private:
    int maxElements;
    double prob;
    int current;
    std::mt19937 generator;
    std::uniform_int_distribution<> unidist;
};

int
uniformdistVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

 uniformdistInfo* uniInfo = static_cast<uniformdistInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(uniInfo){
        delete uniInfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcInt* attrLowerL = static_cast<CcInt*>( args[1].addr);
      CcInt* attrUpperL = static_cast<CcInt*>( args[2].addr);
      int amount = attrAmount -> GetIntval(); 
      int lower = attrLowerL -> GetValue();
      int upper = attrUpperL -> GetValue();
      if ((amount > 0) && (lower > 0 && upper <= INT32_MAX) 
          && (lower < upper)) {
        local.addr = new uniformdistInfo(amount, lower, upper);
      }
      return 0;
    }
    case REQUEST: { 
      if(!uniInfo) {
        return CANCEL;
      } else if (!uniInfo->done()) {
        result.addr = new CcInt(true, uniInfo->draw());
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (uniInfo != 0) {
        delete uniInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

//Generates normaly distributed numbers around a mean
//with a stdDev
/*
2.3.21 Operator ~normaldist~

*/
class normaldistInfo{
  public:
  normaldistInfo(int samples, int mean, int stdDev): 
  maxElements(samples), mean(mean), stdDev(stdDev), current(0), 
  generator{std::random_device{}()},
  normaldist(std::normal_distribution<> (mean,stdDev))
  {}

  ~normaldistInfo(){}

  int draw() {
    if (current < maxElements) {
      int test = normaldist(generator);
      current++;
      return test;
    }
    current++;
    return 0;
  }

  bool done() {
    return (current >= maxElements);
  }

  private:
    int maxElements;
    int mean;
    int stdDev;
    int current;
    std::mt19937 generator;
    std::normal_distribution<> normaldist;
};

int
normaldistVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

 normaldistInfo* normalInfo = static_cast<normaldistInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(normalInfo){
        delete normalInfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcInt* attrMean = static_cast<CcInt*>( args[1].addr);
      CcInt* attrDev = static_cast<CcInt*>( args[2].addr);
      int amount = attrAmount -> GetIntval(); 
      int mean = attrMean -> GetIntval();
      int stdDev = attrDev -> GetIntval();
      if ((amount > 0) && (stdDev >= 0)) {
        local.addr = new normaldistInfo(amount, 
                                        mean, 
                                        stdDev);
      }
      return 0;
    }
    case REQUEST: { 
      if(!normalInfo) {
        return CANCEL;
      } else if (!normalInfo->done()) {
        result.addr = new CcInt(true, normalInfo->draw());
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (normalInfo != 0) {
        delete normalInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

/*
2.3.21 Operator ~normaldistreal~

*/
class normaldistrealInfo{
  public:
  normaldistrealInfo(int samples, double mean, double stdDev): 
  maxElements(samples), mean(mean), stdDev(stdDev), current(0), 
  generator{std::random_device{}()},
  normaldist(std::normal_distribution<double> {mean,stdDev})
  {}

  ~normaldistrealInfo(){}

  double draw() {
    if (current < maxElements) {
      double base = normaldist(generator);
      //In case you want more/less decimal places increase
      //or decrease the factor and denominator by a zero for each
      //additional/fewer decimal
      double result = round(base*100)/100;
      current++;
      return result;
    }
    current++;
    return 0;
  }

  bool done() {
    return (current >= maxElements);
  }

  private:
    int maxElements;
    int mean;
    int stdDev;
    int current;
    std::mt19937 generator;
    std::normal_distribution<double> normaldist;
};

int
normaldistrealVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

 normaldistrealInfo* normalInfo = static_cast<normaldistrealInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(normalInfo){
        delete normalInfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcReal* attrMean = static_cast<CcReal*>( args[1].addr);
      CcReal* attrDev = static_cast<CcReal*>( args[2].addr);
      int amount = attrAmount -> GetIntval(); 
      double mean = attrMean -> GetRealval();
      double stdDev = attrDev -> GetRealval();
      if ((amount > 0) && (stdDev >= 0)) {
        local.addr = new normaldistrealInfo(amount, 
                                        mean, 
                                        stdDev);
      }
      return 0;
    }
    case REQUEST: { 
      if(!normalInfo) {
        return CANCEL;
      } else if (!normalInfo->done()) {
        result.addr = new CcReal(true, normalInfo->draw());
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (normalInfo != 0) {
        delete normalInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

//This Operator was written to be able to determine the 
//accuracy of the values provided by the Lossy Counter 
//and the CMS 
/*
2.3.22 Operator ~distinctcount~

*/
template <class T, class S> 
class distinctcountInfo{
  public:
    distinctcountInfo(Word inputStream, int index, string type): 
                      stream(inputStream), attrIndex(index),
                      attrType(type), lastOut(-1) {
    stream.open();
    typeList = nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
        nl->TwoElemList(
          nl->TwoElemList(
            nl->SymbolAtom("Value"),
            nl->SymbolAtom(type)
          ),
           nl->TwoElemList(
              nl->SymbolAtom("Frequency"),
              nl->SymbolAtom(CcInt::BasicType())          
          )
        )
    );
    
    
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    numTypeList = sc->NumericType(typeList);
    tupleType = new TupleType(numTypeList);
    init(); 
  }

  ~distinctcountInfo(){
    tupleType -> DeleteIfAllowed(); 
    stream.close();
  }

  Tuple* next() {
    lastOut++;
    if(lastOut >= (int) distinct.size()) {
      return 0;
    }
    Tuple* returnEle = createNewTuple(distinct[lastOut]);
    return returnEle;
  }

  
  private:
    Stream<Tuple> stream; 
    int attrIndex;
    string attrType;
    int lastOut; 
    vector<S> distinct;
    //we use the Hashmap to save the distinct elements
    std::map<S,int> distr;

    TupleType* tupleType;
    ListExpr typeList;
    ListExpr numTypeList;

  void init() {
    Tuple* oldTuple; 
    while ((oldTuple = stream.request()) != nullptr) {
      T* attrValue = (T*) oldTuple->GetAttribute(attrIndex);
      S value = attrValue ->GetValue();
      if (distr.count(value) != 0) {
          distr[value]++; 
      } else {
        distr.insert({value, 1});
        distinct.push_back(value);
      }
      oldTuple -> DeleteIfAllowed();
    }
  }

  Tuple* createNewTuple(S lastOutKey) {
    Tuple* newTuple = new Tuple(tupleType);
    S value = lastOutKey; 
    int frequency = distr[lastOutKey];
    CcInt* attrFrequency = new CcInt(true, frequency);
    newTuple->PutAttribute(0, (T*)new T(true, value));
    newTuple->PutAttribute(1, attrFrequency);
    return newTuple;
  }
};

template<class T, class S>
int distinctcountVMT(Word* args, Word& result, 
           int message, Word& local, Supplier s) {

  distinctcountInfo<T,S>* distInfo = (distinctcountInfo<T,S>*) local.addr;

  switch( message )
  {
    case OPEN: {
                  if( distInfo){
                    delete distInfo;
                    local.addr = 0;
                  }
                  CcInt* attrIndex = (CcInt*) args[2].addr;
                  CcString* attrType = (CcString*) args[3].addr;
                  int index = attrIndex -> GetIntval(); 
                  string type = attrType -> GetValue();
                  local.addr = new distinctcountInfo<T,S>(args[0], index, type);
                  return 0;
    }
    case REQUEST: 
                  result.addr = distInfo?distInfo->next():0;
                  return result.addr?YIELD:CANCEL;
    case CLOSE: {
                  if (distInfo != 0) {
                    delete distInfo;
                    local.addr = 0;
                  }
                  return 0;
                }
  }
  return -1;
}

//value Mapping Array
ValueMapping  distinctcountVM[] = { 
              distinctcountVMT<CcInt,int>,
              distinctcountVMT<CcReal,double>,
              distinctcountVMT<CcString,string>,
              distinctcountVMT<CcBool,bool>
};  

// Selection Function
int distinctcountSelect(ListExpr args){
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
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    return 3;
  } else {
    return -1;
  }
}

/*
2.3.23 Operator ~cmsoverreport~

*/
//Used to determine the size of the error, and the overcount guarantees
//given by the CMS. This only works in combination with a feed from 
//the ~distinctcount~ operator. I created a relation with let ~distinctcount~
//and used feed on that one afterward
template <class T>
class cmsoverreportInfo{
  public: 
    cmsoverreportInfo(Word inputStream, CountMinSketch* countmin, 
                      int count, double eps, int index, string type): 
                   stream(inputStream), cms(countmin), n(count), error(eps),
                   attrIndex(index), attrType (type), lastOut(-1){

      stream.open();

      typeList = nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Value"),
              nl->SymbolAtom(attrType)
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Overcount"),
              nl->SymbolAtom(CcReal::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Error"),
              nl->SymbolAtom(CcInt::BasicType())
            )         
          )
        );

      cout << "Output in cms overreport is: " << nl->ToString(typeList) << endl;

      SecondoCatalog* sc = SecondoSystem::GetCatalog();
      numTypeList = sc->NumericType(typeList);
      tupleType = new TupleType(numTypeList);
      init();
    }

    ~cmsoverreportInfo() {
      for(size_t index = lastOut+1; index < annotated.size(); index++) {
        annotated[index]->DeleteIfAllowed();
      }
      tupleType -> DeleteIfAllowed();
      stream.close();
    }

    Tuple* getNext() {
      lastOut++; 
      if(lastOut >= (int)annotated.size()) {
        return 0;
      }
      Tuple* t = annotated[lastOut];
      annotated[lastOut] = 0;
      return t;     
    }

    private:
      Stream<Tuple> stream;
      CountMinSketch* cms;
      int n; 
      double error;

      int attrIndex;
      string attrType;
      int lastOut;

      int cmsEstimate; 
      int realCount;
      double overestimation;
      vector<Tuple*> annotated;
      
      TupleType* tupleType;
      ListExpr typeList;
      ListExpr numTypeList;

    void init() {
      Tuple* oldTuple;
      Attribute* streamElement;
      CcInt* streamElementFreq;
      while ((oldTuple = stream.request()) != nullptr) {
        streamElement = (Attribute*) oldTuple->GetAttribute(0);
        streamElementFreq = (CcInt*) oldTuple->GetAttribute(1);
        size_t secondoHash = streamElement->HashValue();
        cmsEstimate = cms->estimateFrequency(secondoHash);
        realCount = streamElementFreq->GetValue();
        //The CMS count surpasses the accurate count of an Element
        if (cmsEstimate > realCount) {
          //Working with the percentage of the overcount 
          //Tests were also done with the average total overcount 
          //(estimate - realcount)
          overestimation = (cmsEstimate/realCount)*100;
          Tuple* newTuple = new Tuple(tupleType); 
          T* attrValue = (T*) oldTuple->GetAttribute(attrIndex);
          attrValue->Copy(); 
          newTuple->PutAttribute(0, attrValue);
          CcReal* attrOverestimation = new CcReal(true, overestimation);       
          newTuple->PutAttribute(1, attrOverestimation);
          //Error guarantee was violated
          if (!(cmsEstimate <= (realCount + (error*n)))) {
            CcInt* attrError = new CcInt(true,1);
            newTuple->PutAttribute(2, attrError);
          //error guarantee was not violated, but we need
          //an output for the tuple
          } else {
            CcInt* attrError = new CcInt(true,0);
            newTuple->PutAttribute(2, attrError);
          }
          annotated.push_back(newTuple);
          oldTuple->DeleteIfAllowed();
        } else {
          oldTuple->DeleteIfAllowed();
        }
      }
    }     
};

template <class T>
int cmsoverreportVMT(Word* args, Word& result,
           int message, Word& local, Supplier s){
  
  cmsoverreportInfo<T>* overr = (cmsoverreportInfo<T>*) local.addr;
  switch(message){
    case OPEN: {
                  if(overr) {
                    delete overr;
                    local.addr = 0;
                  }
                  CountMinSketch* cms = 
                  (CountMinSketch*) args[1].addr;

                  CcInt* attrCount = (CcInt*) args[3].addr;
                  CcReal* attrEps = (CcReal*) args[4].addr;
                  CcInt* attrIndex = (CcInt*) args[5].addr;
                  CcString* attrType = (CcString*) args[6].addr;

                  if (cms -> getDefined()) {
                    int index = attrIndex->GetValue();
                    string type = attrType->GetValue();
                    int count = attrCount->GetValue();
                    double eps = attrEps->GetValue();
                    local.addr = new 
                    cmsoverreportInfo<T>(args[0], cms, count,
                                       eps, index, type);
                  }
                  return 0;
                }
    case REQUEST: result.addr = overr?overr->getNext():0;
                  return result.addr?YIELD:CANCEL;
    case CLOSE: {
                    if(overr){
                      delete overr;
                      local.addr = 0;
                    }
                    return 0;
                  }
    }
    return -1;
}

//value Mapping Array
ValueMapping  cmsoverreportVM[] = { 
              cmsoverreportVMT<CcInt>,
              cmsoverreportVMT<CcReal>,
              cmsoverreportVMT<CcString>,
              cmsoverreportVMT<CcBool>,
};  

// Selection Function
int  cmsoverreportSelect(ListExpr args){
  NList type(args);
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = type.third().str();
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);
  
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    return 1;
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    return 3;
  } else {
    return -1;
  }
}

/*
2.3.24 Operator ~switchingdist~

*/
//Generates normally distributed numbers, but introduces  a 
//break point, after which the normaldistribution will shift. 
//Currently not parameter implemented, but hardcoded.
class switchingdistInfo{
  public:
  switchingdistInfo(int samples, int mean, int stdDev, double reroll): 
  maxElements(samples), mean(mean), stdDev(stdDev), switchPct(reroll), 
  current(0), generator{std::random_device{}()},
  normaldist(std::normal_distribution<> (mean,stdDev))
  {
    //Determine the point at which the user wants 
    //to switch the mean and stdDev of distribution
    switchPoint = floor(maxElements*switchPct);       
  }

  ~switchingdistInfo(){}

  int draw() {
    if(current == switchPoint) {
      //set the severity of the breakpoint here
      std::normal_distribution<>::param_type d2(10, 5);
      normaldist.param(d2);
    }
    if (current < maxElements) {
      int test = normaldist(generator);
      current++;
      return test;
    }
    current++;
    return 0;
  }

  bool done() {
    return (current >= maxElements);
  }

  private:
    int maxElements;
    int switches;
    int mean;
    int stdDev;
    int switchPoint;
    double switchPct; 
    int current;
    std::mt19937 generator;
    std::normal_distribution<> normaldist;
};

int
switchingdistVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

 switchingdistInfo* swdinfo = static_cast<switchingdistInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(swdinfo){
        delete swdinfo;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcInt* attrMean = static_cast<CcInt*>( args[1].addr);
      CcInt* attrDev = static_cast<CcInt*>( args[2].addr);
      CcReal* attrSwitchPrt = static_cast<CcReal*>( args[3].addr);
      int amount = attrAmount -> GetIntval(); 
      int mean = attrMean -> GetIntval();
      int stdDev = attrDev -> GetIntval();
      double switchPct = attrSwitchPrt -> GetValue();
      if ((amount > 0) && (stdDev >= 0)) {
        local.addr = new switchingdistInfo(amount, 
                                        mean, 
                                        stdDev,
                                        switchPct);
      }
      return 0;
    }
    case REQUEST: { 
      if(!swdinfo) {
        return CANCEL;
      } else if (!swdinfo->done()) {
        result.addr = new CcInt(true, swdinfo->draw());
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (swdinfo != 0) {
        delete swdinfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

/*
2.3.25 Operator ~samplegen~

*/
//Used to test the ~reservoir~ Operator by creating a user determined
//amount of samples, which are split into windows of user given size. 
//Each window only contains the same timestamp values and the time stamps
//will increase by 1 after each window
//since other values than timestamp are irrelevant for testing the reservoir
//no further ways to generate exist
class samplegenInfo{
  public:
  samplegenInfo(int samples, int windowSize): 
  maxElements(samples),  current(0), rolling(0),
  interval(windowSize),
  generator{std::random_device{}()}
  {
    typeList = 
      nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Elem"),
              nl->SymbolAtom(CcInt::BasicType())
            ),
            nl->TwoElemList(
              nl->SymbolAtom("Timestamp"),
              nl->SymbolAtom(CcInt::BasicType())
            )
          )
      );

    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    numTypeList = sc->NumericType(typeList);
    tupleType = new TupleType(numTypeList);
  }

  ~samplegenInfo(){
    tupleType ->DeleteIfAllowed();
  }

  Tuple* next() {
    //Increases the number after the user determined
    //No padding exists at the end so an interval of 10
    //with 15 samples will result in ten 0, but only five 
    //1 values
    if ((current % interval) == 0) {
      rolling++;
    }
    Tuple* generatedTuple = new Tuple(tupleType); 
    CcInt* attrValue = new CcInt(true, generator());
    CcInt* attrTimestamp = new CcInt(true, rolling);
    generatedTuple -> PutAttribute(0, attrValue);
    generatedTuple -> PutAttribute(1, attrTimestamp);
    current++;
    return generatedTuple;
  }

    int maxElements;
    int current;
    int rolling;
    int interval;
    std::mt19937 generator;

    TupleType* tupleType;
    ListExpr typeList;
    ListExpr numTypeList;
};

int
samplegenVM(Word* args, Word& result,
           int message, Word& local, Supplier s) {

 samplegenInfo* sampgen = static_cast<samplegenInfo*>(local.addr);

  switch( message )
  {
    case OPEN: {
      if(sampgen){
        delete sampgen;
      }
      CcInt* attrAmount = static_cast<CcInt*>( args[0].addr);
      CcInt* attrInterval = static_cast<CcInt*>( args[1].addr);
      int amount = attrAmount -> GetIntval(); 
      int interval = attrInterval -> GetIntval();
      if (amount > 0) {
        local.addr = new samplegenInfo(amount, 
                                        interval);
      }
      return 0;
    }
    case REQUEST: { 
      if(!sampgen) {
        return CANCEL;
      } else if (sampgen->current < sampgen->maxElements) {
        result.addr =  sampgen->next();;
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (sampgen != 0) {
        delete sampgen;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}


/*
2.3.26 Operator ~lossycompare~

*/
//Tests whether the counts generated by our lossy counter Implementation
//are within the specified parameters. It is necessary to consume an lcfrequent
//and distinctcount stream from the same basestream for this comparison to work
template <class T, class S>
class lossycompareInfo{
  public:
  lossycompareInfo(Word stream1, Word stream2, int n, double eps, 
  double supp, string type):
  firstStream(stream1), secondStream(stream2), lastOut(-1)
  {
    typeList = nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
      nl->TwoElemList(
          nl->TwoElemList(
            nl->SymbolAtom("Value"),
            nl->SymbolAtom(type)
          ),  
          nl->TwoElemList(
            nl->SymbolAtom("ErrorCode"),
            nl->SymbolAtom(CcInt::BasicType())
          )
        )
    );
    
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    numTypeList = sc->NumericType(typeList);
    tupleType = new TupleType(numTypeList);
    
    //no frequent item should be missing from the
    //~lcfrequent~ result
    falseNegativeThresh = n*supp;
    //Non-Frequent items present in ~lcfrequents~ result
    //should have a real frequency of at least:
    falsePositiveThresh = (supp-eps)*n; 
    //For all frequent Items a difference between their real
    //frequency and the one calculated by ~lcfrequent~ is
    //allowed
    allowedDiff = eps*n;

    firstStream.open();
    secondStream.open();
    collectStream1(); 
    collectStream2();
    testValues(); 
  }

  ~lossycompareInfo(){
    for(size_t index = lastOut+1; index < results.size(); index++) {
      results[index]->DeleteIfAllowed(); 
    }
    firstStream.close();
    secondStream.close();
  }

  Tuple* next() {
    lastOut++;
    if (lastOut >= (int) results.size()) {
      return 0;
    }
    Tuple* mergedValues = results[lastOut];
    results[lastOut] = 0;
    return mergedValues;
  }

  //First stream should always be the correct count we can get from e.g.
  //~distinctcount~. We can then operate on the assumption that
  //stream1.size > stream2.size
  void collectStream1() {
    Tuple* oldTuple;
    while ((oldTuple = firstStream.request()) != 0) {
      S value = ((T*) oldTuple->GetAttribute(0))->GetValue();
      int freq = ((CcInt*) oldTuple ->GetAttribute(1)) -> GetValue();
      buffer1.insert({value,freq});
      oldTuple -> DeleteIfAllowed();
    }
  }

  //
  void collectStream2() {
    Tuple* oldTuple; 
    while ((oldTuple = secondStream.request()) != 0) {
      S value = ((T*) oldTuple->GetAttribute(0))->GetValue();
      int freq = ((CcInt*) oldTuple->GetAttribute(1)) -> GetValue();
      buffer2.insert({value,freq});
      oldTuple -> DeleteIfAllowed();
    }
  }

  void testValues() {
    //We check all distinct items of Testset which we 
    //collected from ~distinctcount~
    for (auto element : buffer1) {
      //These are the items that are frequent by an accurate count
      if (element.second >= falseNegativeThresh) {

        //Even though the Item is frequent, it is not present in our results
        //generated by ~lcfrequent~: This is a false negative
        if (buffer2.find(element.first) == buffer2.end()) {
          cout << "Element is not present in the lcfrequent; creating a tuple " 
          << endl;
          Tuple* newTuple = new Tuple(tupleType);
          S value = element.first;
          T* attrValue = new T(true, value);
          CcInt* reasonCode = new CcInt(true, 1);
          newTuple->PutAttribute(0, attrValue);
          newTuple->PutAttribute(1, reasonCode);
          results.push_back(newTuple);
        //The Item is present in the ~lcfrequent~ result, 
        // but we have to check if The ~lcfrequent~ count is accurate enough
        } else {
          cout << "Calculating the difference between lcfrequency: " 
               << element.second << " and distinct frequency: " 
               << buffer2[element.first] << endl;
          int difference = element.second - buffer2[element.first]; 
          cout << "Difference is: "<<  difference << " vs allowed Dif: "
               << allowedDiff << endl;
          //The frequency calculated by ~lcfrequent~ surpasses the true count
          //by more than eps*n
          if (abs(difference) > allowedDiff) {
            cout << "Difference is surpassing the threshold";
            Tuple* newTuple = new Tuple(tupleType);
            S value = element.first;
            T* attrValue = new T(true, value);
            CcInt* reasonCode = new CcInt(true, 3);
            newTuple->PutAttribute(0, attrValue);
            newTuple->PutAttribute(1, reasonCode);
            results.push_back(newTuple);
          }
        }
      //The Items treated here are not frequent by an accurate count
      } else {
          cout << "Checking item " << element.first 
               << " with freq: " << element.second << endl;
          //Even though they are not frequent, they are present in the 
          //~lcfrequent~ results
          if (!(buffer2.find(element.first) == buffer2.end())) {
            cout << "Item is present even though its " 
                 << "real frequency is below the threshold: " 
                 << falseNegativeThresh << endl;
            //These items are false positives and their accurate frequency 
            //does not fullfill the minimum threshold ((minsup-eps)*n)
            if(element.second < falsePositiveThresh) {
              cout << " Its real frequency does not " 
                   << "surpass the falsepositive threshold: " 
                   << falsePositiveThresh << endl;
              Tuple* newTuple = new Tuple(tupleType);
              cout << "Creating a new tuple" << endl;
              S value = element.first;
              cout << "Assigned Tuple Value " << value << endl;
              T* attrValue = new T(true, value);
              cout << "Assigned attribute value" << endl;
              CcInt* reasonCode = new CcInt(true, 2);
              cout << "Assigned reasoncode value" << endl;
              newTuple->PutAttribute(0, attrValue);
              newTuple->PutAttribute(1, reasonCode);
              results.push_back(newTuple);
             cout << "Put both tuple attributes" << endl;
            }
            cout << "but its real frequency: " << element.second 
                 << ">" << falsePositiveThresh << endl;
          }
      }
    }
  }

  private:
    Stream<Tuple> firstStream;
    Stream<Tuple> secondStream; 
    vector<Tuple*> results;
    std::unordered_map<S, int> buffer1;
    std::unordered_map<S, int> buffer2;
    double falseNegativeThresh;
    double falsePositiveThresh;
    double allowedDiff;

    int lastOut;
    double adaptor;
    TupleType* tupleType;
    ListExpr typeList;
    ListExpr numTypeList;
};

template <class T, class S>
int lossycompareVMT(Word* args, Word& result,
           int message, Word& local, Supplier s) {

  lossycompareInfo<T,S>* lcInfo = 
    static_cast<lossycompareInfo<T,S>*>(local.addr);
  CcInt* attrN = (CcInt*) args[2].addr;
  CcReal* attrEps = (CcReal*) args[3].addr;
  CcReal* attrSup = (CcReal*) args[4].addr;
  CcString* attrType = (CcString*) args[5].addr;
  int n = attrN ->GetValue();
  double eps = attrEps -> GetRealval();
  double sup = attrSup -> GetRealval();
  string type = attrType -> GetValue();

  switch( message )
  {
    case OPEN: {
      if(lcInfo){
        delete lcInfo;
      }
        local.addr = new lossycompareInfo<T,S>(args[0], 
                                          args[1],
                                          n,
                                          eps,
                                          sup,
                                          type);
    }
    case REQUEST : result.addr = lcInfo?lcInfo->next():0;
                  return result.addr?YIELD:CANCEL;
    case CLOSE: {
      if (lcInfo != 0) {
        delete lcInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

//value Mapping Array
ValueMapping lossycompareVM[] = { 
              lossycompareVMT<CcInt,int>,
              lossycompareVMT<CcReal,double>,
              lossycompareVMT<CcString, string>,
              lossycompareVMT<CcBool, bool>
};  

// Selection Function
int lossycompareSelect(ListExpr args){
  NList type(args);
  NList attrList = type.first().second().second();
  ListExpr attrType;
  string attrName = "Value";
  listutils::findAttribute(attrList.listExpr(), 
                           attrName, attrType);
  
  if (nl->ToString(attrType) == CcInt::BasicType()) {
    return 0;
  } else if (nl->ToString(attrType) == CcReal::BasicType()){
    return 1;
  } else if (nl->ToString(attrType) == CcString::BasicType()){
    return 2;
  } else if (nl->ToString(attrType) == CcBool::BasicType()) {
    return 3;
  } else {
    return -1;
  }
}

/*
2.4 Description of Operators

*/

  OperatorSpec reservoirSpec(
    "stream(T) x int -> stream(T), T = TUPLE or T = DATA",
    "_ reservoir [_] ",
    "Creates a biased reservoir sample of a supplied stream of a given size "
    "the second argument specifies the reservoir size, the third argument " 
    " specifies the inclusion Probablity",
    "query intstream(1,1000) reservoir[10] count"
  );

   OperatorSpec tiltedtimeSpec(
    "stream(tuple(X)) x int x int -> stream(T), T = TUPLE or T = DATA",
    "_ tiltedtime [_,_] ",
    "Creates a progressive logarithmic tilted time frame of a stream "
    "the second argument specifies the Attribute Name of which we take "
    "snapshots, the third the amount of snapshots we will take",
    "query intstream(1,1000) tiltedtime[3,10] count"
  );

  OperatorSpec createbloomfilterSpec(
    "stream(tuple(X)) x ATTR x real ->  bloomfilter",
    "_ createbloomfilter [_,_]",
    "Creates a Bloom Filter for an Attribute of a supplied Tuplestream"
    "The second argument specifies the name of the Attribute whose "
    "values we create the filter for, the third the false positive "
    "probability",
    "query Kinos feed createbloomfilter[Name,0.01]"
  );

  OperatorSpec bloomcontainsSpec(
    "scalablebloomfilter x T -> bool, T = DATA",
    "_ bloomcontains [_]",
    "Checks for the presence of an Element of type T in a supplied Bloomfilter."
    "The first argument specifies the Filter, the second the search element",
    "query Kinos feed createbloomfilter[Name,0.01] bloomcontains[\"Astor\"]"
  );

  OperatorSpec createcountminSpec(
    "stream(tuple(X)) x ATTR x int x real ->  countminsketch",
    "_ createcountminSpec [_,_,_]",
    "Creates a Count Mint Sketch for an Attribute of a supplied Tuplestream "
    "the second argument specifies the name of the Attribute whose values we "
    "use to create the filter, the third the errorbound and the fourth the "
    "inverse probability that the values are within the errorbound",
    "query Kinos feed createcountmin[Name,0.01,0.1]"
  );

  OperatorSpec cmscountSpec(
    "countminsketch x T -> bool, T = TUPLE or T = DATA",
    "_ cmscountSpec [_]",
    "Gives an estimate of how often an Element appeared in the Stream the "
    "Count Min Sketch was created for. The second argument presents the  "
    "element whose count we retrieve. Ensuring that was made for the values "
    "you search is user responsiblity. The Sketch might deliver counts for "
    "unrelated search terms too.", 
    "query Kinos feed createcountmin[Name,0.01,0.1] cmscount[\"Astor\"]"
  );

  OperatorSpec createamsSpec(
    "stream(tuple(X)) x ATTR x real x real ->  amssketch",
    "_ createamsSpec [_,_,_]",
    "Creates an AMS Sketch for an Attribute of a supplied Tuplestream "
    "the first argument specifies the name of the Attribute whose values we "
    "use to create the filter, the second the errorbound and the third the "
    "inverse probability that the values are within the errorbound",
    "query Kinos feed createams[Name,0.01,0.1]"
  );

  OperatorSpec amsestimateSpec(
    "amssketch -> int",
    "_ amsestimate ",
    "Creates an estimate of the selfjoin Size of the Stream "
    "which the passed AMS Sketch was created for regarding the "
    "Attribute it was created for",
    "query Kinos feed createams[Name,0.01,0.1] amsestimate"
  );

  OperatorSpec createlossycounterSpec(
    "stream(tuple(X)) x ATTR x real -> stream(tuple(X int real int int))",
    "_ createlossycounter [_,_]",
    "Creates a lossy Counter for an Attributes values in "
    "the supplied Tuple Stream. The second argument specifies "
    "the name of the Attribute whose values we count, the second "
    "the errorbound",
    "query Kinos feed createlossycounter[Name, 0.005] lcfrequent"
  );

  OperatorSpec lcfrequentSpec(
    "stream(tuple(X int int int real)) -> stream(tuple(X int int))",
    "_ lcfrequent [_]",
    "Further mines the Tuplestream created by the ~createlossycounter~ "
    "Operator. The argument is the minimum Support of the Items that "
    "are returned",
    "query Kinos feed createlossycounter[Name,0.001] lcfrequent[0.01]"
  );

  OperatorSpec outlierSpec(
    "stream(tuple(T)) x ATTR x real -> stream(tuple(T, int)), T = int or real",
    "_ outlier [_,_]",
    "Determines the Outlier of a Tuplestream regarding to an int "
    "or real Attribute. Is sadly not very precise. The second argument "
    "is the Name of the Attribute we monitor, the third the z-score "
    "from which on we will consider an Element as outlier.",
    "query intstream(0,100) transformstream outliers[Elem, 3] consume"
  );

  OperatorSpec streamclusterSpec(
    "stream(T) x ATTR x int x int -> outlierstream(tuple(int real int)) "
    "T = int or T = real",
    "_ streamcluster [_,_,_]",
    "Runs the k-means Algorithm on the Attribute of the Tuple whose name "
    "is supplied. The name is the second Argument, the number of "
    "clusters to be generated the second and the third the number of "
    "iterations the Algorithm will run.",
    "query intstream(0,100) transformstream streamcluster[Elem,2,3] consume"
  );

  //The following Operator are the ones created for testing the 
  //implemented Data Structures
  OperatorSpec pointgenSpec(
    "int x int -> stream(point)",
    "pointgen (_,_)",
    "Creates points with x- and y-Coordinate random two dimensional "
    "coordinates. First argument is the amount, second the maximum value "
    "a coordinate can take",
    "query pointgen(10,100) count"
  );
  
  OperatorSpec stringgenSpec(
    "int x int-> stream(string)",
    "stringgen (_,_)",
    "Creates random string values out of a certain set of characters. " 
    "The first argument specifies the amount, the second the length",
    "query stringgen(10,3) count"
  );

  OperatorSpec intgenSpec(
    "int x int-> stream(int)",
    "stringgen (_,_)",
    "Creates random integer values that have certain maximum. " 
    "The first argument specifies the amount, the second the maximum",
    "query intgen(10,100) count"
  );
  
  OperatorSpec realgenSpec(
    "int x real x real x int-> stream(real)",
    "stringgen (_,_)",
    "Creates random real values in a certain range with a determined "
    "amount of decimals. The first argument specifies the amount, the "
    "second the minimum, the third the maximum and the fourth the "
    "amount of decimals",
    "query realgen(100,0.01,2.0,3) count"
  );

  OperatorSpec massquerybloomSpec(
    "Stream(Tuple(X)) x bloomfilter x ATTR -> stream(tuple(x bool))",
    "_ massquerybloomOp [_,_]",
    "Sends membership queries to the Bloomfilter regarding all Attribute "
    "Values of a requested Attribute in the Tuplestream. The first argument "
    "is the Tuplestream, the second the Bloomfilter to query and the "
    "third the Attribute Name of the Attribute whose values we query",
    "query int(1,100) transformstream massquerybloom[<bloomfilter>, Elem]" 

  );

  OperatorSpec inttuplegenSpec(
    "int -> stream(Tuple(Elem int))",
    "inttuplegen(_)",
    "Creates a Tuple Stream consisting of a set amount "
    "of single int Attribute Tuples. The argument determines " 
    "the amount of tuples that will be created",
    "query inttuplegen(1,100) createbloomfilter[Elem,0.1]"
  );

  OperatorSpec stringtuplegenSpec(
    "int x int -> stream(Tuple(Elem string))",
    "stringtuplegen(_,_)",
    "Creates a Tuple Stream consisting of a set amount "
    "of string Attribute Tuples. The first argument determines " 
    "the amount of tuples that will be created, the second their "
    "length",
    "query inttuplegen(1,100) createbloomfilter[Elem,0.1]"
  );

  OperatorSpec bloomfalsepositiveSpec(
    "bloomfilter x int x int x int -> int",
    "bloomfalsepositve(_,_,_)",
    "Generates random int, string or point values and checks "
    "the values for membership in a provided Bloom Filter "
    "The first argument is the amount of Elements to generate, "
    "the second the type (1 = int, 2 = string, 3 = point) and "
    "the last the lower limit the generated int values can take. "
    "These were used to test the p(FP) of Bloomfilters",
    "query bloomfalsepositive [<bloomfilter>, 100, 1, 10000]"
  );

  OperatorSpec geometricdistSpec(
    "int x real -> Stream(int)",
    "geometricdist(_,_)",
    "Generates a geometrically distributed Stream of numbers "
    "the first argument is the amount to be generated, the second "
    "the success probability for each try",
    "query geometricdist(100,0.2) consume"
  );

  OperatorSpec uniformdistSpec(
    "int x int x int -> Stream(int)",
    "uniformdist(_,_,_)",
    "Generates a uniformly distributed Stream of numbers "
    "the first argument is the amount to be generated, the second "
    "the upper limit of the numbers generated and the third the "
    "lower limit",
    "query uniformdist(100,1,100) consume"
  );

  OperatorSpec normaldistSpec(
    "int x int x int -> Stream(int)",
    "normaldist(_,_,_)",
    "Generates a uniformly distributed Stream of integer numbers "
    "the first argument is the amount to be generated, the second "
    "the mean and the third the standard Deviation",
    "query normaldist(100,0,1) consume"
  );

  OperatorSpec normaldistrealSpec(
    "int x real x real -> Stream(real)",
    "normaldist(_,_,_)",
    "Generates a uniformly distributed Stream of real numbers "
    "the first argument is the amount to be generated, the second "
    "the mean and the third the standard Deviation",
    "query geometricdist(100,0.0,1.0) consume"
  );

  OperatorSpec distinctcountSpec(
    "Stream(T) x ATTR -> Stream(tuple(ATTR Value, ATTR Frequency))"
    "T in Int, Real, String, Bool",
    "_ distinctcount(_)",
    "Gives an accurate distinct count of the Attribute values of a "
    "set Attribute of a Tuple Stream. The second Argument provides "
    "the Attributes Name whose values should be counted",
    "query intstream(1,100) transformstream distinctcount[Elem] consume"
  );

  OperatorSpec cmsoverreportSpec(
    "Stream(Tuple) x countminsketch x ATTR -> stream(real)",
    "_ cmsoverreport(_,_)",
    "Compares the accurate count of elements with the one made"
    "by a CMS. The first argument has to be a Tuplestream created "
    "via the ~distinctcount~ Operator, the second one a CMS and the "
    "third the Attribute Name of the Element from the Stream. Since " 
    "this is restricted to distinctcount Streams it will be Value",
    "query intstream(1,100) transformstream distinctcount[Elem] "
    "cmsoverreport[<countmin>, Value]"
  );

  OperatorSpec switchingdistSpec(
    "int x int x int -> Stream(int)",
    "normaldist(_,_,_,_)",
    "Generates a uniformly distributed Stream of integer numbers, "
    "but introduces a breakpoint after a number of elements "
    "the first argument is the amount to be generated, the second "
    "the mean, the third the standard Deviation of the initial "
    "distribution, the fourth is the percentage of samples generated "
    "after which the shift to the other distribution should occur",
    "query switchingdist(100,0,1,0.3) consume"
  );

  OperatorSpec samplegenSpec(
    "int x int -> Stream(int)",
    "samplegendist(_,_)",
    "Generates a Stream of Tuples with timestamps, which are split into "
    "windows. Each window element contains the same timestamp. The size "
    "of the Window is user determind. The first argument determines the "
    "amount of samples generated. The second the window size. No padding "
    "exists",
    "query samplegen(100,10) count"
  );

  OperatorSpec lossycompareSpec(
    "Stream(tuple(X int)) x Stream(T(tuple(X int))) -> Stream(tuple(X int))",
    "_ _ lossycompare",
    "Generates a Stream of Elements and the error guarantee which they " 
    "violated. Violations are coded, as 1 for a frequentElement not being "
    "in output of ~lcfrequent~, 2 for an Item being a false positive "
    "violating the errorguarantee and 3 for the Element being overcounted "
    "too much by ~lcfrequent~. Only works with streams from ~distinctcount~ "
    "and ~lcfrequent~ being the first and second argument respectively.",
    "query <distinctcount rel> feed <lcfrequent rel> feed lossycompare consume"
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

Operator pointgenOp(
  "pointgen",
  pointgenSpec.getStr(),
  pointgenVM,
  Operator::SimpleSelect,
  pointgenTM
);

Operator stringgenOp(
  "stringgen",
  stringgenSpec.getStr(),
  stringgenVM,
  Operator::SimpleSelect,
  stringgenTM
);

Operator intgenOp(
  "intgen",
  intgenSpec.getStr(),
  intgenVM,
  Operator::SimpleSelect,
  intgenTM
);

Operator realgenOp(
  "realgen",
  realgenSpec.getStr(),
  realgenVM,
  Operator::SimpleSelect,
  realgenTM
);

Operator massquerybloomOp(
  "massquerybloom",
  massquerybloomSpec.getStr(),
  4,
  massquerybloomVM,
  massquerybloomSelect,
  massquerybloomTM
);

Operator inttuplegenOp(
  "inttuplegen",
  inttuplegenSpec.getStr(),
  inttuplegenVM, 
  Operator::SimpleSelect,
  inttuplegenTM
);

Operator stringtuplegenOp(
  "stringtuplegen",
  stringtuplegenSpec.getStr(),
  stringtuplegenVM, 
  Operator::SimpleSelect,
  stringtuplegenTM
);

Operator bloomfalsepositiveOp(
  "bloomfalsepositive",
  bloomfalsepositiveSpec.getStr(),
  bloomfalsepositiveVM, 
  Operator::SimpleSelect,
  bloomfalsepositiveTM
);

Operator geometricdistOp(
  "geometricdist",
  geometricdistSpec.getStr(),
  geometricdistVM, 
  Operator::SimpleSelect,
  geometricdistTM
);

Operator uniformdistOp(
  "uniformdist",
  uniformdistSpec.getStr(),
  uniformdistVM, 
  Operator::SimpleSelect,
  uniformdistTM
);

Operator normaldistOp(
  "normaldist",
  normaldistSpec.getStr(),
  normaldistVM, 
  Operator::SimpleSelect,
  normaldistTM
);

Operator normaldistrealOp(
  "normaldistreal",
  normaldistrealSpec.getStr(),
  normaldistrealVM, 
  Operator::SimpleSelect,
  normaldistrealTM
);

Operator distinctcountOp(
  "distinctcount",
  distinctcountSpec.getStr(),
  4,
  distinctcountVM,
  distinctcountSelect,
  distinctcountTM
);

Operator cmsoverreportOp(
  "cmsoverreport",
  cmsoverreportSpec.getStr(),
  4,
  cmsoverreportVM,
  cmsoverreportSelect,
  cmsoverreportTM
);

Operator switchingdistOp(
  "switchingdist",
  switchingdistSpec.getStr(),
  switchingdistVM, 
  Operator::SimpleSelect,
  switchingdistTM
);


Operator samplegenOp(
  "samplegen",
  samplegenSpec.getStr(),
  samplegenVM, 
  Operator::SimpleSelect,
  samplegenTM
);

Operator lossycompareOp(
  "lossycompare",
  lossycompareSpec.getStr(),
  4,
  lossycompareVM, 
  lossycompareSelect,
  lossycompareTM
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
    //Assure that the Operator has a set part of the
    //Main Memory at its disposal
    streamclusterOp.SetUsesMemory();

    //Gen Operator for testing
    AddOperator(&pointgenOp);
    AddOperator(&stringgenOp);
    AddOperator(&intgenOp);
    AddOperator(&realgenOp);
    AddOperator(&massquerybloomOp);
    AddOperator(&inttuplegenOp);
    AddOperator(&stringtuplegenOp);
    AddOperator(&bloomfalsepositiveOp);
    AddOperator(&geometricdistOp);
    AddOperator(&uniformdistOp);
    AddOperator(&normaldistOp);
    AddOperator(&normaldistrealOp);
    AddOperator(&distinctcountOp);
    AddOperator(&cmsoverreportOp);
    AddOperator(&switchingdistOp);
    AddOperator(&samplegenOp);
    AddOperator(&lossycompareOp);
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

