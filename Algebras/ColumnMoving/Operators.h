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

*/

#pragma once

#include "AttrArrayOperator.h"

namespace ColumnMovingAlgebra
{
  class PresentOperator : public AttrArrayOperator
  {
    public:
    
    PresentOperator() : AttrArrayOperator(info, valueMappings, 
                                          SelectValueMapping, TypeMapping) {}
                                          
    private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static list<AttrArrayOperatorSignatur> signatures();
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
  };
  
  class AtInstantOperator : public AttrArrayOperator
  {
    public:
    
    AtInstantOperator() : AttrArrayOperator(info, valueMappings,  
                                            SelectValueMapping, TypeMapping) {}

    private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping1(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };
  
  class AtPeriodsOperator : public AttrArrayOperator
  {
    public:
    
    AtPeriodsOperator() : AttrArrayOperator(info, valueMappings,  
                                            SelectValueMapping, TypeMapping) {}

    private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
    static int ValueMapping1(ArgVector args, Word &result, int message, 
                             Word &local, Supplier s);
  };
  
  class PassesOperator : public AttrArrayOperator
  {
    public:
    
    PassesOperator() : AttrArrayOperator(info, valueMappings,  
                                         SelectValueMapping, TypeMapping) {}

    private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping10(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
    static int ValueMapping11(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };
  
  class AtOperator : public AttrArrayOperator
  {
    public:
    
    AtOperator() : AttrArrayOperator(info, valueMappings, SelectValueMapping,  
                                     TypeMapping) {}

    private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static list<AttrArrayOperatorSignatur> signatures();
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
  };
  
  class AddRandomOperator : public AttrArrayOperator
  {
    public:
    
    AddRandomOperator() : AttrArrayOperator(info, valueMappings,  
                                            SelectValueMapping, TypeMapping) {}

    private:
    
    static const OperatorInfo info;
    static ValueMapping valueMappings[];
    
    static list<AttrArrayOperatorSignatur> signatures();
    static ListExpr TypeMapping(ListExpr args);
    static int SelectValueMapping(ListExpr args);
    static int ValueMapping0(ArgVector args, Word &result, int message, 
                              Word &local, Supplier s);
  };
  
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
