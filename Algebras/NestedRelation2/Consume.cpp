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

Consume::~Consume()
{
}

/*static*/ListExpr Consume::MapType(ListExpr args)
{
  ListExpr result = nl->TypeError();
  if ((!nl->HasLength(args, 1)) ||
      (!listutils::isTupleStream(nl->First(args))))
  {
    return listutils::typeError("Expecting a tuple stream as input");
  }
  //ListExpr attributesTypes = nl->Second(nl->Second(nl->First(args)));
  //AutoWrite(attributesTypes);
  //bool arelFound = false;
  //listForeach(attributesTypes, current)
  //{
  //  const ListExpr currentType = nl->Second(current);
  //  if (nl->HasMinLength(currentType, 2)
  //      && listutils::isSymbol(nl->First(currentType), ARel::BasicType()))
  //  {
  //    arelFound = true;
  //    break;
  //  }
  //}
  //if (!arelFound)
  //{
  //  return listutils::typeError("Input stream is expected to have at "
  //      "least one attribute of type arel2");
  //}
  result = nl->TwoElemList(nl->SymbolAtom(NRel::BasicType()),
      nl->Second(nl->First(args)));
  return result;
}

/*static*/int Consume::SelectFunction(ListExpr args)
{
  return 0;
}

/*static*/ValueMapping Consume::functions[] = { ConsumeValue, NULL };

/*
All provided tuples are added to the resulting nested relation.

*/
/*static*/int Consume::ConsumeValue(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  int resultInt = 0;
  NRel* nrel = (NRel*) (qp->ResultStorage(s).addr);
  Stream<Tuple> *stream = (Stream<Tuple> *) local.addr;
  switch (message)
  {
    case OPEN:
    case REQUEST:
      if (stream != NULL)
      {
        delete stream;
      }
      stream = new Stream<Tuple>(args[0]);
      local.addr = stream;
      //break;

      stream->open();
      Tuple* tuple;
      while ((tuple = stream->request()) != 0)
      {
        nrel->AppendTuple(tuple);
      }
      stream->close();

      result.setAddr(nrel);
      //break;

    //case CLOSE:
      delete stream;
      break;
  }
  return resultInt;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation Consume::costEstimators[] =
  { BlockingProgressEstimator::Build };
