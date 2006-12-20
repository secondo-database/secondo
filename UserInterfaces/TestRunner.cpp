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


August 2005, M. Spiekermann. The semantics of reading input files was changed.
Now commands can end with spaces and also a "blank" or a newline will indicate
the end of a command. Moreover, reporting of the error and success messages has
been changed and was implemented in special functions.  Finally, unused
functions and variables were removed!.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. 

February 2006, M. Spiekermann. Reimplementation of the ~GetCommand()~ method.
Parsing the input lines is now done with less code by using some useful
functions operating on ~strings~ implemented in "CharTransform.h". Moreover
some bugs concerning result evaluation have been fixed. Finally, some new
features (Approximate comparison of float values, result specification in
external files and envrionment variable expansion in file names) were
implemented.

Nov 2006, M. Spiekermann. Results may now be also specified as database
objects. Moreover, a new directive coverage was introduced which reports an
error if not all operators of an specified algebra module were used inside teh
tests.

1 Overview

This is the test enviroment for Secondo. The code is derived from SecondoTTY.

2 Includes and defines

*/


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <utility>

#include <unistd.h>
#include <fcntl.h>

#include "Application.h"
#include "Profiles.h"
#include "FileSystem.h"

#include "SecondoSystem.h"
#include "SecondoInterface.h"
#include "SecondoSMI.h"
#include "SecParser.h"

#include "NestedList.h"
#include "DisplayTTY.h"
#include "CharTransform.h"
#include "LogMsg.h"
#include "ExampleReader.h"

using namespace std;

class TestRunner : public Application
{
 public:
  TestRunner( const TTYParameter& tp );
  virtual ~TestRunner() { };
  int  Execute();

 private:

  void ProcessFile( const string& fileName );
  void ProcessCommand();
  void ProcessCommands();
  void ProcessExamples();

  bool RunCmd(const string& dbName, SecErrInfo& err);

  ListExpr CallSecondo();
  void CallSecondo2();
  void RegisterError();
 
  ListExpr MakeConstant(const string& type, ListExpr value); 

  void CoverageQuery(const string& algebra); 

  // read only functions 
  bool AbortOnSignal( int sig ) const;
  bool IsInternalCommand( const string& line ) const;
  bool GetCommand();

  void ShowErrCodeInfo( const int errorCode, 
                        const string& errorMessage, 
                        const ListExpr outList      ) const;

  void ShowCommand( const string& cmd) const;
  void ShowTestTitle() const;
  void ShowTestErrorMsg() const;
  void ShowErrorSummary() const;
  void ShowTestSuccessMsg(const string& msg) const;

  void VerifyResult(ListExpr outList, ListExpr expectedResult); 

  void DisplayError( const string& cmd, ListExpr expectedResult, 
                     int errorCode, const string& errorMessage, 
                     ListExpr outList ); 

  string            parmFile;
  string            user;
  string            pswd;
  string            host;
  string            port;
  string            iFileName;
  string            oFileName;
  string            cmd;
  string            missingOps;
  string            rightArrow, leftArrow;
  int               num;
  bool              isStdInput;
  bool              runExamples;
  bool              quit;
  bool              verbose;
  NestedList*       nl;
  bool              isQuery;
  SecondoInterface* si;

  typedef list< pair<int,int> > ErrorInfo; 
  ErrorInfo errorLines;


/* 
the following variables and constants are 
needed for maintaining the test state 

*/
  
  Tolerance realValTolerance;
  ListExpr expectedResult;
  
  string testName;
  string testCaseName;
 
  typedef enum { Success, Error, Result, 
                 Unknown, UndefinedObj, Coverage} YieldState; 
  YieldState yieldState;
  bool skipToTearDown;
  
  int testCaseNumber;
  int testCaseLine;
  int numErrors;

  int state;
  static const int START = 0;
  static const int SETUP = 1;
  static const int TESTCASE = 2;
  static const int TEARDOWN = 3;

};



