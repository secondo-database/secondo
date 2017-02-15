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

#include "Include.h"

using namespace nr2a;

Aconsume::~Aconsume()
{
}

/*static*/ListExpr Aconsume::MapType(ListExpr args)
{
  ListExpr result = nl->TypeError();
  if ((nl->ListLength(args) == 1) && listutils::isStream(nl->First(args)))
  {
    result = nl->TwoElemList(nl->SymbolAtom(ARel::BasicType()),
        nl->Second(nl->First(args)));
  }
  return result;
}

ValueMapping Aconsume::functions[] = { AconsumeValue, NULL };

/*static*/int Aconsume::SelectFunction(ListExpr args)
{
  return 0;
}

/*
All provided tuples are added to the resulting attribute relation.

*/
/*static*/int Aconsume::AconsumeValue(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  Word w = qp->ResultStorage(s);
  ARel* arel = (ARel*) (w.addr);
  arel->Clear();

  Stream<Tuple> stream(args[0]);

  stream.open();
  Tuple* tup;
  while ((tup = stream.request()) != NULL)
  {
    arel->AppendTuple(tup);
  }
  stream.close();

  result.setAddr(arel);
  return 0;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation Aconsume::costEstimators[] =
  { BlockingProgressEstimator::Build };

