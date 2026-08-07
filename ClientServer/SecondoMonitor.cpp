/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of 
Mathematics & Computer Science, Database Systems for New Applications.

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

Oct 2009, M. Spiekermann. Input, command processsing and termination revised

*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Application.h"
#include "Processes.h"
#include "SecondoSMI.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "CharTransform.h"
#include "WinUnix.h"
#include "TTYParameter.h"

using namespace std;

class SecondoMonitor : public Application
{
 public:
  SecondoMonitor( const int argc, const char** argv );
  virtual ~SecondoMonitor() {};
  int  Execute(bool autostartup);
  // Hands the monitor the write end of the readiness pipe used by --daemon.
  // Execute() reports the outcome of its startup through it exactly once, so
  // that the process which forked us can exit with a meaningful status
  // instead of polling and guessing. Without a pipe (-1) this is a no-op and
  // the foreground path is unchanged.
  void SetReadyFd( int fd ) { readyFd = fd; }

 private:
  bool AbortOnSignal( int sig ) const;
  void Usage();
  void SignalReady( bool ok );
  void ExecStartUp();
  void ExecShutDown();
  void ExecShow();
  void ExecQuit();
  bool CheckConfiguration();
  bool Initialize();
  void ProcessCommands();
  void Terminate();
  

  SmiEnvironment::SmiType smiType;
  string cfgFile;
  string prompt;
  string line;
  string port;
  string dbDir;
  int  pidRegistrar;
  int  pidCheckpoint;
  int  pidListener;
  bool running;
  bool quit;
  // Set when the checkpoint service had to be killed instead of shutting down
  // on request. See Terminate() for what that costs us.
  bool checkpointKilled;
  // Write end of the --daemon readiness pipe, or -1 in the foreground case.
  int  readyFd;

  typedef enum {xUsage, xStartUp, xShutDown, xShow, xQuit} cmdTok;
};


// string defining the version of the SecondoMonitor
static string VersionInfo ="1.2";

// Where a local domain socket name is turned into a path. Mirrors
// unixSocketDir in UnixSocket.cpp, which is where the registrar socket is
// actually created; the pid file of --daemon is placed next to it so that both
// are keyed on the port in the same way.
static const string monitorRunDir = "/tmp/";

// How long --shutdown waits for a monitor to go away on its own before it
// resorts to SIGKILL. Same grace period Terminate() gives the daemons it owns.
static const int shutdownGraceSeconds = 30;

// How long --daemon waits for the monitor it forked to report that it is
// serving. Startup runs a BerkeleyDB recovery, which is the slow part.
static const int daemonStartupSeconds = 120;


/*
Resolves the configuration file the way CheckConfiguration does: an explicit
-c wins, then SECONDO\_CONFIG, then SecondoConfig.ini in the current directory.
The lifecycle commands below run before the monitor object exists, so they
cannot ask it.

*/
static string ResolveConfigFile( const string& cfgArg )
{
  if ( !cfgArg.empty() )
  {
    return cfgArg;
  }
  const char* config = getenv( "SECONDO_CONFIG" );
  if ( config != 0 && *config != '\0' )
  {
    return string( config );
  }
  string cwd = FileSystem::GetCurrentFolder();
  FileSystem::AppendSlash( cwd );
  return cwd + "SecondoConfig.ini";
}

/*
Resolves the port an explicit -p, otherwise the configuration file. Note that
GetParameter also honours SECONDO\_PARAM\_SecondoPort, so a port set in the
environment is picked up here just as the monitor itself would pick it up.

*/
static string ResolvePort( const string& cfgFile, const string& portArg )
{
  if ( !portArg.empty() )
  {
    return portArg;
  }
  return SmiProfile::GetParameter( "Environment", "SecondoPort", "", cfgFile );
}

static string ResolveHost( const string& cfgFile )
{
  string host = SmiProfile::GetParameter( "Environment", "SecondoHost", "",
                                          cfgFile );
  return host.empty() ? string( "127.0.0.1" ) : host;
}

static string ResolvePidFile( const string& pidArg, const string& cfgFile,
                              const string& port )
{
  if ( !pidArg.empty() )
  {
    return pidArg;
  }
  return monitorRunDir + SmiProfile::GetUniqueSocketName( cfgFile, port )
         + ".pid";
}

/*
Reports whether something accepts connections at host:port. One attempt, no
retry: every caller here drives its own loop and wants an immediate answer.

*/
static bool PortAccepts( const string& host, const string& port )
{
  Socket* probe = Socket::Connect( host, port, Socket::SockGlobalDomain, 1, 0 );
  bool accepted = ( probe != 0 && probe->IsOk() );
  if ( probe != 0 )
  {
    if ( accepted )
    {
      probe->Close();
    }
    delete probe;
  }
  return accepted;
}

