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

\def\CC{C\raise.22ex\hbox{{\footnotesize +}}\raise.22ex\hbox{\footnotesize +}\xs
pace}
\centerline{\LARGE \bf  TestRunner}

\centerline{Friedhelm Becker , Dec1997}

\begin{center}
\footnotesize
\tableofcontents
\end{center}

1 Overview

This is the test enviroment for Secondo. The code is derived from SecondoTTY.

2 Includes and defines

*/

using namespace std;

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>

#include "Application.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "SecondoSystem.h"
#include "SecondoInterface.h"
#include "SecondoSMI.h"
#include "NestedList.h"
#include "DisplayTTY.h"
#include "CharTransform.h"

static const bool needIdent = false;

class TestRunner : public Application
{
 public:
  TestRunner( const int argc, const char** argv );
  virtual ~TestRunner() {};
  bool AbortOnSignal( int sig );
  void ProcessFile( const string& fileName );
  void ProcessCommand();
  void ProcessCommands();
  bool CheckConfiguration();
  int  Execute();
  void ShowPrompt( const bool first );
  void TypeOutputList ( ListExpr list );
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

  /* the following variables and constants are 
     needed for maintaining the test state */
  int state;
  string testName;
  string testCaseName;
  string yields;
  bool yieldsError;
  bool skipToTearDown;
  int testCaseNumber;
  int nErrors;

  static const int START = 0;
  static const int SETUP = 1;
  static const int TESTCASE = 2;
  static const int TEARDOWN = 3;

};

TestRunner::TestRunner( const int argc, const char** argv )
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
  currentLevel  = ExecutableLevel;
  si            = 0;

  state = START;
  skipToTearDown = false;
  testCaseNumber = 0;
  nErrors = 0;
}

bool
TestRunner::AbortOnSignal( int sig )
{
  return (true);
}

