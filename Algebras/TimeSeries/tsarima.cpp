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

#include "tsarima.h"

using std::default_random_engine;
using std::to_string;
using std::normal_distribution;
using std::min;
using std::vector;

TsArima::TsArima()
{

}

/** Typemapping for Operation which adds white noise to the values of a timeseries represented
 * by an OrderedRelation consisting of a timestamp in form of an instant and an arbitrary number
 * of numerical values.
 * @brief TsArima::tsWhiteNoiseTypeMap
 * @return
 */
ListExpr TsArima::tsWhiteNoiseTypeMap(ListExpr args)
{
    string error = "Timeseries white noise type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr mean = nl->Second(args);

    if(!CcReal::checkType(mean)){
        return listutils::typeError(error +
           "(second argument should be a real value with the "
           "mean of the normal distribution)");
    }

    ListExpr variance = nl->Third(args);

    if(!CcReal::checkType(variance)){
        return listutils::typeError(error + "(third argument should be a real"
                                            " value with the variance of the"
                                            " normal distribution)");
    }

    return timeseries;
}

/** Valuemapping for operation which adds white noise to the values of a timeseries represented
 * by an OrderedRelation consisting of a timestamp in form of an instant and an arbitrary number
 * of numerical values.
 * @brief TsArima::tsWhiteNoiseValueMap
 * @return
 */
int TsArima::tsWhiteNoiseValueMap(Word *args, Word &result,
                                  int message, Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    OrderedRelation* noisy_timeseries = (OrderedRelation*) result.addr;

    double mean = ((CcReal*) args[1].addr)->GetValue();
    double variance = ((CcReal*) args[2].addr)->GetValue();

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);

    normal_distribution<double> white_noise{mean, variance};

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(!ts_iter->EndOfScan())
    {
        Tuple* current_tuple = ts_iter->GetNextTuple();
        if( ts_iter->EndOfScan())
            break;

        Tuple* clone = current_tuple->Clone();

        for(int attr_no = 1; attr_no < clone->GetNoAttributes(); attr_no ++)
        {
            Attribute* attribute = clone->GetAttribute(attr_no);
            CcReal* value = ((CcReal*)attribute);
            cout << "Ursprünglicher Wert: "
                 << to_string(value->GetValue()) << " ";
            value->Set(value->GetValue() + white_noise(generator));
            cout << "Noisy value : " << to_string(value->GetValue()) << endl;
        }
        noisy_timeseries->AppendTuple(clone);
        current_tuple->DeleteIfAllowed();
        clone->DeleteIfAllowed();
    }
    return 0;
}

/**
 * @brief TsArima::tsDifTypeMap Typemapping for tsdif operation for differencing timeseries to achieve weak stationarity
 * @param args
 * @return
 */
ListExpr TsArima::tsDifTypeMap(ListExpr args)
{
    string error = "Timeseries differencing type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr differencing_steps = nl->Second(args);

    if(!CcInt::checkType(differencing_steps)){
        return listutils::typeError(error + "(second argument should "
                                            "be an int value with the number "
                                            "of differencing steps.)");
    }


    return timeseries;

}

/**
 * @brief TsArima::tsDifValueMap valuemapping for tsdif operation. Result is a timeseries which is weak stationary.
 * @param args args[0] OrderedRelation -> timeseries, args[1] CcInt -> number of differencing steps
 * @param result differenzed timeseries
 * @param message
 * @param local
 * @param s
 * @return
 */
int TsArima::tsDifValueMap(Word *args, Word &result, int message,
                           Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    OrderedRelation* differenced_timeseries = (OrderedRelation*) result.addr;
    int differecing_steps = ((CcInt*) args[1].addr)->GetValue();

    vector<Tuple*> timeseries_values;

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(! ts_iter->EndOfScan())
    {
        Tuple* current = ts_iter->GetNextTuple();
        if( current == 0 )
                break;
        timeseries_values.push_back(current);
    }

    while( differecing_steps > 0 && timeseries_values.size() > 1)
    {
        differecing_steps --;
        vector<Tuple*> differenced_values;
        for(size_t index = 1; index < timeseries_values.size(); index++)
        {
            Tuple* first = timeseries_values.at(index);
            Tuple* second = timeseries_values.at(index -1);
            Tuple* differenced_value = first->Clone();

            for(int attr_no = 1; attr_no < first->GetNoAttributes(); attr_no++)
            {
                double first_value = ((CcReal*) differenced_value->
                                      GetAttribute(attr_no))->GetValue();
                double second_value = ((CcReal*) second->
                                       GetAttribute(attr_no))->GetValue();
                ((CcReal*) differenced_value->
                        GetAttribute(attr_no))->Set(first_value - second_value);
                cout<< "Differenzierter Wert: "
                    << to_string(first_value-second_value) << endl;
            }
            differenced_values.push_back(differenced_value);

        }
        for(auto tuple : timeseries_values)
        {
            tuple->DeleteIfAllowed();
        }
        timeseries_values = differenced_values;
    }

    for(auto tuple : timeseries_values)
    {
        differenced_timeseries->AppendTuple(tuple);
        tuple->DeleteIfAllowed();
    }

    return 0;
}

