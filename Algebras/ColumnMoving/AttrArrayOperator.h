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

1 AttrArrayOperator.h

The class AttrArrayOperator is the base class for the operators present,
atinstant, atperiods, passes, at und addrandom. It implements a generic
type mapping and select value mapping.

*/

#pragma once

#include <cassert>
#include <string>
#include <list>
#include "NestedList.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Algebras/CRel/AttrArray.h"

extern NestedList *nl;
extern QueryProcessor *qp;

namespace ColumnMovingAlgebra
{

/*

1.1 Declaration

The struct AttrArrayOperatorSignatur contains information of one signature
for a operator.

*/
  struct AttrArrayOperatorSignatur {
    std::string operandTypeA;
    std::string operandTypeB;
    ListExpr resultType;
  };
  
  class AttrArrayOperator : public Operator {
  public:
    using Operator::Operator;
    
/*
~typeMapping~ and ~selectValueMapping~ are generic functions for type mapping 
and select value mapping. the function ~mapping~ is a common helper method for 
both functions.

*/    
    
    static bool mapping(
      std::list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args, int & signatureId, ListExpr & resultType, 
      std::string & error);
    static ListExpr typeMapping(
      std::list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args);
    static int selectValueMapping(
      std::list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args);
  };
  
/*

1.2 Implementation

The helper function ~mapping~ searchs a matching signature for ~args~ from 
the list ~signatures~. if successful ~signatureId~ is set to the 
corresponding index and ~resultType~ to the corresponding result type and
the return value is true. If not successful it sets ~error~ to a error 
message and returns false.

*/
  
  inline bool AttrArrayOperator::mapping(
      std::list<AttrArrayOperatorSignatur> signatures, 
      ListExpr args, int & signatureId, ListExpr & resultType, 
      std::string & error)
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
  
/*
generic type mapping function

*/  

  inline ListExpr AttrArrayOperator::typeMapping(
    std::list<AttrArrayOperatorSignatur> signatures, ListExpr args)
  {
    int signatureId;
    ListExpr resultType;
    std::string error;
    if (!mapping(signatures, args, signatureId, resultType, error))
      return NList::typeError(error);
      
    return resultType;
  }

/*
generic select value mapping function

*/  

  inline int AttrArrayOperator::selectValueMapping(
    std::list<AttrArrayOperatorSignatur> signatures, ListExpr args)
  {
    int signatureId;
    ListExpr resultType;
    std::string error;
    bool success = mapping(signatures, args, signatureId, resultType, error);
    assert(success);
    return signatureId;
  }

}
