
/*
----
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen, Department of Computer Science,
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
#include <sstream>

namespace regexcreator{

/** Representation of a regular expression as a tree **/

class RegEx{

   enum RegExType {EPSILON,SIMPLE,OR,CONCAT,STAR,PLUS};

  public:

/*

 Creates an epsilon transition 

*/
  RegEx(): type(EPSILON), part1(0), part2(0), value(0){ }

/*
Creates a transition using symbol

*/
  explicit RegEx(const char symbol):
     type(SIMPLE), part1(0), part2(0), value(symbol){ }


/**
 The copy constructor 

**/
  RegEx(const RegEx& src): type(src.type),part1(0), part2(0), value(src.value){
     if(src.part1!=0){
        part1 = new RegEx(*src.part1);
     }
     if(src.part2!=0){
        part2 = new RegEx(*src.part2);
     }
  }

/**

The assignement operator

**/
  RegEx& operator=(const RegEx& src){
     type = src.type;
     value = src.value;
     if(part1) {delete part1, part1=0;}
     if(part2) {delete part2, part2=0;}
     if(src.part1){
       part1 = new RegEx(*src.part1);
     }
     if(src.part2){
       part2 = new RegEx(*src.part2);
     }
     return *this;
  }

/*
~Destructor~

*/
  ~RegEx(){
     if(part1) delete part1;
     if(part2) delete part2;
  }


/** 
Checks whether this reg ex is an epsilon transition 

*/

  bool isEpsilon() const{
    return type==EPSILON;
  }


/*
Check for equality 

*/
  bool equals(const RegEx&  r) const{
     if(type!=r.type) return false;
     if(value!=r.value) return false;
     if(part1==0 && r.part1!=0){
          return false;
     }
     if(part1!=0 && r.part1==0){
          return false;
     }
     if(part2==0 && r.part2!=0){
          return false;
     }
     if(part2!=0 && r.part2==0){
          return false;
     }
     if(part1!=0 && !part1->equals(*r.part1)){
        return false;
     }
     if(part2!=0 && !part2->equals(*r.part2)){
         return false;
     }
     return true;
  }

/*
Creates a new reg ex by connecting two reg ex by or.

*/
  static RegEx makeor(RegEx r1, RegEx r2){

    // A | A = A
    if(r1.equals(r2)){
       return RegEx(r1);
    }

    // A+ | € = A*
    if(r1.type==PLUS && r2.type==EPSILON){
       RegEx p1(*r1.part1);
       return star(p1);
    }
    // A*B | B = A*B
    if(r1.type==CONCAT && r1.part2->equals(r2) && r1.part1->type==STAR ){
        return RegEx(r1);
    }
    // AB* | A = AB*
    if(r1.type==CONCAT && r1.part1->equals(r2) && r1.part2->type==STAR){
        return RegEx(r1);
    }    
    // A+B | B = A*B
    if(r1.type==CONCAT && r1.part1->type==PLUS && r1.part2->equals(r2)){
       return concat(star(*r1.part1->part1),r2);
    }
    // AB+ | A = AB*
    if(r1.type==CONCAT && r1.part1->equals(r2) && r1.part2->type==PLUS){
        return concat(r2, star(*r1.part2->part1));
    }
    // AB | CB = (A | C )B
    if(r1.type==CONCAT && r2.type==CONCAT && r1.part2->equals(*r2.part2)){
       return concat( makeor(*r1.part1,*r2.part1), *r1.part2);
    }
    // AB | AC = A (B | C)
    if(r1.type==CONCAT && r2.type==CONCAT && r1.part1->equals(*r2.part1)){
       return concat(*r1.part1,makeor(*r1.part2,*r2.part2));
    } 

    // switch r1 and r2 and try the same rule again

    RegEx r3(r1);
    r1 = r2;
    r2 = r3;
    // A+ | € = A*
    if(r1.type==PLUS && r2.type==EPSILON){
       RegEx p1(*r1.part1);
       return star(p1);
    }
    // A*B | B = A*B
    if(r1.type==CONCAT && r1.part2->equals(r2) && r1.part1->type==STAR ){
        return  RegEx(r1);
    }
    // AB* | A = AB*
    if(r1.type==CONCAT && r1.part1->equals(r2) && r1.part2->type==STAR){
        return  RegEx(r1);
    }    
    // A+B | B = A*B
    if(r1.type==CONCAT && r1.part1->type==PLUS && r1.part2->equals(r2)){
       return concat(star(*r1.part1->part1),r2);
    }
    // AB+ | A = AB*
    if(r1.type==CONCAT && r1.part1->equals(r2) && r1.part2->type==PLUS){
        return concat(r2, star(*r1.part2->part1));
    }
    // AB | CB = (A | C )B
    if(r1.type==CONCAT && r2.type==CONCAT && r1.part2->equals(*r2.part2)){
       return concat( makeor(*r1.part1,*r2.part1), *r1.part2);
    }
    // AB | AC = A (B | C)
    if(r1.type==CONCAT && r2.type==CONCAT && r1.part1->equals(*r2.part1)){
       return concat(*r1.part1,makeor(*r1.part2,*r2.part2));
    } 

    // damn, a no rule fits
    RegEx res;
    res.type = OR;
    res.part1 = new RegEx(r2);
    res.part2 = new RegEx(r1);
    return res;
  }


/*
builts the concatenation of r1 and r2.

*/
  static RegEx concat(RegEx r1, RegEx r2){
     // €A = A
     if(r1.type==EPSILON){
        return RegEx(r2);
     }
     // A€ = A
     if(r2.type==EPSILON){
        return RegEx(r1);
     }
     // A*A* = A*
     if(r1.type==STAR && r1.equals(r2)){
        return RegEx(r1);
     }
     // AA* = A+
     if(r2.type==STAR && r2.part1->equals(r1)){
        return plus(r1);
     }
     // A*A = A+
     if(r1.type==STAR && r1.part1->equals(r1)){
        return plus(r2);
     }
     RegEx res;
     res.type = CONCAT;
     res.part1 = new RegEx(r1);
     res.part2 = new RegEx(r2); 
     return res;
  }

