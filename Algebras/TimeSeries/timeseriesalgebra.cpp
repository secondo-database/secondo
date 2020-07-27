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
#include "timeseriesalgebra.h"

namespace TSer {

    ListExpr TimeSeriesAlgebra::genericTSTypeMapping(ListExpr timeSeries){
        const string error_message = "OrderedRelation with an instant as"
                                     " order criteria and an arbitrary number"
                                     " of reals exspected.";

        //Check type of arguments
        if(!listutils::isOrelDescription(nl->First(timeSeries))){
            return listutils::typeError(error_message);
        }

        ListExpr firstTSExpression = nl->First(timeSeries);

        ListExpr tsTupleDescription = nl->Second(firstTSExpression);


        if(!listutils::isTupleDescription(tsTupleDescription)){
                return listutils::typeError(error_message +
                                            " ( TupleDescription exspected.)");
        }

        if(!datetime::DateTime::checkType(nl
                                          ->Second(nl
                                          ->First(tsTupleDescription)))){
           return listutils::typeError((error_message +
                                        " ( First attribute should "
                                        "be of type instant.)"));
        }
        if(!CcString::checkType(nl->Second(nl->Second(tsTupleDescription)))){
            return listutils::typeError((error_message + "( Second attribute"
                                                " should be of type string.)"));
        }

        int noOfAttributes = nl->ListLength(tsTupleDescription);

        for(int indexOfAttribute = 2;
            indexOfAttribute < noOfAttributes; indexOfAttribute++){
            if(!CcReal::checkType(nl
                                 ->Second(nl
                                 ->Nth(indexOfAttribute, tsTupleDescription)))){
                return listutils::typeError(error_message +
                       "( only attributes of type real exspected after"
                       " second attribute.)");
            }
        }

        return timeSeries;
    };


    OperatorSpec timeSeriesDistanceFunSpec(
            "orel x orel x(fun : tuple x tuple) -> real ",
            "_ _ tsdistancefun[ _ ] ",
            "computes the distance between the two "
            "given timeseries utilizing the defined function",
            "query [ const (orel(tuple((Time instant)(Value real)))"
            " (Time)) value((1 1.0)) ] feed {a} [ const"
            " (orel(tuple((Time instant)(Value real))) (Time)) value((1 2.0)) ]"
            "feed {b} tsdistancefun[ "
            "fun (tuple1: TUPLE2, tuple2: TUPLE2) "
            "sqrt(pow(attr(tuple1, Value_a) - attr(tuple2, Value_b),2))];"
            );

    Operator tsDistanceFunOp(
            "tsdistancefun",
            timeSeriesDistanceFunSpec.getStr(),
            TimeSeriesDistance::timeSeriesDistanceFunOpValueMapping,
            //SimpleSelect ist für nicht überladene Operatoren,
            //bei weiteren Distanzmetriken -> anpassen!
            Operator::SimpleSelect,
            TimeSeriesDistance::distanceOpFunTypeMap
            );

    OperatorSpec timeSeriesDistanceSpec(
        "orel x orel -> real",
        "tsdistance(_, _)",
        "Computes the distance between two"
        " timeseries utilizing the euklidian metric.",
        "query dist([ const (orel(tuple((Time instant)(Value real)))"
        " (Time)) value((1 1.0)) ],[ const (orel(tuple((Time instant)"
        "(Value real))) (Time)) value((1 2.0)) ])"
    );


    Operator tsDistanceOp(
            "tsdistance",
            timeSeriesDistanceSpec.getStr(),
            TimeSeriesDistance::timeSeriesDistanceValueMapping,
            //SimpleSelect ist für nicht überladene Operatoren,
            //bei weiteren Distanzmetriken -> anpassen!
            Operator::SimpleSelect,
            TimeSeriesDistance::distanceOpTypeMap
            );

    OperatorSpec tsMotifBfSpec(
            "orel -> orel",
            " _ tsmotifbf(_,_) or _ tsmotifbf(_,_,[FUN])",
            "Computes the motif of the timeseries with the most not"
            " trivial matches of length n within a range r, utilizing"
            " the euklidian distance. The alternative utilizes an"
            " individual distance measurement.",
            "query tsmotifbf([ const (orel(tuple((Time instant"
            ")(Value real))) (Time)) value((1 1.0)(2 1.0)) ],1, 0 ) ");

    OperatorSpec tsDTWSpec(
            "orel x orel -> orel",
            "_ _ tsdtw",
            "Computes the warping path to match the first timeseries"
            " as good as possible to the second timeseries.",
            "query [ const (orel(tuple((Time instant)(Value real)))"
            " (Time)) value((1 1.0)(2 1.0)) ] "
            "[ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] tsdtw count; ");

