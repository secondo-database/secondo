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



#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <functional>
#include "Functions.h"
#include <assert.h>



#ifndef  NFA_H
#define NFA_H

/*
2 Class State

This class stores the transitions of an state of an nfa.


*/

template<typename sigma>
class State{

public: 
/*
1.1. Constructor

Creates an state without transitions.

*/
  State(): transitions(), epsilonTransitions() {}  


/*
1.2 Copy Constructor

*/
  State(const State<sigma>& src): transitions(src.transitions),
    epsilonTransitions(src.epsilonTransitions){}


/*
1.2 Assignment Operator

*/
  State& operator=(const State<sigma>& src){
    transitions = src.transitions;
    epsilonTransitions = src.epsilonTransitions;
    return *this;
  }
  
/*
1.3 Destructor

*/

  ~State() {}


/*
1.4 Public Functions

~insertTransition~

This function creates a transition to another state.
The state is referrenced by a number __fstate__. The 
character producing the transition is given by __sigma__

*/
  void insertTransition(const sigma s,const int fstate){
    typename std::map<sigma, std::set<int> >::iterator it;

    it = transitions.find(s);
    if(it==transitions.end()){
      std::set<int> t;
      t.insert(fstate);
      transitions[s]= t;
    }  else {
       transitions[s].insert(fstate);
    }  
  }

  void insertTransitions(const std::map<sigma, std::set<int> >& trans){
     typename std::map<sigma, std::set<int> >::const_iterator it;
     for(it = trans.begin(); it!=trans.end();it++){
        sigma s = it->first;
        typename std::set<int>::iterator it2;
        for(it2=it->second.begin(); it2!=it->second.end();it2++){
           insertTransition(s,*it2);
        }
     }
  }


/*
~removeTransition~

This function removes an exactly specified transition.

*/
  void removeTransition(const sigma s, const int fstate){
    typename std::map<sigma, std::set<int> >::iterator it = transitions.find(s);
    if(it!=transitions.end()){
      it->erase(fstate);
    }
  }


/*
~insertEpsilonTransition~

Inserts an epsilon transition to the given state.

*/
  void insertEpsilonTransition(const int fstate){
    epsilonTransitions.insert(fstate);
  }


/*
~removeEpsilonTransition~

This function removes the epsilon transition to the given state.

*/
  void removeEpsilonTransition(const int fstate){
    epsilonTransitions.erase(fstate);
  }


  void removeEpsilonTransitions(){
     epsilonTransitions.clear();
  }


/*
~clear~

Removes all transitions starting at this state.

*/
  void clear(){
    transitions.clear();
    epsilonTransitions.clear();
  }


/*
~shift~

This function adds the given number to all identifiers 
of contained target states.

*/

  void shift(const int no){
    typename std::map<sigma, std::set<int> >::iterator it = transitions.begin();
    while(it != transitions.end()){
       std::set<int> s2;
       std::set<int>::iterator it2 = it->second.begin();
       while(it2!=it->second.end()){
         s2.insert(*it2 + no);
         it2++;
       }
       it->second = s2; // replace the current set by the new created one
       it++;
    }
    
    std::set<int>::iterator it3 = epsilonTransitions.begin();
    std::set<int> s3;
    while(it3 != epsilonTransitions.end()){
      s3.insert(*it3 + no);
      it3++;
    }
    epsilonTransitions = s3;
  }

/*
~containsEpsilonTransitions~

Checks whether there are epsilon transitions within this state.

*/
  bool containsEpsilonTransitions() const{
    return epsilonTransitions.size() >0;
  }

/*
~getEpsilonTransitions~

Returns the contained epsilon transitions.

*/

  const std::set<int>& getEpsilonTransitions() const{
     return epsilonTransitions;
  }


  const std::map<sigma, std::set<int> >& getTransitions() const{
     return transitions;
  }

  std::set<int> getTransitions(const sigma& s) const{
    typename std::map<sigma, std::set<int> >::const_iterator it = 
                           transitions.find(s);
    if(it!=transitions.end()){
      return it->second;
    } else {  // return an empty set
      std::set<int> res; 
      return res;
    }
  }  


  std::set<int> getTargets(bool includeEpsilonTransitions) const{
    std::set<int> result;
    typename std::map<sigma, std::set<int> >::const_iterator it;
    for(it = transitions.begin(); it!=transitions.end(); it++){
       result.insert(it->second.begin(), it->second.end());
    }
    if(includeEpsilonTransitions){
       result.insert(epsilonTransitions.begin(), epsilonTransitions.end());
    }
    return result;
  }


