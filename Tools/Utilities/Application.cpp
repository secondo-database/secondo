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

May 2005 M. Spiekermann. Demangling of stack trace improved.

July 2005 M. Spiekermann. Retrieval of the application name for the addr2line
command instead of hard coded application name SecondoTTYBDB

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
#include "License.h"
#include "WinUnix.h"

#ifndef SECONDO_WIN32
#include <libgen.h>
#include <unistd.h>
#endif

#ifndef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX 256
#endif



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
  char* pgmName = strdup(programName.c_str());
  appName = basename( pgmName );
  appPath = dirname( pgmName );
  free(pgmName);
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

  // --- Trap all signals that would terminate the program by default anyway.
  signalStr[SIGHUP] = "SIGINT";
  signal( SIGHUP,    Application::AbortOnSignalHandler );

  signalStr[SIGINT] = "SIGINT";
  signal( SIGINT,    Application::AbortOnSignalHandler );

  signalStr[SIGQUIT] = "SIGQUIT";
  signal( SIGQUIT,   Application::AbortOnSignalHandler );

  signalStr[SIGILL] = "SIGILL";
  signal( SIGILL,    Application::AbortOnSignalHandler );

  signalStr[SIGABRT] = "SIGABRT";
  signal( SIGABRT,   Application::AbortOnSignalHandler );

  signalStr[SIGFPE] = "SIGFPE";
  signal( SIGFPE,    Application::AbortOnSignalHandler );

  signal( SIGPIPE,   Application::AbortOnSignalHandler );
  signal( SIGALRM,   Application::AbortOnSignalHandler );

  signalStr[SIGTERM] = "SIGTERM";
  signal( SIGTERM,   Application::AbortOnSignalHandler );

  signalStr[SIGSEGV] = "SIGSEGV";
  signal( SIGSEGV,   Application::AbortOnSignalHandler );
  
  signalStr[SIGUSR1] = "SIGUSR1";
  signal( SIGUSR1,   Application::UserSignalHandler );
  
  signalStr[SIGUSR2] = "SIGUSR2";
  signal( SIGUSR2,   Application::UserSignalHandler );
  
  signalStr[SIGTRAP] = "SIGTRAP";
  signal( SIGTRAP,   Application::AbortOnSignalHandler );
  
  signalStr[SIGBUS] = "SIGBUS";
  signal( SIGBUS,    Application::AbortOnSignalHandler );
#ifdef SIGSTKFLT
  signalStr[SIGSTKFLT] = "SIGSTKFLT";
  signal( SIGSTKFLT, Application::AbortOnSignalHandler );
#endif
  signalStr[SIGIO] = "SIGKIO";
  signal( SIGIO,     Application::AbortOnSignalHandler );
#ifdef SIGPOLL
  signalStr[SIGPOLL] = "SIGPOLL";
  signal( SIGPOLL,   Application::AbortOnSignalHandler );
#endif
  signalStr[SIGXCPU] = "SIGXCPU";
  signal( SIGXCPU,   Application::AbortOnSignalHandler );
  
  signalStr[SIGXFSZ] = "SIGXFSZ";
  signal( SIGXFSZ,   Application::AbortOnSignalHandler );
  
  signalStr[SIGVTALRM] = "SIGVTALRM";
  signal( SIGVTALRM, Application::AbortOnSignalHandler );
  
  signalStr[SIGPROF] = "SIGPROF";
  signal( SIGPROF,   Application::AbortOnSignalHandler );
#ifdef SIGPWR
  signalStr[SIGPWR] = "SIGPWR";
  signal( SIGPWR,    Application::AbortOnSignalHandler );
#endif
#else
  ::SetConsoleCtrlHandler( Application::AbortOnSignalHandler, TRUE );

  DWORD dwProcess = ::GetCurrentProcessId();
  ostringstream os;
  os << "SECONDO_RSH_" << dwProcess;
  rshSocket = Socket::CreateLocal( os.str() );

  HANDLE rshHandle;
  DWORD  rshId;
  rshHandle = CreateThread( 0, 0, Application::RemoteSignalThread, 
                           (LPVOID) this, 0, &rshId                );
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

void
Application::AbortOnSignalHandler ( int sig )
{
/*
This is the default signal handler for all signals that would
abort the process if not handled otherwise.

*/
  cout << endl << "*** Signal " << signalStr[sig] 
       << " (" << sig << ") caught!";
  
  if ( sig == SIGSEGV || sig == SIGFPE )
  {
  cout << " Printing stack trace ...";
  cout << endl << " ************ BEGIN STACKTRACE *******************" << endl;
  Application* ap = Application::Instance();
  string fullAppName = ap->GetApplicationPath() 
		       + "/" + ap->GetApplicationName();
  WinUnix::stacktrace(fullAppName);
  cout << endl << " *********** END STACKTRACE **********************" 
       << endl << endl;
  }
  cout << " Calling default signal handler ..." << endl;
  signal( sig, SIG_DFL );
  raise(sig);
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