/*
Sends one command to the Secondo Registrar over its local domain socket and
collects the reply. The registrar answers with a header line carrying a return
code and a count, then one line per entry, terminated by a line starting with
'0' or '-'. Returns false if the registrar could not be reached at all, which
is what separates "no monitor here" from "monitor with nothing to report".

*/
static bool AskRegistrar( const string& cfgFile, const string& port,
                          const string& cmd, string& headerLine,
                          vector<string>& entries )
{
  string regName = SmiProfile::GetUniqueSocketName( cfgFile, port );
  Socket* msgClient = Socket::Connect( regName, "", Socket::SockLocalDomain,
                                       1, 0 );
  headerLine = "";
  if ( msgClient != 0 && msgClient->IsOk() )
  {
    iostream& ss = msgClient->GetSocketStream();
    ss << cmd << endl;
    string answer( "" );
    bool first = true;
    do
    {
      getline( ss, answer );
      if ( first )
      {
        headerLine = answer;
        first = false;
      }
      else if ( answer[0] != '0' && answer[0] != '-' && !ss.fail() )
      {
        entries.push_back( answer );
      }
    }
    while ( answer[0] != '0' && answer[0] != '-' && !ss.fail() );
  }
  if ( msgClient != 0 )
  {
    delete msgClient;
  }
  return !headerLine.empty();
}


SecondoMonitor::SecondoMonitor( const int argc, const char** argv )
  : Application( argc, argv )
{
  char **argvalues = (char**)argv;
  TTYParameter ttyp(argc, argvalues);
  smiType       = SmiEnvironment::GetImplementationType();
  cfgFile       = ttyp.parmFile;
  prompt        = "SEC_MON> ";
  line          = "";
  pidRegistrar  = 0;
  pidListener   = 0;
  pidCheckpoint = 0;
  running       = false;
  quit          = false;
  checkpointKilled = false;
  readyFd       = -1;

  // Terminating signals are left to the base Application handler, which just
  // sets the abort flag that ProcessCommands checks. The shutdown work must not
  // run from a signal handler: it tears down BerkeleyDB, whose region locks are
  // not robust, so doing it on the interrupted stack deadlocks.
  SetGracefulTermination( true );
}

bool SecondoMonitor::AbortOnSignal( int sig ) const
{
  return (false);
}

void SecondoMonitor::Usage()
{
  cout 
  << "The following commands are available:" << endl << endl
  << "  ?, HELP        - display this message" << endl
  << "  STARTUP        - of Listener, Registrar and Checkpoint processes" 
  << endl
  << "  SHUTDOWN       - of Listener, Registrar and Checkpoint processes" 
  << endl
  << "  SHOW {OPTION}  - show system status information" << endl
  << "                   OPTION = { LOG | USERS | DATABASES | LOCKS }" << endl
  << "                     LOG        - new log file entries" << endl
  << "                     USERS      - currently connected users" << endl
  << "                     DATABASES  - databases currently in use" << endl
  << "                     LOCKS      - databases currently locked" << endl
  << "  QUIT           - shut down (if necessary) and exit" << endl << endl;
}

void SecondoMonitor::ExecStartUp() {
  if (!running) {
    cout << "Startup in progress ... ";
    string pgmListener = SmiProfile::GetParameter("Environment", 
                                                "ListenerProgram", "", cfgFile);
    string pgmArgs = string( "\"" ) + cfgFile + "\" " + port + 
                     (port.empty() ? "" : " ") + dbDir;
    if (ProcessFactory::SpawnProcess(pgmListener, pgmArgs, pidListener, true)) {
      cout << "completed." << endl;
      running = true;
    }
    else {
      cout << "failed." << endl;
    }
  }
  else {
    cout << "Secondo Listener already running." << endl;
  }
}

void
SecondoMonitor::ExecShutDown()
{
  if ( running )
  {
    cout << "Shutdown in progress ... ";
    ProcessFactory::SignalProcess( pidListener );
    bool killed = false;
    if ( !ProcessFactory::WaitForProcess( pidListener, 30, &killed ) )
    {
      cout << "Secondo Listener could not be terminated." << endl;
    }
    else if ( killed )
    {
      cout << "Secondo Listener ignored the shutdown request "
           << "and had to be killed." << endl;
    }
    cout << "completed." << endl;
    int status = 0;
    ProcessFactory::GetExitCode( pidListener, status );
    cout << "Secondo Listener terminated with return code " 
         << status << "." << endl;
    running = false;
  }
  else
  {
    cout << "Secondo Listener not running." << endl;
  }
}

void
SecondoMonitor::ExecShow()
{
  string cmd(""), cmdword("");

  istringstream in(line);
  in >> cmdword; // eat up show
  in >> cmdword;
  transform( cmdword.begin(), cmdword.end(), 
             cmdword.begin(), ::toupper );

  if ( cmdword != "USERS"     && cmdword != "LOCKS" &&
       cmdword != "DATABASES" && cmdword != "LOG" )
  {
    cout << "show [option]: Invalid option '" << cmdword << "'" << endl
         << "Valid options are: 'log', 'users', "
         << "'databases' and 'locks'." << endl;
    return;
  }

  if      ( cmdword == "LOG"       ) cmd = "SHOWMSGS";
  else if ( cmdword == "USERS"     ) cmd = "SHOWUSERS";
  else if ( cmdword == "DATABASES" ) cmd = "SHOWDATABASES";
  else if ( cmdword == "LOCKS"     ) cmd = "SHOWLOCKS";
 
  string headerLine;
  vector<string> entries;
  if ( AskRegistrar( cfgFile, port, cmd, headerLine, entries ) )
  {
    istringstream is( headerLine );
    int rc, count;
    string dummy, header;
    is >> rc >> dummy >> count;
    if      ( cmdword == "LOG"       ) header = " log messages.";
    else if ( cmdword == "USERS"     ) header = " users logged in.";
    else if ( cmdword == "DATABASES" ) header = " databases in use.";
    else if ( cmdword == "LOCKS"     ) header = " database locks active.";
    cout << count << header << endl;
    cout << "------------------------------" << endl;
    for ( size_t i = 0; i < entries.size(); i++ )
    {
      cout << entries[i] << endl;
    }
    cout << "------------------------------" << endl;
  }
  else
  {
    cout << "Error: Connect to Secondo Registrar failed." << endl;
    cout << "*** Please shutdown, quit and restart SecondoMonitor ***" << endl;
  }
}

