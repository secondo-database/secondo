/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//[_] [\_]

\vspace{1cm}

\centerline{\LARGE \bf  SecondoTTY}


Changes:


December 1997: Friedhelm Becker

July 1999: Jose Antonio Cotelo Lema: changes in the code and interface of the
Gettext() and getline() functions, to allow input commands of arbitrary size.

Dec 2004, M. Spiekermann. The read in command lines will be separated by a
"\\n" symbol, otherwise the parser can't calculate a position in terms of lines
and cols.

July 2005, M. Spiekermann. Help message improved.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains.

February 2006, M. Spiekermann reorganized the makefiles in order to save linking
instructions. Now only two applications are built, namely ~SecondoBDB~ and ~SecondoCS~.
The former known applications are replaced by shell scripts invoking one of the applications
above with suitable paramaters.

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

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctype.h>
#include "getCommand.h"

#ifdef HAVE_LIBREADLINE
  #include <stdio.h>
  #include <readline/readline.h>
  #include <readline/history.h>
  #define HISTORY_FILE ".secondo_history"
  #define HISTORY_FILE_ENTRIES 200
#endif

#include "Application.h"

#include "Profiles.h"
#include "FileSystem.h"
#include "SecondoSystem.h"
#include "SecondoInterface.h"

#if !defined(SECONDO_CLIENT_SERVER) && !defined(REPLAY)
#include "SecondoInterfaceTTY.h"
#elif defined(SECONDO_CLIENT_SERVER)
#include "SecondoInterfaceCS.h"
#elif defined(REPLAY)
#include "SecondoInterfaceREPLAY.h"
#else
#include "SecondoInterfaceCS.h"
#endif

#include "SecondoSMI.h"
#include "NestedList.h"
#include "DisplayTTY.h"
#include "ErrorCodes.h"

// The embedded build optimizes the SQL dialect in-process via these helpers.
#ifndef NO_OPTIMIZER
#include "SecondoPL.h"
#endif
#include "CharTransform.h"
#include "LogMsg.h"
#include "TTYParameter.h"

using namespace std;

class SecondoTTY : public Application
{
 public:
  SecondoTTY( const TTYParameter& t );
  virtual ~SecondoTTY() {};
  bool AbortOnSignal( int sig ) const;
  int  Execute();

 private:
  void Usage();
  bool ProcessFile( const string& fileName, const bool stopOnError,
                    const bool isPD );
  bool ProcessCommand();
  bool ProcessCommands( const bool stopOnError, const bool isPD);
  void ShowPrompt( const bool first );
  void TypeOutputList ( ListExpr list );
  bool IsInternalCommand( const string& line );
  bool GetCommand( const bool isPD);
  void ShowQueryResult( ListExpr list );
  ListExpr CallSecondo();
  void CallSecondo2();
  bool MatchQuery(string& w, istringstream& is) const;
  string ReadCommand(istringstream& is) const;

  string parmFile;
  string home;
  string user;
  string pswd;
  string host;
  string port;
  string replayFile;
  string iFileName;
  string oFileName;

  string            cmd;
  string            prompt;
  const string      prompt_first;
  const string      prompt_next;

  int               errorCode;
  bool              isStdInput;
  bool              quit;
  NestedList*       nl;
  bool              isQuery;
  //SecondoInterface* si;
  #if !defined(SECONDO_CLIENT_SERVER) && !defined(REPLAY)
  SecondoInterfaceTTY* si;
  #elif defined(SECONDO_CLIENT_SERVER)
  SecondoInterfaceCS* si;
  #elif defined(REPLAY)
  SecondoInterfaceREPLAY* si;
  #else
  SecondoInterfaceCS* si;
  #endif
};

SecondoTTY::SecondoTTY( const TTYParameter& t )
  : Application( t.numArgs, (const char**)t.argValues ),
    cmd(""),
    prompt(""),
    prompt_first("Secondo => "),
    prompt_next("Secondo -> ")
{
  parmFile = t.parmFile;
  home = t.home;
  user = t.user;
  pswd = t.pswd;
  host = t.host;
  port = t.port;
  replayFile = t.replayFile;
  iFileName = t.iFileName;
  oFileName = t.oFileName;

  isStdInput    = iFileName.empty();
  quit          = false;
  nl            = 0;
  si            = 0;
  errorCode     = 0;
}

bool
SecondoTTY::AbortOnSignal( int sig ) const
{
  return (true);
}

