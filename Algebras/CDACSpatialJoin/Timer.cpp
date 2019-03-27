/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{2}
\tableofcontents


1 Timer class

*/
#include <assert.h>
#include <iomanip>
#include <sstream>

#include "Timer.h"
#include "Utils.h"

#ifdef TIMER_USES_PAPI
#include "papi.h"
#endif

using namespace cdacspatialjoin;
using namespace std;

/*
1.1 Task class

*/
void Task::reset() {
   count = 0;
   sumTime = 0;
   minTime = 0;
   maxTime = 0;
#ifdef TIMER_USES_PAPI
   sumL1InstrMisses = 0;
   sumL1DataMisses = 0;
   sumL2Misses = 0;
   sumL3Misses = 0;
#endif
}

void Task::add(clock_t time) {
   if (count == 0) {
      sumTime = time;
      minTime = time;
      maxTime = time;
   } else {
      sumTime += time;
      minTime = min(minTime, time);
      maxTime = max(maxTime, time);
   }
   ++count;
}

#ifdef TIMER_USES_PAPI
void Task::add(clock_t time, size_t l1InstrMisses, size_t l1DataMisses,
         size_t l2Misses, size_t l3Misses) {
   add(time);
   sumL1InstrMisses += l1InstrMisses;
   sumL1DataMisses += l1DataMisses;
   sumL2Misses += l2Misses;
   sumL3Misses += l3Misses;
}
#endif

void Task::add(const Task& task) {
   count += task.count;
   sumTime += task.sumTime;
   // several task types may be running consecutively to fulfil a common goal,
   // so it may make some sense to be adding up the minimum and maximum values
   // of the different task types
   minTime += task.minTime;
   maxTime += task.maxTime;
#ifdef TIMER_USES_PAPI
   sumL1InstrMisses += task.sumL1InstrMisses;
   sumL1DataMisses += task.sumL1DataMisses;
   sumL2Misses += task.sumL2Misses;
   sumL3Misses += task.sumL3Misses;
#endif
}

void Task::report(ostream& out) {
   // determine the common output length for runtime values
   constexpr int timeLength = 8;

   out << setfill(' ');
   out << name << ": ";
   out << "total " << setw(timeLength) << formatMillis(sumTime)
       << " (" << formatInt(count) << " calls, "
       << "avg " << setw(timeLength) << formatMillis(sumTime / count) << ", "
       << "min " << setw(timeLength) << formatMillis(minTime) << ", "
       << "max " << setw(timeLength) << formatMillis(maxTime) << ")";

#ifdef TIMER_USES_PAPI
   out << "; Cache Misses: "
       << "L1-Instruction: " << formatInt(sumL1InstrMisses) << ", "
       << "L1-Data: " << formatInt(sumL1DataMisses) << "; "
       << "L2: " << formatInt(sumL2Misses) << "; "
       << "L3: " << formatInt(sumL3Misses);
#endif
   out << endl;
}

void Task::reportTable(std::ostream& out, const bool reportCount,
                        const bool reportSum, const bool reportAvg,
                        const bool reportMin, const bool reportMax,
                        const unsigned maxNameLength) const {

   // set fill character ' ' (is set to '0' by Query Progress Estimation)
   out << setfill(' ');

   // report the name and add spaces to align all names to the same length
   out << name;
   if (name.size() < maxNameLength)
      out << string(maxNameLength - name.size(), ' ');
   out << " |";

   // report the desired runtime information
   if (reportCount)
      out << setw(countWidth - 1) << formatInt(count) << " |";
   if (reportSum)
      out << setw(sumTimeWidth - 1) << formatMillis(sumTime) << " |";
   if (reportAvg)
      out << setw(avgTimeWidth - 1) << formatMillis(getAvgTime()) << " |";
   if (reportMin)
      out << setw(minTimeWidth - 1) << formatMillis(minTime) << " |";
   if (reportMax)
      out << setw(maxTimeWidth - 1) << formatMillis(maxTime) << " |";

#ifdef TIMER_USES_PAPI
   // report the desired cache miss information
   if (reportSum) {
      out << setw(sumL1IWidth - 1) << formatInt(sumL1InstrMisses) << " |";
      out << setw(sumL1DWidth - 1) << formatInt(sumL1DataMisses) << " |";
      out << setw(sumL2UWidth - 1) << formatInt(sumL2Misses) << " |";
      out << setw(sumL3UWidth - 1) << formatInt(sumL3Misses) << " |";
   }
   if (reportAvg) {
      out << setw(avgL1IWidth - 1) << formatInt(getAvgL1InstrCacheMisses())
          << " |";
      out << setw(avgL1IWidth - 1) << formatInt(getAvgL1DataCacheMisses())
          << " |";
      out << setw(avgL1IWidth - 1) << formatInt(getAvgL2CacheMisses()) << " |";
      out << setw(avgL1IWidth - 1) << formatInt(getAvgL3CacheMisses()) << " |";
   }
#endif
   out << endl;
}