  static RegEx star(RegEx& r){
    if(r.type==EPSILON){
       return RegEx(r);
    }
    if(r.type==STAR || r.type==PLUS){
       r = *r.part1;    
    }
    RegEx res;
    res.type=STAR;
    res.part1 = new RegEx(r);
    return res;
  }

  static RegEx plus(RegEx& r){
    if(r.type==EPSILON){
       return RegEx(r);
    }
     if(r.type==STAR || r.type==PLUS){
       return RegEx(r);
     }
     RegEx res;
     res.type = PLUS;
     res.part1 = new RegEx(r);
     return res;
  }


  stringstream&  printTo(stringstream& ss){
     bool b;
     switch(type){
        case EPSILON : ss << "€"; break;
        case SIMPLE  : ss << value; break;
        case OR      : part1->printTo(ss) << "|" ;
                       part2->printTo(ss);
                       break;
        case  CONCAT : b = part1->type==OR;
                       if(b) ss << "(";
                       part1->printTo(ss);
                       if(b)  ss << ")";
                       b = part2->type==OR;
                       if(b) ss << "(";
                       part2->printTo(ss); 
                       if(b) ss << ")";
                       break;
        case STAR :    b = part1->type!=EPSILON 
                                && part1->type!=SIMPLE;
                       if(b) ss << "(";
                       part1->printTo(ss);
                       if(b) ss << ")";     
                       ss << "*";
                       break; 
        case PLUS :    b = part1->type!=EPSILON 
                                && part1->type!=SIMPLE;
                       if(b) ss << "(";
                       part1->printTo(ss);
                       if(b) ss << ")";     
                       ss << "+";
                       break; 

     }
     return ss;
  }

  private:

   RegExType type;
   RegEx* part1;
   RegEx* part2;
   char value; 

};



