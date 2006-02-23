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

1 Implementation of the Process Management

April 2002 Ulrich Telle

April 2003 Ulrich Telle Fixed a bug in the transfer of Win32 socket handle on Windows 98 and ME systems

*/

using namespace std;

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "Processes.h"

#ifndef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX 256
#endif

#ifndef WAIT_ANY
#define WAIT_ANY (-1)
#endif

ProcessFactory* ProcessFactory::instance = 0;

ProcessFactory::ProcessFactory( 
  const bool reuseTerminated /* = true */,
  const int maxChildProcesses /* = DEFAULT_MAX_PROCESSES */ )
  : processList( maxChildProcesses ), maxChilds( maxChildProcesses ),
    reuseTerminatedEntries( reuseTerminated )
{
  processDirectory = "";
#ifndef SECONDO_WIN32
  // --- Always trap the SIGCHLD signal to avoid zombie processes
  signal( SIGCHLD, ProcessFactory::ChildTerminationHandler );
#endif
}

ProcessFactory::~ProcessFactory()
{
  // TODO: Check all child processes
}

bool
ProcessFactory::StartUp( 
  const bool reuseTerminated /* = true */,
  const int maxChildProcesses /*=DEFAULT_MAX_PROCESSES*/ )
{
  int maxChild = (maxChildProcesses > 0) ? maxChildProcesses : 1;
  if ( ProcessFactory::instance == 0 )
  {
    ProcessFactory::instance = new ProcessFactory( reuseTerminated, maxChild );
  }
  return (ProcessFactory::instance != 0);
}

bool
ProcessFactory::ShutDown()
{
  if ( instance != 0 )
  {
    delete instance;
    instance = 0;
  }
  return (instance == 0);
}

void
ProcessFactory::SetDirectory( const string& directory )
{
  instance->processDirectory = directory;
}

