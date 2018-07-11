/*
---- 
This file is part of SECONDO.

Copyright (C) 2006, University in Hagen, 
Faculty of Mathematics and  Computer Science, 
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


In this file, the pltty mode is defined.
Is the name of the mode suggest, this mode is a mix between the 
optimizer interface and the usual secondo interface. 

In this mode, four different kinds of commands are distinguished.

The first catefory of commands are internal commands. These commands are
handled directly by the user interface. Examples are '@<filename>', 'help' or
'quit'.

The second category include sql commands. These commands will be translated
into a plan using the sqlToPlan predicate defined in the optimizer. Before 
optimization, some rewritings are done within the command. For example, symbols
starting with a upper case letter are replaced by lower case letters. 
Furthermore, brackets are checked before optimization. If the optimization 
succeds, the generated plan is send to the kernel. The result is displayed 
using the kernel display functions.

The third category of commands are commands that are directly send to the 
kernel without aid of the optimizer. 

If a command cannot assigned to one of the categories mentioned before, 
it is tried to convert it into a callable prolog term and call it using the 
prolog engine. All generated solutions are printed out.

*/

/*
1 Includes

*/

#include <iostream> 
#include <stack>
#include "SWI-Prolog.h"
#include <stdlib.h>
#include "SecondoConfig.h"
#include "Application.h"
#include "TTYParameter.h"
#include <string>
#include "StringUtils.h"
#include "SecondoInterface.h"
#include "DisplayTTY.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>


#ifdef READLINE
  #include <stdio.h>
  #include <readline/readline.h>
  #include <readline/history.h>
  #define HISTORY_FILE ".secondopltty_history"
  #define HISTORY_FILE_ENTRIES 200
#endif

using namespace std;



string blanks = " \t\n\r";


/*
2 predicates

The ~predicates~ array contains all prolog extensions required by
Secondo. These things are defined in SecondoPL. 

*/

extern PL_extension predicates[];
extern void handle_exit(void);
extern bool StartSecondoC(TTYParameter& tp);

extern SecondoInterface* si;
NestedList* mnl;

bool globalAbort = false; // used if quit occurs within a script


