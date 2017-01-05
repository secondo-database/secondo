

/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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


#include "Dijkstra.h"

#include "RelationAlgebra.h"
#include "StandardTypes.h"
#include "Attribute.h"
#include "LongInt.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "ListUtils.h"
#include "Stream.h"
#include "Symbols.h"

#include <string>

#include <map>
#include <vector>
#include <assert.h>



extern NestedList* nl;
extern QueryProcessor* qp;

/*
1 General Dijkstra

This module implements a dijkstra algorithm without
knowlegde about underlying structures. Instead of use
of a fixed structure, e.g. an ordered relation, functions
are use to retrieve the successors of a node, or the
costs of an edge.


*/


namespace general_dijkstra{


/*
2 Type Map operator

*/

ListExpr GDTM(ListExpr args){
  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("at least one argument expected");
  }

  ListExpr fun = nl->First(args);
  if(!listutils::isMap<1>(fun)){
    return listutils::typeError("first arg is not an unary function");
  }
  ListExpr stream = nl->Third(fun);
  if(!Stream<Tuple>::checkType(stream)){
    return listutils::typeError("function result is not a tuple stream");
  }
  return nl->Second(stream);
}




/*
 Type  Mapping

The general dijkstra has the following inputs:

  a function returning the successors of a node
   as a stream of tuples together with an attribute
   name giving the attribute of the target node

  the attribute name of the target attribute

  the initial node

  the target node

  a function computing the costs for an edge
 
  the mode

  a maximum depth, if the target node is not reached,
   the result will be empty (optionally)

The disjktra produces a stream of tuples describing the
edges extended by a number.

*/

ListExpr gdijkstraTM(ListExpr args){

  if(!nl->HasLength(args,6) && !nl->HasLength(args,7)){
    return listutils::typeError("5 or 6 arguments expected");
  } 
  ListExpr fun = nl->First(args);
  if(!listutils::isMap<1>(fun)){
    return listutils::typeError("first arg must be an unary function");
  }
 
  ListExpr nodeType = nl->Second(fun);
  if(!CcInt::checkType(nodeType) 
     && !LongInt::checkType(nodeType)){
    return listutils::typeError("argument of the first function must "
                                "be of type int or longint");
  } 
 
  ListExpr succStream = nl->Third(fun);
  if(!Stream<Tuple>::checkType(succStream)){
    return listutils::typeError("result of the first function must be "
                                "a tuple stream");
  }
  ListExpr edgeType = nl->Second(succStream);
  
  if(!listutils::isSymbol(nl->Second(args))){
    return listutils::typeError("Second arg is not an valid attribute name");
  }

  ListExpr attrList = nl->Second(edgeType);
  ListExpr attrType;
  std::string attrName = nl->SymbolValue(nl->Second(args));
  int index = listutils::findAttribute(attrList, attrName, attrType);
  if(!index){
    return listutils::typeError("Attribute " + attrName 
                                + " not found in tuple");
  }
  if(!nl->Equal(nodeType, attrType)){
    return listutils::typeError("function argument and type of target "
                                "node in tuple differ");
  }

  if(!nl->Equal(nodeType, nl->Third(args))){
    return listutils::typeError("function argument and typr of start "
                                "node differ");
  }
  if(!nl->Equal(nodeType, nl->Fourth(args))){
    return listutils::typeError("function argument and type of target "
                                "node differ");
  }

  ListExpr costFun = nl->Fifth(args);
  if(!listutils::isMap<1>(costFun)){
    return listutils::typeError("5th argument (cost function) is not "
                                "a unary function");
  }

  if(!nl->Equal(nl->Second(costFun), edgeType)){
    return listutils::typeError("cost function argument differs to edge type "
                                "defined by the successor fucntion");
  }

  if(!CcReal::checkType(nl->Third(costFun))){
    return listutils::typeError("cost function does not returns a double");
  }

  if(!CcInt::checkType(nl->Sixth(args))){
    return listutils::typeError("6th argument (mode) is not an int");
  }

  ListExpr extendTuple = nl->TwoElemList(
                               nl->TwoElemList(
                                 nl->SymbolAtom("EdgeNoSP"),
                                 listutils::basicSymbol<CcInt>()),
                               nl->TwoElemList(
                                 nl->SymbolAtom("CostsSP"),
                                 listutils::basicSymbol<CcReal>())
                             );

  ListExpr resAttrList = listutils::concat(attrList, extendTuple);
  if(!listutils::isAttrList(resAttrList)){
    return listutils::typeError("EdgeType contains already an "
                                "EdgeNoSP attribute");
  }

  ListExpr resType = nl->TwoElemList(
                       listutils::basicSymbol<Stream<Tuple> >(),
                       nl->TwoElemList(
                           listutils::basicSymbol<Tuple>(),
                           resAttrList)); 

  if(nl->HasLength(args,6)){
     return nl->ThreeElemList( 
                      nl->SymbolAtom(Symbols::APPEND()),
                      nl->TwoElemList(
                           nl->IntAtom(-1), // no max depth
                           nl->IntAtom(index -1)),
                      resType);
        
  }

  if(!CcInt::checkType(nl->Sixth(nl->Rest(args)))){
     return listutils::typeError("7th arg (maxDepth) must be of type int");
  }

 
  return nl->ThreeElemList( 
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->OneElemList( nl->IntAtom(index -1)),
                 resType);

}

