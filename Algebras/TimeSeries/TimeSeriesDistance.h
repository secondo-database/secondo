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

#ifndef TIMESERIESDISTANCE_H
#define TIMESERIESDISTANCE_H

#include "Stream.h"
#include "timeseriesdistanceopli.h"


class TimeSeriesDistance{
public:
    CcReal distance(Stream<Tuple>*, Stream<Tuple>*);
    static Operator distanceOp(std::string, std::string, int, int, ListExpr );
    static Operator distanceOpFun(std::string,std::string, int, int, ListExpr);
    static ListExpr distanceOpTypeMap(ListExpr);
    static int timeSeriesDistanceValueMapping(Word* args, Word& result,
                                  int message, Word& local, Supplier s);
    static Operator distanceFunOp(std::string, std::string, int, int, ListExpr);
    static ListExpr distanceOpFunTypeMap(ListExpr);
    static int timeSeriesDistanceFunOpValueMapping(Word* args, Word& result,
                                  int message, Word& local, Supplier s);

private:
    enum Norm {Manhattan, Euklidian, Maximum};
    NList checkTSTupleStream(ListExpr&);
    double manhattanDistance(std::list<double>&, std::list<double>&);
    double euklidianDistance(std::list<double>&, std::list<double>&);
    double maximumDistance(std::list<double>&, std::list<double>&);
    double computePointDistance(std::list<double>&, std::list<double>&, Norm);
    ListExpr appendNumericIndices(NList ts1, NList ts2);
    static int openFunStream(Word *args, Word &local,
                             TimeSeriesDistanceOpLI* li);


};

#endif // TIMESERIESDISTANCE_H
