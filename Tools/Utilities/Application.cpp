/*
1 Implementation of the Application Management

April 2002 Ulrich Telle

August 2002 Ulrich Telle Bug fix for uninitialized variables

*/

#include <cstdio>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <string>
#include <sys/stat.h>
using namespace std;

#include "SecondoConfig.h"
#include "Application.h"

#ifndef SECONDO_WIN32
#include <libgen.h>
#include <unistd.h>
#endif

#ifndef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX	256
#endif

Application* Application::appPointer = 0;

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
  abortMode = false;
  abortFlag = false;
  user1Flag = false;
  user2Flag = false;

#ifndef SECONDO_WIN32
  // --- Trap all signals that would terminate the program by default anyway.
//  signal( SIGHUP,    Application::AbortOnSignalHandler );
//  /*signal( SIGINT,    Application::AbortOnSignalHandler );*/
//  signal( SIGQUIT,   Application::AbortOnSignalHandler );
//  signal( SIGILL,    Application::AbortOnSignalHandler );
//  signal( SIGABRT,   Application::AbortOnSignalHandler );
//  signal( SIGFPE,    Application::AbortOnSignalHandler );
//  signal( SIGPIPE,   Application::AbortOnSignalHandler );
//  signal( SIGALRM,   Application::AbortOnSignalHandler );
//  signal( SIGTERM,   Application::AbortOnSignalHandler );
//  signal( SIGUSR1,   Application::UserSignalHandler );
//  signal( SIGUSR2,   Application::UserSignalHandler );
//  signal( SIGTRAP,   Application::AbortOnSignalHandler );
//  signal( SIGBUS,    Application::AbortOnSignalHandler );
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

  if ( Application::appPointer->abortMode )
  {
    if ( Application::appPointer->AbortOnSignal( sig ) )
    {
      signal( sig, SIG_DFL );
      kill( getpid(), sig );
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

