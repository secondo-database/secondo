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
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "NestedList.h"
#include "SecondoCatalog.h"
#include "SecondoSystem.h"
#include "MEdge.h"
#include "StopWatch.h"

#include <string>
#include <vector>
#include <list>
#include <mmheap.h>
//#include <priority_queue>

namespace mm2algebra{
    
/*
1 Class ~shortCutInfo~

This is an auxiliary class for graph contraction.

*/
class shortCutInfo{
      public:
        shortCutInfo(size_t _source, size_t _target, double _cost,
                     size_t _middle):
           source(_source), target(_target), cost(_cost), middle(_middle){}
        size_t source;
        size_t target;
        double cost;
        size_t middle;
};

/*
2 Class ~MGraphCommon~

This class represents a graph object.

*/

class MGraphCommon : public MemoryObject{
  protected:
    typedef std::pair<std::list<MEdge>,std::list<MEdge> > alist;

   public:

/*
2.1 constructor

*/
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
        memSize += tt->GetTotalSize();
     }

/*
2.2 Destructor

*/
     ~MGraphCommon(){
         tt->DeleteIfAllowed();
         memSize = 0; 
      }
        
/*
2.3 ~numSuccessors~

Returns the number of successors of a node.

*/
     size_t numSuccessor(size_t vertex)const{
        if(vertex>=graph.size()){
          return 0;
        }
        return graph[vertex].first.size();
     }

/*
2.4 ~numPredecessors~

Returns the number of predecessors of a node.

*/ 

     size_t numPredecessor(size_t vertex) const{
        if(vertex>=graph.size()){
          return 0;
        }
        return graph[vertex].second.size();
     }

/*
2.5 ~insertGraphEdge~

Inserts an edge to this graph. Source, Target and costs are
taken from the argument. If one of these valus is undefined or
outside the allowed range, false is returned and the graph
is keepts as before. Otherwise, the edge is inserted into the
graph.

*/

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
        memSize += ginfo->GetSize();
        return true;
     }

/*
2.6 ~inserts~

Inserts an edge to this graph.

*/
     void insert(MEdge& e){
        int source = e.source;
        int target = e.target;
        assert(source>=0);
        assert(target>=0);
        assert((size_t)source<graph.size()); 
        assert((size_t)target<graph.size());
        graph[source].first.push_back(e);
        graph[target].second.push_back(e);
        memSize += sizeof(MEdge);
     }


/*
2.7 ~numVertices~

Returns the number of vertices of this graph.

*/
     size_t numVertices() const{
       return graph.size();
     }


/*
2.8 Auxiliary class ~edgeIterator~

*/
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

     }; // end of class edgeIterator

/*
2.9 ~getEdgeIt~

Returns an iterator over all edges of this graph.

*/

     edgeIterator* getEdgeIt(){
       return new edgeIterator(this);
     }


/*
2.10 Auxiliary class ~singleNodeIterator~

*/
     class singleNodeIterator{
        friend class MGraphCommon;
        public:
           virtual Tuple* next(){
             if(it!=list->end()){
               Tuple* res = it->info;
               res->IncReference();
               it++;
               return res;
             }
             return 0;
           }
           virtual ~singleNodeIterator(){}

        private:
           std::list<MEdge>* list;
           std::list<MEdge>::iterator it;

           singleNodeIterator( std::list<MEdge>* _list){
              this->list = _list;
              if(list!=nullptr){
                it = list->begin();
              }
           } 
     }; // end of class singleNodeIterator
     
     class singleNodeDeleteIterator : public singleNodeIterator{
        friend class MGraphCommon;
        public:
  
         virtual Tuple* next(){
            if(vlist.empty()){
               return nullptr;
            }
            MEdge e = vlist.front();
            vlist.pop_front();
            if(e.info){
              e.info->IncReference();
            }
            return e.info;
           }
  
        private:
           std::list<MEdge> vlist;

           singleNodeDeleteIterator( int _vertex,
                               MGraphCommon* _graph,
                               bool _successors):singleNodeIterator(0){
              if(_successors){
                std::swap(vlist,_graph->graph[_vertex].first);
              } else {
                std::swap(vlist,_graph->graph[_vertex].second);
              }
              delBack(vlist,_graph,_successors,_vertex);
           } 

           void delBack(std::list<MEdge>& l, 
                        MGraphCommon* g, 
                        bool succ,
                        int v){
              std::list<MEdge>::iterator it;
              for(it = l.begin();it!=l.end(); it++){
                MEdge e = *it; 
                if(succ){
                   MGraphCommon::removeSource(g->graph[e.target].second,v);
                } else {
                   removeTarget(g->graph[e.source].first,v);
                }   
              } 
           }
     }; // end of class singleNodeDeleteIterator

/*
2.11 ~getSuccessors~

Returns an iterator over all edges of this graph having ~v~ as source.

*/
     singleNodeIterator* getSuccessors(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return 0;
        }
        return new singleNodeIterator(&(graph[v].first));
     }

     singleNodeIterator* getSuccessors(int v, bool delOption){
        if(v<0 || (size_t)v >= graph.size()){
           return 0;
        }
        if(!delOption){
           return new singleNodeIterator(&(graph[v].first));
        }
        return new singleNodeDeleteIterator(v,this,true);
     }
     
