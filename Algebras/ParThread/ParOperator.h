/*
---- 
This file is part of SECONDO.

Copyright (C) 2019, University in Hagen, Department of Computer Science, 
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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: ParOperator

September 2019, Fischer Thomas

1.1 Overview

The par- operator is a leaf of a query plan subtree. This subtree 
describes an execution context, which is usually evaluated in a
separate thread parallel with other execution contexts. 

This operator consumes a local tuple buffer which needs to be set by the 
~ParallelQueryProcessor~ and is filled by adjacent execution contexts.

1.2 Imports

*/
#ifndef ALGEBRAS_PARTHREAD_PAROPERATOR
#define ALGEBRAS_PARTHREAD_PAROPERATOR

#include "Operator.h"

namespace parThread
{

    class ParOperator
    {
    public: //static methods
/*
1.3 Type mapping function of operator ~par~

Result type of filter operation:

----    (stream (tuple x) x int x [string] ) -> (stream (tuple x))
----

The first parameter represents the number of used entities for the 
connected context. The second parameter is optional and indicates that 
the attribut with the name given by the parameter is used for hash-
partitioning.

*/
        static ListExpr ParTM(ListExpr args);

/*
1.4 Value mapping function of operator ~par~

*/
        static int ParVM(Word *args, Word &result, int message,
                         Word &local, Supplier s);

    private: //static methods
/*
1.5 Support functions

*/

        static bool IsOptimizedForParallelQueryExecution(Supplier s);
/*
Indicates that the operator has a valid local2- object 
that was set in the ParallelQueryOptimizer. The parallel query 
optimizer adds automatically ~par~ operators which are part of the 
ParThread- algebra. However the algebra can't be ruled out in the
query language and every user can set additional par- operators,
even if they can't be parallized in this context. In this case the 
function will return ~false~ and the operator will not be parallized.

*/

        static int ParVMSerial(Word *args, Word &result, int message,
                               Word &local, Supplier s);

        static int ParVMParallel(Word *args, Word &result, int message,
                                 Word &local, Supplier s);
/*
Different implementations of the value mapping function depending on the 
state of the operator. ~ParVMSerial~ is used for a serial execution 
without a valid ParNodeInfo-object set in local2 space. The 
~ParVMParallel~ method uses the stream-handler of the ParNodeInfos execution 
context reference to pass on stream messages.

*/
    };

} // end namespace parThread

#endif /* ALGEBRAS_PARTHREAD_PAROPERATOR */
