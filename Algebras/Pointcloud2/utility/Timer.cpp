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

#include "Timer.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include "MathUtils.h"

namespace pointcloud2 {
Timer::Timer(const UNIT unit) {
    _unit = unit;
    reset();
}

void Timer::reset() {
    _startTime = clock();
    _totalTime = 0;
    _completedTaskCount = 0;
    _currentTaskName = "";
    _lastTaskReport = "";
    _maxLineLength = 0;
    _report.clear();
}

void Timer::startTask(const std::string task) {
    stopTask();
    _currentTaskName = task;
    _startTime = clock();
}

void Timer::stopTask() {
    if (_currentTaskName.length() == 0)
        return;

    clock_t clocks = clock() - _startTime;
    _totalTime += clocks;
    ++_completedTaskCount;

    double duration = clocks / static_cast<double>(CLOCKS_PER_SEC);
    _lastTaskReport = getLine(duration, _currentTaskName);
    _report << _lastTaskReport << std::endl;
    _currentTaskName = "";
}

std::string Timer::getReportForLastTask() {
    stopTask();
    return _lastTaskReport;
}

std::string Timer::getReportForAllTasks() {
    stopTask();

    std::stringstream result;
    result << _report.str();
    if (_completedTaskCount > 1) {
        if (_maxLineLength > 0) {
            for (size_t i = 0; i < _maxLineLength; ++i)
                result << '-';
            result << std::endl;
        }
        std::stringstream sum;
        sum << "for all " << _completedTaskCount << " operations.";
        result << getLine(_totalTime / static_cast<double>(CLOCKS_PER_SEC),
                sum.str()) << std::endl;
    }
    return result.str();
}

std::stringstream& Timer::getReportStream() {
    stopTask();
    return _report;
}

std::string Timer::getInset() {
    std::stringstream result;
    for (size_t i = 0; i < NUM_WIDTH + 5; ++i)
        result << " ";
    return result.str();
}

std::string Timer::getLine(
        const double duration, const std::string text) {
    std::stringstream result;
    result << std::right << std::setw(NUM_WIDTH);
    switch(_unit) {
    case UNIT::secs:
        result << format(duration) << " s ";
        break;
    case UNIT::millis:
        result << format(1000.0 * duration) << " ms";
        break;
    case UNIT::nanos:
        result << format(1000000.0 * duration) << " ns";
        break;
    }
    result << "  " << text;
    std::string resultStr = result.str();
    _maxLineLength = std::max(_maxLineLength, resultStr.length());
    return resultStr;
}

std::string Timer::format(const double num) {
    return formatInt(static_cast<long>(std::round(num)));
}

} /* namespace pointcloud2 */
