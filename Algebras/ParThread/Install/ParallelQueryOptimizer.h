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

1 Header File: ParallelQueryOptimizer

September 2019, Fischer Thomas

1.1 Overview

The ~ParallelQueryOptimizer~ is part of the ~QueryProcessor~ and responsible
to parallelize parts of the query tree. This is realized by ~par~-operators, 
which separate the operator tree in so called execution contexts. Each context
can be executed concurrently with other contexts. 

1.2 Imports

*/

#ifndef PARALLEL_QUERY_OPTIMIZER_H
#define PARALLEL_QUERY_OPTIMIZER_H

#include "AlgebraTypes.h"

#include <memory>

class QueryProcessor;

namespace parthread
{
/*
1.3 ParallelQueryOptimizer

There are two different implementations of the ~ParallelQueryOptimizer~. Which one 
is used in the ~QueryProcessor~ depends if the ~parThread~-algebra is activated.
The algebra uses the define bellow to switch between to code segments. 

*/
#ifdef USE_MULTIHREADED_QUERY_PROCESSING


/*
If the algebra is activated, the ~ParallelQueryOptimizer~ tries to read some 
settings from secondos configuration file. This is done once. 

For each new query, the ~QueryProcessor~ calls ~ParallelizeQueryPlan~ and passes
the generated operator tree to the method. The method initializes the execution
contexts by investigating and validating the par-operators. It is able to insert
 ~par~ operators by simple rules.

After ~ParallelQueryOptimizer~ modified the operator tree for parallel execution
it is evaluated by the ~QueryProcessor~ in a usual way.

*/
class ParallelQueryOptimizer
{
public: //types
 static const char *ParOperatorName;

private: //types
  struct ParallelQueryOptimizerImpl;

public: //methods
  ParallelQueryOptimizer();

  ~ParallelQueryOptimizer();

  void ParallelizeQueryPlan(QueryProcessor *queryProcessor, void *queryPlan,
                            size_t memorySpent, int noMemoryOperators);

private: //member
  std::unique_ptr<ParallelQueryOptimizerImpl> m_pImpl;
};

#else
/*
If the ~parThread~-algebra is not activated the implementation is replaced by an
empty ~ParallelizeQueryPlan~ method. The ~QueryProcessor~ passes the ~queryPlan~,
but it is not changed and the regular, sequential execution is used. 

*/
class ParallelQueryOptimizer
{
public: //methods
  ParallelQueryOptimizer() = default;

  ~ParallelQueryOptimizer() = default;

  void ParallelizeQueryPlan(QueryProcessor *queryProcessor, void *queryPlan,
                            size_t memorySpent, int noMemoryOperators)
  {
    //do nothing if ParThread-library is not loaded
  };

};
#endif // USE_MULTIHREADED_QUERY_PROCESSING

} // namespace parthread

#endif // PARALLEL_QUERY_OPTIMIZER_H