bool
ProcessFactory::SpawnProcess( const string& programpath,
                              const string& arguments,
                              int& processId,
                              const bool hidden /* = true */,
                              Socket* clientSocket /* = 0 */ )
{
  int    idx;
  string pathbuf = programpath;
  string localArgs = arguments;

  processId = 0;

  // --- Find entry in process list

  for ( idx = 0; idx < instance->maxChilds; idx++)
  {
    if ( !instance->processList[idx].reserved )
    {
      instance->processList[idx].reserved = true;
      break;
    }
  }
  if ( idx >= instance->maxChilds && instance->reuseTerminatedEntries )
  {
    for ( idx = 0; idx < instance->maxChilds; idx++)
    {
      if ( instance->processList[idx].terminated )
      {
        instance->processList[idx].reserved = true;
        break;
      }
    }
    // no space left in process table
    if ( idx >= instance->maxChilds )
    {
      return (false);
    }
  }

#ifdef SECONDO_WIN32
  char   pathext[_MAX_EXT];
  STARTUPINFO startInfo =
    { sizeof(STARTUPINFO), NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0,
      STARTF_USESHOWWINDOW,
      SW_HIDE, 0, NULL, 0, 0, 0 };
  PROCESS_INFORMATION processInfo;
  DWORD               createFlags;
  BOOL                success;

  _splitpath( pathbuf.c_str(), NULL, NULL, NULL, pathext );
  if ( *pathext == '\0' )
  {
    // no filename extension supplied; .exe is assumed
    pathbuf += ".exe";
  }
  localArgs = "\"" + pathbuf + "\" " + arguments;

  createFlags = CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS;
  if ( !hidden )
  {
    startInfo.wShowWindow = SW_SHOW;
  }

  // --- Spawn child process

  ostringstream os;
  os << " --ppid=" << GetCurrentProcessId();
  localArgs += os.str();

  SocketDescriptor sdDup = 0;
  if ( clientSocket != 0 )
  {
    SocketDescriptor sd = clientSocket->GetDescriptor();
    DuplicateHandle( GetCurrentProcess(), (HANDLE) sd,
                     GetCurrentProcess(), (HANDLE*) &sdDup,
                     0, TRUE, DUPLICATE_SAME_ACCESS );
    os.str( "" );
    os << " --socket=" << sdDup;
    localArgs += os.str();
  }
  char* argsbuf = new char[localArgs.length()+1];
  localArgs.copy( argsbuf, localArgs.length() );
  argsbuf[localArgs.length()] = 0;

  LPCTSTR processDir = NULL;
  if (instance->processDirectory.length() > 0 )
  {
    processDir = instance->processDirectory.c_str();
  }
  cout << "Starting Process:" << endl
       << "Program: " << pathbuf << endl
       << "Args: " << argsbuf << endl;

  success = CreateProcess( pathbuf.c_str(),
                           argsbuf,
                           NULL,        // default process security attr
                           NULL,        // default thread security attr
                           TRUE,        // inherit handles
                           createFlags, // creation control flags
                           0,           // environment variable block
                           processDir,  // current directory is that of parent
                           &startInfo,  // startup info block
                           &processInfo);
  delete argsbuf;
  if ( !success )
  {
    // failed to spawn process
    ::CloseHandle( processInfo.hThread );
    ::CloseHandle( processInfo.hProcess );
    instance->processList[idx].reserved = false;
    return (false);
  }
  ::CloseHandle( processInfo.hThread );

  // ---- Initialize process table entry

  instance->processList[idx].processInfo = processInfo;
  instance->processList[idx].terminated = false;
  instance->processList[idx].cycle++;
  if ( instance->processList[idx].cycle >= instance->maxChilds )
  {
    instance->processList[idx].cycle = 1;
  }
  if ( clientSocket != 0 )
  {
    instance->processList[idx].hasSocket = true;
    instance->processList[idx].clientSocket = clientSocket;
    instance->processList[idx].inheritableSocket = sdDup;
  }
  else
  {
    instance->processList[idx].hasSocket = false;
    instance->processList[idx].clientSocket = 0;
  }

  // --- Start child process watcher thread

  instance->processList[idx].event = ::CreateEvent( NULL, TRUE, FALSE, NULL );
  instance->processList[idx].ActivateWaiter();
  Sleep( 0 );

#else // Unix and Solaris

  if ( clientSocket != 0 )
  {
    ostringstream os;
    os << " --socket=" << clientSocket->GetDescriptor();
    localArgs += os.str();
  }
  char* argsbuf = new char[localArgs.length()+1];
  localArgs.copy( argsbuf, localArgs.length() );
  argsbuf[localArgs.length()] = 0;


  cout << "Starting Process:" << endl
       << "Program: " << pathbuf << endl
       << "Args: " << argsbuf << endl;
  
  // --- Compute "argc"

  int argc = 1;
  if ( argsbuf && *argsbuf )
  {
    char *pos = argsbuf;
    while (*pos)
    {
      if ( *pos == '\"' )
      {
        pos++;     // skip the '\"' at the beginning
        argc++;
        while (*pos && *pos != '\"')
        {
          pos++;
        }
        if ( *pos )
        {
          pos++;   // skip the '\"' at the end
        }
      }
      else if ( *pos != ' ' && *pos != '\t' )
      {
        argc++;
        while (*pos && *pos != ' ' && *pos != '\t' && *pos != '\"')
        {
          pos++;
        }
      }
      else
      {
        pos++;
      }
    }
  }

  // --- Create "argv"

  char** argv = new char*[argc+1];
  char* spath = strdup( pathbuf.c_str() );
  argv[0] = spath;
  if ( argc > 1 )
  {
    int arg = 1;
    char* pos = argsbuf;
    while (*pos)
    {
      if ( *pos == '\"' )
      {
        *pos++ = 0;    // make sure the end of the previous argument is set
        argv[arg++] = pos;
        while (*pos && *pos != '\"')
        {
          pos++;
        }
        if (*pos)
        {
          *pos++ = 0;
        }
      }
      else if ( *pos != ' ' && *pos != '\t' )
      {
        argv[arg++] = pos;
        while (*pos && *pos != ' ' && *pos != '\t' && *pos != '\"')
        {
          pos++;
        }
        if ( *pos && *pos != '\"' )    // do not delete the next leading '\"'
        {
          *pos++ = 0;
        }
      }
      else
      {
        pos++;
      }
    }
  }
  argv[argc] = 0;

  // --- Spawn child process

  int ctty = -1;
  bool foreground = !hidden;
  if ( foreground )
  {
/*
If spawning a *new* foreground process, require that at least one of
stdin, stdout or stderr refer to the control tty, and that the current
process is in the foreground.
Only check for controlling tty if starting a new foreground
process in an existing job.
A session without a control tty can only have background jobs.

*/
    pid_t curpgrp;
    if ( (curpgrp = tcgetpgrp( ctty = 2 )) < 0 &&
         (curpgrp = tcgetpgrp( ctty = 0 )) < 0 &&
         (curpgrp = tcgetpgrp( ctty = 1 )) < 0 )
    {
      // No control tty found (ENOTTY)
      foreground = false;
    }
    else if ( curpgrp != getpgrp() )
    {
      // current process not in foreground (EPERM)
      foreground = false;
    }
  }

  pid_t pid, pgrp;
  if ( (pid = fork()) == 0 )
  {
    // --- child process

    int fdlimit = sysconf( _SC_OPEN_MAX );
    if ( fdlimit == -1 )
    {
      fdlimit = _POSIX_OPEN_MAX;
    }
    int sd = (clientSocket != 0) ? clientSocket->GetDescriptor() : 0;
    for ( int fd = 3; fd < fdlimit; fd++ )
    {
      if ( fd != sd )
      {
        close( fd );
      }
    }
    pgrp = getpid();
    if ( foreground && setpgid( 0, pgrp ) == 0 )
    {
      sigset_t sigs;
      sigset_t oldsigs;
      int rc;
      sigemptyset( &sigs );
      sigaddset( &sigs, SIGTTOU );
      sigprocmask( SIG_BLOCK, &sigs, &oldsigs );
      rc = tcsetpgrp( ctty, pgrp );
      sigprocmask( SIG_SETMASK, &oldsigs, NULL );
    }
    if (instance->processDirectory.length() > 0 )
    {
      chdir( instance->processDirectory.c_str() );
    }
    execv( spath, argv );
    exit( -3 );  // only reached if exec() failed
  }

  // -- parent process
  delete []argv;
  delete argsbuf;
  
  if ( pid < 0 )
  {
    instance->processList[idx].reserved = false;
    return (false);
  }

  instance->processList[idx].pid = pid;
  instance->processList[idx].terminated = false;
  instance->processList[idx].cycle++;
  if ( clientSocket != 0 )
  {
    clientSocket->Close();
  }

  // --- establish child process group here too.
  if ( foreground )
  {
    pgrp = pid;
    setpgid( pid, pgrp );
  }
  Sleep( 0 );
#endif
  instance->processDirectory = "";
  processId = idx * (instance->maxChilds+1) + instance->processList[idx].cycle;
  return (true);
}

