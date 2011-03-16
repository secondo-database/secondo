%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "NestedList.h"
#include "SecondoInterface.h"


#define YYDEBUG 1
#define YYERROR_VERBOSE 1


#ifdef __cplusplus
extern "C"{
 int optlex();
 int opterror (const char *error);
 void opt_scan_string(const char* argument);
}
#endif


extern NestedList* nl;       // use global si
extern SecondoInterface* si;  // use the same si as the rest of prolog


char* err_message;
bool success;


%}


%union {
 char* strval;
 int numval;
}


%name-prefix="opt"

%token SELECT FROM STAR ERROR
%token<strval> ID


%%

simplequery :  SELECT STAR FROM sources {
          err_message = 0;      
          success = true;
   }
;

sources:  ID {
      char* relname = $1; 
      // check whether a database is opened
 /*      ListExpr res;
      int errorCode = 0;
      int errorPos =0;
      string errorMsg ="";
      si->Secondo("query getDatabaseName()",
                  0, // command as nested list
                  1, // command level
                  true, // command as text
                  false, // result as texta
                  res,
                  errorCode,
                  errorPos,
                  errorMsg);

      if(errorCode!=0){
        
      }
   */

     }
;

%%


int opterror (const char *error)
{
  success=false;
  err_message = (char*)malloc(strlen(error)+1);
  strcpy(err_message, error);
  return 0;
}

extern "C"{void lexDestroy();}


bool checkOptimizerQuery(const char* argument, char*& errmsg){
    lexDestroy();
 
    opt_scan_string(argument);
    yyparse();
 
    if(success && err_message){
         free(err_message);
         err_message=0;
     }
     if(!success){
        errmsg = err_message;
        err_message = 0;
     }
     return success;
}






