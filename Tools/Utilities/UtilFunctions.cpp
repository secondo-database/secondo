/*
Implementention of some useful helper funtions. 

November 2002 M. Spiekermann, Implementation of TimeTest.

September 2003 M. Spiekermann, Implementation of LogMsg/RTFlags.

December 2003 M. Spiekermann, 

*/

#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include <map>
#include <sstream>
#include <string>

#include "TimeTest.h"
#include "LogMsg.h"
#include "Counter.h"

using namespace std;

/*
1 Implementation of Class TimeTest

*/

const string
TimeTest::diffReal() {

   static bool firstcall = true;
   static time_t lasttime = 0;
   time_t currenttime = 0;
   ostringstream buffer;
   
   if (time(&currenttime) == (-1)) {
        buffer << "Error when calling time()!" << endl;
	return buffer.str();
   }
   
   double diffseconds = 0, full =0, frac = 0, sec =0, min = 0;
   if (firstcall) {
     diffseconds = 0;
     firstcall = false;
     time(&lasttime);
   } else {
     diffseconds = difftime(currenttime, lasttime); 
     // save time for next function call!
     lasttime = currenttime;     
   }	   
    

  frac = modf(diffseconds/60, &full);
  min = full;
  sec = diffseconds - (60 * min);
  
  const int BUFSIZE=20;
  char sbuf1[BUFSIZE+1];
  char sbuf2[BUFSIZE+1];
  const tm *ltime = localtime(&currenttime);
  strftime(&sbuf1[0], BUFSIZE, "%H:%M:%S", ltime);
  sprintf(&sbuf2[0], "%.0f:%02.0f", min, sec);
  buffer << sbuf1 << " -> elapsed time " << sbuf2  << " minutes."; 
  
  return buffer.str(); 
}


const string
TimeTest::diffCPU() {

   static bool firstcall = true;
   static clock_t lasttime = 0;
   clock_t currenttime = 0;
   double cputime = 0;
   ostringstream buffer;
   
   if (firstcall) {
     cputime = 0;
     lasttime = clock();
     firstcall = false;
   } else {   
     currenttime = clock();
     cputime = ((double) (currenttime - lasttime)) / CLOCKS_PER_SEC;
     lasttime = currenttime;
   }
   
   buffer << "Used CPU Time: " << cputime << " seconds."; 
   
   return buffer.str();
   
}

/*
2 Implementation of Class RTFlag

*/

map<string,bool>::iterator
RTFlag::it;

map<string,bool>
RTFlag::flagMap;

void
RTFlag::initByString( const string &keyList ) {

   /*  The string contains a comma separated list of keywords which is inserted 
    *  into a map. 
    */
   const char* sep = ",";
   int n = keyList.length();
   
   if (n == 0) return;

   char* pbuf = new char[n+1];
   keyList.copy(pbuf,n);
   pbuf[n] = *sep;

   char* pkey = 0;  
   pkey=strtok(pbuf,sep);

   string key = string(pkey);
   flagMap[key] = true;
    
   while ( (pkey=strtok(0,sep)) != NULL ) {

     key = string(pkey);
     flagMap[key] = true;
   }

   delete [] pbuf;
}


/*
3 Implementation of Class Counter

*/


map<string,long>::iterator
Counter::it;

map<string,long>
Counter::CounterMap;

