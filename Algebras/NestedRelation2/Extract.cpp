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

#include "BlockingProgressEstimator.h"
#include "Include.h"


using namespace nr2a;
using namespace std;

Extract::~Extract()
{
}

/*
The first argument is expected to be an input stream and the second argument
must be the name of an attribute of the first one. The resulting type is the
one of the mentioned attribute.

*/
/*static*/ListExpr Extract::MapType(ListExpr args)
{
  AutoWrite(args);
  ListExpr result = nl->TheEmptyList();
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Two arguments expected");
  }
  ListExpr inStreamType = nl->First(args);
  ListExpr attributeToExtract = nl->Second(args);
  if (!listutils::isTupleStream(inStreamType))
  {
    return listutils::typeError(
        "The first argument is expected to be an input stream");
  }
  string attributeUnnestName = "";
  if (listutils::isSymbol(attributeToExtract))
  {
    ListExpr type = nl->Empty();
    attributeUnnestName = nl->SymbolValue(attributeToExtract);
    if (listutils::findAttribute(nl->Second(nl->Second(inStreamType)),
        attributeUnnestName, type) == 0)
    {
      return listutils::typeError("The attribute mentioned in the "
          "second argument does not exist in input stream");
    }
    else
    {
      if (nl->IsEqual(nl->First(type), ARel::BasicType()))
      {
        result = type;
      }
      else
      {
        return listutils::typeError("The attribute mentioned in the "
            "second argument is not a nested relation");
      }
    }
  }
  else
  {
    return listutils::typeError("The second argument must be the name "
        "of an attribute of the first argument");
  }

  return result;
}

/*static*/ValueMapping Extract::functions[] = { ExtractValue, NULL };

/*static*/int Extract::SelectFunction(ListExpr args)
{
  return 0;
}

/*
The operator consumes ther first tuple of the stream and ~extracts~ the value
of the given attribute.

*/
/*static*/int Extract::ExtractValue(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  int resultInt = 0;
  if (message == OPEN)
  {
    Word actual;

    ListExpr attributeTypes = nl->Second(
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 0))));
    string subRelName = nl->SymbolValue(
        qp->GetSupplierTypeExpr(qp->GetSon(s, 1)));

    ARel* arel = (ARel*) ((qp->ResultStorage(s)).addr);
    if (arel->GetTupleCount() > 0)
    {
      assert(false);
    }

    Tuple* tuple;
    ListExpr type;
    int attributeIndex = listutils::findAttribute(attributeTypes, subRelName,
        type);
    Stream<Tuple> stream(args[0]);
    stream.open();

    tuple = stream.request();
    if (tuple != NULL)
    {
      arel->CopyFrom(static_cast<ARel*>
        (tuple->GetAttribute(attributeIndex)));
      tuple->DeleteIfAllowed();
    }
    else
    {
      arel->SetDefined(false);
    }
    stream.close();

    result.setAddr(arel);
  }
  return resultInt;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation Extract::costEstimators[] =
  { BlockingProgressEstimator::Build };
