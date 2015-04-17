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

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctype.h>

#ifdef READLINE
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
#ifndef SECONDO_CLIENT_SERVER
  #include "SecondoInterfaceTTY.h"
#else
  #include "SecondoInterfaceCS.h"
#endif

#include "SecondoSMI.h"
#include "NestedList.h"
#include "DisplayTTY.h"
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
  bool ProcessFile( const string& fileName, const bool stopOnError );
  bool ProcessCommand();
  bool ProcessCommands( const bool stopOnError);
  void ShowPrompt( const bool first );
  void TypeOutputList ( ListExpr list );
  void TypeOutputListFormatted ( ListExpr list );
  bool IsInternalCommand( const string& line );
  bool GetCommand();
  void ShowQueryResult( ListExpr list );
  ListExpr CallSecondo();
  void CallSecondo2();
  bool MatchQuery(string& w, istringstream& is) const;
  string ReadCommand(istringstream& is) const;

  string parmFile;
  string user;
  string pswd;
  string host;
  string port;
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
#ifndef SECONDO_CLIENT_SERVER
  SecondoInterfaceTTY* si;
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
  user = t.user;
  pswd = t.pswd;
  host = t.host;
  port = t.port ;
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
  "  @FILE    - read commands from file 'FILE' (may be nested)\n" <<
  "  @@FILE   - read commands from file 'FILE' until 'FILE' is " <<
  "completly processed or an error is occurred\n" <<
  "  DEBUG n  - set debug level to n where n is an integer where each " <<
  "bit corresponds to one setting:\n" <<
  "           bit  0: debug mode (show annotated query and operator tree)\n" <<
  "           bit  1: trace (show type mapping and recursive calls)\n" <<
  "           bit  2: trace nodes (construction of nodes of the op. tree,\n" <<
  "                   and execution of the query processor's Eval() method)\n"<<
  "           bit  3: localInfo (prints a warning if an operator did not\n" <<
  "                   destroy its localinfo before the operator tree\n"<<
  "                   is deconstructed)\n" <<
  "           bit  4: debug progress (after sending a REQUESTPROGRESS\n " <<
  "                   message to an operator, the ranges in the \n" <<
  "                   ProgressInfo are checked for whether tey are\n" <<
  "                   reasonable. If not so, the according operator and \n" <<
  "                   ProgressInfo are reported) \n" <<
  "           bit  5: trace progress (prints the result of \n " <<
  "                   each REQUESTPROGRESS message) \n" <<
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
SecondoTTY::ProcessFile( const string& fileName , const bool stopOnError)
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

    bool res = ProcessCommands(stopOnError);

    cout << "Runtime for " << fileName << ": "
         << scriptTime.diffTimes() << endl;

    isStdInput = saveIsStdInput;
    if(!res){
      cout << "Errors during processing the file "  << fileName << "."<< endl; 
    } else {
      cout << "File " << fileName << " successful processed." << endl;
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
             ToUpperProperFunction );
  return cmdWord;
}

bool
SecondoTTY::MatchQuery(string& cmdWord, istringstream& is) const
{
  bool isQuery = false;
  size_t pos = cmd.find("query");

  if (pos == string::npos)
    return isQuery;

  if ( cmdWord == "QUERY" )
  {
    isQuery = true;
  }
  else
  {
    if ( cmdWord == "(" )
    {
      cmdWord = ReadCommand(is);
      if (cmdWord == "QUERY")
        isQuery = true;
      else
        isQuery = false;
    }
  }
  return isQuery;
}



bool SecondoTTY::ProcessCommand()
{
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
    if(cmdWord.length()>1 && cmdWord[1] == '@'){
      start = 2;
      stopOnError = true;
    }
    success = ProcessFile( cmd.substr( start, ( cmd.length() - start ) ),
                           stopOnError );
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

  #ifdef READLINE
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
             ToUpperProperFunction );

  return ( cmdWord == "?" || cmdWord == "HELP"        ||
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
    #ifdef READLINE
      if(isStdInput){
         char* cline = readline(prompt.c_str());
         line = string(cline);
         free(cline);
      }
      else
    #endif
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
          cmd = cmd + "\n" + line + " ";
        }
      }
    }
    else                           // Empty line ends command
    {
      complete = cmd.length() > 0;
      first = true;
    }
  }
  // remove spaces from the end of cmd
  size_t end = cmd.find_last_not_of(" \t");
  if(end==string::npos)
     end = 0;
  else
     end += 1;
  cmd = cmd.substr(0,end);
  #ifdef READLINE
     if(complete && (cmd.length()>0) && isStdInput){
        // get the last entry from the history if avaiable
        int noe = history_length;
        string last = "";
        if(noe){
           HIST_ENTRY* he = history_get(noe);
           if(he)
               last = string(he->line);
        }
        if(last!=cmd && (cmd.find_last_not_of(" \t\n")!=string::npos))
          add_history(cmd.c_str());
       }
  #endif
  return (complete);
}