ProcessId
ProcessFactory::GetRealProcessId( const int processId )
{
  ProcessId pid = INVALID_PID;
  int index = processId / (instance->maxChilds+1);
  int cycle = processId % (instance->maxChilds+1);
  if ( instance->processList[index].cycle == cycle &&
       instance->processList[index].reserved &&
      !instance->processList[index].terminated )
  {
#ifdef SECONDO_WIN32
    pid = instance->processList[index].processInfo.dwProcessId;
#else
    pid = instance->processList[index].pid;
#endif
  }
  return (pid);
}

bool
ProcessFactory::SignalProcess( const int processId,
                               const ProcessSignal sig /* = eSIGTERM */ )
{
  bool ok = false;
  int index = processId / (instance->maxChilds+1);
  int cycle = processId % (instance->maxChilds+1);
  if ( instance->processList[index].cycle == cycle &&
       instance->processList[index].reserved &&
      !instance->processList[index].terminated )
  {
    instance->processList[index].SendSignal( sig );
    ok = true;
  }
  return (ok);
}

bool
ProcessFactory::SignalRealProcess( const ProcessId processId,
                                   const ProcessSignal signo /* = eSIGTERM */ )
{
  bool ok = false;
  int idx;
  bool found = false;
  for ( idx = 0; !found && idx < instance->maxChilds; idx++ )
  {
#ifdef SECONDO_WIN32
    if ( instance->processList[idx].reserved &&
         instance->processList[idx].processInfo.dwProcessId == processId )
#else
    if ( instance->processList[idx].reserved &&
         instance->processList[idx].pid == processId )
#endif
    {
      found = true;
    }
  }
  if ( !found )
  {
    Process proc;
    proc.reserved   = true;
    proc.terminated = false;
#ifdef SECONDO_WIN32
    proc.processInfo.dwProcessId = processId;
#else
    proc.pid = processId;
#endif
    ok = proc.SendSignal( signo );
  }
  return (ok);
}

bool
ProcessFactory::GetExitCode( const int processId, int& status )
{
  bool ok = false;
  int index = processId / (instance->maxChilds+1);
  int cycle = processId % (instance->maxChilds+1);
  if ( instance->processList[index].cycle == cycle &&
       instance->processList[index].reserved &&
       instance->processList[index].terminated )
  {
    status = instance->processList[index].exitStatus;
    ok = true;
  }
  return (ok);
}

