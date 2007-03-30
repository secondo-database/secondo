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
#include <io.h>
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

#include "CharTransform.h"
#include "WinUnix.h"
#include "LogMsg.h"

using namespace std;

const int
WinUnix::endian_detect = 1;


#ifdef SECONDO_WIN32
const bool
WinUnix::win32 = true;
#else
const bool
WinUnix::win32 = false; 
#endif  

int
WinUnix::getPageSize() { 

#ifndef SECONDO_WIN32
   return ( getpagesize() );
#else
   SYSTEM_INFO SysInf;
   GetSystemInfo( &SysInf );
   return ( SysInf.dwPageSize );
#endif
}


int
WinUnix::getpid() { 

#ifdef SECONDO_WIN32
  return ::GetCurrentProcessId();
#else
  return ::getpid();
#endif
}





string
WinUnix::getPlatformStr() { 

  if (win32)
    return "win32";
  else
    return "linux";

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

/*
Implementation of class ~CFile~

*/   

const char*
CFile::pathSepWin32 = "\\";

const char*
CFile::pathSepUnix = "/";

#ifdef SECONDO_WIN32
const char*
CFile::pathSep = pathSepWin32;
#else
const char*
CFile::pathSep = pathSepUnix;
#endif  

string
CFile::MakePath(const string& s)
{ 
  string t=s;
  if ( WinUnix::isWin32() )
    t = translate(t, pathSepUnix, pathSepWin32);
  else
    t = translate(t, pathSepWin32, pathSepUnix);
  return t;  
}

bool CFile::exists() 
{
  bool rc = access( fileName.c_str(), F_OK ) != -1;
  return rc;
} 

bool CFile::open() 
{
  object.open(fileName.c_str(), ios::in);
  return object;
} 
   
bool CFile::overwrite() 
{
  object.open(fileName.c_str(), ios::out|ios::trunc);
  return object;
} 

bool CFile::append() 
{
  object.open(fileName.c_str(), ios::out|ios::app);
  return object;
} 


bool CFile::close() 
{
  object.close();
  return object.good(); 
}


string CFile::getPath() const 
{
  size_t pos = fileName.find_last_of(pathSep);
  if (pos != string::npos)
    return fileName.substr(0,pos+1);
  else
    return "";
}


string CFile::getName() const
{
  string name = fileName;
  string path = getPath();
  if (path != "")
    removePrefix(path, name);
  return name;
}