/*
2.12 ~getPredecessors~


Returns an iterator over all edges of this graph having ~v~ as target.


*/

     singleNodeIterator* getPredecessors(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return 0;
        }
        return new singleNodeIterator(&(graph[v].second));
     }

     singleNodeIterator* getPredecessors(int v, bool delOption){
        if(v<0 || (size_t)v >= graph.size()){
           return 0;
        }
        if(!delOption){
           return new singleNodeIterator(&(graph[v].second));
        }
        return new singleNodeDeleteIterator(v,this,false);
     }

/*
2.13 ~getSuccList~

Returns the list of successorts of ~vertex~

*/
     const std::list<MEdge>& getSuccList(int vertex){
         return graph[vertex].first;
     }
     

/*
2.14 ~succCount~

Returns the numbers of successors of a node.

*/
     int succCount(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return -1;
        }
        return graph[v].first.size();
     }

/*
2.16 ~predCount~

Returns the number of predecessors of a node.

*/
     int predCount(int v){
        if(v<0 || (size_t)v >= graph.size()){
           return -1;
        }
        return graph[v].second.size();
     }


/*
2.17 ~removeAllEdges~

Removes all Edges from Source to target.

*/
     void removeAllEdges(int source, int target){
       if(source< 0 || source >= (int) graph.size()) return;
       if(target< 0 || target >= (int) graph.size()) return;
       removeTarget(graph[source].first,target);
       removeSource(graph[target].second,source);
       memSize -= sizeof(MEdge);
     }
   




/*
2.17. ~disconnect~

Removes all edges from and to a specified node.

*/
     bool disconnect(int vertex){
        if(vertex<0 || (size_t)vertex >=graph.size()){
          return false;
        }
        alist& vlist = graph[vertex];
        // remove vertex from the predecessors list of its successors
        std::list<MEdge>::iterator it;
        for(it = vlist.first.begin(); it!=vlist.first.end();it++){
           MEdge& edge = *it;
           //assert(edge.source==vertex);
           removeSource(graph[edge.target].second,vertex);  
        }

        // remove vertex from successors list of its predecessors
        for(it = vlist.second.begin(); it!=vlist.second.end();it++){
           MEdge& edge = *it;
           //assert(edge.target==vertex);
           removeTarget(graph[edge.source].first,vertex);  
           memSize -= sizeof(MEdge);
        }

        // clean lists of vertex
        vlist.first.clear();
        vlist.second.clear();
        return true;
     }


/*
2.18 ~components~

Assigns a component number to each node.

*/
     void components(std::vector<int>& v){
         v.clear();
         std::vector<tarjanInfo> v2;
         tarjan(v2);
         for(size_t i=0;i<v2.size();i++){
           v.push_back(v2[i].compNo);
         }
     }



/*
2.19 ~contract~

This operator crontracts this graph. 

*/
     size_t contract(int maxPrio, int minBlockSize, 
                     int maxHopsF, int maxHopsB,
                     std::vector<shortCutInfo>& allShortCuts,
                     int variant,
                     size_t skipRecalculate,
                     const size_t maxEdges){
       
       std::vector<uint64_t> finished(graph.size(),0);
       uint64_t finishedMark = 1;
       if(variant == 1){
         return simpleContraction1(maxPrio, minBlockSize, maxHopsF,
                                   maxHopsB, allShortCuts, 
                                   skipRecalculate, maxEdges,
                                   &finished, finishedMark);
       } else {
         return simpleContraction2(maxPrio, minBlockSize, maxHopsF,
                                   maxHopsB, allShortCuts,
                                   skipRecalculate, maxEdges,
                                   &finished, finishedMark);
       }
     }

