/*
1 Implementation of the Application Management

April 2002 Ulrich Telle

*/

#include <cstdio>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <string>
#include <sys/stat.h>

#include "SecondoConfig.h"
#include "Application.h"

#ifndef SECONDO_WIN32
#include <libgen.h>
#include <unistd.h>
#endif

using namespace std;

#ifndef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX	256
#endif

Application* Application::appPointer = 0;

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
cout << "Socket is " << sd << endl;
    clientSocket = Socket::CreateClient( sd );
  }
  if ( strncmp( argv[argc-1], "--ppid=", 7 ) == 0 )
  {
    argCount--;
    istringstream is( argv[argc-1]+7 );
    is >> parent;
cout << "Parent is " << parent;
  }
  else
  {
    parent = INVALID_PID;
  }
	
#ifndef SECONDO_WIN32
  char* pgmName = strdup( programName.c_str() );
  appName = strdup( basename( pgmName ) );
  appPath = strdup( dirname( pgmName ) );
#else
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
	
#ifndef SECONDO_WIN32
  // --- Trap all signals that would terminate the program by default anyway.
  signal( SIGHUP,    Application::AbortOnSignalHandler );
  signal( SIGINT,    Application::AbortOnSignalHandler );
  signal( SIGQUIT,   Application::AbortOnSignalHandler );
  signal( SIGILL,    Application::AbortOnSignalHandler );
  signal( SIGABRT,   Application::AbortOnSignalHandler );
  signal( SIGFPE,    Application::AbortOnSignalHandler );
  signal( SIGKILL,   Application::AbortOnSignalHandler );
  signal( SIGSEGV,   Application::AbortOnSignalHandler );
  signal( SIGPIPE,   Application::AbortOnSignalHandler );
  signal( SIGALRM,   Application::AbortOnSignalHandler );
  signal( SIGTERM,   Application::AbortOnSignalHandler );
  signal( SIGUSR1,   Application::UserSignalHandler );
  signal( SIGUSR2,   Application::UserSignalHandler );
  signal( SIGTRAP,   Application::AbortOnSignalHandler );
  signal( SIGBUS,    Application::AbortOnSignalHandler );
#ifdef SIGSTKFLT
  signal( SIGSTKFLT, Application::AbortOnSignalHandler );
#endif
  signal( SIGIO,     Application::AbortOnSignalHandler );
  signal( SIGPOLL,   Application::AbortOnSignalHandler );
  signal( SIGXCPU,   Application::AbortOnSignalHandler );
  signal( SIGXFSZ,   Application::AbortOnSignalHandler );
  signal( SIGVTALRM, Application::AbortOnSignalHandler );
  signal( SIGPROF,   Application::AbortOnSignalHandler );
  signal( SIGPWR,    Application::AbortOnSignalHandler );
#else
  ::SetConsoleCtrlHandler( Application::AbortOnSignalHandler, TRUE );

  DWORD dwProcess = ::GetCurrentProcessId();
  ostringstream os;
  os << "SECONDO_RSH_" << dwProcess;
cout << "RemoteSignalHandler: " << os.str() << endl;
  rshSocket = Socket::CreateLocal( os.str() );

  HANDLE rshHandle;
  DWORD  rshId;
  rshHandle = CreateThread( 0, 0, Application::RemoteSignalThread, (LPVOID) this, 0, &rshId );
cout << "rshId=" << rshId << endl;
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
cout << " CancelAccept start" << endl;
    rshSocket->CancelAccept();
cout << " CancelAccept ready" << endl;
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
cout << "RemoteSignalHandler cmd=<" << cmd << ">" << endl;
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
cout << "RemoteSignalHandler exit" << endl;
  return (0);
}

BOOL
Application::AbortOnSignalHandler( DWORD sig )
{
  Application::appPointer->lastSignal = sig;
//  cout << "sigtype=" << sigtype << endl;
  if ( sig == CTRL_C_EVENT || 
       sig == CTRL_BREAK_EVENT ||
       sig == CTRL_CLOSE_EVENT ||
       sig == CTRL_LOGOFF_EVENT ||
       sig == CTRL_SHUTDOWN_EVENT )
  {
    if ( Application::appPointer->abortMode )
    {
      Application::appPointer->AbortOnSignal( sig );
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

