
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


1 class dfa

This class is the implementation of a finite automaton. 



*/

#include <vector>
#include <string>
#include <algorithm>

#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "TopRel.h"
#include "NestedList.h" 
#include "IntNfa.h"
#include "GenericTC.h"

/*
1.1 Auxiliary Functions



~lengthSmaller~


This function checks whether the length of the first argument string is smaller
than the length of the second argument string.

*/


bool lengthSmaller( const string& s1, const string& s2){
   return s1.length() < s2.length();
}

/*
~lengthiGreater~


This function checks whether the length of the first argument string is greater
than the length of the second argument string.

*/
bool lengthGreater( const string& s1, const string& s2){
   return s1.length() > s2.length();
}


/*
~num2str~

This function converts an integer to a string.

*/
string num2str(const int num){
 stringstream s;
 s << num;
 return s.str();

}


/*
1.2 some forward declarations

*/

int yyparse();


int parseString(const char* arg, IntNfa** res);



/*
2. Class ~TopRelDfa~

This class provides a finite automaton where the alphabet is provided by the
members of a predicategroup. This means, if your predicategroup contains
the topological relationships inside, meet, and disjoint, these the 
tokens are the alphabet of the automaton.


*/


class TopRelDfa : public Attribute{

public:

/*
Constructors

*/


/*
~Standard Constructor~

This constructor should only be used within  the cast function.

*/

  TopRelDfa():Attribute(),canDestroy(false) {}


/*
~Constructor~

This constructor creates an empty finite automaton.

*/
  TopRelDfa(const int dummy): Attribute(false),
                              startState(-1),
                              currentState(-1),
                              transitions(0),
                              finalStates(0),
                              symbol2Cluster(0),
                              cluster2Symbol(0),
                              predicateGroup(0),
                              numOfSymbols(0),
                              canDestroy(false) {}

/*
~Copy Constructor~

*/

   TopRelDfa(const TopRelDfa& d):
     Attribute(d.IsDefined()),
     startState(d.startState),
     currentState(d.currentState),
     transitions(0),
     finalStates(0),
     symbol2Cluster(0),
     cluster2Symbol(0),
     predicateGroup(0),
     numOfSymbols(d.numOfSymbols),
     canDestroy(false) {
      transitions.copyFrom(d.transitions);
      finalStates.copyFrom(d.finalStates);
      predicateGroup.CopyFrom(&d.predicateGroup);
    }


/*
~Assignment Operator~

*/

   TopRelDfa& operator=(const TopRelDfa& src){
     Attribute::operator=(src);
     SetDefined(src.IsDefined());
     startState = src.startState;
     currentState = src.currentState;
     transitions.copyFrom(src.transitions);
     finalStates.copyFrom(src.finalStates);
     cluster2Symbol.copyFrom(src.cluster2Symbol);
     symbol2Cluster.copyFrom(src.symbol2Cluster);
     predicateGroup.CopyFrom(&src.predicateGroup);
     numOfSymbols = src.numOfSymbols;
     return *this;
   }


/*
~setTo~

Sets this automaton to be able to recognize the given regular expression
when using the names for topological relationships from the predicate group.


*/