namespace pltty{


// forward declaration of processCommands
bool processCommands(istream&, bool,bool);


/*
~isStdInput~

This boolean value describes whether the current input is cin or
a file.


*/

bool isStdInput = true;
string prompt = "";

void ShowPrompt( const bool first ) {
  prompt = first ? "SecondoPLTTY => ": "SecondoPLTTY -> ";
  #ifdef READLINE
    rl_set_prompt( prompt.c_str() );
  #else
    cout << prompt;
  #endif
}



char getChar(){
   struct termios old,n;
   char res;
   tcgetattr(fileno(stdin),&old);
   n=old;
   n.c_lflag &= ~ICANON;
   n.c_lflag &= ~ECHO;
   tcsetattr(fileno(stdin),TCSANOW,&n);
   res = getc(stdin);
   tcsetattr(fileno(stdin),TCSANOW,&old);
   return res;
}




/*
~isUpperCase~

Auxiliary function chinging whether an character is an upper case.

*/
bool isUpperCase(char c){
  return (c>='A') && (c <='Z');
}

/*
~tolower~

This function converts an upper case character into a lower case.

*/
char tolower(char c){
   if(!isUpperCase(c)) return c;
   return  (c + 'a') - 'A';
}


/*
~isIdentChar~

This functions checks whether a character is allowed to be part of
an identifier.

*/
bool isIdentChar(char c){
  return stringutils::isLetter(c) || stringutils::isDigit(c) || (c=='_');
}



string term2string(term_t t){
   string res ="<error>";
   fid_t fid = PL_open_foreign_frame();
   predicate_t pl_convert = PL_predicate("term_to_atom",2,"");
   term_t args = PL_new_term_refs(2);
   term_t r = args+1;
   PL_put_term(args, t);
   qid_t qid = PL_open_query(NULL, PL_Q_CATCH_EXCEPTION, pl_convert, args);
   if(!PL_next_solution(qid)){
     PL_close_query(qid);
     PL_discard_foreign_frame(fid);
     return res;
   }
   char* s;
   if( PL_get_atom_chars(r,&s)){
        res = string(s);
   } 
   PL_close_query(qid);
   PL_discard_foreign_frame(fid);
   return res;
}

/*
~checkSQLCommand~

This function checks the brackets within an command. 
The command itselfs is rewritten by converting symbols starting
with an upper case into lower case symbols. If brackets are 
not correct, the problem description is stored into errMsg and 
correct is set to false;


*/
string checkSQLCommand(const string& cmd, bool& correct, string& errMsg){
   stack<char> brackets;

   int state = 0; // start of a word

   // states
   // 0 begin of a symbol
   // 1 within a double quoted string
   // 2 within a single quoted string
   // 3 within a symbol starting with a capital
   // 4 within a symbol starting lower case
   stringstream res;

   size_t pos = 0;

   while(pos < cmd.size()){
     char c = cmd[pos];
     switch(state){ // somewhere
        case 0: {
          if(c=='"'){
             state = 1;
             res << c; 
          } else if(c=='\''){
             state = 2;
             res << c;
          } else if(stringutils::isLetter(c)){
               if(isUpperCase(c)){
                   state = 3;
                   res << tolower(c);
               } else {
                   state = 4;
                   res << c;
               }
          } else {
             if((c=='(') ||  c=='[' || c=='{'){
               brackets.push(c);
             } else if(c==')' || c==']' || c=='}'){
               if(brackets.empty()){
                 errMsg = string("found closing ") + c + " that was not opened";
                 correct = false;
                 return "";
               } 
               char open = brackets.top();
               brackets.pop();
               switch(c){
                   case ')' : if (open!='(') {
                              errMsg = string("closing ") + c 
                                       + " was opened by " + open;
                              correct= false;
                              return "";
                              }
                              break;
                   case ']' : if (open!='[') {
                                 errMsg = string("closing ") + c 
                                           + " was opened by " + open;
                                 correct = false;
                                 return "";
                              }
                              break;
                   case '}' : if (open!='}') {
                                 errMsg = string("closing ") + c 
                                          + " was opened by " + open;
                                 correct = false;
                                 return "";
                              }
                              break;
               } 
             } else if(c=='.') { // replace followed by a letter  by a ':'
                if(pos<cmd.size()-1){
                  if(stringutils::isLetter(cmd[pos+1])){
                     c = ':';
                  }
                }
             }
             res << c;
          }
          pos++;
          break;
        }
        case 1 : { // within an double quoted string
           res << c;
           if(c=='"'){
              state = 0;
           }
           pos++;
           break;
        }
        case 2: { // within a single quoted string
           res << c;
           if(c=='\''){
             state = 0;
           }
           pos++;
           break;
        } 
        case 3: // within an identifier to reqrite
            if(isIdentChar(c)){
               res << tolower(c);
               pos++;
            } else {
               state = 0;
            }
            break;
        case 4:
            if(isIdentChar(c)){ // whithin an normal identifier
               res << c;
               pos++;
            } else {
               state = 0;
            }
            break;
        }
     }
     // check whether all opened brackts are closed
     if(!brackets.empty()){
          correct = false;
          errMsg = "found unclosed brackets";
          return "";
     } 
     correct = true;
     errMsg = "";
     return res.str();
}


/*
~IsInternalCommand~

This function checks whether a command should be handeld internally.

*/

bool IsInternalCommand(const string& cmd){
  if(cmd=="quit") return true;
  if(cmd=="q") return true;
  if(cmd=="?") return true;
  if(cmd.size()>1){
    if(cmd[0]=='@'){
      return true;
    }
  }
  return false;
}

/*
~processQuit~

This function is called if a wuit command is executed.

*/
bool processQuit(){
   cout << "Thank you for using Secondo." << endl;
   globalAbort = true; 
   return true;
}

/*
~processHelp~

This function is the implementation of the help command.

*/
bool processHelp(){
  cout << endl
       << "secondo or optimizer command are executed as usual " << endl
       << "if an sql select statement ends with a dot, the user is " << endl
       << "ask whether the optimized query should be executed " << endl
       << "?        -  show this help" << endl
       << "@FILE    - read commands from file " << endl
       << "@@FILE   - read commands from file until an error occurred" << endl
       << "@\%FILE   - read commands from file ignoring pd comments" << endl
       << "@&FILE   - read commands from file ignoring pd comments "
       << "until an error occurred" << endl
       << "quit     - exits the program" << endl;
     return true;
}


/*
~processScript~

This command is called if the command starts with an @ mentioning the
execution of some script.

*/

bool processScript(const string& cmd){
   bool lastStdInput = isStdInput;
   bool isPD = false;
   bool haltOnErrors = false;
   int pos=1;
   char second = cmd[1];
   if(second=='@'){
      haltOnErrors = true;
      pos++;
   }else if(second=='%'){
      isPD = true;
      pos++;
   } else if(second=='&'){
      isPD=true;
      haltOnErrors = true;
      pos++;
   }  
   string filename = cmd.substr(pos);
   stringutils::trim(filename);
   ifstream in(filename.c_str());
   if(!in){
     cout << "Error: could not open file: " << filename << endl;
     return false;
   }

   isStdInput = false;
   bool res = processCommands(in,haltOnErrors, isPD);
   isStdInput = lastStdInput;

   if(!res){
     cout << "there was errors during processing " << filename << endl;
   } else {
     cout << "file " << filename << " successfull processed" << endl;
   }
   in.close();
   return res;
}


/*
~processInternalCommand~

This function is called if an internal command has been recognized.
It checks the kind of the internal command and calles the 
appropriate function.

*/
bool processInternalCommand(const string& cmd){
  if(cmd=="quit" || cmd=="q"){
     return processQuit();
  }
  if(cmd=="?"){
     return processHelp();
  }
  if(cmd.size()>1){
    if(cmd[0]=='@'){
      return processScript(cmd);
    }
  }
  cout << "Error: command " << cmd << " recognized as an internal "
       <<   "command but handled not correctly";
  return false;
}



/*
~getCommand~

This function extracts the next command from the input stream.
If the isPD flag is set to true, comments in PD-style are 
ignored.

*/
string getCommand(istream& in, bool isPD){
  bool complete = false;
  bool first = true;
  string line = "";
  string cmd = "";
  bool inPD = false;
  while (!complete && !in.eof() && !in.fail())
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
         getline( in, line );



    if ( line.length() > 0  || inPD) {
      if ( !isStdInput )           // Echo input if not standard input
      {
        cout << " " << line << endl;
      }
      bool comment = false;
      if(!isPD){
         comment = line[0] == '#';
      } else {
        if(!inPD){
          if(line.length()>0){
             comment = line[0] == '#';
          } 
          if(!comment) {
            if(line.length()>1){
              if((line[0]=='/') && (line[1]=='/')){ // single line comment
                 comment = true;
              } else if((line[0]=='/') && (line[1]=='*')) { // big comment
                 comment = true;
                 inPD = true;
              }
            }
          }
        } else {
          comment = true;
          if(line.length()>1){
            if( (line[0]=='*') && (line[1]=='/')){
              inPD = false;
              line = line.substr(2);
              stringutils::trim(line);
              comment = line.empty();
            }
          }
        }
     }

      if ( !comment )        // Process if not comment line
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
  return cmd ;
}



/*
3.1 Function ~isDirectSecondoCommand~

Returns true if the command starts with a secondo keyword,
a bracket or a curly bracket  

*/ 
bool isDirectSecondoCommand(const string& cmd){
   stringutils::StringTokenizer st(cmd, blanks, true);
   if(!st.hasNextToken()){
     return false;
   } 
   string first = st.nextToken();
   // simple, recognize using the first keyword
   if(    first=="query"   
       || first=="if"
       || first=="while"
       || stringutils::startsWith(cmd,"(") // nl command
       || stringutils::startsWith(cmd,"{") // command sequence
      ){
       return true;
    }
   // inquiries
   if(first=="list"){
     if(!st.hasNextToken()){
       return false;
     }
     string second = st.nextToken();
     if(   second=="operators" 
        || second=="algebras"
        || second=="databases"
        || second=="types"
        || second=="objects"){
        return !st.hasNextToken();
     }
     if(second=="type"){
        if(!st.hasNextToken()){
          return false;
        }
        string third = st.nextToken();
        if(third!="constructors"){
           return false;
        }
        return !st.hasNextToken();
     }
     if(second=="algebra"){
       if(!st.hasNextToken()){
           return false;
       }
       string third = st.nextToken();
       if(!stringutils::isIdent(third)){
         return false;
       }
       return !st.hasNextToken();
     }
     return false;
   }
   // transactions
   if(     first=="begin"
       ||  first=="commit"
       ||  first=="abort"){
      if(!st.hasNextToken()){
        return false;
      }
      string second = st.nextToken();
      if(second!="transaction"){
          return false;
      }
      return !st.hasNextToken();
   }
   // close database
   if(first=="close"){
      if(!st.hasNextToken()){
        return false;
      }
      string second = st.nextToken();
      if(second!="database"){
         return false;
      }
      return !st.hasNextToken();
   }
   // kill
   if(first=="kill"){
     if(!st.hasNextToken()){
        return false;
     }
     string second = st.nextToken();
     if(!stringutils::isIdent(second)){
        return false;
     }
     return !st.hasNextToken();
   }
   // save
   if(first=="save"){
     if(!st.hasNextToken()){
        return false;
     }
     string second = st.nextToken();
     if(!stringutils::isIdent(second)){
        // not that also 'database' is a valid identifier
        return false;
     }
     if(!st.hasNextToken()){
        return false;
     }
     if(st.nextToken()!="to"){
        return false;
     }
     // filename must be given
     return st.hasNextToken();
   }
   // restore
   if(first=="restore"){
      if(!st.hasNextToken()){
        return false;
      }
      string second = st.nextToken();
      if(!stringutils::isIdent(second)){
        return false;
      }
      // bring object restore and database restore to the same rest
      if(second=="database"){
         if(!st.hasNextToken()){
            return false;
         }
         second=st.nextToken();
         if(!stringutils::isIdent(second)){
           return false;
         }
      }
      // check from <file>
      if(!st.hasNextToken()){
        return false;
      }
      if(st.nextToken()!="to"){
        return false;
      }
      return st.hasNextToken(); // the filename
   }
   // delete variants
   if(first=="delete"){
      if(!st.hasNextToken()){
         return false;
      }
      string second = st.nextToken();
      if(!stringutils::isIdent(second)){
         return false;
      }
      if(!st.hasNextToken()){ // delete object
         return true; 
      }
      if(second!="database" && second != "type"){
          return false;
      }
      if(!st.hasNextToken()){ // type or database name
         return false;
      }
      string third = st.nextToken();
      if(!stringutils::isIdent(third)){
           return false;
      }
      return !st.hasNextToken();
   } 
   // create commands
   if(first=="create"){
      if(!st.hasNextToken()){
        return false;
      }
      string second = st.nextToken();
      if(!stringutils::isIdent(second)){
          return false;
      }
      if(!st.hasNextToken()){
          return false;
      }
      string third = st.nextToken();
      if(second=="database"){
          if(!stringutils::isIdent(third)){
            return false;
          }
          return !st.hasNextToken();
      }
      // object creattion
      if(third!=":"){
        return false;
      }
      return st.hasNextToken();
   }
   // let, derive & type
   if(first=="let" || first=="derive" || first=="type"){
       if(!st.hasNextToken()){
          return false;
       }
       string second = st.nextToken();
       if(!stringutils::isIdent(second)){
         return false;
       }
       if(!st.hasNextToken()){
           return false;
       } 
       string third = st.nextToken();
       if(third!="="){
          return false;
       }
       return st.hasNextToken();
   }
   // update
   if(first=="update"){
       if(!st.hasNextToken()){
          return false;
       }
       string second = st.nextToken();
       if(!stringutils::isIdent(second)){
         return false;
       }
       if(!st.hasNextToken()){
           return false;
       } 
       string third = st.nextToken();
       if(third!=":="){
          return false;
       }
       return st.hasNextToken();
   }
   // currently, no other secondo commands are known
   // extend here if this changed

   return false;

}


/*
~catalogChanges~

This funciton checks whether a direct secondo command manipulates 
the content of a database. In suche cases. the optimizer has to be
informed about this.

*/
bool catalogChanges(const string& cmd){
   stringutils::StringTokenizer st(cmd, blanks, true);
   if(!st.hasNextToken()){
     return false;
   } 
   string first = st.nextToken();
   if(   first=="if"      || stringutils::startsWith(cmd, "{") 
      || first=="kill"    || first =="delete" || first=="derive"
      || first=="create"  || first=="let" || stringutils::startsWith(cmd,"(")
      || first=="restore"){
     return true;
   }
   return false;
}


/*
~closeOptDB~

This function calles the closeDB predicate of the optimizer.

*/
bool closeOptDB(){
  fid_t fid = PL_open_foreign_frame();
  static predicate_t p;
  p=PL_predicate("closeDB",0,"");
  qid_t id;
  bool ok= false;
  try{
     term_t dummy = PL_new_term_ref();
     id = PL_open_query(NULL, PL_Q_NORMAL, p, dummy);
     if(PL_next_solution(id)){
       ok = true;
     }
     PL_close_query(id);
  } catch(...){
      ok = false;
  }
  PL_discard_foreign_frame(fid);
  return ok;
}



/*
~updateOptCatalog~

This function updates the optimizer's catalog.

*/
bool updateOptCatalog(){
   fid_t fid = PL_open_foreign_frame();
   static predicate_t p;
   p=PL_predicate("updateCatalog",0,"");
   qid_t id;
   bool ok= false;
   try{
       term_t dummy=PL_new_term_ref();
       id = PL_open_query(NULL, PL_Q_NORMAL, p, dummy);
       if(PL_next_solution(id)){
         ok = true;
       }
       PL_close_query(id);
    } catch(...){
        ok = false;
    }
    if(!ok){
      cout << "update optimizer catalog  failed" << endl;
    }
    PL_discard_foreign_frame(fid);
    return ok;
}


/*
~showResult~

Shows a nested list in a formatted manner.

*/
void showResult(ListExpr res){
   if(!mnl->HasLength(res,2)){
      mnl->WriteStringTo(res,cout);
   } else {
       if(!DisplayTTY::GetInstance().DisplayResult(mnl->First(res),
                         mnl->Second(res))){
         mnl->WriteStringTo(res,cout);
       }
   }
   cout << endl << endl;
}


bool isBlank(char c){
  for(size_t i=0;i<blanks.length();i++){
    if(c==blanks[i]) return true;
  }
  return false;
}


vector<pair<int,int> > detectSelects(const string& cmd, bool& ok){
   vector<pair<int,int> > res;
   // search for ( <blank>* select ... ) within cmd and 
   // returns the positions of the brackets within cmd
   // if several such constrvcts are present in cmd, all
   // occurrences will be returned
   size_t pos=0;
   size_t state = 0; 
       // 0  = outside select
       // 1  = inside double quoted string outside select
       // 2  = inside single quoted string outside select
       // 3  = after bracket outside select
       // 4  = after s
       // 5  = after e
       // 6  = after l
       // 7  = after e
       // 8  = after c
       // 9  = after t
       // 10 = select reached
       // 11 = inside double quoted string inside select
       // 12 = inside single quoted string inside select

   int open_brackets = 0; // number of open round brackets 
                         // within select

  int begin=-1;

   while(pos < cmd.length()){
      char c = cmd[pos];
      switch(state){
       case 0 : 
          switch(c){
             case '"'  : state = 1; break;
             case '\'' : state = 2; break;
             case '('  : state = 3; 
                         begin = pos;
                         break;
             // for all other cases, the state is not changed
          }
          pos++;
          break;
       case 1 :
           if(c=='"'){
             state = 0;
           }    
           pos++;
           break;
       case 2 :
           if(c=='\''){
             state = 0;
           }    
           pos++;
           break;
       case 3:
           if(c=='s'){
              state = 4;
              pos++;
           } else if(isBlank(c)){
              pos++;
           } else {
              state = 0;
           }
           break;
       case 4:
           if(c=='e'){
              state = 5;
              pos++;
           } else {
              state = 0;
           }
           break;
       case 5:
           if(c=='l'){
              state = 6;
              pos++;
           } else {
              state = 0;
           }
           break;
       case 6:
           if(c=='e'){
              state = 7;
              pos++;
           } else {
              state = 0;
           }
           break;
       case 7:
           if(c=='c'){
              state = 8;
              pos++;
           } else {
              state = 0;
           }
           break;
       case 8:
           if(c=='t'){
              state = 9;
              pos++;
           } else {
              state = 0;
           }
           break;
       case 9:
           if(isBlank(c)){
              state = 10;
              pos++;
           } else {
              state = 0;
           }
           break;

       case 10:
          switch(c){
             case '"'  : state=11; break;
             case '\'' : state=12; break;
             case '('  : open_brackets++; break;
             case ')' :
                 if(open_brackets==0){
                    // end of select reached
                    res.push_back(pair<int,int>(begin,pos));
                    state = 0;
                 } else {
                    open_brackets--;
                 }
          }
          pos++;
          break;
       case 11: 
            if(c=='"'){
              state=10;
            }
            pos++;
            break;
       case 12:
            if(c=='\''){
              state=10;
            }
            pos++;
            break;
       default : assert(false);
      }
   }
   ok = (state==0) &&  (open_brackets==0);
   return res;
}


string select2Secondo(const string& select, bool& correct){
   string errMsg;
   string cmd = checkSQLCommand("sql "+select,correct, errMsg);
   if(!correct){
     cout << "syntax error in command :" << errMsg << endl;
     return "";
   }
   fid_t fid = PL_open_foreign_frame();
   term_t a0 = PL_new_term_refs(2);
   PL_put_atom_chars(a0, (cmd).c_str());
   predicate_t p = PL_predicate("sqlToPlan",2,"");
   qid_t id;
   string plan="";
   int count =0;

   try{
      id = PL_open_query(NULL, PL_Q_CATCH_EXCEPTION, p, a0);
      if(PL_next_solution(id)){
        count++;
        char* res;
        if(PL_get_atom_chars(a0+1,&res)){
           string answer(res);
           if(stringutils::startsWith(answer,"::ERROR::")){
               errMsg = answer.substr(9);
               correct = false;
           } else {
                cmd =  answer; 
            }
         } else {
            correct =  false;
         }
       }
       PL_close_query(id);
    } catch(...){
        errMsg =  "Exception occurred";
        correct = false;
    }
    PL_discard_foreign_frame(fid);
    return cmd;
}


string rewriteSelects(const string& cmd, bool& ok){
   vector<pair<int,int> > selects = detectSelects(cmd,ok);
   if((selects.size()==0) || !ok){
     return cmd;
   }
   string res="";
   size_t posCmd = 0;
   size_t posSel = 0;
   while(posCmd <  cmd.length() && ok){
      int l = selects[posSel].first - posCmd;
      res += cmd.substr(posCmd, l);
      res += "( "+ select2Secondo(cmd.substr(selects[posSel].first + 1, 
              selects[posSel].second - (1 + selects[posSel].first)),ok)
             + " )";
      posCmd = selects[posSel].second + 1;
      posSel++;
      if(posSel==selects.size()){
          res += cmd.substr(posCmd);
          posCmd = cmd.length(); 
      }
   }
   if(ok){
      cout << endl;
      cout << "command after replacing embedded select clauses:" << endl;
      cout << res << endl << endl;
   }

   return res;
}


/*
~processDirectSecondoCommand~

This functions sends a command to the kernel and shows the
result.

*/
bool processDirectSecondoCommand(string cmd){
   SecErrInfo err;
   ListExpr resList;

  if(stringutils::startsWith(cmd,"close")){
      // redirect to close predicate
      bool ok = closeOptDB();
      if(ok){
         cout << "database successfully closed" << endl;
      } else {
         cout << "closing database failed" << endl;
      } 
      return ok;
   } else if(stringutils::startsWith(cmd,"(")){ 
       // assume command in nested list format
      ListExpr cmdList;
      if(!mnl->ReadFromString(cmd,cmdList)){
         cout << "Error in parsing command in nested list format";
         return false;
      }
      si->Secondo(cmdList, resList,err);
   } else {
       // usual secondo command without in user level syntax
       bool ok=true;
       cmd = rewriteSelects(cmd, ok);
       if(!ok){
         cout << "error during detecting embedded select clauses" << endl;
         return false;
       } 
       si->Secondo(cmd, resList, err);
   }
   if(err.code==0){
      // special treatment for optimizer
      if(catalogChanges(cmd)){
         cout << "Catalog has been changed" << endl;
         updateOptCatalog(); 
      }
      // output of result
      showResult(resList);
   } else {
      cout << "error in command " << cmd << endl;
      cout << "error code = " << err.code << endl;
      cout << err.msg << endl << endl;
   }
   return err.code==0;
}

/*
~show Error~

prints ou some error message.

*/
void showError(const string& s){
  cerr << s << endl;
}


/*
~getBindings~

Converts a bindings description into a vector of bindings/term pairs.

*/
bool getBindings( term_t bindings, 
                  vector<pair<string, term_t> > & res  ){

   
  // bindings should be a list of  ' = ( name, term )'
  if(!PL_is_list(bindings)){
     cerr << "bindings is not a list" << endl;
     return false;
  }

  while(!PL_get_nil(bindings)){
     term_t head = PL_new_term_ref();
     term_t tail = PL_new_term_ref();
     if(!PL_get_list(bindings, head, tail)){
        cerr << "problem in dividing list" << endl;
        return false;
     }
     bindings = tail;  // get the rest of the list
     int arity;
     term_t name  = PL_new_term_ref();
     if(!PL_get_name_arity(head, &name, &arity)){
       cerr << "name_arity failöed" << endl;
       return false;
     }
     if(arity!=2){
        cerr << "found invalid arity in binding" << endl;
        return false;
     }
     term_t a1 = PL_new_term_ref();
     if(!PL_get_arg(1, head,a1)){
       cerr << "get a1 failed" << endl;
       return false;
     }
     term_t a2 = PL_new_term_ref();
     if(!PL_get_arg (2,head,a2)){
        cerr << "get a2 failed" << endl;
        return false;
     }
     string s1;
     char* s;  
     if(PL_get_chars(a1,&s, CVT_ALL)){
        s1 = string(s);
     } else {
        cerr << "getting a1's name failed" << endl;
        return false;
     }
     pair<string,term_t> p(s1, a2);
     res.push_back(p);
  }
  return true;
 }



/*
3.5 ~display~

*/
bool display(term_t t) { 
   string st = term2string(t);
   cout << st;
   return true; 
}

/*
~processPrologCommand~

This function processes an arbitrary prolog question. 
It converts the string into a term and extract the variables
within the term.
After that, the predicate is called and for each existing 
solution, the values are printed out.

*/
bool processPrologCommand(const string& cmd){

   fid_t fid = PL_open_foreign_frame();

  // convert command into a string and get list of bindings
   predicate_t conv = PL_predicate("atom_to_term", 3,"");
   term_t a0 = PL_new_term_refs(3);
   term_t term = a0+1;
   term_t bindings = a0+2;
   PL_put_atom_chars(a0, cmd.c_str());

   if(!PL_call_predicate(NULL, PL_Q_NODEBUG, conv, a0)){
      cerr << "converting string to query failed" << endl;
      PL_discard_foreign_frame(fid);
      return false;
   }

   if(!PL_is_callable(term)){
      cerr << "Created term is not a query" << endl;
      return false;
   }
 
   // extract variables and there names 
   vector<pair<string, term_t> > b;
   if(!getBindings( bindings, b)){
      cerr << "problem in analyisng term" << endl;
      PL_discard_foreign_frame(fid);
      return false;
   }

   // try to avaluate the term using the call predicate
   predicate_t pl_call = PL_predicate("call",1,"");
   qid_t qid = PL_open_query(NULL, PL_Q_CATCH_EXCEPTION, pl_call, term);
   // print all solutions
   int count = 0;
   bool next = true; 
   while(next && PL_next_solution(qid)){
     if(count > 0){
        char k = getChar();
        next = k!='.';
        if(next){
          cout << ";" << endl;
        }
     }
     if(next){
       count++;
       for(size_t i=0;i<b.size();i++){
         cout << b[i].first << " = ";
         display(b[i].second);
         if(i<b.size()-1){
           cout << endl;
         }
       }
     }
   }
   if(b.size()>0){
      cout << ".";
   } 
   cout << endl;
   if(count==0){
     cerr << "no" << endl;;
   } else {
     cout << "computed " << count << " solution" << (count!=1?"s":"")<< endl;
   }

   PL_close_query(qid);

   PL_discard_foreign_frame(fid);
   return true;
  PL_discard_foreign_frame(fid);
  return true;
}


/*

~processSqlCommand~

This function rewrites the command by replacing upper cases to lower cases
and sends the result to the optimizers ~sqlToPlan~ predicate. If this
was successful, the result is sent to the kernel and the result of this
call is displayed.

*/

bool processSqlCommand(string& cmd, bool isDot){
   bool correct;
   string errMsg;
   cmd = checkSQLCommand(cmd,correct, errMsg);
 
   if(!correct){
     cout << "syntax error in command :" << errMsg << endl;
     return false;
   } 

   stringutils::StringTokenizer st(cmd,blanks,true);
   string s = st.nextToken(); // always sql
   s = st.nextToken();
   if(s!="select"){
     // handle only select statements using sqlToPlan
     // all other kinds of sql statements are directly
     // handled by the optimizer
     return processPrologCommand(cmd);
   }

    fid_t fid = PL_open_foreign_frame();
    term_t a0 = PL_new_term_refs(3);
    PL_put_atom_chars(a0, (cmd).c_str());
    predicate_t p = PL_predicate("sqlToPlan",3,"");
    qid_t id;
    bool ok= true;
    string plan="";
    string costs="";
    int count =0;

    try{
       id = PL_open_query(NULL, PL_Q_CATCH_EXCEPTION, p, a0);
       if(PL_next_solution(id)){
         count++;
         char* res;
         if(PL_get_atom_chars(a0+1,&res)){
            string answer(res);
            if(stringutils::startsWith(answer,"::ERROR::")){
                errMsg = answer.substr(9);
               ok = false;
            } else {
                plan = "query "+ answer;
                costs = term2string(a0+2); 
            }
         } else {
            ok = false;
         }
       }
       PL_close_query(id);
    } catch(...){
        errMsg =  "Exception occurred";
        ok = false;
    }
    PL_discard_foreign_frame(fid);


    if(!ok){
      cout << "error in optimization" << endl;
      if(errMsg.size()>0){
         showError(errMsg);
      }
      return false;
    }
    if((count !=1)){
       cout << "found no solution in optimization" << endl;
       return false;
    }
    cout << endl;
    cout << "Optimized plan is:" << endl << plan << endl;
    cout << endl << "Estimated Costs are:" << endl << costs << endl << endl;

    if(isStdInput && isDot){
      char k=' ';
       do{
          cout << "execute query ? (y/n) ";
          k = getChar();
          k = tolower(k);
       } while((k!='n') && (k!='y'));
       cout << endl;
       if(k=='n'){
          return true;
       }      
    }

    return processDirectSecondoCommand(plan);
};


/*
~isSqlCommand~

This function checks whether a command is a sql command.

*/

bool isSqlCommand(string& cmd){
   string cmdcopy = cmd;
   stringutils::trim(cmdcopy);
   stringutils::toLower(cmdcopy);
   stringutils::StringTokenizer st(cmdcopy, blanks, true);
   if(!st.hasNextToken()){
     return false;
   }
   string first = st.nextToken();
   // complete sql command
   if(first=="sql"){
     return true; // no change required
   }
   // without sql, first token is sufficient
   if(first == "drop"){
      cmd = "sql " + cmd;
      return true;
   }
   if(first == "select"){
      cmd = "sql " + cmd;
      return true;
   }
   // next token required
   if(!st.hasNextToken()){
      return false;
   }
   string second = st.nextToken();
   if(first=="delete" && second=="from"){
      cmd = "sql " + cmd;
      return true;
   }
   if(first=="insert" && second=="into"){
      cmd = "sql " + cmd;
      return true;
   }
   if(first=="create" && second=="table"){
      cmd = "sql " + cmd;
      return true;
   }
   if(first=="create" && second=="index"){
      cmd = "sql " + cmd;
      return true;
   }
   // third token required
   if(st.hasNextToken()){
     return false;
   }
   string third = st.nextToken();
   if(first=="update" && third=="set"){
      cmd = "sql " + cmd;
      return true;
   }
   return false; 
}



/*
~processCommand~

This function determines the kind of the command and calles the
appropriate command handler.

*/
bool processCommand(string& cmd){
  stringutils::trim(cmd);
  bool  isDot = !cmd.empty() && (cmd[cmd.length()-1]=='.');
  if(IsInternalCommand(cmd)){
     // cout << "internal command recognized" << endl;
     return processInternalCommand(cmd);
  } else if (isSqlCommand(cmd)) {
     // cout << "sql command recognized" << endl;
     return processSqlCommand(cmd,isDot);
  } else if(isDirectSecondoCommand(cmd)){
     // cout << "direct secondo command recognized" << endl;
     return processDirectSecondoCommand(cmd);
  } else {
     // cout << "general prolog command recognized" << endl;
     return processPrologCommand(cmd);
  }
}


/*
~processCommands~

This function extracts commands from a stream and executes them.
If haltOnErrors is true, the processing is stoped immediately
if an error is occured. If the flag ~pdstyle~ is set, comments
in pd style in the  stream are ignored during processing.

*/
bool  processCommands(istream& in, bool haltOnErrors, bool pdstyle ){
  string cmd = "";
  bool error = false;
  bool stop = false;
  while((cmd!="quit") && (cmd!="q") && !globalAbort && !stop && !in.eof()){
    cmd = getCommand(in, pdstyle);
    stringutils::trim(cmd);
    if(cmd.length()>0){
       if(!processCommand(cmd)){
         cout << "Error occurred during command '" <<  cmd << "'" << endl;
         error = true;
         if(haltOnErrors){
           stop = true;
         }       
       }
    }
  }
  return !error;
}


} // end of namespace pltty


/*
The function ~secondo_completion~  enables 
tab extension if the readline library is used.

*/
#ifdef READLINE
extern char** secondo_completion(const char* text, int start, int end);
#endif


/*
2 SecondoPLTTYMode

This function is the ~main~ function of SecondoPLTTY.

*/


int SecondoPLTTYMode(TTYParameter& tp)
{
  atexit(handle_exit);

  if( !StartSecondoC(tp) )
  {
    cout << "Usage : SecondoPL [Secondo-options] [Prolog-options]" 
         << endl;
    exit(1);
  }

#ifdef READLINE
  string historyFile;
  rl_initialize();
  rl_readline_name = "secondopl";
  rl_attempted_completion_function = secondo_completion;
  //rl_attempted_completion_function = secondo_completion;
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

  /* Start PROLOG interpreter with our extensions. */
  PL_register_extensions(predicates);

  /* initialize the PROLOG engine */

  int argc = 0;
  char** argv = tp.Get_plargs(argc);

  cerr << endl <<__FILE__ << ":" << __LINE__ 
       << " Calling PL_initialize with ";    

  for (int i = 0; i < argc; i++) {
    cerr << argv[i] << " ";    
  }    
  cerr << endl << endl;

  if( !PL_initialise(argc,argv) ) 
  {
    PL_halt(1);
  }
  else
  {
      {
      // VTA - 15.11.2005
      // I added this piece of code in order to run with newer versions
      // of prolog. Without this code, the libraries (e.g. list.pl) are
      // not automatically loaded. It seems that something in our code
      // (auxiliary.pl and calloptimizer.pl) prevents them to be 
      // automatically loaded. In order to solve this problem I added
      // a call to 'member(x, []).' so that the libraries are loaded 
      // before running our scripts.
      term_t t0 = PL_new_term_refs(2),
             t1 = t0+1;
      PL_put_atom_chars(t0, "x");
      if(!PL_put_list_chars(t1, "")){
        assert(false);
      }
      predicate_t p = PL_predicate("member",2,"");
      PL_call_predicate(NULL,PL_Q_NORMAL,p,t0);
      // end VTA 
      }   

     /* load the auxiliary and calloptimizer */
     term_t a0 = PL_new_term_refs(1);
     static predicate_t p = PL_predicate("consult",1,"");
     PL_put_atom_chars(a0,"auxiliary");
     PL_call_predicate(NULL,PL_Q_NORMAL,p,a0);
     PL_put_atom_chars(a0,"calloptimizer");
     PL_call_predicate(NULL,PL_Q_NORMAL,p,a0);
     /* switch to prolog-user-interface */
  }

  mnl = si->GetNestedList();
  //DisplayTTY::Set_SI(si);
  DisplayTTY::Set_NL(mnl); 
  NList::setNLRef(mnl);
  DisplayTTY::Initialize();
  bool ok;
  if(tp.iFileName.length()==0){
      ok =  pltty::processCommands(cin, false, false);
  } else {
      ok = pltty::processScript("@"+tp.iFileName);
  }

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

  DisplayTTY::Finish();

  PL_halt(0);

  return ok?0:1;

}


