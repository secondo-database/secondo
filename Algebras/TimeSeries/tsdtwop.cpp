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
#include "tsdtwop.h"

using std::string;
using std::min;
using std::cout;
using std::to_string;
using std::numeric_limits;

TsDTWOp::TsDTWOp()
{

}


//  Type mapping for the timeseries algebra operators for
//   dynamic time warping(DTW)

ListExpr TsDTWOp::tsDTWTypeMap(ListExpr args)
{
    string error = "Type Mapping error for Operator tsdtw ";
    if(!nl->HasLength(args,2)){return listutils::typeError(error +
                                " ( tworarguments exspected.)");}

        ListExpr timeseries_a = nl->First(args);
        ListExpr timeseries_b = nl->Second(args);

        if(!OrderedRelation::checkType(timeseries_a)
                || !OrderedRelation::checkType((timeseries_b))){
            return listutils::typeError(error +
                   " (first and second argument should be a ordered relation"
                   " representing a timeseries.");
        }
        if(!listutils::isTupleDescription(nl->Second(timeseries_a))
                || !listutils::isTupleDescription(nl->Second(timeseries_b))){
            return listutils::typeError((error +
                   " (Tuple description should be valid!)"));
        }

        ListExpr typeinfo_a = nl->Second(nl->Second(timeseries_a));
        ListExpr typeinfo_b = nl->Second(nl->Second(timeseries_b));
        ListExpr result_a = checkAttributes( typeinfo_a );
        ListExpr result_b = checkAttributes( typeinfo_b );

        if (!nl->IsEmpty(result_a))
        {
            return result_a;
        }
        else if ( !nl->IsEmpty(result_b))
        {
            return result_b;
        }

    return timeseries_a;

}


//Value mapping for the derived dynamic time warping function.
//Returns an warped first argument to match the second argument.
int TsDTWOp::tsDDTWValueMap(Word *args, Word &result, int message,
                            Word &local, Supplier s)
{

    OrderedRelation* ts1 = (OrderedRelation*) args[0].addr;
    OrderedRelation* ts2 = (OrderedRelation*) args[1].addr;

    vector<double> ts1_data;
    vector<Tuple*> ts1_tuples;
    vector<double> ts2_data;


    GenericRelationIterator* ts1_iter = ts1->MakeScan();
    GenericRelationIterator* ts2_iter = ts2->MakeScan();

    int ts1_size = ts1->GetNoTuples();
    int ts2_size = ts2->GetNoTuples();

    Tuple* current = 0;

    while((current = ts1_iter->GetNextTuple()) != 0)
    {
        ts1_data.push_back(((CcReal*)current->GetAttribute(1))->GetRealval());
        ts1_tuples.push_back(current);
    }

    while((current = ts2_iter->GetNextTuple()) != 0)
    {
        ts2_data.push_back(((CcReal*)current->GetAttribute(1))->GetRealval());
    }

    vector<vector<double>> cost_matrix(ts1_size, vector<double>
                           (ts2_size,numeric_limits<double>::infinity()));



    cost_matrix.at(0).at(0) = 0.0;
    cost_matrix[ts1_size-1][ts2_size-1] = 0.0;

    for ( int ts1_index = 1; ts1_index < ts1_size -1; ts1_index++)
    {
        for ( int ts2_index = 1; ts2_index < ts2_size -1; ts2_index++)
        {
            double cost = derivativeDistance(computeDerivative(
                          ts1_data[ts1_index -1],
                          ts1_data[ts1_index],
                          ts1_data[ts1_index +1]),
                    computeDerivative(ts2_data[ts2_index -1],
                    ts2_data[ts2_index],
                    ts2_data[ts2_index +1]));
            cost_matrix.at(ts1_index).at(ts2_index) = cost + min({
                                     cost_matrix.at(ts1_index-1).at(ts2_index),
                                     cost_matrix.at(ts1_index).at(ts2_index-1),
                                     cost_matrix.at(ts1_index-1).at(ts2_index-1)
                                                                 });
        }
    }

    vector<WarpingPathPoint> warping_path;

    optimalWarpingPath(cost_matrix, warping_path );

    double distance = 0;

    for(size_t i = 0; i < warping_path.size(); ++i)
    {
        WarpingPathPoint point = warping_path[i];
        distance += derivativeDistance(ts1_data[point._x], ts2_data[point._y]);
    }

    cout << "Distance: " << to_string(distance) << endl;

    result = qp->ResultStorage(s);

    OrderedRelation* warpedTS = (OrderedRelation*) result.addr;

    int no_warping_points = warping_path.size();

    for(int i = 0; i < no_warping_points; ++i)
    {
        Tuple* ts1_tuple = ts1_tuples[warping_path[i]._x]->Clone();
        int count_repeats = 1;
        while(count_repeats + 1 < no_warping_points && warping_path[i]._y
              == warping_path[i+count_repeats]._y)
        {
            count_repeats++;
        }
        if(count_repeats > 1)
        {
            i = i + count_repeats -1;
        }


        warpedTS->AppendTuple(ts1_tuple);
    }

    return 0;
}


