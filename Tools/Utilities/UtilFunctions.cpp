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

Implementention of some useful helper funtions. 

November 2002 M. Spiekermann, Implementation of TimeTest.

September 2003 M. Spiekermann, Implementation of LogMsg/RTFlags.

December 2003 M. Spiekermann, Implementation of class Counter.

July 2004 M. Spiekermann, Implementation of showActiveFlags. 

*/

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include <map>
#include <sstream>
#include <vector>
#include <string>

#include "StopWatch.h"
#include "LogMsg.h"
#include "Counter.h"
#include "CharTransform.h"

using namespace std;

/*
1 Implementation of Class StopWatch

*/


StopWatch::StopWatch() :
startCPU(0),
stopCPU(0)
{
  start();
}


void
StopWatch::start() {

#ifndef SECONDO_WIN32
  gettimeofday(&startReal, 0);
#else
  time(&startReal);
#endif 
  startCPU=clock();
}


const double
StopWatch::diffSecondsReal() {

#ifndef SECONDO_WIN32
  gettimeofday(&stopReal, 0);
  double diffSec = (stopReal.tv_sec - startReal.tv_sec)*1.0;
  diffSec += (stopReal.tv_usec - startReal.tv_usec)*1.0 / 1000000;
  return diffSec;
#else
  return difftime(stopReal, startReal);
#endif
}


const double
StopWatch::diffSecondsCPU() {

  stopCPU = clock();
  return ((double) (stopCPU - startCPU)) / CLOCKS_PER_SEC;
}


const string
StopWatch::minutesAndSeconds(const double seconds) {
  
  static char sbuf[20+1];
  static double frac = 0, sec = 0, min = 0;
  
  frac = modf(seconds/60, &min);
  sec = seconds - (60 * min);
  sprintf(&sbuf[0], "%.0f:%02.0f", min, sec);
  
  return string((const char*) sbuf);
}


const string
StopWatch::timeStr(const time_t& inTime /* = 0*/) {

  static char sbuf[20+1];
  
  time_t usedTime = 0;
  if ( inTime == 0 ) {
    time(&usedTime);
  } else {
    usedTime = inTime;
  }
  
  const tm *ltime = localtime(&usedTime);
  strftime(&sbuf[0], 20, "%H:%M:%S", ltime);
  
  return string((const char*) sbuf);
}


const string
StopWatch::diffReal() {

  ostringstream buffer;
  
  buffer << "Elapsed time: " << diffSecondsReal() << " seconds."; 
  
  return buffer.str(); 
}


const string
StopWatch::diffCPU() {

   ostringstream buffer;
      
   buffer << "Used CPU time: " << diffSecondsCPU() << " seconds."; 
   
   return buffer.str();   
}


const string
StopWatch::diffTimes() {

   ostringstream buffer;
   double sReal = diffSecondsReal();
   double sCPU = diffSecondsCPU();
   
   buffer << "Times (real/cpu): ";

   if (sReal > 60.0) 
   {
     buffer << minutesAndSeconds(sReal) << "min ";
     buffer << "(" << sReal << "sec) /";  
   }
   else
   {
     buffer << sReal << "sec / ";  
   }

   buffer << sCPU << "sec = " << sReal/sCPU;
	  
   return buffer.str();  
}


/*
2 Implementation of Class RTFlag

*/

map<string,bool>::iterator RTFlag::it;

map<string,bool> RTFlag::flagMap;


void
RTFlag::showActiveFlags(ostream& os) {

  os << "Active runtime flags:" << endl;
  if ( flagMap.size() == 0 ) {
    os << "  -none- " << endl;
  }
  for ( it = flagMap.begin(); it != flagMap.end(); it++ ) {
    os << "  -" << it->first << "-" << endl;
  }

}

void
RTFlag::initByString( const string &keyList ) {

   //  The string contains a comma separated list of
   //  keywords which is inserted into a map. 

   const char* sep = ",";
   int n = keyList.length();
   
   if (n == 0) {
     return;
   }  

   char* pbuf = new char[n+1];
   keyList.copy(pbuf,n);
   pbuf[n] = 0;

   char* pkey = 0;  
   pkey=strtok(pbuf,sep);

   string key = string(pkey);
   flagMap[key] = true;
    
   while ( (pkey=strtok(0,sep)) != NULL ) {

     key = string(pkey);
     flagMap[key] = true;
   }


   // actions based on some activated flags
   if ( RTFlag::isActive("CMSG:Color") ) {
     color::useColors(true);
   }

   delete [] pbuf;
}


/*
3 Implementation of Class Counter

*/

map<string, Counter::CounterInfo>::iterator Counter::it;

map<string, Counter::CounterInfo> Counter::CounterMap;

/*
4 Implementation of ~operator<<~ for class ~tab~ see CharTransform.h

*/

ostream& operator << (ostream& os, const tab& f) {
  return f(os);
}


/*
5 Parts of class ~color~

*/

vector<string> color::col;
int color::n = 1;

ostream& operator << (ostream& os, const color& c) {
  return c(os);
}

