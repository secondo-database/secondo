
/*

----
This file is part of SECONDO.

Copyright (C) 2017, 
University in Hagen, 
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


//[_] [\_]

*/

#ifndef MGRAPHCOMMON_H
#define MGRAPHCOMMON_H

#include "MainMemoryExt.h"
#include "RelationAlgebra.h"
#include "NestedList.h"
#include "SecondoCatalog.h"
#include "SecondoSystem.h"
#include "MEdge.h"

#include <string>
#include <vector>
#include <list>

namespace mm2algebra{

class MGraphCommon : public MemoryObject{
  protected:
    typedef std::pair<std::list<MEdge>,std::list<MEdge> > alist;

   public:
     MGraphCommon(const bool _flob, 
                  const std::string& _database,
                  const std::string& _type,
                  const int _sourcePos,
                  const int _targetPos,
                  const int _costPos): 
                  MemoryObject(_flob,_database,_type),
                  sourcePos(_sourcePos), 
                  targetPos(_targetPos),
                  costPos(_costPos){
        ListExpr k;
        if(!nl->ReadFromString(_type,k)){
          std::cerr << "Invalid type description " << _type << endl;
          assert(false);
        }
        // remove mem(mgraphX ...
        k = nl->Second(nl->Second(k)); 
        SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
        k = ctlg->NumericType(k);
        tt = new TupleType(k);
     }

     ~MGraphCommon(){
         tt->DeleteIfAllowed();
      }
        
     size_t numSuccessor(size_t vertex){
        if(vertex>=graph.size()){
          return 0;
        }
        return graph[vertex].first.size();
     }
 

     size_t numPredecessor(size_t vertex){
        if(vertex>=graph.size()){
          return 0;
        }
        return graph[vertex].second.size();
     }

     bool insertGraphEdge(Tuple* ginfo){
        CcInt* Source = (CcInt*) ginfo->GetAttribute(sourcePos);
        CcInt* Target = (CcInt*) ginfo->GetAttribute(targetPos);
        CcReal* Costs = (CcReal*) ginfo->GetAttribute(costPos);
        if(!Source->IsDefined() || !Target->IsDefined()
           || !Costs->IsDefined()){
           return false;
        }
        int source = Source->GetValue();
        int target = Target->GetValue();
        double costs = Costs->GetValue();
        if(source < 0 || (size_t)source>=graph.size()) return false;
        if(target < 0 || (size_t)target>=graph.size()) return false;
        if(costs<0) return false;
        if(flob) ginfo->bringToMemory();
        MEdge e(source, target, costs, ginfo);
        graph[source].first.push_back(e);
        graph[target].second.push_back(e);
        return true;
     }

     size_t numVertices() const{
       return graph.size();
     }


     class edgeIterator{
        public:
           friend class MGraphCommon;
           Tuple* next(){
              while(vit!=g->graph.end()){
                 if(eit!=eite){
                    Tuple* res = eit->info;
                    eit++;
                    res->IncReference();
                    return res;               
                 }
                 vit++;
                 if(vit != g->graph.end()){
                   eit = vit->first.begin();
                   eite = vit->first.end();
                 }              
              }
              return 0;
           }
           


        private:
           std::vector<alist>::iterator vit;
           std::list<MEdge>::iterator eit;
           std::list<MEdge>::iterator eite;
           MGraphCommon* g;

           edgeIterator(MGraphCommon* _g){
              g = _g;
              vit = g->graph.begin();
              if(vit != g->graph.end()){
                 eit = vit->first.begin();
                 eite = vit->first.end();
              }              
           }

     };

     edgeIterator* getEdgeIt(){
       return new edgeIterator(this);
     }


     class singleNodeIterator{
        friend class MGraphCommon;
        public:
           Tuple* next(){
             if(it!=list->end()){
               Tuple* res = it->info;
               res->IncReference();
               it++;
               return res;
             }
             return 0;
           }
        private:
           std::list<MEdge>* list;
           std::list<MEdge>::iterator it;

           singleNodeIterator( std::list<MEdge>* _list){
              this->list = _list;
              it = list->begin();
           } 
     };

     singleNodeIterator* getSuccessors(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return 0;
        }
        return new singleNodeIterator(&(graph[v].first));
     }
     