bool
ProcessFactory::IsProcessOk( const int processId )
{
  bool ok = false;
  int index = processId / (instance->maxChilds+1);
  int cycle = processId % (instance->maxChilds+1);
  if ( instance->processList[index].cycle == cycle &&
       instance->processList[index].reserved )
  {
    ok = true;
  }
  return (ok);
}

bool
ProcessFactory::IsProcessTerminated( const int processId )
{
  bool ok = false;
  int index = processId / (instance->maxChilds+1);
  int cycle = processId % (instance->maxChilds+1);
  if ( instance->processList[index].cycle == cycle &&
       instance->processList[index].reserved )
  {
    ok = instance->processList[index].terminated;
  }
  return (ok);
}

bool
ProcessFactory::WaitForProcess( const int processId )
{
  bool ok = false;
  int index = processId / (instance->maxChilds+1);
  int cycle = processId % (instance->maxChilds+1);

  if ( instance->processList[index].cycle == cycle &&
       instance->processList[index].reserved )
  {
    if ( !instance->processList[index].terminated )
    {
      ok = instance->processList[index].WaitForTermination();
    }
    else
    {
      ok = true;
    }
  }
  return (ok);
}

bool
ProcessFactory::WaitForAll()
{
  bool ok = true;
#ifdef SECONDO_WIN32
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  int hCount;
  DWORD dwEvent; 
  do
  {
    hCount = 0;
    for ( int idx = 0; idx < instance->maxChilds; idx++)
    {
      if ( instance->processList[idx].reserved &&
          !instance->processList[idx].terminated )
      {
        handles[hCount] = instance->processList[idx].processInfo.hProcess;
        hCount++;
        if ( hCount >= MAXIMUM_WAIT_OBJECTS )
        {
          break;
        }
      }
    }
    if ( hCount > 0 )
    {
      dwEvent = ::WaitForMultipleObjects( hCount, handles, TRUE, INFINITE );
      if ( dwEvent == WAIT_FAILED )
      {
        ok = false;
      }
    }
  }
  while (hCount > 0);
#else
  int serrno = errno;
  int exitcode;
  pid_t childId;
  while ((childId = waitpid( WAIT_ANY, &exitcode, WNOHANG )) > 0)
  {
    for ( int idx = 0; idx < instance->maxChilds; idx++)
    {
      if ( childId == instance->processList[idx].pid )
      {
        instance->processList[idx].terminated = true;
        instance->processList[idx].exitStatus = (uint32_t) exitcode;
        break;
      }
    }
  }
  errno = serrno;

  int pCount = 0;
  for ( int idx = 0; idx < instance->maxChilds; idx++)
  {
    if ( instance->processList[idx].reserved &&
        !instance->processList[idx].terminated )
    {
      pCount++;
    }
  }
  ok = (pCount == 0);
#endif
  return (ok);
}

void
ProcessFactory::Sleep( const int seconds )
{
#ifdef SECONDO_WIN32
  ::Sleep( (DWORD) (seconds*1000) );
#else
  ::sleep( seconds );
#endif
}

#ifndef SECONDO_WIN32

void
ProcessFactory::ChildTerminationHandler( int sig )
{
  int serrno = errno;
  int exitcode;
  pid_t childId;
  while ((childId = waitpid( WAIT_ANY, &exitcode, WNOHANG )) > 0)
  {
    for ( int idx = 0; idx < instance->maxChilds; idx++)
    {
      if ( childId == instance->processList[idx].pid )
      {
        instance->processList[idx].terminated = true;
        instance->processList[idx].exitStatus = (uint32_t) exitcode;
        break;
      }
    }
  }
  errno = serrno;
  signal( sig, ChildTerminationHandler );
}

#endif

Process::Process()
  : reserved( false ), cycle( 0 ), terminated( false ), exitStatus( 0 )
{
#ifdef SECONDO_WIN32
  hasSocket    = false;
  clientSocket = 0, 
  notMonitored = false;
//  PROCESS_INFORMATION processInfo;
  event = 0;
  inheritableSocket = 0;
#else
  pid = -1;
#endif
}

Process::~Process()
{
#ifdef SECONDO_WIN32
  if ( event )
  {
    ::CloseHandle( event );
    event = 0;
  }
#endif
}

Process::Process( const Process& other )
{
  Process::operator=( other );
}

Process& 
Process::operator=( Process const &other )
{
  reserved     = other.reserved;
  cycle        = other.cycle;
  terminated   = other.terminated;
  exitStatus   = other.exitStatus;
#ifdef SECONDO_WIN32
  hasSocket    = other.hasSocket;
  clientSocket = other.clientSocket;
  processInfo  = other.processInfo;
  inheritableSocket = other.inheritableSocket; // ??? TODO
  notMonitored = other.notMonitored;
  if ( other.event )
  {
    event = ::CreateEvent( NULL, TRUE, FALSE, NULL );
  }
  else
  {
    event = 0;
  }
#else
  pid          = other.pid;
#endif
  return (*this);
}

