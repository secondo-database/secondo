/*
//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[11]	title:		[{\large \bf ] [}]
//paragraph	[12]	title:		[{\normalsize \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Process Management

April 2002 Ulrich Telle

1.1 Overview

The module ~Processes~ provides support for creating and managing child
processes in an operating system independent manner. Processes are created
using a process factory which is capable of managing a collection of a
user specified number of processes. After spawning child processes the
parent process can interact with the child process by a signal mechanism.
Currently two user signals and a termination signal are supported for
child processes which use the ~Application~ class for implementing the
signal mechanism. The parent process may wait for completion of one or
all of his child processes. After termination of a child process its exit
code can be accessed. After inspecting the exit code of child process
its entry in the process collection may be reused by the process factory.
On startup of the process factory the application may allow to reuse
entries of terminated processes which exit code was not inspected.

Additionally support for passing a communication socket to a child process
is provided. This is useful when a server process listens on a port for
client connections (through the method ~Accept~ of the socket module) and
spawns a child process for servicing each client.

The process factory takes care of any necessary clean up actions after
termination of a child process, especially when client sockets for network
communication are involved.

1.2 Interface Methods

This module consists of two classes: the ~Process~ class and the ~ProcessFactory~
class. Usually the ~Process~ class should not be used directly by an application.
The class ~ProcessFactory~ offers the following routines:

[23]	Process factory & Process handling  & Process information \\
	[--------]
        StartUp         & SpawnProcess      & IsProcessOk         \\
        ShutDown        & WaitForProcess    & IsProcessTerminated \\
        GetInstance     & WaitForAll        & GetExitCode         \\
                        & SignalProcess     & GetRealProcessId    \\
                        & SignalRealProcess &                     \\
                        & Sleep             &                     \\

1.3 Imports, Constants, Types

*/

#ifndef PROCESSES_H
#define PROCESSES_H

#include <string>
#include <vector>
#include "SecondoConfig.h"
#include "SocketIO.h"

#ifndef SECONDO_WIN32
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#endif

#define DEFAULT_MAX_PROCESSES 10
/*
Is the default size of the process collection of the process factory;

*/

#ifndef SECONDO_PID
#define SECONDO_PID
#ifndef SECONDO_WIN32
typedef int   ProcessId;
#define INVALID_PID (-1)
#else
typedef DWORD ProcessId;
#define INVALID_PID ((DWORD)-1)
#endif
#endif
/*
Is the type definition for process identifiers.

*/

  enum ProcessSignal { eSIGTERM, eSIGUSR1, eSIGUSR2 };
/*
Is an enumeration of supported signals.

The following signals are currently supported:

  * ~eSIGTERM~ -- is a request for the signaled process to terminate itself.

  * ~eSIGUSR1~ -- is a request for the signaled process to execute the first
of two possible user specified actions.

  * ~eSIGUSR1~ -- is a request for the signaled process to execute the second
of two possible user specified actions.

*/

class ProcessFactory;

/*
Forward declaration of class ~ProcessFactory~

*/
/*
1.4 Class "Process"[1]

*/

class Process
{
 public:
  Process();
/*
Constructs a process administration instance.

*/
  ~Process();
/*
Destroys a process administration instance.

*/
  bool SendSignal( const ProcessSignal signo = eSIGTERM );
/*
Sends the signal ~signo~ to the associated process.
In case of success "true"[4] is returned, otherwise "false"[4].

*/
  bool WaitForTermination();
/*
Waits for the termination of the associated process. The methode
returns "true"[4], if a termination signal was received. In case of an error
"false"[4] is returned.

*/
  void Finish();
/*
Cleans up a reserved, but terminated process administration instance.
This function is used by the process factory to reclaim entries in the
process collection of terminated processes, if the application has not
checked the exit code of a terminated process but has allowed the reuse
of such entries.

*/
  Process( const Process& other );
  Process& operator=( Process const &other );
/*
A copy constructor and an assignment operator are only defined since they
are required by the standard class ~vector~ which is used to manage a
collection of processes in the process factory. These methods should not
be used.

*/
 protected:
  bool      reserved;
  int       cycle;
  bool      terminated;
  int       exitStatus;

#ifdef SECONDO_WIN32
  bool      hasSocket;
  Socket*   clientSocket;
  SOCKET    inheritableSocket;
  HANDLE    event;
  bool      notMonitored;
  PROCESS_INFORMATION processInfo;