/*
1.2 Timer class

*/
#ifdef TIMER_USES_PAPI
bool Timer::papiCountersRunning = false;

// determine the hardware events to be counted on this system. Note that the
// Timer and Task classes are 'hard coded' for the cache events listed here
int Timer::papiEvents[papiEventCount] {
     PAPI_L1_ICM,  // Level 1 instruction cache misses
     PAPI_L1_DCM,  // Level 1 data cache misses
     PAPI_L2_TCM,  // Level 2 total cache misses
     PAPI_L3_TCM,  // Level 3 total cache misses
     // from these values, other information may be derived:
     // - PAPI_L1_TCM = PAPI_L1_ICM + PAPI_L1_DCM
     // - PAPI_L2_TCA = PAPI_L1_TCM
     // - PAPI_L3_TCA = PAPI_L2_TCM
};
#endif

Timer::Timer(const unsigned taskCount) :
      tasks { taskCount } {

   // instantiate the given number of tasks, assigning them standard names
   // "Task 0", "Task 1" etc.
   for (unsigned i = 0; i < taskCount; ++i) {
      stringstream name;
      name << "Task " << i;
      tasks[i].name = name.str();
   }
   maxNameLength = getMaxNameLength();
}

Timer::Timer(const vector<string>& taskNames) :
        tasks { taskNames.size() } {

   // instantiate the (implicitly) given number of tasks, assigning them the
   // names given by the taskNames vector
   unsigned task = 0;
   for (const string& taskName : taskNames)
      tasks[task++].name = taskName;
   maxNameLength = getMaxNameLength();
}

Timer::~Timer() {
   stop();
}

void Timer::start(const unsigned taskID /* = 0 */) {
   if (currentTask != NO_TASK)
      stop();

   assert (taskID < tasks.size());

   startTime = clock();

#ifdef TIMER_USES_PAPI
   // ensure that no Task is currently running in another Timer instance
   // (this is only necessary if PAPI is being used, as otherwise overlapping
   // measurements pose no problem)
   assert (!papiCountersRunning);
   // set the static variable 'papiCountersRunning' to true to prevent other
   // Timer instances from simultaneously starting a task
   papiCountersRunning = true;

   // Start counting hardware events
   if (PAPI_start_counters(papiEvents, papiEventCount) != PAPI_OK)
      handlePapiError(1);
#endif

   currentTask = taskID;
}

clock_t Timer::stop() {
   if (currentTask < 0)
      return 0;

   lastTime = clock() - startTime;

#ifdef TIMER_USES_PAPI
   // Stop counting hardware events
   long long int papiValues[papiEventCount];
   if (PAPI_stop_counters(papiValues, papiEventCount) != PAPI_OK)
      handlePapiError(1);
   papiCountersRunning = false;

   lastL1InstrMisses = static_cast<size_t>(papiValues[0]);
   lastL1DataMisses = static_cast<size_t>(papiValues[1]);
   lastL2Misses = static_cast<size_t>(papiValues[2]);
   lastL3Misses = static_cast<size_t>(papiValues[3]);
   tasks[currentTask].add(lastTime, lastL1InstrMisses, lastL1DataMisses,
           lastL2Misses, lastL3Misses);
#else
   tasks[currentTask].add(lastTime);
#endif

   currentTask = NO_TASK;
   return lastTime;
}

Task* Timer::getCurrentTask() {
   return (currentTask == NO_TASK) ? nullptr : &tasks[currentTask];
}

Task* Timer::getTask(unsigned taskID) {
   assert (taskID < tasks.size());
   return &tasks[taskID];
}