/*
2.20 ~pathCosts~

Computes the length of the minimum path between ~source~ and ~target~.
The search is restricted to a certain number of hops from source and from 
target. The forbidden node is not used. If the maximum costs are reached,
the search terminates as well. Using negative number for the number of
hops and the forbidden node, then the search spans the whole graph.

*/
     double pathCosts(const int source, const int target, 
                      const int maxHopsForward, const int maxHopsBackward,
                      const int forbidden, 
                      double maxCosts = std::numeric_limits<double>::max()){
          minPathCosts mpc;
          if(maxCosts<=0){
            maxCosts = std::numeric_limits<double>::max();
          }
          return mpc(this,source, target, maxHopsForward, maxHopsBackward, 
                     maxCosts, forbidden, false, 
                     std::numeric_limits<size_t>::max(),0,0);
     }


     struct ddsgedge{
      ddsgedge(const size_t _target,
               const size_t _cost,
               const size_t _dir): target(_target), cost(_cost),dir(_dir){}

      size_t target;
      size_t cost;
      size_t dir; // 0 both, 1 forward, 2 backward

      void print(std::ostream& o, size_t source){
        o << source << " " << target << " " << cost << " " << dir << std::endl; 
      }

     };

     bool exportDDSG(std::string& filename, const double scaleCost){
       std::ofstream out(filename.c_str(), std::ios::out | std::ios::trunc);
       if(!out){
         return false;
       }
       std::vector<std::vector<ddsgedge> > edges;
       size_t noEdges = fillddsg(edges, scaleCost);
       out << "d" << endl;
       out << edges.size() << " " << noEdges << endl;
       for(size_t i=0;i<edges.size();i++){
          std::vector<ddsgedge>& v = edges[i];
          for(size_t j=0;j<v.size();j++){
              v[j].print(out,i);
          }
       }
       out.close();
       return true;
     }

     uint32_t fillddsg(std::vector< std::vector<ddsgedge> >& result, 
                       double scale){
        result.clear();
        for(size_t i=0;i<graph.size();i++){
           std::vector<ddsgedge> v;
           result.push_back(v);
        }
        uint32_t noEdges = 0;
        for(size_t i=0;i<graph.size();i++){
           std::list<MEdge> successors = graph[i].first;
           std::list<MEdge>::iterator it;
           for( it = successors.begin(); it!=successors.end(); it++){
              size_t source = i;
              MEdge e = *it;
              size_t target = e.target;
              if(source != target){
                size_t cost = (size_t)((e.costs*scale)+0.5);
                size_t dir = 1;
                if(source > target){
                   std::swap(source,target);
                   dir = 2;
                }    
                std::vector<ddsgedge>& v = result[source];
                bool found = false;
                for(size_t e=0;e<v.size();e++){
                   ddsgedge& d = v[e];
                   if(d.target==target){
                      found = true;
                      if(d.cost > cost){
                         d.cost = cost;
                      }
                      if(d.dir!=dir){
                         d.dir = 0;
                      }
                   }
                }
                if(!found){
                   v.push_back(ddsgedge(target,cost,dir));
                   noEdges++;
                }
             }
           }           
        }
        return noEdges;
     }


     std::ostream& print(std::ostream& out, std::vector<std::string>* names, 
                         bool printBackward){

       out << "Graph " << endl;
       if(names != nullptr){
          out << "sourcePos : " << sourcePos << endl;
          out << "targetPos : " << targetPos << endl;
          out << "costPost  : " << costPos   << endl;
       }
       for(size_t node = 0; node < graph.size() ; node++){
          printNode(out, node, graph[node], names, printBackward); 
       }
       return out;
     }


  protected:
    std::vector<alist> graph; // the graph representation
    TupleType* tt;
    int sourcePos;      // positions within the tuples
    int targetPos;
    int costPos;
    std::vector<int> nodeOrder; // for contraction


    static void printNode(std::ostream& out, size_t nodeNumber,
                          alist& edges, std::vector<std::string>* names,
                          bool backward){

       out << " --- Node " << nodeNumber << " --- " << endl;
       auto fit = edges.first.begin();
       if(backward){
           out << "   --- outgoing edges " << endl;
       }
       while( fit != edges.first.end()){
          fit->print(out, names);
          fit++;
          out << endl;
       }
       if(backward){
          out << endl;  
          out << "   --- incoming  edges " << endl;
          auto bit = edges.second.begin();
          while(bit != edges.second.end()){
            bit->print(out,names);
            bit++;
            out << endl;
          }
       }
       out << endl;
   }



/*
2.21 ~setPositions~

Sets the attribute positions for the edge tuples where to find 
the source, the target and the costs.

*/
    void setPositions(int s, int t, int c){
      sourcePos = s;
      targetPos = t;
      costPos = c;
    }


/*
2.22 Auxiliary class ~tarjanInfo~

This class helps to compute the strongly connected components.

*/
    
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

/*
2.23 ~tarjan~

This function computes the connected components. Each node is assigned to a 
component number.

*/

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


/*
2.24 ~tarjan~

Auxiliary method to compute connnected components.

*/
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

/*
2.28 ~removeSource~

Removes all edges from ~l~ with source = ~s~.

*/

    static void removeSource(std::list<MEdge>& l, int s){
        std::list<MEdge> nl;
        std::list<MEdge>::iterator it;
        for(it = l.begin();it!=l.end();it++){
           if(it->source!=s){
               nl.push_back(*it);
           }
        }
        //std::swap(l,nl);
        l = nl;
    }
/*
2.29 ~removeTarget~

Removes all edges from ~l~ with target = ~s~.

*/
    static void removeTarget(std::list<MEdge>& l, int s){
        std::list<MEdge> nl;
        std::list<MEdge>::iterator it;
        for(it = l.begin();it!=l.end();it++){
           if(it->target!=s){
               nl.push_back(*it);
           }
        }
        //std::swap(l,nl);
        l = nl;
    }


/*
2.30 ~createEdge~

Creates a new MEdge from the given infos.

*/
    static MEdge createEdge(Tuple* templ, int source, 
                            int target, double cost){
     /*
      Tuple* t = new Tuple(tt);
      for(int i=0;i<t->GetNoAttributes();i++){
           Attribute* a;
           if(i==sourcePos){
             a = new CcInt(true,source);
           }  else if(i==targetPos){
             a = new CcInt(true,target);
           } else if(i==costPos){
             a = new CcReal(true,cost);
           } else {
             a = templ->GetAttribute(i)->Clone();
             a->SetDefined(false);
           }
           t->PutAttribute(i,a);
      }
      */
      Tuple* t = 0;
      MEdge e(source, target,cost,t);
      //t->DeleteIfAllowed();
      return e;
   }



