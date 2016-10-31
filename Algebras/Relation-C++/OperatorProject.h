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
#ifndef ALGEBRAS_RELATION_C___OPERATORPROJECT_H_
#define ALGEBRAS_RELATION_C___OPERATORPROJECT_H_

#include "Operator.h"

class OperatorProject
{
public:

/*
5.9 Operator ~project~

5.9.1 Type mapping function of operator ~project~

Result type of project operation.

----  ((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))  ->
        (APPEND
          (k (i1 ... ik))
          (stream (tuple ((ai1 Ti1) ... (aik Tik))))
        )
----

The type mapping computes the number of attributes and the list of
attribute numbers for the given projection attributes and asks the
query processor to append it to the given arguments.

*/
    static ListExpr ProjectTypeMap(ListExpr args);

/*
5.9.2 Value mapping function of operator ~project~

*/
    static int
    Project(Word* args, Word& result, int message,
            Word& local, Supplier s);

# ifdef USE_PROGRESS
    static CostEstimation* ProjectCostEstimationFunc();
#endif

};

#endif /* ALGEBRAS_RELATION_C___OPERATORPROJECT_H_ */
