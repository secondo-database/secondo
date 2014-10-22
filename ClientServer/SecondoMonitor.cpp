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
#include <string>
#include <algorithm>
#include <map>
#include <iostream>
#include <sstream>

#include "Application.h"
#include "Processes.h"
#include "SecondoSMI.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "CharTransform.h"
#include "WinUnix.h"

using namespace std;

class SecondoMonitor : public Application
{
 public:
  SecondoMonitor( const int argc, const char** argv );
  virtual ~SecondoMonitor() {};
  int  Execute(bool autostartup);

 private:
  bool AbortOnSignal( int sig ) const;
  void Usage();
  void ExecStartUp();
  void ExecShutDown();
  void ExecShow();
  void ExecQuit();
  bool CheckConfiguration();
  bool Initialize();
  void ProcessCommands();
  void Terminate();
  
  static SecondoMonitor* p;
  static void HandleShutDown(int sig);

  SmiEnvironment::SmiType smiType;
  string parmFile;
  string prompt;
  string line;
  int  pidRegistrar;
  int  pidCheckpoint;
  int  pidListener;
  bool running;
  bool quit;

  typedef enum {xUsage, xStartUp, xShutDown, xShow, xQuit} cmdTok;	
};


// string defining the version of the SecondoMonitor
static string VersionInfo ="1.1";


SecondoMonitor::SecondoMonitor( const int argc, const char** argv )
  : Application( argc, argv )
{
  smiType       = SmiEnvironment::GetImplementationType();
  parmFile      = "";
  prompt        = "SEC_MON> ";
  line          = "";
  pidRegistrar  = 0;
  pidListener   = 0;
  pidCheckpoint = 0;
  running       = false;
  quit          = false;

  p = this;
#ifndef SECONDO_WIN32
  signal(SIGTERM, HandleShutDown);
  signal(SIGINT,  HandleShutDown);
  signal(SIGKILL, HandleShutDown);
#endif
}


void SecondoMonitor::HandleShutDown(int sig) {

#ifndef SECONDO_WIN32
  if (sig == SIGTERM || sig == SIGINT || sig == SIGKILL) 
  {
    cerr << endl << "SIGTERM, SIGINT or SIGKILL received, "
	 << "terminating child processes." << endl;

    if (p) {
      p->ExecShutDown();
      p->Terminate();
      p = 0;
    }  
  }	    
  signal(sig, SIG_DFL);
  raise(sig);	    
#endif

}	  

SecondoMonitor* SecondoMonitor::p = 0;

bool
SecondoMonitor::AbortOnSignal( int sig ) const
{
  return (false);
}