//Implementation of the classic dynamic time warping algorithm
//without optimization or limitations.
int TsDTWOp::tsDTWValueMap(Word *args, Word &result, int message,
                           Word &local, Supplier s)
{
    OrderedRelation* ts1 = (OrderedRelation*) args[0].addr;
    OrderedRelation* ts2 = (OrderedRelation*) args[1].addr;

    vector<Tuple*> ts1_data;
    vector<Tuple*> ts2_data;
    int ts1_size = ts1->GetNoTuples();
    int ts2_size = ts2->GetNoTuples();
    vector<vector<double>> cost_matrix;

    GenericRelationIterator* ts1_iter = ts1->MakeScan();
    GenericRelationIterator* ts2_iter = ts2->MakeScan();

    while(!ts1_iter->EndOfScan())
    {
        ts1_data.push_back(ts1_iter->GetNextTuple());
    }

    while(!ts2_iter->EndOfScan())
    {
        ts2_data.push_back(ts2_iter->GetNextTuple());
    }

    for(int index_outer_loop = 0;
        index_outer_loop < ts1_size;
        index_outer_loop++)
    {
        vector<double> inner_vector;
        for(int index_inner_loop = 0;
            index_inner_loop < ts2_size;
            index_inner_loop++)
        {
            inner_vector.push_back(numeric_limits<double>::infinity());
        }
        cost_matrix.push_back(inner_vector);
    }

    cost_matrix.at(0).at(0) = 0.0;

    for ( int ts1_index = 1; ts1_index < ts1_size; ts1_index++)
    {
        for ( int ts2_index = 1; ts2_index < ts2_size; ts2_index++)
        {
            double cost = euklidianDistance(ts1_data.at(ts1_index),
                                            ts2_data.at(ts2_index));
            cost_matrix.at(ts1_index).at(ts2_index) = cost + min({
                                     cost_matrix.at(ts1_index-1).at(ts2_index),
                                     cost_matrix.at(ts1_index).at(ts2_index-1),
                                     cost_matrix.at(ts1_index-1).at(ts2_index-1)
                                                                 });
        }
    }

    vector<WarpingPathPoint> warping_path;

    optimalWarpingPath(cost_matrix, warping_path );


    result = qp->ResultStorage(s);

    OrderedRelation* warpedTS = (OrderedRelation*) result.addr;

    int no_warping_points = warping_path.size();

    for(int i = 0; i < no_warping_points; ++i)
    {
        Tuple* ts1_tuple = ts1_data.at(warping_path[i]._x)->Clone();
        int count_repeats = 1;
        while(count_repeats + 1 < no_warping_points && warping_path[i]._y
              == warping_path[i+count_repeats]._y)
        {
            count_repeats++;
        }
        if(count_repeats > 1)
        {
            i = i + count_repeats -1;
        }


        warpedTS->AppendTuple(ts1_tuple);
    }

    double distance = 0;

    for(size_t i = 0; i < warping_path.size(); ++i)
    {
        WarpingPathPoint point = warping_path[i];
        distance += euklidianDistance(ts1_data[point._x],ts2_data[point._y]);
    }

    cout << "Distance: " << to_string(distance) << endl;

    return 0;
}


//Typemapping function for the dynamic time warping utilizing
//the Sakoe Chiba band.
ListExpr TsDTWOp::tsDTWSCTypeMap(ListExpr args)
{
    string error = "";

    if(!nl->HasLength(args,3)){
        return listutils::typeError(error + " ( three arguments exspected.)");
    }

        ListExpr timeseries_a = nl->First(args);
        ListExpr timeseries_b = nl->Second(args);

        if(!OrderedRelation::checkType(timeseries_a)
                || !OrderedRelation::checkType((timeseries_b))){
            return listutils::typeError(error +
                   " (first and second argument should be a ordered"
                   " relation representing a timeseries.");
        }
        if(!listutils::isTupleDescription(nl->Second(timeseries_a))
                || !listutils::isTupleDescription(nl->Second(timeseries_b))){
            return listutils::typeError((error +
                   " (Tuple description should be valid!)"));
        }

        ListExpr typeinfo_a = nl->Second(nl->Second(timeseries_a));
        ListExpr typeinfo_b = nl->Second(nl->Second(timeseries_b));
        ListExpr result_a = checkAttributes( typeinfo_a );
        ListExpr result_b = checkAttributes( typeinfo_b );

        ListExpr band_size = nl->Third(args);
        if(!CcInt::checkType(band_size))
            return listutils::typeError(error +
                   "(third argument must be a positive integer for the limit"
                   " of the sakoe chiba band.)");

        if (!nl->IsEmpty(result_a))
        {
            return result_a;
        }
        else if ( !nl->IsEmpty(result_b))
        {
            return result_b;
        }

    return timeseries_a;

}



