
/*
----
This file is part of SECONDO.

Copyright (C) 2007, 
Faculty of Mathematics and Computer Science,
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

1 File for Testing Nfa.h


*/




#include <string>
#include <iostream>

#include "../FText/IntNfa.h"


int yyparse();

namespace temporalalgebra{


int parseString(const char* arg, IntNfa** res);


}

int main(int argc, char** argv){

   if(argc<2){
    std::cout << "argument missing" << std::endl;
    return 127;
   }

   const char* expr = argv[1];

   IntNfa* res = 0;




   std::cout << "call parseString" << std::endl;
  
   temporalalgebra::parseString(expr, &res);

   if(res){
    res->nfa.print(std::cout);


    std::cout << "without epsilon transitions " << std::endl;

    res->nfa.removeEpsilonTransitions();

    res->nfa.print(std::cout) << std::endl; 


    std::cout << "make deterministic " << std::endl;

    res->nfa.makeDeterministic();

    
    res->nfa.print(std::cout);

    std::cout << "minimize" << std::endl;
    res->nfa.minimize();
    res->nfa.bringStartStateToTop();


    res->nfa.print(std::cout);


    delete res;

   } else {
     std::cout << "Error during parsing" << std::endl;
   }
   



}


