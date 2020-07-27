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

static const string INT_TYPE = "int";
static const string REAL_TYPE = "real";

using std::list;

    /**
     * @brief distanceOpTypeMap
     * @param args
     * @return
     */
    ListExpr TimeSeriesDistance::distanceOpTypeMap(ListExpr args){
        TimeSeriesDistance distanceOp = TimeSeriesDistance();

       if (!nl->HasLength(args, 2)){
            return NList::typeError("Two arguments exspected.");
        }

        ListExpr arg_1 = nl->First(args);
        NList arg_1_index = distanceOp.checkTSTupleStream(arg_1);
        ListExpr arg_2 = nl->Second(args);
        NList arg_2_index = distanceOp.checkTSTupleStream(arg_2);

        return distanceOp.appendNumericIndices(arg_1_index, arg_2_index);




    }
    /**
     * @brief TimeSeriesDistance::appendNumericIndices appends the indices
     *  of the found numeric attributes
     * in the tuple description to the result
     * @param numeric_index_ts1 Nested list with the indices of the numeric
     *  attributes in the tuple description of a timeseries
     * @param numeric_index_ts2 Nested list with the indices of the numeric
     *  attributes in the tuple description of a timeseries
     * @return
     */
    ListExpr TimeSeriesDistance::appendNumericIndices(NList numeric_index_ts1,
                                                      NList numeric_index_ts2){
        numeric_index_ts1.print(cout) << endl;
        numeric_index_ts2.print(cout) << endl;

        if(numeric_index_ts1.length() != numeric_index_ts2.length()){
            return NList::typeError("Equal number of"
                                    " numeric attributes exspected");
        }

        NList argumentList;
        NList element;
        argumentList.intAtom(numeric_index_ts1.length());
        for(Cardinal i = 1 ; i <= numeric_index_ts1.length(); ++i){
            argumentList.append(
                        element.intAtom(numeric_index_ts1.elem(i).intval()));
        }

        for(Cardinal i = 1 ; i <= numeric_index_ts2.length(); ++i){
            argumentList.append(
                        element.intAtom(numeric_index_ts2.elem(i).intval()));
        }

        argumentList.print(cout)<<endl;

       return NList(NList(Symbol::APPEND()),
                    argumentList, CcReal::BasicType()).listExpr();
    }

    /**
     * @brief checkTSTupleStream Checks whether the NList contains a
     *  tuple stream and finds the index of
     *        numeric data contained in the tuple
     * @param type NList which should describe a tuple stream
     * @return NList which contains the index of numeric attributes
     *  in the tuples of the stream.
     */
    NList TimeSeriesDistance::checkTSTupleStream(ListExpr& listExpr){

        NList type = NList(listExpr);

        if (
          !type.hasLength(2)  ||
          !listutils::isTupleDescription(type.second().listExpr())||
          !listutils::isTupleStream(listExpr))
        {
          return NList::typeError("Tuplestream exspected.");
        }

        list<int> indexOfNumerics;
        cout<<"Tuple Description:";
        NList(type.second()).print(cout) <<endl;
        cout<<"Tuple detail:";
        NList(type.second().second()).print(cout) <<endl;
        ListExpr rest = type.second().second().listExpr();
        ListExpr current;

        //Find index of numeric Values in tuple description
        int index = 0;
        while(!nl->IsEmpty(rest)){
            current = nl->First(rest);
            rest = nl->Rest(rest);

            if(nl->IsEqual(nl->Second(current), INT_TYPE)||
                    nl->IsEqual(nl->Second(current), REAL_TYPE)){
                    indexOfNumerics.push_front(index);
            }
            index++;
        }

        if(indexOfNumerics.empty()){
            return NList::typeError("No numeric values found.");
        }
        //Create nested list of index of numerics
        NList indexList;
        NList element;
        for(auto it = next(indexOfNumerics.begin());
            it != indexOfNumerics.end(); ++it){
            indexList.append(element.intAtom(*it));
        }

        return indexList;
    }



    int TimeSeriesDistance::timeSeriesDistanceValueMapping(Word* args,
                                                           Word& result,
                                                           int message,
                                                           Word& local,
                                                           Supplier s)
    {

        cout<<"Inside Value Mapping"<< endl;

        qp->Open(args[0].addr);//Open first argument stream
        qp->Open(args[1].addr);//Open second argument stream
        cout<<"After argument stream" <<endl;
        Stream<Tuple> ts1Stream(args[0]);
        Stream<Tuple> ts2Stream(args[1]);
        int lenghOfIndexList = ((CcInt*)args[2].addr)->GetIntval();
        cout<<"Länge der Indexlisten: " << lenghOfIndexList <<endl;

        int endOfArguments = lenghOfIndexList *2 + 2;

        set<int> numericIndexTs1Set;
        set<int> numericIndexTs2Set;

        for( int index = 3; index <= endOfArguments; index++){
            int argumentindex = ((CcInt*)args[index].addr)->GetIntval();
            if(index - 3 < lenghOfIndexList){
                numericIndexTs1Set.insert(argumentindex);
            }
            else{
                numericIndexTs2Set.insert(argumentindex);
            }
        }


        ts1Stream.open();
        ts2Stream.open();

        Tuple* nextTs1Tuple = ts1Stream.request();
        Tuple* nextTs2Tuple = ts2Stream.request();

        double distance = 0;

        while(nextTs1Tuple != 0 && nextTs2Tuple != 0){
            list<double> tupleValuesTs1;
            list<double> tupleValuesTs2;

            for(int attributeIndex = 0;
                attributeIndex < nextTs1Tuple->GetNoAttributes();
                attributeIndex++)
            {
                Attribute* ts1Attribute = nextTs1Tuple
                        ->GetAttribute(attributeIndex);
                Attribute* ts2Attribute = nextTs2Tuple
                        ->GetAttribute(attributeIndex);
                if(numericIndexTs1Set.find(attributeIndex)
                        != numericIndexTs1Set.end() ){
                    tupleValuesTs1.push_back(
                                ((CcReal*)ts1Attribute)->GetValue());
                }
                if(numericIndexTs2Set.find(attributeIndex)
                        != numericIndexTs2Set.end()){
                    tupleValuesTs2.push_back(
                                ((CcReal*)ts2Attribute)->GetValue());
                }
                ts1Attribute->DeleteIfAllowed();
                ts2Attribute->DeleteIfAllowed();

            }
            TimeSeriesDistance distanceOp = TimeSeriesDistance();
            distance += distanceOp.computePointDistance(tupleValuesTs1,
                                                        tupleValuesTs2,
                                                        Euklidian);

            if(nextTs1Tuple == 0 || nextTs2Tuple == 0){
                break;
            }
          //  nextTs1Tuple->DeleteIfAllowed();
          //  nextTs2Tuple->DeleteIfAllowed();
            nextTs1Tuple = ts1Stream.request();
            nextTs2Tuple = ts2Stream.request();
        }

        ts1Stream.close();
        ts2Stream.close();

        result = qp->ResultStorage(s);
        CcReal* resultDistance = (CcReal*) result.addr;
        resultDistance->Set(true, distance);

        return 0;
    }

    ListExpr TimeSeriesDistance::distanceOpFunTypeMap(ListExpr args){
        cout << "Arguments: " <<endl << nl->ToString(args) << endl;

        if(nl->ListLength(args)!=3){
          return listutils::typeError("three arguments expected");
        }
        ListExpr stream1 = nl->First(args);
        ListExpr stream2 = nl->Second(args);
        ListExpr map = nl->Third(args);

        string err = "stream(tuple1) x stream(tuple2) x "
                     "( tuple1 x tuple2 -> real) expected";
        if(!listutils::isTupleStream(stream1) ||
           !listutils::isTupleStream(stream2) ||
           !listutils::isMap<2>(map)){
          return listutils::typeError(err);
        }
        return listutils::basicSymbol<CcReal>();
    }

    int TimeSeriesDistance::timeSeriesDistanceFunOpValueMapping(Word *args,
                                                                Word &result,
                                                                int message,
                                                                Word &local,
                                                                Supplier s){

        TimeSeriesDistanceOpLI* localInfo = 0;
        cout << "Value mapping arguments message: " << message <<endl;

        localInfo = new TimeSeriesDistanceOpLI(args[0], args[1], args[2]);
        local.addr = localInfo;
        localInfo->compute();


        result = qp->ResultStorage(s);
        CcReal* resultDistance = (CcReal*) result.addr;
        resultDistance->Set(true, localInfo->getResult());
        cout <<"Result: " << localInfo->getResult() << endl;

        delete localInfo;
        return 0;
    }


    double TimeSeriesDistance::computePointDistance(list<double> &firstPoint,
                                                    list<double> &secondPoint,
                                                    Norm norm){
        switch(norm){
        case Manhattan: return manhattanDistance(firstPoint, secondPoint);
        case Euklidian: return euklidianDistance(firstPoint, secondPoint);
        case Maximum: return maximumDistance(firstPoint, secondPoint);
        }
        return 0;
    }



    double TimeSeriesDistance::manhattanDistance(list<double> &firstPoint,
                                                 list<double> &secondPoint){
        return 0;
    }

    double TimeSeriesDistance::euklidianDistance(list<double> &firstPoint,
                                                 list<double> &secondPoint){
        auto it1 = firstPoint.begin();
        auto it2 = secondPoint.begin();
        double distance = 0;
        for(; it1 != firstPoint.end() && it2 != secondPoint.end(); ++it1, ++it2)
        {
            distance += pow((*it1 - *it2),2);
        }
        return sqrt(distance);
    }

    double TimeSeriesDistance::maximumDistance(list<double> &firstPoint,
                                               list<double> &secondPoint){
        return 0;
    }