void SecondoMonitor::ExecQuit()
{
  if ( running )
  {
    cout << "Really shutdown the system and quit "
         << "(confirm with 'y' or 'yes')? " << endl
         << prompt;

    string answer("");
    getline( cin, answer );
    if ( answer == "y" || answer == "yes" )
    {
      ExecShutDown();
      quit = true;
    }
  }
  else
  {
    quit = true;
  }
  if ( quit )
  {
    cout << "" << endl;
  }
}

void SecondoMonitor::ProcessCommands() {
  map<string, cmdTok> commandTable;
  map<string, cmdTok>::iterator cmdPos;
  commandTable["?"] = xUsage;
  commandTable["HELP"] = xUsage;
  commandTable["STARTUP"] = xStartUp;
  commandTable["SHUTDOWN"] = xShutDown;
  commandTable["SHOW"] = xShow;
  commandTable["QUIT"] = xQuit;

  string cmd("");
  do {
    // Handle a termination signal first, on every iteration and independent of
    // stdin state. We have to do the shutdown outside of a signal handler.
    // Otherwise, we will get deadlocked in BerkeleyDB teardown;
    if (Application::Instance()->ShouldAbort()) {
      cout << endl
           << "*** Termination signal received, initiating shutdown!" << endl;
      if (running) {
        ExecShutDown();
      }
      quit = true;
      break;
    }

    if (!cin.eof()) {
      cout << prompt << flush;
      // Wait for a command line, but interruptibly. A plain getline blocks in a
      // read() that SA_RESTART restarts, so a signal would not break it; poll()
      // is interrupted and the abort is handled at the top of the loop.
      struct pollfd pfd;
      pfd.fd = STDIN_FILENO;
      pfd.events = POLLIN;
      int ready;
      do {
        ready = poll( &pfd, 1, 1000 );
      } while ( ready == 0 && !Application::Instance()->ShouldAbort() );

      if ( ready <= 0 ) {
        continue;
      }

      line = "";
      cmd = "";
      getline(cin, line);
      istringstream in(line);
      in >> cmd;

      if (cmd != "") {
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        cmdPos = commandTable.find(cmd);
        if (cmdPos != commandTable.end()) {

          switch (cmdPos->second) {

          case xUsage:
            Usage();
            break;

          case xStartUp:
            ExecStartUp();
            break;

          case xShutDown:
            ExecShutDown();
            break;

          case xShow:
            ExecShow();
            break;

          case xQuit:
            ExecQuit();
            break;

          default:
            cout << "Unkown Command '" << cmd << "'." << endl;
          }
        } else {
          cout << "Unknown Command '" << cmd << "'." << endl
               << "Enter 'HELP' or '?' to get a list of "
               << "valid commands." << endl;
        }
      }
    } else {
      poll( NULL, 0, 1000 );
    }
  } while (!quit);
}

