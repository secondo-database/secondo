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
#include "NestedList.h"
#include "ListUtils.h"
#include "AlgebraTypes.h"

namespace eschbach {
    class ScalableBloomFilter
{
  public:
  ScalableBloomFilter(const float inputErrorRate, const size_t expectedInput);
  ScalableBloomFilter(const ScalableBloomFilter& rhs);
  ~ScalableBloomFilter(){}

  //Getter und Setter
  bool getDefined() const;
  void setDefined();
  size_t getInserts() const; 
  float getFP() const;
  bool getElement(size_t index) const;
  int getNumberHashes() const;
  size_t getFilterSize() const;
  std::vector<bool> getFilter() const;
  void setFilter(std::vector<bool> filter); 

  //Auxiliary Functions
  void initialize(float fp, size_t entries);
  bool contains(std::vector<size_t> hashResults) const;
  void add(std::vector<size_t> hashResults);
  size_t optimalSize (const long expectedInserts, const double fPProb);
  long optimalHashes (const long expectedInserts, const long filterSize);

  //Support Functions
  static Word     In( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );
  
  static ListExpr Out( ListExpr typeInfo, Word value );

  //Storage Record
  static Word     Create( const ListExpr typeInfo );

  static void     Delete( const ListExpr typeInfo, Word& w );

  /*static void     Open(SmiRecord& valueRecord, size_t& offset, 
                         const ListExpr typeInfo, Word& value);
  */
  static void     Close( const ListExpr typeInfo, Word& w );

  /*static bool     Save(SmiRecord & valueRecord , size_t & offset,
                       const ListExpr typeInfo , Word & value);
  */
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
    ScalableBloomFilter() {}
    friend struct  ConstructorFunctions<ScalableBloomFilter>;


    bool defined;

    //int DEFAULT_SIZE= 4096;
    //float MAX_FILL = 0.5f;
    //int HASH_SEED = 123;
    //int GROWTH_RATE = 2; 
    //float TIGHTENING_RATIO = 0.8;
    //int BITS_PER_ENTRY = 64;
    //long maxBits; 
    
    int numHashfunctions;
    float falsePositiveProbability;
    size_t expectedInserts;
    size_t filterSize;
    std::vector<bool> filter;

    //int insertions;

    //arrayalgebra::Array test1;
    //pointcloud2::BitArray test2;
    //ScalableBloomFilter* subFilter;
    //std::vector<std::vector<bool>> filterList;
};

}
