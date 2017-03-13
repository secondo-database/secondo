
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

#include "MGraphCommon.h"

namespace mm2algebra{

class MGraph2 : public MGraphCommon{

   public:
     MGraph2(const bool _flob, const std::string& _database,
            const std::string& _type): 
            MGraphCommon(_flob,_database,_type,0,0,0){
        int tl = tt->GetNoAttributes();
        setPositions(tl-3, tl-2, tl-1);
     }

     ~MGraph2(){
      }
        

      static const std::string BasicType() { return "mgraph2"; }

      static bool checkType(ListExpr t){
          return nl->HasLength(t,2) 
                 && listutils::isSymbol(nl->First(t),BasicType())
                 && Tuple::checkType(nl->Second(t));
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
     

     int mapNode(int64_t orig)const{
       std::map<int64_t,size_t>::const_iterator it = nodemap.find(orig);
       if(it==nodemap.end()){
         return -1;
       } else {
         return it->second;
       }
     }



  private:
    std::map<int64_t,size_t> nodemap; 

};


} // end of namespace mm2algebra

#endif

