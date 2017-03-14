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

1.1 Count

*/

#include "BlockingProgressEstimator.h"
#include "Include.h"


using namespace nr2a;

Count::~Count()
{
}

/*
"Count"[2] accepts "nrel2"[2] and "arel2"[2] as input and always outputs an
"int"[2].

*/
/*static*/ListExpr Count::MapType(ListExpr args)
{
  if (nl->HasLength(args, 1))
  {
    if (Nr2aHelper::IsNestedRelation(nl->First(args)))
    {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }
  return listutils::typeError("Expecting nrel2 or arel2 as input");
}

ValueMapping Count::functions[] = { CountArel, CountNrel, NULL };

/*static*/int Count::SelectFunction(ListExpr args)
{
  return Nr2aHelper::DefaultSelect(nl->First(args));
}

/*
Both types of relations offer an function to request their amount of tuples.

*/
/*static*/int Count::CountArel(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  int resultInt = 0;
  //if (message == OPEN)
  {
    ARel *arel = static_cast<ARel*>(args[0].addr);

    result = qp->ResultStorage(s);
    CcInt* resultValue = static_cast<CcInt*>(result.addr);
    int resultInt = arel->GetTupleCount();

    resultValue->Set(true, resultInt);
  }
  return resultInt;
}

/*static*/int Count::CountNrel(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  int resultInt = 0;
  //if (message == OPEN)
  {
    NRel *nrel = static_cast<NRel*>(args[0].addr);

    result = qp->ResultStorage(s);
    CcInt* resultValue = static_cast<CcInt*>(result.addr);
    int resultInt = nrel->GetTupleCount();

    resultValue->Set(true, resultInt);
  }
  return resultInt;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation Count::costEstimators[] =
  { BlockingProgressEstimator::Build, BlockingProgressEstimator::Build };
