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
#include "Attribute.h"
#include "StandardTypes.h"
#include "counterPair.h"


namespace eschbach {
    class lossyCounter
{
  public:
  lossyCounter(const float epsilon);
  lossyCounter(const lossyCounter& rhs);
  ~lossyCounter() {}


  //Getter und Setter
  void setDefined(bool value); 
  bool getDefined(); 
  size_t getEleCounter(); 
  float getEpsilon(); 
  long getCurrentWindowIndex();
  int getWindowSize();
  int getElement(int index);

  //Auxiliary Functions
  void initialize(const float epsilon);
  void incrCount(int element); 
  void addElement(int element);
  void insertElement(int element);
  bool elementPresent(int element);
  void reduce();
  void updateWindowIndex(); 
  std::vector<int> getFrequentElements(double minSupport);
  long estimateCount(int element);


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
    return "lossycounter";
  }

  static const bool checkType (const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }

  private:
    lossyCounter() {}
    friend struct ConstructorFunctions<lossyCounter>;
    bool defined;
    float epsilon; 
    size_t eleCounter; 
    int windowSize; 
    long windowIndex;
    std::unordered_map<int, counterPair> frequencyList;
  };

}