    OperatorSpec tsDTWSakoeChibaSpec(
            "orel x orel x int-> orel",
            "dtwsc(_,_,_)",
            "Computes the warping path to match the first timeseries as good "
            "as possible to the second timeseries."
            "The integer paramter defines the Sakoe-Chiba band in wich the"
            " warping may deviate from the diagonal.",
            "query [ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] [ const (orel(tuple((Time instant)"
            "(Value real))) (Time)) value((1 1.0)(2 1.0)) ] tsdtw count;");

    OperatorSpec tsWhiteNoiseSpec(
            "orel x real x real -> orel",
            "_ tswhitenoise(_, _)",
            "Adds white noise to all values of the timeseries."
            " The parameters are the variance and mean of"
            " a gaussian distribution.",
            "query [ const (orel(tuple((Time instant)(Value real)))"
            " (Time)) value((1 1.0)(2 1.0)) ] tswhitenoise(0.0, 1.0)"
            );

    OperatorSpec tsDifSpec(
            "orel x int -> orel",
            "_ tsdif(_)",
            "Computes the differenzing of the timeseries n times"
            " where n is the operation parameter",
            "query [ const (orel(tuple((Time instant)(Value real))) "
            "(Time)) value((1 1.0)(2 1.0)) ] tsdif(1)"
            );

    OperatorSpec predictMaSpec(
            "orel x int x int -> orel",
            "_ predictma[_,_]",
            "Utilizes an Moving Average Process MA(p)"
            " to predict the next n points in the time series."
            " Order of the MA is the first parameter,"
            " number of predictions the second. ",
            "query [ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] predictma[1,1]"
            );

    OperatorSpec predictArSpec(
            "orel x int x int -> orel",
            "_ predictar[_,_]",
            "Utilizes an Autoregressive Process AR(q) to predict the next"
            " n points in the time series. Order of the AR is the"
            " first parameter, number of predictions the second. ",
            "query [ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] predictar[1,1]"
            );

    OperatorSpec predictARIMASpec(
            "orel x int x int x int x int -> orel",
            "_ predictarima[_,_,_,_]",
            "Utilizes an Integrated Autoregressive Moving Average Prozess"
            " ARIMA(p,q,i) to predict the next n points in the time series."
            " Order of the AR is the first parameter,"
            " the second the degree of differenzing, the third order of the"
            " MA Prozess and the fourth the number of predictions. ",
            "query [ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] predictarima[1,0,1,1]"
            );

    OperatorSpec predictARMASpec(
            "orel x int x int x int -> orel",
            "_ predictarima[_,_,_]",
            "Utilizes an Autoregressive Moving Average Prozess ARMA(p,q,i)"
            " to predict the next n points in the time series. Order of the"
            " AR is the first parameter,"
            " the second the order of the MA Prozess and the third the number"
            " of predictions. ",
            "query [ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] predictarima[1,0,1,1]"
            );

    OperatorSpec tsDDTWSpec(
            "orel x orel -> orel",
            "ddtw(_,_)",
            "Utilizes an approximation of point distances to compute an"
            " optimal warping path to match the first timeseries argument"
            " to the second",
            "query ddtw([ const (orel(tuple((Time instant)(Value real)))"
            " (Time))"
            " value((1 1.0)(2 1.0)) ],"
            " ddtw([ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)(3 1.5)) ])"
            );

    OperatorSpec tsPaaSpec(
            "orel x int -> orel",
            "_ paa[_]",
            "Uses the piecewise approximate aggregation described"
            " by Keogh to produce a dimensionally reduced timeseries from"
            " the original consisting of int frames.",
            "query paa([ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ],1)"
            );

    OperatorSpec tsMotifMKSpec(
            "orel x int -> orel",
            "_ motifmk[_]",
            "",
            "query motifmk ([ const (orel(tuple((Time instant)(Value real)))"
            "(Time)) value((1 1.0)(2 1.0)) ],1)"
            );

    OperatorSpec tsPacfSpec(
            "orel x int -> int",
            "_ pacf[_]",
            "",
            "query ([ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] pacf[1]"
            );
    OperatorSpec tsAcfSpec(
            "orel x int -> int",
            "_ acf[_]",
            "",
            "query ([ const (orel(tuple((Time instant)(Value real))) (Time))"
            " value((1 1.0)(2 1.0)) ] acf[1]"
            );

