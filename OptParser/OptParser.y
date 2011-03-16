%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define YYDEBUG 1
#define YYERROR_VERBOSE 1


#ifdef __cplusplus
extern "C"{
 int optlex();
 int opterror (const char *error);
 void opt_scan_string(const char* argument);
}
#endif


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







main(int argc, char* argv[]) {
   if(argc<1){
     return 1;
   }
   char* err=0;
  
   bool ok = checkOptimizerQuery(argv[1],err);
   
   if(ok){
      printf("query is valid\n");
   } else {
      printf("invalid query %s\n",err);
      free(err); 
   }

}
