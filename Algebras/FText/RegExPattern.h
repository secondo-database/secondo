

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


1 class RegExPattern

This class is the implementation of a finite automaton. 



*/

#define NUMCHARS 256

#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "GenericTC.h"
#include "IntNfa.h"


int parseRegEx(const char* argument, IntNfa** T);

class RegExPattern: public Attribute{
 public:

/*
1.1 Standard Constructor

Must be nothing.  Never cal it outside the cast function.

*/  
   RegExPattern() {}

/*
1.2 Constructors

*/
   RegExPattern(bool defined): Attribute(defined),
                               numOfStates(0),
                               transitions(0),
                               finalStates(0) {}

   RegExPattern(const RegExPattern& p): Attribute(p),
                                       numOfStates(p.numOfStates),
                                       transitions(p.transitions.Size()),
                                       finalStates(p.finalStates.Size()){
      p.transitions.copyTo(transitions);
      p.finalStates.copyTo(finalStates);
  }
               
  RegExPattern& operator=(const RegExPattern& p){
     Attribute::operator=(p);
     numOfStates = p.numOfStates;
     p.transitions.copyTo(transitions);
     p.finalStates.copyTo(finalStates);
     return *this;
  }

/*
1.3 Destructor

*/
  ~RegExPattern() {}


/*
1.5 Functions for acting as an Attribute 

*/

   virtual int NumOfFLOBs() const{
    return 2;
   }
  
   Flob* GetFLOB(int index) {
      if(index==0) return &transitions;
      if(index==1) return &finalStates;
      assert(false);
   }   

   size_t Sizeof() const{
      return sizeof(*this);
   }

   int Compare(const Attribute* right) const{
      RegExPattern* p = (RegExPattern*) right;
      if(numOfStates<p->numOfStates) return -1;
      if(numOfStates>p->numOfStates) return 1;
      if(transitions.Size() < p->transitions.Size()){
         return -1;
      }
      if(transitions.Size() > p->transitions.Size()){
         return 1;
      }
      if(finalStates.Size() < p->finalStates.Size()){
         return -1;
      }
      if(finalStates.Size() > p->finalStates.Size()){
         return 1;
      }
      for(int i=0;i<transitions.Size();i++){
         int tt; int tp;
         transitions.Get(i,tt);
         p->transitions.Get(i,tp);
         if(tt<tp) return -1;
         if(tt>tp) return 1;
      }
      for(int i=0;i<finalStates.Size();i++){
         bool tf; bool pf;
         finalStates.Get(i,tf);
         p->finalStates.Get(i,pf);
         if(tf<pf) return -1;
         if(tf>pf) return 1;
      }
      return 0;
   }

   bool Adjacent(const Attribute* r) const{
   // there is no meaningful implementation for it
   return false;
   }

   Attribute* Clone() const{
     return new RegExPattern(*this);
   }

   size_t HashValue() const{
     return numOfStates + transitions.Size() + finalStates.Size();
   }

   void CopyFrom(const Attribute* right){
        operator=(*((RegExPattern*) right));
   }

/*
1.5 Functions supporting Geneic Type Constructors

*/

   static const string BasicType(){
     return "regex";
   }

   static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
   }

    
   static ListExpr Property(){
     return gentc::GenProperty("-> DATA",
                           BasicType(),
                          "(numStates (listOfTransitions) (listOfFinalStates)",
                          "(a big list)");
   }

   static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
   }


/*
1.6 ConstructFrom

Creates a DFA from a string describing a regular expression. 
If the string is not a valid regular expression, the dfa is set to
be undefined.

*/

   bool constructFrom(const string& regex){
      IntNfa* nfa;
      if(parseRegEx(regex.c_str(),&nfa)!=0){
         SetDefined(false);
         return false;
      } 
      nfa->nfa.makeDeterministic();
      nfa->nfa.minimize();        
      nfa->nfa.bringStartStateToTop();
      transitions.clean();
      finalStates.clean();
      numOfStates = nfa->nfa.numOfStates();
      for(int i=0;i<numOfStates;i++){
         bool isFinal = (nfa->nfa.isFinalState(i));
         finalStates.Append(isFinal);
      }
      for(int i=0;i<numOfStates;i++){
        State<int> s = nfa->nfa.getState(i);
        for(int c=0;c<NUMCHARS;c++){
           std::set<int> t = s.getTransitions(c);
           if(t.empty()){
              transitions.Append(-1);
           } else {
              assert(t.size()==1);
              transitions.Append(*t.begin());
           }
        } 
      }
      delete nfa;
      SetDefined(true);
      return true;
   }


/*
1.6 matches

Checks whether the argument is regognized by this dfa.

*/
   bool matches(const string& text){
      if(!IsDefined()){
        return false;
      }
      if((finalStates.Size()==0) || (numOfStates==0)){
        return false;
      }
      int state = 0;
      const char* tc = text.c_str();
      for(size_t i=0;i<text.length();i++){
         state = nextState(state,(unsigned char)tc[i]);
         if(state<0){
           return false;
         }
      }
      bool isFinal;
      finalStates.Get(state,isFinal);
      return isFinal;
   }

