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

#include "Attr.h"

#include "AttrArray.h"
#include <cstdint>
#include <exception>
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <string>
#include "Symbols.h"
#include "TBlock.h"
#include "TBlockTC.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

Attr::Attr() :
  Operator(info, ValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo Attr::info = OperatorInfo(
  "attr", "tblock x symbol -> ATTRARRAY",
  "attr(_, _)",
  "Returns the column of the tuple block which has the given name.",
  "query people feed filter[attr(., Age) > 50] count");

ListExpr Attr::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if (!nl->HasLength(args, 2))
  {
    return GetTypeError("Expected two arguments!");
  }

  //Check the first argument for 'tblock' type
  ListExpr tblockType = nl->First(nl->First(args));

  if (!TBlockTI::Check(tblockType))
  {
    return GetTypeError(0 , "block", "Not of type tblock.");
  }

  //Check the second argument for 'symbol' type
  ListExpr columnNameExpr = nl->Second(nl->Second(args));

  if (!nl->IsNodeType(SymbolType, columnNameExpr))
  {
    return GetTypeError(1, "column name", "Not of type symbol.");
  }

  //Find the column with matching name
  const string columnName = nl->SymbolValue(columnNameExpr);
  const TBlockTI tblockInfo(tblockType, false);
  uint64_t index = 0;

  for (const TBlockTI::ColumnInfo &columnInfo : tblockInfo.columnInfos)
  {
    if (columnInfo.name == columnName)
    {
      //Append the matching column's index
      //Result type is the matching column's type
      return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                               nl->OneElemList(nl->IntAtom(index)),
                               columnInfo.type);
    }

    ++index;
  }

  return GetTypeError(1, "column name", "No such column.");
}

int Attr::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    const TBlock &block = *(TBlock*)args[0].addr;
    const CcInt &attributeIndex = *(CcInt*)args[2].addr;

    //Don't want to return a new instance so delete it
    qp->DeleteResultStorage(s);

    AttrArray *array = &block[attributeIndex.GetValue()];
    array->IncRef();

    //Return the pointer to the column
    //Let the delete function only decrease the ref-count
    qp->ChangeResultStorage(s, result = Word(array));
    qp->SetDeleteFunction(s, [](const ListExpr, Word &value)
    {
      ((AttrArray*)value.addr)->DecRef();
    });

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}