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

using namespace std;
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
  #define HISTORY_FILE_ENTRIES 100
#endif

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


class SecondoTTY : public Application 
{
 public:
  SecondoTTY( const TTYParameter& t );
  virtual ~SecondoTTY() {};
  bool AbortOnSignal( int sig );
  void Usage();
  void ProcessFile( const string& fileName );
  void ProcessCommand();
  void ProcessCommands();
  int  Execute();
  void ShowPrompt( const bool first );
  void TypeOutputList ( ListExpr list );
  void TypeOutputListFormatted ( ListExpr list );
  bool IsInternalCommand( const string& line );
  bool GetCommand();
  void ShowQueryResult( ListExpr list );
  ListExpr CallSecondo();
  void CallSecondo2();
 private:
  string parmFile;
  string user;
  string pswd;
  string host;
  string port;
  string iFileName;
  string oFileName;

  string            cmd;
  string            prompt;
  bool              isStdInput;
  bool              quit;
  NestedList*       nl;
  bool              isQuery;
  SecondoInterface* si;
};

SecondoTTY::SecondoTTY( const TTYParameter& t )
  : Application( t.numArgs, (const char**)t.argValues )
{
  parmFile = t.parmFile;
  user = t.user;
  pswd = t.pswd;
  host = t.host;
  port = t.port ;
  iFileName = t.iFileName;
  oFileName = t.oFileName;

  isStdInput    = true;
  quit          = false;
  nl            = 0;
  si            = 0;
}

bool
SecondoTTY::AbortOnSignal( int sig )
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
  "  @{FILE}  - read commands from file 'FILE' (may be nested)\n" <<
  "  DEBUG n  - set debug level to n, with\n" <<
  "             0: debug and trace turned off\n" <<
  "             1: debug mode (show annotated query and operator tree)\n" <<
  "             2: debug and trace (show type mapping and rec. calls)\n" <<
  "             3: trace more (construction of nodes of the op. tree,\n" <<
  "                and execution of the query processors Eval method)\n" <<
  "  Q, QUIT  - exit the program\n" <<
  "  # ...    - comment line (first character on line has to be '#')\n" <<
  "\n" <<
  "Additionally you may enter any valid SECONDO command. Internal\n" <<
  "commands are restricted to ONE line, while SECONDO commands may\n" << 
  "span several lines; a semicolon as the last character on a line ends a\n" <<
  "command, but is not part of the command, alternatively you may enter\n" <<
  "an empty line.";

  cout << cmdList.str() << endl << endl;
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
  transform( cmdWord.begin(), cmdWord.end(), cmdWord.begin(),
             ToUpperProperFunction );

  if ( cmdWord == "?" || cmdWord == "HELP" )
  {
    Usage();
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
    cout << "*** Thank you for using SECONDO!" << endl << endl;
    quit = true;
  }
  else if ( cmdWord[0] == '@' )
  {
    ProcessFile( cmd.substr( 1, ( cmd.length() - 1 ) ) );
  }
  else
  {
    isQuery = (    cmdWord == "QUERY" 
                || cmdWord == "(QUERY" 
                || cmdWord == "( QUERY" );

    CallSecondo2();
  }
}

void
SecondoTTY::ShowPrompt( const bool first )
{
  if ( isStdInput ) // Display input prompt
  {
    if ( first )
    {
      prompt = "Secondo => ";
      #ifdef READLINE
         rl_set_prompt(prompt.c_str());
      #else
         cout << prompt;  // First line of command
      #endif
    }
    else
    {
      prompt = "Secondo -> ";
      #ifdef READLINE
         rl_set_prompt(prompt.c_str());
      #else
         cout <<  prompt; // Continuation line of command
      #endif
    }
  }
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
      if(isStdInput)
         line = string(readline(prompt.c_str()));
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
  unsigned int end = cmd.find_last_not_of(" \t");
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

void
SecondoTTY::ProcessCommands()
{
  while (!cin.eof() && !quit)
  {
    if ( GetCommand() )
    {
      try {
        ProcessCommand();
      } 
      catch (SecondoException e) {
        cerr << "Exception caught: " << e.msg() << endl;
      }  
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
11 CallSecondo

This function gives a query to secondo and receives the result from secondo.

*/

ListExpr
SecondoTTY::CallSecondo()
{
  int errorCode = 0, errorPos = 0;
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

This would normally be  the  main function of SecondoTTY.

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
    streambuf* oldInputBuffer  = 0;
    streambuf* oldOutputBuffer = 0;
    ifstream fileInput;
    ofstream fileOutput;

    si = new SecondoInterface();
    
     
    if ( si->Initialize( user, pswd, host, port, parmFile ) )
    {
    // Add message handlers
    MessageCenter* msg = MessageCenter::GetInstance();
    
    // uncomment the lines below in order to 
    // activate the example handler. The operator
    // count2 demonstrates how to send messages.
    SimpleHandler* sh = new SimpleHandler();
    msg->AddHandler(sh);

     
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
      NList::setNLRef(nl);
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
char* keywords[] = { "abort", "algebra", "algebras", "begin", "commit",
                     "close", "constructors", "consume","count", "create",
                     "database", "databases", "DEBUG", "delete",
                     "extend", "feed", "filter", "from",  "let", "list",
                     "objects", "open", "operators", "query",
                     "restore", "save", "SHOW", "transaction", "type",
                     "types", "update","SEC2TYPEINFO","SEC2OPERATORUSAGE",
                     "SEC2OPERATORINFO","SEC2FILEINFO","SEC2COUNTERS",
                     "SEC2COMMANDS","SEC2CACHEINFO",
                     (char *)0 };


/*
~dupstr~

This fucntion returns a clone of the argument string;

*/
char *
dupstr (char* s)
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
   char* name;
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
  rl_initialize();
  rl_readline_name = "secondo";
  rl_attempted_completion_function = secondo_completion;
  /* read the history from file */
  ifstream hist_file(HISTORY_FILE);
  string histline;
  if(hist_file){
    while(!hist_file.eof()){
       getline(hist_file,histline);
       if(histline.find_last_not_of(" \t\n")!=string::npos){
          add_history(histline.c_str());
       }
    }
    hist_file.close();
  }
#endif
  int rc = appPointer->Execute();
  delete appPointer;
#ifdef READLINE
  /* save the last xxx enties in the history to a file */
  ofstream out_history(HISTORY_FILE,ofstream::out | ofstream::trunc );
  if(out_history){
     int start_history = history_length-HISTORY_FILE_ENTRIES;
     if(start_history <0) start_history=0;
     HIST_ENTRY* he;
     for(int i=0;i<history_length;i++){ // ignore quit
        he = history_get(i);
        if(he)
           out_history << he->line << endl;
     }
     out_history.close();
  }
#endif
  return (rc);
} 




