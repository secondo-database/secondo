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

# define LONG_PRIME 4294967311l

namespace eschbach {
    class amsSketch
{
  public:
  amsSketch(const float epsilon, const float delta);
  amsSketch(const amsSketch& rhs);
  ~amsSketch() {}


  //Getter und Setter
  bool getDefined();
  size_t getWidth();
  size_t getDepth();
  float getEpsilon();
  float getDelta();
  size_t getTotalCount();
  int getElement(int counterNumber, int elementIndex);
  void setElement(int counterNumber, int elementIndex, int value);
  int getConstantTwA(int index);
  int getConstantTwB(int index);
  int getConstantFwA(int index);
  int getConstantFwB(int index);  
  int getConstantFwC(int index);
  int getConstantFwD(int index);


  std::vector<std::vector<long>> getConstantsTw();
  void setConstantsTw(int counterNumber, long a, long b);
  std::vector<std::vector<long>> getConstantsFw();
  void setConstantsFw(int counterNumber, long a, long b, long c, long d);
  std::vector<std::vector<int>> getMatrix();

  //Auxiliary Functions
  void initialize(const float epsilon, const float delta);
  void generateConstants(int index);
  void generateFwConstants(int index);
  void changeWeight(size_t value);
  int estimateInnerProduct();

  void swap(int* a, int* b);
  int partition(int arr[], int l, int r);
  int randomPartition(int arr[], int l, int r);
  void medianDecider(int arr[], int l, int r, int k, int& a, int& b);
  int findMedian(int arr[]);

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
    return "amsSketch";
  }

  static const bool checkType (const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }

  private:
    amsSketch() {}
    friend struct ConstructorFunctions<amsSketch>;
    bool defined;
    float eps;
    float delta; 
    size_t width;
    size_t depth;
    size_t totalCount;
    std::vector<std::vector<int>> matrix;
    std::vector<std::vector<long>> twConstants;
    std::vector<std::vector<long>> fwConstants;
  };

}
