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

#include "AttributeType.h"

#include "AttrArray.h"
#include "LogMsg.h"
#include "Symbols.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using listutils::isStream;

extern NestedList *nl;

AttributeType::AttributeType() :
  Operator(info, nullptr, TypeMapping)
{
}

const OperatorInfo AttributeType::info = OperatorInfo(
  "ATTRIBUTETYPE", "ATTRARRAY -> DATA",
  "type operator",
  "Determines the attribute type of the entries of a attribute array.",
  "Not for use with sos-syntax");

ListExpr AttributeType::TypeMapping(ListExpr args)
{
  if(nl->IsEmpty(args))
  {
    return listutils::typeError("Expected at least one argument.");
  }

  const ListExpr arg = nl->First(args);

  if (isStream(arg))
  {
    //Recursion if stream-type
    return TypeMapping(GetStreamType(arg));
  }

  if (!CheckKind(Kind::ATTRARRAY(), arg))
  {
    return listutils::typeError("Expected argument of kind ATTRARRAY.");
  }

  AttrArrayTypeConstructor &constructor =
    (AttrArrayTypeConstructor&)*GetTypeConstructor(arg);

  //Return Attribute-Type
  return constructor.GetAttributeType(arg, false);
}