  std::ostream& print(std::ostream& o, const std::string secondIdent="") const{
    if(!transitions.empty()){
      typename std::map<sigma, std::set<int> >::const_iterator it;
      for(it = transitions.begin(); it!=transitions.end(); it++){
         if(it!=transitions.begin()){
           o << secondIdent;
         }
         
         o << "    " <<  it->first << " -> "; 
         printset<int>(it->second,o) << std::endl; 
      }
    }
    if(!epsilonTransitions.empty()){
      if(!transitions.empty()) {
         o << secondIdent;
      }
      o << "    " << "    " << " -> ";
      printset<int>(epsilonTransitions, o);
    }

       
    return o;
  }


/*
~ReplaceTargets~

replaces all targets contained in the set gives as the first parameter 
to the new target

*/

void replacesTargets(const std::set<int>& oldTargets, int newTarget){

   unsigned int oldSize = epsilonTransitions.size();

   std::set<int>::iterator it;
   for(it=oldTargets.begin(); it!=oldTargets.end(); it++){
     epsilonTransitions.erase(*it);
   }
   if(epsilonTransitions.size()!=oldSize){
       epsilonTransitions.insert(newTarget);
   }


   typename std::map<sigma, std::set<int> >::iterator it2;
   for( it2 = transitions.begin(); it2!=transitions.end(); it2++){
       std::set<int>  targets = it2->second;
       oldSize = targets.size();
       for(it=oldTargets.begin(); it!=oldTargets.end(); it++){
         targets.erase(*it);
       }
      if(targets.size()!=oldSize){
        targets.insert(newTarget);
      }
      it2->second = targets; 
   }
}


/*
~switchTargets~

switch the tagets gives as arguments.

*/
void switchTargets(const int target1, const int target2){
   if(target1==target2){
     return;
   }
   bool contains1;
   bool contains2;
   contains1 = epsilonTransitions.find(target1) != epsilonTransitions.end();
   contains2 = epsilonTransitions.find(target2) != epsilonTransitions.end();
   if(contains1!=contains2){
     if(contains1){
        epsilonTransitions.erase(target1);
        epsilonTransitions.insert(target2);
     } else {
        epsilonTransitions.erase(target2);
        epsilonTransitions.insert(target1);
     }
   }
   typename std::map<sigma, std::set<int> >::iterator it;
   for( it=transitions.begin(); it!=transitions.end(); it++){
     contains1 = it->second.find(target1) != it->second.end();
     contains2 = it->second.find(target2) != it->second.end();
     if(contains1!=contains2){
       if(contains1){
          it->second.erase(target1);
          it->second.insert(target2);
       } else {
          it->second.erase(target2);
          it->second.insert(target1);
       }
     }
   }


}





/*
~removeState~

This function deletes all transitions to the given state. Furthermore,
the target of all transitions having a target greater than state is
decremented by one.

*/
void removeState(int state){
   if(state<0 ){ // nonsense
    return;
   }
   ::removeState(epsilonTransitions, state);
   typename   std::map<sigma, std::set<int> >:: iterator it;
   for(it=transitions.begin(); it!=transitions.end(); it++){
     ::removeState(it->second, state);
   }
}





/*
~isDeterministic~

This function checks whether this state has no epsilon transitions and 
for aeach element of sigma at most one transition.

*/
bool isDeterministic() const{
  if(! epsilonTransitions.empty()){
    return false;
  }  
  typename std::map<sigma, std::set<int> >::const_iterator it;
  for(it=transitions.begin(); it!=transitions.end(); it++){
    if(it->second.size()>1){
      return false;
    }
  }
  return true;
}

/*
~removeNonDeterministicTransitions~


This function removes all non deterministic transitions (without epsilon 
transitions) from this state and returns these.

*/

