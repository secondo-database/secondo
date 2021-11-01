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

#ifndef COUNTMINSKETCH_H
#define COUNTMINSKETCH_H

#include "NestedList.h"
#include "ListUtils.h"
#include "AlgebraTypes.h"

# define LONG_PRIME 4294967311l

namespace eschbach {
    class CountMinSketch
{
  public:
  CountMinSketch(const double epsilon, const double delta);
  CountMinSketch(const CountMinSketch& rhs);
  ~CountMinSketch() {}


  //Getter und Setter
  bool getDefined();
  size_t getWidth();
  size_t getDepth();
  double getEpsilon();
  double getDelta();
  size_t getTotalCount();
  void setTotalCount(size_t count);
  int getElement(int counterNumber, int elementIndex);
  void setElement(int counterNumber, int elementIndex, int value);
  long getConstantA(int index);
  long getConstantB(int index);
  void setConstants(int counterNumber, long a, long b);
  std::vector<std::vector<long>> getConstants();
  std::vector<std::vector<int>> getMatrix();

  //Auxiliary Functions
  void initialize(const double epsilon, const double delta);
  //Generates the constants for every rows hashfunction
  void generateConstants(int index);
  //Increases an elements counter
  void increaseCount(long hashedEleValue);
  //Gets the counter of an element
  int estimateFrequency(long hashedEleValue);

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
    return "countminsketch";
  }

  static const bool checkType (const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }

  private:
    CountMinSketch() {}
    friend struct ConstructorFunctions<CountMinSketch>;
    bool defined;
    //Errorbound
    double eps;
    //Inverse Probability of the answers being
    //within the errorbound
    double delta; 
    //Width of the Matrix
    size_t width;
    //Depth of the Matrix
    size_t depth;
    //Amount of elements seen
    size_t totalCount;
    //2d structure for the counters
    std::vector<std::vector<int>> matrix;
    //Vector for saving the hashConstants calculated
    //for each row of the matrix
    std::vector<std::vector<long>> hashConstants;
  };
}
#endif