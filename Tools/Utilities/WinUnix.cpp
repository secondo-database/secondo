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

September 2003, M. Spiekermann: Implementation of getpagesize()

*/


#include "SecondoConfig.h"

#ifdef SECONDO_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef SECONDO_LINUX
#include <execinfo.h>
#endif

#include <iostream>
#include <string>

#include "WinUnix.h"
#include "LogMsg.h"

using namespace std;

const int
WinUnix::endian_detect = 1;

int
WinUnix::getPageSize( void ) { 

#ifndef SECONDO_WIN32
   return ( getpagesize() );
#else
   SYSTEM_INFO SysInf;
   GetSystemInfo( &SysInf );
   return ( SysInf.dwPageSize );
#endif
}


/* Obtain a backtrace and print it to stdout. */
#ifdef SECONDO_LINUX
   
void
WinUnix::stacktrace(const string& fullAppName)
{
  void* buffer[256];
  int depth = backtrace(buffer,256);
  char** STP = backtrace_symbols(buffer,depth);

  bool runaddr2line = RTFlag::isActive("DEBUG:DemangleStackTrace");

  

  string cmdStr = "addr2line -C auto -fse " + fullAppName + " "; 
  cout << " Result of  " << cmdStr << endl;
  string addressList = "";

  for(int i=0;i<depth;i++)
  {
    string str(STP[i]);
    string address = "";

    // extract address information
    unsigned int p1 = str.find_last_of("[");
    unsigned int p2 = str.find_last_of("]");

    bool addressFound = (p1 != string::npos) && (p2 != string::npos);

    if ( addressFound ) {
      address = str.substr(p1+1,p2-p1-1);
      addressList += (address + " ");
    }

    if ( !addressFound || !runaddr2line ) { 
      cout << str << endl;
    } 
  }
  if ( runaddr2line )
  {
    FILE* fp = 0;
    char line[2048];
    if ( addressList != "" ) {
      fp = popen( (cmdStr + addressList).c_str(), "r" );
      if (fp == 0) {
        cerr << "popen failed! Could not demangle stack trace!" << endl;
      } else {
        while ( true ) { 

         if ( fgets(&line[0],1024,fp) ) { // function name
          string fname(line);
          cout << " " << fname.substr(0, fname.length()-1);
         } else {
          break;
         }
       
         if ( fgets(&line[0],1024,fp) ) { // file name
          string fname(line);
          cout << " --> [ " << fname.substr(0, fname.length()-1) 
               << " ]" << endl << endl;
         } else {
          break;
         }

        }
      pclose(fp);
      }
    }
  }
  free(STP);
}
#else

void
WinUnix::stacktrace(const string& fullAppName)
{
  cerr << "Sorry - stack trace not supported." << endl;
}
#endif