  std::map<sigma, std::set<int> > removeNonDeterministicTransitions(){
     std::map<sigma, std::set<int> > result;
     std::map<sigma, std::set<int> > remainder;
     typename std::map<sigma, std::set<int> >::iterator it;
     for(it = transitions.begin(); it!=transitions.end(); it++){
        if(it->second.size() > 1){ // non deterministic transition
           result[it->first] = it->second;
        } else {
           remainder[it->first] = it->second;
        }
     }
     transitions = remainder;
     return result;
  }


/*
~extractSigmas~

This function inserts all sigmas defining transitions starting
at this state into the given set.

*/
  
void extractSigmas(std::set<sigma>& initial){
  typename std::map<sigma, std::set<int> >::const_iterator it;
  for(it = transitions.begin(); it!=transitions.end(); it++){
      initial.insert(it->first);
  }
}

/*
~extractTargets~

This function inserts all targets coming from the given sigma into
the given set.

*/

void extractTargets(sigma s, std::set<int>& initial){
  typename std::map<sigma, std::set<int> >::const_iterator it;
  it = transitions.find(s);
  if(it==transitions.end()){
     return;
  } else {
    initial.insert(it->second.begin(), it->second.end());
  }
}  



private:

/*
1.5 Members

~transitions~

A map represention the normal transitions.

*/
  std::map<sigma, std::set<int> > transitions;


/*
~epsilonTransitions~

The target states of epsilon transitions.

*/
  std::set<int> epsilonTransitions;

};








/*
2 Class Nfa

The class Nfa represents a non deterministic finite automaton.

*/

template<typename sigma>
class Nfa{

public:
/*
~constructors~

Creates an Nfa accepting the empty word.

*/

  Nfa();


/*
Creates an Nfa accepting c

*/
  Nfa(const sigma& c);

/*
Creates an Nfa as disjunction of the contents of the vector

*/
  Nfa(const std::vector<sigma>& v);

/*
Copy Constructor

*/

  Nfa(const Nfa<sigma>& src);


/*
Assignment Operator

*/
  Nfa<sigma>& operator=(const Nfa<sigma>& src);


/*
Destructor

*/
  ~Nfa();



/*
~concat~

concats another Nfa to this Nfa

*/

  void concat(const Nfa<sigma>& second);


/*
~disjunction~


Changes this nfa to represent the disjuntion of this and the argument.

*/
  void disjunction(const Nfa<sigma>& second);


/*
~star~

Changes this nfa orig  such that its represents orig star

*/

  void star();


/*
~print~

prints out this nfa

*/

 std::ostream& print(std::ostream& o) const{
   if(states.size()==0){
      o << "empty" << std::endl;
      return o;
   }
   for(unsigned int i=0;i<states.size();i++){
     if((signed int)i==startState){
         o << "-> ";
     } else {
         o << "   ";
     }
     if(finalStates.find(i)!=finalStates.end()){
        o << "* ";
     } else {
        o << "  ";
     }
     o << " state " << i << ": " << std::endl;
     states[i].print(o,"     ");
     o << std::endl;
     o << std::endl;
   }
   return o;

 }




/*
~containsEpsilonTransitions~

Checks whether this automaton contains epsilon transitions.

*/
  bool containsEpsilonTransitions() const{
    typename std::vector<State<sigma> >::const_iterator it = states.begin();
    while(it!=states.end()){
      if(it->containsEpsilonTransitions()){
        return true;
      }
      it++;
    }
    return false;
  }


/*
~removeEpsilonTransitions~

Converts this Nfa into an Nfa without epsilon transitions accepting the same 
regular expression.  

*/

