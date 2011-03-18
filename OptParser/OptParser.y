%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OptSecUtils.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1


#ifdef __cplusplus
extern "C"{
 int optlex();
 int opterror (const char *error);
 void opt_scan_string(const char* argument);
}
#endif

/*
Use the prolog nested list plnl.

*/
extern NestedList* plnl;

/*
Variables for return value and error message.

*/
char* err_message;
bool success;

/*
some variables corresponding the the options.

*/
bool subqueries = false;
// insert further options here



%}

/*
We need the content of some tokens, e.g. for TOKEN_ID. Hence just the
token is not enough to return. We need some more sophisticated return value.
For further checks, we have to extend this definition.

*/
%union {
 char* strval;
 int numval;
}


%name-prefix="opt"  // use opt instead of yy to avoid naming conflicts with other 
                    // parsers of the system


/*
Define simple token and token holding a value.

*/

%token TOKEN_SELECT TOKEN_FROM TOKEN_STAR TOKEN_ERROR
%token<strval> TOKEN_ID TOKEN_VARIABLE


%%

simplequery :  TOKEN_SELECT TOKEN_STAR TOKEN_FROM sources {
          err_message = 0;      
          success = true;
   }
;

sources:  TOKEN_ID {

     string dbname;

  
     string errorMsg;
     if(!optutils::isDatabaseOpen(dbname,errorMsg)){
        opterror(errorMsg.c_str());
        return false;
      } 

     


      char* relname = $1; 
      ListExpr type = plnl->TheEmptyList();
      string realname;
      if(!optutils::isObject(relname, realname, type)){
         string err = "Object " + string(relname) + " not known in the database " + dbname;
         opterror(err.c_str());
         return false;
      } 

      if(!optutils::isRelDescription(type)){
         string err = "The object " + realname + " is not a relation.";
         opterror(err.c_str());
         return false;
       }
  

     }


  | TOKEN_VARIABLE {
     opterror("The name of a relation must start with a lower case letter");
     return false;
  }

;

%%

/*
error handling

*/
int opterror (const char *error)
{
  success=false;
  err_message = (char*)malloc(strlen(error)+1);
  strcpy(err_message, error);
  return 0;
}


/*
declaration of a function work around memory leaks.


*/
extern "C"{void optlexDestroy();}


/*

Main function. Checks a sql query against the requierements of the 
secondo's optimizer.

*/

bool checkOptimizerQuery(const char* argument, char*& errmsg){

   try{

    optlexDestroy();
 
    opt_scan_string(argument);

    optparse();
 
    if(success && err_message){
         free(err_message);
         err_message=0;
     }
     if(!success){
        errmsg = err_message;
        err_message = 0;
     }
     return success;
  } catch(...){
      opterror("internal error during parsing");;
      return false;
  }

}

/*
Sets an compilerOption. If the option is not found, the ersult will be false. 

*/
bool setSqlParserOption(const string& optionName, const bool enable){
   if(optionName == "subqueries") {
       subqueries = enable;
       return true;
   }
   // insert further options here
   return false;
}






