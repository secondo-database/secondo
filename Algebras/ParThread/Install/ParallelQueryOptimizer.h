/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

1.2 Imports

*/

#ifndef PARALLEL_QUERY_OPTIMIZER_H
#define PARALLEL_QUERY_OPTIMIZER_H

#include "AlgebraTypes.h"

#include <memory>

class QueryProcessor;

namespace parthread
{


#ifdef USE_MULTIHREADED_QUERY_PROCESSING

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

  void WriteDebugOutput(const std::string message);

private: //member
  std::unique_ptr<ParallelQueryOptimizerImpl> m_pImpl;
};

#else

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

  void WriteDebugOutput(const std::string message);
  {
    //do nothing if ParThread-library is not loaded
  }
};
#endif // USE_MULTIHREADED_QUERY_PROCESSING

} // namespace parthread

#endif // PARALLEL_QUERY_OPTIMIZER_H