/*
1.7 starts

Checks whether the argument starts with a word of the language represented
by this dfa.

*/
   bool starts(const string& text){
      if(!IsDefined()){
        return false;
      }
      if((finalStates.Size()==0) || (numOfStates==0)){
        return false;
      }
      int state = 0;
      const char* tc = text.c_str();
      bool isFinal;
      for(size_t i=0;i<text.length();i++){
         finalStates.Get(state,isFinal);
         if(isFinal){
            return true;
         }   
         state = nextState(state,(unsigned char)tc[i]);
         if(state<0){
           return false;
         }
      }
      finalStates.Get(state,isFinal);
      return isFinal;
   }

/*
1.7 readFrom

Constructs a dfa from a nested list. If the nested list is not a 
valid representation of a dfa, this object is set to be undefined.

*/

   bool ReadFrom(const ListExpr value, const ListExpr typeInfo){

     if(listutils::isSymbolUndefined(value)){
        SetDefined(false);
        return true;
     }

     if(nl->AtomType(value)==StringType){
        return constructFrom(nl->StringValue(value));
     }

     if(nl->AtomType(value)==TextType){
        return constructFrom(nl->Text2String(value));
     }

     transitions.clean();
     finalStates.clean();
     SetDefined(true);      
     if(!nl->HasLength(value,3)){
       SetDefined(false);
       return false;
     }     
     if(nl->AtomType(nl->First(value))!=IntType){
        SetDefined(false);
        return false;
     }
     int ns = nl->IntValue(nl->First(value));
     numOfStates=ns;
     for(int i=0;i<ns*NUMCHARS;i++){
         transitions.Append(-1);
     } 
     for(int i=0;i<ns;i++){
       finalStates.Append(false);
     }
     ListExpr T = nl->Second(value);
     while(!nl->IsEmpty(T)){
       ListExpr t = nl->First(T);
       T = nl->Rest(T);
       if(!nl->HasLength(t,3)){
         SetDefined(false);
         return false;
       }
       if(nl->AtomType(nl->First(t)!=IntType) ||
          nl->AtomType(nl->Second(t)!=IntType) ||
          nl->AtomType(nl->Third(t)!=IntType)){
          SetDefined(false);
          return false;
       }
       int source = nl->IntValue(nl->First(t));
       int arg   = nl->IntValue(nl->Second(t));
       int target = nl->IntValue(nl->Third(t));
       if(arg<0 || arg >= NUMCHARS){
          SetDefined(false);
          return false;
       }
       if(source <0 || source >= numOfStates || target<0 || target >=NUMCHARS){
         SetDefined(false);
         return  false;
       }
       transitions.Put(source*NUMCHARS+arg,target);
     }
     ListExpr F = nl->Third(value);
     while(!nl->IsEmpty(F)){
       ListExpr f = nl->First(F);
       F = nl->Rest(F);
       if(nl->AtomType(f)!=IntType){
          SetDefined(false);
          return false;
       }
       int fi = nl->IntValue(f);
       if(fi<0 || fi>=ns){
          SetDefined(false);
          return false;
       }
       finalStates.Put(fi,true);
     }
     return true;
   }

/*
1.8 toListExpr 

Returns the representation of this dfa as a nested list.

*/

   ListExpr ToListExpr(const ListExpr typeInfo){
     if(!IsDefined()){
        return listutils::getUndefined();
     }
     ListExpr ns = nl->IntAtom(numOfStates);
     ListExpr tr = nl->TheEmptyList();
     ListExpr last;
     bool first = true;
     for(int i=0;i<transitions.Size();i++){
         int t;
         transitions.Get(i,t);
         if(t>=0){
           ListExpr trans = nl->ThreeElemList(nl->IntAtom(i/NUMCHARS),
                                              nl->IntAtom(i%NUMCHARS),
                                              nl->IntAtom(t));
           if(first){
             tr = nl->OneElemList(trans);
             last = tr;
             first = false;
           } else {
             last = nl->Append(last,trans);
           }
         }
     } 
     ListExpr fin;
     first = true;
     for(int i=0;i<finalStates.Size();i++){
        bool isf;
        finalStates.Get(i,isf);
        if(isf){
          if(first){
             fin = nl->OneElemList(nl->IntAtom(i));
             last = fin;
             first = false;
          } else {
             last = nl->Append(last,nl->IntAtom(i));
          }
        }
     }
     return nl->ThreeElemList(ns,tr,fin);
   }




 private: 
   int numOfStates;            // number of states
   DbArray<int>  transitions;  // transitions size is NUMCHARS * numberofStates
   DbArray<bool> finalStates;  // bitvector true for a final state

/*
1.9 nextState

returns the next state by given current state and character

*/
   int nextState(const int state, const int c) const{
      assert(state >= 0 && state < numOfStates);
      assert(c>=0 && c<=NUMCHARS);
      int pos = state*NUMCHARS+c;
      int res;
      transitions.Get(pos,res);
      return res;
   }


};




