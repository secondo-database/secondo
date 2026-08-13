/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics 
and Computer Science, Database Systems for New Applications.

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
#include <mutex>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <string>
#include <map>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>

#include "SecondoConfig.h"
#include "Application.h"
#include "Counter.h"
#include "LogMsg.h"
#include "License.h"
#include "WinUnix.h"

#ifndef SECONDO_WIN32
#include <libgen.h>
#include <unistd.h>
#include <execinfo.h>
#include <sys/types.h>
#include <fcntl.h>
#endif

#ifndef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX 256
#endif

#if defined(SECONDO_LINUX)
#include <link.h>
#endif

#include "Messages.h"
MessageCenter* MessageCenter::msg = 0;
#ifdef THREAD_SAFE
 std::mutex MessageCenter::mtx; 
#endif

 using namespace std;
 
Application* Application::appPointer = 0;
const char* Application::signalStr[NSIG] = {};
bool Application::dumpStacktrace = true;
char* Application::stacktraceOutput = NULL;
char* Application::relocationInfo = NULL;
char* Application::stacktraceAppName = NULL;

Application* Application::Instance()
{
  return (appPointer);
}

#ifndef SECONDO_WIN32

/*
Installs a signal handler. This is what signal() already does for us here, but
only by accident: signal() is specified loosely, and glibc gives it the BSD
meaning -- handler stays installed, interrupted calls are restarted -- only
while a feature test macro happens to be defined. Without it the same call
means System V instead, where the handler resets to the default the first time
it fires. sigaction states both properties instead of inheriting them.

The signal being handled is blocked for the duration of its own handler; that
is what an empty mask means, and it is what signal() does too.

*/
static void
InstallSignalHandler( int sig, void (*handler)( int ) )
{
  struct sigaction sa;

  memset( &sa, 0, sizeof(sa) );
  sa.sa_handler = handler;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = SA_RESTART;

  sigaction( sig, &sa, NULL );
}

