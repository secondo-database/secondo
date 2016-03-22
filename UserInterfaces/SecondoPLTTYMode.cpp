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


#ifdef READLINE
  #include <stdio.h>
  #include <readline/readline.h>
  #include <readline/history.h>
  #define HISTORY_FILE ".secondopl_history"
  #define HISTORY_FILE_ENTRIES 200
#endif

using namespace std;


/*
2 predicates

The ~predicates~ array contains all prolog extensions required by
Secondo.

*/

extern PL_extension predicates[];
extern void handle_exit(void);
//extern char* GetConfigFileNameFromArgV(int&, char**);
//extern char* GetParameterFromArg(int&, char**,string,string);
extern bool StartSecondoC(TTYParameter& tp);

extern SecondoInterface* si;
NestedList* mnl;




bool isStdInput = true;
string prompt = "";

void ShowPrompt( const bool first ) {

  prompt = first ? "SecondoPL => ": "SecondoPL -> ";

  #ifdef READLINE
    rl_set_prompt( prompt.c_str() );
  #else
    cout << prompt;
  #endif
}




bool IsInternalCommand(const string& cmd){
  if(cmd=="quit") return true;
  if(cmd=="q") return true;
  if(cmd=="?") return true;
  return false;
}

void processInternalCommand(const string& cmd){
  if(cmd=="quit" || cmd=="q"){
     cout << "Thank you for using Secondo." << endl;
     return;
  }
  if(cmd=="?"){
     cout << endl
          << "enter commands " << endl
          << "quit for quit the program" << endl;
     return;
  }
  assert(false); // unhandlet internal command
}




string getCommand(bool isPD){
  bool complete = false;
  bool first = true;
  string line = "";
  string cmd = "";
  bool inPD = false;
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




bool isDirectSecondoCommand(const string& cmd){
   stringutils::StringTokenizer st(cmd, " \n\r\t");
   if(!st.hasNextToken()){
     return false;
   } 
   string first = st.nextToken();
   return first=="query" || first=="while";  // to be continued
}

void showResult(ListExpr res){
   if(!mnl->HasLength(res,2)){
      mnl->WriteStringTo(res,cout);
   } else {
       DisplayTTY::GetInstance().DisplayResult(mnl->First(res),
                         mnl->Second(res));
   }
   cout << endl << endl;
}


bool processDirectSecondoCommand(const string& cmd){
   SecErrInfo err;
   ListExpr resList;
   si->Secondo(cmd, resList, err);   
   if(err.code==0){
      showResult(resList);
   } else {
      cout << "error in command " << cmd << endl;
      cout << "error code = " << err.code << endl;
      cout << err.msg << endl << endl;
   }
   return err.code==0;
}

bool isUpperCase(char c){
  return (c>='A') && (c <='Z');
}

char tolower(char c){
   if(!isUpperCase(c)) return c;
   return  (c + 'a') - 'A';
}



bool isIdentChar(char c){
  return stringutils::isLetter(c) || stringutils::isDigit(c) || (c=='_');
}


string checkPrologCommand(const string& cmd, bool& correct, string& errMsg){
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
     switch(state){
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
             }
             res << c;
          }
          pos++;
          break;
        }
        case 1 : {
           res << c;
           if(c=='"'){
              state = 0;
           }
           pos++;
           break;
        }
        case 2: {
           res << c;
           if(c=='\''){
             state = 0;
           }
           pos++;
           break;
        } 
        case 3:
            if(isIdentChar(c)){
               res << tolower(c);
               pos++;
            } else {
               state = 0;
            }
            break;
        case 4:
            if(isIdentChar(c)){
               res << c;
               pos++;
            } else {
               state = 0;
            }
            break;
        }
     }
     if(!brackets.empty()){
          correct = false;
          errMsg = "found unclosed brackets";
          return "";
     } 
     correct = true;
     errMsg = "";
     return res.str();
}


void processOpenCommand(string rest){

    term_t a0 = PL_new_term_refs(2);
    static predicate_t p;
    p=PL_predicate("open",1,"");
    PL_put_atom_chars(a0, rest.c_str());
    qid_t id;
    bool ok= false;
    try{
       id = PL_open_query(NULL, PL_Q_NORMAL, p, a0);
       if(PL_next_solution(id)){
         ok = true;
       }
       PL_close_query(id);
    } catch(...){
        ok = false;
    }
    if(ok){
      cout << "opening database successful" << endl;
    } else {
      cout << "opening database failed" << endl;
    }
  
}

void processSqlCommand(string cmd){
 
    term_t a0 = PL_new_term_refs(2);
    PL_put_atom_chars(a0, (cmd).c_str());
    predicate_t p = PL_predicate("sqlToPlan",2,"");
    qid_t id;
    bool ok= true;
    string plan="";
    int count =0;
    try{
       id = PL_open_query(NULL, PL_Q_CATCH_EXCEPTION, p, a0);
       while(PL_next_solution(id)){
         count++;
         char** res= new char*;
         if(PL_get_atom_chars(a0+1,res)){
            cout << "got plan at address" << res << endl;
            plan = "query "+ string(*res); 
         } else {
            ok = false;
         }
       }
       PL_close_query(id);
    } catch(...){
        cout << "Exception occurred" << endl;
        ok = false;
    }
    if(!ok){
      cout << "error in optimization" << endl;
      return;
    }
    if((count !=1)){
       cout << "found more than one solution in optimization" << endl;
       cout << "number of solutions " << count << endl;
       return;
    }
    cout << "optimized plan  is "  << plan << endl;
    processDirectSecondoCommand(plan);
 
};

void processGeneralCommand(const string& cmd){
   cout << "general commands are not supported yet" << endl;

}

void processPrologCommand(string cmd){
  bool ok;
  string errMsg;
  stringutils::trim(cmd);
  stringutils::StringTokenizer st(cmd, " \t\n\r");
  if(!st.hasNextToken()){
    cout << "empty command found" << endl;
    return;
  }

  string keyword = st.nextToken();

  if(keyword=="sql"){
    cmd = checkPrologCommand(cmd,ok, errMsg);
    if(!ok){
      cout << "Error in command " << errMsg << endl;
      return;
    }
  }

  // handle some special commands
  if(keyword=="open"){
     processOpenCommand(st.getRest());
  } else if(keyword=="sql"){
     processSqlCommand(cmd);
  } else {
     processGeneralCommand(cmd);
  }
}

void processCommand(const string& cmd){
  if(IsInternalCommand(cmd)){
     processInternalCommand(cmd);
  } else if(isDirectSecondoCommand(cmd)){
     processDirectSecondoCommand(cmd);
  } else {
     processPrologCommand(cmd);
  }
}



int processCommands(){
  string cmd = "";
  while((cmd!="quit") && (cmd!="q")){
    cmd = getCommand(false);
    processCommand(cmd);
  }
  return 0;
}


/*
2 SecondoPLMode

This function is the ~main~ function of SecondoPL.

*/

int
SecondoPLMode(TTYParameter& tp)
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
      int t = PL_put_list_chars(t1, "");
      cout << t;
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
  DisplayTTY::Set_SI(si);
  DisplayTTY::Set_NL(mnl); 
  DisplayTTY::Initialize();
  int rc =  processCommands();

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

  return rc;


}