void
SecondoTTY::Usage()
{
  stringstream cmdList;
  cmdList <<
  "The following internal commands are available:\n" <<
  "\n" <<
  "  ?, HELP  - display this message\n" <<
  "\n" << 
  "  @FILE    - read commands from file 'FILE' (may be nested)\n" <<
  "  @@FILE   - read commands from file 'FILE' until 'FILE' is \n" <<
  "             completely processed or an error occurred\n" <<
  "  @\%FILE   - read commands from file ignoring comments in pd style\n" <<
  "  @&FILE   - read commands from file ignoring comments in pd style\n" <<
  "             until file is completely processed or an error occurred\n"<<
  "\n" <<
  "  DEBUG n  - set debug level to n where n is an integer where each \n" <<
  "             bit corresponds to one setting:\n" <<
  "           bit  0: debug mode (show annotated query and operator tree)\n" <<
  "           bit  1: trace (show recursive calls)\n" <<
  "           bit  2: trace nodes (construction of nodes of the op. tree,\n" <<
  "                   and execution of the query processor's Eval() method)\n"<<
  "           bit  3: localInfo (prints a warning if an operator did not\n" <<
  "                   destroy its localinfo before the operator tree\n"<<
  "                   is deconstructed)\n" <<
  "           bit  4: debug progress (after sending a REQUESTPROGRESS\n" <<
  "                   message to an operator, the ranges in the \n" <<
  "                   ProgressInfo are checked for whether tey are\n" <<
  "                   reasonable. If not so, the according operator and \n" <<
  "                   ProgressInfo are reported) \n" <<
  "           bit  5: trace progress (prints the result of \n" <<
  "                   each REQUESTPROGRESS message) \n" <<
  "           bit  6: show type mappings \n" <<
  "\n" <<
  "  Q, QUIT  - exit the program\n" <<
  "  # ...    - comment line (first character on line has to be '#')\n" <<
  "  REPEAT n <query> - execute <query> n times.\n" <<
  "\n" <<
  "Moreover, you may enter any valid SECONDO command introduced by the \n" <<
  "keywords 'query', 'let', 'restore', etc. "
  "Refer to the \"User Manual\" for \n" <<
  "details. Internal commands are restricted to ONE line, while SECONDO \n" <<
  "commands may span several lines; a semicolon as the last character on \n" <<
  "a line terminates a command, but is not part of the command. \n" <<
  "Alternatively, you may enter an empty line.\n";

  cout << cmdList.str() << endl << endl;
}

bool
SecondoTTY::ProcessFile( const string& fileName , const bool stopOnError,
                         const bool isPD)
{
  bool saveIsStdInput = isStdInput;
  streambuf* oldBuffer;
  ifstream fileInput( fileName.c_str() );
  if ( fileInput ) {
    oldBuffer = cin.rdbuf( fileInput.rdbuf() );
    cout << "*** Begin processing file '" << fileName << "'." << endl;
    isStdInput = false;

    StopWatch scriptTime;
    scriptTime.start();

    bool res = ProcessCommands(stopOnError, isPD);

    cout << "Runtime for " << fileName << ": "
         << scriptTime.diffTimes() << endl;

    isStdInput = saveIsStdInput;
    if(!res){
      cout << "Errors during processing the file "  << fileName << "."<< endl; 
    } else {
      cout << "File " << fileName << " successfully processed." << endl;
    }
    cin.rdbuf( oldBuffer );
    return res; 
  } else {
    cerr << "*** Error: Could not access file '" << fileName << "'." << endl;
    return false;
  }
}

string
SecondoTTY::ReadCommand(istringstream& is) const
{
  string cmdWord = parse<string>(is);
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(),
             ::toupper );
  return cmdWord;
}

bool
SecondoTTY::MatchQuery(string& cmdWord, istringstream& is) const
{
  bool isQuery = false;
  size_t pos = cmd.find("query");
  size_t pos2 = cmd.find("querynt");
  size_t pos3 = cmd.find("pquery");
  // The SQL dialect ("sql ..." or bare "select ...") produces a result to be
  // displayed too.
  size_t pos4 = cmd.find("sql");
  size_t pos5 = cmd.find("select");
  size_t pos6 = cmd.find("SELECT");

  if ((pos == string::npos) && (pos2==string::npos) && (pos3==string::npos)
       && (pos4==string::npos) && (pos5==string::npos) && (pos6==string::npos)){
    return isQuery;
  }


  // remove leading brackets
  while(!cmdWord.empty() && cmdWord[0]=='('){
     cmdWord = cmdWord.substr(1);
  }

  if ( (cmdWord == "QUERY") || (cmdWord== "QUERYNT") || (cmdWord=="PQUERY")
       || (cmdWord=="SQL") || (cmdWord=="SELECT") )
  {
    isQuery = true;
  }
  else
  {
    if ( cmdWord == "" )
    {
      cmdWord = ReadCommand(is);
      if((cmdWord == "QUERY") || (cmdWord == "QUERYNT") || (cmdWord=="PQUERY")
          || (cmdWord=="SQL") || (cmdWord=="SELECT"))
        isQuery = true;
      else
        isQuery = false;
    }
  }
  return isQuery;
}