TestRunner::TestRunner( const TTYParameter& tp )
  : Application( tp.numArgs, (const char**)tp.argValues )
{
  parmFile      = tp.parmFile;
  user          = tp.user;
  pswd          = tp.pswd;
  host          = tp.host;
  port          = tp.port;
  iFileName     = tp.iFileName;
  oFileName     = tp.oFileName;
  num           = parse<int>(tp.num);
  runExamples   = tp.runExamples;

  string cmd    = "";
  missingOps    = "";
  isStdInput    = true;
  quit          = false;
  nl            = 0;
  si            = 0;

  state = START;
  
  skipToTearDown = false;
  yieldState = Success;
  
  testCaseNumber = 0;
  testCaseLine = 0;
  numErrors = 0;

  expectedResult = nl->TheEmptyList();
  
  verbose = false;

  const int n = 30;
  const string ra(n,'>'); 
  const string la(n,'<'); 
  stringstream tmp;
  tmp << color(red) << ra << color(normal);
  rightArrow = tmp.str();
  tmp.str("");
  tmp << color(red) << la << color(normal);
  leftArrow = tmp.str();

}

bool
TestRunner::AbortOnSignal( int sig ) const
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
TestRunner::ShowTestTitle() const
{
  cout
    << endl << endl << color(green)
    << "*** Test " << testCaseNumber 
    << " (line " << testCaseLine << "): "
    << testCaseName << " ***"
    << color(normal) << endl;
}



void
TestRunner::ShowTestSuccessMsg(const string& msg) const
{
  cout
    << color(green)
    << "==> [OK] " << msg << color(normal) << endl;
}


void
TestRunner::ShowTestErrorMsg() const
{
  cout
    << endl << color(red) 
    << "==> [ERROR]" << color(normal) << endl;
}


void
TestRunner::ShowErrCodeInfo(  const int errorCode, 
                              const string& errorMessage, 
                              const ListExpr outList      ) const
{
   cout 
     << "Error-Code: " <<  errorCode << endl
     << "Error-Text: \"" << SecondoInterface::GetErrorMessage( errorCode ) 
     << "\""
     << endl;

   if ( errorMessage.length() > 0 ) {
   cout 
     << "Error-Msgs: \"" << errorMessage << "\"" << endl;
   }

   if ( !nl->IsEmpty(outList) ) 
   {
     cout << " Error-List: " << endl;
     si->WriteErrorList(outList);
   }
}


void
TestRunner::ShowCommand( const string& cmd) const
{
  cout 
    << endl
    << "Secondo-Cmd: " << endl 
    << "  " << cmd << endl;
}


void 
TestRunner::ShowErrorSummary() const 
{
  cout 
    << endl 
    << "TEST SUMMARY :" << endl
    << "--------------" << endl;

  if(numErrors == 0)
  {
    cout << "There were *** no *** errors." << endl;
  }
  else if(numErrors == 1)
  {
    cout << "There was *** 1 *** error." << endl;
  }
  else if (numErrors > 1)
  {
    cout << "There were *** " << numErrors << " *** errors." << endl;
  }
  else 
  {
    cout << "There were Errors outside of the test cases!" << endl;
  }

  if ( numErrors > 0 ) {

    cout << endl << "<testno>@<lineno> which caused an error: ";
    ErrorInfo::const_iterator it = errorLines.begin();
    while ( it != errorLines.end() )  
    {
      cout << " " << it->first << "@" << it->second;
      it++;
    }
    cout << endl << endl;
  }

  cout << endl
       << "COVERAGE (untested operators):" << endl
       << "------------------------------" << endl
       << missingOps << endl;

}

void
TestRunner::RegisterError()
{
  numErrors++;
  errorLines.push_back( make_pair(testCaseNumber, testCaseLine) );
  ShowTestErrorMsg();
}


