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
#ifndef ALGEBRAS_RELATION_C___OPERATORFEEDPROJECT_H_
#define ALGEBRAS_RELATION_C___OPERATORFEEDPROJECT_H_

#include "Operator.h"

#ifdef USE_PROGRESS

struct FeedProjectInfo : OperatorInfo {

   FeedProjectInfo() : OperatorInfo()
   {
     name =      "feedproject";

     signature = "rel(tuple(a_1 ... a_n)) x (a_i1 ... a_ik)\n"
                 "-> stream(tuple(a_i1 ... a_ik))";
     syntax =    "_ feedproject[ _ ]";
     meaning =   "Creates a stream of tuples from a given relation "
                 " and project them to the given list of attributes.";
     example =   "plz feedproject[PLZ] count";

     supportsProgress = true;
   }

};

class OperatorFeedProject
{
public:
    static ListExpr feedproject_tm(ListExpr args);

    static CostEstimation* FeedProjectCostEstimationFunc();

    static int
    feedproject_vm(Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s);
};
#endif

#endif /* ALGEBRAS_RELATION_C___OPERATORFEEDPROJECT_H_ */

