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

1 Header File: ExecutionContextLogger

September 2019, Fischer Thomas

1.1 Overview

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_EXECUTIONCONTEXTTYPES_H
#define SECONDO_PARTHREAD_EXECUTIONCONTEXTTYPES_H

#include "DebugWriter.h"

#include <string>
#include <memory>
#include <vector>
#include <future>
#include <tuple>
#include <map>
#include <chrono>
#include <assert.h>

namespace parthread
{
  namespace sc = std::chrono;

class ExecutionContextLogger
{

public: //types
  enum class LoggerModes
  {
    NoLogging = 0,
    DebugOutput,
    TraceOutput,
    CompleteOutput = TraceOutput
  };

public: //methods
  ExecutionContextLogger(LoggerModes mode, const std::string &outputPath)
      : m_idGenerator(1), m_loggerMode(mode), m_loggerPath(outputPath)
  {
    m_startTime = sc::high_resolution_clock::now();
  };

  ~ExecutionContextLogger() = default;

  bool DebugMode()
  {
    return m_loggerMode > LoggerModes::NoLogging;
  }

  bool TraceMode()
  {
    return m_loggerMode == LoggerModes::TraceOutput;
  }

  void WriteDebugOutput(const std::string &message)
  {
    if (DebugMode())
    {
      std::thread::id tid = std::this_thread::get_id();
      sc::high_resolution_clock::time_point endTime = 
      sc::high_resolution_clock::now();
      int64_t ellapsedTimeInUs = 
      sc::duration_cast<sc::microseconds>(endTime - m_startTime).count();
      
      std::stringstream debugText;
      debugText << message << " after " << ellapsedTimeInUs << " us";

      m_writer.write(true, std::cout, &tid, 0, debugText.str());
    }
  };

  int NextContextId()
  {
    return m_idGenerator++;
  }

  const std::string &LoggerDebugPath()
  {
    return m_loggerPath;
  }

private:
  std::atomic_int m_idGenerator;
  DebugWriter m_writer;
  LoggerModes m_loggerMode;
  std::mutex m_accessMtx;
  std::string m_loggerPath;
  sc::high_resolution_clock::time_point m_startTime;
};

enum class ExecutionContextStates
{
  Created = 0,
  Initialized,
  Opened,
  Closed,
  Canceled,
  Finished
};

struct ExecutionContextSetting
{
  size_t QueueCapacityThreshold;
  size_t MaxNumberOfConcurrentThreads;
  size_t MaxNumberOfTuplesPerBlock;
  size_t TotalBufferSizeInBytes;
  size_t MaxDegreeOfDataParallelism;
  bool UsePipelineParallelism;
  bool UseOptimization;
  std::shared_ptr<ExecutionContextLogger> Logger;
  std::set<std::string> OperatorsSupportingDataParallelism;
  std::set<std::string> OperatorsRetainingOrder;
  std::set<std::string> OperatorsRequiringHashPartitioning;
};

class ExecutionContextGuard
{
public:
  ExecutionContextGuard(const size_t numEntities)
      : m_numEntities(numEntities){};

  ~ExecutionContextGuard() = default;

  void Reset()
  {
    std::unique_lock<std::mutex> lock(m_accessMtx);
    m_stateMap.clear();
  };

  void SetState(const int id, ExecutionContextStates state)
  {
    std::unique_lock<std::mutex> lock(m_accessMtx);
    m_stateMap[id] = state;
  };

  bool Pass(ExecutionContextStates state)
  {
    std::unique_lock<std::mutex> lock(m_accessMtx);
    assert(m_stateMap.size() <= m_numEntities);
    if (m_stateMap.size() < m_numEntities)
    {
      return false;
    }

    for (std::pair<int, ExecutionContextStates> keyValue : m_stateMap)
    {
      if (keyValue.second != state)
      {
        return false;
      }
    }
    return true;
  };

private:
  size_t m_numEntities;
  typedef std::map<int, ExecutionContextStates> CtxStateMap;
  CtxStateMap m_stateMap;
  std::mutex m_accessMtx;
};

} // namespace parthread
#endif
