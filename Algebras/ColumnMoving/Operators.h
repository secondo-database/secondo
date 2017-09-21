/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

1 Operators.h

*/

#pragma once

#include "AttrArrayOperator.h"

namespace ColumnMovingAlgebra
{

/*
1.1 Declaration of the class PresentOperator

The following declarations of operators are very similar. They consist of
a ~OperatorInfo~ structure which contains information for the user interface,
a list of value mapping functions, a type mapping function and value mapping
function, a function that returns all signatures of the operator and finally 
the value mapping functions. 

*/

  class PresentOperator : public AttrArrayOperator
  {
  public:
    
    PresentOperator() : AttrArrayOperator(info, valueMappings, 
                                          SelectValueMapping, TypeMapping) {}
                                          
  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static std::list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping00(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping01(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping10(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping11(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping20(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping21(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping30(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping31(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping40(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping41(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping50(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping51(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };

/*
1.2 Declaration of the class AtInstantOperator

*/
 
  class AtInstantOperator : public AttrArrayOperator
  {
  public:
    
    AtInstantOperator() : AttrArrayOperator(info, valueMappings,  
                                            SelectValueMapping, TypeMapping) {}

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static std::list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping1(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping2(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping3(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping4(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping5(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };

/*
1.3 Declaration of the class AtPeriodsOperator

*/
 
  class AtPeriodsOperator : public AttrArrayOperator
  {
  public:
    
    AtPeriodsOperator() : AttrArrayOperator(info, valueMappings,  
                                            SelectValueMapping, TypeMapping) {}

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static std::list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping1(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping2(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping3(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping4(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping5(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
  };

/*
1.4 Declaration of the class PassesOperator

*/
 
  class PassesOperator : public AttrArrayOperator
  {
  public:
    
    PassesOperator() : AttrArrayOperator(info, valueMappings,  
                                         SelectValueMapping, TypeMapping) {}

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static std::list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping00(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping01(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping10(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping11(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping20(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping21(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping30(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping31(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping40(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping41(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };

/*
1.5 Declaration of the class AtOperator

*/
 
  class AtOperator : public AttrArrayOperator
  {
  public:
    
    AtOperator() : AttrArrayOperator(info, valueMappings, SelectValueMapping,  
                                     TypeMapping) {}

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static std::list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping00(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping01(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping10(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping11(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping20(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping21(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping30(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping31(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping40(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping41(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };

/*
1.6 Declaration of the class InsideOperator

*/
 
  class InsideOperator : public Operator
  {
  public:
    
    InsideOperator() : Operator(info, valueMappings, SelectValueMapping,  
                                TypeMapping) {}

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static int mapping(ListExpr args);
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping1(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping2(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping3(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
  };

/*
1.7 Declaration of the class IntersectionOperator

*/
 
  class IntersectionOperator : public Operator
  {
  public:
    
    IntersectionOperator() : Operator(info, valueMappings, SelectValueMapping,  
                                TypeMapping) {}

    static int mapping(ListExpr args);

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping1(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping2(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping3(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
  };

/*
1.8 Declaration of the class AddRandomOperator

*/
 
  class AddRandomOperator : public AttrArrayOperator
  {
  public:
    
    AddRandomOperator() : AttrArrayOperator(info, valueMappings,  
                                            SelectValueMapping, TypeMapping) {}

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static std::list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping1(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping2(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping3(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping4(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };

/*
1.9 Declaration of the class IndexOperator

The index operator is implemented differently, as it has a more complex
signature.

*/
 
  class IndexOperator : public Operator
  {
  public:
    
    IndexOperator() : Operator(info, valueMappings,  
                               SelectValueMapping, TypeMapping) {}

  private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };
  
  
  
  
  


}