bool
SecondoMonitor::CheckConfiguration()
{
  bool found = false;
  cout << "Checking configuration ..." << endl;

  // The processes spawned below (Listener, Registrar, Checkpoint and the
  // Secondo server itself) require SECONDO_BUILD_DIR to be set; without it
  // they fail only after being started. Detect this here and fail fast so the
  // error is reported by the monitor instead of the detached child processes.
  const char* buildDir = getenv( "SECONDO_BUILD_DIR" );
  if (buildDir == 0 || *buildDir == '\0') {
    cerr << "Environment variable SECONDO_BUILD_DIR is not set. "
         << "Terminating program." << endl;
    return false;
  }

  int pos = 1;
  string host, smi;
  while (pos < GetArgCount()) {
    string argValue(GetArgValues()[pos]);
    if (argValue == "-c") {
      pos++;
      if (pos >= GetArgCount()) {
        return false;
      }
      cfgFile = GetArgValues()[pos];
    }
    else if (argValue == "-d") {
      pos++;
      if (pos >= GetArgCount()) {
        return false;
      }
      dbDir = GetArgValues()[pos];
    }
    else if (argValue == "-p") {
      pos++;
      if (pos >= GetArgCount()) {
        return false;
      }
      port = GetArgValues()[pos];
    }
    else if (argValue == "-s") {}
    else {
      cout << "Invalid parameter " << GetArgValues()[pos] << endl;
      return false;
    }
    pos++;
  }

  // arguments are processed
  if (cfgFile.empty()) { // no cfgFile in argument list, search in environment
    char* config = getenv( "SECONDO_CONFIG" );
    if (config != 0) {
      cfgFile = config;
      cout << "Configuration file from environment variable: " 
           << cfgFile << endl;
    }
  } else {
    cout << "Configuration file from command line : " << cfgFile << endl;
  }
  if (cfgFile.empty()) { // no cfgFile in argument or environment
    string cwd = FileSystem::GetCurrentFolder();
    FileSystem::AppendSlash(cwd);
    cfgFile = cwd + "SecondoConfig.ini";
    cout << "No configuration file specified, use default one: " 
         << cfgFile << endl;
  }
  found = FileSystem::FileOrFolderExists(cfgFile);
  if (found) {
    // set SecondoHome
    string value, foundValue;
    if(dbDir.empty()){
       char* home = getenv( "SECONDO_HOME" );
       if (home != 0) {
         dbDir = home;
       }
       if(dbDir.empty()){
         dbDir = SmiProfile::GetParameter("Environment",
                                          "SecondoHome", "", cfgFile); 
       }
       if(dbDir.empty()){
         cerr << "SecondoHome not specified" << endl;
         return false;
       }
    }
    found = FileSystem::FileOrFolderExists(dbDir);
    if(!found){
      found = FileSystem::CreateFolderEx(dbDir);
      if(!found){
         cerr << "SecondoHome '" << dbDir 
              << "' does not exists and could not be created" << endl;
         return false;
      }
    } else if(!FileSystem::IsDirectory(dbDir)){
        cerr << "SecondoHome '" << dbDir << "' exists but is not "
             << " a directory" << endl;
        return false;
    }
    // process port
    if(port.empty()){
      port = SmiProfile::GetParameter("Environment", 
                                      "SecondoPort", "", cfgFile);
    }
    if(port.empty()){
      cerr << "port not specified as command line argument and not found in " 
           << cfgFile << endl;
      return false;
    }
    if(host.empty()){
       host = SmiProfile::GetParameter("Environment", 
                                  "SecondoHost", "", cfgFile);
    }
    if (smiType == SmiEnvironment::SmiBerkeleyDB) {
      value = SmiProfile::GetParameter("BerkeleyDB", "ServerProgram", "", 
                                       cfgFile);
      if (value == "" || !FileSystem::SearchPath(value, foundValue)) {
        cout << "Error: Server program '" << value << "' not found." << endl;
        return false;
      }
    } 
    else {
      cout << "Unknown SMI-Type" << endl;
      exit(1);
    } 
    cout << "Configuration seems to be ok." << endl << endl;
  }
  else {
    cout << "Sorry, configuration file '" << cfgFile 
         << "' not found. Terminating program." << endl;
  }
  return found;
}

bool
SecondoMonitor::Initialize()
{
  bool ok = true;
  // --- Start up process factory
  cout << "Initializing process management ... ";
  if ( !ProcessFactory::StartUp( false, 3 ) )
  {
    cout << "failed." << endl;
    return (false);
  }
  cout << "completed." << endl;

  // --- Check storage management interface
  cout << "Initializing storage management interface ... " << endl;
  if (SmiEnvironment::StartUp(SmiEnvironment::MultiUserMaster, 
                              cfgFile, dbDir, cout, port)) {
    cout << "completed." << endl;

    dbDir = SmiEnvironment::GetSecondoHome();

    if (smiType == SmiEnvironment::SmiBerkeleyDB) {
      cout << "Launching Checkpoint service ... ";
      string pgmCheckpoint = SmiProfile::GetParameter("BerkeleyDB", 
                                              "CheckpointProgram", "", cfgFile);
      string pgmArgs = cfgFile + " " + dbDir;
      if ( ProcessFactory::SpawnProcess(pgmCheckpoint, 
                                        pgmArgs, pidCheckpoint, true)) {
        cout << "completed." << endl;
      }
      else {
        cout << "failed." << endl;
        ok = false;
      }
    }
    else if ( smiType == SmiEnvironment::SmiOracleDB )
    {
      SmiEnvironment::ShutDown();
    }
  }
  else
  {
    cout << "failed." << endl;
    string errMsg;
    SmiEnvironment::GetLastErrorCode( errMsg );
    cout << "Error: " << errMsg << endl;
    ok = false;
  }

  if ( ok )
  {
    // --- Launch the Secondo registrar
    cout << "Launching Secondo Registrar ... ";
    string pgmRegistrar = SmiProfile::GetParameter( "Environment", 
                                                    "RegistrarProgram", 
                                                    "", cfgFile );

    string pgmArgs = string( "\"" ) + cfgFile + "\" " + port;
    if ( ProcessFactory::SpawnProcess( pgmRegistrar, 
                                       pgmArgs, pidRegistrar, true ) )
    {
      cout << "completed." << endl;
      ProcessFactory::Sleep( 0 );
    }
    else
    {
      cout << "failed." << endl;
      ok = false;
    }
  }
  return (ok);
}

