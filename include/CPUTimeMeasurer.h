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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Module CPU Time Measurer

*/
#ifndef _CPU_TIME_MEASURER_H_
#define _CPU_TIME_MEASURER_H_

#include <set>
#include <time.h>

/*
1 Utility class ~CPUTimeMeasurer~

If ~MEASURE\_OPERATORS~ is defined, detailed data about
the runnning times of several operators is gathered
and printed to ~cerr~.

*/

//#define MEASURE_OPERATORS

class CPUTimeMeasurer
{
private:
  clock_t enterTime;
  clock_t exitTime;
  clock_t accumulated;

public:
  CPUTimeMeasurer()
  {
    accumulated = 0;
  }

  inline void Enter()
  {
#ifdef MEASURE_OPERATORS
    enterTime = clock();
#endif
  }

  inline void Exit()
  {
#ifdef MEASURE_OPERATORS
    exitTime = clock();
    accumulated += exitTime - enterTime;
#endif
  }

  inline double GetCPUTimeAndReset()
  {
    double accumulatedDouble = 0.0;

#ifdef MEASURE_OPERATORS
    accumulatedDouble = ((double)accumulated) / CLOCKS_PER_SEC;
    accumulated = 0;
#endif
    return accumulatedDouble;
  }

  inline void PrintCPUTimeAndReset(char* prefix)
  {
#ifdef MEASURE_OPERATORS
    cerr << prefix << GetCPUTimeAndReset() << endl;
#endif
  }
};

#endif // _CPU_TIME_MEASURER_H_