class Edge{

  public:

  Edge(int _target, string label):
    target(_target), regex() {
    if(label.size()>0){
       regex = RegEx(label[0]);
    } 
  }

  Edge(const Edge& e): target(e.target), regex(e.regex) {}

  Edge(const int _target, const RegEx& _regex):
   target(_target), regex(_regex){
  }
 
  Edge& operator=(const Edge& e){
     target = e.target;
     regex = e.regex;
     return  *this;
  }

  ~Edge(){}


  RegEx getLabel() const{
    return regex;
  }
 
  int getTarget() const {
     return target;
  }

  private:

   int target;
   RegEx regex;
};



class DeaRegEx{


 public:


/*
Creates an empty DEA 

*/
    DeaRegEx(): successors(), predecessors(), loops(), start(1), finals() {}

/*
Copy constructor 

*/
    DeaRegEx(const DeaRegEx& src):
      successors(src.successors), predecessors(src.predecessors), 
                 loops(src.loops),
      start(src.start), finals(src.finals){}

/*
Destructor

*/

    ~DeaRegEx(){
        for(size_t i=0;i<successors.size();i++){
             if(successors[i]){
                 delete successors[i];
                 delete predecessors[i];
                 delete loops[i];
             }
        }
     }

/*
Assigment operator 

*/
    DeaRegEx& operator=(const DeaRegEx& src){
      successors = src.successors;
      predecessors = src.predecessors;
      loops = src.loops;
      start = src.start;
      finals = src.finals;
      return *this;
    }

/*
Sets the start state of this automaton

*/
     void setStart(const int s){
         start = s;
     }


/*
Adds a final state to this automaton

*/
     void addFinal(const int i){
        finals.push_back(i);
     }


/*
Add a transition to this automaton.

*/

    void addEdge(int source, int target, char label){
      addEdge(source,target, RegEx(label));
    }

     void addEdge(int source, int target, RegEx label){

    // extend vectors for non existing source
    for(int i=successors.size();i<=source;i++){
          successors.push_back(0);
          predecessors.push_back(0);
          loops.push_back(0); // epsilon loop
    }
    // extend vectors for non existing target
    for(int i=successors.size();i<=target;i++){
      successors.push_back(0);
      predecessors.push_back(0);
      loops.push_back(0);
    }
    if(!successors[source]){
      successors[source] = new vector<Edge>();
      predecessors[source] = new vector<Edge>();
      loops[source] = new RegEx();
    }
    if(!successors[target]){
      successors[target] = new vector<Edge>();
      predecessors[target] = new vector<Edge>();
      loops[target] = new RegEx();
    }
    if(source!=target){
       successors[source]->push_back(Edge(target,label));
       predecessors[target]->push_back(Edge(source,label));
    } else {
      RegEx* l = loops[source];
      if(l->isEpsilon()){
        delete loops[source];
        loops[source] = new RegEx(label);
      } else {
        RegEx l2 = RegEx::makeor(*l,label);
        delete loops[source];
        loops[source] = new RegEx(l2);
      }
    }
 }