  void setTo(const string& regex,
             const toprel::PredicateGroup& pg) {
    
   /* special case, predicate group is not defined,
      set this automaton to be undefined and exit.
   */
    if(!pg.IsDefined()){
      SetDefined(false);
      return;
    }  


    /*
      Next, we assign each name to a number. 
      To be able to process string which a an prefix from another name,
      the names are sorted by its length decreasing.
      We replace all names of topological relationships by its number within
      the regular expression. 
    */

    std::vector<string> names = pg.getNames();
    // sort vector by the  length of its elements
    std::sort(names.begin(), names.end(), lengthGreater); 



    string changed = regex;
    for(unsigned int i=0;i<names.size();i++){
      string res = " " + num2str(i)+" ";
      changed = replaceAll(changed,names[i],res); 
    }

    // replaces all dots by all possible symbols
    if(names.size()>0){
       string repl = "( ";
       for(unsigned int i=0;i< names.size(); i++){
          if(i>0){
             repl += " | ";
          }
          repl += num2str(i);
       } 
       repl += " )";
       changed = replaceAll(changed, ".", repl);
     }




    symbol2Cluster.clean();
    cluster2Symbol.clean();
    symbol2Cluster.resize(names.size());
    cluster2Symbol.resize(names.size());

    for(unsigned int i=0;i< names.size();i++){
       int clusterNumber = pg.getClusterNumber(names[i]);
       assert(clusterNumber>=0);
       cluster2Symbol.Put(clusterNumber, i);
       symbol2Cluster.Put(i,clusterNumber);
    }


    /*
     Now, we create a non-deterministic finite automaton from the
     regular expression. Because we have replaced all names by numbers,
     we can use an nfa which deals with integer numbers as alphabet.

    */
    IntNfa* nfa = 0;
    parseString(changed.c_str(),&nfa);


    /*
      If there is no return value, parsing of the regular expression is
      failed. In this case, we set the automaton to be undefined and
      exit.
    */ 
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

    nfa->nfa.minimize();

    nfa->nfa.bringStartStateToTop();

    /*
      The nfa has no non-deterministic parts at this place.
      We copy the main memory structure from nfa to this object.
    */ 
    SetDefined(true);


    // remove old stuff from db arrays
    transitions.clean();
    finalStates.clean();
 
    for(unsigned int i=0; i< nfa->nfa.numOfStates(); i++){
      finalStates.Append(nfa->nfa.isFinalState(i));
      State<int> state = nfa->nfa.getState(i);
      for(unsigned int j = 0; j < names.size(); j++){
        std::set<int> targets = state.getTransitions(j);
        if(targets.empty()){
          transitions.Append(-1); // mark an error
        } else {
          assert(targets.size()==1);
          transitions.Append(*targets.begin());
        }
      }
    }



    predicateGroup.CopyFrom(&pg);

    startState = nfa->nfa.getStartState();
    numOfSymbols = names.size();
    delete nfa;


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

  void next(int clusternum){
     if((currentState < 0) || (startState<0)){
        return;
     }
     int symbol;
     cluster2Symbol.Get(clusternum,symbol);
     int index = numOfSymbols*currentState + symbol;
     transitions.Get(index,currentState);
  }



/*
1.4 Attribute functions

*/


 int NumOfFLOBs(){
    return predicateGroup.NumOfFLOBs() + 4;
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
   } else if (index==1) {
     return &finalStates;
   } else if(index==2){
     return &symbol2Cluster;
   } else if(index==3){
     return &cluster2Symbol;
   }
   return 0;
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


 static const string BasicType(){
    return "mtoprel";
 }


 static ListExpr Property(){
     return gentc::GenProperty("-> DATA",
                           BasicType(),
                          "(complicated list)",
                          "(a big list)");
 }

 static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
 }


/*
~ReadFrom~

Reads the value of this automaton from the given listexpr.
If the list does not represent a valid dfa, this will be set
to be undefined.  

*/