void
TestRunner::CoverageQuery(const string& algebra) 
{
  cout << "Computing coverage for algebra " << algebra << endl;
  cmd = "query SEC2OPERATORUSAGE feed filter[.Algebra = \"" 
        + algebra 
        + "\"] filter[.Calls = 0] project[Operator] consume"; 
  yieldState = Coverage;
  string resText = "((rel(tuple((Operator string))))())";
  bool ok = nl->ReadFromString(resText, expectedResult);
  assert(ok);       
}


void
TestRunner::ProcessCommand()
{
  string cmdWord = "";
  istringstream is( cmd );
  is >> cmdWord;
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), 
             ToUpperProperFunction );

  if ( cmdWord == "Q" || cmdWord == "QUIT" )
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
    isQuery = ( cmdWord == "QUERY" 
                || cmdWord == "(QUERY" 
                || cmdWord == "( QUERY" );
    CallSecondo2();
  }
}


bool
TestRunner::IsInternalCommand( const string& line ) const
{
  string cmdWord = "";
  istringstream is( line );
  is >> cmdWord;
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(), 
             ToUpperProperFunction );

  return ( cmdWord == "?" || cmdWord == "HELP"        ||
           cmdWord == "Q" || cmdWord == "QUIT"        ||
           cmdWord == "DEBUG" || cmdWord == "SHOW" || cmdWord[0] == '@' );
}

