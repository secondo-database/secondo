using namespace std;

#include <cstdlib>
#include <string>
#include <algorithm>
#include <map>

#include "Application.h"
#include "Processes.h"
#include "SecondoSMI.h"
#include "Profiles.h"
#include "FileSystem.h"

class SecondoMonitor;
typedef void (SecondoMonitor::*ExecCommand)();

class SecondoMonitor : public Application
{
 public:
  SecondoMonitor( const int argc, const char** argv );
  virtual ~SecondoMonitor() {};
  bool AbortOnSignal( int sig );
  void Usage();
  void ExecStartUp();
  void ExecShutDown();
  void ExecShow();
  void ExecQuit();
  bool CheckConfiguration();
  bool Initialize();
  void ProcessCommands();
  void Terminate();
  int  Execute();
 private:
  SmiEnvironment::SmiType smiType;
  string parmFile;
  int  pidRegistrar;
  int  pidCheckpoint;
  int  pidListener;
  bool running;
  bool quit;
};

SecondoMonitor::SecondoMonitor( const int argc, const char** argv )
  : Application( argc, argv )
{
  smiType       = SmiEnvironment::GetImplementationType();
  parmFile      = "";
  pidRegistrar  = 0;
  pidListener   = 0;
  pidCheckpoint = 0;
  running       = false;
  quit          = false;
}

bool
SecondoMonitor::AbortOnSignal( int sig )
{
  return (false);
}

void
SecondoMonitor::Usage()
{
  cout << "The following commands are available:" << endl << endl
       << "  ?, HELP        - display this message" << endl
       << "  STARTUP        - start up the Secondo Listener" << endl
       << "  SHUTDOWN       - shut down the Secondo Listener" << endl
       << "  SHOW {OPTION}  - show system status information" << endl
       << "                   OPTION = { LOG | USERS | DATABASES | LOCKS }" << endl
       << "                     LOG        - new log file entries" << endl
       << "                     USERS      - currently connected users" << endl
       << "                     DATABASES  - databases currently in use" << endl
       << "                     LOCKS      - databases currently locked" << endl
       << "  QUIT           - shut down (if necessary) and exit" << endl << endl;
}

