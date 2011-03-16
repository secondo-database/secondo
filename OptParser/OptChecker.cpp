
/*
Main function for testing the optimizer checker

*/

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "OptimizerChecker.h"

int
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