 void removeEpsilonTransitions(){
   if(containsEpsilonTransitions()){
     removeEpsilonLoops();
     if(containsEpsilonTransitions()) {
         removeEpsilonChains();
     } 
   }
   
   removeUnreachable();
   removeDeadEnds();
 }



/*
~isDeterministic~

Checks whether this automaton is deterministic.

*/
bool isDeterministic(){
  for(unsigned i=0;i< states.size();i++){
    if(!states[i].isDeterministic()){
       return false;
    }
  }
  return true;
}


static bool lxor(bool a, bool b){
    return (a && !b) || (!a && b);
}


/*
~minimize~

This function computes a minimum automaton from a deterministic one. 
Returns true if the amount of states could be decreased.

*/

bool minimize(){
  assert(isDeterministic());

  bool res = false;
  res = res ||  removeUnreachable();
  res = res || removeDeadEnds();

  bool marks[states.size()][ states.size()];
  // initialize matrix, set to true iff the states ares different in there 
  // isFinal Property
  for(unsigned int i=0;i<states.size(); i++){
    marks[i][i] = false; 
    for(unsigned int j=i+1; j<states.size(); j++){
       bool value = lxor(isFinalState(i), isFinalState(j));
       marks[i][j] = value;
       marks[j][i] = value;          
    }
  }

  /*
  // debug::start
  std::cout << "initialized matrix for minimization" << std::endl;
  for(unsigned int i=0;i<states.size(); i++){
    std::cout << "\t" << i;
  }
  std::cout << std::endl;
  for(unsigned int i=0;i< states.size(); i++){
    std::cout << i;
    for(unsigned int j=0;j<states.size(); j++){
      std::cout << "\t" << marks[i][j];
    }
    std::cout << std::endl;
  }
  // debug::end
  */


  bool changed = true;
  while(changed){
    changed = false;
    for(unsigned int i=0;i<states.size(); i++){
      for(unsigned int j=i+1; j < states.size(); j++){
         if(!marks[i][j]){
            State<sigma> s_i = states[i];
            State<sigma> s_j = states[j];
            std::set<sigma> sigma_i;
            s_i.extractSigmas(sigma_i);
            std::set<sigma> sigma_j;
            s_j.extractSigmas(sigma_j);

            if(sigma_i == sigma_j ){ 
               bool markFound = false;
               typename std::set<sigma>::iterator it;
               for(it = sigma_i.begin(); 
                         (it != sigma_i.end()) && !markFound;
                          it++){
                  int target_i =  *(s_i.getTransitions(*it).begin());
                  int target_j = *(s_j.getTransitions(*it).begin());
                  markFound = marks[target_i][ target_j];
               }
               if(markFound){ // one of the  target pairs is marked
                  marks[i][j] = true;
                  marks[j][i] = true;
                  changed = true;
               }
            } else {
               marks[i][j] = true;
               marks[j][i] = true;
               changed = true;
            }
         }
      }
    }
  }

  /*
  // debug::start
  std::cout << "completly computed matrix" << std::endl;
  for(unsigned int i=0;i<states.size(); i++){
    std::cout << "\t" << i;
  }
  std::cout << std::endl;
  for(unsigned int i=0;i< states.size(); i++){
    std::cout << i;
    for(unsigned int j=0;j<states.size(); j++){
      std::cout << "\t" << marks[i][j];
    }
    std::cout << std::endl;
  }
  // debug::end
  */

  // compute the sets of states which could be summarized
  bool used[states.size()];
  for(unsigned int i=0; i< states.size(); i++){
    used[i] = false;
  }

  std::set<std::set<int> > clusters;

  for(unsigned int i=0; i<states.size(); i++){
    if(!used[i]){
      used[i] = true;
      std::set<int> cluster;
      cluster.insert(i);
      for(unsigned int j=i+1; j< states.size(); j++){
        if(!marks[i][j]){
           assert(!used[j]);
           cluster.insert(j);
           used[j] = true;
        }
      }
      if(cluster.size() > 1){
         clusters.insert(cluster);
      }
    }
  } 

  /*
  // debug::start
  if(clusters.size()>0){
    std::cout << "found some clusters: " << std::endl;
    int count =0;
    typename  std::set<std::set<int> >::iterator it;
    for( it = clusters.begin(); it!=clusters.end(); it++){
      std::cout << "Cluster " << count++ << " :";
      printset(*it, std::cout) << std::endl;
    }    

  } 

  // debug::end
  */
 


  // now, we have clusters which could be summarized
  if(clusters.size()>0){
     res = true;
 
     typename std::set<std::set<int> >::iterator it;

     for(it = clusters.begin(); it!= clusters.end(); it++){
       std::set<int> cluster = *it;
       int first = *(cluster.begin());
       cluster.erase(first);
       if(cluster.find(startState)!=cluster.end()){
          startState=first;
        }       


       for(unsigned int i=0;i<states.size();i++){
         states[i].replacesTargets(cluster, first);
       }
     } 

     removeUnreachable();
     removeDeadEnds();
   }

  return res;
   

}








static unsigned int 
    retrieveOrCreateIndex(std::vector<std::set<int> >& newStates,
                          std::set<int> elem){
   for(unsigned i=0; i<newStates.size(); i++){
      if(newStates[i] == elem){
        return i;
      }
   }
   newStates.push_back(elem);
   return newStates.size() - 1;
}


/*
~makeDeterministic~

This function creates a determinitic finite automaton from this 
object.  The automaton is always represented as an nfa. 

*/
  void makeDeterministic(){

     removeEpsilonTransitions();

     if(isDeterministic()){
        return;
     }
     
     // ok, we have to introduce additional states representing whole sets 
     // of "old" states

     std::vector<std::set<int> > newStates;

     for(unsigned int i=0;i<states.size(); i++){
        if(!states[i].isDeterministic()){
           std::map<sigma, std::set<int> > ndt = 
                states[i].removeNonDeterministicTransitions();
           typename std::map<sigma, std::set<int> >:: iterator it;
           for(it = ndt.begin(); it!=ndt.end(); it++){
               int index = retrieveOrCreateIndex(newStates, it->second);
               states[i].insertTransition(it->first,states.size()+index);
           }
        }
     }

     
     unsigned int oldsize = states.size();

     // process newly created states ....

     // insert new states into vector
    
     for(unsigned int i=0; i< newStates.size();i++){
       State<sigma> s;
       if(intersects<int>(newStates[i], finalStates)){
           finalStates.insert(states.size());
       }
       states.push_back(s);
     }



     // insert transitions from big states , eventually creating new big states
     unsigned int pos = 0;
     while( pos < newStates.size() ){

       std::set<int> current = newStates[pos];
      
       // collect all sigmas introducing transitions from all contained states
       // from the current state

       std::set<sigma> usedSigmas;
       typename std::set<int> ::iterator it1;
       for(it1=current.begin(); it1!=current.end(); it1++){
          states[*it1].extractSigmas(usedSigmas);
       }

       typename std::set<sigma>::iterator it2;
       for(it2=usedSigmas.begin(); it2!=usedSigmas.end(); it2++){
          sigma s = *it2;
          std::set<int> targets;
          targets.clear();
          for(it1=current.begin(); it1!=current.end(); it1++){
             states[*it1].extractTargets(s,targets);
          }
          if(targets.size()==1){ // target is a simple state
             states[pos+oldsize].insertTransition(s,*(targets.begin()));
          } else {
             // replace all targets which are "big" by the
             //  contents stored in newStates
             std::set<int> newTargets;
             typename std::set<int>::iterator it;
             for(it =targets.begin(); it!=targets.end(); it++){
                unsigned int target = *it;
                if(target<oldsize){
                  newTargets.insert(target);
                } else {
                  std::set<int> targetset = newStates[target-oldsize];
                  std::set<int>::iterator it2;
                  for(it2=targetset.begin(); it2!=targetset.end(); it2++){
                    newTargets.insert(*it2);
                  }
                }
             }
             targets=newTargets; 

             unsigned int bigsize = newStates.size();
             unsigned int index = retrieveOrCreateIndex(newStates, targets);
             if(newStates.size()!=bigsize){ // create a new state
                State<sigma> s;
                if(intersects<int>(newStates[index], finalStates)){
                  finalStates.insert(states.size());
                }
                states.push_back(s);
             }             
             states[pos+oldsize].insertTransition(s,index+oldsize);
          }
       }

       pos++;
     }  

     removeUnreachable();

     removeDeadEnds();

  }