bool SecondoTTY::ProcessCommand()
{
  if(cmd.find_first_not_of(" \t\t\n")==string::npos){
    // found empty command
    cmd="";
    return true;
  }

  istringstream is(cmd);
  string cmdWord = ReadCommand(is);

  bool success = true;

  // analyse command
  if ( cmdWord == "?" || cmdWord == "HELP" )
  {
    Usage();
  }
  else if ( cmdWord == "DEBUG" )
  {
    int debugLevel = parse<int>(is);
    si->SetDebugLevel( debugLevel );
    cout << "*** Debug level set to " << debugLevel << "." << endl;
  }
  else if ( cmdWord == "Q" || cmdWord == "QUIT" )
  {
    cout << "*** Thank you for using SECONDO!" << endl << endl;
    quit = true;
  }
  else if ( cmdWord[0] == '@' )
  {
    bool stopOnError = false;
    int start=1;
    bool pdfile=false;
    if(cmdWord.length()>1 && cmdWord[1] == '@'){
      start = 2;
      stopOnError = true;
    }
    if(cmdWord.length()>1 && cmdWord[1] == '%'){
       start = 2;
       pdfile = true;       
    }
    if(cmdWord.length()>1 && cmdWord[1] == '&'){
       start = 2;
       pdfile = true;       
       stopOnError = true;
    }
    success = ProcessFile( cmd.substr( start, ( cmd.length() - start ) ),
                           stopOnError, pdfile );
  }
  else if ( cmdWord == "REPEAT" )
  {
    int repeatCtr = parse<int>(is);
    cmdWord = ReadCommand(is);

    isQuery = MatchQuery(cmdWord, is);
    const string err =
              "Syntax Error: Expecting REPEAT n { query ... | ( query ...]}!";

    if (!isQuery)
    {
     cerr << err << endl;
     success = false;
    }
    else
    {
      // remove repeat <n> from the cmd
      cmd = cmd.substr(6);
      size_t pos = cmd.find_first_of("(q");
      if ( pos != string::npos )
      {
        cmd = cmd.substr( pos );
        cout << "Repeating next query " << repeatCtr << " times ..." << endl;

        while (repeatCtr > 0) {
          CallSecondo2();
          if (errorCode > 0){ // exit loop on failure
            success = false;
            break;
          }
          repeatCtr--;
        }
      }
      else
      {
        // should never happen !
        success = false;
        cerr << err << endl;
      }
    }
  }
  else 
  {
    isQuery = MatchQuery(cmdWord,is);
    CallSecondo2();
    success = errorCode == 0;
  }
  cmd="";
  return success;
}



void
SecondoTTY::ShowPrompt( const bool first )
{
  if ( !isStdInput ) // don't show command prompt
    return;

  prompt = first ? prompt_first : prompt_next;

  #ifdef HAVE_LIBREADLINE
    rl_set_prompt( prompt.c_str() );
  #else
    cout << prompt;
  #endif
}

bool
SecondoTTY::IsInternalCommand( const string& line )
{
  string cmdWord;
  istringstream is( line );
  is >> cmdWord;
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(),
             ::toupper );

  return ( cmdWord == "?" || cmdWord == "HELP"        ||
           cmdWord == "Q" || cmdWord == "QUIT"        ||
           cmdWord == "DEBUG" || cmdWord == "SHOW" || cmdWord[0] == '@' );
}

bool
SecondoTTY::GetCommand( const bool isPD)
{
  function<void(const bool)> showPrompt 
                = [this](const bool first) {ShowPrompt(first); };
  function<bool(const string&)> isInternalCommand 
                = [this](const std::string& c) {return IsInternalCommand(c); };
  bool res =  getCommand(cin, isPD, cmd,
                    showPrompt, isInternalCommand,
                    isStdInput, prompt);
  return res;
}

bool
SecondoTTY::ProcessCommands(const bool stopOnError, const bool isPD) {
  bool errorFound = false;;
  while (!cin.eof() && !quit && ( !stopOnError || !errorFound ))
  {
    if ( GetCommand(isPD) )
    {
      try {
        if( ! ProcessCommand()){
           errorFound = true;
        }
      }
      catch (SecondoException &e) {
        cerr << "Exception caught: " << e.msg() << endl;
      }
    }
  }
  return !errorFound;
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
    DisplayTTY::GetInstance().DisplayResult( nl->First( list ),
                                             nl->Second( list ) );
  }
}


