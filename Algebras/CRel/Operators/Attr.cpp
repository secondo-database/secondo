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
#include <cstddef>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <string>
#include "Symbols.h"
#include "TBlock.h"
#include "TBlockTI.h"

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
    return listutils::typeError("Expected two arguments!");
  }

  //Check the first argument for 'tblock' type
  ListExpr tblockType = nl->First(nl->First(args));
  string tblockTypeError;

  if (!TBlockTI::Check(tblockType, tblockTypeError))
  {
    return listutils::typeError("First argument isn't a tblock: " +
                                tblockTypeError);
  }

  //Check the second argument for 'symbol' type
  ListExpr attributeNameExpr = nl->Second(nl->Second(args));

  if (!nl->IsNodeType(SymbolType, attributeNameExpr))
  {
    return listutils::typeError("Second argument (attribute name) isn't a "
                                "symbol.");
  }

  //Find the column with matching name
  const string attributeName = nl->SymbolValue(attributeNameExpr);
  const TBlockTI tblockInfo(tblockType, false);
  size_t index = 0;

  for (const TBlockTI::ColumnInfo &columnInfo : tblockInfo.columnInfos)
  {
    if (columnInfo.name == attributeName)
    {
      //Append the matching column's index
      //Result type is the matching column's type
      return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                               nl->OneElemList(nl->IntAtom(index)),
                               columnInfo.type);
    }

    ++index;
  }

  return listutils::typeError("Second argument (attribute name) didn't match "
                              "any in the tblock's attribute list.");
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