bool
TestRunner::GetCommand()
{
  static int lineCtr = 0;
  bool complete = false;
  bool first = true;
  string line = "";
  cmd = "";

  // clear last expected result
  expectedResult = nl->Empty();
  
  while (!complete && !cin.eof() && !cin.fail())
  {
    line = "";
    getline( cin, line );
    lineCtr++;
    if ( line.length() > 0 )
    {
      if ( line[0] != '#' )    // # no comment => SECONDO command 
      {
        if ( line[line.length()-1] == ';' ) // check for command end
        {
          complete = true;
          line.erase( line.length()-1 );
        }
        if ( first ) // check for an internal command      
        {
          if ( !complete ) // check only if no command end was found
          {
            complete = IsInternalCommand( line );
          }
          cmd = line + " ";
          first = false;  // don't check again
        }
        else // concat command parts
        {
          // check if line contains only spaces
          if ( isSpaceStr(line) ) {
            complete = true; // command ends
          } else {
            cmd = cmd + line + " "; // concat command parts
          }
        }
      }
      else
      {
        // process a test runner directive
        if(line.size() < 2)
        {
          /* assume that this line is part of a real comment */
        }
        else // check for directives
        {
          // concat lines with continuation symbol "\"
          static const char contSym = '\\';
          size_t endPos = line.size() - 1;
          while( line[endPos] == contSym ) 
          {
            string nextLine;
            getline(cin, nextLine);
            
            /*
            cout << "Concating lines:" << endl;   
            cout << "1st: <" << line << ">" << endl
                 << "2nd: <" << nextLine << ">" << endl;
            */   
                 
            line.erase(endPos);
            line += "\n" + nextLine;
            endPos = line.size() - 1;
          }
          //cout << "Concat result:" << line << endl;     

          string command = parse<string>(line.substr(1));
          size_t pos = line.find(command) + command.size();
          string restOfLine = trim( line.substr(pos) );

          //cout << "command: " << command << endl;
          //cout << "rest: <" << restOfLine << ">" << endl;
          
          // check for TestRunner directives
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
          else if(command.find("description") == 0) {
            cout << "description: " << endl << restOfLine << endl;
          }
          else if(command.find("stop") == 0) {
            if (state == START) {
              cout << endl 
                   << "Stop directive found! Test will not be executed." 
                   << endl
                   << "Reason: " << restOfLine << endl << endl;
              exit(0);
            } else {
              cout << "Warning: stop directives after setup are ignored!" 
                   << endl;
            }
          }
          else if(command.find("testcase") == 0)
          {
            state = TESTCASE;
            testCaseName = restOfLine;
            testCaseNumber++;
            testCaseLine = lineCtr;
          }
          else if(command.find("tolerance_real") == 0)
          {
            assert(state == TESTCASE);
            string toleranceStr = parse<string>( restOfLine );
            int offset=0;
            string suffix("");
            if (toleranceStr[0] == '%') 
            {
              realValTolerance.isRelative = true;
              cout << "Relative";
              offset=1;
              suffix="%";
            }              
            else
            {
              realValTolerance.isRelative = false;
              cout << "Absolute";
            }               
            realValTolerance.value = 
                        parse<double>( toleranceStr.substr(offset) );
            cout << " tolerance set to " 
                 << realValTolerance.value << suffix << endl;
          }
          else if(command.find("yields") == 0)
          {
            assert(state == TESTCASE);
            size_t pos = firstNonSpace(restOfLine);
            if (pos == string::npos)
            {               
              yieldState = Unknown;
            }
            else
            {       
              char first=restOfLine[pos];           
              if ( (first=='@') ||  (first=='(') || (first=='*') ) 
              {
                // result specified
                yieldState = Result;

                if (first=='@')
                {             
                  // result stored in a separate file        
                  string resultFileStr = parse<string>( restOfLine.substr(1) );
                  cout << "Query result specified in file '" << resultFileStr 
                       << "'" << endl;
                  nl->ReadFromFile( expandVar(resultFileStr), expectedResult );
                }       
                else if (first=='(')
                {
                  nl->ReadFromString(restOfLine, expectedResult);
                }
                else
                {
                  string ident = parse<string>( restOfLine.substr(1) );
                  cout << "Query result specified in object '" << ident 
                       << "'" << endl;
                  ListExpr resList = nl->Empty();
                  SecErrInfo err;
                  bool ok = si->Secondo( "query " + ident, resList, err );

                  if (!ok) {
                    expectedResult = nl->Empty();
                    yieldState = UndefinedObj;
                  } 
                  else
                  {
                    expectedResult = resList;
                  }
                }             
              }
              else if ( restOfLine.find("error") != string::npos )
              {
                yieldState = Error;
              }       
              else if ( restOfLine.find("success") != string::npos )
              {               
                yieldState = Success;
              } 
              else
              {
                yieldState = Unknown;
              }       
            }       
          }
          else if(command.find("coverage") == 0)
          {
            yieldState = Coverage;
            CoverageQuery(restOfLine); 
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
    else  // Empty line ends command
    {
      // check if cmd contains information, otherwise continue
      complete = (cmd.length() > 0); 
      first = true;
    }
  }
  return (complete);
}

void
TestRunner::ProcessCommands()
{
  if (num > 0) {
     cout << "Running only test case number " << num << "!" << endl;
  }
   
  while (!cin.eof() && !quit)
  {
    if ( GetCommand() )
    {
      // if a number is specified only run a specific test case
      if (num > 0) {        
        if ( state == TESTCASE ) {
          if (testCaseNumber == num)
            ProcessCommand();
        } else {
          ProcessCommand();
        }  
      }     
      else // run every command 
      {
        ProcessCommand();
      }  
    }
  }
}

void
TestRunner::ProcessExamples()
{
 
  cout << "Processing examples for " << iFileName << "!" << endl;
 
  const char* envVar = "SECONDO_BUILD_DIR";
  char* buildDir = getenv(envVar);
  if (buildDir == 0) {
    cerr << "Variable " << envVar << " not defined!" << endl;
    exit(1);
  }

  // parse example file
  CFile in(iFileName);
  string algebraShort = in.getName();
  removeSuffix(".examples", algebraShort); 
  string algebra = algebraShort + "Algebra";

  ExampleReader examples(iFileName);
  bool parseOk = examples.parse();
  if (!parseOk) {
    numErrors++;
    return;
  } 

  bool needsRestore = examples.getRestoreFlag();
  bool rc = false;
  SecErrInfo err;

  string dbName = examples.getDB();
  string dbFile = dbName;

  if (!needsRestore) {

    cout << "Opening database  " << dbName << "!" << endl;
    rc = RunCmd("open database " + dbName, err);
    if (!rc) 
    {
       cout << "Command failed with msg: " << err.msg << endl;
       cout << "Trying to restore ..." << endl;
       needsRestore = true;
    } 
  }


  if (needsRestore) {

    if (err.code == ERR_IDENT_UNKNOWN_DB_NAME)
      rc = RunCmd("create database " + dbName, err);
    if (!rc) {
      numErrors++;
      cout << "Restore failed with msg: " << err.msg << endl;
      cout << "Giving up!" << endl;
      return;
    } 


    rc = RunCmd("restore database " + dbName + " from " + dbFile, err);
    if (!rc) {
      numErrors++;
      cout << "Restore failed with msg: " << err.msg << endl;
      cout << "Giving up!" << endl;
      return;
    } 
  }

  state = TESTCASE;
  yieldState = Result;

  // iterate over all examples
  ExampleInfo info;
  examples.initScan();
  while ( examples.next(info) )
  {
     cmd = info.example;
     expectedResult = nl->Empty();
     ListExpr tmpList = nl->Empty();
     bool resultOk = false;

     if (info.result != "") {
     if (  (info.result[0] != '(') ) 
     {
       //SecParser sp;            // translates SECONDO syntax into nested list
       //string listCommand = ""; // buffer for command in list form 
       //resultOk = (sp.Text2List( info.result, listCommand ) == 0);
       //if (resultOk)
       resultOk = nl->ReadFromString("(" + info.result + ")", tmpList);
       //cout << nl->ToString(tmpList) << endl;
       tmpList = (nl->First(tmpList));
       realValTolerance.isRelative = true;
       realValTolerance.value = 0.0;
       //realValTolerance.value = 8.881784197e-16; // ~ 2^(-50)
       if (resultOk) {
       switch ( nl->AtomType(tmpList) ) {

          case BoolType: {
                          expectedResult =  MakeConstant("bool", tmpList); 
                          break;
                        }               
          case IntType: {
                          expectedResult =  MakeConstant("int", tmpList); 
                          break;
                        }               
          case RealType:{
                          expectedResult =  MakeConstant("real", tmpList); 
                          //realValTolerance.isRelative = false;
                          //realValTolerance.value = 0.001;
                          break;
                        }
          case TextType:{
                          expectedResult =  MakeConstant("text", tmpList);
                          break; 
                        }
          case StringType:{
                            expectedResult =  MakeConstant("string", tmpList); 
                            break;
                          }
          default: { // result will be interpreted as file name
                     string file = info.result;
                     string opIdent = info.opName;
                     if (info.aliasName != "")
                       opIdent = info.aliasName;

                     if ( file == "file" ) {
                       stringstream ss;
                       ss << "result" << info.number 
                          << "_" << opIdent << "_" << algebraShort;
                       file = ss.str();
                     }
                     file = string(buildDir) + "/Selftest/" + file; 
                     ListExpr objList = nl->Empty();
                     cout << "Reading result from file " << file << endl;
                     resultOk = nl->ReadFromFile(file, objList);
                     if (resultOk)
                     {
		       expectedResult = objList;
		       if ( nl->ListLength(objList) == 5 
			    && nl->IsEqual(nl->First(objList),"OBJECT") ) 
		       {
			 ListExpr fourth = nl->Fourth(objList);
			 ListExpr fifth = nl->Fifth(objList);
			 expectedResult = nl->TwoElemList(fourth, fifth);
		       }  
                     }
                     else
                     {
                       cerr << "File " << file << " contains not a "
                            << " correct nested list!" 
                            << endl;
                     }
                   }                
       } 
       }
     }
     else
     { 
       resultOk = nl->ReadFromString(info.result, expectedResult);
     }
     } 

     if (!resultOk) {
       cerr << "Error: Could not parse result!" << endl;
       expectedResult = nl->SymbolAtom("ERROR");
       numErrors++;
     }  

     testCaseNumber++;
     testCaseName = info.opName;
     testCaseLine = info.lineNo;
     CallSecondo2(); 
  }

  testCaseNumber++;
  testCaseName = "Coverage test for " + algebra;
  testCaseLine = 0;
  yieldState = Coverage;
  CoverageQuery(algebra); 
  CallSecondo2(); 
}


ListExpr 
TestRunner::MakeConstant(const string& type, ListExpr value) 
{
   return nl->TwoElemList(nl->SymbolAtom(type), value);
} 


bool
TestRunner::RunCmd(const string& cmd, SecErrInfo& err) 
{
   ListExpr result = nl->Empty();
   cout << "cmd:" << cmd << endl;
   bool rc = si->Secondo( cmd, result, err);
   return rc;
}

void 
TestRunner::VerifyResult(ListExpr outList, ListExpr expectedResult) 
{
  /* verify that the expected results were delivered */
  bool result = false;
  if (realValTolerance.value != 0.0)
  { 
    cout << "realValTolerance!" << endl;
    result = nl->Equal(outList, expectedResult, realValTolerance);
  }  
  else
  { 
    result = nl->Equal(outList, expectedResult);
  }  

  if(result)
  {
    ShowTestSuccessMsg("computing result as specified.");
  }
  else 
  {
    RegisterError();
    cout 
      << color(red)
      << "The test returned unexpected results!" << endl
      << rightArrow << endl
      << color(normal)
      << "Expected : " << endl;
    nl->WriteListExpr(expectedResult, cout);
    cout << endl << "But got : ";
    nl->WriteListExpr(outList, cout);
    ShowCommand(cmd);
    cout << color(red)
         << leftArrow
         << color(normal) << endl;
  }
}

void 
TestRunner::DisplayError( const string& cmd, ListExpr expectedResult, 
                          int errorCode, const string& errorMessage, 
                          ListExpr outList) 
{
  cout 
    << color(red)
    << "The test returned an error, but should not do so!" << endl
    << rightArrow << endl
    << "Expected result : " << endl
    << nl->ToString(expectedResult) << endl;
  ShowErrCodeInfo(errorCode, errorMessage, outList);
  ShowCommand(cmd);
  cout << color(red)
       << leftArrow 
       << color(normal) << endl;
}


/*
11 CallSecondo

This function gives a query to secondo and receives the result from secondo.

*/

ListExpr
TestRunner::CallSecondo()
{
  int errorCode = 0, errorPos = 0;
  ListExpr cmdList = nl->TheEmptyList();
  ListExpr outList = nl->TheEmptyList();
  string errorMessage = "";
  string errorText = "";


  if(!skipToTearDown)
  {
    if (verbose) { 
      cmsg.info() 
        << "Passing command >>" 
        << cmd << "<< to the secondo interface!" << endl;
      cmsg.send();
    }
 
    if ( cmd[0] == '(' )
    {
      if ( nl->ReadFromString( cmd, cmdList ) )
      {
        si->Secondo( cmd, cmdList, 0, false, false,
                    outList, errorCode, errorPos, errorMessage );
      }
      else
      {
        cout << endl << "*** Error: list expression expected!" << endl;
      }
    }
    else
    {
      cout << cmd << endl;
      si->Secondo( cmd, cmdList, 1, false, false,
                  outList, errorCode, errorPos, errorMessage );
    }
  }



  switch(state)
  {
    case START:
      if( (errorCode != 0) || (errorMessage.length() > 0) )
      {
        /* should we report errors in the intial section? */
        cout
          << color(red)
          << "*** Encountered error in initial section. ***" << endl
          << color(normal)
          << rightArrow << endl;
        ShowErrCodeInfo(errorCode, errorMessage, outList);
        ShowCommand(cmd);
        cout
          << leftArrow << endl;
      }
      break;
    case SETUP:
      if(errorCode != 0)
      {
        cout
          << "*** Encountered error during setup, skipping to teardown. ***" 
          << endl
          << rightArrow << endl;
        ShowErrCodeInfo(errorCode, errorMessage, outList);
        ShowCommand(cmd);
        skipToTearDown = true;
        numErrors = -128;
        cout
          << leftArrow << endl;
      }
      break;
    case TESTCASE:
      if(!skipToTearDown)
      {
 
        if( yieldState == Error)
        {
          if (errorCode)
          {       
           ShowTestSuccessMsg("returned an error as expected.");
          }
          else
          {               
          RegisterError();
          cout 
            << color(red)
            << "The test suceeded, but an error was expected!" << endl
            << color(normal)
            << rightArrow << endl
            << "Result : " << endl;
          nl->WriteListExpr(outList, cout);
          ShowCommand(cmd);
          cout
            << leftArrow << endl;
          }
        }
        else if( yieldState == Success )
        {
          if(!errorCode)
          {
            ShowTestSuccessMsg("succeeded as expected.");
          }
          else
          {
            RegisterError();
            DisplayError(cmd, expectedResult, errorCode, errorMessage, outList);
          }
        }
        else if( yieldState == Result )
        {
          if(errorCode)
          {
            RegisterError();
            DisplayError(cmd, expectedResult, errorCode, errorMessage, outList);
          }
          else 
          {
            VerifyResult(outList, expectedResult);
          }  
        }
        else if( yieldState == Coverage )
        {
          if(errorCode)
          {
            RegisterError();
            DisplayError(cmd, expectedResult, errorCode, errorMessage, outList);
          }
          else 
          {
            VerifyResult(outList, expectedResult);
            missingOps += nl->ToString(nl->Second(outList));
          }  
        }
        else if( yieldState == Unknown )
        {
          RegisterError();
          cout 
            << color(red)
            << "The test has an unknown yields value!" << endl
            << color(normal) << endl;
        }
        else if( yieldState == UndefinedObj )
        {
          RegisterError();
          cout 
            << color(red)
            << "The test's yields value is an undefined or " 
            << "unpresent database object!" << endl
            << color(normal) << endl;
        }
        else { // default

          assert( false ); // should never happen
        }
      }
      break;
    case TEARDOWN:
      if(errorCode != 0)
      {
        cout
          << color(red)
          << "*** Encountered error during teardown. ***" << endl 
          << color(normal)
          << rightArrow << endl;
          ShowErrCodeInfo(errorCode, errorMessage, outList);
          ShowCommand(cmd);
        cout
          << leftArrow << endl;
      }

      break;
  }

  if ( errorCode != 0 )
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
  ShowTestTitle();
  ListExpr result;
  result = CallSecondo();
  nl->initializeListMemory();
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
  cout << endl
       << "--- Secondo TestRunner ---"
       << endl << endl;
  
    streambuf* oldInputBuffer  = 0;
    streambuf* oldOutputBuffer = 0;
    ifstream fileInput;
    ofstream fileOutput;
    si = new SecondoInterface();
    if ( si->Initialize( user, pswd, host, port, parmFile ) )
    {
      if ( iFileName.length() > 0 )
      {
        cout << endl << "--- Opening file " 
             << iFileName << " ---" << endl << endl;
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


      // check RTFlags
      if ( RTFlag::isActive("Test:Verbose") ) {
        verbose = true;
      } 


      if (runExamples) 
      {
        ProcessExamples();
      }
      else { 
        ProcessCommands();
      }  

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
    ShowErrorSummary();

    si->Terminate();
    delete si;
    cout << "--- Secondo TestRunner terminated ---" << endl;

  return (numErrors);
}

int SecondoTestRunner(const TTYParameter& tp)
{
  TestRunner* appPointer = new TestRunner( tp );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}
