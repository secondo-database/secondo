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
#ifndef ALGEBRAS_RELATION_C___OPERATORFEED_H_
#define ALGEBRAS_RELATION_C___OPERATORFEED_H_

#include "Operator.h"

class OperatorFeed
{
public:

/*

5.5 Operator ~feed~

Produces a stream from a relation by scanning the relation tuple by
tuple.

5.5.1 Type mapping function of operator ~feed~

A type mapping function takes a nested list as argument. Its
contents are type descriptions of an operator's input parameters.
A nested list describing the output type of the operator is returned.

Result type of feed operation.

----    ((rel x))  -> (stream x)
----

*/
    static ListExpr FeedTypeMap(ListExpr args);

/*

5.5.2 Value mapping function of operator ~feed~

*/

    static int
    Feed(Word* args, Word& result, int message, Word& local, Supplier s);

#ifdef USE_PROGRESS
    static CostEstimation* FeedCostEstimationFunc();
#endif

};

#endif /* ALGEBRAS_RELATION_C___OPERATORFEED_H_ */