/**
 * @brief TsArima::tsARValueMap type mapping for the prediction of future timeseries value utilizing an MA(q) process.
 * @param args
 * @param result
 * @param message
 * @param local
 * @param s
 * @return
 */
int TsArima::tsARValueMap(Word *args, Word &result, int message,
                          Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    OrderedRelation* predictedtimeseries = (OrderedRelation*) result.addr;
    int ar_order = ((CcInt*) args[1].addr)->GetValue();
    int steps = ((CcInt*) args[2].addr)->GetValue();

    vector<Tuple*> timeseries_values =
            mapOrelToVectorAndPrepareResult(timeseries, predictedtimeseries);

    Arima* arima = new Arima(ar_order, 0, 0, steps, timeseries_values);
    vector<double> prediction = arima->ar();

    prepareResult(timeseries_values, prediction,
                  timeseries, predictedtimeseries, steps);

    return 0;
}


/**
 * @brief TsArima::tsARTypeMap type mapping for AR model forecasting of timeseries
 * @param args
 * @return
 */
ListExpr TsArima::tsARTypeMap(ListExpr args)
{
    string error = "Timeseries autoregressive model type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr order = nl->Second(args);

    if(!CcInt::checkType(order)){
        return listutils::typeError(error + "(second argument should be "
                                            "an integer value with the order "
                                            "of the AR process)");
    }

    ListExpr steps = nl->Third(args);

    if(!CcInt::checkType(steps)){
        return listutils::typeError(error + "(third argument should be"
                                            " an integer value with the number"
                                            " of steps to be predicted)");
    }

    return timeseries;
}


/**
 * @brief TsArima::tsMATypeMap type mapping for MA model forecasting of timeseries
 * @param args
 * @return
 */
ListExpr TsArima::tsMATypeMap(ListExpr args)
{
    string error = "Timeseries moving average model type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr order = nl->Second(args);

    if(!CcInt::checkType(order)){
        return listutils::typeError(error + "(second argument should be"
                                            " an integer value with the order"
                                            " of the MA process)");
    }

    ListExpr steps = nl->Third(args);

    if(!CcInt::checkType(steps)){
        return listutils::typeError(error + "(third argument should be an"
                                            " integer value with the number"
                                            " of steps to be predicted)");
    }

    return timeseries;
}


 // @brief TsArima::tsMAValueMap value mapping for prediction of
 // future time series values utilizing an AR(p) prozess
int TsArima::tsMAValueMap(Word *args, Word &result, int message,
                          Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    OrderedRelation* predictedtimeseries = (OrderedRelation*) result.addr;
    int ma_order = ((CcInt*) args[1].addr)->GetValue();
    int steps = ((CcInt*) args[2].addr)->GetValue();

    vector<Tuple*> timeseries_values;

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(! ts_iter->EndOfScan())
    {
        Tuple* current = ts_iter->GetNextTuple();
        if( current == 0 )
                break;
        timeseries_values.push_back(current);
        //predictedtimeseries->AppendTuple(current->Clone());
    }

    Arima* arima = new Arima(0, 0, ma_order, steps, timeseries_values);
    vector<double> prediction = arima->ma();
    cout << "Prepare Result MA" << endl;
    prepareResult(timeseries_values, prediction,
                  timeseries, predictedtimeseries, steps);

    return 0;
}

/**
 * @brief TsArima::tsARTypeMap type mapping for ARIMA model
 *  forecasting of timeseries
 * @param args
 * @return
 */
ListExpr TsArima::tsARIMATypeMap(ListExpr args)
{
    string error = "Timeseries autoregressive integrated moving "
                   "average (ARIMA) model type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr orderAR = nl->Second(args);

    if(!CcInt::checkType(orderAR)){
        return listutils::typeError(error + "(second argument should be"
                                            " an integer value with the"
                                            " order of the AR process)");
    }

    ListExpr integration = nl->Third(args);

    if(!CcInt::checkType(integration)){
        return listutils::typeError(error + "(third argument should be "
                                            "an integer value with the order"
                                            " of differencing steps)");
    }

    ListExpr orderMA = nl->Fourth(args);

    if(!CcInt::checkType(orderMA)){
        return listutils::typeError(error + "(fourth argument should be"
                                            " an integer value with the order"
                                            " of the MA process)");
    }

    ListExpr steps = nl->Fifth(args);

    if(!CcInt::checkType(steps)){
        return listutils::typeError(error + "(fifth argument should be"
                                            " an integer with the number"
                                            " of timesteps to be predicted)");
    }

    return timeseries;
}

