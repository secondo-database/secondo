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

1 Implementation of the Application Management

April 2002 Ulrich Telle

August 2002 Ulrich Telle Bug fix for uninitialized variables

Nov 2004 M. Spiekermann. A global instance of class CMsg is defined
in this file to be used by an application to transmit Informations
to files, screen or (in case of the server) to a client. 

*/

#include <cstdio>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <string>
#include <map>
#include <sys/stat.h>
using namespace std;

#include "SecondoConfig.h"
#include "Application.h"
#include "Counter.h"
#include "LogMsg.h"

#ifndef SECONDO_WIN32
#include <libgen.h>
#include <unistd.h>
#endif

#ifndef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX	256
#endif

// global instance of a Message object. All messages should be reported
// using this object to have a flexible mechanism for message handling.
// The class is dclared in the file LogMsg.h.
CMsg cmsg;


Application* 
Application::appPointer = 0;

map<int, string>
Application::signalStr;

Application* Application::Instance()
{
  return (appPointer);
}

/*
Class constructors/destructors

*/

Application::Application( int argc, const char** argv )
{

  if ( appPointer )
  {
    cerr << "Fatal error: Only one *Application* instance allowed!" << endl;
    exit( -999 );
  }
	
  string programName = argv[0];
  appPointer = this;
  argCount   = argc;
  argValues  = argv;
  if ( strncmp( argv[argc-1], "--socket=", 9 ) == 0 )
  {
    argCount--;
    hasSocket = true;
    SocketDescriptor sd;
    istringstream is( argv[argc-1]+9 );
    is >> sd;
    clientSocket = Socket::CreateClient( sd );
  }
  else
  {
    hasSocket = false;
    clientSocket = 0;
  }
  if ( strncmp( argv[argc-1], "--ppid=", 7 ) == 0 )
  {
    argCount--;
    istringstream is( argv[argc-1]+7 );
    is >> parent;
  }
  else
  {
    parent = INVALID_PID;
  }
	
#ifndef SECONDO_WIN32
  ownpid = getpid();
  char* pgmName = strdup( programName.c_str() );
  appName = strdup( basename( pgmName ) );
  appPath = strdup( dirname( pgmName ) );
#else
  rshSocket = 0;
  ownpid = ::GetCurrentProcessId();
  char fileName[MAX_PATH];
  if ( GetModuleFileName( NULL, fileName, MAX_PATH ) != 0 )
  {
    programName = fileName;
  }
  string::size_type idx = programName.rfind( PATH_SLASHCHAR );
  if ( idx != string::npos )
  {
    appName = programName.substr( idx+1 );
    appPath = programName.substr( 0, idx );
  }
  else
  {
    appName = programName;
    appPath = "";
  }
#endif
  lastSignal = 0;
  abortMode = true;
  abortFlag = false;
  user1Flag = false;
  user2Flag = false;

#ifndef SECONDO_WIN32
  signalStr[SIGINT] = "SIGINT";
  signalStr[SIGQUIT] = "SIGQUIT";
  signalStr[SIGILL] = "SIGILL";
  signalStr[SIGABRT] = "SIGABRT";
  signalStr[SIGFPE] = "SIGFPE";
  signalStr[SIGTERM] = "SIGTERM";
  signalStr[SIGSEGV] = "SIGSEGV";

  // --- Trap all signals that would terminate the program by default anyway.
//  signal( SIGHUP,    Application::AbortOnSignalHandler );
  signal( SIGINT,    Application::AbortOnSignalHandler );
  signal( SIGQUIT,   Application::AbortOnSignalHandler );
  signal( SIGILL,    Application::AbortOnSignalHandler );
  signal( SIGABRT,   Application::AbortOnSignalHandler );
  signal( SIGFPE,    Application::AbortOnSignalHandler );
//  signal( SIGPIPE,   Application::AbortOnSignalHandler );
//  signal( SIGALRM,   Application::AbortOnSignalHandler );
  signal( SIGTERM,   Application::AbortOnSignalHandler );
  signal( SIGSEGV,   Application::AbortOnSignalHandler );
//  signal( SIGUSR1,   Application::UserSignalHandler );
//  signal( SIGUSR2,   Application::UserSignalHandler );
//  signal( SIGTRAP,   Application::AbortOnSignalHandler );
  signal( SIGBUS,    Application::AbortOnSignalHandler );
#ifdef SIGSTKFLT
//  signal( SIGSTKFLT, Application::AbortOnSignalHandler );
#endif
//  signal( SIGIO,     Application::AbortOnSignalHandler );
//  signal( SIGPOLL,   Application::AbortOnSignalHandler );
//  signal( SIGXCPU,   Application::AbortOnSignalHandler );
//  signal( SIGXFSZ,   Application::AbortOnSignalHandler );
//  signal( SIGVTALRM, Application::AbortOnSignalHandler );
//  signal( SIGPROF,   Application::AbortOnSignalHandler );
//  signal( SIGPWR,    Application::AbortOnSignalHandler );
#else
  ::SetConsoleCtrlHandler( Application::AbortOnSignalHandler, TRUE );

  DWORD dwProcess = ::GetCurrentProcessId();
  ostringstream os;
  os << "SECONDO_RSH_" << dwProcess;
  rshSocket = Socket::CreateLocal( os.str() );

  HANDLE rshHandle;
  DWORD  rshId;
  rshHandle = CreateThread( 0, 0, Application::RemoteSignalThread, (LPVOID) this, 0, &rshId );
  if ( rshHandle != 0 )
  {
    ::CloseHandle( rshHandle );
  }

#endif
}

