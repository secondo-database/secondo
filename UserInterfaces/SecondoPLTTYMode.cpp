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

string blanks = " \t\n\r";


/*
2 predicates

The ~predicates~ array contains all prolog extensions required by
Secondo.

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
  if(cmd.size()>1){
    if(cmd[0]=='@'){
      return true;
    }
  }
  return false;
}


bool processQuit(){
   cout << "Thank you for using Secondo." << endl;
   globalAbort = true; 
   return true;
}

bool processHelp(){
  cout << endl
       << "secondo or optimizer command are executes as usual " << endl
       << "?        -  show this help" << endl
       << "@FILE    - read commands from file " << endl
       << "@@FILE   - read commands from file until error " << endl
       << "@\%FILE   - read commands from file ignoring pd comments" << endl
       << "@&FILE   - read commands from file ignoring pd comments "
       << "until error" << endl
       << "quit for quit the program" << endl;
     return true;
}

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
   }
   in.close();
   return res;
}



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
       <<   "command but handlet not correctly";
  return false;
}




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
   if(   first=="query"   || first=="while"   || first =="list"
      || first=="begin"   || first=="commit"  || first=="abort"
      || first=="save"    || first=="if"      
      || stringutils::startsWith(cmd, "{") 
      || first=="kill"    || first =="delete" || first=="derive"
      || first=="create"  || first=="let"     || first=="close" 
      || first=="restore" || first=="save"    
      || stringutils::startsWith(cmd, "(")){
     return true;
   }
   return false;
}

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

bool processDirectSecondoCommand(const string& cmd){
   SecErrInfo err;
   ListExpr resList;

  if(stringutils::startsWith(cmd,"close")){
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


bool processOpenCommand(string cmd){
   stringutils::StringTokenizer st(cmd,blanks,true);
   string rest = st.nextToken();
   rest = st.getRest();
   fid_t fid = PL_open_foreign_frame();
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
    PL_discard_foreign_frame(fid);
    return ok;
}

void showError(const string& s){
  cerr << s << endl;
}


bool processSqlCommand(string cmd){
   bool correct;
   string errMsg;
   cmd = checkSQLCommand(cmd,correct, errMsg);
 
   if(!correct){
     cout << "syntax error in command :" << errMsg << endl;
     return false;
   } 
   fid_t fid = PL_open_foreign_frame();
    term_t a0 = PL_new_term_refs(2);
    PL_put_atom_chars(a0, (cmd).c_str());
    predicate_t p = PL_predicate("sqlToPlan",2,"");
    qid_t id;
    bool ok= true;
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
               ok = false;
            } else {
                plan = "query "+ answer; 
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
    cout << "optimized plan  is "  << plan << endl;
    return processDirectSecondoCommand(plan);
};

string typestr(int no){

  switch(no){
   case PL_VARIABLE : return "PL_VARIABLE";
     case PL_ATOM  : return "PL_ATOM";
  //     case PL_NIL : return  "PL_NIL";
  //   case PL_BLOB : return "PL_BLOB";
     case PL_STRING : return "PL_STRING";
     case PL_INTEGER : return "PL_INTEGER";
     case PL_FLOAT : return "PL_FLOAT";
     case PL_TERM : return "PL_TERM ";
  //   case PL_LIST_PAIR : return  "PL_LIST_PAIR";
  //   case PL_DICT : return "PL_DICT";
     default : return "unknown";
  }

}


string getDescr(term_t& t){

 stringstream out;

 out << typestr(PL_term_type(t));

 if(PL_is_variable(t)){
    out << ", isVariable" << endl;
 }
 if(PL_is_ground(t)){
    out << ", isGround" << endl;
 }
 if(PL_is_atom(t)){
    out << ", isAtom" << endl;
 }
 if(PL_is_string(t)){
    out << ", is_string" << endl;
 }
 if(PL_is_integer(t)){
    out << ", is_integer" << endl;
 }
 if(PL_is_float(t)){
    out << ", is_float" << endl;
 }
 if(PL_is_callable(t)){
    out << ", is_callable" << endl;
 }
 if(PL_is_compound(t)){
    out << ", is_compound" << endl;
 }
 //if(PL_is_functor(t)){
 //   out << ", is_functor" << endl;
 //}
 if(PL_is_list(t)){
    out << ", is_list" << endl;
 }
 //if(PL_is_pair(t)){
 //   out << ", is_pair" << endl;
 //}
 if(PL_is_atomic(t)){
    out << ", is_atomic" << endl;
 }
 if(PL_is_number(t)){
    out << ", is_number" << endl;
 }
 if(PL_is_acyclic(t)){
    out << ", is_acylic" << endl;
 }
 return out.str();
}




bool getBindings( term_t term, term_t bindings, 
                  predicate_t& p, 
                  term_t& args, 
                  vector<pair<int, string> > & res  ){

   
  if(!PL_is_list(bindings)){
     cerr << "bindings is not a list" << endl;
     return false;
  }


  cout << "analyse bindings" << endl;

  // analyse bindings,
  // bindings should be a list of  ' = ( usename, origName )'
  map<string,string> nm;
  while(!PL_get_nil(bindings)){
     term_t head = PL_new_term_ref();
     term_t tail = PL_new_term_ref();
     if(!PL_get_list(bindings, head, tail)){
        cerr << "problem in dividing list" << endl;
        return false;
     }
     bindings = tail;
     int arity;
     term_t name  = PL_new_term_ref();
     if(!PL_get_name_arity(head, &name, &arity)){
       cerr << "name_arity failöed" << endl;
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
     string s2;
     char* s;  
     if(PL_get_chars(a1,&s, CVT_ALL)){
        s1 = string(s);
     } else {
        cerr << "getting a1's name failed" << endl;
        return false;
     }
     if(PL_get_chars(a2,&s, CVT_VARIABLE)){
        s2 = string(s);
     } else {
        cerr << "getting a2's name failed" << endl;
        return false;
     }
     nm[s2] = s1;
  }
  cout << "bindings ok" << endl;

  map<string,string>::iterator it = nm.begin();
  while(it!=nm.end()){
    cout << it->first << " -> " << it->second << endl;
    it++;
  } 


  // now, the important things a known

  term_t n = PL_new_term_ref();
  int arity;


  cout << "try get_name_arity" << endl;
  cout << "term type : " << typestr(PL_term_type(term)) << endl;

  cout << "detailed_type : " << getDescr(term) << endl;

  functor_t func = PL_new_term_ref();

  if(!PL_get_functor(term, &func)){
     cerr << "cannot extract functor" << endl;
     return false;
  }

  cout << "func type : " <<  getDescr(func) << endl;
  




  if(!PL_get_name_arity(term, &n, &arity)){
    cerr << "could not get name and arity of term" << endl;
    return false;
  }



  // return false;
  cout << "got name and arity" << endl;
  cout << "arity = " << arity << endl;
  cout << "name type : " <<  getDescr(n) << endl;
  char* pn;
  if(!PL_get_chars(n,&pn,CVT_ALL | REP_UTF8)){
    cerr << "could not get main name" << endl;
    return false;
  }


  cout << "arity = " << arity;
  cout << "name = " << pn << endl;
  
  p = PL_predicate(pn, arity, "");


   





  args = PL_new_term_refs(arity); 

  for(int i=0;i<arity;i++){
     cout << "process arg " << i << endl;
     if(!PL_get_arg(i+1, term, args+i)){
          cerr << "could not get arg " << (i+1) << endl;
          return false;
     }
     if(PL_term_type(args+i)==PL_VARIABLE){
         char* arg_n;
         if(PL_get_chars(args+i, &arg_n, CVT_VARIABLE)){
             string working_name(arg_n);
             map<string,string>::iterator it = nm.find(working_name);
             if(it!=nm.end()){
                pair<int,string> p(i+1, it->second);
                res.push_back(p);
             } else {
                cerr << working_name << " not bind " << endl;
             }
         } else {
            cerr << "get chars faield" << endl;
         }
     }
  }

  return false;
  return true;
}





bool processPrologCommand(const string& cmd){

   cerr << "general prolog commands not implemented yet" << endl;
   return false; 



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
  
   predicate_t p;
   term_t args;
   vector<pair<int, string> > b;
   if(!getBindings(term, bindings, p, args, b)){
      cerr << "problem in analyisng term" << endl;
      PL_discard_foreign_frame(fid);
      return false;
   }


   

    
   




  /*
   if (!PL_chars_to_term(cmd.c_str(), g) ) {
     cout << "could not create a prolog term" << endl;
     ok = false;
   } else {
    int arity;
    atom_t name = PL_new_term_ref();
    if(!PL_get_name_arity(g, &name, &arity)){
      cout << "could not determine name and arity of predicate" << endl;
      ok = false;
    } else {
      cout << "arity : " << arity << endl;
      int g_type = PL_term_type(g);
      cout << "type of g : " << typestr(g_type) << endl;
      int name_type = PL_term_type(name);
      cout << "type of name : " << typestr(name_type) << endl;
      char* n;
      if(PL_get_chars(name, &n,CVT_ALL)){
        cout << "Name = " << n << endl;
      } else {
         cout << "PL_get__chars failed" << endl;
      }
      // determine the variables
      term_t arg = PL_new_term_ref();
      vector<pair<int,string> > variables;
      for(int i=0;i<arity;i++){
          if(!PL_get_arg(i+1,g , arg)){
            cout << "error during getting arg " << (i+1) << endl;
          } else {
             int arg_type = PL_term_type(arg);
             if(arg_type==PL_VARIABLE){
               cout << "found variable as arg" << (i+1) << endl;
               char* s;
               string vn;
               if(PL_get_chars(arg,&s, CVT_VARIABLE)){
                  vn = string(s);
               } else {
                  cout << "problem in getting name of vatriable at position "
                       << (i+1) << endl;
                  vn = "unknown";
               }
               variables.push_back(pair<int,string>(i+1,vn));
             }
          }
      }
      cout << "found " << variables.size() << " variables " << endl;
      for(size_t i=0;i<variables.size();i++){
          cout << variables[i].first << " : " << variables[i].second << endl;
      }



    }
  } 
  */

  PL_discard_foreign_frame(fid);
  return true;
}






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