/*
10.1 Recognizing the SQL dialect

Deciding whether a typed command belongs to the optimizer or to the kernel is
not a matter of the first keyword alone: ~delete~, ~create~, ~update~ and ~let~
exist in both languages and are told apart by the following token(s). The rule
set below is the one the retired hybrid TTY (~SecondoPLTTYMode::isSqlCommand~)
and the JavaGUI (~CommandPanel.optimize~) independently arrived at.

Anything not recognized here falls through to the kernel -- the fallback is
always the kernel, in every client.

*/

#if defined(SECONDO_CLIENT_SERVER) || !defined(NO_OPTIMIZER)

// Split cmd into at most maxTok lower-cased tokens. The delimiter set matches
// the JavaGUI's classifier, so that "let r5=select ..." and "create table t(a)"
// tokenize identically there and here.
static void sqlTokens(const std::string& cmd, std::vector<std::string>& toks,
                      size_t maxTok)
{
  const string delims = " \t\n\r\f\v\b\a([{=.,;";
  size_t pos = 0;
  while ( toks.size() < maxTok )
  {
    size_t s = cmd.find_first_not_of(delims, pos);
    if ( s == string::npos ) break;
    size_t e = cmd.find_first_of(delims, s);
    string t = cmd.substr(s, (e==string::npos) ? string::npos : e - s);
    for ( size_t i = 0; i < t.size(); i++ )
    {
      t[i] = tolower((unsigned char) t[i]);
    }
    toks.push_back(t);
    if ( e == string::npos ) break;
    pos = e;
  }
}

static bool looksLikeSql(const std::string& cmd)
{
  size_t s = cmd.find_first_not_of(" \t\n\r\f\v\b\a");
  if ( s == string::npos ) return false;
  // nested list command or command sequence: always the kernel
  if ( cmd[s] == '(' || cmd[s] == '{' ) return false;

  std::vector<std::string> t;
  sqlTokens( cmd, t, 3 );
  if ( t.empty() ) return false;

  // unambiguous openers
  if (    t[0] == "sql"   || t[0] == "select" || t[0] == "union"
       || t[0] == "intersection" || t[0] == "drop" ) return true;

  // ambiguous with kernel commands: need the second token
  if ( t.size() < 2 ) return false;
  if ( t[0] == "delete" && t[1] == "from" ) return true;
  if ( t[0] == "insert" && t[1] == "into" ) return true;
  if ( t[0] == "create" && (t[1] == "table" || t[1] == "index") ) return true;

  // ... or the third
  if ( t.size() < 3 ) return false;
  if ( t[0] == "update" && t[2] == "set" ) return true;
  // "let <ident> = select|union|intersection ...": an SQL right-hand side. The
  // server splits the prefix off and re-wraps the generated plan.
  if ( t[0] == "let" && (   t[2] == "select" || t[2] == "union"
                         || t[2] == "intersection" ) ) return true;

  return false;
}

/*
Show the optimizer's cost estimate for the plan it just produced. Printed
separately from the plan so both clients report it the same way, and skipped
when there is no estimate (the optimizer yields 0 for a plan it did not cost,
and for create/drop, which it executes itself).

*/
static void showCosts(double costs)
{
  if ( costs <= 0.0 ) return;
  cout << color(blue) << "Estimated costs: " << costs
       << color(normal) << endl;
}

/*
Say that the plan was not run, so an empty result is not mistaken for a query
that returned nothing.

*/
static void showPlanOnlyNote(bool planOnly)
{
  if ( !planOnly ) return;
  cout << color(blue) << "Plan only -- not executed." << color(normal) << endl;
}

/*
Strip a leading ~optimizer~ keyword (the prefix the JavaGUI uses to address the
optimizer directly). Returns true and puts the remainder into ~rest~ if the
prefix was there; otherwise returns false and leaves ~rest~ = ~cmd~.

*/
static bool stripOptimizerPrefix(const std::string& cmd, std::string& rest)
{
  rest = cmd;
  size_t s = cmd.find_first_not_of(" \t\n\r\f\v\b\a");
  if ( s == string::npos ) return false;

  const string kw = "optimizer";
  if ( cmd.size() - s < kw.size() + 1 ) return false;
  for ( size_t i = 0; i < kw.size(); i++ )
  {
    if ( tolower((unsigned char) cmd[s+i]) != kw[i] ) return false;
  }
  // must be a whole word, and something has to follow it
  size_t after = s + kw.size();
  if ( !isspace((unsigned char) cmd[after]) ) return false;
  size_t r = cmd.find_first_not_of(" \t\n\r\f\v\b\a", after);
  if ( r == string::npos ) return false;

  rest = cmd.substr(r);
  return true;
}

