/*
\def\CC{C\raise.22ex\hbox{{\footnotesize +}}\raise.22ex\hbox{\footnotesize +}\xs
pace}
\centerline{\LARGE \bf  SecondoTTY}
 
\centerline{Friedhelm Becker , Dec1997}

Changes:

July 1999: Jose Antonio Cotelo Lema: changes in the code and interface of the
Gettext() and getline() functions, to allow input commands of arbitrary size. 

\begin{center}
\footnotesize
\tableofcontents
\end{center}

1 Overview

This is the TTY-Interface of Secondo. It makes it possible to enter queries
to Secondo and returns the output from Secondo. Input can be from  keyboard
or from file. In the latter you must enter ~uf~ or ~use file~ and
then you will be prompted for the filename.
 
2 Includes and defines

*/

using namespace std;

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Application.h"
//#include "Processes.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "SecondoSystem.h"
#include "SecondoInterface.h"
#include "SecondoSMI.h"
#include "NestedList.h"
#include "DisplayTTY.h"

class SecondoTTY : public Application
{
 public:
  SecondoTTY( const int argc, const char** argv );
  virtual ~SecondoTTY() {};
  bool AbortOnSignal( int sig );
  void Usage();
  void ProcessFile( const string& fileName );
  void ProcessCommand();
  void ProcessCommands();
  bool CheckConfiguration();
  bool Initialize();
  void Terminate();
  int  Execute();
  void ShowPrompt( const bool first );
  void TypeOutputList ( ListExpr list );
  bool IsInternalCommand( const string& line );
  bool GetCommand();
  void ShowQueryResult( ListExpr list );
  void WriteErrorList ( ListExpr list );
  ListExpr CallSecondo();
  void CallSecondo2();
  void TestNL();
 private:
  SmiEnvironment::SmiType smiType;
  string parmFile;
  int  pidCheckpoint;
  string cmd;
  bool isStdInput;
  bool quit;
  NestedList* nl;
  AlgebraLevel currentLevel;
  bool batchMode;
  bool isQuery;
  SecondoInterface* si;
};

void
SecondoTTY::TestNL()
{
  ListExpr list;
cout << "TestNL start" << endl;
  nl->ReadFromString( "(<text> (int int) -> int, (int real) -> real, (real int) -> real, (real real) -> real </text---><text> Addition. </text--->)", list );
cout << "TestNL before write" << endl;
  nl->WriteListExpr( list );
cout << "TestNL stop" << endl;
}

SecondoTTY::SecondoTTY( const int argc, const char** argv )
  : Application( argc, argv )
{
  smiType       = SmiEnvironment::GetImplementationType();
  parmFile      = "";
  pidCheckpoint = 0;
  string cmd    = "";
  isStdInput    = true;
  quit          = false;
  nl            = 0;
  currentLevel  = DescriptiveLevel;
  batchMode     = false;
  si            = 0;
}

bool
SecondoTTY::AbortOnSignal( int sig )
{
  return (false);
}

void
SecondoTTY::Usage()
{
  cout << "The following internal commands are available:" << endl << endl
       << "  ?, HELP         - display this message" << endl
       << "  @{FILE}         - read commands from file 'FILE' (may be nested)" << endl
       << "  D, DESCRIPTIVE  - set 'DESCRIPTIVE' level" << endl
       << "  E, EXECUTABLE   - set 'EXECUTABLE' level" << endl
       << "  H, HYBRID       - set 'HYBRID' level, i.e. commands are executed" << endl
       << "                    first at 'DESCRIPTIVE' and then at 'EXECUTABLE' level" << endl
       << "  SHOW {OPTION}   - show system status information" << endl
       << "                    OPTION = { LEVEL }" << endl
       << "                      LEVEL      - current level" << endl
       << "  Q, QUIT         - exit the program" << endl << endl
       << "  # ...           - comment line (first character on line has to be '#')" << endl << endl
       << "Additionally you may enter any valid Secondo command." << endl
       << "Internal commands are restricted to ONE line, while Secondo commands may span" << endl
       << "several lines; a semicolon as the last character on a line ends a command, but" << endl
       << "is not part of the command, alternatively you may enter an empty line." << endl << endl;
}

