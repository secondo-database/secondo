/*
----
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]
//[_] [\_]
//[TOC] [\tableofcontents]

[1] Loadgenerator. This programm generates CSV data and send it 
to a network socket. A sample line looks like:

gEpYm0eUDk,fAgVgUHWPo,bClSVK17HX,ixjXTTW7yh,qdsU8WzP1O,
CcZw52F47W,bpRKKsoq0m,YoNOJWsGtt,c5U92XBHbG,kA5CUO4GE2

You can specify the number of lines to be send and parameter
like delay, number of columns or the size of a column.
*/

/* 
1.0 Includes

*/

#include <time.h>
#include <sys/time.h>

/*
2.0 Timer Class

*/
class Timer {
  
public:
  void start() {
     gettimeofday(&startTime, NULL);
  }
  
  long long getDiff() {
      struct timeval stopTime;
      gettimeofday(&stopTime, NULL);
      
      return ((stopTime.tv_sec - startTime.tv_sec) * 1000000L 
            + stopTime.tv_usec) - startTime.tv_usec;
  }
  
private: 
  struct timeval startTime;
};
