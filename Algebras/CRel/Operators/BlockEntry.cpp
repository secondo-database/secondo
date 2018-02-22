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

#include "BlockEntry.h"

#include "AttrArray.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "Symbols.h"
#include "../TypeConstructors/TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

extern NestedList *nl;

BlockEntry::BlockEntry() :
  Operator(info, nullptr, TypeMapping)
{
}

const OperatorInfo BlockEntry::info = OperatorInfo(
  "BLOCKENTRY", "ATTRARRAY -> DATA or tblock -> tuple",
  "type operator",
  "Determines the attribute type of the entries of a (stream of) attribute "
  "array or the tuple type of a (stream of) tuple block.",
  "Not for use with sos-syntax");

ListExpr BlockEntry::TypeMapping(ListExpr args)
{
  if(!nl->HasLength(args, 1))
  {
    return GetTypeError("Expected one argument.");
  }

  ListExpr arg = nl->First(args);

  if (TBlockTI::Check(arg))
  {
    return TBlockTI(arg, false).GetTupleTypeExpr();
  }

  //Either stream or nonstream
  arg = GetStreamType(arg, true);

  AttrArrayTypeConstructor *constructor = AttrArray::GetTypeConstructor(arg);

  if (constructor == nullptr)
  {
    return GetTypeError(0, "type", "Not (stream) of type tblock or kind "
                      "ATTRARRAY.");
  }

  //Return Attribute-Type
  return constructor->GetAttributeType(arg, false);
}
