/*

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
    time_t startReal;
    time_t stopReal;
    
    clock_t startCPU; 
    clock_t stopCPU; 
        
};

#endif