bool
SecondoTTY::ProcessCommands(const bool stopOnError)
{
  bool errorFound = false;;
  while (!cin.eof() && !quit && ( !stopOnError || !errorFound ))
  {
    if ( GetCommand() )
    {
      try {
        if( ! ProcessCommand()){
           errorFound = true;
        }
      }
      catch (SecondoException e) {
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
8 TypeOutputListFormatted

TypeOutputList prints the result of a nonquery input (e. g. list) in formatted
manner.

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
    DisplayTTY::GetInstance().DisplayResult2( list );
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

  if ( cmd[cmd.find_first_not_of(" \n\r\t\v\b\a\f")] == '(' )
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
           nl->SymbolValue(nl->First(result)) == "inquiry"  )
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

  #ifndef SECONDO_CLIENT_SERVER
  si = new SecondoInterfaceTTY( false);
  #else
  si = new SecondoInterfaceCS(true);
  #endif
  string errorMsg("");
  if ( si->Initialize( user, pswd, host, port, parmFile, errorMsg  ) )
  {

    // initialize the pointer to the nested list instance
    nl = si->GetNestedList();
    NList::setNLRef(nl);
    DisplayTTY::Set_SI( si );
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
      cout << endl << "Secondo TTY ready for operation." << endl
           << "Type 'HELP' to get a list of available commands." << endl;
      ProcessCommands( false);
    }
    else
    {
      ProcessFile(iFileName, false);
    }

    if ( useOutputFile ){
      cout.rdbuf( oldOutputBuffer );
    }
    DisplayTTY::Finish();
  } else {
    cerr << "Error in initializing the secondo system: "
         << errorMsg << endl;
  }

  try {
    si->Terminate();
    delete si;
    si = 0;
  }
  catch (SecondoException e)
  {
     cerr << e.msg() << endl;
     rc = 17;
  }
  return (rc);
}


#ifdef READLINE
/*
15 Keyword extraction

~commands~

This array contains all keywords used in SECONDO for expansion with
the tab key.
Duplicates are not allowed in this array and the last entry has to
be NULL.

*/
const char* keywords[] = { "abort", "algebra", "algebras", "begin", "commit",
                     "close", "constructors", "consume","count", "create",
                     "database", "databases", "DEBUG", "delete", "derive",
                     "else","endif","endwhile","extend", "feed", "filter",
                     "from", "if", "kill", "let", "list","objects", "open",
                     "operators", "query","restore", "save", "SHOW", "then",
                     "transaction", "type","types", "update","while",
                     "SEC2TYPEINFO","SEC2OPERATORUSAGE","SEC2OPERATORINFO",
                     "SEC2FILEINFO","SEC2COUNTERS","SEC2COMMANDS",
                     "SEC2CACHEINFO",
                     (char *)0 };


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
#ifdef READLINE
  string historyFile;
  rl_initialize();
  rl_readline_name = "secondo";
  rl_attempted_completion_function = secondo_completion;
  /* read the history from file */
  ifstream hist_file(HISTORY_FILE);
  string histline;
  fstream out_history;
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
    historyFile = FileSystem::GetCurrentFolder();
    FileSystem::AppendItem(historyFile, HISTORY_FILE);
  } else {
    out_history.open(HISTORY_FILE, ios::out);
  }
#endif
  int rc = appPointer->Execute();
  delete appPointer;
#ifdef READLINE
  /* save the last xxx enties in the history to a file */
  out_history.open(historyFile.c_str(), ios::out );
  out_history.seekg(0,ios::end);
  if(out_history){
     int start_history = history_length-HISTORY_FILE_ENTRIES;
     if(start_history <0) start_history=0;
     HIST_ENTRY* he;
     out_history.seekg(0,ios::beg);
     for(int i=start_history;i<history_length;i++){ // ignore quit
        he = history_get(i);
        if(he){
           out_history << he->line << endl << endl;
        }
     }
     out_history.flush();
     out_history.close();
  } else {
    cerr << "Error: could not write the history file" << endl;
  }
#endif
  return (rc);
}