// The optimizer control directives the TTY routes to the directive channel
// when they are typed bare. With an explicit "optimizer " prefix any non-SQL
// text is treated as a directive, so this list is only the convenience case.
static bool isBareOptimizerDirective(const std::string& cmd)
{
  size_t s = cmd.find_first_not_of(" \n\r\t\v\b\a\f");
  if ( s == string::npos ) return false;
  size_t nameEnd = cmd.find_first_of(" \n\r\t\v\b\a\f(", s);
  string name = cmd.substr(s, (nameEnd==string::npos) ? string::npos
                                                      : nameEnd - s);
  // The directive names are Prolog atoms, matched case-sensitively; the
  // argument (if any) starts at the '('.
  static const char* const directives[] = {
    "setOption", "delOption", "showOptions",
    "loadOptions", "saveOptions", "defaultOptions",
    "updateCatalog", "resetKnowledgeDB",
    "helpMe" };   // advertised by the showOptions listing itself
  for ( size_t i = 0; i < sizeof(directives)/sizeof(directives[0]); i++ )
  {
    if ( name == directives[i] ) return true;
  }
  return false;
}

#endif

/*
11 CallSecondo

This function gives a query to secondo and receives the result from secondo.

*/

ListExpr
SecondoTTY::CallSecondo()
{
  errorCode = 0;
  int errorPos = 0;
  ListExpr cmdList = nl->TheEmptyList();
  ListExpr outList = nl->TheEmptyList();
  string errorMessage = "";
  string errorText = "";

  size_t cmdStart = cmd.find_first_not_of(" \n\r\t\v\b\a\f");

  // An explicit "optimizer " prefix addresses the optimizer directly -- the
  // same prefix the JavaGUI offers. What follows decides what it means:
  //   optimizer <sql>        optimize only, show the plan, do NOT run it
  //   optimizer <goal>       run an optimizer control directive
  // Without the prefix, SQL is optimized *and* executed, which is the common
  // case and what a bare "select ..." does.
  //
  // optCmd is the command with the prefix removed; it is what gets sent, so
  // the optimizer never sees the keyword itself.
  string optCmd = cmd;
  bool optPrefix = false;
#if defined(SECONDO_CLIENT_SERVER) || !defined(NO_OPTIMIZER)
  optPrefix = stripOptimizerPrefix( cmd, optCmd );
#endif

  // Detect a leading "sql" or "select" token: the SQL dialect. Both forms are
  // accepted (the "sql" prefix is optional), case insensitively. In the
  // client/server build it is sent to the server (command level 2, or level 3
  // when only the plan is wanted) and optimized there; in the embedded build it
  // is optimized in-process. The detection is only active where an optimizer is
  // reachable.
  bool isSql = false;
#if defined(SECONDO_CLIENT_SERVER) || !defined(NO_OPTIMIZER)
  isSql = looksLikeSql( optCmd );
#endif

  // "optimizer <sql>": stop after optimizing and report the plan.
  bool planOnly = optPrefix && isSql;

  // Detect an optimizer control directive (setOption(X), delOption(X),
  // showOptions, updateCatalog, ...). Like the SQL detection above this is only
  // active where an optimizer is reachable, and it is routed to the optimizer's
  // directive channel below (over the network in SecondoCS, in-process in the
  // standalone SecondoBDB).
  //
  // Bare, the known directive names are recognized. After an explicit
  // "optimizer " prefix anything that is not SQL is a directive, so arbitrary
  // optimizer goals can be run -- which is what the prefix does in the JavaGUI.
  bool isOptDirective = false;
#if defined(SECONDO_CLIENT_SERVER) || !defined(NO_OPTIMIZER)
  if ( !isSql && cmdStart != string::npos )
  {
    isOptDirective = optPrefix ? true : isBareOptimizerDirective( cmd );
  }
#endif

  if ( isSql )
  {
#ifdef SECONDO_CLIENT_SERVER
    // Client/server: send the SQL to the server, which optimizes it and
    // returns the list (optimizedPlan result costs). With planOnly ("optimizer
    // <sql>") the server stops after optimizing and the result half comes back
    // empty.
    ((SecondoInterfaceCS*) si)->SecondoSql( optCmd, planOnly, outList,
                                            errorCode, errorPos, errorMessage );
    NList::setNLRef(nl);
    // Accept any length from two upwards: costs was appended to the original
    // (plan result) shape, so a server that does not send it still works.
    if ( errorCode == 0 && nl->ListLength( outList ) >= 2 )
    {
      string planStr = "";
      if ( nl->AtomType( nl->First( outList )) == TextType )
      {
        nl->Text2String( nl->First( outList ), planStr );
      }
      if ( planStr == "done" )
      {
        // create/drop: the server's optimizer carried the command out itself
        // (same wording as the standalone build, see the embedded branch).
        cout << endl << color(blue)
             << "Executed by the optimizer (no plan to run)."
             << color(normal) << endl;
      }
      else
      {
        cout << endl << color(blue) << "Optimized plan: " << planStr
             << color(normal) << endl;
        if ( nl->ListLength( outList ) >= 3
             && nl->AtomType( nl->Third( outList )) == RealType )
        {
          showCosts( nl->RealValue( nl->Third( outList )) );
        }
        showPlanOnlyNote( planOnly );
      }
      outList = nl->Second( outList );
    }
#elif !defined(NO_OPTIMIZER)
    // Embedded: run the optimizer in-process to turn the SQL into an
    // executable plan, then execute the plan.
    if ( !initEmbeddedOptimizer( si ) )
    {
      errorCode = ERR_OPTIMIZER_NOT_AVAILABLE;
      errorMessage = "Could not initialize the optimizer.";
    }
    else
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        embeddedOptimizerUseDatabase(
            SecondoSystem::GetInstance()->GetDatabaseName() );
      }
      string plan = "", optErr = "";
      double costs = 0.0;
      bool alreadyExecuted = false;
      if ( embeddedSqlToPlan( optCmd, plan, costs, optErr, alreadyExecuted ) )
      {
        if ( alreadyExecuted )
        {
          // create/drop: the optimizer carried the command out itself.
          cout << endl << color(blue)
               << "Executed by the optimizer (no plan to run)."
               << color(normal) << endl;
        }
        else
        {
          cout << endl << color(blue) << "Optimized plan: " << plan
               << color(normal) << endl;
          showCosts( costs );
          showPlanOnlyNote( planOnly );
          if ( !planOnly )
          {
            si->Secondo( plan, cmdList, 1, false, false,
                         outList, errorCode, errorPos, errorMessage );
            NList::setNLRef(nl);
          }
        }
        // DDL run through the optimizer changes the catalog it has cached;
        // refresh it so later queries see the new schema. Not conditional on
        // planOnly: sqlToPlan carries create/drop out while translating, so
        // "plan only" cannot keep them from changing the catalog.
        if ( errorCode == 0 && embeddedOptimizerSqlChangesCatalog( optCmd ) )
        {
          string refreshErr = "";
          embeddedOptimizerRefreshCatalog( refreshErr );
          NList::setNLRef(nl);
        }
      }
      else
      {
        errorCode = ERR_OPTIMIZER_NOT_AVAILABLE;
        errorMessage = "Optimization failed: " + optErr;
      }
    }