void
SecondoTTY::ProcessFile( const string& fileName )
{
  streambuf* oldBuffer;
  ifstream fileInput( fileName.c_str() );
  if ( fileInput )
  {
    oldBuffer = cin.rdbuf( fileInput.rdbuf() );
    cout << "*** Begin processing file '" << fileName << "'." << endl;
    ProcessCommands();
    cout << "*** End processing file '" << fileName << "'." << endl;
    cin.rdbuf( oldBuffer );
  }
  else
  {
    cerr << "*** Error: Could not access file '" << fileName << "'." << endl;
  }
}

void
SecondoTTY::ProcessCommand()
{
  string cmdWord;
  istringstream is( cmd );
  is >> cmdWord;
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), toupper );

  if ( cmdWord == "?" || cmdWord == "HELP" )
  {
    Usage();
  }
  else if ( cmdWord == "D" || cmdWord == "DESCRIPTIVE" )
  {
    currentLevel = DescriptiveLevel;
    cout << "*** Level set to 'DESCRIPTIVE'." << endl;
  }
  else if ( cmdWord == "E" || cmdWord == "EXECUTABLE" )
  {
    currentLevel = ExecutableLevel;
    cout << "*** Level set to 'EXECUTABLE'." << endl;
  }
  else if ( cmdWord == "H" || cmdWord == "HYBRID" )
  {
    currentLevel = HybridLevel;
    cout << "*** Level set to 'HYBRID'." << endl;
  }
  else if ( cmdWord == "SHOW" )
  {
    is >> cmdWord;
    transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), toupper );
    if ( cmdWord == "LEVEL" )
    {
      switch (currentLevel)
      {
        case DescriptiveLevel:
        {
          cout << "*** Current level is 'DESCRIPTIVE'." << endl;
          break;
        }
        case ExecutableLevel:
        {
          cout << "*** Current level is 'EXECUTABLE'." << endl;
          break;
        }
        case HybridLevel:
        {
          cout << "*** Current level is 'HYBRID'." << endl;
          break;
        }
        default:
        {
          cout << "*** Current level is invalid." << endl;
          break;
        }
      }
    }
    else
    {
      cout << "*** Invalid SHOW option '" << cmdWord << "'." << endl;
    }
  }
  else if ( cmdWord == "Q" || cmdWord == "QUIT" )
  {
    cout << "*** Thank you for using SECONDO!" << endl;
    quit = true;
  }
  else if ( cmdWord[0] == '@' )
  {
    ProcessFile( cmd.substr( 1 ) );
  }
  else
  {
    if ( currentLevel == HybridLevel )
    {
      if ( cmdWord == "QUERY" || cmdWord == "(QUERY" ||
           cmdWord == "UPDATE" || cmdWord == "(UPDATE" )
      {
        cout << "*** Hey, don't do that in 'HYBRID' mode!" << endl;
      }
      else
      {
        currentLevel = DescriptiveLevel;
        CallSecondo2();
        currentLevel = ExecutableLevel;
        CallSecondo2();
        currentLevel = HybridLevel;
      }
    }
    else
    {
      CallSecondo2();
    }
  }
}

void
SecondoTTY::ShowPrompt( const bool first )
{
  if ( isStdInput ) // Display input prompt
  {
    if ( first )
    {
      cout << "Secondo => ";     // First line of command
    }
    else
    {
      cout << "Secondo -> ";     // Continuation line of command
    }
  }
}

bool
SecondoTTY::IsInternalCommand( const string& line )
{
  string cmdWord;
  istringstream is( line );
  is >> cmdWord;
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), toupper );

  return ( cmdWord == "?" || cmdWord == "HELP"        ||
           cmdWord == "D" || cmdWord == "DESCRIPTIVE" ||
           cmdWord == "E" || cmdWord == "EXECUTABLE"  ||
           cmdWord == "H" || cmdWord == "HYBRID"      ||
           cmdWord == "Q" || cmdWord == "QUIT"        ||
           cmdWord == "SHOW" || cmdWord[0] == '@' );
}

