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

1 Header File: ExecutionContextLogger

September 2019, Fischer Thomas

1.1 Overview

Helper classes and types used in the execution context scope 

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

/*
1.3 ExecutionContextLogger

This class is used to write the messages to the command line and trace task states. 
It has two modes:

  1. ~DebugOutput~: only writes messages to the command line considering concurrent
                   access.

  2. ~TraceOutput~: writes the tasktrace to file. ~WriteTaskTrace~ creates a new 
                    record to the file with information like the Tasks start time,
                    context, entity ...

*/

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



/*
1.4 ExecutionContextStates

Enum containing the different states the execution context goes through during
execution. They have the same meaning like the stream messages.

*/
  enum class ExecutionContextStates
  {
    Created = 0,
    Initialized,
    Opened,
    Closed,
    Canceled,
    Finished
  };

/*
1.5 ExecutionContextSetting

Settings read from secondos config file 

*/
  struct ExecutionContextSetting
  {
    size_t QueueCapacityThreshold;
/*
The maximum capacity of tuple blocks in the queue to start producers tasks. 
If the buffers capacity exceeds the threshold, no producers are triggered.  

*/
    size_t MaxNumberOfConcurrentThreads;
/*
Number of working threads processing execution contexts in parallel. 
The multithreaded query processing needs at least 2 threads. If the property is
set to 1, the regular, single threaded query processing is used. All values < 1  
let the query processor choose the number of concurrent threads depending on the 
number of processors (cores) used by the system.

*/
    size_t MaxNumberOfTuplesPerBlock;
/*
Maximum number of tuples stored in one data block. After a data block is 
complete filled or the stream answered cancel, the block is submitted to the 
shared buffer and is available for consuming par operators.

*/
    size_t TotalBufferSizeInBytes;
/*
Maximum buffer size in MB for all execution contexts in total. If the value is set
to 0, then the buffers size will be adjusted by the query processor as part of 
the operator memory assignment. If the query processor assigned less memory to 
the related par-Operator as the automatic memory distribution this setting is 
ignored. 

*/
    size_t MaxDegreeOfDataParallelism;

/*
A value of >1 enables the use of multiple threads to execute instances
of the same execution context in parallel. Set to 1 indicates that only 
independent and pipeline parallelism is used.

*/
    clock_t TimeoutPerTaskInMicroSeconds;
/*
The maximum time in microseconds per request task. The timeout is checked after 
every tuple block is completly filled. If set to 0 the timeout is not used and 
the task ends after the first filled tuple block

*/

    bool UsePipelineParallelism;
/*
A value of 1 enables the use of pipeline parallelism, otherwise it is deactivated

*/

    bool UseOptimization;
/*
A value 1 enables the automatic integration of parallel oparators. Set to 0
disables the parallel optimization, but all user defined parallel operators 
will still be checked and removed if inappropriate inserted in the query 
expression.

*/

    std::shared_ptr<ExecutionContextLogger> Logger;
/*
The logger mechanism used to write information to the command line or 
separate files.

*/

    std::set<std::string> OperatorsSupportingDataParallelism;
/*
A set of operatornames. Only the operatores listed here can be part of a 
dataparallized context. 

*/

    std::set<std::string> OperatorsRetainingOrder;
/*
A set of operatornames. Operators which rely on a tuple order. All contexts 
after such a operator can't use dataparallelism, because this can change the 
order of tuples. Value is a list of operator names delimitered by spaces. 

*/

    std::set<std::string> OperatorsRequiringHashPartitioning;
/*
A set of operatornames. Operators in this list need a partitioning by hashvalues 
to work correct with dataparallelism.

*/
  };


/*
1.6 ExecutionContextGuard

A synchronisation mechanism to controll access to child contexts. The ~Pass~
method returns ~true~ after a defined number of different entities passed the
guard and reached a defined state. Entities are registered to the guard by 
the ~SetState~ method.

*/
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

/*
1.7 ThreadSpecificStopWatch

A helper class for time measurement. The stop watch only measures the cpu time
for the thread creating the class. ~SpendTimeInMicroseconds~ returns the time
span in micro seconds from initialization to the timepoint of the methodcall.

*/
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