#else
    // Should be unreachable: isSql is only set when an optimizer is reachable.
    errorCode = ERR_OPTIMIZER_NOT_AVAILABLE;
    errorMessage = "SQL is not available (optimizer not compiled in).";
#endif
  }
  else if ( isOptDirective )
  {
#ifdef SECONDO_CLIENT_SERVER
    // Client/server (SecondoCS): run the directive on the server's embedded
    // optimizer via the <OptimizerCommand> channel and print what it wrote.
    string out = ((SecondoInterfaceCS*)si)->optimizerCommand( optCmd );
    cout << endl << out << endl;
#elif !defined(NO_OPTIMIZER)
    // Standalone (SecondoBDB): run the directive in the in-process optimizer.
    if ( !initEmbeddedOptimizer( si ) )
    {
      errorCode = ERR_OPTIMIZER_NOT_AVAILABLE;
      errorMessage = "Could not initialize the optimizer.";
    }
    else
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        embeddedOptimizerUseDatabase(
            SecondoSystem::GetInstance()->GetDatabaseName() );
      }
      string out = "", optErr = "";
      if ( embeddedOptimizerRunGoal( optCmd, out, optErr ) )
      {
        cout << endl << out << endl;
      }
      else
      {
        // A failing directive may still have printed something useful.
        cout << endl << (out.empty() ? optErr : out) << endl;
      }
    }
#else
    errorCode = ERR_OPTIMIZER_NOT_AVAILABLE;
    errorMessage = "The optimizer is not available (not compiled in).";