#endif

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
  hasSocket    = false;
  clientSocket = 0;
  parent       = INVALID_PID;

  // Consume all arguments
  bool consumed = true;
  while ( consumed && argCount > 1 )
  {
    const char* last = argValues[argCount-1];
    if ( strncmp( last, "--socket=", 9 ) == 0 )
    {
      hasSocket = true;
      SocketDescriptor sd;
      istringstream is( last+9 );
      is >> sd;
      clientSocket = Socket::CreateClient( sd );
      argCount--;
    }
    else if ( strncmp( last, "--ppid=", 7 ) == 0 )
    {
      istringstream is( last+7 );
      is >> parent;
      argCount--;
    }
    else
    {
      consumed = false;
    }
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
  gracefulTermination = false;
  user1Flag = false;
  user2Flag = false;

 
#if defined(SECONDO_LINUX)
  // Fetch in-memory relocation offset. Needed when compiled
  // as position-independent-code (-fPIC) and tools which work 
  // with ELF addresses (e.g., addr2line) should be used.
  //
  // Calculation of the string needs to be made here. In the 
  // signal handler, no new memory allcations can be made.
  uintptr_t relocation = _r_debug.r_map->l_addr;

  Application::relocationInfo = 
      (char *) malloc(128 * sizeof(char));

  snprintf(relocationInfo, 128, 
      "\nBinary relocation: 0x%" PRIxPTR "\n\n", relocation);
#endif
 
#ifndef SECONDO_WIN32
  
  // Store stacktrace output filenanme for later 
  // use. If the application crashes, it's not ensured that the 
  // env variabes can be accessed. 

  // Keep a plain copy of the application name for the stacktrace. The signal
  // handler passes it on, and GetApplicationName hands back a std::string by
  // value, which is an allocation the handler must not make.
  Application::stacktraceAppName = strdup( appName.c_str() );

  char* localStacktrace = getenv("SECONDO_LOCAL_STACKTRACE");
  if(localStacktrace == 0){
    char *output = getenv ("SEGFAULT_OUTPUT_NAME");
    if(output != NULL && output[0] != '\0') {
     int ret = access (output, R_OK | W_OK);
 
     if(ret == 0 || (ret == -1 && errno == ENOENT)) {
       Application::stacktraceOutput = strdup(output);
     }
    }
  }else {
     stringstream st;
     st << "secondo_stacktrace." << WinUnix::getpid();
     Application::stacktraceOutput = strdup(st.str().c_str());
  }


  // --- Trap all signals that would terminate the program by default anyway.
  // When SECONDO_NO_SIGNAL_HANDLERS is set, leave the signals at their default
  // disposition so a sanitizer (AddressSanitizer) or a debugger can catch and
  // symbolize faults such as SIGSEGV directly.
  if (getenv("SECONDO_NO_SIGNAL_HANDLERS") == 0) {
  signalStr[SIGHUP] = "SIGHUP";
  InstallSignalHandler( SIGHUP,    Application::AbortOnSignalHandler );

  signalStr[SIGINT] = "SIGINT";
  InstallSignalHandler( SIGINT,    Application::AbortOnSignalHandler );

  signalStr[SIGQUIT] = "SIGQUIT";
  InstallSignalHandler( SIGQUIT,   Application::AbortOnSignalHandler );

  signalStr[SIGILL] = "SIGILL";
  InstallSignalHandler( SIGILL,    Application::AbortOnSignalHandler );

  signalStr[SIGABRT] = "SIGABRT";
  InstallSignalHandler( SIGABRT,   Application::AbortOnSignalHandler );

  signalStr[SIGFPE] = "SIGFPE";
  InstallSignalHandler( SIGFPE,    Application::AbortOnSignalHandler );

  signalStr[SIGPIPE] = "SIGPIPE";
  InstallSignalHandler( SIGPIPE,   Application::AbortOnSignalHandler );

  signalStr[SIGALRM] = "SIGALRM";
  InstallSignalHandler( SIGALRM,   Application::AbortOnSignalHandler );

  signalStr[SIGTERM] = "SIGTERM";
  InstallSignalHandler( SIGTERM,   Application::AbortOnSignalHandler );

  signalStr[SIGSEGV] = "SIGSEGV";
  InstallSignalHandler( SIGSEGV,   Application::AbortOnSignalHandler );
  
  signalStr[SIGUSR1] = "SIGUSR1";
  InstallSignalHandler( SIGUSR1,   Application::UserSignalHandler );
  
  signalStr[SIGUSR2] = "SIGUSR2";
  InstallSignalHandler( SIGUSR2,   Application::UserSignalHandler );
  
  signalStr[SIGTRAP] = "SIGTRAP";
  InstallSignalHandler( SIGTRAP,   Application::AbortOnSignalHandler );
  
  signalStr[SIGBUS] = "SIGBUS";
  InstallSignalHandler( SIGBUS,    Application::AbortOnSignalHandler );
#ifdef SIGSTKFLT
  signalStr[SIGSTKFLT] = "SIGSTKFLT";
  InstallSignalHandler( SIGSTKFLT, Application::AbortOnSignalHandler );
#endif
  signalStr[SIGIO] = "SIGIO";
  InstallSignalHandler( SIGIO,     Application::AbortOnSignalHandler );
#ifdef SIGPOLL
  signalStr[SIGPOLL] = "SIGPOLL";
  InstallSignalHandler( SIGPOLL,   Application::AbortOnSignalHandler );
#endif
  signalStr[SIGXCPU] = "SIGXCPU";
  InstallSignalHandler( SIGXCPU,   Application::AbortOnSignalHandler );
  
  signalStr[SIGXFSZ] = "SIGXFSZ";
  InstallSignalHandler( SIGXFSZ,   Application::AbortOnSignalHandler );
  
  signalStr[SIGVTALRM] = "SIGVTALRM";
  InstallSignalHandler( SIGVTALRM, Application::AbortOnSignalHandler );
  
  //signalStr[SIGPROF] = "SIGPROF";
  //InstallSignalHandler( SIGPROF,   Application::AbortOnSignalHandler );
#ifdef SIGPWR
  signalStr[SIGPWR] = "SIGPWR";
  InstallSignalHandler( SIGPWR,    Application::AbortOnSignalHandler );
#endif
  } // SECONDO_NO_SIGNAL_HANDLERS
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
    // Reset the pointer, because the signal handlers this class installs stay
    // installed for the life of the process and every one of them reaches
    // through appPointer. Leaving it set meant that a signal arriving during
    // or after teardown read a destroyed object. ThreadSanitizer caught
    // exactly that on the server shutdown path.
    Application::appPointer = 0;

    if(Application::relocationInfo != NULL) {
        free(Application::relocationInfo);
        Application::relocationInfo = NULL;
    }

    if(Application::stacktraceOutput != NULL) {
        free(Application::stacktraceOutput);
        Application::stacktraceOutput = NULL;
    }

    if(Application::stacktraceAppName != NULL) {
        free(Application::stacktraceAppName);
        Application::stacktraceAppName = NULL;
    }

#ifdef SECONDO_WIN32
  if ( rshSocket != 0 )
  {
    rshSocket->CancelAccept();
    delete rshSocket;
    rshSocket = 0;
  }
#endif
}