template<typename T>
class treeEntry{
  public:

  treeEntry(): pred(0),costs(0),depth(0),edge(0){}

  treeEntry(T _pred, double _costs, 
            uint32_t _depth, Tuple* _edge):
     pred(_pred), costs(_costs), depth(_depth),
     edge(_edge){
  }

  treeEntry(const treeEntry<T>& e):
     pred(e.pred), costs(e.costs), depth(e.depth),
     edge(e.edge){}


  bool operator<(const treeEntry<T>& e)const{
    if(costs < e.costs) return true;
    if(costs > e.costs) return false;
    if(pred < e.pred) return true;
    if(pred > e.pred) return false;
    if(depth < e.depth) return true;
    if(depth > e.depth) return false;
    return false;
  }

  bool operator>(const treeEntry<T>& e)const{
    if(costs > e.costs) return true;
    if(costs < e.costs) return false;
    if(pred > e.pred) return true;
    if(pred < e.pred) return false;
    if(depth > e.depth) return true;
    if(depth < e.depth) return false;
    return false;
  }

    T pred;       // predecessor node
    double costs; // current costs
    uint32_t depth; // current depth
    Tuple*  edge; // edge from pred to target
};


template<class T>
class queueEntry{

  public:

    queueEntry(T* _node, double _costs, int _depth):
       node(_node) , costs(_costs), depth(_depth)
    {
         nodeValue = node->GetValue();

    }

    bool operator<(const queueEntry<T>& other) const{
       if(costs > other.costs) return true;
       return false;
    }

    std::ostream& print(std::ostream& out) const{
        out << "node : " << node->GetValue() << ", costs : " 
            << costs << ", depth : " << depth;
        return out;
    }

  T* node;
  typename T::ctype nodeValue;
  double costs;
  uint32_t depth;

};



template<class T>
class gdijkstraInfo{

  public:

    gdijkstraInfo(Supplier _succfun,
                  T* _initialNode,
                  T* _targetNode,
                  Supplier _costFun,
                  int _mode,
                  int _maxDepth,
                  int _targetPos,
                  ListExpr _tt):
        succFun(_succfun),
        initialNode(_initialNode->GetValue()),
        targetNode(_targetNode->GetValue()),
        costFun(_costFun),
        mode(_mode),
        maxDepth(_maxDepth),
        targetPos(_targetPos){
          tt = new TupleType(_tt);
          succFunArg = qp->Argument(succFun);
          costFunArg = qp->Argument(costFun);
					found = initialNode==targetNode;
          queueEntry<T> qe(_initialNode, 0,0);
          front.push(qe);
          if(mode==0 || mode==1){
              dijkstra();
          }
      }
        

   ~gdijkstraInfo(){
        tt->DeleteIfAllowed();
        // remove edges in tree
        typename std::map<ct,treeEntry<ct> >::iterator it = tree.begin();
        while(it!=tree.end()){
            it->second.edge->DeleteIfAllowed();
            it++;
        }
        while(!yellowEdges.empty()){
           yellowEdges.back()->DeleteIfAllowed();
           yellowEdges.pop_back();
        }
    }
                