void
TestRunner::ProcessFile( const string& fileName )
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
TestRunner::ProcessCommand()
{
  string cmdWord;
  istringstream is( cmd );
  is >> cmdWord;
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), ToUpperProperFunction );

  if ( cmdWord == "D" || cmdWord == "DESCRIPTIVE" )
  {
    currentLevel = DescriptiveLevel;
  }
  else if ( cmdWord == "E" || cmdWord == "EXECUTABLE" )
  {
    currentLevel = ExecutableLevel;
  }
  else if ( cmdWord == "H" || cmdWord == "HYBRID" )
  {
    currentLevel = HybridLevel;
  }
  else if ( cmdWord == "Q" || cmdWord == "QUIT" )
  {
    cout << "*** Thank you for using SECONDO!" << endl;
    quit = true;
  }
  else if ( cmdWord == "DEBUG" )
  {
    int debugLevel;
    is >> debugLevel;
    si->SetDebugLevel( debugLevel );
    cout << "*** Debug level set to " << debugLevel << "." << endl;
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
TestRunner::ShowPrompt( const bool first )
{
}

bool
TestRunner::IsInternalCommand( const string& line )
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
TestRunner::GetCommand()
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
      else
      {
        // processing directive
        if(line.size() < 2)
        {
          /* assume that this line is part of a real comment */
        }
        else
        {
          while(line[line.size() - 1] == '\\')
          {
            line.erase(line.size() - 1);
            string nextline;
            getline(cin, nextline);
            line += nextline;
          }

          size_t pos = 1;
          char current = line[pos];
          string command;
          string restOfLine;

          while(isspace(current) && pos < line.size() - 1)
          {
            pos++;
            current = line[pos];
          }

          command += current;
          while(isalpha(current) && pos < line.size() - 1)
          {
            pos++;
            current = line[pos];
            command += current;
          }
          if(pos == line.size() - 1)
          {
            pos++;
          }

          restOfLine = line.substr(pos);
          while(restOfLine.size() > 0 && isspace(restOfLine[0]))
          {
            restOfLine.erase(0, 1);
          }

          if(command.find("setup") == 0)
          {
            if(state != START)
            {
              cout
                << "Setup directive must appear just once in a test file."
                << endl;
              exit(1);
            }
            state = SETUP;
            testName = restOfLine;
            cout << "Starting test " << restOfLine << endl;
          }
          else if(command.find("testcase") == 0)
          {
            state = TESTCASE;
            testCaseName = restOfLine;
            testCaseNumber++;
          }
          else if(command.find("yields") == 0)
          {
            assert(state == TESTCASE);
            yields = restOfLine;
            yieldsError = yields.find("error") != string::npos;
          }
          else if(command.find("teardown") == 0)
          {
            if(state == TEARDOWN)
            {
              cout
                << "Teardown directive must not appear more than " 
                << "once in a test file." << endl;
            }

            cout << "Starting teardown." << endl;
            state = TEARDOWN;
          }
          else
          {
            /* current line is comment */
          }
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
TestRunner::ProcessCommands()
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
10 WriteErrorList

This Function prints an errortext.

*/

void
TestRunner::WriteErrorList ( ListExpr list )
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
TestRunner::CallSecondo()
{
  int errorCode = 0, errorPos, levelOffset;
  ListExpr cmdList = nl->TheEmptyList();
  ListExpr outList = nl->TheEmptyList();
  string errorMessage;
  string errorText;
  ListExpr expectedResult;

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
    cout << endl << "*** Level problem in TestRunner::CallSecondo" << endl;;
    return (nl->TheEmptyList());
  }

  if(!skipToTearDown)
  {
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
  }

  switch(state)
  {
    case START:
      if(errorCode != 0)
      {
        /* should we report errors in the intial section?
        cout
          << "Encountered error in initial section." << endl;
        cout << "    error code : " << errorCode << endl;
        cout << "    " << SecondoInterface::GetErrorMessage( errorCode ) << endl;
        if ( errorMessage.length() > 0 )
        {
          cout << "    " << errorMessage << endl;
        }
        WriteErrorList(outList);
        cout << "while processing command " << endl << cmd << endl;
        cout << endl; */
      }
      break;
    case SETUP:
      if(errorCode != 0)
      {
        cout
          << "*** Encountered error during setup, skipping to teardown. ***"
          << endl;
        cout << "    error code : " << errorCode << endl;
        cout << "    " << SecondoInterface::GetErrorMessage( errorCode ) << endl;
        if ( errorMessage.length() > 0 )
        {
          cout << "    " << errorMessage << endl;
        }
        WriteErrorList(outList);
        cout << "while processing command " << endl << cmd << endl;
        cout << endl;
        skipToTearDown = true;
      }
      break;
    case TESTCASE:
      if(!skipToTearDown)
      {
        if(errorCode == 0 && yieldsError)
        {
          nErrors++;
          cout
            << "*** ERROR *** : Testcase "
            << testCaseNumber << " : " << testCaseName
            << " suceeded, but should not do so." << endl;
          cout
            << "Expected error but got result : ";
          nl->WriteListExpr(outList, cout);
          cout << endl << "command was : " << endl << cmd << endl;
          cout << endl;
        }
        else if(errorCode > 0 && yieldsError)
        {
          cout
            << "*** OK *** Testcase " << testCaseNumber << " : " << testCaseName <<
            " returned an error as expected." << endl;
        }
        else if(errorCode == 0 && !yieldsError)
        {
          if(yields.find("success") == string::npos)
          {
            /* verify that the expected results were delivered */
            nl->ReadFromString(yields, expectedResult);
            if(nl->Equal(outList, expectedResult))
            {
              cout
                << "*** OK *** Testcase " << testCaseNumber << " : " << testCaseName
                << " suceeded as expected." << endl;
            }
            else
            {
              nErrors++;
              cout << "*** ERROR *** Testcase "
                << testCaseNumber << " : " << testCaseName
                << " returned unexpected results." << endl;
              cout << "Expected : ";
              nl->WriteListExpr(expectedResult, cout);
              cout << endl << "But got : ";
              nl->WriteListExpr(outList, cout);
              cout << endl << "command was : " << endl << cmd << endl;
              cout << endl;
            }
          }
          else
          {
            cout
              << "*** OK *** Testcase " << testCaseNumber << " : "
              << testCaseName << " suceeded as expected." << endl;
          }
        }
        else if(errorCode > 0 && !yieldsError)
        {
          nErrors++;
          cout
            << "*** ERROR *** : Testcase "
            << testCaseNumber << " : " << testCaseName
            << " returned an error, but should not do so." << endl;
          cout
            << "Expected result : " << endl
            << yields << endl
            << "but got error : " << endl;
          cout << "    error code : " << errorCode << endl;
          cout << "    " << SecondoInterface::GetErrorMessage( errorCode ) << endl;
          if ( errorMessage.length() > 0 )
          {
            cout << "    " << errorMessage << endl;
          }
          WriteErrorList(outList);
          cout << "command was : " << endl << cmd << endl;
          cout << endl;
        }
      }
      break;
    case TEARDOWN:
      if(errorCode > 0)
      {
        cout
          << "*** Encountered error during teardown. ***" << endl;
          cout << "    error code : " << errorCode << endl;
          cout << "    " << SecondoInterface::GetErrorMessage( errorCode ) << endl;
          if ( errorMessage.length() > 0 )
          {
            cout << "    " << errorMessage << endl;
          }
          WriteErrorList(outList);
          cout << "command was : " << endl << cmd << endl;
          cout << endl;
      }

      break;
  }

  if ( errorCode > 0 )
  {
    nl->Destroy( outList );
    outList = nl->TheEmptyList();
  }
  if ( cmdList != nl->TheEmptyList() )
  {
    nl->Destroy( cmdList );
  }
  return (outList);
}

void
TestRunner::CallSecondo2()
{
  ListExpr result;
  result = CallSecondo();
  nl->initializeListMemory();
}

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
TestRunner::CheckConfiguration()
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
      cout << "Usage: TestRunner [options] <TestFile" << endl << endl
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
    else if ( argOk && argSwitch == "-i" )
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
TestRunner::Execute()
{
  int rc = 0;
  cout << endl
       << "--- Secondo TestRunner ---"
       << endl << endl;
  if ( CheckConfiguration() )
  {
    streambuf* oldInputBuffer  = 0;
    streambuf* oldOutputBuffer = 0;
    ifstream fileInput;
    ofstream fileOutput;
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
    if(nErrors == 0)
    {
      cout << "There were *** no *** errors." << endl;
    }
    else if(nErrors == 1)
    {
      cout << "There was *** 1 *** error." << endl;
    }
    else
    {
      cout << "There were *** " << nErrors << " *** errors." << endl;
    };

    si->Terminate();
    delete si;
    cout << "Secondo TestRunner terminated." << endl;
  }
  else
  {
    return -1;
  }
  return (nErrors);
}

/*
14 main

The main function creates the TestRunner and starts its execution.

*/
int
main( const int argc, const char* argv[] )
{
  int devNullFd = open("/dev/null", O_RDONLY);
  dup2(devNullFd, 2);

  TestRunner* appPointer = new TestRunner( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

