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

Small class for time measurement. Elapsed (real) time and used CPU time can
be computed according to a reference time defined by creation of the object or
by the start method. Times can be returnd as a double value representing seconds
or formatted as a string.

July 2004, M. Spiekermann. The class was created as replacement for calss TimeTest. 
An instance could be used as a clock to measure time differences.

*/

#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <string>

#include "SecondoConfig.h"

using namespace std;

class StopWatch {

     
  public:
    StopWatch();
    ~StopWatch() {};
     
    // define start time. Differences are computed with respect to this time
    void start();
          
    // return difference (now-start) in real time formatted as string 
    const string diffReal();

    // return used CPU time since start formatted as string
    const string diffCPU();

    // return both times
    const string diffTimes();

    // return the time in format HH:MM:SS
    static const string timeStr(const time_t& inTime = 0);

    // return times in seconds
    const double diffSecondsReal();
    const double diffSecondsCPU();
    
    const string minutesAndSeconds(const double seconds);
  
  private:

/*
On windows we have no timeval struct and no gettineofday function. But these could
be later defined in Winunix.h and implemented via the ~QueryPerformanceCounter~ function
declared in winbase.h. Code example from 

----
http://www.decompile.com/html/windows_timer_api.html

  LARGE_INTEGER ticksPerSecond;
  LARGE_INTEGER tick;   // A point in time
  LARGE_INTEGER time;   // For converting tick into real time

  // get the high resolution counter's accuracy
  QueryPerformanceFrequency(&ticksPerSecond);

  // what time is it?
  QueryPerformanceCounter(&tick);

  // convert the tick number into the number of seconds
  // since the system was started...
  time.QuadPart = tick.QuadPart/ticksPerSecond.QuadPart;

  //get the number of hours
  int hours = time.QuadPart/3600;

  //get the number of minutes
  time.QuadPart = time.QuadPart - (hours * 3600);
  int minutes = time.QuadPart/60;

  //get the number of seconds
  int seconds = time.QuadPart - (minutes * 60);
----

*/

#ifdef SECONDO_WIN32 
    time_t startReal;
    time_t stopReal;
#else
    timeval startReal;
    timeval stopReal;
#endif
    
    clock_t startCPU; 
    clock_t stopCPU; 
        
};

#endif