void Timer::reset() {
   // stop the last task, if any
   stop();
   // reset all tasks
   for (Task& task : tasks)
      task.reset();
}

void Timer::reportTable(std::ostream& out, const bool reportCount,
                        const bool reportSum, const bool reportAvg,
                        const bool reportMin, const bool reportMax) const {

   // set fill character ' ' (is set to '0' by Query Progress Estimation)
   out << setfill(' ');
   out << endl;

   // 1. print table header

   // a) name column
   out << string(maxNameLength + 1, ' ') << "|";

   // b) count and time columns
   if (reportCount)
      out << setw(Task::countWidth - 1) << right << "count" << " |";
   if (reportSum)
      out << setw(Task::sumTimeWidth - 1) << right << "total time" << " |";
   if (reportAvg)
      out << setw(Task::avgTimeWidth - 1) << right << "avg time" << " |";
   if (reportMin)
      out << setw(Task::minTimeWidth - 1) << right << "min time" << " |";
   if (reportMax)
      out << setw(Task::maxTimeWidth - 1) << right << "max time" << " |";

#ifdef TIMER_USES_PAPI
   // c) cache columns
   if (reportSum) {
      out << setw(Task::sumL1IWidth - 1) << right << "L1-Instr Miss" << " |";
      out << setw(Task::sumL1DWidth - 1) << right << "L1-Data Miss" << " |";
      out << setw(Task::sumL2UWidth - 1) << right << "L2 Miss" << " |";
      out << setw(Task::sumL3UWidth - 1) << right << "L3 Miss" << " |";
   }
   if (reportAvg) {
      out << setw(Task::avgL1IWidth - 1) << right << "L1-I Miss-avg" << " |";
      out << setw(Task::avgL1DWidth - 1) << right << "L1-D Miss-avg" << " |";
      out << setw(Task::avgL2UWidth - 1) << right << "L2 Miss-avg" << " |";
      out << setw(Task::avgL3UWidth - 1) << right << "L3 Miss-avg" << " |";
   }
#endif
   out << endl;

   // -----------------------------------------------------

   Task sum;
   for (unsigned part = 0; part < 2; ++part) {
      // 2. = 4. print horizontal separator

      // a) name column
      out << string(maxNameLength + 1, '-') << "+";

      // b) count and time columns
      if (reportCount)
         out << string(Task::countWidth, '-') << "+";
      if (reportSum)
         out << string(Task::sumTimeWidth, '-') << "+";
      if (reportAvg)
         out << string(Task::avgTimeWidth, '-') << "+";
      if (reportMin)
         out << string(Task::minTimeWidth, '-') << "+";
      if (reportMax)
         out << string(Task::maxTimeWidth, '-') << "+";

#ifdef TIMER_USES_PAPI
      // c) cache columns
      if (reportSum) {
         out << string(Task::sumL1IWidth, '-') << "+";
         out << string(Task::sumL1DWidth, '-') << "+";
         out << string(Task::sumL2UWidth, '-') << "+";
         out << string(Task::sumL3UWidth, '-') << "+";
      }
      if (reportAvg) {
         out << string(Task::avgL1IWidth, '-') << "+";
         out << string(Task::avgL1DWidth, '-') << "+";
         out << string(Task::avgL2UWidth, '-') << "+";
         out << string(Task::avgL3UWidth, '-') << "+";
      }
#endif
      out << endl;

      if (part == 0) {
         // 3. print table body
         for (const Task& task : tasks) {
            task.reportTable(out, reportCount, reportSum, reportAvg,
                             reportMin, reportMax, maxNameLength);
            sum.add(task);
         }

      } else {
         // 5. print sum row
         sum.reportTable(out, reportCount, reportSum, reportAvg,
                         reportMin, reportMax, maxNameLength);
      }
      if (tasks.size() == 1)
         break; // no sum needed for just one task
   }
   out << endl;
}

unsigned Timer::getMaxNameLength() const {
   size_t result = 0;
   for (const Task& task : tasks)
      result = max(result, task.name.size());
   return static_cast<unsigned>(result);
}

#ifdef TIMER_USES_PAPI
void Timer::handlePapiError(const int result) {
   printf("PAPI error %d: %s\n", result, PAPI_strerror(result));
   exit(1);
}
#endif
