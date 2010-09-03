
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

*/


enum DfaState = {normal, final, complete, completefinal, error};

class DFA{

public:

/*
Constructors

*/

  DFA() {}

  DFA(const int sigma,  // size of the alphabet
      const string regex); // regular expression over numbers


  ~DFA(){

   }


  void Destroy(){
    states.Destroy();
    transitions.Destroy();
    start = -1;
    current = -1;
  }
 

/*
~isUsuable~

Checks whether the creation of the dfa was
successful and the dfs is not already destroyed;

*/
  bool isUsuable() const{
    return start>=0;
  }


/*
using the dfa


~start~

Brings the dfa into its initial state.

*/

  void start() {
    current = start;
  }  

/*
~isFinal~

Checks whether the dfa is in a final state.	

*/

  bool isFinal() const{
    if(current<0) {
      return false;
    }
    DfaState state;
    states.Get(current, state);
    switch(state){
      case final:
      case completefinal: return true;
      default: return false;
    }
  }


/*
~isError~


Checks whether this automaton is in an error state.

*/

  bool isError() const{
   if(current<0){
     return true;
   }
   DfaState state;
   states.Get(current, state);
   return state==error;
  } 

/*
~isComplete~

Checks whether the current state is never leaved independent
on further inputs.

*/

  bool isComplete() const{
    if(current<0) {
      return false;
    }
    DfaState state;
    states.Get(current, state);
    switch(state){
      case complete:
      case completefinal: return true;
      default: return false;
    }


/*
~next~

Goes into the next state depending on the input.

*/

  void next(int symbol){
     assert(symbol>=0);
     assert(symbol<=numOfSymbols);

     if((current < 0) || (start<0)){
        return;
     }
     int index = states.Size()*current + symbol;
     transitions.Get(index,current);
  }



private:

   int start;    // start state
   int current;  // current state
   DbArray<DfaState> states; // marks for each state

   DbArray<int> transitions;  // transition table
   int numOfsymbols;    // size of sigma

 
};