  unsigned int numOfStates() const{
    return states.size();
  }

  State<sigma> getState(const int index) const{
     assert( (index >= 0 ) );
     assert(index < static_cast<int>(states.size()));
     return states[index];
  }

  bool isFinalState(const int index) const{
     return finalStates.find(index) != finalStates.end();
  }

  int getStartState() const{
    return startState;
  }


  void bringStartStateToTop(){
    if(startState <=0){
        return;
    }
    bool final0 = finalStates.find(0)!=finalStates.end();
    bool finalStart = finalStates.find(startState) != finalStates.end();
    if(final0 != finalStart){ // different final property of the 
                              // states to switch
       if(final0){
          finalStates.erase(0);
          finalStates.insert(startState); 
       } else {
          finalStates.erase(startState);
          finalStates.insert(0);
       }
    }    
    for(unsigned int i=0;i<states.size();i++){
        states[i].switchTargets(0, startState);
    }   
    State<sigma> start = states[startState];
    states[startState] = states[0];
    states[0] = start;
    startState=0;
  }


  bool createFrom(int startState, 
                  const std::vector<State<sigma> >& states, 
                  const std::set<int>& finalStates){

    // 1check for consistency

    // 1.1 check start state
    int numOfStates = states.size();
    if( (startState<0) || (startState >= numOfStates)){
       return  false;
    }
    // 1.2 all elements of finalStates must be valid
    typename std::set<int>::const_iterator it1;
    for( it1 = finalStates.begin(); it1 != finalStates.end(); it1++){
      int s = *it1;
      if(s<0 || s>=numOfStates){
         return false;
      }
    }

    // 1.2 all targets must be valid states
    typename std::map<sigma, std::set<int> >::const_iterator it2;
    for(it2 = states.begin(); it2!=states.end(); it2++){
       for(it1 = it2->begin(); it1 != it2->end(); it1++){
         int s = *it1;
         if(s<0 || s>=numOfStates){
            return false;
         }
       }
    }
   
    this->startState = startState;
    this->states = states;
    this->finalStates = finalStates;
 }


private:
 
/*
1.3 Member Variables

*/
 
