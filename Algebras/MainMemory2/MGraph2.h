
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

#ifndef MGRAPH2_H
#define MGRAPH2_H

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

class MGraph2 : public MemoryObject{
  private:
    typedef std::pair<std::list<MEdge>,std::list<MEdge> > alist;

   public:
     MGraph2(const bool _flob, const std::string& _database,
            const std::string& _type): 
            MemoryObject(_flob,_database,_type){
        ListExpr k;
        if(!nl->ReadFromString(_type,k)){
          std::cerr << "Invalid type description " << _type << endl;
          assert(false);
        }
        // remove mem(mgraph2 ...
        k = nl->Second(nl->Second(k)); 
        SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
        k = ctlg->NumericType(k);
        tt = new TupleType(k);
     }

     ~MGraph2(){
         tt->DeleteIfAllowed();
      }
        

      static const std::string BasicType() { return "mgraph2"; }

      static bool checkType(ListExpr t){
          return nl->HasLength(t,2) 
                 && listutils::isSymbol(nl->First(t),BasicType())
                 && Tuple::checkType(nl->Second(t));
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

     template<class V>
     Tuple* insertOrigEdge(Tuple* oinfo, typename V::ctype source, 
                         typename V::ctype target, double costs){
        
        int na = oinfo->GetNoAttributes();
        Tuple* edgeTuple = new Tuple(tt);
        for(int i=0;i<oinfo->GetNoAttributes();i++){
           edgeTuple->CopyAttribute(i,oinfo,i);
        }
        if(costs < 0){
           edgeTuple->PutAttribute(na, new CcInt(false, 0));
           edgeTuple->PutAttribute(na+1, new CcInt(false, 0));
           edgeTuple->PutAttribute(na+2, new CcReal(false, 0));
           return edgeTuple;
        }
        
        typename std::map<int64_t,size_t>::iterator it;
        it = nodemap.find(source);
        size_t sourceNode;
        if(it==nodemap.end()){
          // node with new id
          std::pair<std::list<MEdge>,std::list<MEdge> > o;
          sourceNode = graph.size();
          nodemap[source] = sourceNode;
          graph.push_back(o);
        } else {
          sourceNode = it->second;
        }
        it = nodemap.find(target);
        size_t targetNode;
        if(it==nodemap.end()){
          // node with new id
          std::pair<std::list<MEdge>,std::list<MEdge> > o;
          targetNode = graph.size();
          nodemap[target] = targetNode;
          graph.push_back(o);
        } else {
          targetNode = it->second;
        }
        // insert new attributes to tuple
        edgeTuple->PutAttribute(na, new CcInt(true, sourceNode));
        edgeTuple->PutAttribute(na+1, new CcInt(true, targetNode));
        edgeTuple->PutAttribute(na+2, new CcReal(true, costs));
        if(flob) edgeTuple->bringToMemory();
        MEdge e(sourceNode, targetNode, costs, edgeTuple);
        graph[sourceNode].first.push_back(e);
        graph[targetNode].second.push_back(e);
        return edgeTuple; 
     } 
     
     bool insertGraphEdge(Tuple* ginfo ){
        int tl = ginfo->GetNoAttributes();
        CcInt* Source = (CcInt*) ginfo->GetAttribute(tl-3);
        CcInt* Target = (CcInt*) ginfo->GetAttribute(tl-2);
        CcReal* Costs = (CcReal*) ginfo->GetAttribute(tl-1);
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

     int mapNode(int64_t orig)const{
       std::map<int64_t,size_t>::const_iterator it = nodemap.find(orig);
       if(it==nodemap.end()){
         return -1;
       } else {
         return it->second;
       }
     }

     int numVertices() const{
       return graph.size();
     }


     class edgeIterator{
        public:
           friend class MGraph2;
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
           MGraph2* g;

           edgeIterator(MGraph2* _g){
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
        friend class MGraph2;
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


  private:
    std::vector<alist> graph;
    std::map<int64_t,size_t> nodemap; 
    TupleType* tt;

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

