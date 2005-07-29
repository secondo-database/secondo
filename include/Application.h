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

1 Header File: Application Management

April 2002 Ulrich Telle

1.1 Overview

The module ~Application~ provides support for managing an application process.
Access to the name of the executable and the command line parameters is offered
in a portable way.

When used together with the module ~Processes~ a very simple process communication
based on a signal mechanism is available. Currently two user defined signals and
a termination signal are supported. The user signals are flagged in member
variables of the application and can be queried and reset at any time. For
handling a termination signal two different ways are offered:

First, simple flagging is possible and the flag can be queried at any time in the
event loop of the application. The flag should be checked regularly and appropriate
action should be taken when it is set.

Second, the application could provide overwrite the empty virtual method
~AbortOnSignal~. In this method any application specific clean up could take
place. If the method returns to the calling signal handler the application
is exited with a return code of *-999*.

Additionally a communication socket can be passed from a parent process to its
child processes in an operating system independent manner.

Usually the application developer derives his own class from the ~Application~
base class. Only one instance of an application is allowed. The instantiation
of the application should be the very first task in the main program,
since the signal mechanism is activated in the constructor and could miss
signals when not created immediately after start up of the application.

1.2 Interface Methods

This module offers the following routines:

[23]	Creation/Removal   & Information retrieval & Process Communication \\
	[--------]
	Application        & GetArgCount        & GetParent     \\
	[tilde]Application & GetArgValues       & HasSocket     \\
                           & GetApplicationName & GetSocket     \\
                           & GetApplicationPath & ShouldAbort   \\
                           & Instance           & GetUser1Flag  \\
                           & GetOwnProcessId    & GetUser2Flag  \\
                           &                    & ResetUser1Flag  \\
                           & Sleep              & ResetUser2Flag  \\

1.3 Imports, Constants, Types

*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "SecondoConfig.h"
#include "SocketIO.h"
#include <string>
#include <map>

#ifndef SECONDO_PID
#define SECONDO_PID
#ifndef SECONDO_WIN32
typedef int   ProcessId;
#define INVALID_PID (-1)
#else
#include <windows.h>
typedef DWORD ProcessId;
#define INVALID_PID ((DWORD)-1)
#endif
#endif

/*
1.4 Class "Application"[1]

This class provides an application framework.

*/
class SDB_EXPORT Application
{
 public:
  Application( int argc, const char **argv, const bool showLicense = true );
/*
Creates and initializes the ~Application~ object.

*NOTE*: The constructor is usually called directly from the program's main()
routine. The parameters ~argc~ and ~argv~ are usually those passed to the
main() function.

*NOTE*: Only exactly *one* ~Application~ instance is allowed per process.

*/
  virtual ~Application();
/*
Destroys the ~Application~ object.

*/
  int GetArgCount() const { return (argCount); };
/*
Returns the number of command line arguments.

*/
  const char** GetArgValues() const { return (argValues); };
/*
Returns the pointer to the array of command line arguments.

*/
  const string GetApplicationName() const
  { return (appName); };
/*
Returns the name of the executable file.

*/
  const string GetApplicationPath() const
  { return (appPath); };
/*
Returns the path name where the application was started from.

*/
  const ProcessId GetOwnProcessId() { return (ownpid); };
/*
Returns the real process identification of the process itself.

*/
  const ProcessId GetParent() { return (parent); };
/*
Returns the real process identification of the parent process, if available.
If it is not available "INVALID\_PID"[4] is returned.

*NOTE*: Unfortunately this information is not available on all platforms.
For example the operating system ~Microsoft Windows~ does not provide it
on its own, but for child processes which are created using the
~ProcessFactory~ class the parent process identification is accessible.

*/
  static Application* Instance();
/*
Returns a reference to the single ~Application~ instance.

*/
  bool ShouldAbort() const { return (abortFlag); };
/*
Checks whether the abort flag was set by a signal handler.
If this method returns "true"[4], the application should terminate as soon as
possible.

*/
  bool GetUser1Flag() { return (user1Flag); };
  bool GetUser2Flag() { return (user2Flag); };
/*
Check whether one of the user flags has been set by a remote signal.

*/
  void ResetUser1Flag() { user1Flag = false; };
  void ResetUser2Flag() { user2Flag = false; };
/*
Reset the user flags to unsignaled state, thus allowing to receive
further user signals. The meaning of these signals is not defined by
the ~Application~ class.

*/
  bool HasSocket() { return (hasSocket); };
/*
Returns "true"[4] if a socket handle was passed to the application through the
argument list.

*NOTE*: This is useful for communication server processes where a listener
process starts a child process for servicing client requests.

*/
  Socket* GetSocket() { return (clientSocket); };
/*
Returns a reference to the socket, which might be passed to the application
through the argument list.

*/
  static void Sleep( const int seconds );
/*
Causes the application to enter a wait state until a time interval of ~seconds~
seconds has expired.

*/
/*
The following methodes are only available to derived application classes:

*/
 protected:
  void SetAbortMode( bool activate ) { abortMode = activate; };
/*
Activates or deactivates the abortion mode of the application.

When the abortion mode is *activated*, the method ~AbortOnSignal~ is
invoked, when the application receives a signal on which the application
should be terminated. After return from the method ~AbortOnSignal~ the
application is terminated immediately. 

When the abortion mode is *not activated*, the method ~AbortOnSignal~ is
*not* invoked, when the application receives a signal on which the application
should be terminated. Instead an abort flag is set which should be checked
regularly in the event loop of the application.

*/
  bool GetAbortMode() { return (abortMode); };
/*
Returns the current state of the abort mode. If the abort mode is
activated "true"[4] is returned, otherwise "false"[4].

*/
  virtual bool AbortOnSignal ( int sig ) { return (true); };
/*
Is called by the application's default signal handler whenever
a signal is caught that would have aborted the process anyway. This
is the case for most signals like "SIGTERM"[4], "SIGQUIT"[4] and so on. The
pre-installed signal handler ensures proper application shutdown in
such circumstances.

*/
 private:
  int           argCount;     // number of arguments
  const char**  argValues;    // array of arguments
  string        appName;      // name of application
  string        appPath;      // path of application
  ProcessId     ownpid;       // own process id
  ProcessId     parent;       // parent process id
  bool          hasSocket;    // flag 
  Socket*       clientSocket; // reference to client socket
  int           lastSignal;   // last signal received
  bool          abortMode;    // abort mode
  volatile bool abortFlag;    // abort signal flag
  volatile bool user1Flag;    // user1 signal flag
  volatile bool user2Flag;    // user2 signal flag

#ifndef SECONDO_WIN32
  static void AbortOnSignalHandler( int sig );
/*
Is the default signal handler for handling signals which usually would terminate
the process.

*/
  static void UserSignalHandler( int sig );
/*
Is the default signal handler for handling user signals (SIGUSR1 and SIGUSR2).

*/

 static void Application::PrintStacktrace(void);
/*
Print out a stack trace in cas of abnormal program termination.

*/

#else
  Socket* rshSocket;
  DWORD WINAPI RemoteSignalHandler();
  static DWORD WINAPI RemoteSignalThread( LPVOID app )
  {
    return (((Application*) app)->RemoteSignalHandler());
  }
  static BOOL __stdcall AbortOnSignalHandler( DWORD sig );
/*
These methods emulate the signal mechanism for the ~Microsoft Windows~ platform.

*/
#endif

  static Application* appPointer;
  static map<int, std::string> signalStr;
};

#endif // APPLICATION_H