/*
2.31 ~struct queueentry~

Structure supporting dijkstra algorithm.


*/
   // computes the costs for all targets that can be reached within maxHops hops
   // without using the node forbidden.
   struct queueentry{
      queueentry(size_t _node, double _cost, size_t _depth):
       node(_node), cost(_cost), depth(_depth){
      }
      bool operator<(const queueentry& e) const{
         return cost > e.cost;
      }

      size_t node;
      double cost;
      size_t depth;
   };


/*
2.32 queueentryComp

This is a comparator class for queueentry.

*/
  class queueentryComp{
    public:
       bool operator()(const queueentry& f, const queueentry& s) const {
          return f.cost < s.cost;
       }    
  };

   //typedef mmheap::mmheap<queueentry, queueentryComp> queue_t;
   typedef std::priority_queue<queueentry> queue_t;
   typedef mmheap::mmheap<queueentry,queueentryComp> queue_t2;


/*
2.33 ~processNode~

Auxiliary function for computing multi-target-path-costs.

*/
   template<class Q> 
   void processNode(queueentry e, size_t forbidden, 
                    std::map<size_t,double>& targets,
                    size_t maxHops, size_t reached, 
                    std::vector<uint64_t>* finished,
                    uint64_t& finishedMark,
                    Q& front, size_t& processedEdges,
                    const size_t maxEdges
                    ){

      // check wether node has already been processed
      if( (*finished)[e.node] == finishedMark){
         return;
      }

      // mark node as finished
      (*finished)[e.node] = finishedMark;
      // check whether node is one of the target nodes
      std::map<size_t,double>::iterator it = targets.find(e.node);
      if(it!=targets.end()){
         it->second = e.cost;
         reached--;
         if(reached==0) return;
      }
      // check whether maxHops has been reached
      if(e.depth >= maxHops){
         return;
      }
      // process edges
      if(processedEdges >= maxEdges){ // early stop
          return;
      }
      std::list<MEdge>& succs =  graph[e.node].first;
      std::list<MEdge>::iterator sit;
      for(sit = succs.begin(); 
         (sit!=succs.end()) && (processedEdges < maxEdges);
         sit++){
         MEdge& me = *sit;
         size_t t = me.target;
         if( (t!=forbidden) && ((*finished)[t] != finishedMark)){
            processedEdges++;
            queueentry et(t, e.cost + me.costs,e.depth+1);
            front.push(et);
         } 
      }
   }

/*
2.34 ~conputeCosts~

Computes the costs from source to all targets in the graph.
The search stops if the number of hops, the maximum costs, or
the maximum number of edges is reached. Unreached targets will 
have maximum costs.

*/
   void computeCosts(size_t source, size_t forbidden, 
                     std::map<size_t, double>& targets, 
                     double maxCost, size_t maxHops,
                     size_t maxEdges,
                     std::vector<uint64_t>* finished,
                     uint64_t finishedMark){
       queueentry e(source,0,0);
       queue_t2 front;
       //std::set<size_t> finished;
       front.push(e);
       size_t edges = 0;
       size_t reached = targets.size();
       while(!front.empty() && reached>0){
          queueentry e = front.top();
          front.pop();
          if(e.cost > maxCost){
             return;
          }
          processNode(e, forbidden, targets, maxHops, reached, 
                      finished,finishedMark,front, 
                      edges, maxEdges);
       }
   }


/*
2.35 ~computeShortCuts1~

This operator computes the shortcuts if a given node would be 
contracted. This variant computes the costs from a source
to all targets in a single step.

*/
   void computeShortCuts1(size_t node, size_t maxHops, 
                         std::vector<MEdge>& result,
                         const size_t maxEdges,
                         std::vector<uint64_t>* finished,
                         uint64_t& finishedMark){
       // collect all target nodes in a set
       result.clear();
       std::list<MEdge>& preds = graph[node].second;
       std::list<MEdge>& succs = graph[node].first;
       typedef std::list<MEdge>::iterator edgeIt;
       std::map<size_t, std::map<size_t,double> > cand;
       std::map<size_t,double> maxCost;

       size_t candsize = 0;
      
       // collect all candidates and there costs 
       for(edgeIt predIt = preds.begin(); predIt!=preds.end(); predIt++){
          MEdge& inEdge = *predIt;
          size_t source = inEdge.source;
          double sc = inEdge.costs;
          std::map<size_t, double> cmap;
          double max = 0;
          for(edgeIt succit = succs.begin();succit!=succs.end();succit++){
             MEdge& outEdge = *succit;
             size_t target = outEdge.target;
             if(source != target){
                std::map<size_t,double>::iterator it = cmap.find(target);
                double cost = sc + outEdge.costs;
                if(cost>max){
                   max = cost;
                }
                if(it==cmap.end()){
                  cmap[target] = cost;
                }  else  if(it->second > cost){
                   it->second = cost;
                }
             }
          }
          if(cmap.size()>0){
             cand[source] = cmap;
             maxCost[source] = max;
          }
          candsize += cmap.size();
       }
      
       // all potential shortcuts are collected in cand
       Tuple* tuple = 0;
       if(cand.size()>0){
         tuple = graph[node].first.begin()->info;
       }

       std::map<size_t, std::map<size_t,double> >::iterator sit;
       for(sit= cand.begin(); sit!=cand.end();sit++){
          int source = sit->first;
          double costs = maxCost[source];
          std::map<size_t,double>::iterator tit;
          std::map<size_t,double>  targets;
          for(tit =  sit->second.begin(); tit!=sit->second.end(); tit++){
             targets[tit->first] = std::numeric_limits<double>::max();
          } 
          // store minPathCost in targets
          computeCosts(source, node, targets, costs, maxHops, maxEdges, 
                       finished, finishedMark);
          finishedMark++;
          if(finishedMark==std::numeric_limits<uint64_t>::max()){
              for(size_t i=0;i<finished->size();i++){
                 (*finished)[i] =0;
              }
              finishedMark = 1;
          }
          //
          for(tit =  sit->second.begin(); tit!=sit->second.end(); tit++){
             if(tit->second < targets[tit->first]){
                // shortcut is shorter than another way
                result.push_back(createEdge(tuple, source, tit->first, 
                                            tit->second));   
             }
          } 
       }
   } 


