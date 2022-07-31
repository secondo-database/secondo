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
#include "Application.h"
#include "TTYParameter.h"
#include <string>
#include <assert.h>

using namespace std;

/*
2 predicates

The ~predicates~ array contains all prolog extensions required by
Secondo.

*/

extern PL_extension predicates[];
extern void handle_exit(void);
// extern char* GetConfigFileNameFromArgV(int&, char**);
// extern char* GetParameterFromArg(int&, char**,string,string);
extern bool StartSecondoC(TTYParameter &tp);

/*
2 SecondoPLMode

This function is the ~main~ function of SecondoPL.

*/

int SecondoPLMode(TTYParameter &tp) {
  atexit(handle_exit);

  if (!StartSecondoC(tp)) {
    cout << "Usage : SecondoPL [Secondo-options] [Prolog-options]" << endl;
    exit(1);
  }

  /* Start PROLOG interpreter with our extensions. */
  PL_register_extensions(predicates);

  /* initialize the PROLOG engine */

  int argc = 0;
  char **argv = tp.Get_plargs(argc);

  // cerr << endl <<__FILE__ << ":" << __LINE__
  //     << " Calling PL_initialize with ";

  // for (int i = 0; i < argc; i++) {
  //  cerr << argv[i] << " ";
  //}
  // cerr << endl << endl;

  if (!PL_initialise(argc, argv)) {
    PL_halt(1);
  } else {

    {
      // VTA - 15.11.2005
      // I added this piece of code in order to run with newer versions
      // of prolog. Without this code, the libraries (e.g. list.pl) are
      // not automatically loaded. It seems that something in our code
      // (auxiliary.pl and calloptimizer.pl) prevents them to be
      // automatically loaded. In order to solve this problem I added
      // a call to 'member(x, []).' so that the libraries are loaded
      // before running our scripts.
      term_t t0 = PL_new_term_refs(2), t1 = t0 + 1;
      PL_put_atom_chars(t0, "x");
      if (!PL_put_list_chars(t1, "")) {
        cerr << "problem with prolog interface" << endl;
        assert(false);
      }
      predicate_t p = PL_predicate("member", 2, "");
      PL_call_predicate(NULL, PL_Q_NORMAL, p, t0);
      // end VTA
    }


#if PLVERSION > 80000
    // Restrict autoloading for SWI-Prolog 8.x compatibility
    // set_prolog_flag(autoload, user).
    // See https://github.com/SWI-Prolog/issues/issues/117
    term_t term_first = PL_new_term_refs(2);
    term_t term_second = term_first + 1;
    PL_put_atom_chars(term_first, "autoload");
    PL_put_atom_chars(term_second, "user");
    predicate_t flag_predicate = PL_predicate("set_prolog_flag", 2, "");
    PL_call_predicate(NULL, PL_Q_NORMAL, flag_predicate, term_first);
#endif

    /* load the auxiliary and calloptimizer */
    term_t a0 = PL_new_term_refs(1);
    static predicate_t p = PL_predicate("consult", 1, "");
    PL_put_atom_chars(a0, "auxiliary");
    PL_call_predicate(NULL, PL_Q_NORMAL, p, a0);
    PL_put_atom_chars(a0, "calloptimizer");
    PL_call_predicate(NULL, PL_Q_NORMAL, p, a0);
    /* switch to prolog-user-interface */
  }

// readline support is only needed on unix systems.
#ifndef SECONDO_WIN32
  string histfile = ".secondopl_history";

#if PLVERSION < 70334
  PL_install_readline();
#endif

  // term_t ah = PL_new_term_refs(1);
  // static predicate_t prh = PL_predicate("rl_read_history",1,"");
  // PL_put_atom_chars(ah,histfile.c_str());
  // PL_call_predicate(NULL, PL_Q_CATCH_EXCEPTION, prh, ah);
#endif

  return PL_toplevel();
}