 RegEx* computeRegEx(const bool  usePrio){

    if(successors.size()==0u || finals.size()==0u || start<0u 
       || (uint32_t)start >= successors.size()
      || successors[start]==0u){
       return 0;
    }

    // add new start and new end if required
    if(    predecessors[start]->size()>0     // link to start
        || finals.size()>0                  // more than one final
        || successors[finals[0]]->size()>0   // final has successors
              || !loops[start]->isEpsilon()        // start contains a loop
                || !loops[finals[0]]->isEpsilon()){  // final contains a loop
       extend();
    }


    // now, the automaton has exactly one start state without back edge to it
    // and exactly one final state having no back edges from it

    // replace parallel edges by regular expressions

    uint32_t f = finals[0];
    const uint32_t start = (uint32_t) this->start;
    
    for(size_t i=0;i<successors.size();i++){
       removeParallel(i);
    }
    

    // remove all nodes except s and f from the automaton 
    if(!usePrio){
       for(size_t i=0;i<successors.size();i++){
          if(i!=(uint32_t)start && i!=f){      
             removeNode(i);
          }
       }
    } else {
       int nodes = successors.size()-2; // remove all nodes excpet start and end
       while(nodes > 0){
          int index = -1;
          int min = -1;
          // search node where successors * predecsssors + loops is minimal

          for(size_t n=0;n<successors.size();n++){
             if(n!=start && n != f && successors[n]!=0){
                 int ne = successors[n]->size() * predecessors[n]->size();
                 if(!loops[n]->isEpsilon()){
                    ne++;
                 }
                 if(index < 0 || ne < min){
                    index = n;
                    min = ne;
                 }
             }
          }
          removeNode(index);
          nodes--; 
       }
    }

    // now, all nodes except start and final are removed

    removeParallel(start);


    return new RegEx(successors[start]->at(0).getLabel());
 } 

 
 private:

  vector<vector<Edge>* > successors;
  vector<vector<Edge>* > predecessors;  
  vector<RegEx*> loops;  
  int start;
  vector<int> finals;

  
/*
Adds a new start and a new final state to this automaton.

*/
  void extend(){
    int s = successors.size();
    int f = s+1;
    RegEx eps;
    addEdge(s,start,eps);

    for(size_t i=0;i<finals.size();i++) {
        int of = finals[i];
        addEdge(of,f,eps);
    }
    start = f-1;
    finals.clear();
    finals.push_back(f);
  }

/*
Merges parallel edges in the automaton

*/
  void removeParallel(){
     for(size_t i=0;i<successors.size();i++){
        removeParallel(i);
     }
  }


/*
Removes node ~node~ from the automaton

*/
  void removeNode(int node){
     vector<Edge>* suc = successors[node];
     vector<Edge>* pred = predecessors[node];
     RegEx* loop1 = loops[node];
     RegEx  loop = RegEx::star(*loop1);
     successors[node] = 0;
     predecessors[node] = 0;
     loops[node] = 0; 

     for(size_t p=0;p<pred->size();p++){
        Edge pe = pred->at(p);
        removeTarget(successors[pe.getTarget()],node);
        for(size_t s=0;s<suc->size();s++){
           Edge su = suc->at(s);
           if(p==0){
               removeTarget(predecessors[su.getTarget()],node);
           }
           int src = pe.getTarget();
           int target = su.getTarget();
           RegEx peLab = pe.getLabel();
           RegEx suLab = su.getLabel();
           RegEx complete = RegEx::concat(RegEx::concat(peLab,loop),suLab);
           addEdge(src,target,complete);
           removeParallel(src);
        }
     }
     delete suc;
     delete pred;
     delete loop1;
  }

  void removeTarget(vector<Edge>* v, int target){
     for(int i=v->size()-1;i>=0;i--){
         if(v->at(i).getTarget()==target){
           v->erase(v->begin()+i);
         }
     }
  }


  // merges edges starting at src and 
  void removeParallel(int src){
     vector<Edge>* suc = successors[src];
     map<int,RegEx> amap;
     for(size_t i=0;i<suc->size();i++){
        Edge e = suc->at(i);
        int t = e.getTarget();
        if(amap.find(t) == amap.end()){
           amap[t] = e.getLabel();
        } else {
           RegEx label = e.getLabel();
           amap[t] = RegEx::makeor(amap[t],e.getLabel()); 
        }
     }
     vector<Edge>* nsuc = new vector<Edge>();
     map<int,RegEx>::iterator it;
     for(it = amap.begin();it!=amap.end();it++){
        nsuc->push_back(Edge(it->first,it->second));
        int target = it->first;
        removeTarget(predecessors[target],src);
        predecessors[target]->push_back(Edge(src,it->second)); 
     }
     delete successors[src];
     successors[src] = nsuc;
  }

};


} // end of namespace regexcreator