/*
2.36 computeShortCuts2

This function computes the shortcuts to be inserted into the graph
if the node ~node~ is contracted. This variant computes the shortest
path costs for all combinations of source and target using a 
bidirectional dijkstra restricted by hops and maxEdges.

*/

   void computeShortCuts2(size_t node, size_t maxHopsF, 
                          size_t maxHopsB, std::vector<MEdge>& result,
                          const size_t maxEdges,
                          std::vector<uint64_t>* finished,
                          uint64_t& finishedMark){
       result.clear();
       std::list<MEdge>& preds1  = graph[node].second;
       std::list<MEdge>& succs1 = graph[node].first;
       static minPathCosts mpc;

       // remove duplicates from both lists, keep only such with minimum costs
       std::map<int,MEdge> preds;
       std::list<MEdge>::iterator itp1;

       for(itp1 = preds1.begin(); itp1!=preds1.end(); itp1++){
          std::map<int,MEdge>::iterator i1 = preds.find(itp1->source);
          if(i1==preds.end()){
             preds[itp1->source] = (*itp1);
          } else {
             if(i1->second.costs > itp1->costs){
                preds[itp1->source] = *itp1;
             }
          }
       }
       
       std::map<int,MEdge> succs;
       std::list<MEdge>::iterator itp2;
       for(itp2 = succs1.begin(); itp2!=succs1.end(); itp2++){
          std::map<int,MEdge>::iterator i2 = succs.find(itp2->target);
          if(i2==succs.end()){
             succs[itp2->target] = *itp2;
          } else {
             if(i2->second.costs > itp2->costs){
                succs[itp2->target] = *itp2;
             }
          }
       }
       


       std::map<int,MEdge>::iterator itp;
       for(itp = preds.begin(); itp != preds.end();itp++){
          MEdge& pedge = itp->second;
          double c1 = pedge.costs;
          int s = pedge.source;

          std::map<int,MEdge>::iterator its;
          for(its=succs.begin();its!=succs.end();its++){
            MEdge& sedge = its->second;
            int t = sedge.target;
            double c = c1 + sedge.costs;
            if(s!=t){
               double spc = mpc(this,s,t,maxHopsF, maxHopsB,c,
                                node,true, maxEdges,
                                finished, finishedMark);
               finishedMark++;
               if(finishedMark==std::numeric_limits<uint64_t>::max()){
                  for(size_t i=0;i<finished->size();i++){
                    (*finished)[i] =0;
                  }
                  finishedMark = 1;
               }
               if(spc > c){ // path longer than going over node
                  result.push_back(createEdge(pedge.info, s, t, c));
               }
            }
          }
       }
   }

/*
2.40 ~insertShortCuts~

Inserts all edges in ~edges~ into this graph.

*/
    void insertShortCuts(std::vector<MEdge> & edges){
       for(size_t i = 0; i< edges.size();i++){
           insert(edges[i]);
       }
    }


/*
2.41 Auxiliary class ~simplePrioEntry~

This class represents a queue entry for contraction.

*/

    // simple contraction as done in Script
    class simplePrioEntry{
       public:
          simplePrioEntry(size_t _node): node(_node), prio(0.0)
                         , reinsertions(0){}
          simplePrioEntry(size_t _node, double _prio):
           node(_node), prio(_prio),reinsertions(0){}
          bool operator<(const simplePrioEntry& e) const{
              return prio > e.prio;
          }
          size_t node;
          double prio;
          size_t reinsertions;
       
    };

/*
2.42 ~simplePrioEntryComp~

Comparator class for simplePrioEntry.

*/
  class simplePrioEntryComp{
    public:
       bool operator()(const simplePrioEntry& f, 
                       const simplePrioEntry& s) const {
          return f.prio > s.prio;
       }    
  };


/*
2.42 ~edgeDifferenceA~

Computes the edgeDifference in a simple way. It ignores common 
source and target nodes, i.e., it also counts edges having the same
source and target.

*/
    int edgeDifferenceA(int node) const{
       const std::list<MEdge>& inL = graph[node].second;
       const std::list<MEdge>& outL = graph[node].first;
       int in = inL.size();
       int out = outL.size();
       return in*out - (in + out);
    }