bool
Process::WaitForTermination()
{
  bool ok = false;
  if ( reserved && !terminated )
  {
#ifdef SECONDO_WIN32
    ok = ( ::WaitForSingleObject( processInfo.hProcess, INFINITE ) 
           == WAIT_OBJECT_0 );
//
//  - exit status besorgen
//  - process handle schliessen
//  - socket schliessen, falls vorhanden
//
#else
    ok = (waitpid( pid, 0, 0 ) >= 0);
    if ( !terminated )
    {
      terminated = true;
    }
#endif
  }
  else
  {
    ok = (reserved && terminated);
  }
  return (ok);
}

bool
Process::SendSignal( const ProcessSignal signo /* = eSIGTERM */ )
{
  bool ok = false;
  if ( reserved && !terminated )
  {
#ifdef SECONDO_WIN32
    ostringstream os;
    os << "SECONDO_RSH_" << processInfo.dwProcessId;
    Socket* rshClient = Socket::Connect( os.str(), "", 
                                         Socket::SockLocalDomain );
    if ( rshClient->IsOk() )
    {
      iostream& ss = rshClient->GetSocketStream();
      switch (signo)
      {
        case eSIGUSR1:
        {
          ss << "USER1" << endl;
          break;
        }
        case eSIGUSR2:
        {
          ss << "USER2" << endl;
          break;
        }
        case eSIGTERM:
        default:
        {
          ss << "TERMINATE" << endl;
          break;
        }
      }
      ok = true;
    }
    else
    {
    }
    delete rshClient;
#else
    switch (signo)
    {
      case eSIGUSR1:
      {
        ok = (kill( pid, SIGUSR1 ) == 0);
        break;
      }
      case eSIGUSR2:
      {
        ok = (kill( pid, SIGUSR2 ) == 0);
        break;
      }
      case eSIGTERM:
      default:
      {
        ok = (kill( pid, SIGTERM ) == 0);
        break;
      }
    }
#endif
  }
  return (ok);
}

void
Process::Finish()
{
  if ( reserved && terminated )
  {
    reserved   = false;
    terminated = false;
#ifdef SECONDO_WIN32
    if ( event )
    {
      ::CloseHandle( event );
      event = 0;
    }
#endif
  }
}

#ifdef SECONDO_WIN32

void
Process::ActivateWaiter()
{
  HANDLE waitHandle;
  DWORD  waitId;
  waitHandle = CreateThread( 0, 0, WaiterThread, (LPVOID) this, 0, &waitId );
  if ( waitHandle != 0 )
  {
    ::CloseHandle( waitHandle );
  }
  notMonitored = (waitHandle == 0);
  return;
}

DWORD WINAPI
Process::Waiter()
{
/*
For each newly created process a thread ~WaiterThread~ is spawned
to monitor the status of the child process.

The parameter ~param~ must be the corresponding index into the child
process table.

*/
  HANDLE childProcessHandle;
  BOOL   ok;
  DWORD  rc = (DWORD) -1;

  // retrieve handle for child process from process table and duplicate

  ok = DuplicateHandle(
         GetCurrentProcess(),     // source process
         processInfo.hProcess,    // source handle to dup
         GetCurrentProcess(),     // target process
         &childProcessHandle,     // target handle (duplicate)
         0,                       // access (ignored)
         FALSE,                   // not inheritable
         DUPLICATE_SAME_ACCESS );

  if ( ok )
  {
    // --- wait for child process to terminate

    if ( WaitForSingleObject( childProcessHandle, INFINITE ) == WAIT_OBJECT_0 )
    {
      // --- child process terminated; mark in table and signal event
      terminated = true;
      GetExitCodeProcess( childProcessHandle, (DWORD*) &exitStatus );
      rc = 0;
      if ( hasSocket )
      {
        ::closesocket( inheritableSocket );
        clientSocket->Close();
        delete clientSocket;
        clientSocket = 0;
        hasSocket = false;
      }
      if ( event )
      {
        ::SetEvent( event );
      }
    }
    CloseHandle( childProcessHandle );
  }

/*
Nothing can be done if ~DuplicateHandle~ or ~WaitForSingleObject~ fail,
however, this should never happen.

*/
  return (rc);
}

#endif