  void ActivateWaiter();
  DWORD WINAPI Waiter();
  static DWORD WINAPI WaiterThread( LPVOID p )
  {
    return (((Process*) p)->Waiter());
  }
#else
  pid_t         pid;
#endif
  friend class ProcessFactory;
};

/*
1.5 Class "ProcessFactory"[1]

This class provides methods to manage a collection of subprocesses.
After spawning a new process it is possible to wait for completion
of the process and to check its exit code. Simple means to communicate
with the process are availabe through a signal mechanism.

*/
class ProcessFactory
{
 public:
  static bool StartUp( const bool reuseTerminated = true,
                       const int  maxChildProcesses =
                                    DEFAULT_MAX_PROCESSES );
/*
Initializes the process factory. The flag ~reuseTerminated~ controls
whether entries in the internal child process table may be reused
after termination of the child process but before the parent process
has checked the exit code of the child process. If the parent process
is not interested in the exit codes, the flag should be set to "true"[4].

The parameter ~maxChildProcesses~ defines the size of the internal
child process table, i.e. the maximal number of concurrent child processes.

*/
  static bool ShutDown();
/*
Shuts down the process factory.

*/
  static bool SpawnProcess( const string& programpath,
                            const string& arguments,
                            int& processId,
                            const bool hidden = true,
                            Socket* clientSocket = 0 );
/*
Spawns a process. The program specified by ~programpath~ will be started as a
separate process and receives the ~arguments~ string as its command line.
The internal process identifier is returned as ~processId~.

If the flag ~hidden~ is set (which it is by default), the process is started
as a background process; if the flag is *not* set, the process is started as
a foreground process, if that is possible.

To support communication on client sockets across process boundaries a
reference to a socket ~clientSocket~ can be specified and is transfered to
the newly created process. Client sockets are usually created by the
~Accept~ method of the socket module.

*/
  static ProcessId GetRealProcessId( const int processId );
/*
Returns the operating system dependent process identifier. If the process
does not exist or is already terminated, the value "INVALID\_PID"[4] is returned.

*NOTE*: In situations where an application needs to send signals to processes
it did not spawn itself access to the real process identifier is necessary.
One should *not* use this identifier for manipulating a process directly
since this could interfer with this class and could cause unpredictable results.

*/
  static bool SignalProcess( const int processId,
                             const ProcessSignal signo =
                                     eSIGTERM );
/*
Sends the specified signal ~signo~ to the process ~processId~, if that
process is still running. In case of success "true"[4] is returned, otherwise
"false"[4].

*/
  static bool SignalRealProcess( const ProcessId processId,
                                 const ProcessSignal signo =
                                         eSIGTERM );
/*
Sends the specified signal ~signo~ to the process ~processId~, if that
process is still running. In case of success "true"[4] is returned, otherwise
"false"[4].

*/
  static bool GetExitCode( const int processId, int& status );
/*
Provides access to the exit code ~status~ of the process ~processId~.
The method returns "true"[4], if the process has already terminated.

*/
  static bool IsProcessOk( const int processId );
/*
Checks whether the process ~processId~ exists in the process collection.
The method returns "true"[4] if the process exists and is still running or
is in terminated state, otherwise "false"[4] is returned.

*/
  static bool IsProcessTerminated( const int processId );
/*
Checks whether the process ~processId~ is in terminated state.
If the process is terminated "true"[4] is returned, otherwise "false"[4].
An application should check both ~IsProcessOk~ *and* ~IsProcessTerminated~
to detect an error condition.

*/
  static bool WaitForProcess( const int processId );
/*
Waits for the termination of process ~processId~.
In case the process terminated "true"[4] is returned, in case of an error
"false"[4] is returned.

*/
  static bool WaitForAll();
/*
Waits for the termination *all* processes under control of the process factory.
The method returns "true"[4] if all processes have terminated; in case of an error
"false"[4] is returned.

*/
  static void Sleep( const int seconds );
/*
Causes the application to enter a wait state until a time interval of ~seconds~
seconds has expired.

*/
  ProcessFactory* GetInstance() { return (instance); }
/*
Returns a reference to the single instance of the process factory.

*/
 protected:
  ProcessFactory( const bool reuseTerminated = true,
                  const int maxChildProcesses =
                              DEFAULT_MAX_PROCESSES );
  virtual ~ProcessFactory();
 private:
  ProcessFactory( ProcessFactory& );

  static ProcessFactory* instance;
  vector<Process> processList;
  int  maxChilds;
  bool reuseTerminatedEntries;
#ifndef SECONDO_WIN32
  static void ChildTerminationHandler( int sig );
#endif
};

#endif

