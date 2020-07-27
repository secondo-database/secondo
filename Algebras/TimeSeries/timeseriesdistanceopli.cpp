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
#include "timeseriesdistanceopli.h"

TimeSeriesDistanceOpLI::TimeSeriesDistanceOpLI(Word ts1, Word ts2, Word f) :
    ts1_stream(ts1), ts2_stream(ts2), fun(f)
{
   this->ts1_stream.open();
   this->ts2_stream.open();
   this->funargs = qp->Argument(f.addr);
   this->distance = 0;
}

TimeSeriesDistanceOpLI::~TimeSeriesDistanceOpLI(){
    cout << "Delete Local Info";
    this->ts1_stream.close();

    this->ts2_stream.close();

}


 // Computes the distance between two timeseries utilizing the distance function
 // in the arguments.
 // @brief TimeSeriesDistanceOpLI::compute
void TimeSeriesDistanceOpLI::compute(){
    Tuple* funarg_ts1 = this->ts1_stream.request();
    funarg_ts1->Print( cout) << endl;
    Tuple* funarg_ts2 = this->ts2_stream.request();

    while(funarg_ts1 && funarg_ts2)
    {

    (*funargs[0]) = funarg_ts1;
    (*funargs[1]) = funarg_ts2;

    //get the result of the computation
    Word funres;
    qp->Request(this->funargs, funres);

    funarg_ts1->DeleteIfAllowed();
    funarg_ts2->DeleteIfAllowed();

    CcReal tupleDistance  = static_cast<CcReal>(funres.addr);

    this->distance += tupleDistance.GetValue();

    funarg_ts1 = ts1_stream.request();
    funarg_ts2 = ts2_stream.request();
    }
}

double TimeSeriesDistanceOpLI::getResult(){
    return this->distance;
}