void
SecondoMonitor::Usage()
{
  cout 
  << "The following commands are available:" << endl << endl
  << "  ?, HELP        - display this message" << endl
  << "  STARTUP        - of Listener, Registrar and CheckPoint processes" 
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

void
SecondoMonitor::ExecStartUp()
{
  if ( !running )
  {
    cout << "Startup in progress ... ";
    string pgmListener = SmiProfile::GetParameter( "Environment", 
                                                   "ListenerProgram", 
                                                   "", parmFile );

    string pgmArgs = string( "\"" ) + parmFile + "\"";
    if ( ProcessFactory::SpawnProcess( pgmListener, pgmArgs, 
                                       pidListener, true )   )
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
  string cmd(""), cmdword(""), answer("");
  
  istringstream in(line);
  in >> cmdword; // eat up show
  in >> cmdword;
  transform( cmdword.begin(), cmdword.end(), 
             cmdword.begin(), ToUpperProperFunction );

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
 
  string regName = SmiProfile::GetUniqueSocketName( parmFile );

  Socket* msgClient = Socket::Connect( regName, "", Socket::SockLocalDomain );
  if ( msgClient && msgClient->IsOk() )
  {
    iostream& ss = msgClient->GetSocketStream();
    ss << cmd << endl;
    bool first = true;
    do
    {
      getline( ss, answer );
      if ( first )
      {
        first = false;
        istringstream is( answer );
        int rc, count;
        string dummy, header;
        is >> rc >> dummy >> count;
        if      ( cmdword == "LOG"       ) header = " log messages.";
        else if ( cmdword == "USERS"     ) header = " users logged in.";
        else if ( cmdword == "DATABASES" ) header = " databases in use.";
        else if ( cmdword == "LOCKS"     ) header = " database locks active.";
        cout << count << header << endl;
        cout << "------------------------------" << endl;
      }
      else
      {
        if ( answer[0] != '0' && answer[0] != '-' && !ss.fail() )
        {
          cout << answer << endl;
        }
      }
    }
    while (answer[0] != '0' && answer[0] != '-' && !ss.fail());
    cout << "------------------------------" << endl;
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

void
SecondoMonitor::ProcessCommands()
{
  map<string,cmdTok> commandTable;
  map<string,cmdTok>::iterator cmdPos;
  commandTable["?"]        = xUsage;
  commandTable["HELP"]     = xUsage;
  commandTable["STARTUP"]  = xStartUp;
  commandTable["SHUTDOWN"] = xShutDown;
  commandTable["SHOW"]     = xShow;
  commandTable["QUIT"]     = xQuit;

  string cmd("");
  do
  {
    if (!cin.eof()) 
    {	  
      line = "";
      cmd = "";
      cout << prompt;
      getline( cin, line );
      //cout << "line = '" << line << "'" << endl;
      istringstream in(line);
      in >> cmd;
      //cout << "input = '" << cmd << "'" << endl;

      if (cmd != "") 
      {
        transform( cmd.begin(), cmd.end(), cmd.begin(), ToUpperProperFunction );
        cmdPos = commandTable.find( cmd );
	if ( cmdPos != commandTable.end() )
	{

          switch (cmdPos->second) {

          case xUsage:   Usage(); break;  

          case xStartUp: ExecStartUp(); break;

	  case xShutDown: ExecShutDown(); break;

	  case xShow:     ExecShow(); break;

	  case xQuit:     ExecQuit(); break;

          default:
            cout << "Unkown Command '" << cmd << "'." << endl;
	  }	
	}
	else
	{
	  cout << "Unknown Command '" << cmd << "'." << endl
	       << "Enter 'HELP' or '?' to get a list of "
	       << "valid commands." << endl;
	}
      } 

      if ( Application::Instance()->ShouldAbort() )
      {
	cout << "*** Termination signal received, initiating shutdown!" 
	     << endl;
	ExecQuit();
      }
    }
    else
    {
      // since input is eof avoid consuming cpu time	    
      WinUnix::sleep(60);
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
    char* config = getenv( "SECONDO_CONFIG" );
    if ( config != 0 )
    {
      parmFile = config;
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
      cout << "Environment variable SECONDO_CONFIG not defined." << endl;
    }
    if ( !found )
    {
      cout << "Searching current directory for configuration file ..."
           << endl;
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
    if ( SmiProfile::GetParameter( "Environment", 
                                   "SecondoHome", "", parmFile ) == "" )
    {
      cout << "Error: Secondo home directory not specified." << endl;
      found = false;
    }
    if ( SmiProfile::GetParameter( "Environment", 
                                   "SecondoHost", "", parmFile ) == "" ||
         SmiProfile::GetParameter( "Environment", 
                                   "SecondoPort", "", parmFile ) == ""    )
    {
      cout << "Error: Secondo host and/or port not specified." << endl;
      found = false;
    }
    if ( smiType == SmiEnvironment::SmiBerkeleyDB )
    {
      value = SmiProfile::GetParameter( "BerkeleyDB", 
                                        "ServerProgram", "", parmFile );
      
      if ( value == "" || !FileSystem::SearchPath( value, foundValue ) )
      {
        cout << "Error: Server program '" << value << "' not found." << endl;
        found = false;
      }
    } else {
      cout << "Unknown SMI-Type" << endl;
      exit(1);
    } 
     
    if ( found )
    {
      cout << "Configuration seems to be ok." << endl << endl;
    }
    else
    {
      cout << "Sorry, configuration parameters missing. Terminating program."
           << endl;
    }
  }
  else
  {
    cout << "Sorry, no configuration file found. Terminating program."
         << endl;
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
  if ( SmiEnvironment::StartUp( SmiEnvironment::MultiUserMaster, 
                                parmFile, cout )                  )
  {
    cout << "completed." << endl;
    if ( smiType == SmiEnvironment::SmiBerkeleyDB )
    {
      cout << "Launching Checkpoint service ... ";
      string pgmCheckpoint = SmiProfile::GetParameter( "BerkeleyDB", 
                                                       "CheckpointProgram", 
                                                       "", 
                                                       parmFile );

      string pgmArgs = string( "\"" ) + parmFile + "\"";
      if ( ProcessFactory::SpawnProcess( pgmCheckpoint, 
                                         pgmArgs, pidCheckpoint, true ) )
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
    string pgmRegistrar = SmiProfile::GetParameter( "Environment", 
                                                    "RegistrarProgram", 
                                                    "", parmFile );

    string pgmArgs = string( "\"" ) + parmFile + "\"";
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
    cout << "Secondo Registrar terminated with return code " 
         << status << "." << endl;
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
      cout << "Checkpoint service terminated with return code " 
           << status << "." << endl;
    }
    if ( !SmiEnvironment::ShutDown() )
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

int
SecondoMonitor::Execute(bool autostartup)
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
      if(autostartup){
         ExecStartUp();
      } 
      ProcessCommands();
    }
    Terminate();
  }
  return (0);
}

int main( const int argc, const char* argv[] )
{
  bool execute = true;
  bool done = false;
  bool autostartup = false;

  for(int i=1; i<argc && !done ;i++){ // start at 1, 0 is the program name
     string arg;
     arg = argv[i];
     if( (arg=="-s") || (arg=="-startup") ){
        done = true;
        execute = true;
        autostartup = true;
     } else if(arg=="--help"){
        // list allowed arguments
        cout << "Usage: " << argv[0] 
	     << " [option]. Combinations are not supported!" 
	     << endl;
        cout << "Options:" << endl;
        cout << "   --help          Display this information and exit"
             << endl;
        cout << "   -s or -startup  Run Startup command automatically" 
	     << endl;
        cout << "   -sc1            Run Startup command and close stdin" 
	     << endl;
        cout << "   -V or -version  Display version information and exit"
             << endl;
        done = true;
        execute = false;
     } else if((arg=="-V") || (arg=="-version")){
       cout << argv[0] << " version " << VersionInfo << endl;
       execute = false;;
     } else { // unknow command
        cout << "unknow command line argument" << endl;
        cout << "try " << argv[0] << " --help" << endl;
        execute = false;
        return -1;
     }
  }

  if(execute){
     SecondoMonitor* appPointer = new SecondoMonitor( argc, argv );
     int rc = appPointer->Execute(autostartup);
     delete appPointer;
     return (rc);
  }else{
     return 0;
  }
}