 bool ReadFrom(const ListExpr value, const ListExpr typeInfo){
    if(nl->ListLength(value)!=5){
      SetDefined(false);
      return  false;
    }
    ListExpr current = nl->First(value);
    if(nl->AtomType(current!=IntType)){
      SetDefined(false);
      return false;
    }
    startState = nl->IntValue(current);
    currentState = startState;

    current = nl->Second(value);
    if(nl->AtomType(current)!=NoAtom){
       SetDefined(false);
       return false;
    }
    transitions.clean();
    while(!nl->IsEmpty(current)){
       ListExpr first = nl->First(current);
       if(nl->AtomType(first)!=IntType){
         SetDefined(false);
         return false;
       }
       transitions.Append(nl->IntValue(first));
       current = nl->Rest(current);
    }

    finalStates.clean();
    current = nl->Third(value);
    while(!nl->IsEmpty(current)){
       ListExpr first = nl->First(current);
       if(nl->AtomType(first)!=BoolType){
         SetDefined(false);
         return false;
       }
       finalStates.Append(nl->BoolValue(first));
       current = nl->Rest(current);
    }

    ListExpr typeInfo1 = nl->SymbolAtom("predicategroup");

    if(!predicateGroup.ReadFrom(nl->Fourth(value), typeInfo1)){
       SetDefined(false);
       return false;
    }
    current = nl->Fifth(value);
    if(nl->AtomType(current)!=IntType){
       SetDefined(false);
       return false;
    }
    numOfSymbols= nl->IntValue(current);
    if(!numOfSymbols * finalStates.Size() == transitions.Size()){
       SetDefined(false);
       return false;
    }
    SetDefined(true);
    return true;

 }


/*
~ToListExpr~

Produces a listexpr for the value of this dfa.


*/

 ListExpr ToListExpr(const ListExpr typeInfo){
    if(!IsDefined()){
      return nl->SymbolAtom("undef");
    }
    ListExpr first = nl->IntAtom(startState);
    ListExpr second;
    if(transitions.Size()==0){
       second = nl->TheEmptyList();
    } else {
      int x;
      transitions.Get(0,x);
      second = nl->OneElemList(nl->IntAtom(x));
      ListExpr last = second;
      for(int i=1; i< transitions.Size();i++){
        transitions.Get(i,x);
        last = nl->Append(last, nl->IntAtom(x));
      }
    }

    ListExpr third;
    if(finalStates.Size()==0){
      third = nl->TheEmptyList();
    } else {
      bool x;
      finalStates.Get(0,x);
      third = nl->OneElemList(nl->BoolAtom(x));
      ListExpr last = third;
      for(int i=1; i< finalStates.Size();i++){
        finalStates.Get(i,x);
        last = nl->Append(last, nl->BoolAtom(x));
      }
    }

    ListExpr typeInfo1 = nl->SymbolAtom("predicategroup");
    ListExpr fourth = predicateGroup.ToListExpr(typeInfo1);
    ListExpr fifth = nl->IntAtom(numOfSymbols);


    return nl->FiveElemList(first, second, third, fourth, fifth);
 }


 ostream& Print( ostream& o ) const{
   if(!IsDefined()){
      o << "undefined";
      return o;
   }

    o << "DFA" << endl;
    o << "start state " << startState << endl;
    o << "current state : " << currentState << endl;
    o << "state \t symbol \t next state " << endl;
    o << "num of symbols" << numOfSymbols << endl;
    for(int i=0;i<transitions.Size() ; i++){
        int next;
        transitions.Get(i, &next);
        int start = i / numOfSymbols;
        int symb = i % numOfSymbols;
        int clusterNum;
        symbol2Cluster.Get(symb,clusterNum);
        toprel::Cluster c;
        predicateGroup.getCluster(clusterNum,c);
        if(next >=0){
            bool finalstart;
            finalStates.Get(start,finalstart);
            string finalstartMark = finalstart?"(*) ":"( ) ";
            bool finalnext;
            finalStates.Get(next,finalnext);
            string finalnextMark = finalnext?"(*) ":"( ) ";
            o << finalstartMark << start << " \t "  << c.GetName()  
              << " \t\t " << next << finalnextMark << endl;
        }
    }    
    return o;

 }






private:

   int startState;             // start state
   int currentState;           // current state
   DbArray<int> transitions;   // transition table
   DbArray<bool> finalStates;  // marks an state to be final
   DbArray<int> symbol2Cluster; // mapping from symbol number to cluster number
   DbArray<int> cluster2Symbol; // mapping from cluster number to symbol number
   toprel::PredicateGroup predicateGroup;
   int numOfSymbols;           // size of sigma
   bool canDestroy;
};
