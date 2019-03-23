/*
1 Timer class

This class provides measurement of runtime and cache misses for a given number
of tasks. Measurement for a given tasks may me started and stopped many times;
in this case, the number of calls, the sum (of runtime and cache misses), and
the average can be retrieved.

Measurement of cache misses requires the Performance API (PAPI) to be installed
and TIMER\_USES\_PAPI to be defined (see below).


1.1 includes

*/

#pragma once

#include <vector>
#include <ctime>
#include <iostream>
#include <ostream>

#include "Base.h"

/*
1.2 Performance API (PAPI) usage

If TIMER\_USES\_PAPI is defined, the Performance API (PAPI) will be
used in the Timer class to count and report cache misses. To use PAPI,

  * download and extract PAPI from https://icl.utk.edu/papi

  * follow INSTALL.txt to compile ('make'), test ('make fulltest') and
    install ('sudo make install-all') PAPI;

  * in secondo/makefile.algebras, e.g. below the line 'ALGEBRAS +=
    CDACSpatialJoinAlgebra', add the line

---- ALGEBRA_LINK_FLAGS += -L/usr/local/lib64 -lpapi
----

  * in the (hidden) file .bashrc in the home directory, add

----  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib64
----

  * restart the computer and recompile SECONDO with TIMER\_USES\_PAPI
    defined.

*/
#define TIMER_USES_PAPI

namespace cdacspatialjoin {

/*
1.3 Task class

Provides statistical information on the number of calls, the sum, minimum,
maximum, and average runtime, and the sum and average cache misses on
L1 Instruction, L1 Data, L2 (unified), and L3 (unified) caches.

*/
class Task {
   // spacing
   static const unsigned countWidth = 8;
   static const unsigned sumTimeWidth = 13;
   static const unsigned avgTimeWidth = sumTimeWidth;
   static const unsigned minTimeWidth = sumTimeWidth;
   static const unsigned maxTimeWidth = sumTimeWidth;
   static const unsigned sumL1IWidth = 15;
   static const unsigned sumL1DWidth = sumL1IWidth;
   static const unsigned sumL2UWidth = sumL1IWidth;
   static const unsigned sumL3UWidth = sumL1IWidth;
   static const unsigned avgL1IWidth = 15;
   static const unsigned avgL1DWidth = avgL1IWidth;
   static const unsigned avgL2UWidth = avgL1IWidth;
   static const unsigned avgL3UWidth = avgL1IWidth;

   /* the name of this task used in output */
   std::string name = "";

   /* the number of times this task was started and stopped so far */
   size_t count = 0;

   /* the total runtime of all calls to this task */
   clock_t sumTime = 0;

   /* the minimum runtime between starting and stopping this task */
   clock_t minTime = 0;

   /* the maximum runtime between starting and stopping this task */
   clock_t maxTime = 0;

#ifdef TIMER_USES_PAPI
   /* the total number of Level 1 Instruction Cache misses measured while
    * this task was running */
   size_t sumL1InstrMisses = 0;

   /* the total number of Level 1 Data Cache misses measured by PAPI while
    * this task was running */
   size_t sumL1DataMisses = 0;

   /* the total number of Level 2 (Unified) Cache misses measured by PAPI while
    * this task was running */
   size_t sumL2Misses = 0;

   /* the total number of Level 3 (Unified) Cache misses measured by PAPI while
    * this task was running */
   size_t sumL3Misses = 0;
#endif

public:
   /* constructor, sets all statistical data to zero */
   Task() = default;

   /* destructor */
   ~Task() = default;

   /* reports the statistical data compiled for this task to one (non-tabular)
    * line on the given output */
   void report(std::ostream& out);

   /* reports the statistical data compiled for this task to one table row on
    * the given output. The given parameters determine which data will be
    * printed */
   void reportTable(std::ostream& out, bool reportCount, bool reportSum,
                    bool reportAvg, bool reportMin, bool reportMax,
                    unsigned maxNameLength) const;

   /* returns the number of times this task was started and stopped so far */
   size_t getCount() const { return count; }

   /* returns the total runtime of all calls to this task */
   clock_t getTimeSum() const { return sumTime; }

   /* returns the minimum runtime between starting and stopping this task */
   clock_t getMinTime() const { return minTime; }

   /* returns the maximum runtime between starting and stopping this task */
   clock_t getMaxTime() const { return maxTime; }

   /* returns the average runtime between starting and stopping this task */
   clock_t getAvgTime() const { return sumTime / count; }

#ifdef TIMER_USES_PAPI
   /* returns the total number of Level 1 Instruction Cache misses measured
    * while this task was running */
   size_t getL1InstrCacheMisses() const { return sumL1InstrMisses; }

   /* returns the total number of Level 1 Data Cache misses measured
    * while this task was running */
   size_t getL1DataCacheMisses() const { return sumL1DataMisses; }

   /* returns the total number of Level 2 (Unified) Cache misses measured
    * while this task was running */
   size_t getL2CacheMisses() const { return sumL2Misses; }

   /* returns the total number of Level 3 (Unified) Cache misses measured
    * while this task was running */
   size_t getL3CacheMisses() const { return sumL3Misses; }

   /* returns the average number of Level 1 Instruction Cache misses measured
    * while this task was running */
   size_t getAvgL1InstrCacheMisses() const { return sumL1InstrMisses / count; }

   /* returns the average number of Level 1 Data Cache misses measured
    * while this task was running */
   size_t getAvgL1DataCacheMisses() const { return sumL1DataMisses / count; }