bool
Application::ShouldAbort() const
{
  if ( abortFlag )
  {
    return (true);
  }
#ifndef SECONDO_WIN32
  // Belt and braces for the orphaned-child case: if this process was started
  // by the ProcessFactory, so its parent pid is known, and that parent has
  // since died, getppid() no longer matches and we should shut down. On Linux
  // this backs up PR_SET_PDEATHSIG; on a platform without it, it is the only
  // mechanism. Costs one getppid() per poll tick, and nothing for a process
  // that was never given a --ppid.
  if ( parent != INVALID_PID && getppid() != (pid_t) parent )
  {
    return (true);
  }
#endif
  return (false);
}


bool
Application::ProcessPendingSignals()
{
  if ( !abortFlag )
  {
    return (false);
  }

  // The abort mode decides whether the cleanup hook runs here. Either way the
  // caller is told an abort is pending and is expected to leave its loop, so
  // AbortOnSignal is reached at most once.
  if ( abortMode )
  {
    AbortOnSignal( lastSignal );
  }

  return (true);
}


#ifndef SECONDO_WIN32

/*
Writes a number as decimal digits, for the signal handlers below. They must not
call snprintf or a stream to do it: neither may be used from a handler, and a
signal arriving while the main program is inside one of them would deadlock on
the lock it already holds. WinUnix::string2stdout covers the text.

*/
static void
number2stdout( unsigned int value )
{
  char   buf[16];
  size_t pos = sizeof(buf);

  do
  {
    buf[--pos] = (char) ('0' + (value % 10));
    value /= 10;
  }
  while ( value > 0 && pos > 0 );

  // A handler has nowhere to report a failed write to.
  ssize_t written = write( STDOUT_FILENO, buf + pos, sizeof(buf) - pos );
  (void) written;
}

/*
Tells the terminating signals -- the ones a daemon may want to shut down on --
apart from the fatal ones. A fatal signal means the process is already corrupt
and must not run cleanup, so it is never deferred.

*/
static bool
IsTerminatingSignal( int sig )
{
  return sig == SIGINT || sig == SIGTERM || sig == SIGHUP || sig == SIGQUIT;
}

void
Application::AbortOnSignalHandler ( int sig )
{
/*
This is the default signal handler for all signals that would
abort the process if not handled otherwise.

*/
  // The instance may already be gone: these handlers stay installed for the
  // whole process, while appPointer is cleared by ~Application. With nothing
  // left to record the signal for, hand it to the default handler rather than
  // dereference a destroyed object.
  if ( appPointer == 0 )
  {
    signal( sig, SIG_DFL );
    raise( sig );
    return;
  }

  // When the application opted in, a terminating signal is only recorded here;
  // the shutdown itself runs later from the main loop, via
  // ProcessPendingSignals. Everything else -- and every signal when the opt-in
  // is off -- keeps the original behaviour below.
  if ( appPointer->gracefulTermination && IsTerminatingSignal( sig ) )
  {
    appPointer->lastSignal = sig;
    appPointer->abortFlag = 1;
    return;
  }

  const char* signame = ( sig > 0 && sig < NSIG && signalStr[sig] != 0 )
                        ? signalStr[sig] : "unknown";
  WinUnix::string2stdout( "\n*** Signal " );
  WinUnix::string2stdout( signame );
  WinUnix::string2stdout( " (" );
  number2stdout( (unsigned int) sig );
  WinUnix::string2stdout( ") caught!" );

  if ( sig == SIGABRT || sig == SIGSEGV || sig == SIGFPE || sig == SIGILL)
  {
     if(Application::dumpStacktrace) {
        // Never a std::string here: see stacktraceAppName in the constructor.
        const char* appname = ( Application::stacktraceAppName != NULL )
                              ? Application::stacktraceAppName : "unknown";
        WinUnix::stacktrace(appname, stacktraceOutput,
            Application::relocationInfo);
     }
  }
  WinUnix::string2stdout( " Calling default signal handler ...\n" );
  signal( sig, SIG_DFL );
  raise(sig);
}

void
Application::UserSignalHandler ( int sig )
{
  if ( Application::appPointer == 0 )
  {
    return;
  }

  // SIGUSR1 is used to cancel running queries
  if ( sig == SIGUSR1 )
  {
    WinUnix::string2stdout( "\n\nGot Signal, cancel running query.\n" );

    Application::appPointer->user1Flag = true;
  }
  else
  {
    Application::appPointer->user2Flag = true;
  }
  Application::appPointer->lastSignal = sig;
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
  if ( Application::appPointer == 0 )
  {
    return (FALSE);
  }

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