// Value mapping for the  dynamic time warping Sakoe Chiba function.
// Returns an warped first argument to match the second argument.
int TsDTWOp::tsDTWSCValueMap(Word *args, Word &result, int message,
                             Word &local, Supplier s)
{

    OrderedRelation* ts1 = (OrderedRelation*) args[0].addr;
    OrderedRelation* ts2 = (OrderedRelation*) args[1].addr;

    int band_size = ((CcInt*) args[2].addr)->GetIntval();

    vector<Tuple*> ts1_data;
    vector<Tuple*> ts2_data;
    int ts1_size = ts1->GetNoTuples();
    int ts2_size = ts2->GetNoTuples();
    vector<vector<double>> cost_matrix;

    GenericRelationIterator* ts1_iter = ts1->MakeScan();
    GenericRelationIterator* ts2_iter = ts2->MakeScan();

    while(!ts1_iter->EndOfScan())
    {
        ts1_data.push_back(ts1_iter->GetNextTuple());
    }

    while(!ts2_iter->EndOfScan())
    {
        ts2_data.push_back(ts2_iter->GetNextTuple());
    }

    for(int index_outer_loop = 0;
        index_outer_loop < ts1_size;
        index_outer_loop++)
    {
        vector<double> inner_vector;
        for(int index_inner_loop = 0;
            index_inner_loop < ts2_size;
            index_inner_loop++)
        {
            inner_vector.push_back(numeric_limits<double>::infinity());
        }
        cost_matrix.push_back(inner_vector);
    }

    cost_matrix.at(0).at(0) = 0.0;

    for ( int ts1_index = 1; ts1_index < ts1_size; ts1_index++)
    {
        for ( int ts2_index = 1; ts2_index < ts2_size; ts2_index++)
        {
            double cost = euklidianDistance(ts1_data.at(ts1_index),
                                            ts2_data.at(ts2_index));
            cost_matrix.at(ts1_index).at(ts2_index) = cost + min({
                                     cost_matrix.at(ts1_index-1).at(ts2_index),
                                     cost_matrix.at(ts1_index).at(ts2_index-1),
                                     cost_matrix.at(ts1_index-1).at(ts2_index-1)
                                                                 });
        }
    }

    vector<WarpingPathPoint> warping_path;

    optimalWarpingPath(cost_matrix, warping_path, band_size );


    result = qp->ResultStorage(s);

    OrderedRelation* warpedTS = (OrderedRelation*) result.addr;

    int no_warping_points = warping_path.size();

    for(int i = 0; i < no_warping_points; ++i)
    {
        Tuple* ts1_tuple = ts1_data.at(warping_path[i]._x)->Clone();
        int count_repeats = 1;
        while(count_repeats + 1 < no_warping_points && warping_path[i]._y ==
              warping_path[i+count_repeats]._y)
        {
            count_repeats++;
        }
        if(count_repeats > 1)
        {
            i = i + count_repeats -1;
        }


        warpedTS->AppendTuple(ts1_tuple);
    }

    double distance = 0;

    for(size_t i = 0; i < warping_path.size(); ++i)
    {
        WarpingPathPoint point = warping_path[i];
        distance += euklidianDistance(ts1_data[point._x],ts2_data[point._y]);
    }

    cout << "Distance: " << to_string(distance) << endl;

    return 0;
}


 // @brief TsDTWOp::checkAttributes checks wheter the attributes of an
 // ordered relation correspond to a timeseries
 // @param rest attribute info
 // @return empty nested list or type error
ListExpr TsDTWOp::checkAttributes(ListExpr rest)
{
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
    return rest;
}



// Count the number of attributes which are a numeric value.
int TsDTWOp::countValues(ListExpr tuple_attr)
{
    int attr_count= 0;
    ListExpr rest = tuple_attr;


    while(!nl->IsEmpty(rest)){
        ListExpr current = nl->First(rest);
        rest = nl->Rest(rest);

        if (CcReal::checkType(nl->Second(current))
                || CcInt::checkType(nl->Second(current)))
        {
            attr_count++;
        }

    }

    return attr_count;
}