     singleNodeIterator* getPredecessors(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return 0;
        }
        return new singleNodeIterator(&(graph[v].second));
     }

     const std::list<MEdge>& getSuccList(int vertex){
         return graph[vertex].first;
     }
     

     int succCount(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return -1;
        }
        return graph[v].first.size();
     }

     int predCount(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return -1;
        }
        return graph[v].second.size();
     }

     bool disconnect(int vertex){
        if(vertex<0 || (size_t)vertex >=graph.size()){
          return false;
        }
        alist& vlist = graph[vertex];
        // remove vertex from the predecessors list of its successors
        std::list<MEdge>::iterator it;
        for(it = vlist.first.begin(); it!=vlist.first.end();it++){
           MEdge& edge = *it;
           assert(edge.source==vertex);
           removeSource(graph[edge.target].second,vertex);  
        }

        // remove vertex from successors list of its predecessors
        for(it = vlist.second.begin(); it!=vlist.second.end();it++){
           MEdge& edge = *it;
           assert(edge.target==vertex);
           removeTarget(graph[edge.source].first,vertex);  
        }

        // clean lists of vertex
        vlist.first.clear();
        vlist.second.clear();
        return true;
     }

     void components(std::vector<int>& v){
         v.clear();
         std::vector<tarjanInfo> v2;
         tarjan(v2);
         for(size_t i=0;i<v2.size();i++){
           v.push_back(v2[i].compNo);
         }
     }
 

  protected:
    std::vector<alist> graph;
    TupleType* tt;
    int sourcePos;
    int targetPos;
    int costPos;

    void setPositions(int s, int t, int c){
      sourcePos = s;
      targetPos = t;
      costPos = c;
    }


    
    class tarjanInfo{
      public:
        tarjanInfo():
          seen(false), index(0), lowlink(0),
          inStack(false), compNo(-1){} 
        bool seen;
        int index;
        int lowlink;
        bool inStack; 
        int compNo; 
    };


    void tarjan(std::vector<tarjanInfo>& tarjanVector) {

      tarjanVector.clear();
      for(size_t i = 0; i< graph.size();i++){
          tarjanInfo ti;
          tarjanVector.push_back(ti);
      }
      std::stack<int> tarjanstack;
      int index = 0;
      int compNo = 1;
      for(size_t i=0;i<tarjanVector.size();i++){
        if(!tarjanVector[i].seen){
          tarjan(i,tarjanVector, index,tarjanstack, compNo);
        }
      }
    } 

    void tarjan(int vertex, std::vector<tarjanInfo>& tarjanVector,
                int& index, std::stack<int>& stack, int& compNo){
      
        // pair of vertex number and iterator in successor list
        typedef std::pair<size_t, std::list<MEdge>::iterator > stackentry; 
        std::stack<stackentry> rstack; // recursion stack
        rstack.push(stackentry(vertex,graph[vertex].first.begin()));

        while(!rstack.empty()){
           stackentry e = rstack.top();
           rstack.pop();
           vertex = e.first;
           std::list<MEdge>::iterator pos = e.second;
           if(!tarjanVector[vertex].seen 
              || pos != graph[vertex].first.begin()){ 
              if(!tarjanVector[vertex].seen ){ // first time visited
                tarjanVector[vertex].index = index;
                tarjanVector[vertex].lowlink = index;
                index++;
                tarjanVector[vertex].inStack = true;
                tarjanVector[vertex].seen  = true;
                stack.push(vertex);
              }
              if(pos != graph[vertex].first.end()){ 
                 // not the end of vertex's successors
                 int w = pos->target;
                 pos++;
                 rstack.push(stackentry(vertex,pos));
                 if(!tarjanVector[w].seen){
                    rstack.push(stackentry(w,graph[w].first.begin()));
                 } else  if(tarjanVector[w].inStack) {
                    tarjanVector[vertex].lowlink = std::min( 
                                              tarjanVector[vertex].lowlink,
                                              tarjanVector[w].index);      
                 }
              } else { // end of successors
                 std::list<MEdge>::iterator sit;
                 for(sit=graph[vertex].first.begin(); 
                     sit!=graph[vertex].first.end(); sit++){
                   int w = sit->target;
                   if(tarjanVector[w].compNo<0){
                      tarjanVector[vertex].lowlink = std::min(
                                                  tarjanVector[vertex].lowlink,
                                                  tarjanVector[w].lowlink);
                   }
                 }
                 if(tarjanVector[vertex].lowlink == tarjanVector[vertex].index){
                    assert(tarjanVector[vertex].compNo<0);
                    tarjanVector[vertex].compNo = compNo;
                    while(true){
                        int w = stack.top();
                        stack.pop();
                        tarjanVector[w].inStack=false;
                        if(w==vertex)
                           break;
                        assert(tarjanVector[w].compNo<0);
                        tarjanVector[w].compNo = compNo;
                    }
                    compNo++;
                 }
              }
           }
        } // while 
    } // tarjan


  private:
    void removeSource(std::list<MEdge>& l, int s){
        std::list<MEdge> nl;
        std::list<MEdge>::iterator it;
        for(it = l.begin();it!=l.end();it++){
           if(it->source!=s){
               nl.push_back(*it);
           }
        }
        std::swap(l,nl);
    }
    void removeTarget(std::list<MEdge>& l, int s){
        std::list<MEdge> nl;
        std::list<MEdge>::iterator it;
        for(it = l.begin();it!=l.end();it++){
           if(it->target!=s){
               nl.push_back(*it);
           }
        }
        std::swap(l,nl);
    }


};


} // end of namespace mm2algebra

#endif

