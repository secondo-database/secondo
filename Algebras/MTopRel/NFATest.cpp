
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

#include "Nfa.h"




int main(int argc, char** argv){

  Nfa<char> nfa_1('a');

  std::cout << "nfa a: " << std::endl; nfa_1.print(std::cout) << std::endl;

  Nfa<char> nfa_2('b');
  Nfa<char> nfa_3('c');

  nfa_1.concat(nfa_2);

  std::cout << "nfa ab: " << std::endl; nfa_1.print(std::cout) << std::endl;
  
  nfa_1.disjunction(nfa_3);


  std::cout << "nfa ab | c : " << std::endl; nfa_1.print(std::cout) 
            << std::endl;


  nfa_1.star();


  std::cout << "(nfa ab | c)* : " 
            << std::endl; nfa_1.print(std::cout) 
            << std::endl;


  nfa_1.removeEpsilonTransitions();


  std::cout << "(nfa ab | c)* (without epsilon): " 
            << std::endl; nfa_1.print(std::cout) 
            << std::endl;

  nfa_1.makeDeterministic();

  std::cout << "(nfa ab | c)* (deterministic version): " 
            << std::endl; nfa_1.print(std::cout) 
            << std::endl;


}