   int startState;
   std::vector<State<sigma> > states;
   std::set<int> finalStates; 

 
/*
1.4 Private Functions

*/

/*
~append~

Appends second to this automaton without connecting it.

*/
 void append(const Nfa<sigma>& second);


/*
~removeEpsilonLoops~

Replaces states within an epsilon loop by a single one.

*/
  void removeEpsilonLoops(){
    std::vector<int> loop;
    unsigned int pos = 0;
    while(pos < states.size()){
       if(findEpsilonLoop(pos,loop)){
          mergeEpsilonLoop(loop);
          pos = 0;
       } else {
          pos++;
       }
    }
  }


/*
~removeEpsilonChains~

  removes chains of epsilon transitions

*/
  void removeEpsilonChains(){

   for(unsigned int i=0;i<states.size();i++){
      if(states[i].containsEpsilonTransitions()){
         removeEpsilonChain(i);
      }
   }
  } 

  void removeEpsilonChain(const int pos){
     std::set<int> et = states[pos].getEpsilonTransitions();
     if(et.size()==0){ // last element in chain
       return;
     }
     typename std::set<int>::iterator it;
     for(it=et.begin(); it!=et.end();it++){
        removeEpsilonChain(*it); // recursive call
        // add transition from successor
        states[pos].insertTransitions(states[*it].getTransitions());
        if(finalStates.find(*it)!=finalStates.end()){
          finalStates.insert(pos);
        }
     }
     states[pos].removeEpsilonTransitions();
  }




/*
~findEpsilonLoop~

Finds an epsilon loop containing the given state if such a loop exist.
If a loop exist, the return  value will be true and the set given as an
parameter contains the states within that loop. Otherwise, the return value 
will be false. 

*/ 

   bool findEpsilonLoop(const int state, std::vector<int>& loop) const{
     loop.clear();
     loop.push_back(state);

     return findEpsilonLoop(loop);
   }