#endif
  }
  else if ( cmdStart != string::npos && cmd[cmdStart] == '(' )
  {
    if ( nl->ReadFromString( cmd, cmdList ) )
    {
      si->Secondo( cmd, cmdList, 0, false, false,
                   outList, errorCode, errorPos, errorMessage );
      NList::setNLRef(nl);
    }
    else
    {
      cmsg.error() << endl << "*** Error: list expression expected!" << endl;
      cmsg.send();
    }
  }
  else
  {
    si->Secondo( cmd, cmdList, 1, false, false,
                 outList, errorCode, errorPos, errorMessage );
    NList::setNLRef(nl);
  }

  if ( errorCode != 0 )
  {
    si->WriteErrorList( outList );
    nl->Destroy( outList );
    outList = nl->TheEmptyList();
  }
  if ( cmdList != nl->TheEmptyList() )
  {
    nl->Destroy( cmdList );
  }

  // print out error messages
  cerr << endl;
  if (errorCode != 0)
  {
    if (errorMessage != "")
    {
      cerr << color(red) << errorMessage << color(normal) << endl;
    }
    else
    {
      // retrieve error message for errorCode
      cmsg.error() << si->GetErrorMessage( errorCode ) << endl;
      cmsg.send();
    }

    if(errorCode == ERR_SYSTEM_DIED) {
       cerr << "A fatal error has occurred!" << endl;
       cerr << "Please check the logs for more information." << endl;
       exit(EXIT_FAILURE);
    }
  }

  return (outList);
}

/*
12 Secondo2

This would normally be the main function of SecondoTTY.

*/

void
SecondoTTY::CallSecondo2()
{
  ListExpr result;
  result = CallSecondo();

#ifdef NL_DEBUG
  cerr << endl << "### ResultStr: " << nl->ToString(result) << endl;
#endif

  if ( isQuery ) {
    ShowQueryResult( result );
  }
  else {
    if(nl->HasLength(result,2) &&
       nl->IsEqual(nl->First(result),"inquiry")){
       ShowQueryResult( result );
    } else {
       TypeOutputList( result );
    }
  }
  nl->initializeListMemory();
}


/*
1 Execute

This function checks the configuration of the Secondo system. If the
configuration seems to be ok the system is intialized. If the initialization
succeeds the user commands are processed. If the initialization fails or the
user finishes work the system is terminated.

*/

int
SecondoTTY::Execute()
{
  int rc = 0;
  cout << endl
       << "*** Secondo TTY ***"
       << endl << endl;

  streambuf* oldOutputBuffer = 0;
  ofstream fileOutput;
  bool useOutputFile = oFileName.length() > 0;

  #if !defined(SECONDO_CLIENT_SERVER) && !defined(REPLAY)
  si = new SecondoInterfaceTTY(false);
  #elif defined(SECONDO_CLIENT_SERVER)

  si = new SecondoInterfaceCS(true,0,true);

  #elif defined(REPLAY)
  si = new SecondoInterfaceREPLAY(true);
  ((SecondoInterfaceREPLAY*)si)->setReplayFile(replayFile);
  #else
  si = new SecondoInterfaceCS(true,0,true);
  #endif

  string errorMsg("");
  if ( si->Initialize( user, pswd, host, port, parmFile,home, errorMsg  ) )
  {

    // initialize the pointer to the nested list instance
    nl = si->GetNestedList();
    NList::setNLRef(nl);
  //  DisplayTTY::Set_SI( si );
    DisplayTTY::Set_NL( nl );
    DisplayTTY::Initialize();

    if ( useOutputFile )
    {
      fileOutput.open( oFileName.c_str() );
      if ( fileOutput.is_open() )
      {
        oldOutputBuffer = cout.rdbuf( fileOutput.rdbuf() );
        cout << endl << "Redirecting output to " << oFileName << endl;
      }
      else
      {
        cerr << "Error: Could not redirect ouput to " << oFileName << endl;
        useOutputFile = false;
      }
    }

    if ( isStdInput )
    {
      cout << endl << "Secondo TTY ready for operation." << endl;
#ifdef SECONDO_CLIENT_SERVER
      cout << "Optimizer (SQL dialect): "
           << (((SecondoInterfaceCS*)si)->optimizerAvailable()
                 ? "available" : "not available")
           << " on the connected server." << endl;
#elif !defined(NO_OPTIMIZER)
      cout << "Optimizer (SQL dialect): available (in-process)." << endl;
#endif
      cout << "Type 'HELP' to get a list of available commands." << endl;
      ProcessCommands( false, false);
    }
    else
    {
      // Report a failed command file to the shell.
      if ( !ProcessFile(iFileName, false, false) )
      {
        rc = 2;
      }
    }

    if ( useOutputFile ){
      cout.rdbuf( oldOutputBuffer );
    }
    DisplayTTY::Finish();
  } else {
    if ( errorMsg.empty() )
    {
      errorMsg = "No further information available.";
    }
    cerr << endl << "Error in initializing the Secondo system:" << endl
         << errorMsg << endl;
    rc = 1;
  }

  try {
    si->Terminate();
    delete si;
    si = 0;
  }
  catch (SecondoException &e)
  {
     cerr << e.msg() << endl;
     rc = 17;
  }
  return (rc);
}


