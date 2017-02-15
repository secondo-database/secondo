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

TypeOf::~TypeOf()
{
}
/*
The operator accepts only a single argument of any type. The value mapping is
not used.

*/
/*static*/ListExpr TypeOf::MapType(ListExpr args)
{
  ListExpr type = nl->First(args);
  string out;
  nl->WriteToString(out, type);
  //cerr << "**\n**\n** type mapping: " << out << "\n**\n**\n" << endl;
  return
      listutils::typeError("\n*** type of expression is :\n" + out + "\n***");
}

ValueMapping TypeOf::functions[] = { MapValue, NULL };

/*static*/int TypeOf::SelectFunction(ListExpr args)
{
  return 0;
}

/*static*/int TypeOf::MapValue(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  assert(false);
  return 0;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation TypeOf::costEstimators[] =
  { BlockingProgressEstimator::Build };
