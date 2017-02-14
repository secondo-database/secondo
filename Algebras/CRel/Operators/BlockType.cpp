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

#include "BlockType.h"

#include "AttrArrayTI.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include <string>

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::string;

extern NestedList *nl;

BlockType::BlockType() :
  Operator(info, NULL, TypeMapping)
{
}

const OperatorInfo BlockType::info = OperatorInfo(
  "BLOCKTYPE", "",
  "",
  "",
  "");

ListExpr BlockType::TypeMapping(ListExpr args)
{
  if(!nl->IsEmpty(args))
  {
    const ListExpr firstArg = nl->First(args);

    if (!nl->IsAtom(firstArg) && nl->HasLength(firstArg, 2))
    {
      if (nl->IsEqual(nl->First(firstArg), Symbols::STREAM()))
      {
        //Recursion if stream-type
        return TypeMapping(nl->Second(firstArg));
      }
    }

    string typeError;
    if (!AttrArrayTI::Check(firstArg, typeError))
    {
      return listutils::typeError("Expected (stream of) attrblock.: " +
                                  typeError);
    }

    //Return Attribute-Type
    return AttrArrayTI(firstArg).GetAttributeType();
  }

  return listutils::typeError("Expected at least one argument.");
}