Application::~Application()
{
#ifdef SECONDO_WIN32
  if ( rshSocket != 0 )
  {
    rshSocket->CancelAccept();
    delete rshSocket;
    rshSocket = 0;
  }
#endif
  Counter::reportValues();
}

void
Application::Sleep( const int seconds )
{
#ifdef SECONDO_WIN32
  ::Sleep( (DWORD) (seconds*1000) );
#else
  ::sleep( seconds );
#endif
}

#ifndef SECONDO_WIN32


#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

/* Obtain a backtrace and print it to stdout. */
void
Application::PrintStacktrace(void)
{
  void* buffer[256];
  int depth = backtrace(buffer,256);
  char** STP = backtrace_symbols(buffer,depth);

  const string tenStars = " ********** ";
  const string stackTrace = "Stacktrace";

  cout << tenStars << " Begin " << stackTrace << tenStars << endl;
  for(int i=0;i<depth;i++)
  {
    string str(STP[i]);

    string cmd1 = "addr2line -e SecondoTTYBDB ";
    string cmd2 = "c++filt ";
    string address = "";
    string symbolName = "";

    // extract address information
    unsigned int p1 = str.find_last_of("[");
    unsigned int p2 = str.find_last_of("]");
    if (p1==string::npos || p2==string::npos) {

      cmd1 = "";

    } else {
      
      address = str.substr(p1+1,p2-p1-1);
      cmd1 += address;
    }

    // extract demangled C++ function name
    p1 = str.find("(");
    p2 = str.find_last_of("+");

    if (p1==string::npos || p2==string::npos) 
    {
      cmd2 = "";

    } else {

      symbolName = str.substr(p1+1,p2-p1-1);
      cmd2 += symbolName;
    }

    if ( RTFlag::isActive("DEBUG:TranslateStacktrace") )
    {
      if ( cmd1 != "" )
        system(cmd1.c_str());
      if ( cmd2 != "" )
        system(cmd2.c_str());

    } else {
     
      if ( (cmd1 == "") || (cmd2 == "") ) { 
        cout << str << endl;
      } else {
        cout << symbolName << " " << address << endl;
      }
    }
  }
  cout << tenStars << " End " << stackTrace << tenStars << endl;


  free(STP);
}

void
Application::AbortOnSignalHandler ( int sig )
{
/*
This is the default signal handler for all signals that would
abort the process if not handled otherwise.

*/
  Counter::reportValues();
  cout << endl << " ***************************************";
  cout << endl << " **";
  cout << endl << " ** Signal #" << signalStr[sig] << " caught! ";
  cout << endl << " **" << endl;
  PrintStacktrace();
  if ( Application::appPointer->abortMode )
  {
    if ( Application::appPointer->AbortOnSignal( sig ) )
    {
      //signal( sig, SIG_DFL );
      //kill( getpid(), sig );
      exit(1);
    }
    else
    {
      Application::appPointer->abortFlag = true;
      Application::appPointer->lastSignal = sig;
      signal( sig, Application::AbortOnSignalHandler );
    }
  }
  else
  {
    Application::appPointer->abortFlag = true;
    Application::appPointer->lastSignal = sig;
    signal( sig, Application::AbortOnSignalHandler );
  }
}

void
Application::UserSignalHandler ( int sig )
{
  if ( sig == SIGUSR1 )
  {
    Application::appPointer->user1Flag = true;
  }
  else
  {
    Application::appPointer->user2Flag = true;
  }
  Application::appPointer->lastSignal = sig;
  signal( sig, Application::UserSignalHandler );
}

#else // Windows

DWORD WINAPI
Application::RemoteSignalHandler()
{
  bool ok = true;
  while (ok)
  {
    Socket* request = rshSocket->Accept();
    if ( request != 0 && request->IsOk() )
    {
      iostream& ss = request->GetSocketStream();
      string cmd;
      ss >> cmd;
      if ( cmd == "TERMINATE" )
      {
        if ( abortMode )
        {
          if ( AbortOnSignal( SIGTERM ) )
          {
            exit( -999 );
          }
          else
          {
            abortFlag = true;
          }
        }
        else
        {
          abortFlag = true;
        }
      }
      else if ( cmd == "USER1" )
      {
        user1Flag = true;
      }
      else if ( cmd == "USER2" )
      {
        user2Flag = true;
      }
    }
    else
    {
      ok = false;
    }
    if ( request != 0 )
    {
      delete request;
    }
  }
  return (0);
}

BOOL
Application::AbortOnSignalHandler( DWORD sig )
{
  Application::appPointer->lastSignal = sig;
  if ( sig == CTRL_C_EVENT || 
       sig == CTRL_BREAK_EVENT ||
       sig == CTRL_CLOSE_EVENT ||
       sig == CTRL_LOGOFF_EVENT ||
       sig == CTRL_SHUTDOWN_EVENT )
  {
    Application::appPointer->lastSignal = sig;
    if ( Application::appPointer->abortMode )
    {
      if ( Application::appPointer->AbortOnSignal( sig ) )
      {
        exit( -999 );
      }
      else
      {
        Application::appPointer->abortFlag = true;
      }
    }
    else
    {
      Application::appPointer->abortFlag = true;
    }
    return (TRUE);
  }
  return (FALSE);
}

#endif

/***** end of file *****/

