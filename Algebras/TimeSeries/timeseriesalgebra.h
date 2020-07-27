/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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
#ifndef TIMESERIESALGEBRA_H
#define TIMESERIESALGEBRA_H

#include "Algebra.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "NList.h"
#include "Operator.h"
#include "AlgebraManager.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Stream.h"
#include "GenericTC.h"
#include "LogMsg.h"
#include "../Algebras/Relation-C++/RelationAlgebra.h"
#include "../Algebras/OrderedRelation/OrderedRelationAlgebra.h"
#include "TimeSeriesDistance.h"
#include "tsmotifop.h"
#include "tsdtwop.h"
#include "tsarima.h"




// Import of SECONDO specific Members implemented as Singleton.
extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;
using std::string;



namespace TSer{
    static const string INT_TYPE = "int";
    static const string REAL_TYPE = "real";

    class TimeSeriesAlgebra: public Algebra{

    public:
        TimeSeriesAlgebra();
        ~TimeSeriesAlgebra(){};
        static ListExpr genericTSTypeMapping(ListExpr timeSeries);

    };

}
#endif // TIMESERIESALGEBRA_H