    Operator tsMotifOpFun(
            "tsmotifbffun",
            tsMotifBfSpec.getStr(),
            TSMotifOp::tsMotifBfValueMap,
            Operator::SimpleSelect,
            TSMotifOp::tsMotifBfTypeMap
            );

    Operator tsMotifBfOp(
            "tsmotifbf",
            tsMotifBfSpec.getStr(),
            TSMotifOp::tsMotifBfValueMap,
            Operator::SimpleSelect,
            TSMotifOp::tsMotifBfTypeMap
            );

    Operator tsDTWOp(
            "tsdtw",
            tsDTWSpec.getStr(),
            TsDTWOp::tsDTWValueMap,
            Operator::SimpleSelect,
            TsDTWOp::tsDTWTypeMap
            );

    Operator tsDifOp(
            "tsdif",
            tsDifSpec.getStr(),
            TsArima::tsDifValueMap,
            Operator::SimpleSelect,
            TsArima::tsDifTypeMap
            );

    Operator tsWhiteNoiseOp(
            "tswhitenoise",
            tsWhiteNoiseSpec.getStr(),
            TsArima::tsWhiteNoiseValueMap,
            Operator::SimpleSelect,
            TsArima::tsWhiteNoiseTypeMap
            );

    Operator predictMA(
            "predictma",
            predictMaSpec.getStr(),
            TsArima::tsMAValueMap,
            Operator::SimpleSelect,
            TsArima::tsMATypeMap
            );

    Operator predictAR(
            "predictar",
            predictMaSpec.getStr(),
            TsArima::tsARValueMap,
            Operator::SimpleSelect,
            TsArima::tsARTypeMap
            );

    Operator predictARMA(
            "predictarma",
            predictMaSpec.getStr(),
            TsArima::tsARMAValueMap,
            Operator::SimpleSelect,
            TsArima::tsARMATypeMap
            );

    Operator predictARIMA(
            "predictarima",
            predictMaSpec.getStr(),
            TsArima::tsARIMAValueMap,
            Operator::SimpleSelect,
            TsArima::tsARIMATypeMap
            );

    Operator tsDDTWOp(
            "ddtw",
            tsDDTWSpec.getStr(),
            TsDTWOp::tsDDTWValueMap,
            Operator::SimpleSelect,
            TsDTWOp::tsDTWTypeMap
            );

    Operator tsDTWSakoeChibaOp(
            "dtwsc",
            tsDTWSakoeChibaSpec.getStr(),
            TsDTWOp::tsDTWSCValueMap,
            Operator::SimpleSelect,
            TsDTWOp::tsDTWSCTypeMap
            );


    Operator tsPaaOp(
            "paa",
            tsPaaSpec.getStr(),
            TSMotifOp::tsPaaValueMap,
            Operator::SimpleSelect,
            TSMotifOp::tsPaaTypeMap
            );

    Operator motifMK(
            "motifMK",
            tsMotifMKSpec.getStr(),
            TSMotifOp::tsMotifMueenKeoghValueMap,
            Operator::SimpleSelect,
            TSMotifOp::tsMotifMueenKeoghTypeMap
            );

    Operator pacf(
            "pacf",
            tsPacfSpec.getStr(),
            TsArima::pacfValueMap,
            Operator::SimpleSelect,
            TsArima::pacfTypeMap
            );

    Operator acf(
            "acf",
            tsPacfSpec.getStr(),
            TsArima::acfValueMap,
            Operator::SimpleSelect,
            TsArima::acfTypeMap
            );

    TimeSeriesAlgebra::TimeSeriesAlgebra() : Algebra(){
        AddOperator(&tsDistanceOp);
        AddOperator(&tsDistanceFunOp);
        AddOperator(&tsMotifBfOp);
        AddOperator(&tsDTWOp);
        AddOperator(&tsDifOp);
        AddOperator(&tsWhiteNoiseOp);
        AddOperator(&predictAR);
        AddOperator(&predictMA);
        AddOperator(&predictARMA);
        AddOperator(&predictARIMA);
        AddOperator(&tsPaaOp);
        AddOperator(&tsDTWSakoeChibaOp);
        AddOperator(&tsDDTWOp);
        AddOperator(&pacf);
        AddOperator(&acf);
        //Type Mapping noch nicht implementiert, Operator nicht getestet.
        //     AddOperator(&motifMK);
    }



/*
5 Initialization

*/



    extern "C"
    Algebra*
    InitializeTimeSeriesAlgebra( NestedList* nlRef,
        QueryProcessor* qpRef )
    {
      // The C++ scope-operator :: must be used to qualify the full name
      return new TSer::TimeSeriesAlgebra;
    }
}