   bool findEpsilonLoop( std::vector<int>& loop) const{

     int last = loop[loop.size()-1];  // last state added to loop
     State<sigma> s = states[last]; // the according state

     std::set<int> trans = s.getEpsilonTransitions();
     if(trans.empty()){  // dead end
        return false;
     }
     
     // first, look whether one of the follow states closes the loop
     typename std::set<int>::iterator it1;
     for(it1=trans.begin(); it1!=trans.end();it1++){
        int next = *it1;
        int index = getVectorIndex<int>(loop, next);
        if(index >= 0) {
           std::vector<int> part(loop.begin()+index, loop.end());
           loop = part;
           return true;
        }
     }

     // ok, no loop found, extend the loop
     for(it1 = trans.begin(); it1!=trans.end(); it1++){
        loop.push_back(*it1);
        if(findEpsilonLoop(loop)){
          return true;
        } else {
          loop.pop_back();
        }
     } 
     // no loop found
     return false;
   }

/*
~mergeEpsilonLoop~

Merges states given in s by a single state.

*/
  void mergeEpsilonLoop(const std::vector<int>& loop){


     // create a state for the merged states
     State<sigma> S;
     std::vector<State<sigma> > newStates;

     newStates.push_back(S);



     // compute the new positions for each state
     std::map<int, int> posmap;
     int offset = 1; // 1 for the new state
     for(unsigned int i=0;i<states.size();i++){
       if(contains<int>(loop,i)){
         posmap[i] = 0;
         offset--;
       } else {
         posmap[i] = i + offset;
       }
     }


     // handle transitions and epsilon transitions
     for(unsigned int i=0;i<states.size();i++){
        State<sigma> s = states[i];
        if(contains<int>(loop,i)){
           std::map<sigma, std::set<int> > transitions = s.getTransitions();
           typename std::map<sigma, std::set<int> >::iterator it1;

           // process normal transitions
           for(it1 = transitions.begin(); it1!=transitions.end(); it1++){
              typename std::set<int>::iterator it2; 
              for(it2=it1->second.begin(); it2!=it1->second.end(); it2++){
                 newStates[0].insertTransition(it1->first, posmap[*it2]);
              }
           }
 
           // process epsilon transitions
           std::set<int> epsTransitions = s.getEpsilonTransitions();
           typename std::set<int>::iterator it3;
           for(it3 = epsTransitions.begin(); it3!=epsTransitions.end(); it3++){
              if(posmap[*it3] != 0){
                newStates[0].insertEpsilonTransition(posmap[*it3]);
              }
           }
        } else {
          State<sigma> newState;

          // process normal transitions
          std::map<sigma, std::set<int> > transitions = s.getTransitions();
          typename std::map<sigma,std::set<int> >::iterator it1;
          for(it1= transitions.begin(); it1!=transitions.end(); it1++){
              typename std::set<int>::iterator it2;
              for(it2=it1->second.begin();it2!=it1->second.end(); it2++){
                  newState.insertTransition(it1->first, posmap[*it2]);
              }
           }

          // process epsilon transitions
          std::set<int> epsTransitions = s.getEpsilonTransitions();
          typename std::set<int>::iterator it3;
          for(it3=epsTransitions.begin();it3!=epsTransitions.end(); it3++){
             newState.insertEpsilonTransition(posmap[*it3]);
          }
  
          // insert the newly created state
          newStates.push_back(newState);
        }
     }
   
     states = newStates;
     startState = posmap[startState];
     
     // process finalStates
     std::set<int> newFinal;
     typename std::set<int>::iterator it4;   
     for(it4=finalStates.begin();it4!=finalStates.end(); it4++){
        newFinal.insert(posmap[*it4]);
     }
     finalStates=newFinal;


  }



/*
~removeUnreachable~

This function removes all states from the NFA which cannot be reached from the
current start state. This can occur for example after removing epsilon 
transitions.

*/
 bool removeUnreachable(){
   if(states.size()==0){
     return false; // no change
   }
   if(startState>=static_cast<int>(states.size())){
      states.clear();
      finalStates.clear();
      startState = 0;
      return true;
   }
   // there is at least one reachable state
   std::vector<int> unreachable;
   getUnreachable(unreachable);
   bool res =  unreachable.size()>0;
   removeStates(unreachable); 
   return res;
 }


/*
~removeDeadEnds~

This function removes all states from the automatone from which never an 
final state can be reached.

*/

bool removeDeadEnds(){
   if(finalStates.empty() || states.empty() ){ 
     // the automaton recognize nothing
     bool res = states.size() > 0;
     states.clear();
     startState = -1;
     return res;
   }
   std::set<int> finalPossible;
   getFinalPossible(finalPossible);
   if(finalPossible.find(startState)==finalPossible.end()){
     // from the start state never a final state can be reached
     states.clear();
     startState = -1;
     return true;
   }
   std::vector<int> deadEnds;
   for(unsigned int i=0;i<states.size();i++){
      if(finalPossible.find(i)==finalPossible.end()){
         deadEnds.push_back(i);
      }
   }
   bool res = deadEnds.size() > 0;
   removeStates(deadEnds);
   return res;
}



/*
~getFinalPossible~


*/

void getFinalPossible(std::set<int>& initial){
  initial = finalStates;
  unsigned int size = initial.size();
  do{
    size = initial.size();
    for(unsigned int i=0;i<states.size();i++){
      if(initial.find(i)==initial.end()){
         std::set<int> trans = states[i].getTargets(true);
         if(intersects<int>(trans,initial)){
            initial.insert(i);
         }
      }
    }

  } while(size!=initial.size());

}








/*
~removeStates~

Removes all states given within the vector;

*/

void  removeStates(std::vector<int>& toRemove){

   std::sort(toRemove.begin(), toRemove.end(), std::greater<int>() );     
 
   int lastState = -1;
   for(unsigned int i=0;i<toRemove.size();i++){
      if(lastState!=toRemove[i]){
         removeState(toRemove[i]);
         lastState = toRemove[i];
      }
   }
}

void removeState(int state1){
   if(state1<0){
     return;
   }
   int state = state1;
 
   if( ( state>=static_cast<int>(states.size())) || 
       (state == startState) ){
      return;
   }
   states.erase(states.begin()+state);
   typename std::vector<State<sigma> >::iterator it;
   for(it = states.begin(); it !=states.end(); it++){
      it->removeState(state);
   }
   ::removeState(finalStates, state);

   if(startState>state){
       startState--;
   }
}







 void getUnreachable(std::vector<int>& unreachable){
    std::set<int> reachable;
    getReachable(reachable,startState);
    if(reachable.size() == states.size()){
       // all states are reachable
       return;
    }
    for(unsigned int i=0;i<states.size();i++){
      if(reachable.find(i)==reachable.end()){
        unreachable.push_back(i);
      }
    }
 }


/*
 ~getReachable~

 The function extens the set reachable by all states which can be reached
 by the given state. This function should initially be called with 
 reachable=={} and initial == startState.  

*/