void SecondoMonitor::Terminate()
{
  cout << "Terminating Secondo Monitor ..." << endl;
  if ( pidRegistrar != 0 )
  {
    cout << "Terminating Secondo Registrar ... ";
    ProcessFactory::SignalProcess( pidRegistrar );
    bool killed = false;
    if ( !ProcessFactory::WaitForProcess( pidRegistrar, 30, &killed ) )
    {
      cout << "Secondo Registrar could not be terminated." << endl;
    }
    else if ( killed )
    {
      cout << "Secondo Registrar ignored the shutdown request "
           << "and had to be killed." << endl;
    }
    cout << "completed." << endl;
    int status = 0;
    ProcessFactory::GetExitCode( pidRegistrar, status );
    cout << "Secondo Registrar terminated with return code " 
         << status << "." << endl;
  }
  if ( smiType == SmiEnvironment::SmiBerkeleyDB )
  {
    if ( pidCheckpoint != 0 )
    {
      cout << "Terminating Checkpoint Service ... ";
      ProcessFactory::SignalProcess( pidCheckpoint );
      // A checkpoint service wedged on a BerkeleyDB region lock used to hang
      // the monitor here forever; it is now killed once the grace period is up.
      // Say so explicitly: killing it may have orphaned a region lock, which is
      // the first thing to suspect if the SmiEnvironment shutdown below then
      // misbehaves.
      bool killed = false;
      if ( !ProcessFactory::WaitForProcess( pidCheckpoint, 30, &killed ) )
      {
        cout << "Checkpoint service could not be terminated." << endl;
        checkpointKilled = true;
      }
      else if ( killed )
      {
        cout << "Checkpoint service ignored the shutdown request and had to "
             << "be killed; a BerkeleyDB region lock may have been orphaned."
             << endl;
        checkpointKilled = true;
      }
      cout << "completed." << endl;
      int status = 0;
      ProcessFactory::GetExitCode( pidCheckpoint, status );
      cout << "Checkpoint service terminated with return code "
           << status << "." << endl;
    }
    // The checkpoint service spends its life inside Berkeley DB, so the only
    // reason it can ignore a shutdown request is that it is blocked in there,
    // holding a region mutex. Those mutexes are not robust: killing it leaves
    // the mutex locked forever, and the shutdown below -- which enters the very
    // same environment -- is then certain to block on it. That is not a
    // hypothetical, it is what the monitor did in CI.
    //
    // So do not enter Berkeley DB at all on that path. Nothing is lost by
    // skipping it: the monitor opens the environment with DB_RECOVER
    // (MultiUserMaster, see Initialize), so the next startup discards the
    // orphaned regions and recovers from the log, exactly as it would after any
    // other unclean exit. Hanging here, by contrast, wedges the shutdown and
    // strands every child process the monitor still owns.
    if ( checkpointKilled )
    {
      cout << "Skipping the shutdown of the storage management interface: "
           << "the killed checkpoint service may hold a Berkeley DB region "
           << "mutex, and closing the environment would block on it forever. "
           << "The next startup recovers the environment." << endl;
      // The temporary environment is still worth removing manually. Called
      // during ShutDown() and would be skipped if we did not do it here.
      SmiEnvironment::DeleteTmpEnvironment();
    }
    else if ( !SmiEnvironment::ShutDown( &cout ) )
    {
      string errMsg;
      SmiEnvironment::GetLastErrorCode( errMsg );
      cout << "Error: Shutdown of the storage management interface failed."
           << endl;
      cout << "Error: " << errMsg << endl;
    }
  }
  ProcessFactory::ShutDown();
  cout << "SecondoMonitor terminated." << endl;
}

/*
Reports the outcome of startup to whoever forked us, once. Every path out of
Execute() has to pass through here, otherwise a --daemon parent would wait for
the full startup timeout on a monitor that has already given up.

*/
void SecondoMonitor::SignalReady( bool ok )
{
  if ( readyFd < 0 )
  {
    return;
  }
  char status = ok ? '1' : '0';
  ssize_t written = write( readyFd, &status, 1 );
  (void) written;
  close( readyFd );
  readyFd = -1;
}

int SecondoMonitor::Execute(bool autostartup)
{
  cout << endl
       << "*** Secondo Monitor ***"
       << endl << endl;
  int rc = 0;
  if ( CheckConfiguration() )
  {
    if ( Initialize() )
    {
      cout << endl << "Secondo Monitor ready for operation." << endl
           << "Type 'HELP' to get a list of available commands." << endl;
      if(autostartup){
         ExecStartUp();
      }
      // Spawning the listener only means the process was created; it still has
      // to bind the port, and it may yet fail to. So do not report readiness
      // on "running" alone -- wait until the port really answers, which is the
      // property a caller of --daemon is waiting for.
      if ( readyFd >= 0 )
      {
        bool serving = false;
        bool listenerGone = !running;
        string host = ResolveHost( cfgFile );
        for ( int i = 0; !listenerGone && !serving
                         && i < daemonStartupSeconds * 10; i++ )
        {
          serving = PortAccepts( host, port );
          if ( !serving )
          {
            // A listener that failed to bind exits instead of retrying, so
            // notice that rather than waiting out the whole timeout for a
            // port that is never going to answer.
            listenerGone = ProcessFactory::IsProcessTerminated( pidListener );
            poll( NULL, 0, 100 );
          }
        }
        if ( !serving )
        {
          cout << "The Secondo Listener did not accept connections on port "
               << port << "." << endl;
          rc = 1;
        }
        SignalReady( serving );
      }
      if ( rc == 0 )
      {
        ProcessCommands();
      }
    }
    Terminate();
  }
  SignalReady( false );
  return (rc);
}