    Tuple* next(){
      switch(mode){
        case 0: return next0(); // shortest path only
        case 1: return next1(); // edges to the front
        case 2: return next2(); // all visited edges
        case 3: return next3(); // shortest path tree
      }
      return 0; // not implemented mode
    }

 
                  
   private:
      Supplier succFun;
      typename T::ctype initialNode;
      typename T::ctype targetNode;
      Supplier costFun;
      int mode;
      int maxDepth;
      int targetPos;
      TupleType* tt;
      ArgVectorPointer succFunArg;
      ArgVectorPointer costFunArg;


      bool found; // used in mode 0 and 1

      typedef typename T::ctype ct;

      ct currentTarget;               // for mode 1 only

      std::vector<Tuple*> yellowEdges; // for mode 2 only


      // representation of the current tree
      std::map<ct,treeEntry<ct> > tree;
      std::priority_queue<queueEntry<T> > front;
      std::set<ct> processedNodes;


 
      inline Tuple* createResultTuple( treeEntry<ct>& entry){
         return createResultTuple(entry.edge, entry.depth, 
                                  entry.costs,entry.pred);
      }

      Tuple* createResultTuple(Tuple* oedge, int depth, double costs, ct pred){
         // create result tuple from edge and some counter
         Tuple* res = new Tuple(tt);
         for(int i=0;i<oedge->GetNoAttributes(); i++){
           res->CopyAttribute(i,oedge,i);
         } 
         res->PutAttribute(oedge->GetNoAttributes(), 
                           new CcInt(true,depth));
         res->PutAttribute(oedge->GetNoAttributes()+1, 
                           new CcReal(true,costs));
         currentTarget = pred;
         oedge->DeleteIfAllowed();
         return res;
      }


/*
~processNode~

This node will never be changed in the future.
All successor of this node will be processed.


*/

      void processNode(T* node,ct nvalue, double costs, uint32_t depth){
         if(processedNodes.find(nvalue)!=processedNodes.end()){
            // node already processed. this case may occur because 
            // updates values are not remove from the priority 
            // queue
            return;
         }
         processedNodes.insert(node->GetValue());
         if((mode!=3) && // in mode 3, the target node is ignored
            (node->GetValue() == targetNode)){
             found = true;
            return;
         }

         if(maxDepth>0 && depth>=maxDepth){
            // early stop because of path length restriction
            return;
         }

         (*succFunArg)[0] = node;
         qp->Open(succFun);
         Word value;
         qp->Request(succFun,value);
         while(qp->Received(succFun)){
             processEdge(node, costs, (Tuple*) value.addr, 
                        depth);
             qp->Request(succFun,value);
         } 
         qp->Close(succFun);
      } 

      

/*
~processEdge~

Processes a successor of a node. If the edge is invalid,
this edge is ignored completely, except in mode 2.

*/ 
      void processEdge(T* node, double nc, 
                       Tuple* edge, uint32_t depth){
         
         T* target = (T*) edge->GetAttribute(targetPos);
         if(!target->IsDefined()){
            if(mode!=2){
               edge->DeleteIfAllowed();
            } else {
               yellowEdges.push_back(createResultTuple(edge,-1*(depth+1),
                                     -1,node->GetValue()));
            }
            return;
         }

         double costs = getCosts(edge);
         if(costs<0){
            std::cerr << "found invalid costs of " << costs << std::endl;
            if(mode!=2){
               edge->DeleteIfAllowed();
            } else {
               yellowEdges.push_back(createResultTuple(edge,-1*(depth+1),
                                                       -1,node->GetValue()));
            }
            return;
         }

         typename T::ctype t = target->GetValue();
         double wc = nc + costs; 
       
         if(processedNodes.find(t)==processedNodes.end()){
           if(tree.find(t) == tree.end()){
              // t found the first time
              tree[t] = treeEntry<ct>(node->GetValue(), wc, depth+1, edge);
              front.push( queueEntry<T>(target,wc,depth+1));
           } else {
              // check for shorter path to t
              treeEntry<typename T::ctype> te = tree[t];
              if(wc < te.costs){
                 tree[t] = treeEntry<ct>(node->GetValue(), wc,depth+1,edge);
                 if(mode!=2){
                    te.edge->DeleteIfAllowed();
                 } else {
                    yellowEdges.push_back(createResultTuple(te.edge,
                                                -1*(depth+1),
                                                 te.costs,node->GetValue()));
                 }
                 front.push( queueEntry<T>(target,wc,depth+1));
              }  else {
                 if(mode!=2){
                   edge->DeleteIfAllowed();
                 } else {
                   yellowEdges.push_back(createResultTuple(edge,-1*(depth+1),
                                                        wc,node->GetValue()));
                 }
              }
           }
        } else {
           if(mode!=2){
             edge->DeleteIfAllowed();
           } else {
             yellowEdges.push_back(createResultTuple(edge,-1*(depth+1),
                                                     wc,node->GetValue()));
           }
        }
      }
 

