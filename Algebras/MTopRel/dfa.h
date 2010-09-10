
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

#include <vector>
#include <string>
#include <algorithm>

#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "TopRel.h"
#include "NestedList.h" 
#include "IntNfa.h"



bool lengthSmaller( const string& s1, const string& s2){
   return s1.length() < s2.length();
}



string num2str(const int num){
 stringstream s;
 s << num;
 return s.str();

}

/*
2 some forward declarations

*/

int yyparse();


int parseString(const char* arg, IntNfa** res);


class TopRelDfa : public Attribute{

public:

/*
Constructors

*/

  TopRelDfa():Attribute(),canDestroy(false) {}

  TopRelDfa(const int dummy): Attribute(false),
                              startState(-1),
                              currentState(-1),
                              transitions(0),
                              finalStates(0),
                              predicateGroup(0),
                              numOfSymbols(0),
                              canDestroy(false) {}


   TopRelDfa(const TopRelDfa& d):
     Attribute(d.IsDefined()),
     startState(d.startState),
     currentState(d.currentState),
     transitions(d.transitions),
     finalStates(d.finalStates),
     predicateGroup(d.predicateGroup),
     numOfSymbols(d.numOfSymbols),
     canDestroy(false) {}


   TopRelDfa& operator=(const TopRelDfa& src){
     Attribute::operator=(src);
     SetDefined(src.IsDefined());
     startState = src.startState;
     currentState = src.currentState;
     transitions = src.transitions;
     finalStates = src.finalStates;
     predicateGroup = src.predicateGroup;
     numOfSymbols = src.numOfSymbols;
     return *this;
   }




  void setTo(const string& regex,
             const toprel::PredicateGroup& pg) {
    if(!pg.IsDefined()){
      SetDefined(false);
      return;
    }  
    std::vector<string> names = pg.getNames();
    // sort vector by the  length of its elements
    std::sort(names.begin(), names.end(), lengthSmaller); 


    // replaces all names 
    string changed = regex;
    for(unsigned int i=names.size()-1; i>=0; i--){
      changed = replaceAll(changed,names[i], string(" ") + 
                           num2str(i) + string(" ")); 
    }

    IntNfa* nfa = 0;
    parseString(changed.c_str(),&nfa);

    if(nfa==0){ // error in regular expression -> Set to be undefined
      SetDefined(false);
      return;
    }

    // construct a "deterministic nfa"
    nfa->nfa.makeDeterministic();

    if(nfa->nfa.getStartState() < 0){
      SetDefined(false);
      delete nfa;
      return;
    }
    

    transitions.clean();
    finalStates.clean();
 
    for(unsigned int i=0; i< nfa->nfa.numOfStates(); i++){
      finalStates.Append(nfa->nfa.isFinalState(i));
      State<int> state = nfa->nfa.getState(i);
      for(unsigned int j = 0; j < changed.size(); j++){
        std::set<int> targets = state.getTransitions(j);
        if(targets.empty()){
          transitions.Append(-1); // mark an error
        } else {
          assert(targets.size()==1);
          transitions.Append(*targets.begin());
        }
      }
    }








 
   


   }
     


  ~TopRelDfa(){
     if(canDestroy){
       finalStates.Destroy();
       transitions.Destroy();
     }
   } // no pointers 


  void Destroy(){
    canDestroy= true;
  }
 

/*
~isUsuable~

Checks whether the creation of the dfa was
successful and the dfs is not already destroyed;

*/
  bool isUsuable() const{
    return startState>=0;
  }


/*
using the dfa


~start~

Brings the dfa into its initial state.

*/

  void start() {
    currentState = startState;
  }  


/*
~isFinal~

Checks whether the dfa is in a final state.	

*/

  bool isFinal() const{
    if(currentState<0) {
      return false;
    }
    bool isFinal;
    finalStates.Get(currentState, isFinal);
    return isFinal;
  }


/*
~isError~


Checks whether this automaton is in an error state.

*/

  bool isError() const{
     return currentState<0;
  } 


/*
~next~

Goes into the next state depending on the input.

*/

  void next(int symbol){
     if((currentState < 0) || (startState<0)){
        return;
     }
     int index = numOfSymbols*currentState + symbol;
     transitions.Get(index,currentState);
  }



/*
1.4 Attribute functions

*/


 int NumOfFLOBs(){
    return predicateGroup.NumOfFLOBs() + 2;
 }

 Flob* GetFLOB(int index){
   assert(index>=0);
   assert(index < NumOfFLOBs());
   if(index < predicateGroup.NumOfFLOBs()){
      return  predicateGroup.GetFLOB(index);
   }
   index = index - predicateGroup.NumOfFLOBs();
   if(index==0){
     return &transitions;
   } else {
     return &finalStates;
  }
 }


 size_t Sizeof() const{
   return sizeof(*this);
 }

 int Compare(const Attribute* right) const{
   if(!IsDefined() && !right->IsDefined()){
      return 0;
   }
   if(!IsDefined()){
      return -1;
   }
   if(!right->IsDefined()){
      return 1;
   }
   TopRelDfa* d = (TopRelDfa*) right;

   if(startState<d->startState){
     return -1;
   }

   if(startState > d->startState){
     return 1;
   }
   // current state is ignored for comparision

   if(numOfSymbols < d->numOfSymbols){
     return -1;
   }

   if(numOfSymbols > d->numOfSymbols){
     return 1;
   }
   // ignore canDestroy

   int comp = predicateGroup.Compare(&(d->predicateGroup));

   if(comp!=0){
      return comp;
   }

   if(transitions.Size() < d->transitions.Size()){
     return -1;
   }
   if(transitions.Size() > d->transitions.Size()){
     return 1;
   }
   
   if(finalStates.Size() < d->transitions.Size()){
     return -1;
   }
   if(finalStates.Size() > d->transitions.Size()){
     return 1;
   }

   for(int i=0;i<transitions.Size();i++){
     int tv;
     int dv;
     transitions.Get(i,tv);
     d->transitions.Get(i,dv);
     if(tv < dv){
      return -1;
     }
     if(tv > dv){
       return 1;
     }
   }

   for(int i=0;i<finalStates.Size();i++){
     bool tv;
     bool dv;
     finalStates.Get(i,tv);
     d->finalStates.Get(i,dv);
     if(!tv && dv){
      return -1;
     }
     if(tv && !dv){
       return 1;
     }
   }
   return 0;
 }

 bool Adjacent(const Attribute* r) const{
    // there is no meaningful implementation for it
    return false;
 }

 TopRelDfa* Clone() const{
    return new TopRelDfa(*this);
 }



 size_t HashValue() const{
   return startState + transitions.Size() + finalStates.Size() +
          predicateGroup.HashValue() + numOfSymbols;
 }

 void CopyFrom(const Attribute* right){
    operator=(*((TopRelDfa*) right));
 }


private:

   int startState;             // start state
   int currentState;           // current state
   DbArray<int> transitions;   // transition table
   DbArray<bool> finalStates;  // marks an state to be final
   toprel::PredicateGroup predicateGroup;
   int numOfSymbols;           // size of sigma
   bool canDestroy;
};
