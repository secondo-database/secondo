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
#include "SWI-Prolog.h"
#include <stdlib.h>
#include "SecondoConfig.h"

using namespace std;


/*
2 predicates

The ~predicates~ array contains all prolog extensions required by
Secondo.

*/

extern PL_extension predicates[];
extern void handle_exit(void);
extern char* GetConfigFileNameFromArgV(int&, char**);
extern bool StartSecondoC(char*);

/*
2 SecondoPLMode

This function is the ~main~ function of SecondoPL.

*/

int
SecondoPLMode(int argc, char **argv)
{
  char* configFile;

  atexit(handle_exit);

  /* Start Secondo and remove Secondo command line arguments
     from the argument vector .*/
  configFile = GetConfigFileNameFromArgV(argc, argv);
  if(configFile == 0 || !StartSecondoC(configFile))
  {
    cout << "Usage : SecondoPL [-c ConfigFileName] [prolog engine options]" 
         << endl;
    exit(1);
  }

  /* Start PROLOG interpreter with our extensions. */
  PL_register_extensions(predicates);

  /* initialize the prologb engine */
  char * initargs[argc+3];
  int p=0;
  for(p=0;p<argc;p++)   // copy arguments
     initargs[p]=argv[p];
  initargs[argc] ="pl";
  initargs[argc+1] ="-g";
  initargs[argc+2] ="true";
  if(!PL_initialise(argc+3,initargs))
      PL_halt(1);
  else{

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
      PL_put_list_chars(t1, "");
      predicate_t p = PL_predicate("member",2,"");
      PL_call_predicate(NULL,PL_Q_NORMAL,p,t0);
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

// readline support is only needed on unix systems.
#ifndef SECONDO_WIN32
  PL_install_readline();
#endif

  return PL_toplevel();
  // this function never returns. Entering "halt." at the Userinterface
  // calls exit().

}