      double getCosts(Tuple* t){
        (*costFunArg)[0] = t;
        Word value;
        qp->Request(costFun, value);
        CcReal* g = (CcReal*) value.addr;
        if(!g->IsDefined()){
          return -1;
        } 
        return g->GetValue();
      }

      void dijkstra(){
          while(!found && !front.empty()){
              queueEntry<T> n = front.top();
              front.pop();
              processNode(n.node,n.nodeValue, n.costs,n.depth);
          }
          currentTarget = found?targetNode:initialNode;
         
      }

     Tuple* next0(){ // path version
        if(!found){
          return 0;
        }
        if(currentTarget == initialNode){
           return 0;
        }
        treeEntry<ct> entry = tree[currentTarget];
        tree.erase(currentTarget);
        return createResultTuple(entry);
    }


    Tuple* next3(){ // tree version
        while(!front.empty() ){
            queueEntry<T> n = front.top();
            front.pop();
            if(processedNodes.find(n.nodeValue)==processedNodes.end()){
                processNode(n.node, n.nodeValue,n.costs,n.depth);
                if(n.nodeValue!=initialNode){ // the initial node has no pred
                    treeEntry<ct> entry = tree[n.nodeValue];
                    tree.erase(n.nodeValue);
                    return createResultTuple(entry);
                }
            } 

        } 
        return 0;
    }

    Tuple* next2(){ // all edges version
      if(!yellowEdges.empty()){
          Tuple* res = yellowEdges.back();
          yellowEdges.pop_back();
          return res;
      }
      if(found) return 0;
      return next3();
    }

    Tuple* next1(){ // front version
       while(!front.empty()){
         queueEntry<T> n = front.top();
         front.pop();
         if(processedNodes.find(n.nodeValue)
            ==processedNodes.end()
          ){
             processedNodes.insert(n.nodeValue);   
             treeEntry<ct> entry = tree[n.nodeValue];
             tree.erase(n.nodeValue);
             return createResultTuple(entry);

         }
          
       }
       return 0;
    }

};

template<class T>
int gdijkstraVMT(Word* args, Word& result, int message, 
                 Word& local, Supplier s) {

  gdijkstraInfo<T>* li = (gdijkstraInfo<T>*) local.addr;
  
  switch(message){
    case OPEN:{
          if(li){
             delete li;
             local.addr = 0;
          } 
         T* source = (T*) args[2].addr;
         if(!source->IsDefined()){
           return 0;
         }
         T* target = (T*) args[3].addr;
         if(!target->IsDefined()){
           return 0;
         }
         CcInt* mode = (CcInt*) args[5].addr;
         if(!mode->IsDefined()){
            return 0;
         }
         int mode1 = mode->GetValue();

         int depth = -1;
         CcInt* d = (CcInt*) args[6].addr;
         if(d->IsDefined()){
           depth = d->GetValue();
         }
         int attrPos = ((CcInt*) args[7].addr)->GetValue();
         local.addr = new gdijkstraInfo<T>(
                           args[0].addr,
                           source, target, args[4].addr,
                           mode1, depth, attrPos,
                           nl->Second(GetTupleResultType(s)));
         return 0;
   }

   case REQUEST:
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;
   case CLOSE:
           if(li){
               delete li;
               local.addr = 0;
           }
  }
  return-1;
}

template int gdijkstraVMT<CcInt>(Word* args, Word& result, int message,
                                 Word& local, Supplier s);

template int gdijkstraVMT<LongInt>(Word* args, Word& result, int message,
                                 Word& local, Supplier s);


} // end of namespace general_dijkstra