/**
 * @brief TsArima::tsARIMAValueMap typemapping for the prediction
 *  of timeseries values utilizing an ARIMA(p,d,q) process
 * @param args
 * @param result
 * @param message
 * @param local
 * @param s
 * @return
 */
int TsArima::tsARIMAValueMap(Word *args, Word &result,
                             int message, Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    OrderedRelation* predictedtimeseries = (OrderedRelation*) result.addr;

    int ar_order = ((CcInt*) args[1].addr)->GetValue();
    int ma_order = ((CcInt*) args[2].addr)->GetValue();
    int differencing_order = ((CcInt*) args[3].addr)->GetValue();
    int steps = ((CcInt*) args[4].addr)->GetValue();

    vector<Tuple*> timeseries_values;

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(! ts_iter->EndOfScan())
    {
        Tuple* current = ts_iter->GetNextTuple();
        if( current == 0 )
                break;
        timeseries_values.push_back(current);
    }

    Arima* arima = new Arima(ar_order, differencing_order,
                             ma_order, steps, timeseries_values);
    vector<double> prediction = arima->forecast();
    cout << "Prepare Result ARIMA" << endl;
    prepareResult(timeseries_values, prediction, timeseries,
                  predictedtimeseries, steps);

    return 0;
}

/**
 * @brief TsArima::tsARTypeMap type mapping for ARMA model
 *  forecasting of timeseries
 * @param args
 * @return
 */
ListExpr TsArima::tsARMATypeMap(ListExpr args)
{
    string error = "Timeseries autoregressive moving average"
                   " (ARMA) model type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr orderAR = nl->Second(args);

    if(!CcInt::checkType(orderAR)){
        return listutils::typeError(error + "(second argument should be"
                                            " an integer value with the order"
                                            " of the AR process)");
    }


    ListExpr orderMA = nl->Third(args);

    if(!CcInt::checkType(orderMA)){
        return listutils::typeError(error + "(third argument should be an"
                                            " integer value with the order"
                                            " of the MA process)");
    }

    ListExpr steps = nl->Fourth(args);

    if(!CcInt::checkType(steps)){
        return listutils::typeError(error + "(fourth argument should be an"
                                            " integer with the number of"
                                            " timesteps to be predicted)");
    }

    return timeseries;
}

/**
 * @brief TsArima::tsARMAValueMap value mapping for prediction of
 *  timeseries values utilizing an ARMA(p,q) process.
 * @param args
 * @param result
 * @param message
 * @param local
 * @param s
 * @return
 */
int TsArima::tsARMAValueMap(Word *args, Word &result, int message,
                            Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    OrderedRelation* predictedtimeseries = (OrderedRelation*) result.addr;

    int ar_order = ((CcInt*) args[1].addr)->GetValue();
    int ma_order = ((CcInt*) args[2].addr)->GetValue();
    int steps = ((CcInt*) args[3].addr)->GetValue();

    vector<Tuple*> timeseries_values;

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(! ts_iter->EndOfScan())
    {
        Tuple* current = ts_iter->GetNextTuple();
        if( current == 0 )
                break;
        timeseries_values.push_back(current);
    }

    Arima* arima = new Arima(ar_order, 0, ma_order, steps, timeseries_values);
    vector<double> prediction = arima->forecast();
    cout << "Prepare Result ARMA" << endl;
    prepareResult(timeseries_values, prediction, timeseries,
                  predictedtimeseries, steps);

    return 0;
}

/**
 * @brief TsArima::compute_ts_interval computes the instant interval used
 *  in the timeseries for tuple constuction of the prediction
 * @param timeseries
 * @return
 */
double TsArima::compute_ts_interval(vector<Tuple *> timeseries)
{
    vector<Instant*> instants;

    for(size_t i = 0; i < timeseries.size(); i++)
    {
        Tuple* tuple = timeseries[i];
        instants.push_back((Instant*) tuple->GetAttribute(0));
    }

    double mean = 0;

    for(size_t i = 1; i < instants.size(); i++)
    {
        mean += instants[i]->ToDouble() - instants[i-1]->ToDouble();
    }

    return mean / instants.size();
}

