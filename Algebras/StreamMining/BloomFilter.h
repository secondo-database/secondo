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
  bool contains(std::vector<size_t> hashResults, int filterIndex) const;
  void add(std::vector<size_t> hashResults);
  bool isSaturated();
  size_t optimalSize (const long maxInserts, const double fPProb);
  long optimalHashes (const long maxInserts, const long filterSize);
  void updateFilterValues(); 

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
      cout << "Used Scalalble Constructor without Param" << endl;
    }
    friend struct  ConstructorFunctions<ScalableBloomFilter>;

    //Constants for Filter Creation
    int DEFAULT_SIZE= 64;
    int GROWTH_RATE = 2; 
    double TIGHTENING_RATIO = 0.8;
    
    bool defined;
    int numHashfunctions;
    double falsePositiveProbability;
    double rollingFP;
    size_t maxInserts;
    size_t filterSize;
    size_t currentInserts;
    int curFilterIndex;
    std::vector<int> ithFilterHashes;
    std::vector<std::vector<bool>> filterList;
  };
}
#endif