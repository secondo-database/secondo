/*
----
This file is part of SECONDO.

Copyright (C) 2020,
Faculty of Mathematics and Computer Science,
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


//[$][\$]
//[_][\_]

*/

#include "tsop.h"
using std::string;

TsOp::TsOp()
{

}


 // @brief TsOp::checkTimeSeriesType checks whether the
 //ListExpr satisfies the conditions to
 // be handled as a timeseries.
 // @param timeseries Listexpression to check
 // @return error or a copy of the original input

ListExpr TsOp::checkTimeSeriesType(ListExpr& timeseries)
{
    string error = "Timeseries exspected";

    NList timeserie = NList(timeseries);

    NList typeList = timeserie.second().second();

    if(!OrderedRelation::checkType(timeseries)){
        return listutils::typeError(error +
                                    " (first argument should be a ordered "
                                    "relation representing a timeseries.");
    }
    if(!listutils::isTupleDescription(nl->Second(timeseries))){
        return listutils::typeError((error +
                                     " (Tuple description should be valid!)"));
    }

    ListExpr rest = typeList.listExpr();

    ListExpr timestamp = nl->First(rest);
    rest = nl->Rest(rest);

    if(!Instant::checkType(nl->Second(timestamp)))
    {
        return listutils::typeError("First argument of tuple"
                                    " should be of type INSTANT.");
    }

    while(!nl->IsEmpty(rest)){
        ListExpr current = nl->First(rest);
        rest = nl->Rest(rest);

        if (!(CcReal::checkType(nl->Second(current))
              || CcInt::checkType(nl->Second(current))))
        {
            return listutils::typeError("All attributes but the first,"
                   " should be numeric to represent a timeseries.");
        }

    }

    return timeseries;
}
