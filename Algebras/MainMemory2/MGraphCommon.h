
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

#include <string>
#include <vector>
#include <list>
//#include <mmheap.h>
//#include <priority_queue>

namespace mm2algebra{
    

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

     void insert(MEdge& e){
        int source = e.source;
        int target = e.target;
        assert(source>=0);
        assert(target>=0);
        assert((size_t)source<graph.size()); 
        assert((size_t)target<graph.size());
        graph[source].first.push_back(e);
        graph[target].second.push_back(e);
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




     size_t contract(int maxPrio, int minBlockSize, 
                     int maxHopsF, int maxHopsB,
                     std::vector<shortCutInfo>& allShortCuts,
                     int variant){
       if(variant == 1){
         return simpleContraction1(maxPrio, minBlockSize, maxHopsF,
                                   maxHopsB, allShortCuts);
       } else {
         return simpleContraction2(maxPrio, minBlockSize, maxHopsF,
                                   maxHopsB, allShortCuts);
       }
     }


     double pathCosts(const int source, const int target, 
                      const int maxHopsForward, const int maxHopsBackward,
                      const int forbidden, 
                      double maxCosts = std::numeric_limits<double>::max()){
          minPathCosts mpc(this);
          if(maxCosts<=0){
            maxCosts = std::numeric_limits<double>::max();
          }
          return mpc(source, target, maxHopsForward, maxHopsBackward, 
                     maxCosts, forbidden);
     }



  protected:
    std::vector<alist> graph;
    TupleType* tt;
    int sourcePos;
    int targetPos;
    int costPos;
    std::vector<int> nodeOrder;

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


    MEdge createEdge(Tuple* templ, int source, int target, double cost)const{
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
      MEdge e(source, target,cost,t);
      t->DeleteIfAllowed();
      return e;
   }


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

  class queueentryComp{
    public:
       bool operator()(const queueentry& f, const queueentry& s) const {
          return f.cost < s.cost;
       }    
  };

   //typedef mmheap::mmheap<queueentry, queueentryComp> queue_t;
   typedef std::priority_queue<queueentry> queue_t;

   void processNode(queueentry e, size_t forbidden, 
                    std::map<size_t,double>& targets,
                    size_t maxHops, size_t reached, 
                    std::set<size_t>& finished,
                    queue_t& q
                    ){

      // check wether node has already been processed
      if(finished.find(e.node)!=finished.end()){
         return;
      }
      finished.insert(e.node);
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
      std::list<MEdge>& succs =  graph[e.node].first;
      std::list<MEdge>::iterator sit;
      for(sit = succs.begin(); sit!=succs.end();sit++){
         MEdge& me = *sit;
         size_t t = me.target;
         if(t!=forbidden && finished.find(t)==finished.end()){
            queueentry et(t, e.cost + me.costs,e.depth+1);
            q.push(et);
         } 
      }
   }


   void computeCosts(size_t source, size_t forbidden, 
                     std::map<size_t, double>& targets, 
                     double maxCost, size_t maxHops){
       queueentry e(source,0,0);
       queue_t q;
       std::set<size_t> finished;
       q.push(e);
       size_t reached = targets.size();
       while(!q.empty() && reached>0){
          queueentry e = q.top();
          q.pop();
          if(e.cost > maxCost){
             //cout << "abort because maxCosts reached" << endl;
             //cout << "maxCosts : " << maxCost << endl;
             return;
          }
          processNode(e, forbidden, targets, maxHops, reached, finished,q);
       }
 
   }


   void computeShortCuts1(size_t node, size_t maxHops, 
                         std::vector<MEdge>& result){
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
       //  cout << "Candidates : " << candsize << endl;
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
          computeCosts(source, node, targets, costs, maxHops);
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


   void computeShortCuts2(size_t node, size_t maxHopsF, 
                          size_t maxHopsB, std::vector<MEdge>& result){
       result.clear();
       std::list<MEdge>& preds1  = graph[node].second;
       std::list<MEdge>& succs1 = graph[node].first;
       minPathCosts mpc(this);

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
               double spc = mpc(s,t,maxHopsF, maxHopsB,c,node);
               if(spc > c){ // path longer than going over node
                  result.push_back(createEdge(pedge.info, s, t, c));
               }
            }
          }
       }
   }


    void insertShortCuts(std::vector<MEdge> & edges){
       for(size_t i = 0; i< edges.size();i++){
           insert(edges[i]);
       }
    }

    // simple contraction as done in Script
    class simplePrioEntry{
       public:
          simplePrioEntry(size_t _node): node(_node), prio(0.0){}
          bool operator<(const simplePrioEntry& e) const{
              return prio > e.prio;
          }
          size_t node;
          double prio;
       
    };


  class simplePrioEntryComp{
    public:
       bool operator()(const simplePrioEntry& f, 
                       const simplePrioEntry& s) const {
          return f.prio > s.prio;
       }    
  };


    size_t simpleContraction1(int maxPrio, size_t minBlockSize, int maxHopsF,
                              int maxHopsB, 
                              std::vector<shortCutInfo>& allShortCuts){

       //std::cout << "called simple contraction" << std::endl;

       // initialize priority queue with all nodes and prio 0.0
       // typedef mmheap::mmheap<simplePrioEntry,simplePrioEntryComp> queue_t;
        
       typedef std::priority_queue<simplePrioEntry> queue_t;
       queue_t queue;
       for(size_t i=0;i<graph.size();i++){
          simplePrioEntry e(i);
          queue.push(e);
       }
       nodeOrder.clear();
       size_t blockCount = 0;
       //typedef std::pair<MEdge, size_t> contractEdgeT;

       size_t cs = 0;
       size_t prog = queue.size() / 100;
       if(prog <2) prog = 2;
       size_t p2 = 0;

       std::cout << "start with queue of size " << queue.size() << std::endl;
       std::cout << "write a dot each " << prog << " processed nodes" << endl;
       
       std::vector<MEdge> shortcuts;
       allShortCuts.clear();

       size_t removedEdges = 0;

       while(!queue.empty()){
          simplePrioEntry e = queue.top();
          queue.pop();
          size_t node = e.node;
          double prio = e.prio;
          int in = numPredecessor(node);
          int out = numSuccessor(node);
          if(in*out > prio){ // reinsert level 1
             e.prio = in*out;
             queue.push(e);
          } else {
            if(maxHopsB<=0){
               computeShortCuts1(node, maxHopsF, shortcuts);
            } else {
               computeShortCuts2(node, maxHopsF, maxHopsB,shortcuts);
            }
            if(   (in*out + (shortcuts.size() - (in + out)) > prio)
               && (shortcuts.size() > (size_t)(in+out))){
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
                  // reinit queue
                  //cout << "reinit queue" << endl;
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

       return cs;
    }

    size_t simpleContraction2(int maxPrio, size_t minBlockSize, 
                              int maxHopsF, int maxHopsB,
                              std::vector<shortCutInfo>& allShortCuts){

       //std::cout << "called simple contraction" << std::endl;

       // initialize priority queue with all nodes and prio 0.0
       //typedef mmheap::mmheap<simplePrioEntry,simplePrioEntryComp> queue_t;
       typedef std::priority_queue<simplePrioEntry> queue_t;

       queue_t queue;
       for(size_t i=0;i<graph.size();i++){
          simplePrioEntry e(i);
          queue.push(e);
       }
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

       while(!queue.empty()){
          simplePrioEntry e = queue.top();
          queue.pop();
          size_t node = e.node;
          double prio = e.prio;
          int in = numPredecessor(node);
          int out = numSuccessor(node);
          int extra = in*out - (in + out); // Edge difference 
          if(extra > prio){
            e.prio = extra;
            queue.push(e);
          } else { // do contraction
            if(maxHopsB<=0){
               computeShortCuts1(node, maxHopsF, shortcuts);
            } else {
               computeShortCuts2(node, maxHopsF, maxHopsB,shortcuts);
            }
            insertShortCuts(shortcuts);
            disconnect(node);
            nodeOrder.push_back(node);
            removedEdges += (in + out);

            for(size_t i=0;i<shortcuts.size();i++){
                MEdge& sc = shortcuts[i];
                shortCutInfo sci(sc.source, sc.target, sc.costs, node);
                allShortCuts.push_back(sci);
            }

            cs += shortcuts.size();
            blockCount++;
            if(prio>maxPrio && blockCount > minBlockSize){
                  // reinit queue
                  //cout << "reinit queue" << endl;
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
       cout << "processed " << nodeOrder.size() << "nodes" << endl;
       cout << "shortcut edges " << allShortCuts.size() << endl;
       cout << "removed edges " << removedEdges << endl;
       return cs;
    }





    class minPathCosts{

      public:
          minPathCosts(MGraphCommon* _g){
             g = _g;
          }
          double operator()(const int _source, 
                            const int _target,
                            const int _maxHopsForward,
                            const int _maxHopsBackward,
                            const double _maxCosts,
                            const int _forbidden){


            source = _source;
            target = _target;
            maxHopsForward = _maxHopsForward;
            maxHopsBackward = _maxHopsBackward;
            maxCosts = _maxCosts;
            forbidden = _forbidden;
            

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
            processedForward.clear();
            processedBackward.clear();
            //while(!frontForward.empty()){frontForward.pop();}
            //while(!frontBackward.empty()){frontBackward.pop();}
            //frontForward.clear();
            //frontBackward.clear();
            queue_t q1;
            std::swap(q1,frontForward);
            queue_t q2;
            std::swap(q2,frontBackward);
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
      //   std::priority_queue<queueentry> frontForward;
      //   std::priority_queue<queueentry> frontBackward;
        // typedef mmheap::mmheap<queueentry,queueentryComp> queue_t;
         typedef std::priority_queue<queueentry> queue_t;
         queue_t frontForward;
         queue_t frontBackward;

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
              if(e.cost > maxCosts || e.cost > costs){
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
            std::list<MEdge>& nextNodes = forward?g->graph[node].first
                                                 :g->graph[node].second;
            std::list<MEdge>::iterator itN;
            for(itN=nextNodes.begin(); itN!=nextNodes.end(); itN++){
               processEdge(*itN,forward,e);
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