bool
SecondoTTY::GetCommand()
{
  bool complete = false;
  bool first = true;
  string line = "";
  cmd = "";
  while (!complete && !cin.eof() && !cin.fail())
  {
    line = "";
    ShowPrompt( first );
    getline( cin, line );
    if ( line.length() > 0 )
    {
      if ( !isStdInput )           // Echo input if not standard input
      {
        cout << " " << line << endl;
      }
      if ( line[0] != '#' )        // Process if not comment line
      {
        if ( line[line.length()-1] == ';' )
        {
          complete = true;
          line.erase( line.length()-1 );
        }
        if ( first )               // Check for single line command
        {
          if ( !complete )
          {
            complete = IsInternalCommand( line );
          }
          cmd = line;
          first = false;
        }
        else
        {
          cmd += line;
        }
      }
    }
    else                           // Empty line ends command
    {
      complete = cmd.length() > 0;
      first = true;
    }
  }
  return (complete);
}

void
SecondoTTY::ProcessCommands()
{
  while (!cin.eof() && !quit)
  {
    if ( GetCommand() )
    {
      ProcessCommand();
    }
  }
}

/*
8 TypeOutputList

TypeOutputList prints the result of a nonquery input (e. g. list).

*/

void
SecondoTTY::TypeOutputList ( ListExpr list )
{
  if ( nl->IsEmpty( list ) )
  {
    cout << "=> []" << endl;
  }
  else
  {
    cout << "=> Result:" << endl;
cout << "List " << list << endl;
    nl->WriteListExpr( list );
//    nl->WriteToFile( "tol.xxx", list );
    cout << endl;
  }
}

/*
9 ShowQueryResult

This function prints the result of a query by calling DisplayTTY,
which writes the result in a file.  This file is then printed by
this function. The filename is given to DisplayTTY by this function. 

*/

void
SecondoTTY::ShowQueryResult( ListExpr list )
{
  if ( nl->IsEmpty( list ) || (nl->ListLength( list ) != 2) )
  {
    cout << "=> []" << endl;
    return;
  }
  else
  {
    cout << "=> Result of query:" << endl;
    DisplayTTY::DisplayResult( nl->First( list ), nl->Second( list ) );
  }
}

/*
10 WriteErrorList

This Function prints an errortext.

*/

void
SecondoTTY::WriteErrorList ( ListExpr list )
{
  int errorCode;
  string errorText;

  if ( !nl->IsEmpty( list ) )
  {
    list = nl->Rest( list );
    while (!nl->IsEmpty( list ))
    {
      nl->WriteListExpr( nl->First( list ) );
      errorCode = nl->IntValue( nl->First( nl->First( list ) ) );
      errorText = si->GetErrorMessage( errorCode );
      cout << "=> " << errorText << endl;
      list = nl->Rest( list );
    }
  }
}

/*
11 CallSecondo

This function gives a query to secondo and receives the result from secondo.

*/

ListExpr
SecondoTTY::CallSecondo()
{
  int errorCode = 0, errorPos, levelOffset;
  ListExpr cmdList = nl->TheEmptyList();
  ListExpr outList = nl->TheEmptyList();
  string errorMessage;
  string errorText;

  if ( currentLevel == ExecutableLevel )
  {
    levelOffset = 0;
  }
  else if ( currentLevel == DescriptiveLevel )
  {
    levelOffset = 2;
  }
  else
  {
    cout << endl << "*** Level problem in SecondoTTY::CallSecondo" << endl;;
    return (nl->TheEmptyList());
  }

  if ( cmd[0] == '(' )
  {
    if ( nl->ReadFromString( cmd, cmdList ) )
    {
      si->Secondo( cmd, cmdList, levelOffset, false, false,
                   outList, errorCode, errorPos, errorMessage );
    }
    else
    {
      cout << endl << "*** Error: list expression expected!" << endl;
    }
  }
  else
  {
    si->Secondo( cmd, cmdList, levelOffset+1, false, false,
                 outList, errorCode, errorPos, errorMessage ); 
  }
  if ( errorCode > 0 )
  {
    cout << "*** Error in Secondo command:" << endl;
    cout << SecondoInterface::GetErrorMessage( errorCode ) << endl;
    if ( errorMessage.length() > 0 )
    {
      cout << errorMessage << endl;
    }
    WriteErrorList( outList );
    cout << endl;
    nl->Destroy( outList );
    outList = nl->TheEmptyList();
  }
  if ( cmdList != nl->TheEmptyList() )
  {
    nl->Destroy( cmdList );
  }
  return (outList);
}

/*
12 Secondo2

This would normally be  the  mainfunction of SecondoTTY.

*/

