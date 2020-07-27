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
#ifndef TSMOTIFOP_H
#define TSMOTIFOP_H

#include "Stream.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../Algebras/OrderedRelation/OrderedRelationAlgebra.h"
#include "../ExtRelation-2/TupleBuffer2.h"
#include "tsop.h"
#include "secondohelper.h"
#include <numeric>
#include <ctime>

using std::vector;
using std::string;
using std::set;
using std::cout;
using std::to_string;

class TSMotifOp : public TsOp
{
public:
    TSMotifOp();
    static ListExpr tsMotifBfTypeMap(ListExpr);
    static int tsMotifBfFunValueMap(Word*, Word&, int, Word&, Supplier);
    static int tsMotifBfValueMap(Word*, Word&, int, Word&, Supplier);
    static int tsPaaValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsPaaTypeMap(ListExpr);
    static int tsMotifMueenKeoghValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsMotifMueenKeoghTypeMap(ListExpr);
private:
    static bool is_non_trivial_match(vector<Tuple*>,
                                     int, vector<Tuple*>, int, double);
    static double compute_distance(vector<Tuple*>,
                                   vector<Tuple*>);
    static bool is_non_trivial_match_fun(vector<Tuple*>,
                                   int, vector<Tuple*>, int,
                                   double, ArgVectorPointer);
    static double compute_distance_fun(vector<Tuple*>,
                                       vector<Tuple*>, ArgVectorPointer);
    static vector<Tuple*> mueenKeogh(vector<Tuple*> timeseries_data,
                                     int number_of_references, int motif_length,
                                     int subsequence_length);
    static double standard_deviation(vector<double> distance);
    static double dist(const vector<double> &d_i, const vector<double> &d_j);
};

#endif // TSMOTIFOP_H
