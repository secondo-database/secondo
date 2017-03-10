
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

#include "MainMemoryExt.h"
#include "RelationAlgebra.h"
#include "NestedList.h"
#include "SecondoCatalog.h"
#include "SecondoSystem.h"
#include "MEdge.h"
#include "ListUtils.h"
#include "StandardTypes.h"

#include <string>
#include <vector>
#include <list>

namespace mm2algebra{

class MGraph3 : public MemoryObject{
  private:
    typedef std::pair<std::list<MEdge>,std::list<MEdge> > alist;

   public:
     MGraph3(const bool _flob, 
            const std::string& _database,
            const std::string& _type,
            int _srcPos,
            int _targetPos,
            int _costPos,
            int size): 
            MemoryObject(_flob,_database,_type),
            srcPos(_srcPos),
            targetPos(_targetPos),
            costPos(_costPos){
        ListExpr k;
        if(!nl->ReadFromString(_type,k)){
          std::cerr << "Invalid type description " << _type << endl;
          assert(false);
        }
        // remove the leading (mem from the list
        k = nl->Second(k);
        assert(checkType(k));
        ListExpr tupleList = nl->Second(k);
        ListExpr attrList = nl->Second(tupleList);
        int len = nl->ListLength(attrList);
        // check valid range
        assert(srcPos>=0 && srcPos < len);
        assert(targetPos>=0 && targetPos < len);
        assert(costPos>=0 && costPos < len);
        // check correct type
        int pos = 0;
        while(!nl->IsEmpty(attrList)){
           ListExpr attrType = nl->Second(nl->First(attrList));
           attrList = nl->Rest(attrList);
           if(pos==srcPos){
             assert(CcInt::checkType(attrType));
           } 
           if(pos==targetPos){
             assert(CcInt::checkType(attrType));
           } 
           if(pos==costPos){
             assert(CcReal::checkType(attrType));
           } 
           pos++;
        }

        SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
        tupleList = ctlg->NumericType(tupleList);
        for(int i=0;i<size;i++){
           alist l;
           graph.push_back(l);
        }
        tt = new TupleType(tupleList);
     }

     ~MGraph3(){
         tt->DeleteIfAllowed();
      }
        

      static const std::string BasicType() { return "mgraph3"; }

      static bool checkType(ListExpr t){
          return nl->HasLength(t,2) 
                 && listutils::isSymbol(nl->First(t),BasicType())
                 && Tuple::checkType(nl->Second(t));
      }

     
     int numSuccessor(int vertex){
        if(vertex<0 || (size_t) vertex>=graph.size()){
          return -1;
        }
        return graph[vertex].first.size();
     }
 

     int numPredecessor(int vertex){
        if(vertex<0 || (size_t)vertex>=graph.size()){
          return -1;
        }
        return graph[vertex].second.size();
     }

     bool insertGraphEdge(Tuple* ginfo ){
        CcInt* Source = (CcInt*) ginfo->GetAttribute(srcPos);
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

     int numVertices() const{
       return graph.size();
     }


     class edgeIterator{
        public:
           friend class MGraph3;
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
           MGraph3* g;

           edgeIterator(MGraph3* _g){
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
        friend class MGraph3;
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
    int srcPos;
    int targetPos;
    int costPos;
    std::vector<alist> graph;
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