#ifdef HAVE_LIBREADLINE
/*
15 Keyword extraction

~commands~

This array contains all keywords used in SECONDO for expansion with
the tab key.
Duplicates are not allowed in this array and the last entry has to
be NULL.

*/
const char *keywords[] = {"abort",
                          "algebra",
                          "algebras",
                          "begin",
                          "commit",
                          "close",
                          "constructors",
                          "consume",
                          "count",
                          "create",
                          "database",
                          "databases",
                          "DEBUG",
                          "delete",
                          "derive",
                          "else",
                          "endif",
                          "endwhile",
                          "extend",
                          "feed",
                          "filter",
                          "from",
                          "if",
                          "kill",
                          "let",
                          "let_",
                          "list",
                          "objects",
                          "open",
                          "operators",
                          "query",
                          "restore",
                          "save",
                          "SHOW",
                          "then",
                          "transaction",
                          "type",
                          "types",
                          "update",
                          "while",
                          "SEC2TYPEINFO",
                          "SEC2OPERATORUSAGE",
                          "SEC2OPERATORINFO",
                          "SEC2FILEINFO",
                          "SEC2COUNTERS",
                          "SEC2COMMANDS",
                          "SEC2CACHEINFO",
                          (char *)0};

/*
~dupstr~

This funCtion returns a clone of the argument string;

*/
char *
dupstr (const char* s)
{
  char *r;
  r =(char*) malloc (strlen (s) + 1);
  strcpy (r, s);
  return (r);
}


/*
~strcmp~

The function ~strcmp~ compares two string up to a given length.

*/
int strcmp(const char* s1,const char* s2, int len){
 int index = 0;
 while((s1[index]==s2[index]) && (index<len)) index++;
 if (index==len) index--;
 if(s1[index]<s2[index]) return -1;
 if(s1[index]>s2[index]) return 1;
 return 0;
}

/*
~command[_]generator~

This function computes the next match of the current partial input
to the avaiable keywords.

*/
char* command_generator(const char* text, int state)
{
   static int index,len;
   const char* name;
   if(!state){
      index=0;
      len = strlen(text);
   }
   while( (name = keywords[index])){
      index++;
      int cmp = strcmp(name,text,len);
      if(cmp==0){
         return (dupstr(name));
      }
   }
   return ((char*)NULL);
}


/*
~secondo[_]completion~

This function is used by the readline library to determine all string
with the same beginning like the current input.

*/
char** secondo_completion(const char* text, int start, int end){
   return rl_completion_matches(text,command_generator);
}

#endif

int SecondoTTYMode(const TTYParameter& tp)
{
  SecondoTTY* appPointer = new SecondoTTY( tp );
#ifdef HAVE_LIBREADLINE
  rl_initialize();
  rl_readline_name = "secondo";
  rl_attempted_completion_function = secondo_completion;
  
  /* read the history from file */
  ifstream hist_file(HISTORY_FILE);
  string histline;
  if(hist_file){
    string query("");
    while(!hist_file.eof()){
       getline(hist_file,histline);
       if(histline.find_last_not_of(" \t\n")!=string::npos){
          if(query!=""){
              query = query + "\n" + histline;
          } else {
              query = histline;
          }
       } else if(query.length()>0){
          add_history(query.c_str());
          query = "";
       }
    }
    if(query!=""){
      add_history(query.c_str());
      query = "";
    }
    hist_file.close();
  } 
#endif

  int rc = appPointer->Execute();
  delete appPointer;

#ifdef HAVE_LIBREADLINE
  /* 
   * save the last HISTORY_FILE_ENTRIES elements of the 
   * history to a file 
   */
  
  fstream out_history;
  out_history.open(HISTORY_FILE, fstream::out | fstream::trunc);
  if(! out_history.bad()) {
     HIST_ENTRY* he;
     
     int start_history = max(history_length - HISTORY_FILE_ENTRIES, 0);
     for(int i = start_history; i < history_length; i++) {
        he = history_get(i);
        if(he) {
           out_history << he->line << endl << endl;
        }
     }

     out_history.close();
  } else {
     cerr << "Error: could not write the SECONDO history file" << endl;
  }
#endif

  return (rc);
}