/*
2.43 ~edgeDifference~

Computes the maximum edgeDifference without counting loops of a single node.

*/    
    int edgeDifference(int node) const{
       const std::list<MEdge>& inL = graph[node].second;
       const std::list<MEdge>& outL = graph[node].first;
       if(inL.empty()) return -outL.size();
       if(outL.empty()) return -inL.size(); 

       static std::set<int> inS;
       inS.clear();
       static std::list<MEdge>::const_iterator it;
       for(it = inL.begin();it!=inL.end();it++){
         inS.insert(it->source);
       }
       static std::set<int> outS;
       outS.clear();
       for(it = outL.begin();it!=outL.end();it++){
         outS.insert(it->target);
       }
       int in=0;
       int out=0;
       int inout = 0;
       static std::set<int>::iterator it1;
       it1=inS.begin();
       static std::set<int>::iterator it2;
       it2 = outS.begin();
       while(it1!=inS.end() && it2!=outS.end()){
          if(*it1<*it2){
            in++; it1++;
          } else if(*it1>*it2){
            out++; it2++;
          } else {
            inout++; it1++; it2++;
          }
       }     
       while(it1!=inS.end()){ in++; it1++;}
       while(it2!=outS.end()){ out++; it2++;}


       int create = (in+inout) * (out+inout) - inout;
       int remove = in + out + 2*inout;
       int prio =  create - remove;
       return prio;
    }


/*
2.44 simpleContraction1

This method contracts this graph using a two-step priority computation.

*/

    size_t simpleContraction1(int maxPrio, size_t minBlockSize, int maxHopsF,
                              int maxHopsB, 
                              std::vector<shortCutInfo>& allShortCuts,
                              size_t skipReinsert,
                              const size_t maxEdges,
                              std::vector<uint64_t>* finished,
                              uint64_t& finishedMark){

       //std::cout << "called simple contraction" << std::endl;

       // initialize priority queue with all nodes and prio 0.0
       // typedef mmheap::mmheap<simplePrioEntry,simplePrioEntryComp> queue_t;
        
       typedef std::priority_queue<simplePrioEntry> queue_t;
       queue_t queue;
       for(size_t i=0;i<graph.size();i++){
          simplePrioEntry e(i,-1);
          queue.push(e);
       }
       nodeOrder.clear();
       size_t blockCount = 0;

       size_t cs = 0;
       size_t prog = queue.size() / 100;
       if(prog <2) prog = 2;
       size_t p2 = 0;

       std::cout << "start with queue of size " << queue.size() << std::endl;
       std::cout << "write a dot each " << prog << " processed nodes" << endl;
       
       std::vector<MEdge> shortcuts;
       allShortCuts.clear();

       size_t removedEdges = 0;
       size_t reinits = 0;

       while(!queue.empty()){
          size_t s = queue.size();
          simplePrioEntry e = queue.top();
          queue.pop();
          size_t node = e.node;
          double prio = e.prio;
          int in = numPredecessor(node);
          int out = numSuccessor(node);
          if((in*out > prio) && (s >= skipReinsert)){ // reinsert level 1
             e.prio = in*out;
             queue.push(e);
          } else {
            if(maxHopsB<=0){
               computeShortCuts1(node, maxHopsF, shortcuts, maxEdges, 
                                 finished, finishedMark);
            } else {
               computeShortCuts2(node, maxHopsF, maxHopsB,shortcuts, maxEdges, 
                                 finished, finishedMark);
            }
            if(   (in*out + (shortcuts.size() - (in + out)) > prio)
               && (shortcuts.size() > (size_t)(in+out))
               && (s>=skipReinsert)){
              // reinsert level 2
              e.prio = (in*out + shortcuts.size()) - (in + out);
              queue.push(e);
            } else {
              // this is either the next node to contract or the node will
              // simplify the graph
              // do contraction
              insertShortCuts(shortcuts);
              disconnect(node);
              nodeOrder.push_back(node);
              removedEdges += (in + out);

              /*
              if(shortcuts.size() > (size_t)(in + out)){
                cout << "insert " << shortcuts.size() << " shortcuts " << endl;
                cout << "removed " << in + out << " edges " << endl;
              }
              */
              for(size_t i=0;i<shortcuts.size();i++){
                 MEdge& sc = shortcuts[i];
                 shortCutInfo sci(sc.source, sc.target, sc.costs, node);
                 allShortCuts.push_back(sci);
              }

              cs += shortcuts.size();
              blockCount++;
              if(prio>maxPrio && blockCount > minBlockSize){
                  reinits++;
                  blockCount = 0;
                  queue_t tmp;
                  while(!queue.empty()){
                   simplePrioEntry e = queue.top();
                   queue.pop();
                   e.prio = 0;
                   tmp.push(e);
                  }
                  std::swap(tmp,queue);
                  //queue.swap(tmp);
              }
              p2++; // some progress counter
              if(p2==prog){
                p2=0;
                cout << ".";
                cout.flush();
              }              
            }
          }
       }
       cout << "processed " << nodeOrder.size() << "nodes" << endl;
       cout << "shortcut edges " << allShortCuts.size() << endl;
       cout << "removed edges " << removedEdges << endl;
       cout << "number of reinitializations " << reinits << endl;

       return cs;
    }



