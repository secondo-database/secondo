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

#include "boost/filesystem.hpp"

#include <string>
#include <memory>
#include <vector>
#include <future>
#include <tuple>
#include <map>
#include <chrono>
#include <assert.h>
#include <fstream>

namespace fs = boost::filesystem;

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
      FullOutput = TraceOutput
    };

    struct TaskTrace
    {
      std::thread::id Tid;
      int64_t Start;
      int64_t End;
      std::string TaskType;
      int ContextNo;
      int InstanceNo;
      size_t ProcessedTuples;
    };

  public: //methods
    ExecutionContextLogger(LoggerModes mode, const std::string &outputPath)
        : m_loggerMode(mode), m_loggerPath(outputPath)
    {
      m_startTime = sc::high_resolution_clock::now();
    };

    ~ExecutionContextLogger()
    {
      WriteOutTrace();
    }

    void ResetTimer()
    {
      m_startTime = sc::high_resolution_clock::now();
    }

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
        double ellapsedTime =
            sc::duration_cast<sc::milliseconds>(endTime - m_startTime).count();

        double ellapsedTimeInSec = ellapsedTime / 1E3;

        std::stringstream debugText;
        debugText << message << " after " << std::fixed << std::setprecision(4)
                  << ellapsedTimeInSec << " sec";

        m_writer.write(true, std::cout, &tid, 0, debugText.str());
      }
    };

    void WriteTaskTrace(sc::_V2::high_resolution_clock::time_point &startT,
                        const std::string &taskType, const int contextId,
                        const size_t processedTuples, const int entityId)
    {
      if (TraceMode())
      {
        TaskTrace taskTrace;
        taskTrace.Tid = std::this_thread::get_id();

        taskTrace.Start =
            sc::duration_cast<sc::microseconds>(startT - m_startTime).count();

        sc::high_resolution_clock::time_point endTime =
            sc::high_resolution_clock::now();

        taskTrace.End =
            sc::duration_cast<sc::microseconds>(endTime - m_startTime).count();

        taskTrace.TaskType = taskType;
        taskTrace.ContextNo = contextId;
        taskTrace.InstanceNo = entityId;
        taskTrace.ProcessedTuples = processedTuples;

        m_accessMtx.lock();
        m_taskTraces.push_back(taskTrace);
        m_accessMtx.unlock();
      }
    }

    void WriteOutTrace()
    {
      if (m_taskTraces.size() > 0)
      {
        fs::path outputPath(m_loggerPath);
        outputPath /= "taskTrace.csv";

        std::ofstream traceLogFile;
        const std::string separator = "\t";
        traceLogFile.open(outputPath.c_str());
        traceLogFile << "thread" << separator
                     << "start" << separator
                     << "end" << separator
                     << "context" << separator
                     << "type" << separator
                     << "numtuples" << std::endl;

        int64_t graphStart = INT64_MAX;
        for (const TaskTrace &taskTrace : m_taskTraces)
        {
          graphStart = std::min(graphStart, taskTrace.Start);
        }

        int asciiUpperLetter = 65; //start with "A"
        std::map<std::thread::id, std::string> threadIdMapping;
        for (const TaskTrace &taskTrace : m_taskTraces)
        {
          std::map<std::thread::id, std::string>::iterator
              found = threadIdMapping.find(taskTrace.Tid);
          std::string tid;

          if (found == threadIdMapping.end())
          {
            threadIdMapping[taskTrace.Tid] = char(asciiUpperLetter);
            asciiUpperLetter++;
            tid = (char)asciiUpperLetter;
          }
          else
          {
            tid = found->second;
          }

          traceLogFile << "\"" << tid << "\"" << separator
                       << taskTrace.Start - graphStart << separator
                       << taskTrace.End - graphStart << separator
                       << "\"" << taskTrace.ContextNo << "-"
                       << taskTrace.InstanceNo << "\"" << separator
                       << taskTrace.TaskType << separator
                       << taskTrace.ProcessedTuples << std::endl;
        }

        traceLogFile.close();
        m_taskTraces.clear();
      }
    }

    const std::string &LoggerDebugPath()
    {
      return m_loggerPath;
    }

  private:
    DebugWriter m_writer;
    LoggerModes m_loggerMode;
    std::mutex m_accessMtx;
    std::string m_loggerPath;
    sc::high_resolution_clock::time_point m_startTime;

    std::list<TaskTrace> m_taskTraces;
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
    clock_t TimeoutPerTaskInMicroSeconds;
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

  class ThreadSpecificStopWatch
  {

  public:
    ThreadSpecificStopWatch()
    {
      clock_gettime(CLOCK_THREAD_CPUTIME_ID, &m_startTime);
    }

    time_t SpendTimeInMicroseconds()
    {
      timespec stopTime;
      clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stopTime);

      time_t ms = (stopTime.tv_sec - m_startTime.tv_sec) * 1E6;
      time_t us = (stopTime.tv_nsec - m_startTime.tv_nsec) / 1E3;

      return ms + us;
    }

  private:
    timespec m_startTime;
  };

} // namespace parthread
#endif