/*
2 Lifecycle commands

--shutdown and --health address a monitor that is already running, so they do
all their work before any SecondoMonitor object exists and never open the
storage management interface.

*/

static bool ReadPidFile( const string& pidFile, pid_t& pid )
{
  ifstream in( pidFile.c_str() );
  if ( !in )
  {
    return false;
  }
  long value = 0;
  in >> value;
  if ( in.fail() || value <= 0 )
  {
    return false;
  }
  pid = (pid_t) value;
  return true;
}

static bool ProcessExists( pid_t pid )
{
  return ( kill( pid, 0 ) == 0 ) || ( errno != ESRCH );
}

// FileSystem::DeleteFileOrFolder throws when there is nothing to delete, and
// every caller here removes the pid file on a path where it may or may not
// have been written yet.
static void RemovePidFile( const string& pidFile )
{
  if ( !pidFile.empty() && FileSystem::FileOrFolderExists( pidFile ) )
  {
    FileSystem::DeleteFileOrFolder( pidFile );
  }
}

/*
Stops the monitor named by the pid file and waits for it to be gone. Escalates
to SIGKILL rather than waiting forever: shutdown enters BerkeleyDB teardown and
is the part of the monitor known to wedge, so the one thing this must not do is
hang. Exits non-zero when the monitor had to be killed or was not running,
because both mean the caller cannot assume a clean environment.

*/
static int MonitorShutdown( const string& cfgArg, const string& portArg,
                            const string& pidArg )
{
  string cfgFile = ResolveConfigFile( cfgArg );
  string port    = ResolvePort( cfgFile, portArg );
  string pidFile = ResolvePidFile( pidArg, cfgFile, port );

  pid_t pid = 0;
  if ( !ReadPidFile( pidFile, pid ) )
  {
    cerr << "No SecondoMonitor pid file at '" << pidFile << "'." << endl;
    return 1;
  }
  if ( !ProcessExists( pid ) )
  {
    cerr << "No SecondoMonitor with pid " << pid
         << " is running; removing the stale pid file '" << pidFile << "'."
         << endl;
    RemovePidFile( pidFile );
    return 1;
  }

  cout << "Shutting down SecondoMonitor (pid " << pid << ") ... " << flush;
  kill( pid, SIGTERM );
  bool gone = false;
  for ( int i = 0; !gone && i < shutdownGraceSeconds * 10; i++ )
  {
    gone = !ProcessExists( pid );
    if ( !gone )
    {
      poll( NULL, 0, 100 );
    }
  }

  int rc = 0;
  if ( gone )
  {
    cout << "completed." << endl;
  }
  else
  {
    cout << "no reaction." << endl;
    cerr << "SecondoMonitor " << pid << " ignored the shutdown request for "
         << shutdownGraceSeconds << " seconds and had to be killed." << endl;
    kill( pid, SIGKILL );
    rc = 1;
  }
  RemovePidFile( pidFile );
  return rc;
}

/*
Answers whether a monitor on this port is serving. Two questions, because the
port probe on its own -- which is what every caller used to do in shell -- only
shows that the listener accepted a socket, and says nothing about the registrar
that arbitrates database access. SHOWUSERS is used because, unlike SHOWMSGS, it
does not consume anything on the registrar side.

*/
static int MonitorHealth( const string& cfgArg, const string& portArg )
{
  string cfgFile = ResolveConfigFile( cfgArg );
  string port    = ResolvePort( cfgFile, portArg );
  if ( port.empty() )
  {
    cerr << "No port given and none found in '" << cfgFile << "'." << endl;
    return 1;
  }
  string host = ResolveHost( cfgFile );
  string who  = host + ":" + port;

  if ( !PortAccepts( host, port ) )
  {
    cout << "SecondoMonitor on " << who
         << " is NOT healthy: nothing accepts connections." << endl;
    return 1;
  }

  string headerLine;
  vector<string> entries;
  if ( !AskRegistrar( cfgFile, port, "SHOWUSERS", headerLine, entries ) )
  {
    cout << "SecondoMonitor on " << who
         << " is NOT healthy: the Secondo Registrar does not answer." << endl;
    return 1;
  }
  istringstream is( headerLine );
  int replyCode = -1;
  int users = -1;
  string dummy;
  is >> replyCode >> dummy >> users;
  if ( is.fail() || users < 0 )
  {
    cout << "SecondoMonitor on " << who
         << " is NOT healthy: the Secondo Registrar answered '" << headerLine
         << "'." << endl;
    return 1;
  }
  cout << "SecondoMonitor on " << who << " is healthy, " << users
       << " user(s) connected." << endl;
  return 0;
}

/*
3 Daemon mode

*/

