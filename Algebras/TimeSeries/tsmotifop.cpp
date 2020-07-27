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
#include "tsmotifop.h"

static const string INT_TYPE = "int";
static const string REAL_TYPE = "real";

using std::numeric_limits;

    ListExpr TSMotifOp::tsMotifBfTypeMap(ListExpr args)
    {
        string error = "";

        if(nl->HasLength(args, 3) || nl->HasLength(args, 4)){

            ListExpr timeseries = nl->First(args);
            NList timeserie = NList(timeseries);

            NList typeList = timeserie.second().second();

            if(!OrderedRelation::checkType(timeseries)){
                return listutils::typeError(error + " (first argument should"
                         " be a ordered relation representing a timeseries.");
            }
            if(!listutils::isTupleDescription(nl->Second(timeseries))){
                return listutils::typeError((error +
                         " (Tuple description should be valid!)"));
            }

            ListExpr motifLength = nl->Second(args);
            NList motif = NList(motifLength);

            if(!CcInt::checkType(motifLength)){
                return listutils::typeError(error +
                "(second argument should be an integer with length of motif)");
            }

            NList typeInfo = NList();

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

                if (!(CcReal::checkType(nl->Second(current)) ||
                      CcInt::checkType(nl->Second(current))))
                {
                    return listutils::typeError("All attributes but the first,"
                               " should be numeric to represent a timeseries.");
                }

            }

            ListExpr maxDistance = nl->Third(args);
            NList dist = NList(maxDistance);

            if(!CcReal::checkType(maxDistance)){
                return listutils::typeError(error +" (third argument should be "
                       "a real value for the max. deviation from the motif ).");
            }

            if(nl->HasLength(args,4))
            {
            if(!listutils::isMap<2>(nl->Fourth(args))){
                return listutils::typeError((error +
                    " (fourth argument should be a map with two arguments). "));
            }
        }


        return timeseries;
        }
    else
    {
    return listutils::typeError(error +
     " (three or four arguments exspected. Should be Orel, int, real [Map]). ");
    }
   }

    /*
     * Typemapping function for piecewise approximate aggregation
     *  which exspects a Orel configured as a Timeseries and
     * an integer argument
     */

    ListExpr TSMotifOp::tsPaaTypeMap(ListExpr args)
    {
        string error = "Paa type mapping error: ";

        ListExpr timeseries = nl->First(args);

        ListExpr evaluatedTS = checkTimeSeriesType(timeseries);
        if(!nl->Equal(evaluatedTS, timeseries))
        {
                return evaluatedTS;
        }

        ListExpr orderAR = nl->Second(args);

        if(!CcInt::checkType(orderAR)){
            return listutils::typeError(error +
            "(second argument should be an integer value"
            " with the number of frames for the paa process)");
        }
        return timeseries;
    }

    /**
     * @brief TSMotifOp::tsMotifBfValueMap value mapping for the
     *  naive discovery of a motif in a time series.
     * @param args
     * @param result
     * @param message
     * @param local
     * @param s
     * @return
     */
    int TSMotifOp::tsMotifBfValueMap(Word *args, Word &result,
                                     int message, Word &local, Supplier s)
    {
        //Used for parameter functions as distance function
        ArgVectorPointer funargs = 0;
        if(args[4].addr != 0)
        {
            funargs = qp->Argument(args[4].addr);
        }

        int best_motif_count_so_far = 0;
        size_t best_motif_start_index = 0;
        vector<vector<Tuple*>> motif_matches;

        size_t motif_length = ((CcInt*) args[1].addr)->GetIntval();

        double max_deviation = ((CcReal*) args[2].addr)->GetRealval();

        vector<Tuple*> timeseries_data;

        vector<Tuple*> motif;

        set<int> best_motif_start_indices;

        OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;

        GenericRelationIterator* ts_iter = timeseries->MakeScan();

        Tuple* current = 0;

        while((current = ts_iter->GetNextTuple()) != 0)
        {
            timeseries_data.push_back(current);
        }


        for(size_t i = 0; i < timeseries_data.size() - motif_length; i++)
        {
            int counter = 0;
            vector<vector<Tuple*>> motif_pointers;
            vector<Tuple*> motif;
            set<int> candidate_match_start_indices;
            for(size_t motif_index = i;
                motif_index < i + motif_length; motif_index++)
            {
                motif.push_back(timeseries_data.at(motif_index));
            }

            for(size_t inner_index = 0;
                inner_index < timeseries_data.size() - motif_length;
                inner_index++)
            {
                vector<Tuple*> match_candidate;
                for(size_t candidate_index = inner_index;
                    candidate_index < inner_index + motif_length;
                    candidate_index++)
                {
                    match_candidate.push_back(
                                timeseries_data.at(candidate_index));
                }

                if(funargs != 0)
                {
                    if(is_non_trivial_match_fun(motif, i, match_candidate,
                                          inner_index, max_deviation, funargs))
                    {
                        counter ++;
                        candidate_match_start_indices.insert(inner_index);
                        motif_pointers.push_back(match_candidate);
                    }
                }
                else
                {
                    if(is_non_trivial_match(motif, i, match_candidate,
                                            inner_index, max_deviation))
                    {
                        counter ++;
                        candidate_match_start_indices.insert(inner_index);
                        motif_pointers.push_back(match_candidate);
                    }
                }
            }// End inner for-loop
            if ( counter > best_motif_count_so_far)
            {
                best_motif_count_so_far = counter;
                best_motif_start_index = i;
                motif_matches.clear();
                best_motif_start_indices.clear();
                best_motif_start_indices = candidate_match_start_indices;
                motif_matches = motif_pointers;
            }
        }//end outer for-loop

        //Set motif tuples
        for(size_t index = best_motif_start_index;
            index < best_motif_start_index + motif_length; index++ )
        {
            motif.push_back(timeseries_data.at(index));
        }

        cout << "Motif length: " << motif_length << endl;
        cout << "Best motif match count: " << best_motif_count_so_far << endl;
        cout << "Best motif start index: " << best_motif_start_index << endl;
        cout << "Start indeces of matches:" << "[" ;
        for(int index : best_motif_start_indices)
        {
            cout << index << "," ;
        }
        cout << "]" << endl;
        result = qp->ResultStorage(s);
        OrderedRelation* resultMotif = (OrderedRelation*) result.addr;
        for(auto tuple : motif)
        {
            resultMotif->AppendTuple(tuple);
            tuple->DeleteIfAllowed();
        }

        return 0;
    }//end tsMotifBfValueMap

    /**
     * @brief TSMotifOp::is_non_trivial_match method to check whether
     *  a motiv is trivial e.g. if it is identical
     * to an already discovered motif or minimaly shifted.
     * @param motif
     * @param start_of_motif index of the first value of the motif
     * @param candidate
     * @param start_of_candidate index of the first value of the candidate
     * @param max_distance distance to be trivial
     * @return
     */
    bool TSMotifOp::is_non_trivial_match(vector<Tuple *> motif,
                    int start_of_motif, vector<Tuple *> candidate,
                    int start_of_candidate, double max_distance)
    {
        if(start_of_motif == start_of_candidate)
            return false;
        if(compute_distance(motif, candidate) <= max_distance)
            return true;
        return false;
    }

    /**
     * @brief TSMotifOp::compute_distance computes the distance
     *  between two sequences of a timeseries
     * utilizing the euklidian distance.
     * @param motif
     * @param candidate
     * @return
     */
    double TSMotifOp::compute_distance(vector<Tuple*> motif,
                                       vector<Tuple *> candidate)
    {
       double distance = 0;

       for(size_t i = 0; i <motif.size(); i++)
       {
           Tuple* motif_tuple = motif.at(i);
           Tuple* candidate_tuple = candidate.at(i);

           for(int attr_no = 1;
               attr_no < motif_tuple->GetNoAttributes(); attr_no++)
           {
               Attribute* motif_attr = motif_tuple->GetAttribute(attr_no);
               Attribute* candidate_attr = candidate_tuple
                       ->GetAttribute(attr_no);

               double value_motif = ((CcReal*) motif_attr)
                       ->GetRealval();
               double value_candidate = ((CcReal*) candidate_attr)
                       ->GetRealval();
               distance += pow(value_motif - value_candidate,2);
           }
       }

       return sqrt(distance);
    }//End compute_distance

    bool TSMotifOp::is_non_trivial_match_fun(vector<Tuple*> motif,
                                             int start_of_motiv,
                                             vector<Tuple*> candidate,
                                             int start_of_candidate,
                                             double max_distance,
                                             ArgVectorPointer fun_args)
    {
        if(start_of_motiv == start_of_candidate)
            return false;
        if(compute_distance_fun(motif, candidate, fun_args) <= max_distance)
            return true;
        return false;
    }//End is_non_trivial_match_fun

    double TSMotifOp::compute_distance_fun(vector<Tuple*> motif,
                                           vector<Tuple*> candidate,
                                           ArgVectorPointer fun_args){
        double distance = 0;
        for(size_t index = 0; index < motif.size(); index++)
        {
            (*fun_args[0]) = motif.at(index);
            (*fun_args[1]) = candidate.at((index));
            Word funres;
            qp->Request(fun_args, funres);
            CcReal tupleDistance  = static_cast<CcReal>(funres.addr);
            distance += tupleDistance.GetRealval();
        }
        return distance;

    }//End compute_distance_fun

    int TSMotifOp::tsPaaValueMap(Word *args, Word &result,
                                 int message, Word &local, Supplier s)
    {
        OrderedRelation* timeseries = (OrderedRelation*) args[0].addr;
        int N = ((CcInt*) args[1].addr)->GetIntval();

        vector<Tuple*> timeseries_data;
        vector<Tuple*> paa;

        GenericRelationIterator* ts_iter = timeseries->MakeScan();
        //Read timeseries data
        Tuple* current = 0;

        while((current = ts_iter->GetNextTuple()) != 0)
        {
            timeseries_data.push_back(current);
        }

        size_t n = timeseries_data.size();

        if(n%N != 0)
        {
            cout << "Warning, to receive proper sized frames"
                    " it is neccesary that lenght of the timeseries n = "
                 << to_string(n)
                 << "is divisible by the selected number of frames "
                 << to_string(N)<< " ! " << endl;
        }

        size_t end_frame = N;
        for(size_t i = 0; i < n; ++i)
        {
            Instant* start;
            Instant* end;
            double mean = 0;
            double sum = 0;

            if( i < end_frame )
            {
                Tuple* current = timeseries_data[i];
                sum += ((CcReal*)current->GetAttribute(1))->GetRealval();
                if(i == end_frame -N)
                {
                    start = (Instant*)current->GetAttribute(0);
                }
                if(i == end_frame -1)
                {
                    end = (Instant*)current->GetAttribute(0);
                    Tuple* meanTuple = current->Clone();

                    if(N == 1)
                        mean = start->ToDouble();
                    else
                        mean = (start->ToDouble() + end->ToDouble())/N;

                    ((Instant*) meanTuple->GetAttribute(0))->ReadFrom(mean);
                    ((CcReal*) meanTuple->GetAttribute(1))->Set(sum/N);
                    mean = (end->ToDouble() - start->ToDouble())/N;
                    paa.push_back(meanTuple);
                }
            }
            if(i == end_frame -1)
            {
                end_frame += N;
            }

        }

        result = qp->ResultStorage(s);
        OrderedRelation* resultMotif = (OrderedRelation*) result.addr;
        for(auto tuple : paa)
        {
            resultMotif->AppendTuple(tuple);
            tuple->DeleteIfAllowed();
        }

        return 0;
    }


    int TSMotifOp::tsMotifMueenKeoghValueMap(Word *args, Word &result,
                                             int message, Word &local,
                                             Supplier s)
    {
        return 0;
    }

    /*
     *  TypeMapping for the motif search with the Mueen-Keogh-Algorithm.
     */
    ListExpr TSMotifOp::tsMotifMueenKeoghTypeMap(ListExpr args)
    {
        string error = "";

        if(nl->HasLength(args, 3) || nl->HasLength(args, 4)){

            ListExpr timeseries = nl->First(args);
            NList timeserie = NList(timeseries);

            NList typeList = timeserie.second().second();

            if(!OrderedRelation::checkType(timeseries)){
                return listutils::typeError(error +
                       " (first argument should be a ordered relation"
                       " representing a timeseries.");
            }
            if(!listutils::isTupleDescription(nl->Second(timeseries))){
                return listutils::typeError((error +
                       " (Tuple description should be valid!)"));
            }

            ListExpr motifLength = nl->Second(args);
            NList motif = NList(motifLength);

            if(!CcInt::checkType(motifLength)){
                return listutils::typeError(error +
                       "(second argument should be an integer with"
                       " length of motif)");
            }

            ListExpr numberOfReferences = nl->Third(args);
            NList references = NList(numberOfReferences);

            if(!CcInt::checkType(numberOfReferences)){
                return listutils::typeError(error +
                       "(third argument should be an integer with number"
                       " of references)");
            }

            ListExpr subsequenceLength = nl->Fourth(args);

            if(!CcInt::checkType(subsequenceLength)){
                return listutils::typeError(error
                       + "(fourth argument should be an integer with"
                         " length of subsequences)");
            }

            ListExpr precision = nl->Fifth(args);

            if(!CcReal::checkType(precision)){
                return listutils::typeError(error +
                       "(fourth argument should be an real with "
                       "precission of motif distance)");
            }

            NList typeInfo = NList();

            ListExpr rest = typeList.listExpr();

            ListExpr timestamp = nl->First(rest);
            rest = nl->Rest(rest);

            if(!Instant::checkType(nl->Second(timestamp)))
            {
                return listutils::typeError("First argument of "
                                            "tuple should be of type INSTANT.");
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
    else
    {
    return listutils::typeError(error +
           " (three or four arguments exspected. Should be Orel, "
           "int, real [Map]). ");
    }
    }

    /*
     *  Implementation of the Mueen-Keogh algorithm for motif
     *  discovery adapted to work on a single Timeseries.
     */
    vector<Tuple*> TSMotifOp::mueenKeogh(vector<Tuple *> timeseries_data,
                                         int number_of_references,
                                         int motif_length,
                                         int subsequenze_length)
    {
        double best_so_far = numeric_limits<double>::infinity(),
                lower_bound = 0.0;
        bool abandon, reject;
        int i, j, L1, offset, nobs = timeseries_data.size(),
                m = subsequenze_length;

        srand((unsigned) time(0));

        vector<double> data(timeseries_data.size(), 0);

        //Divide timeseries into subsequences to create timeseries database
        int number_of_subsequences = nobs % subsequenze_length == 0
                ? nobs/subsequenze_length : nobs/subsequenze_length + 1;
        //TimeseriesDatabase
        vector<vector<double>> D(number_of_subsequences,
                                 vector<double>(subsequenze_length, 0));

        //Standard deviation of distance vectors
        vector<double> S;

        SecondoHelper::getTSData(timeseries_data, data);

        for( i = 0; i < number_of_subsequences; ++i)
        {
            vector<double>::const_iterator first =
                    data.begin() + i * subsequenze_length;
            vector<double>::const_iterator last =
                    data.begin() + subsequenze_length + i * subsequenze_length;
            D[i] =vector<double>(first, last);
        }


        vector<vector<double>> distance_matrix(number_of_references,
                                            vector<double>(motif_length, 0.0));

        vector<int> ref(number_of_references, 0);
        vector<vector<int>> distance_ordered_ts_indices;

        for( i = 0; i < number_of_references; ++i)
        {
            ref[i] = rand() % (number_of_subsequences);


            for(j = 0; j < m; j++)
            {
                distance_matrix[i][j] = dist(D[ref[i]],D[j] );

                if(distance_matrix[i][j] < best_so_far)
                {
                    best_so_far = distance_matrix[i][j];
                    L1 = number_of_references;
                }
            }
            S[i] = standard_deviation(distance_matrix[i]);

        }

        //find an ordering Z of the indices to the reference time series
        //in ref such that SZ(i) >= SZ(i+1)
        sort(ref.begin(), ref.end());

        for(i = 0; i < number_of_references; ++i)
            for(j= 0; j < m; ++j)
                distance_ordered_ts_indices[i][j] = j;

        //find an Ordering I of the indices to the time
        //series in D such that DistZ(I),I(J) <= DistZ(I),I(j+1)
        for(i = 0; i < number_of_references; ++i)
        {
            for(j = 0; j < m - 1; ++j)
            {
                for(int x = 0; x < motif_length-j-1; ++x)
                {
                    if(distance_matrix[i][x] > distance_matrix[i][x+1] )
                    {
                        distance_ordered_ts_indices[i][x] = x + 1;
                        distance_ordered_ts_indices[i][x+1] = x;
                    }
                }
            }
        }

        offset = 0;
        abandon = false;

        while(!abandon)
        {
            offset += 1;
            abandon = true;

            for( j = 0; j < m; j++)
            {
                reject = false;

                for( i = 0; i < number_of_references; ++i)
                {
                    lower_bound = abs(distance_matrix[i][j]
                                      - distance_matrix[i][j + offset]);
                    if( lower_bound > best_so_far)
                    {
                        reject = true;
                        break;
                    }
                    else if( i == 1)
                    {
                        abandon = false;
                    }
                }
                if(reject == false)
                {
                    double distance = dist(D[distance_ordered_ts_indices[j][i]],
                            D[distance_ordered_ts_indices[j + offset][i]]);
                    if( distance < best_so_far)
                    {
                        best_so_far = distance;
                        L1 = distance_ordered_ts_indices[j][i];
            //            L2 = distance_ordered_ts_indices[j + offset][i];
                    }

                }
            }
        }


        vector<Tuple*> motif;

        for( i = 0; i < motif_length; ++i)
        {
            Tuple* clone = timeseries_data[0]->Clone();
            Instant* instant = (Instant*) clone->GetAttribute(0);
            instant->ReadFrom(instant->ToDouble() + i);
            CcReal* value = (CcReal*) clone->GetAttribute(1);
            value->Set(D[L1][i]);
            motif.push_back(clone);

        }

        return motif;
    }

    /*
     * Computes the standard deviation of a given distance vector.
     */
    double TSMotifOp::standard_deviation(vector<double> distance)
    {
        double sum = std::accumulate(distance.begin(), distance.end(), 0.0);
        double mean = sum / distance.size();

        double squared = 0.0;

        for(auto dist : distance)
        {
            squared += pow(dist - mean, 2);
        }

        return sqrt(squared / distance.size());
    }

    /*
     * Computes the euklidian distance between two vectors.
     */
    double TSMotifOp::dist(const vector<double> &d_i, const vector<double> &d_j)
    {
        double sum_squared = 0.0;

        for(size_t i = 0; i < d_i.size(); ++i )
            sum_squared += pow(d_i[i] - d_j[i], 2);

        return sqrt(sum_squared);
    }
