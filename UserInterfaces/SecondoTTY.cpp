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
#include <ctype.h>

#include "Application.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "SecondoSystem.h"
#include "SecondoInterface.h"
#include "SecondoSMI.h"
#include "NestedList.h"
#include "DisplayTTY.h"
#include "CharTransform.h"
#include "LogMsg.h"

static const bool needIdent = false;

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
  int  Execute();
  void ShowPrompt( const bool first );
  void TypeOutputList ( ListExpr list );
  void TypeOutputListFormatted ( ListExpr list );
  bool IsInternalCommand( const string& line );
  bool GetCommand();
  void ShowQueryResult( ListExpr list );
  void WriteErrorList ( ListExpr list );
  ListExpr CallSecondo();
  void CallSecondo2();
 private:
  string            parmFile;
  string            user;
  string            pswd;
  string            host;
  string            port;
  string            iFileName;
  string            oFileName;
  string            cmd;
  bool              isStdInput;
  bool              quit;
  NestedList*       nl;
  AlgebraLevel      currentLevel;
  bool              isQuery;
  SecondoInterface* si;
};

SecondoTTY::SecondoTTY( const int argc, const char** argv )
  : Application( argc, argv )
{
  parmFile      = "";
  user          = "";
  pswd          = "";
  host          = "";
  port          = "";
  iFileName     = "";
  oFileName     = "";
  string cmd    = "";
  isStdInput    = true;
  quit          = false;
  nl            = 0;
  currentLevel  = DescriptiveLevel;
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
       << "  DEBUG {0|1|2}   - set debug level:" << endl
       << "                      0   - debug mode turned off" << endl
       << "                      1   - debug mode turned on" << endl
       << "                      2   - debug and trace mode turned on" << endl
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
  bool saveIsStdInput = isStdInput;
  streambuf* oldBuffer;
  ifstream fileInput( fileName.c_str() );
  if ( fileInput )
  {
    oldBuffer = cin.rdbuf( fileInput.rdbuf() );
    cout << "*** Begin processing file '" << fileName << "'." << endl;
    isStdInput = false;
    ProcessCommands();
    isStdInput = saveIsStdInput;
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
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), ToUpperProperFunction );

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
    transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), ToUpperProperFunction );
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
  else if ( cmdWord == "DEBUG" )
  {
    int debugLevel;
    is >> debugLevel;
    si->SetDebugLevel( debugLevel );
    cout << "*** Debug level set to " << debugLevel << "." << endl;
  }
  else if ( cmdWord == "Q" || cmdWord == "QUIT" )
  {
    cout << "*** Thank you for using SECONDO!" << endl;
    quit = true;
  }
  else if ( cmdWord[0] == '@' )
  {
    ProcessFile( cmd.substr( 1, ( cmd.length() - 2 ) ) ); // Delete blank char at the end
  }
  else
  {
    isQuery = (cmdWord == "QUERY" || cmdWord == "(QUERY" || cmdWord == "( QUERY");
    if ( currentLevel == HybridLevel )
    {
      if ( cmdWord == "QUERY"  || cmdWord == "(QUERY"  || cmdWord == "( QUERY" ||
           cmdWord == "UPDATE" || cmdWord == "(UPDATE" || cmdWord == "( UPDATE" )
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
    string level;
    switch (currentLevel)
    {
      case DescriptiveLevel: level = "(D)"; break;
      case ExecutableLevel:  level = "(E)"; break;
      case HybridLevel:      level = "(H)"; break;
      default:               level = "(?)"; break;
    }
    if ( first )
    {
      cout << level << " Secondo => ";  // First line of command
    }
    else
    {
      cout << level << " Secondo -> ";  // Continuation line of command
    }
  }
}

bool
SecondoTTY::IsInternalCommand( const string& line )
{
  string cmdWord;
  istringstream is( line );
  is >> cmdWord;
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), ToUpperProperFunction );

  return ( cmdWord == "?" || cmdWord == "HELP"        ||
           cmdWord == "D" || cmdWord == "DESCRIPTIVE" ||
           cmdWord == "E" || cmdWord == "EXECUTABLE"  ||
           cmdWord == "H" || cmdWord == "HYBRID"      ||
           cmdWord == "Q" || cmdWord == "QUIT"        ||
           cmdWord == "DEBUG" || cmdWord == "SHOW" || cmdWord[0] == '@' );
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
          cmd = line + " ";
          first = false;
        }
        else
        {
          cmd = cmd + line + " ";
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
    nl->WriteListExpr( list, cout );
    cout << endl;
  }
}