vector<Tuple*> TsArima::mapOrelToVectorAndPrepareResult(OrderedRelation
                               *timeseries, OrderedRelation* prediction)
{
    vector<Tuple*> timeseries_values;

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(! ts_iter->EndOfScan())
    {
        Tuple* current = ts_iter->GetNextTuple();
        if( current == 0 )
                break;
        timeseries_values.push_back(current);
    //    prediction->AppendTuple(current->Clone());
    }
    return timeseries_values;
}

/*
 *  Appends the prediction result to the result ordered relation.
 */
void TsArima::prepareResult(vector<Tuple*>& timeseries_values,
                            vector<double>& prediction,
                            OrderedRelation *timeseries,
                            OrderedRelation *predictedtimeseries, int steps)
{
    double ts_interval = compute_ts_interval(timeseries_values);
    double next_instant_value = ((Instant*)
                                 timeseries_values[timeseries_values.size()-1]
            ->GetAttribute(0))->ToDouble();

    for( int i = 0; i < timeseries->GetNoTuples() + steps; ++i)
    {
        if( i > timeseries->GetNoTuples() -1)
        {
            Tuple* tuple = timeseries_values[timeseries_values.size()-1]
                    ->Clone();
            Instant* instant = (Instant*) tuple->GetAttribute(0);
            next_instant_value += ts_interval;
            instant->ReadFrom(next_instant_value);
            CcReal* value = (CcReal*) tuple->GetAttribute(1);
            value->Set(prediction[i]);
            predictedtimeseries->AppendTuple(tuple);
            tuple->DeleteIfAllowed();
        }
        else
        {
            ((CcReal*) timeseries_values[i]->GetAttribute(1))
                    ->Set(prediction[i]);
            predictedtimeseries->AppendTuple(timeseries_values[i]);
        }


    }
}

/*
 * Value Mapping for the console print of the pacf of a timeseries.
 */
ListExpr TsArima::pacfTypeMap(ListExpr args)
{
    string error = "Timeseries autoregressive integrated moving average"
                   " (ARMA) model type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr lags = nl->Second(args);

    if(!CcInt::checkType(lags)){
        return listutils::typeError(error + "(second argument should be"
                                            " an integer value with the number"
                                            " of lags for the display"
                                            " of the PACF)");
    }


    return listutils::basicSymbol<CcInt>();
}

/*
 * Valuemapping for the console print of the PACF of a time series
 */
int TsArima::pacfValueMap(Word *args, Word &result,
                          int message, Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    CcInt* lags = (CcInt*) result.addr;

    int lag = ((CcInt*) args[1].addr)->GetValue();

    vector<Tuple*> timeseries_values;

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(! ts_iter->EndOfScan())
    {
        Tuple* current = ts_iter->GetNextTuple();
        if( current == 0 )
                break;
        timeseries_values.push_back(current);
    }

    Arima* arima = new Arima(0, 0, 0, 0, timeseries_values);
    arima->printPACF(lag);

    lags->Set(true, lag);
    return 0;
}


/*
 * Valuemapping for the console print of the ACF of a time series
 */
ListExpr TsArima::acfTypeMap(ListExpr args)
{
    string error = "Timeseries autoregressive integrated moving average"
                   " (ARMA) model type mapping error: ";

    ListExpr timeseries = nl->First(args);

    ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
    if(!nl->Equal(evaluatedTS, timeseries))
    {
            return evaluatedTS;
    }

    ListExpr lags = nl->Second(args);

    if(!CcInt::checkType(lags)){
        return listutils::typeError(error + "(second argument should be"
                                            " an integer value with the number"
                                            " of lags for the display"
                                            " of the PACF)");
    }


    return listutils::basicSymbol<CcInt>();
}

/*
 * Value Mapping for the console print of the acf of a timeseries.
 */
int TsArima::acfValueMap(Word *args, Word &result,
                         int message, Word &local, Supplier s)
{
    OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
    result = qp->ResultStorage(s);
    CcInt* lags = (CcInt*) result.addr;

    int lag = ((CcInt*) args[1].addr)->GetValue();

    vector<Tuple*> timeseries_values;

    GenericRelationIterator* ts_iter = timeseries->MakeScan();

    while(! ts_iter->EndOfScan())
    {
        Tuple* current = ts_iter->GetNextTuple();
        if( current == 0 )
                break;
        timeseries_values.push_back(current);
    }

    Arima* arima = new Arima(0, 0, 0, 0, timeseries_values);
    arima->printACF(lag);

    lags->Set(true, lag);
    return 0;
}