bool isOpenCommand(const string& cmd){
  stringutils::StringTokenizer st(cmd, blanks, true);
  if(!st.hasNextToken()){
    return false;
  }
  if(st.nextToken()!="open"){
    return false;
  }
  if(!st.hasNextToken()){
    return false;
  }
  if(st.nextToken()!="database"){
    return false;
  }
  // name of the database to open required
  if(!st.hasNextToken()){
    return false;
  }
  st.nextToken();
  return !st.hasNextToken();
}


bool processCommand(string& cmd){
  stringutils::trim(cmd);
  if(IsInternalCommand(cmd)){
     cout << "internal command recognized" << endl;
     return processInternalCommand(cmd);
  } else if (isSqlCommand(cmd)) {
     cout << "sql command recognized" << endl;
     return processSqlCommand(cmd);
  } else if(isOpenCommand(cmd)){
     cout << "open command recognized" << endl;
     return processOpenCommand(cmd);
  } else if(isDirectSecondoCommand(cmd)){
     cout << "direct secondo command recognized" << endl;
     return processDirectSecondoCommand(cmd);
  } else {
     cout << "general prolog command recognized" << endl;
     return processPrologCommand(cmd);
  }
}



bool  processCommands(istream& in, bool haltOnErrors, bool pdstyle ){
  string cmd = "";
  bool error = false;
  while((cmd!="quit") && (cmd!="q") && !globalAbort){
    cmd = getCommand(in, pdstyle);
    if(!processCommand(cmd)){
      cout << "Error occurred during command '" <<  cmd << "'" << endl;
      error = true;
    }
  }
  return !error;
}


} // end of namespace pltty

/*
2 SecondoPLMode

This function is the ~main~ function of SecondoPL.

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
  //DisplayTTY::Set_SI(si);
  DisplayTTY::Set_NL(mnl); 
  NList::setNLRef(mnl);
  DisplayTTY::Initialize();

  int rc =  pltty::processCommands(cin, false, false);

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

  //PL_halt(1);

  return rc;


}