 void getReachable(std::set<int>& reachable, int initial){
   if(reachable.find(initial)!=reachable.end()){
      // initial already processed
      return;
   }
   reachable.insert(initial); // mark as processed
   std::set<int> successors = states[initial].getTargets(true);
   typename std::set<int>::iterator it;
   for(it=successors.begin(); it!=successors.end(); it++){
      getReachable(reachable, *it);
   }
 }


};


/*

2.1 Implementation


*/


/*
2.1.1 Constructors

*/

template<typename sigma>
Nfa<sigma>::Nfa(): states(), finalStates(){
   State<sigma> s1;
   State<sigma> s2;
   s1.insertEpsilonTransition(1); // 0-{}->1
   states.push_back(s1);
   states.push_back(s2);
   startState = 0;
   finalStates.insert(1);
}

template<typename sigma>
Nfa<sigma>::Nfa(const std::vector<sigma>& v): states(), finalStates(){
    State<sigma> s1;
    State<sigma> s2;
    for(size_t i = 0; i< v.size();i++){
      s1.insertTransition(v[i],1);
    }
    states.push_back(s1);
    states.push_back(s2);
    startState = 0;
    finalStates.insert(1);
}



template<typename sigma>
Nfa<sigma>::Nfa(const sigma& c): states(),finalStates(){
   State<sigma> s1;
   State<sigma> s2;
   s1.insertTransition(c,1); // 0-c->1
   states.push_back(s1);
   states.push_back(s2);
   startState = 0;
   finalStates.insert(1);
}

template<typename sigma>
Nfa<sigma>::Nfa(const Nfa<sigma>& src): startState(src.startState),
                                        finalStates(src.finalStates),
                                        states(src.states) {}



/*
2.1.1 Assignment Operator

*/

template<typename sigma>
Nfa<sigma>& Nfa<sigma>::operator=(const Nfa<sigma>& src){
  states = src.states;
  startState = src.startState;
  finalStates = src.finalStates;
  return *this;
}

/*
2.1.2 Destructor

*/

template<typename sigma>
  Nfa<sigma>::~Nfa() {}




template<typename sigma>
void Nfa<sigma>::concat(const Nfa<sigma>& second){
   // first, append second to this
   int num = states.size();
   append(second);

   std::set<int>::iterator it;
   for(it=finalStates.begin();it!=finalStates.end();it++){
      states[*it].insertEpsilonTransition(num + second.startState);
   }
   finalStates.clear();
   for(it = second.finalStates.begin(); it!=second.finalStates.end();it++){
     finalStates.insert( num + *it);
   }
}


/*
~disjunction~


Changes this nfa to represent the disjuntion of this and the argument.

*/
template<typename sigma>
 void Nfa<sigma>::disjunction(const Nfa<sigma>& second){
   int num = states.size();

   append(second);
   
   // create the new start state
   State<sigma> start;
   start.insertEpsilonTransition(startState);
   start.insertEpsilonTransition(num+second.startState);
   states.push_back(start);
   startState = states.size()-1;

   // create the new end state
   State<sigma> end;
   std::set<int>::iterator it;
   for(it = finalStates.begin(); it != finalStates.end(); it++){
      states[*it].insertEpsilonTransition(states.size());
   }
   for(it = second.finalStates.begin(); it !=second.finalStates.end();it++){
     states[num + *it].insertEpsilonTransition(states.size());
   }
   states.push_back(end);
   finalStates.clear();
   finalStates.insert(states.size()-1);

 }

/*
~star~

Changes this nfa orig  such that its represents orig star

*/

template<typename sigma>
  void Nfa<sigma>::star(){
    std::set<int>::iterator it;
    for(it=finalStates.begin();it!=finalStates.end();it++){
      states[*it].insertEpsilonTransition(startState);
    }
    State<sigma> newStart;
    newStart.insertEpsilonTransition(startState);
    newStart.insertEpsilonTransition(states.size()+1);
    for(it=finalStates.begin(); it!=finalStates.end(); it++){
       states[*it].insertEpsilonTransition(states.size()+1);
    }
    State<sigma> newFinal;
    states.push_back(newStart);
    startState = states.size()-1;
    states.push_back(newFinal);
    finalStates.clear();
    finalStates.insert(states.size()-1);
  }


/*
~append~

*/
template<typename sigma>
void Nfa<sigma>::append(const Nfa<sigma>& second){

   typename std::vector<State<sigma> >::const_iterator it;
   int num = states.size();
   for(it = second.states.begin(); it != second.states.end(); it++){
     State<sigma> s = *it;
     s.shift(num);
     states.push_back(s);
   }
}





#endif