void
SecondoMonitor::ExecStartUp()
{
  if ( !running )
  {
    cout << "Startup in progress ... ";
    string pgmListener = SmiProfile::GetParameter( "Environment", "ListenerProgram", "", parmFile );
    string pgmArgs = string( "\"" ) + parmFile + "\"";
    if ( ProcessFactory::SpawnProcess( pgmListener, pgmArgs, pidListener, true ) )
    {
      cout << "completed." << endl;
      running = true;
    }
    else
    {
      cout << "failed." << endl;
    }
  }
  else
  {
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
    ProcessFactory::WaitForProcess( pidListener );
    cout << "completed." << endl;
    int status = 0;
    ProcessFactory::GetExitCode( pidListener, status );
    cout << "Secondo Listener terminated with return code " << status << "." << endl;
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
  string cmd, cmdword, cmdrest, answer;
  cin >> cmdword;
  getline( cin, cmdrest );
  transform( cmdword.begin(), cmdword.end(), cmdword.begin(), toupper );
  if ( cmdword != "USERS"     && cmdword != "LOCKS" &&
       cmdword != "DATABASES" && cmdword != "LOG" )
  {
    cout << "Invalid SHOW option '" << cmdword << "'." << endl
         << "Valid are: 'LOG', 'USERS', 'DATABASES' and 'LOCKS'." << endl;
    return;
  }

  if      ( cmdword == "LOG"       ) cmd = "SHOWMSGS";
  else if ( cmdword == "USERS"     ) cmd = "SHOWUSERS";
  else if ( cmdword == "DATABASES" ) cmd = "SHOWDATABASES";
  else if ( cmdword == "LOCKS"     ) cmd = "SHOWLOCKS";
 
  string regName = SmiProfile::GetParameter( "Environment", "RegistrarName", "SECONDO_REGISTRAR", parmFile );
  Socket* msgClient = Socket::Connect( regName, "", Socket::SockLocalDomain );
  if ( msgClient && msgClient->IsOk() )
  {
    iostream& ss = msgClient->GetSocketStream();
    ss << cmd << endl;
    do
    {
      getline( ss, answer );
      cout << answer << endl;
    }
    while (answer[0] != '0' && answer[0] != '-' && !ss.fail());
  }
  else
  {
    cout << "Error: Connect to Secondo Registrar failed." << endl;
    cout << "*** Please shutdown, quit and restart SecondoMonitor ***" << endl;
  }
  if ( msgClient )
  {
    delete msgClient;
  }
}

void
SecondoMonitor::ExecQuit()
{
  string cmdrest, answer;
  getline( cin, cmdrest );
  if ( running )
  {
    cout << "Are you sure you want to shutdown the system and quit? [yes/no]: ";
    getline( cin, answer );
    if ( answer == "y" || answer == "yes" ||
         answer == "Y" || answer == "YES" )
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

void
SecondoMonitor::ProcessCommands()
{
  map<string,ExecCommand> commandTable;
  map<string,ExecCommand>::iterator cmdPos;
  commandTable["?"]        = &SecondoMonitor::Usage;
  commandTable["HELP"]     = &SecondoMonitor::Usage;
  commandTable["STARTUP"]  = &SecondoMonitor::ExecStartUp;
  commandTable["SHUTDOWN"] = &SecondoMonitor::ExecShutDown;
  commandTable["SHOW"]     = &SecondoMonitor::ExecShow;
  commandTable["QUIT"]     = &SecondoMonitor::ExecQuit;

  string cmd, cmdword, cmdrest, answer;
  do
  {
    cout << "Monitor> ";
    cin >> cmd;
    transform( cmd.begin(), cmd.end(), cmd.begin(), toupper );
    cmdPos = commandTable.find( cmd );
    if ( cmdPos != commandTable.end() )
    {
        (*this.*(cmdPos->second))();
    }
    else
    {
      getline( cin, cmdrest );
      cout << "Unknown Command '" << cmd << "'." << endl
           << "Enter 'HELP' or '?' to get a list of valid commands." << endl;
    }
    if ( Application::Instance()->ShouldAbort() )
    {
      cout << "*** Termination signal received, please shutdown and quit immediately!" << endl;
      ExecQuit();
    }
  }
  while (!quit);
}

bool
SecondoMonitor::CheckConfiguration()
{
  bool found = false;
  cout << "Checking configuration ..." << endl;
  // --- Find configuration file
  parmFile = (GetArgCount() > 1) ? GetArgValues()[1] : "";
  if ( parmFile.length() > 0 )
  {
    cout << "Configuration file '" << parmFile;
    found = FileSystem::FileOrFolderExists( parmFile );
    if ( found )
    {
      cout << "':" << endl;
    }
    else
    {
      cout << "' not found!" << endl;
    }
  }
  if ( !found )
  {
    cout << "Searching environment for configuration file ..." << endl;
    char* home = getenv( "SECONDO_HOME" );
    if ( home != 0 )
    {
      parmFile = home;
      FileSystem::AppendSlash( parmFile );
      parmFile += "SecondoConfig.ini";
      cout << "Configuration file '" << parmFile;
      found = FileSystem::FileOrFolderExists( parmFile );
      if ( found )
      {
        cout << "':" << endl;
      }
      else
      {
        cout << "' not found!" << endl;
      }
    }
    else
    {
      cout << "Environment variable SECONDO_HOME not defined." << endl;
    }
    if ( !found )
    {
      cout << "Searching current directory for configuration file ..." << endl;
      string cwd = FileSystem::GetCurrentFolder();
      FileSystem::AppendSlash( cwd );
      parmFile = cwd + "SecondoConfig.ini";
      cout << "Configuration file '" << parmFile;
      found = FileSystem::FileOrFolderExists( parmFile );
      if ( found )
      {
        cout << "':" << endl;
      }
      else
      {
        cout << "' not found!" << endl;
      }
    }
  }
  if ( found )
  {
    string value, foundValue;
    if ( SmiProfile::GetParameter( "Environment", "SecondoHome", "", parmFile ) == "")
    {
      cout << "Error: Secondo home directory not specified." << endl;
      found = false;
    }
    if ( SmiProfile::GetParameter( "Environment", "SecondoHost", "", parmFile ) == "" ||
         SmiProfile::GetParameter( "Environment", "SecondoPort", "", parmFile ) == "" )
    {
      cout << "Error: Secondo host and/or port not specified." << endl;
      found = false;
    }
    if ( smiType == SmiEnvironment::SmiBerkeleyDB )
    {
      value = SmiProfile::GetParameter( "BerkeleyDB", "ServerProgram", "", parmFile );
      if ( value == "" || !FileSystem::SearchPath( value, foundValue ) )
      {
        cout << "Error: Server program '" << value << "' not found." << endl;
        found = false;
      }
    }
    else if ( smiType == SmiEnvironment::SmiOracleDB )
    {
      if ( SmiProfile::GetParameter( "OracleDB", "ConnectString", "", parmFile ) == "" ||
           SmiProfile::GetParameter( "OracleDB", "SecondoUser", "", parmFile ) == "" ||
           SmiProfile::GetParameter( "OracleDB", "SecondoPswd", "", parmFile ) == "" )
      {
        cout << "Error: Oracle connect parameters incomplete." << endl;
        found = false;
      }
      value = SmiProfile::GetParameter( "OracleDB", "ServerProgram", "", parmFile );
      if ( value == "" || !FileSystem::SearchPath( value, foundValue ) )
      {
        cout << "Error: Server program '" << value << "' not found." << endl;
        found = false;
      }
      cout << "Found file name: " << foundValue << endl;
    }
    if ( found )
    {
      cout << "Configuration seems to be ok." << endl << endl;
    }
    else
    {
      cout << "Sorry, configuration parameters missing. Terminating program." << endl;
    }
  }
  else
  {
    cout << "Sorry, no configuration file found. Terminating program." << endl;
  }
  return (found);
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
  cout << "Initializing storage management interface ... ";
  if ( SmiEnvironment::StartUp( SmiEnvironment::MultiUserMaster, parmFile, cout ) )
  {
    cout << "completed." << endl;
    if ( smiType == SmiEnvironment::SmiBerkeleyDB )
    {
      cout << "Launching Checkpoint service ... ";
      string pgmCheckpoint = SmiProfile::GetParameter( "BerkeleyDB", "CheckpointProgram", "", parmFile );
      string pgmArgs = string( "\"" ) + parmFile + "\"";
      if ( ProcessFactory::SpawnProcess( pgmCheckpoint, pgmArgs, pidCheckpoint, true ) )
      {
        cout << "completed." << endl;
      }
      else
      {
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
    string pgmRegistrar = SmiProfile::GetParameter( "Environment", "RegistrarProgram", "", parmFile );
    string pgmArgs = string( "\"" ) + parmFile + "\"";
    if ( ProcessFactory::SpawnProcess( pgmRegistrar, pgmArgs, pidRegistrar, true ) )
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

void
SecondoMonitor::Terminate()
{
  cout << "Terminating Secondo Monitor ..." << endl;
  if ( pidRegistrar != 0 )
  {
    cout << "Terminating Secondo Registrar ... ";
    ProcessFactory::SignalProcess( pidRegistrar );
    ProcessFactory::WaitForProcess( pidRegistrar );
    cout << "completed." << endl;
    int status = 0;
    ProcessFactory::GetExitCode( pidRegistrar, status );
    cout << "Secondo Registrar terminated with return code " << status << "." << endl;
  }
  if ( smiType == SmiEnvironment::SmiBerkeleyDB )
  {
    if ( pidCheckpoint != 0 )
    {
      cout << "Terminating Checkpoint Service ... ";
      ProcessFactory::SignalProcess( pidCheckpoint );
      ProcessFactory::WaitForProcess( pidCheckpoint );
      cout << "completed." << endl;
      int status = 0;
      ProcessFactory::GetExitCode( pidCheckpoint, status );
      cout << "Checkpoint service terminated with return code " << status << "." << endl;
    }
    if ( !SmiEnvironment::ShutDown() )
    {
      string errMsg;
      SmiEnvironment::GetLastErrorCode( errMsg );
      cout << "Error: Shutdown of the storage management interface failed." << endl;
      cout << "Error: " << errMsg << endl;
    }
  }
  ProcessFactory::ShutDown();
  cout << "SecondoMonitor terminated." << endl;
}

int
SecondoMonitor::Execute()
{
  cout << endl
       << "*** Secondo Monitor ***"
       << endl << endl;
  if ( CheckConfiguration() )
  {
    if ( Initialize() )
    {
      cout << endl << "Secondo Monitor ready for operation." << endl
           << "Type 'HELP' to get a list of available commands." << endl;
      ProcessCommands();
    }
    Terminate();
  }
  return (0);
}

int main( const int argc, const char* argv[] )
{
  SecondoMonitor* appPointer = new SecondoMonitor( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