/*
Detaches the calling process and points its standard streams at the log file.
Deliberately keeps the working directory: the monitor starts its Listener,
Registrar, Checkpoint and server programs from there, and they write their own
logs there too, so a chdir("/") would scatter them.

*/
static bool Detach( const string& logFile )
{
  setsid();

  int devNull = open( "/dev/null", O_RDONLY );
  if ( devNull < 0 )
  {
    return false;
  }
  dup2( devNull, STDIN_FILENO );
  close( devNull );

  int log = open( logFile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644 );
  if ( log < 0 )
  {
    return false;
  }
  dup2( log, STDOUT_FILENO );
  dup2( log, STDERR_FILENO );
  if ( log > STDERR_FILENO )
  {
    close( log );
  }
  return true;
}

// Tells the process waiting in WaitForDaemon that this monitor will not come
// up. Used on the paths that fail before Execute() -- and thus SignalReady --
// is reached.
static void ReportStartupFailure( int readyFd )
{
  char status = '0';
  ssize_t written = write( readyFd, &status, 1 );
  (void) written;
  close( readyFd );
}

static bool WritePidFile( const string& pidFile, pid_t pid )
{
  ofstream out( pidFile.c_str(), ios::trunc );
  if ( !out )
  {
    return false;
  }
  out << pid << endl;
  return !out.fail();
}

/*
Waits for the monitor we just forked to report that it is serving, and turns
that into an exit code. Watches the child as well as the pipe, so a monitor
that dies during startup is reported at once instead of after the timeout.

*/
static int WaitForDaemon( pid_t pid, int readFd, const string& pidFile,
                          const string& logFile )
{
  char status = '0';
  bool reported = false;
  bool exited = false;

  for ( int i = 0; !reported && !exited && i < daemonStartupSeconds * 10; i++ )
  {
    struct pollfd pfd;
    pfd.fd = readFd;
    pfd.events = POLLIN;
    if ( poll( &pfd, 1, 100 ) > 0 )
    {
      reported = ( read( readFd, &status, 1 ) == 1 );
      break;
    }
    exited = ( waitpid( pid, NULL, WNOHANG ) == pid );
  }
  close( readFd );

  if ( reported && status == '1' )
  {
    cout << "SecondoMonitor running as pid " << pid << ", log '" << logFile
         << "'." << endl;
    return 0;
  }

  cerr << "SecondoMonitor failed to start; see '" << logFile << "'." << endl;
  // Do not leave a half-started monitor behind: it may already own a registrar
  // and a checkpoint service, and those would outlive us. A monitor that
  // reported its own failure is already unwinding through Terminate(), so let
  // it finish -- signalling it there would interrupt the BerkeleyDB teardown.
  // Only a monitor that never said anything gets asked to stop.
  if ( !exited )
  {
    if ( !reported )
    {
      kill( pid, SIGTERM );
    }
    bool gone = false;
    for ( int i = 0; !gone && i < shutdownGraceSeconds * 10; i++ )
    {
      gone = ( waitpid( pid, NULL, WNOHANG ) == pid );
      if ( !gone )
      {
        poll( NULL, 0, 100 );
      }
    }
    if ( !gone )
    {
      kill( pid, SIGKILL );
      waitpid( pid, NULL, 0 );
    }
  }
  RemovePidFile( pidFile );
  return 1;
}