/*
8 TypeOutputListFormatted

TypeOutputList prints the result of a nonquery input (e. g. list) in formatted manner.

*/

void
SecondoTTY::TypeOutputListFormatted ( ListExpr list )
{
  if ( nl->IsEmpty( list ) )
  {
    cout << "=> []" << endl;
  }
  else
  {
    cout << "=> Result:" << endl;
    DisplayTTY::DisplayResult2( list );
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
  if ( nl->IsEmpty( list ) || 
       (nl->ListLength( list ) != 2) ||
       ( (nl->ListLength( list ) == 2) && (nl->IsEmpty(nl->Second( list ))) ) 
     )
  {
    cout << "=> []" << endl;
    return;
  }
  else
  {
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
      nl->WriteListExpr( nl->First( list ), cout );
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
    cout << "*** Error in Secondo command: " << errorCode << endl;
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

#ifdef NL_DEBUG
  cerr << endl << "### ResultStr: " << nl->ToString(result) << endl;
#endif
	  
  if ( isQuery )
  {
    ShowQueryResult( result );
  }
  else
  {
    if (!nl->IsEmpty( result ))
    {
      if ( nl->IsAtom(nl->First(result)) && 
           nl->AtomType(nl->First(result)) == SymbolType &&
           nl->SymbolValue(nl->First(result)) == "formatted" ) 
      {
        TypeOutputListFormatted( nl->Second( result ));
	//TypeOutputList( result );
      }
      else
      {
        TypeOutputList( result );
      }

    }
    else
    {
      TypeOutputList( result );
    }
  }
  nl->initializeListMemory();
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
  bool ok = true;
  int i = 1;
  string argSwitch, argValue;
  bool argOk;
  while (i < GetArgCount())
  {
    argSwitch = GetArgValues()[i];
    if ( i < GetArgCount()-1)
    {
      argValue  = GetArgValues()[i+1];
      argOk = (argValue[0] != '-');
    }
    else
    {
      argValue = "";
      argOk = false;
    }
    if ( argSwitch == "-?" || argSwitch == "--help" )  // Help
    {
      cout << "Usage: SecondoTTY{BDB|ORA|CS} [options]" << endl << endl
           << "Options:                                             (Environment)" << endl
           << "  -c config  : Secondo configuration file            (SECONDO_CONFIG)" << endl
           << "  -i input   : Name of input file  (default: stdin)" << endl
           << "  -o output  : Name of output file (default: stdout)" << endl
           << "  -u user    : User id                               (SECONDO_USER)" << endl
           << "  -s pswd    : Password                              (SECONDO_PSWD)" << endl
           << "  -h host    : Host address of Secondo server        (SECONDO_HOST)" << endl
           << "  -p port    : Port of Secondo server                (SECONDO_PORT)" << endl << endl
           << "Command line options overrule environment variables." << endl;
      ok = false;
      break;
    }
    else if ( argOk && argSwitch == "-c" )  // Configuration file
    {
      parmFile = argValue;
    }
    else if ( argOk && argSwitch == "-i" )  // Input file
    {
      iFileName = argValue;
    }
    else if ( argOk && argSwitch == "-o" )  // Output file
    {
      oFileName = argValue;
    }
    else if ( argOk && argSwitch == "-u" )  // User id
    {
      user = argValue;
    }
    else if ( argOk && argSwitch == "-s" )  // Password
    {
      pswd = argValue;
    }
    else if ( argOk && argSwitch == "-h" )  // Host
    {
      host = argValue;
    }
    else if ( argOk && argSwitch == "-p" )  // Port
    {
      port = argValue;
    }
    else
    {
      cout << "Error: Invalid option: '" << argSwitch << "'." << endl;
      if ( argOk )
      {
        cout << "  having option value: '" << argValue << "'." << endl;
      }
      cout << "Use option -? or --help to get information about available options." << endl;
      ok = false;
    }
    i++;
    if ( argOk )
    {
      i++;
    }
  }
  char* envValue;
  if ( parmFile.length() == 0 )
  {
    envValue = getenv( "SECONDO_CONFIG" );
    if ( envValue != 0 )
    {
      parmFile = envValue;
    }
  }
  if ( user.length() == 0 )
  {
    envValue = getenv( "SECONDO_USER" );
    if ( envValue != 0 )
    {
      user = envValue;
    }
  }
  if ( pswd.length() == 0 )
  {
    envValue = getenv( "SECONDO_PSWD" );
    if ( envValue != 0 )
    {
      pswd = envValue;
    }
  }
  if ( host.length() == 0 )
  {
    envValue = getenv( "SECONDO_HOST" );
    if ( envValue != 0 )
    {
      host = envValue;
    }
  }
  if ( port.length() == 0 )
  {
    envValue = getenv( "SECONDO_PORT" );
    if ( envValue != 0 )
    {
      port = envValue;
    }
  }
  if ( needIdent ) // Is user identification needed?
  {
    int count = 0;
    while (count <= 3 && user.length() == 0)
    {
      count++;
      cout << "Enter user id: ";
      getline( cin, user );
    }
    ok = user.length() > 0;
    if ( !ok )
    {
      cout << "Error: No user id specified." << endl;
    }
    if ( ok && pswd.length() == 0 )
    {
      count = 0;
      while (count <= 3 && user.length() == 0)
      {
        count++;
        cout << "Enter password: ";
        getline( cin, pswd );
      }
      if ( pswd.length() == 0 )
      {
        cout << "Error: No password specified." << endl;
        ok = false;
      }
    }
  }
  else
  {
    user = "SECONDO";
    pswd = "SECONDO";
  }
  if ( ok )
  {
    // config file or (host and port) must be specified
    ok = parmFile.length() > 0 || (host.length() > 0 && port.length() > 0);
    if ( !ok )
    {
      cout << "Error: Neither config file nor host and port of Secondo server specified." << endl;
      cout << "Use option -? or --help to get information about available options." << endl;
    }
  }
  return (ok);
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
    streambuf* oldInputBuffer  = 0;
    streambuf* oldOutputBuffer = 0;
    ifstream fileInput;
    ofstream fileOutput;
    
    //set AlgebraLevel and LogMsg prefixes
    string algLevelStr = SmiProfile::GetParameter( "Environment", "AlgebraLevel", "Descriptive", parmFile );
    string logMsgList = SmiProfile::GetParameter( "Environment", "RTFlags", "", parmFile );
    
    RTFlag::initByString(logMsgList);
    
    char chLevel = toupper( (algLevelStr.data())[0] );
    switch (chLevel) {
       case 'E': currentLevel = ExecutableLevel; break;
       case 'D': currentLevel = DescriptiveLevel; break;
       case 'H': currentLevel = HybridLevel; break;
       default:  currentLevel = DescriptiveLevel; 
    }
    

 
    si = new SecondoInterface();
    if ( si->Initialize( user, pswd, host, port, parmFile ) )
    {
      if ( iFileName.length() > 0 )
      {
        fileInput.open( iFileName.c_str() );
        if ( fileInput.is_open() )
        {
          oldInputBuffer = cin.rdbuf( fileInput.rdbuf() );
          isStdInput = false;
        }
      }
      if ( oFileName.length() > 0 )
      {
        fileOutput.open( oFileName.c_str() );
        if ( fileOutput.is_open() )
        {
          oldOutputBuffer = cout.rdbuf( fileOutput.rdbuf() );
        }
      }
      nl = si->GetNestedList();
      DisplayTTY::Initialize( si );
      if ( isStdInput )
      {
        cout << endl << "Secondo TTY ready for operation." << endl
             << "Type 'HELP' to get a list of available commands." << endl;
      }
      ProcessCommands();
      if ( iFileName.length() > 0 )
      {
        if ( fileInput.is_open() )
        {
          cin.rdbuf( oldInputBuffer );
        }
      }
      if ( oFileName.length() > 0 )
      {
        if ( fileOutput.is_open() )
        {
          cout.rdbuf( oldOutputBuffer );
        }
      }
    }
    si->Terminate();
    delete si;
    cout << "SecondoTTY terminated." << endl;
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

