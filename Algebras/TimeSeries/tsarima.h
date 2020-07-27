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
#ifndef TSARIMA_H
#define TSARIMA_H

#include "tsop.h"
#include <random>
#include <chrono>
#include <armahelper.h>
#include <arima.h>

// Header for Class TsARIMA which allows to compute
//forecasts of vectors of doubles
// using AR, MA, ARMA or ARIMA processes.


class TsArima : public TsOp
{
public:
    TsArima();
    static int tsWhiteNoiseValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsWhiteNoiseTypeMap(ListExpr);
    static int tsDifValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsDifTypeMap(ListExpr);

    static int pacfValueMap(Word *args, Word &result,
                            int message, Word &local, Supplier s);
    static ListExpr pacfTypeMap(ListExpr args);

    static int acfValueMap(Word *args, Word &result,
                           int message, Word &local, Supplier s);
    static ListExpr acfTypeMap(ListExpr args);

    static int tsARValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsARTypeMap(ListExpr);
    static int tsMAValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsMATypeMap(ListExpr);
    static int tsARMAValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsARMATypeMap(ListExpr);
    static int tsARIMAValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsARIMATypeMap(ListExpr);
 private:
    static vector<Tuple*> maForecast(vector<Tuple*> timeseries, int lag);
    static double compute_ts_interval(vector<Tuple*> timeseries);
    static vector<Tuple*> mapOrelToVectorAndPrepareResult(OrderedRelation*,
                                                          OrderedRelation*);
    static void prepareResult(vector<Tuple*>& timeseries_values,
                              vector<double>& prediction, OrderedRelation*,
                              OrderedRelation*, int);

};


#endif // TSARIMA_H