int main( const int argc, const char* argv[] )
{
  bool execute = true;
  bool done = false;
  bool autostartup = false;
  bool daemonize = false;
  // Not named "shutdown"/"health": shutdown() is a POSIX function these
  // translation units already see through the socket headers.
  bool doShutdown = false;
  bool doHealth = false;
  // -c and -p are re-parsed by CheckConfiguration from the argument vector, but
  // the lifecycle commands run before there is a monitor to ask, so keep them.
  string cfgArg, portArg, pidArg, logArg;

  int pos = 1;
  while(pos<argc && !done){ // start at 1, 0 is the program name
     string arg;
     arg = argv[pos];
     if((arg=="-s") || (arg=="--startup")) {
        execute = true;
        autostartup = true;
        pos++;
     } else if (arg == "--daemon") {
        // Detach and report readiness, so that a caller does not have to poll
        // the port itself to find out whether the monitor came up.
        daemonize = true;
        autostartup = true;
        pos++;
     } else if (arg == "--shutdown") {
        doShutdown = true;
        pos++;
     } else if (arg == "--health") {
        doHealth = true;
        pos++;
     } else if ((arg == "--pidfile") || (arg == "--log")) {
        pos++;
        if(pos>=argc){
           cout << "missing argument to option " << arg << endl;
           return -1;
        }
        if (arg == "--pidfile") { pidArg = argv[pos]; }
        else                    { logArg = argv[pos]; }
        pos++;
#ifndef NO_OPTIMIZER
     } else if (arg == "--no-optimizer") {
        // Disable the server-side SQL optimizer for the servers this monitor
        // forks. They read SECONDO_PARAM_EnableOptimizer (an env override of
        // the config parameter Environment/EnableOptimizer) and inherit this
        // monitor's environment.
        setenv("SECONDO_PARAM_EnableOptimizer", "false", 1);
        pos++;
#endif
     } else if (arg == "--help") {
       // list allowed arguments
       cout << "Usage: " << argv[0]
            << " [option]. Combinations are not supported!" << endl;
       cout << "Options:" << endl;
       cout << "   --help           Display this information and exit" << endl;
       cout << "   -s or --startup  Run Startup command automatically" << endl;
       cout << "   --daemon         Like -s, but detach and return only once "
               "the monitor serves" << endl;
       cout << "   --shutdown       Stop the monitor named by the pid file"
        << endl;
       cout << "   --health         Report whether a monitor on this port is "
               "serving (exit 0/1)" << endl;
       #ifndef NO_OPTIMIZER
       cout << "   --no-optimizer  Disable the server-side optimizer " << endl;
       #endif
       cout << "   -V or --version  Display version information and exit"
        << endl << endl;
       cout << "The following parameters may be combined with \"-s\", "
               "\"--daemon\", \"--shutdown\" and \"--health\":" << endl;
       cout << "   -c    Specify a configuration file" << endl;
       cout << "   -d    Specify a database directory (override setting from "
               "configuration file)" << endl;
       cout << "   -p    Specify port (override setting from configuration "
               "file)" << endl << endl;
       cout << "The following parameters may be combined with \"--daemon\" "
               "and \"--shutdown\":" << endl;
       cout << "   --pidfile  File holding the pid of the detached monitor"
        << endl;
       cout << "             (default /tmp/<RegistrarSocketNamePrefix>_port"
               "<port>.pid)" << endl;
       cout << "   --log      Where --daemon sends the monitor output "
               "(default ./SecondoMonitor-<port>.log)" << endl;
       done = true;
       execute = false;
     } else if ((arg == "-V") || (arg == "--version")) {
       cout << argv[0] << " version " << VersionInfo << endl;
       execute = false;
       done = true;
     } else if ((arg == "-c") || (arg == "-d") || (arg == "-p")) {
       pos++;
       if(pos>=argc){
          cout << "missing argument to option " << arg << endl;
          execute = false;
          done = true;
          return -1;
       }
       if      (arg == "-c") { cfgArg  = argv[pos]; }
       else if (arg == "-p") { portArg = argv[pos]; }
       pos++; // jump over argument
     } else { // unknown command
       cout << "unknown command line argument '" << arg << "'" << endl;
       cout << "try " << argv[0] << " --help" << endl;
       execute = false;
       done = true;
       return -1;
     }
  }

  // Commands that address an already running monitor. They neither start one
  // nor open the storage management interface, so they are answered here.
  // Behind the execute flag so that --help and -V still win over them.
  if (execute && doShutdown) {
    return MonitorShutdown(cfgArg, portArg, pidArg);
  }
  if (execute && doHealth) {
    return MonitorHealth(cfgArg, portArg);
  }

  if(execute){
     string pidFile, logFile;
     int readyPipe[2] = { -1, -1 };

     if (daemonize) {
       string cfgFile = ResolveConfigFile(cfgArg);
       string port = ResolvePort(cfgFile, portArg);
       if (port.empty()) {
         cerr << "No port given and none found in '" << cfgFile << "'." << endl;
         return 1;
       }
       // Refuse rather than start a second monitor that would find the port
       // taken -- its listener would fail to bind while the monitor itself
       // came up, and it would then steal the registrar socket, which is
       // keyed on the port alone.
       if (PortAccepts(ResolveHost(cfgFile), port)) {
         cerr << "Port " << port << " is already in use; not starting a "
              << "second SecondoMonitor." << endl;
         return 1;
       }
       pidFile = ResolvePidFile(pidArg, cfgFile, port);
       logFile = logArg.empty() ? ("SecondoMonitor-" + port + ".log") : logArg;

       if (pipe(readyPipe) != 0) {
         cerr << "Could not create the readiness pipe: " << strerror(errno)
              << endl;
         return 1;
       }
       pid_t pid = fork();
       if (pid < 0) {
         cerr << "Could not fork: " << strerror(errno) << endl;
         return 1;
       }
       if (pid > 0) { // the caller: wait for the monitor to serve
         close(readyPipe[1]);
         return WaitForDaemon(pid, readyPipe[0], pidFile, logFile);
       }
       // the monitor itself, from here on detached
       close(readyPipe[0]);
       if (!Detach(logFile) || !WritePidFile(pidFile, getpid())) {
         // Too early for Execute() to report through the pipe, and after
         // Detach() there is nowhere left to print to, so answer the caller
         // here -- otherwise it would wait out the whole startup timeout.
         ReportStartupFailure(readyPipe[1]);
         _exit(1);
       }
     }

     // Strip monitor-only flags handled above (via the environment or the fork
     // above) from the argv before it reaches the configuration parser
     // (TTYParameter), which would otherwise reject them as invalid parameters.
     const char** monArgv = new const char*[argc];
     int monArgc = 0;
     for (int i = 0; i < argc; i++) {
       string a(argv[i]);
       if (a == "--no-optimizer" || a == "--daemon") continue;
       if (a == "--pidfile" || a == "--log") { i++; continue; }
       monArgv[monArgc++] = argv[i];
     }
     SecondoMonitor* appPointer = new SecondoMonitor( monArgc, monArgv );
     if (daemonize) {
       appPointer->SetReadyFd(readyPipe[1]);
     }
     int rc = appPointer->Execute(autostartup);
     delete appPointer;
     delete[] monArgv;
     if (daemonize) {
       RemovePidFile(pidFile);
     }
     return (rc);
  }else{
     return 0;
  }
}