/*
2.45 ~simpleContraction2~

Contract this graph using edgeDifference and number of contracted
neighbors as priority.

*/

    size_t simpleContraction2(int maxPrio, size_t minBlockSize, 
                              int maxHopsF, int maxHopsB,
                              std::vector<shortCutInfo>& allShortCuts,
                              size_t skipReinit,
                              const size_t maxEdges,
                              std::vector<uint64_t>* finished,
                              uint64_t& finishedMark){

       //std::cout << "called simple contraction" << std::endl;

       // initialize priority queue with all nodes and prio 0.0
       //typedef mmheap::mmheap<simplePrioEntry,simplePrioEntryComp> queue_t;
       typedef std::priority_queue<simplePrioEntry> queue_t;

       // initialize a vector with number of contracted neighbors
       std::vector<int> contractedNeighbors(graph.size(),0);

       int weight_EdgeDiff = 5;
       int weight_contractedNeighbors = 1;


       cout << "init prio" << endl;
       int minPrio =  1000000;
       int maxPrioQ = -1000000;
       queue_t queue;
       for(size_t i=0;i<graph.size();i++){
          int p = edgeDifference(i) * weight_EdgeDiff;
          simplePrioEntry e(i,p);
          queue.push(e);
          if(minPrio > p) minPrio=p;
          if(maxPrioQ < p) maxPrioQ = p;
       }
       cout << "prio initialized" << endl;
       cout << "minPrio = " << minPrio;
       cout << "maxPrio = " << maxPrioQ; 

       nodeOrder.clear();
       size_t blockCount = 0;

       size_t cs = 0; // number of contraction edges
       size_t prog = queue.size() / 100;
       if(prog <2) prog = 2;
       size_t p2 = 0;

       std::cout << "start with queue of size " << queue.size() << std::endl;
       std::cout << "write a dot each " << prog << " processed nodes" << endl;
       
       std::vector<MEdge> shortcuts;
       allShortCuts.clear();

       size_t removedEdges = 0;
       size_t reinits = 0;
       size_t maxShortcutsPerNode = 0;
       size_t sumreinsertions=0;
       size_t maxreinsertions=0;

       while(queue.size() > 1){
          simplePrioEntry e = queue.top();
          queue.pop();
          size_t node = e.node;
          double prio = e.prio;
          int in = numPredecessor(node);
          int out = numSuccessor(node);
          int extra = edgeDifference(node); // Edge difference 
          int nprio =   weight_EdgeDiff*extra 
                      + weight_contractedNeighbors*contractedNeighbors[node];

          if((nprio > prio) && (queue.size() >= skipReinit)){
            e.prio = nprio;
            e.reinsertions++;
            queue.push(e);
          } else { // do contraction
            sumreinsertions += e.reinsertions;
            if(maxreinsertions < e.reinsertions){
              maxreinsertions = e.reinsertions;
              assert(maxreinsertions < 400); // pure debug code
            }

            if(maxHopsB<=0){
               computeShortCuts1(node, maxHopsF, shortcuts, maxEdges, 
                                 finished, finishedMark);
            } else {
               computeShortCuts2(node, maxHopsF, maxHopsB,shortcuts, maxEdges,
                                 finished, finishedMark);
            }
            insertShortCuts(shortcuts);
            // update number of contracted neighbors
            static std::set<int> neighbors;
            neighbors.clear();
            static std::list<MEdge>::iterator it;
            for(it = graph[node].first.begin(); 
                it!=graph[node].first.end();
                it++){
               int n = it->target;
               if(neighbors.find(n)==neighbors.end()){
                  neighbors.insert(n);
                  contractedNeighbors[n]++;
               }
            }
            for(it = graph[node].second.begin(); 
                it!=graph[node].second.end();
                it++){
               int n = it->source;
               if(neighbors.find(n)==neighbors.end()){
                  neighbors.insert(n);
                  contractedNeighbors[n]++;
               }
            }


            disconnect(node);
            nodeOrder.push_back(node);
            removedEdges += (in + out);

            for(size_t i=0;i<shortcuts.size();i++){
                MEdge& sc = shortcuts[i];
                shortCutInfo sci(sc.source, sc.target, sc.costs, node);
                allShortCuts.push_back(sci);
            }

            cs += shortcuts.size();
            if(maxShortcutsPerNode < shortcuts.size()){
              maxShortcutsPerNode = shortcuts.size();
            }

            blockCount++;
            if( (prio>maxPrio) && (blockCount > minBlockSize)){
               // reinit queue
               reinits++;
               blockCount = 0;
               queue_t tmp;
               while(!queue.empty()){
                 simplePrioEntry e = queue.top();
                 queue.pop();
                 e.prio = weight_EdgeDiff * edgeDifference(e.node) 
                        +weight_contractedNeighbors*contractedNeighbors[e.node];
                 tmp.push(e);
               }
               std::swap(tmp,queue);
            }
            p2++; // some progress counter
            if(p2==prog){
               p2=0;
               cout << ".";
               cout.flush();
             }              
          }
       }
       if(queue.size()>0){ // process the last node
         simplePrioEntry e = queue.top();
         // insert into node order
         nodeOrder.push_back(e.node);
         // remove edges
         graph[e.node].first.clear();
         graph[e.node].second.clear();
       }


       cout << "processed " << nodeOrder.size() << "nodes" << endl;
       cout << "shortcut edges " << allShortCuts.size() << endl;
       cout << "removed edges " << removedEdges << endl;
       cout << "reinitializations " << reinits << endl;
       cout << "maximum shortcuts per node " << maxShortcutsPerNode << endl;
       cout << "sum reinsertions " << sumreinsertions << endl;
       cout << "max reinsertions " << maxreinsertions << endl;
       return cs;
    }



