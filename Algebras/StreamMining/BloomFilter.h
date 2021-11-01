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

*/

#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include "NestedList.h"
#include "ListUtils.h"
#include "AlgebraTypes.h"

namespace eschbach {
    class ScalableBloomFilter
{
  public:
  ScalableBloomFilter(const double inputErrorRate);
  ScalableBloomFilter(const ScalableBloomFilter& rhs);
  ~ScalableBloomFilter(){}

  //Getter und Setter
  bool getDefined() const;
  void setDefined();
  size_t getCurMaxInserts() const; 
  double getFP() const;
  double getRolFP();
  bool getElement(size_t filterIndex, size_t eleIndex) const;
  bool setElement(size_t filterIndex, size_t eleIndex, bool value);
  void setElementOpen(size_t filterIndex, size_t eleIndex, bool value);
  int getCurNumberHashes() const;
  std::vector<int> getFilterHashes() const;
  void setFilterHashes(std::vector<int> nbrHashes);
  size_t getCurFilterSize() const;
  size_t getBloomSize() const; 
  std::vector<bool> getSubFilter(size_t index);
  void setSubFilter(std::vector<bool>);
  std::vector<std::vector<bool>> getFilterList();

  //Auxiliary Functions
  void initialize(double fp);
  //Check whether an Element is included, receives the Index of the filter
  //the hashvalues contained in hashResults are for. 
  //Hashing should be object internal, instead of the current method 
  //in the operators. Would need a refactor.
  bool contains(std::vector<size_t> hashResults, int filterIndex) const;
  //Sets the bits in the current filter for all indexes present in 
  //hashresults
  void add(std::vector<size_t> hashResults);
  //Checks whether a filter has processed more unique elements 
  //than it was created for
  bool isSaturated();
  //Calculates the size of the current subfilter considering 
  //the demanded p(FP) and the size of objects this 
  //should be created for
  size_t optimalSize (const long maxInserts, const double fPProb);
  //Calculates the optimal number of hashes for the current subfilter
  //considering its size and the number of inserts it is created for
  long optimalHashes (const long maxInserts, const long filterSize);
  //Called when a new subfilter is added, because the old one is saturated.
  //Updates the parameters by applying growth rate, tightening ratio, etc. 
  //I do not know whether it is possible in C++ but in Java i would have
  //created a bloomfilter which has bloomfilters as Member variable 
  //mb this is something a refactoring could do here as well
  void updateFilterValues(); 
  size_t Sizeof() const;

  //Support Functions
  static Word     In( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );
  
  static ListExpr Out( ListExpr typeInfo, Word value );

  //Storage Record
  static Word     Create( const ListExpr typeInfo );

  static void     Delete( const ListExpr typeInfo, Word& w );

  static bool     Open(SmiRecord& valueRecord, size_t& offset, 
                         const ListExpr typeInfo, Word& value);
  
  static void     Close( const ListExpr typeInfo, Word& w );

  static bool     Save(SmiRecord & valueRecord , size_t & offset,
                       const ListExpr typeInfo , Word & value);
  
  static Word     Clone( const ListExpr typeInfo, const Word& w );

  static bool     KindCheck( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObj();

  static ListExpr Property();


   static const std::string BasicType() {
    return "bloomfilter";
  }

  static const bool checkType (const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }

  private:
    ScalableBloomFilter() {
    }
    friend struct  ConstructorFunctions<ScalableBloomFilter>;

    //Constants for Filter Creation
    //Size of Elements we create the filter for
    int DEFAULT_SIZE= 4096;
    //Growth rate for the subsequent subfilter
    int GROWTH_RATE = 2; 
    //Tightening Ratio of the errorbound
    double TIGHTENING_RATIO = 0.8;
    
    bool defined;
    //Number of hash functions the current subfilter has 
    //calculated as optimum
    int numHashfunctions;
    //User parameter
    double falsePositiveProbability;
    //FP for the current subfilter
    //for the first filter this equals 
    //falsePositiveProbablity
    double rollingFP;
    //The mximum number of inserts the current subfilter can
    //handle. 4096 for the first filter
    size_t maxInserts;
    //Number of bits in the current subfilter (vector) in
    //our case. Has to be saved for the modulo reduction
    size_t filterSize;
    //The amount of distinct elements we inserted into the 
    //current subfilter
    size_t currentInserts;
    //Currently active subfilter
    int curFilterIndex;
    //Saves vector[i] saves how many hashes are needed for 
    //the ith subfilter
    std::vector<int> ithFilterHashes;
    //The actual bloomfilter consisting of a list of 
    //subfilters
    std::vector<std::vector<bool>> filterList;
  };
}
#endif