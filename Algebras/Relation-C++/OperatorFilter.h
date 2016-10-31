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
#ifndef ALGEBRAS_RELATION_C___OPERATORFILTER_H_
#define ALGEBRAS_RELATION_C___OPERATORFILTER_H_

#include "Operator.h"

class OperatorFilter
{
public:
/*
5.8 Operator ~filter~

Only tuples, fulfilling a certain condition are passed on to the
output stream.

5.8.1 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream (tuple x)) (map (tuple x) bool))
               -> (stream (tuple x))
----

Type mapping function modified to show the possibility of getting
not only types but also arguments in the type mapping. This happens
when an operator
registers "UsesArgsinTypeMapping". Type list now has the form

----  ( (type1 arg1) (type2 arg2) )
----

that is

----  (
      ( (stream (tuple x))  arg1 )
      ( (map (tuple x) bool)  arg2 )
    )
----

*/
    static ListExpr FilterTypeMap(ListExpr args);

/*
5.8.2 Value mapping function of operator ~filter~

*/

    static int
    Filter(Word* args, Word& result, int message,
           Word& local, Supplier s);

#ifdef USE_PROGRESS
    static CostEstimation* FilterCostEstimationFunc();
#endif

};

#endif /* ALGEBRAS_RELATION_C___OPERATORFILTER_H_ */