/*
2.46 Auxiliary class ~minPathCosts~

This class can be used to compute the costs between two nodes
using a bidirectional dijkstra.

*/


    class minPathCosts{

      public:
          minPathCosts(){ }
          double operator()(MGraphCommon* _g,
                            const int _source, 
                            const int _target,
                            const int _maxHopsForward,
                            const int _maxHopsBackward,
                            const double _maxCosts,
                            const int _forbidden,
                            const bool  _skipIfSmaller,
                            const size_t _maxEdges,
                            std::vector<uint64_t>* finished,
                            uint64_t finishedMark){

            g = _g;
            source = _source;
            target = _target;
            maxHopsForward = _maxHopsForward;
            maxHopsBackward = _maxHopsBackward;
            maxCosts = _maxCosts;
            forbidden = _forbidden;
            skipIfSmaller = _skipIfSmaller;
            maxEdges = _maxEdges;
            

            double costs = std::numeric_limits<double>::max();
            if(_source < 0 || source>= g->graph.size()){
               return costs;
            }
            if(_target<0 || target>=g->graph.size()){
              return costs;
            }
            if(source == target){
             return 0;
            }
            // clear all structures
            processedForward.clear();
            processedBackward.clear();
            frontForward.clear();
            frontBackward.clear();
            edges = 0;
            return compute(); 
          }


      private:
         size_t source;
         size_t target;
         int maxHopsForward;
         int maxHopsBackward;
         double maxCosts;
         int forbidden;
         std::map<int,double> processedForward;
         std::map<int,double> processedBackward;
         //typedef std::priority_queue<queueentry> queue_t;
         typedef mmheap::mmheap<queueentry, queueentryComp> queue_t;
         queue_t frontForward;
         queue_t frontBackward;
         bool skipIfSmaller;
         size_t edges;
         size_t maxEdges;
         MGraphCommon* g;


         double compute(){
            queueentry ef(source,0,0);
            frontForward.push(ef);    
            queueentry eb(target,0,0);
            frontBackward.push(eb);
            bool done = false;
            double costs = std::numeric_limits<double>::max();

            while((!frontForward.empty() || !frontBackward.empty()) &&  !done){
               bool forward;
               queueentry e(0,0,0);
               if(frontForward.empty()){
                  e = frontBackward.top();
                  frontBackward.pop();
                  forward = false;
               } else if(frontBackward.empty()){
                  e = frontForward.top();
                  frontForward.pop();
                  forward = true;
               } else {
                  queueentry f = frontForward.top();
                  queueentry b = frontBackward.top();
                  if(b.cost > f.cost){
                     e = f;
                     frontForward.pop();
                     forward = true;
                  } else {
                     e = b;
                     frontBackward.pop();
                     forward = false;
                 }
              }
              if(skipIfSmaller){ 
                if(costs < maxCosts){
                  // found a path shorter than maxcosts
                  done = true;
                }
              }
              if(done || e.cost > maxCosts || e.cost > costs){
                 // maxcosts reached or current path cannot shortened
                 done = true;
              } else {
                 processNode(e, forward, costs, done);
              }
           }
           return costs;
         }

         void processNode(queueentry& e, bool forward, double& costs, 
                         bool& done){
            int node = e.node;
            std::map<int,double>* processed = forward?&processedForward
                                                    :&processedBackward;

            if(processed->find(node) != processed->end()){
               return; // already processed
            }
            (*processed)[node] = e.cost;
             
            // check whether we hit a processed node of the other direction
            std::map<int,double>* processedReverse = !forward
                                                     ?&processedForward
                                                     :&processedBackward;
            std::map<int,double>::iterator it = processedReverse->find(e.node);
            if(it != processedReverse->end()){
               double x = it->second + e.cost;
               if(x < costs){
                 costs = x;
               }
            }
            // check whether maxHops are reached
            int maxHops = forward?maxHopsForward:maxHopsBackward;
            if((int)e.depth >= maxHops){
               return;
            }           
            // process edges
            if(edges >= maxEdges){ // maximum number of edges reached
              return;
            }
            std::list<MEdge>& nextNodes = forward?g->graph[node].first
                                                 :g->graph[node].second;
            std::list<MEdge>::iterator itN;
            for(itN=nextNodes.begin(); 
                (itN!=nextNodes.end()) && (edges<=maxEdges); 
                itN++){
               processEdge(*itN,forward,e);
               edges++;
            }  
         }

         void processEdge(MEdge& edge, bool forward, queueentry& e){
            int target = forward?edge.target:edge.source;
            if(target==forbidden) return; // do not use forbidden node
            std::map<int,double>& processed = forward?processedForward
                                                     :processedBackward;
            if(processed.find(target)!=processed.end()){
               return;
            }
            queue_t& front = forward?frontForward
                                                            :frontBackward;
            queueentry ne(target, e.cost + edge.costs, e.depth+1);
            front.push(ne);
         }

    }; // end of class minPathCosts





};


} // end of namespace mm2algebra

#endif