   /* returns the average number of Level 2 (Unified) Cache misses measured
    * while this task was running */
   size_t getAvgL2CacheMisses() const { return sumL2Misses / count; }

   /* returns the average number of Level 3 (Unified) Cache misses measured
    * while this task was running */
   size_t getAvgL3CacheMisses() const { return sumL3Misses / count; }
#endif

private:
   /* resets all statistical data of this Task to zero */
   void reset();

   /* adds the given runtime to the statistical data and increases count by 1.
    * From the Timer class, this method should only be called if PAPI is
    * inactive */
   void add(clock_t time);

   /* adds the statistical data of the given task to the statistical data of
    * this class */
   void add(const Task& task);

#ifdef TIMER_USES_PAPI
   /* adds the given runtime and cache misses to the statistical data and
    * increases count by 1. This method should be called when PAPI is active */
   void add(clock_t time, size_t l1InstrMisses, size_t l1DataMisses,
           size_t l2Misses, size_t l3Misses);
#endif

   friend class Timer;
};


/*
1.4 Timer class

*/
class Timer {
   /* is used as the currentTask value while no task is running */
   static constexpr int NO_TASK = -1;

#ifdef TIMER_USES_PAPI
   /* true, if a Task was started in any Timer instance. While the
    * instantiation of multiple Timers may be useful, at any given time only
    * one of them may measure a running Task using PAPI. */
   static bool papiCountersRunning;

   /* the number of hardware events counted */
   static constexpr unsigned papiEventCount = 4;

   /* the ids of hardware events counted. For a list of available events,
    * see the papi.h file and PAPI documentation */
   static int papiEvents[papiEventCount];
#endif

   /* the different task types managed by this Timer */
   std::vector<Task> tasks;

   /* the maximum length found in the names of the Task instances */
   unsigned maxNameLength;

   /* the index position of the currently running task in the tasks vector,
    * or NO_TASK if currently none of the observed tasks is running */
   int currentTask = NO_TASK;

   /* the time currentTask was started */
   clock_t startTime = 0;

   /* the runtime measured for the Task call that was last stopped (kept here
    * to support console output for the caller) */
   clock_t lastTime = 0;

#ifdef TIMER_USES_PAPI
   /* the number of Level 1 Instruction Cache misses measured for the Task
    * call that was last stopped */
   size_t lastL1InstrMisses = 0;

   /* the number of Level 1 Data Cache misses measured for the Task
    * call that was last stopped */
   size_t lastL1DataMisses = 0;

   /* the number of Level 2 (Unified) Cache misses measured for the Task
    * call that was last stopped */
   size_t lastL2Misses = 0;

   /* the number of Level 3 (Unified) Cache misses measured for the Task
    * call that was last stopped */
   size_t lastL3Misses = 0;
#endif

public:
   /* instantiates a Timer for the given number of different task types and
    * assigns standard names (Task 0, Task 1, etc.) to these tasks */
   explicit Timer(unsigned taskCount);

   /* instantiates a Timer for the number of different task types provided
    * by the given vector of task names */
   explicit Timer(const std::vector<std::string>& taskNames);

   /* destructor */
   ~Timer();

   /* starts time measurement for the task with the given ID which must be
    * between 0 (inclusive) and the taskCount (exclusive) given in the Timer
    * constructor */
   void start(unsigned taskID = 0);

   /* stops the Task call which was last started and stores the measured
    * runtime and cache misses to the variables that are accessible by the
    * getLast...() functions. Returns the measured runtime. */
   clock_t stop();

   /* returns a pointer to the Task currently running (or nullptr if none of
    * the observed tasks is currently running) */
   Task* getCurrentTask();

   /* returns a pointer to the Task with the given ID which must be
    * between 0 (inclusive) and the taskCount (exclusive) given in the Timer
    * constructor */
   Task* getTask(unsigned taskID);

   /* resets to zero all statistical data of all Tasks managed by this timer */
   void reset();

   /* reports to the given output a table with the statistical data compiled
    * for all Tasks managed by this timer. The given parameters determine
    * which data will be printed */
   void reportTable(std::ostream& out, bool reportCount,
                    bool reportSum, bool reportAvg,
                    bool reportMin, bool reportMax) const;

   /* returns the runtime measured for the Task call that was last stopped */
   clock_t getLastTime() const { return lastTime; }

#ifdef TIMER_USES_PAPI
   /* returns the number of Level 1 Instruction Cache misses measured for the
    * Task call that was last stopped */
   size_t getLastL1InstrCacheMisses() const { return lastL1InstrMisses; }

   /* returns the number of Level 1 Data Cache misses measured for the
    * Task call that was last stopped */
   size_t getLastL1DataCacheMisses() const { return lastL1DataMisses; }

   /* returns the number of Level 2 (Unified) Cache misses measured for the
    * Task call that was last stopped */
   size_t getLastL2CacheMisses() const { return lastL2Misses; }

   /* returns the number of Level 3 (Unified) Cache misses measured for the
    * Task call that was last stopped */
   size_t getLastL3CacheMisses() const { return lastL3Misses; }
#endif

private:
#ifdef TIMER_USES_PAPI
   /* prints the given error to the console and exits the program */
   static void handlePapiError(int result);
#endif

   /* returns the maximum length found in the names of the Task instances */
   unsigned getMaxNameLength() const;
};

} // end of namespace cdacspatialjoin