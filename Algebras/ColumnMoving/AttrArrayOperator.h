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

#include <cassert>
#include <string>
#include <list>
//#include "AlgebraTypes.h"
#include "NestedList.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "AttrArray.h"

using namespace std;

extern NestedList *nl;
extern QueryProcessor *qp;


namespace ColumnMovingAlgebra
{
  struct AttrArrayOperatorSignatur {
    string operandTypeA;
    string operandTypeB;
    ListExpr resultType;
  };
  
  class AttrArrayOperator : public Operator {
  public:
    using Operator::Operator;
    
    static bool mapping(list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args, int & signatureId, ListExpr & resultType, string & error);
    static ListExpr typeMapping(list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args);
    static int selectValueMapping(list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args);
  };
  
  inline bool AttrArrayOperator::mapping(
      list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args, int & signatureId, ListExpr & resultType, string & error)
  {
    if(!nl->HasLength(args,2)) {
      error = "Two arguments expected.";
      return false;
    }

    const ListExpr firstArg = nl->First(args);

    CRelAlgebra::AttrArrayTypeConstructor *typeConstructorA =
      CRelAlgebra::AttrArray::GetTypeConstructor(firstArg);

    if (typeConstructorA == nullptr) {
      error = "First Argument isn't of kind ATTRARRAY.";
      return false;
    }

    const ListExpr attributeType = typeConstructorA->GetAttributeType(firstArg,
                                                                        false);

    const ListExpr secondArg = nl->Second(args);

    int i = -1;
    for (auto & s : signatures) {
      i++;
      
      if (!nl->IsEqual(attributeType, s.operandTypeA))
        continue;

      if (!nl->IsEqual(secondArg,     s.operandTypeB))
        continue;
        
      signatureId = i;
      resultType = s.resultType;
      return true;
    }
    
    error = "\nExpected one of the following operands:\n";
    for (auto & s : signatures) {
      error += "ATTRARRAY(";
      error += s.operandTypeA;
      error += ") x ";
      error += s.operandTypeB;
      error += "\n";
    }
    
    return false;
  }

  inline ListExpr AttrArrayOperator::typeMapping(
    list<AttrArrayOperatorSignatur> signatures, ListExpr args)
  {
    int signatureId;
    ListExpr resultType;
    string error;
    if (!mapping(signatures, args, signatureId, resultType, error))
      return NList::typeError(error);
      
    return resultType;
  }

  inline int AttrArrayOperator::selectValueMapping(
    list<AttrArrayOperatorSignatur> signatures, ListExpr args)
  {
    int signatureId;
    ListExpr resultType;
    string error;
    bool success = mapping(signatures, args, signatureId, resultType, error);
    assert(success);
    return signatureId;
  }

}
