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

\setcounter{tocdepth}{3}
\tableofcontents



1 The Timer class

*/
#pragma once
#include <string>
#include <sstream>
#include <ctime>  /* clock_t, clock, CLOCKS_PER_SEC */

namespace pointcloud2 {

class Timer {
public:
    enum UNIT { nanos, millis, secs };

private:
    /* the number of chars reserved for numbers */
    size_t NUM_WIDTH = 10;
    /* the time unit in which to report tasks */
    UNIT _unit;
    /* the start time of the current task */
    clock_t _startTime;
    /* the total time sum of all completed tasks */
    clock_t _totalTime;
    /* the number of completed tasks */
    size_t _completedTaskCount;
    /* the name of the current task */
    std::string _currentTaskName;
    /* the report for the last task */
    std::string _lastTaskReport;
    /* the length of the longest line in the report */
    size_t _maxLineLength;
    /* the report of all tasks performed since the last reset */
    std::stringstream _report;

public:
    Timer(const UNIT unit);

    ~Timer() = default;

    /* resets the timer and clears the report */
    void reset();

    /* starts the next task. If a task was started, it is automatically
     * stopped and its time added to the report */
    void startTask(const std::string task);

    /* stops the current task. This is called automatically if another
     * task is started or a report for the last task is retrieved */
    void stopTask();

    /* returns the report for the last task (with no line breaks at the end).
     * If the last task is still running, it will be stopped */
    std::string getReportForLastTask();

    /* returns a report for all tasks started since instantiation or last
     * reset of the Timer */
    std::string getReportForAllTasks();

    /* returns the report stream to allow for additional information
     * to be inserted into the stream. Note that the current task is
     * stopped. */
    std::stringstream& getReportStream();

    /* returns a string to add adequate inset to the report stream */
    std::string getInset();

private:
    Timer(const Timer& timer); // cannot copy stringstream

    std::string getLine(const double duration, const std::string text);

    static std::string format(const double num);
};

} /* namespace pointcloud2 */