// Computes the euklidian distance between the values of
// two tuples represented in a time series.
double TsDTWOp::euklidianDistance(Tuple *ts1_tuple, Tuple *ts2_tuple)
{
   double distance = 0;

   for(int attr_no = 1; attr_no < ts1_tuple->GetNoAttributes(); attr_no++)
   {
       distance += pow(((CcReal*)ts1_tuple->GetAttribute(attr_no))->GetValue()
                   - ((CcReal*)ts2_tuple->GetAttribute(attr_no))->GetValue(),2);
   }

   return sqrt(distance);
}


// Computes the optimal warping path utilizing a window size in order to
// keep the warping in the given limits of the Sakoe Chiba band.
void TsDTWOp::optimalWarpingPath(vector<vector<double>>& cost_matrix,
                                 vector<WarpingPathPoint> &warpingPath,
                                 int window_size)
{
    int rows = cost_matrix.size() -1;
    int columns = cost_matrix.at(0).size() -1;

    //Add endpoint of path
    warpingPath.push_back(WarpingPathPoint(rows, columns));

    //compute minimal cost path from endpoint to startpoint.
    while(rows > 1 && columns > 1)
    {
        if(rows == 1)
        {
            columns = columns -1;
        }
        else if(columns == 1)
        {
            rows = rows -1;
        }
        if(abs(columns -rows) <= window_size)
        {
                //normal dtw
                if(cost_matrix.at(rows-1).at(columns) == min({
                                         cost_matrix.at(rows -1).at(columns),
                                         cost_matrix.at(rows).at(columns -1),
                                         cost_matrix.at(rows -1).at(columns -1)
                                                              }))
                {
                rows = rows -1;
                }
                else if(cost_matrix.at(rows).at(columns -1) == min({
                                          cost_matrix.at(rows -1).at(columns),
                                          cost_matrix.at(rows).at(columns -1),
                                          cost_matrix.at(rows -1).at(columns -1)
                                                                   }))
                {
                    columns = columns -1;
                }
                else
                {
                    rows = rows -1;
                    columns = columns -1;
                }
            }
            else if(columns >= rows - window_size){ columns = columns -1;}
            else{rows = rows -1;}

        warpingPath.push_back(WarpingPathPoint(rows, columns));
    }
    //Add beginn of path to path
    warpingPath.push_back(WarpingPathPoint(0,0));
}


//  Computes the optimal warping path to minimize the cost of the
//  warping path in order to receive the
//  warped representation with minimal distance to the reference time series.
void TsDTWOp::optimalWarpingPath(vector<vector<double>>& cost_matrix,
                                 vector<WarpingPathPoint> &warpingPath)
{
    size_t rows = cost_matrix.size() -1;
    size_t columns = cost_matrix.at(0).size() -1;

    //Add endpoint of path
    warpingPath.push_back(WarpingPathPoint(rows, columns));

    //compute minimal cost path from endpoint to startpoint.
    while(rows > 1 && columns > 1)
    {
        if(rows == 1)
        {
            columns = columns -1;
        }
        else if(columns == 1)
        {
            rows = rows -1;
        }
        else
        {
            if(cost_matrix.at(rows-1).at(columns) == min({
                                     cost_matrix.at(rows -1).at(columns),
                                     cost_matrix.at(rows).at(columns -1),
                                     cost_matrix.at(rows -1).at(columns -1)
                                                          }))
            {
            rows = rows -1;
            }
            else if(cost_matrix.at(rows).at(columns -1) == min({
                                        cost_matrix.at(rows -1).at(columns),
                                        cost_matrix.at(rows).at(columns -1),
                                        cost_matrix.at(rows -1).at(columns -1)
                                                               }))
            {
                columns = columns -1;
            }
            else
            {
                rows = rows -1;
                columns = columns -1;
            }
        }
        warpingPath.push_back(WarpingPathPoint(rows, columns));
    }
    //Add beginn of path to path
    warpingPath.push_back(WarpingPathPoint(0,0));
}


// Computes the approximation of the derivative using three points.
double TsDTWOp::computeDerivative(double qiminus1, double qi, double qiplus1)
{
    return ((qi - qiminus1) + ((qiplus1 - qi - 1)/2)) -2;
}


//  Computes the distance as the square of the difference of the estimated
// derivative of the timeseriespoints.
//  Exspects the derivative, as the estimation needs three values each.

double TsDTWOp::derivativeDistance(double derivative_ts1, double derivative_ts2)
{
    return pow((derivative_ts1 - derivative_ts2),2);
}
//End Class