void
SecondoTTY::CallSecondo2()
{
  ListExpr result;
  result = CallSecondo();
  if ( isQuery )
  {
cout << "Query-Result" << endl;
    ShowQueryResult( result );
  }
  else
  {
cout << "Type-Output" << endl;
    TypeOutputList( result );
  }
  if ( result != nl->TheEmptyList() )
  {
    nl->Destroy( result );
  }
}

/*
13 SecondoMain

This is the function where everything is done. If one wants to use the
sotrage manager provided by SHORE, one should struture its program
like this. Since using SHORE makes necessary to be on top of a thread, 
we cannot just initiate the storage manager and then keep in the same
function. Using SHORE we must use this workaround. Call the initiation 
method from the main function, and then we know that the functions
SecondoMain will be "called back"

*/

/*
1 CheckConfiguration

This function checks the Secondo configuration. First it looks for the name
of the configuration file on the command line. If no file name was given on
the command line or a file with the given name does not exist, the environment
variable SECONDO\_HOME is checked. If this variable is defined it should point
to a directory where the configuration file can be found. If the configuration
file is not found there, the current directory will be checked. If no configuration
file can be found the program terminates.

If a valid configuration file was found initialization continues.

*/

bool
SecondoTTY::CheckConfiguration()
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
      cout << "Error: Secondo home directory not specified. Terminating program." << endl;
      found = false;
    }
    else
    {
      cout << "Configuration seems to be ok." << endl << endl;
    }
  }
  else
  {
    cout << "Sorry, no configuration file found. Terminating program." << endl;
  }
  return (found);
}

/*
1 Initialize

This function initializes the Secondo system.

If the SmiEnvironment is based on the Berkeley DB implementation the process factory
and the checkpoint utility must be started.

*/

bool
SecondoTTY::Initialize()
{
  bool ok = true;
  // --- Check storage management interface
  cout << "Initializing storage management interface ... ";
  if ( SmiEnvironment::StartUp( SmiEnvironment::SingleUser, parmFile, cout ) )
  {
    cout << "completed." << endl;
  }
  else
  {
    cout << "failed." << endl;
    string errMsg;
    SmiEnvironment::GetLastErrorCode( errMsg );
    cout << "Error: " << errMsg << endl;
    ok = false;
  }
  if (ok)
  {
    cout << "Initializing the Secondo system ... ";
    nl = SecondoSystem::GetNestedList();
    ok = SecondoSystem::StartUp();
    if ( ok )
    {
      cout << "completed." << endl;
      si = new SecondoInterface();
      DisplayTTY::Initialize( si );
    }
    else
    {
      cout << "failed." << endl;
    }
  }
  return (ok);
}

void
SecondoTTY::Terminate()
{
  cout << "Terminating Secondo TTY ..." << endl;
  cout << "Terminating Secondo system ...";
//  SecondoInterface::Secondo( "close database", 0, 0, 0, 0, &ResultList,
//                             &ErrCode, &ErrPos, &ErrMess );
  delete si;
  if ( SecondoSystem::ShutDown() )
  {
    cout << "completed." << endl;
  }
  else
  {
    cout << "failed." << endl;
  }
  if ( !SmiEnvironment::ShutDown() )
  {
    string errMsg;
    SmiEnvironment::GetLastErrorCode( errMsg );
    cout << "Error: Shutdown of the storage management interface failed." << endl;
    cout << "Error: " << errMsg << endl;
  }
  cout << "SecondoTTY terminated." << endl;
}

/*
1 Execute

This function checks the configuration of the Secondo system. If the configuration
seems to be ok the system is intialized. If the initialization succeeds the user
commands are processed. If the initialization fails or the user finishes work
the system is terminated.

*/

int
SecondoTTY::Execute()
{
  int rc = 0;
  cout << endl
       << "*** Secondo TTY ***"
       << endl << endl;
  if ( CheckConfiguration() )
  {
    if ( Initialize() )
    {
      cout << endl << "Secondo TTY ready for operation." << endl
           << "Type 'HELP' to get a list of available commands." << endl;
      ProcessCommands();
    }
    Terminate();
  }
  else
  {
    rc = 1;
  }
  return (rc);
}

/*
14 main

The main function creates the Secondo TTY application and starts its execution.

*/
int
main( const int argc, const char* argv[] )
{
  SecondoTTY* appPointer = new SecondoTTY( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

