

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
#include <limits>
#include <stack>



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

  if(!nl->HasLength(args,7)){
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

  ListExpr resType =  nl->TwoElemList(
                         listutils::basicSymbol<Stream<Tuple> >(),
                         nl->TwoElemList(
                           listutils::basicSymbol<Tuple>(),
                           resAttrList)); 

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
std::ostream& operator<<(std::ostream& o, const queueEntry<T>& e){
   return e.print(o);
}




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


    double getCosts(){
       assert(mode==4);
       Tuple* tup;
       while(!found && ((tup=next3())!=0)){
          tup->DeleteIfAllowed();
       }
       if(tup){
         tup->DeleteIfAllowed();
       }
       if(!found) {
           return std::numeric_limits<double>::max();
       }
       treeEntry<ct> entry = tree[targetNode];
       return entry.costs();
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

         if((maxDepth>0) && (depth>=(uint32_t)maxDepth)){
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

         bool ok;
         double costs = getCosts(edge, ok);
         if(!ok){
            if(mode!=2){
               edge->DeleteIfAllowed();
            } else {
               yellowEdges.push_back(createResultTuple(edge,-1*(depth+1),
                                                       -1,node->GetValue()));
            }
            return;
         }

         if(costs<0){
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
 

      double getCosts(Tuple* t, bool& correct){
        (*costFunArg)[0].addr = t;
        Word value;
        qp->Request(costFun, value);
        CcReal* g = (CcReal*) value.addr;
        correct = g->IsDefined();
        if(!correct){
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
        while(!front.empty() && !found){
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



/*

2 minPathCosts1 Operator.

Computes the costs of a path from one node to another one using the
same graph representation (as a successor function) as the general
dijkstra. In version 1, the costs are already stored as a attribute 
in each edge tuple, in version 2, a function is used.

2.1 Type Mapping

Arg 1: successor function

Arg 2: attribute name of the target attribute

Arg 3: the initial node

Arg 4: the target node

Arg 5: attribute containing or function computing the costs of an edge

Arg 6: maximim depth 

If no path is found, the returned costs correspond to the maximum double 
value. 

*/

template<bool useFun>
ListExpr minPathCostsTM(ListExpr args){
  if(!nl->HasLength(args,6)){
    return listutils::typeError("6 args expected");
  }
  // arg 1 must be a function from int, longint -> stream(tuple)
  ListExpr fun = nl->First(args);
  if(!listutils::isMap<1>(fun)){
    return listutils::typeError("first arg must be an unary function");
  }
  // check function argument 
  ListExpr nodeType = nl->Second(fun);
  if(!CcInt::checkType(nodeType) 
     && !LongInt::checkType(nodeType)){
    return listutils::typeError("argument of the first function must "
                                "be of type int or longint");
  } 
  // check function result
  ListExpr succStream = nl->Third(fun);
  if(!Stream<Tuple>::checkType(succStream)){
    return listutils::typeError("result of the first function must be "
                                "a tuple stream");
  }
  ListExpr edgeType = nl->Second(succStream);

  // second argument must be an attribute name in the function result stream  
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

  // the third and fourth argument must have the same type as the 
  // function argument

  if(!nl->Equal(nodeType, nl->Third(args))){
    return listutils::typeError("function argument and typr of start "
                                "node differ");
  }
  if(!nl->Equal(nodeType, nl->Fourth(args))){
    return listutils::typeError("function argument and type of target "
                                "node differ");
  }

  // the fifth arguments depends on the template argument
  int index2 = -1;

  if(useFun){
      ListExpr costFun = nl->Fifth(args);
      if(!listutils::isMap<1>(costFun)){
          return listutils::typeError("5th argument (cost function) is not "
                                "a unary function");
      }
      // check arg
      if(!nl->Equal(nl->Second(costFun), edgeType)){
         return listutils::typeError("cost function argument differs to "
                                "edge type defined by the successor fucntion");
      }
      // check result
      if(!CcReal::checkType(nl->Third(costFun))){
        return listutils::typeError("cost function does not returns a double");
      }
   } else {
      // in this case the 5th arg must be an edge attribute name of type double
      if(!listutils::isSymbol(nl->Fifth(args))){
         return listutils::typeError("5th arg is not a valid attribute name");
      } 
      std::string costAttr = nl->SymbolValue(nl->Fifth(args));
      index2 = listutils::findAttribute(attrList, costAttr, attrType);
      if(!index2){
        return listutils::typeError("cost attribute " + costAttr 
                                    + " not part of the edge tuple");
      }
      if(!CcReal::checkType(attrType)){
        return listutils::typeError("cost attribute " + costAttr 
                                    + " not of type real");

      }
   }


   if(!CcInt::checkType(nl->Sixth(args))){
       return listutils::typeError("6th argument (mode) is not an int");
   }

   return nl->ThreeElemList( 
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->TwoElemList( nl->IntAtom(index -1), nl->IntAtom(index2-1)),
                 listutils::basicSymbol<CcReal>());

}


template ListExpr minPathCostsTM<true>(ListExpr args);
template ListExpr minPathCostsTM<false>(ListExpr args);


template<class T>
class minPathCostsInfo{
  public:
     minPathCostsInfo(Word _succFun,
                      T* _sourceNode,
                      T* _targetNode,
                      int _maxHops,
                      int _targetPos,
                      int _costPos):
       succFun(_succFun.addr),
       sourceNode(_sourceNode),
       targetNode(_targetNode->GetValue()),
       maxHops(_maxHops),
       targetPos(_targetPos),
       costPos(_costPos)
     {
       queueEntry<T> qe((T*) _sourceNode->Copy(), 0,0);
       front.push(qe);
       found =sourceNode->GetValue() == targetNode;
       targetCosts = std::numeric_limits<double>::max();
       succFunArg = qp->Argument(succFun);
     }
     

     minPathCostsInfo(Word _succFun,
                      T* _sourceNode,
                      T* _targetNode,
                      int _maxHops,
                      int _targetPos,
                      Word _costFun):
       succFun(_succFun.addr),
       sourceNode(_sourceNode),
       targetNode(_targetNode->GetValue()),
       maxHops(_maxHops),
       targetPos(_targetPos),
       costPos(-1),
       costFun(_costFun.addr) 
     {
       queueEntry<T> qe((T*)_sourceNode->Copy(), 0,0);
       front.push(qe);
       found =sourceNode->GetValue() == targetNode;
       targetCosts = std::numeric_limits<double>::max();
       succFunArg = qp->Argument(succFun);
       costFunArg = qp->Argument(costFun);
     }


     ~minPathCostsInfo(){
         while(!front.empty()){
            queueEntry<T> t = front.top();
            front.pop();
            t.node->DeleteIfAllowed();
         }
     }


     double getCosts(){
       while(!found && !front.empty()){
         queueEntry<T> e = front.top();
         front.pop();
         processNode(e);
         e.node->DeleteIfAllowed();
       }

       return targetCosts;
     }

  private:
     typedef typename T::ctype ct;
     Supplier succFun;
     T* sourceNode;
     ct targetNode;
     int maxHops;
     int targetPos;
     int costPos;

     Supplier costFun;
     ArgVectorPointer costFunArg;


     std::priority_queue<queueEntry<T> > front;
     std::set<ct> processedNodes;
     bool found;
     double targetCosts;
     ArgVectorPointer succFunArg;
     std::map<ct,double> frontCosts;


     void processNode(queueEntry<T>& e){

        if(processedNodes.find(e.nodeValue)!=processedNodes.end()){
           return;
        }
        if(e.nodeValue==targetNode){
          found = true;
          targetCosts = e.costs;
          return;
        }
        processedNodes.insert(e.nodeValue);
        frontCosts.erase(e.nodeValue);

        if(maxHops>0 && e.depth==(uint32_t)maxHops){
            return;
        }
        // cancel computation not possible, process successors
        (*succFunArg)[0] = e.node;
         qp->Open(succFun);
         Word value;
         qp->Request(succFun,value);
         while(qp->Received(succFun)){
             processEdge(e, (Tuple*) value.addr);
             qp->Request(succFun,value);
         } 
         qp->Close(succFun);
     }


     void processEdge(queueEntry<T>& e, Tuple* tup){
         double costs = getCosts(tup);
         if(costs<0){
            tup->DeleteIfAllowed();
            return;
         }
         T* target = (T*) tup->GetAttribute(targetPos);
         if(!target->IsDefined()){
            tup->DeleteIfAllowed();
            return;
         }
         ct targetVal = target->GetValue();
         if(processedNodes.find(targetVal)!=processedNodes.end()){
             tup->DeleteIfAllowed();
             return;
         } 
         target = (T*) target->Copy();
         tup->DeleteIfAllowed();
         double nc = e.costs + costs;
         typename std::map<ct,double>::iterator it 
               = frontCosts.find(targetVal);

         if(it==frontCosts.end()){ // node found the first time 
                                   // otherwise the node is either processed 
                                   // or in front
            frontCosts[targetVal] = nc;  
            front.push(queueEntry<T>(target, nc, e.depth+1));
         } else {
            if(it->second<= nc){
               target->DeleteIfAllowed();
            } else {
               it->second = nc; // update costs  
               front.push(queueEntry<T>(target, nc, e.depth+1));
            }
         }
     }


      double getCosts(Tuple* tup){
         if(costPos >=0){
            CcReal* c = (CcReal*) tup->GetAttribute(costPos);
            if(!c->IsDefined()){
               return -1;
             }
              return c->GetValue();
         } else {
            (*costFunArg)[0].addr = tup;
            Word value;
            qp->Request(costFun, value);
            CcReal* g = (CcReal*) value.addr;
            if(!g->IsDefined()){
               return -1;
            } else {
               return g->GetValue();
            }
         }
      }
};




template<class T>
int minPathCost1VMT(Word* args, Word& result, int message, 
                    Word& local, Supplier s) {

  result = qp->ResultStorage(s);
  CcReal* res = (CcReal*) result.addr;
  T* source = (T*) args[2].addr;
  T* target = (T*) args[3].addr;
  CcInt* mH = (CcInt*) args[5].addr;

  if(!source->IsDefined() || !target->IsDefined() || !mH->IsDefined()){
     res->SetDefined(false);
     return 0;
  } 

  int targetPos = ((CcInt*)args[6].addr)->GetValue();
  int costPos = ((CcInt*)args[7].addr)->GetValue();
 


  minPathCostsInfo<T> info(args[0],source,target,mH->GetValue(), 
                           targetPos,costPos);
  res->Set(true,info.getCosts());                      
  return 0;
}



template<class T>
int minPathCost2VMT(Word* args, Word& result, int message, 
                    Word& local, Supplier s) {

  result = qp->ResultStorage(s);
  CcReal* res = (CcReal*) result.addr;
  T* source = (T*) args[2].addr;
  T* target = (T*) args[3].addr;
  CcInt* mH = (CcInt*) args[5].addr;

  if(!source->IsDefined() || !target->IsDefined() || !mH->IsDefined()){
     res->SetDefined(false);
     return 0;
  } 

  int targetPos = ((CcInt*)args[6].addr)->GetValue();
 


  minPathCostsInfo<T> info(args[0],source,target,mH->GetValue(),
                           targetPos,args[4]);
  res->Set(true,info.getCosts());                      
  return 0;
}




template int minPathCost2VMT<CcInt>(Word* args, Word& result, int message,
                     Word& local, Supplier s);

template int minPathCost2VMT<LongInt>(Word* args, Word& result, int message,
                     Word& local, Supplier s);



template int minPathCost1VMT<CcInt>(Word* args, Word& result, int message,
                     Word& local, Supplier s);

template int minPathCost1VMT<LongInt>(Word* args, Word& result, int message,
                     Word& local, Supplier s);


/*
3 mtMinPathCosts

This operator works similar to the minPathcosts but computes the
minimum path costs for a set of targets.

*/
template<bool costsAsFun>
ListExpr mtMinPathCostsTM(ListExpr args){
  if(!nl->HasLength(args,7)){
    return listutils::typeError("7 arguments expected");
  }
  /*
    1 : successor function
    2 : stream of targets (tuples)
    3 : name of the target in successor function
    4 : source of the search
    5 : name of the target in the target stream
    6 : name of the cost attribute or cost function (depends on template)
    7 : maximum number of hops
  */
   // allowed node types: int, longint
   ListExpr source = nl->Fourth(args);
   if(!CcInt::checkType(source) && !LongInt::checkType(source)){
     return listutils::typeError("invalid node type, supported are "
                                 "int and longint");
   }
   ListExpr succFun = nl->First(args);
   if(!listutils::isMap<1>(succFun)){
     return listutils::typeError("first arg is not a map");
   }
   if(!nl->Equal(nl->Second(succFun), source)){
     return listutils::typeError("type of source node and type of the "
                                 "argument of the successor function differ");
        
   }
   ListExpr succstream = nl->Third(succFun);
   if(!Stream<Tuple>::checkType(succstream)){
     return listutils::typeError("successor function does not produce a "
                                 "tuple stream");
   }
   ListExpr hops = nl->Sixth(nl->Rest(args));
   if(!CcInt::checkType(hops)){
     return listutils::typeError("last argument not of type int");
   }
   ListExpr succTargetName = nl->Third(args);
   if(nl->AtomType(succTargetName)!=SymbolType){
     return listutils::typeError("3rd argument is not a valid attribute name");
   }
   std::string succTarget = nl->SymbolValue(succTargetName);
   ListExpr attrType;
   ListExpr succAttrList = nl->Second(nl->Second(succstream));
   int succIndex = listutils::findAttribute(succAttrList,succTarget,attrType);
   if(!succIndex){
     return listutils::typeError("attribute " + succTarget + " not found in "
                                 "successor function result");
   }
   if(!nl->Equal(attrType,source)){
      return listutils::typeError("type of source node and attribute " + 
                                  succTarget + " in successor function result"
                                  " differ");
   }
   ListExpr targetSetName = nl->Fifth(args);
   if(nl->AtomType(targetSetName)!=SymbolType){
     return listutils::typeError("5th arg is not a valid attribute name");
   }
   std::string targetSet = nl->SymbolValue(targetSetName);
   ListExpr targets = nl->Second(args);
   if(!Stream<Tuple>::checkType(targets)){
     return listutils::typeError("second arg must be a tuple stream");
   }
   ListExpr targetAttrList = nl->Second(nl->Second(targets));
   int targetIndex = listutils::findAttribute(targetAttrList, targetSet,
                                              attrType);
   if(!targetIndex){
     return listutils::typeError("attribute " + targetSet + " not found "
                                 "in target stream");
   }
   if(!nl->Equal(attrType, source)){
     return listutils::typeError("type of " + targetSet + " in target stream"
                                 " differs to the type of the source node");
   }
   
   if(listutils::findAttribute(targetAttrList,"MinPC",attrType)){
     return listutils::typeError("Attribute name MinPC already present in "
                                 "target tuple");
   }

   ListExpr appAttr = nl->OneElemList( nl->TwoElemList(
                            nl->SymbolAtom("MinPC"),
                            listutils::basicSymbol<CcReal>()));
   

   ListExpr resAttrList = listutils::concat(targetAttrList,appAttr);

   ListExpr res = nl->TwoElemList(
                       listutils::basicSymbol<Stream<Tuple> >(),
                       nl->TwoElemList(
                           listutils::basicSymbol<Tuple>(),
                           resAttrList));
   


   ListExpr costsArg = nl->Sixth(args);
   if(!costsAsFun){
      if(nl->AtomType(costsArg) != SymbolType){
        return listutils::typeError("6th arg is not a valid attribute name");
      }
      std::string costName = nl->SymbolValue(costsArg);
      int succCostsIndex = listutils::findAttribute(succAttrList,costName,
                                                    attrType);
      if(!succCostsIndex){
        return listutils::typeError("Attribute " + costName + " not found in " 
                                    "successor tuple");
      }
      if(!CcReal::checkType(attrType)){
          return listutils::typeError("Attribute " + costName + " not of type "
                                      "real in successor tuple");
      }
      ListExpr appendList = nl->ThreeElemList(
                                nl->IntAtom(succIndex -1),
                                nl->IntAtom(targetIndex -1),
                                nl->IntAtom(succCostsIndex-1));
     return nl->ThreeElemList(
                  nl->SymbolAtom(Symbols::APPEND()),
                  appendList,
                  res);
   } 

   // costs are given by a function
   if(!listutils::isMap<1>(costsArg)){
      return listutils::typeError("sixth arg is not an unary function");
   }
   if(!CcReal::checkType(nl->Third(costsArg))){
     return listutils::typeError("result of cost function not of type real");
   }
   ListExpr succTuple = nl->Second(succstream);
   if(!nl->Equal(succTuple, nl->Second(costsArg))){
      return listutils::typeError("successor tuple and argument of the cost "
                                 "function differ");
   }
   ListExpr appendList = nl->TwoElemList(
                               nl->IntAtom(succIndex -1),
                               nl->IntAtom(targetIndex -1));
 
   return nl->ThreeElemList(
              nl->SymbolAtom(Symbols::APPEND()),
              appendList,
              res);
}


template  ListExpr mtMinPathCostsTM<true>(ListExpr args);
template  ListExpr mtMinPathCostsTM<false>(ListExpr args);


template <class T>
class mtMinPathCostsInfo{
  public:
   mtMinPathCostsInfo(Supplier _succFun,
                      Word& _targetStream,
                      int _succIndex,
                      T*  _source,
                      int _targetIndex,
                      int _costsIndex,
                      size_t _maxHops,
                      TupleType* _tt):
     succFun(_succFun),succIndex(_succIndex),
     targetIndex(_targetIndex),
     costsIndex(_costsIndex), maxHops(_maxHops),
     tt(_tt){
       succArg = qp->Argument(succFun);
       tt->IncReference();
       costFun = 0;
       collectTargets(_targetStream);
       queueEntry<T> qe((T*)_source->Copy(), 0,0);
       front.push(qe);
       compute();
       resPos = 0;
     }
   

     mtMinPathCostsInfo(Supplier _succFun,
                      Word& _targetStream,
                      int _succIndex,
                      T*  _source,
                      int _targetIndex,
                      Supplier _costsFun,
                      int _maxHops,
                      TupleType* _tt):
     succFun(_succFun),succIndex(_succIndex),
     targetIndex(_targetIndex),
     costsIndex(-1), maxHops(_maxHops),
     tt(_tt){

       costFun = _costsFun;
       succArg = qp->Argument(succFun);
       tt->IncReference();
       costFunArg = qp->Argument(costFun);
       
       collectTargets(_targetStream);
       queueEntry<T> qe((T*)_source->Copy(), 0,0);
       front.push(qe);
       compute();
       resPos = 0;
     }

     ~mtMinPathCostsInfo(){
        // delete remaining tuples in front
        while(!front.empty()){
           front.top().node->DeleteIfAllowed();
           front.pop();
        }
        // delete remaining tuples from results
        while(resPos < results.size()){
           results[resPos++]->DeleteIfAllowed();
        }
        // delete non-found nodes from targets
        typename std::map<ct,Tuple*>::iterator it;
        for(it=targets.begin();it!=targets.end();it++){
           if(it->second){
             it->second->DeleteIfAllowed();
             it->second=0;
           }
        }
        targets.clear();
        results.clear();
        tt->DeleteIfAllowed();
     }

     Tuple* next(){
         if(resPos>=results.size()){
            return 0;
         }
         Tuple* res = results[resPos];
         results[resPos]=0;
         resPos++;
         return res;
     }


   private:
      Supplier succFun;
      int succIndex;
      int targetIndex;
      int costsIndex;
      size_t maxHops;
      TupleType* tt;

      Supplier costFun;
      ArgVectorPointer costFunArg;


      typedef typename T::ctype ct;
      ArgVectorPointer succArg;
      std::map<ct,Tuple*> targets;
      std::vector<Tuple*> results;
      

      std::priority_queue<queueEntry<T> > front;
      std::set<ct> processedNodes;
      double targetCosts;
      std::map<ct,double> frontCosts;
      
      size_t resPos;
      


/*
~collect targets~

Reads all tuples from the stream of targets and inserts them
into the targets map.


*/
      void collectTargets(Word& s){

          size_t failed = 0;
          Stream<Tuple> stream(s);
          stream.open();
          Tuple* tuple;
          while( (tuple=stream.request())!=0){
             T* target = ((T*) tuple->GetAttribute(targetIndex));
             if(!target->IsDefined()){
               std::cerr << "found undefined target" << std::endl;
               tuple->DeleteIfAllowed();
             } else {
               ct node = target->GetValue();
               if(targets.find(node)==targets.end()){
                  targets[node] = tuple;
               } else {
                  failed++;
                  tuple->DeleteIfAllowed();
               }
            }
          } 
          stream.close();
          if(failed>0){
             cout << "found some targets more than one time " << endl;
             cout << "found " << targets.size() << " unique targets" << endl;
             cout << "ignored " << failed 
                  << " targets which was given more than once" << endl;
          }
      }

/*
~compute~

This function computes the result set. The result set is
available in the results vector.

*/
      void compute(){
         dijkstra();

         // may be there are nodes that are not found.
         // such nodes are inserted into the result set
         // with maximum costs
         typename std::map<ct,Tuple*>::iterator it;
         double c = std::numeric_limits<double>::max();
         for(it=targets.begin();it!=targets.end();it++){
            Tuple* t = it->second;
            results.push_back(createResultTuple(t,c));
            t->DeleteIfAllowed();
            it->second=0;
         }
      }

      Tuple* createResultTuple(const Tuple* t, double c){
         Tuple* res = new Tuple(tt);
         for(int i=0;i<t->GetNoAttributes();i++){
           res->CopyAttribute(i,t,i);
         }
         res->PutAttribute(t->GetNoAttributes(), new CcReal(true,c));
         return res;
      }


      void dijkstra(){
         while(!front.empty() && !targets.empty()){
            queueEntry<T> e = front.top();
            front.pop();
            processNode(e);
            e.node->DeleteIfAllowed();      
         }
      }

      void processNode(queueEntry<T>& e){
         if(processedNodes.find(e.nodeValue)!=processedNodes.end()){
           return;
         }
         processedNodes.insert(e.nodeValue);

         typename std::map<ct,Tuple*>::iterator it;
         it=targets.find(e.nodeValue);

         if(it!=targets.end()){
           // found a target node
           results.push_back(createResultTuple(it->second,e.costs));
           it->second->DeleteIfAllowed();
           targets.erase(it);
           if(targets.empty()){ // all targets reached
              return; 
           }
         }

         if(maxHops>0 && e.depth>=maxHops){
            return;
         }
         // use successors of e node
         (*succArg)[0] = e.node;
         qp->Open(succFun);
         Word value;
         qp->Request(succFun,value);
         while(qp->Received(succFun)){
             processEdge(e, (Tuple*) value.addr);
             qp->Request(succFun,value);
         } 
         qp->Close(succFun);
      } 


      void processEdge(queueEntry<T>& e, Tuple* tup){
         double costs = getCosts(tup);
         // special case: invalid costs
         if(costs<0){
            std::cerr << "found negative or undefined costs" << std::endl;
            tup->DeleteIfAllowed();
            return;
         }
         T* target = (T*) tup->GetAttribute(targetIndex);
         // special case undefined value
         if(!target->IsDefined()){
            std::cerr << " found undefined costs" << endl;
            tup->DeleteIfAllowed();
            return;
         }
         ct targetVal = target->GetValue();
         // target already finished
         if(processedNodes.find(targetVal)!=processedNodes.end()){
             tup->DeleteIfAllowed();
             return;
         } 
         target = (T*) target->Copy();
         tup->DeleteIfAllowed();
         double nc = e.costs + costs;
         typename std::map<ct,double>::iterator it 
               = frontCosts.find(targetVal);

         if(it==frontCosts.end()){ // node found the first time 
            frontCosts[targetVal] = nc;  
            front.push(queueEntry<T>(target, nc, e.depth+1));
         } else {
            if(it->second<= nc){ // old path is shorter
               target->DeleteIfAllowed();
            } else {
               it->second = nc; // update costs  
               front.push(queueEntry<T>(target, nc, e.depth+1));
            }
         }
      }

      double getCosts(Tuple* t){
         if(costFun){
            return getCostsFun(t);
         } else {
            return getCostsAttr(t);
         }
      } 

      double getCostsFun(Tuple* t){
        (*costFunArg)[0].addr = t;
        Word value;
        qp->Request(costFun, value);
        CcReal* g = (CcReal*) value.addr;
        bool correct = g->IsDefined();
        (*costFunArg)[0].addr = 0;
        if(!correct){
          return -1;
        } 
        return g->GetValue();
     }

     double getCostsAttr(Tuple* t){
        CcReal* g = (CcReal*) t->GetAttribute(costsIndex);
        bool correct = g->IsDefined();
        if(!correct){
          return -1;
        } 
        return g->GetValue();
     }
};



template<class T>
int mtMinPathCost1VMT(Word* args, Word& result, int message, 
                      Word& local, Supplier s) {

  mtMinPathCostsInfo<T>* li = (mtMinPathCostsInfo<T>*) local.addr;
  TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;
  switch(message){
     case INIT : {
        tt = new TupleType(nl->Second(GetTupleResultType(s)));
        qp->GetLocal2(s).addr = tt;
        return 0;
     }
     case FINISH: {
        if(tt){
           tt->DeleteIfAllowed();
           qp->GetLocal2(s).addr=0; 
        }
        return 0;
     }
     case OPEN: {
        if(li){
           delete li;
           local.addr = 0;
        }  
        T* source = (T*) args[3].addr;
        if(!source->IsDefined()){
           return 0;
        }
        int hops = 0;
        CcInt* Hops = (CcInt*) args[6].addr;
        if(Hops->IsDefined()){
           hops = Hops->GetValue();  
           if(hops<0){
              hops = 0;
           }
        }
        int succIndex = ((CcInt*) args[7].addr)->GetValue();
        int targetIndex = ((CcInt*) args[8].addr)->GetValue();
        int costsIndex = ((CcInt*) args[9].addr)->GetValue();
        li = new mtMinPathCostsInfo<T>(args[0].addr, args[1],
                                       succIndex, source, targetIndex,
                                       costsIndex, hops,tt);
        local.addr = li;
        return 0;
     }
   case REQUEST: {
      result.addr = li?li->next():0;
      return result.addr?YIELD:CANCEL;
   }
   case CLOSE: {
       if(li){
         delete li;
         local.addr = 0;
       }
       return 0;
   }

  }
  return -1;
}


template<class T>
int mtMinPathCost2VMT(Word* args, Word& result, int message, 
                      Word& local, Supplier s) {

  mtMinPathCostsInfo<T>* li = (mtMinPathCostsInfo<T>*) local.addr;
  TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;
  switch(message){
     case INIT : {
        tt = new TupleType(nl->Second(GetTupleResultType(s)));
        qp->GetLocal2(s).addr = tt;
        return 0;
     }
     case FINISH: {
        if(tt){
           tt->DeleteIfAllowed();
           qp->GetLocal2(s).addr=0; 
        }
        return 0;
     }
     case OPEN: {
        if(li){
           delete li;
           local.addr = 0;
        }  
        T* source = (T*) args[3].addr;
        if(!source->IsDefined()){
           return 0;
        }
        int hops = 0;
        CcInt* Hops = (CcInt*) args[6].addr;
        if(Hops->IsDefined()){
           hops = Hops->GetValue();  
           if(hops<0){
              hops=0;
           }
        }
        int succIndex = ((CcInt*) args[7].addr)->GetValue();
        int targetIndex = ((CcInt*) args[8].addr)->GetValue();
        Supplier costsFun = args[5].addr;
        li = new mtMinPathCostsInfo<T>(args[0].addr, args[1],
                                       succIndex, source, targetIndex,
                                       costsFun, hops,tt);
        local.addr = li;
        return 0;
     }
   case REQUEST: {
      result.addr = li?li->next():0;
      return result.addr?YIELD:CANCEL;
   }
   case CLOSE: {
       if(li){
         delete li;
         local.addr = 0;
       }
       return 0;
   }

  }
  return -1;
}


template int mtMinPathCost1VMT<CcInt>(Word* args, Word& result, int message,
                      Word& local, Supplier s);

template int mtMinPathCost1VMT<LongInt>(Word* args, Word& result, int message,
                      Word& local, Supplier s);


template int mtMinPathCost2VMT<CcInt>(Word* args, Word& result, int message,
                      Word& local, Supplier s);

template int mtMinPathCost2VMT<LongInt>(Word* args, Word& result, int message,
                      Word& local, Supplier s);

/*
3 Implementation of a bidirectional dijkstra.

This operator searches a shortest path between a source and a target node.
In contrast to a normal implementation of dijktra, this operator searches
bidirectional, i.e. from the source and the target at the same time.

3.1 Type Mapping

 Function returning the successor of a node (tuple edge)

 Function returning predecessor of a node   (tuple edge)

 Attribute name of the successor

 attribute name of the predecessor

 source node

 target node

 cost function: (edge tuple -> real)

*/

ListExpr gbidijkstraTM(ListExpr args){
   if(!nl->HasLength(args,7)){
     return listutils::typeError("7 arguments required");
   }
  
   ListExpr fun1 = nl->First(args);
   if(!listutils::isMap<1>(fun1)){
     return listutils::typeError("first arg is not a unary function");
   }
   ListExpr fun2 = nl->Second(args);
   if(!listutils::isMap<1>(fun2)){
     return listutils::typeError("second arg is not a unary function");
   }
   if(!nl->Equal(fun1,fun2)){
     return listutils::typeError("function types must be equal");
   }

   // we allow int, string and longint as node type
   ListExpr source = nl->Fifth(args);
   if(   !CcInt::checkType(source)
      && !LongInt::checkType(source)){
    return listutils::typeError("allowed node types are int, longint");
   }
   if(!nl->Equal(source,nl->Sixth(args))){
      return listutils::typeError("source node and target node "
                                  "have different types");
   } 
   // check function argument
   ListExpr funarg = nl->Second(fun1);
   if(!nl->Equal(funarg,source)){
     return listutils::typeError("successor function's argument and "
                                 "source node type differ");
   }
   // result of the function must be a tuple stream
   ListExpr funres = nl->Third(fun1);
   if(!Stream<Tuple>::checkType(funres)){
      return listutils::typeError("successors function's return type "
                                  "is not a tuple stream");
   }
   ListExpr attrList = nl->Second(nl->Second(funres));

   // the third argument must be an attribute in the edge tuple
   ListExpr succNameL = nl->Third(args);
   if(nl->AtomType(succNameL) != SymbolType){
     return listutils::typeError("third argument is not a valid "
                                 "attribute name");
   }
   std::string succName = nl->SymbolValue(succNameL);
   ListExpr attrType;
   int index1 = listutils::findAttribute(attrList, succName, attrType);
   if(!index1){
      return listutils::typeError("Attribute " + succName 
                                  + " is not part of the edge tuple");
   } 
   if(!nl->Equal(attrType,source)){
     return listutils::typeError("Type of attribute " + succName
                         +" and type of source node differ");
   }
   // the same must be checked for the name of the predecessor
   ListExpr predNameL = nl->Fourth(args);
   if(nl->AtomType(predNameL) != SymbolType){
     return listutils::typeError("fourth attribute is not a valid "
                                 "attribute name");
   }  
   std::string predName = nl->SymbolValue(predNameL);
   int index2 = listutils::findAttribute(attrList, predName, attrType);
   if(!index2){
     return listutils::typeError("attribute " + predName 
                                 + " not part of the edge tuple");
   }
   if(!nl->Equal(attrType,source)){
     return listutils::typeError("attribute " + predName 
                        +" has another type than the nodes ");
   }

   // check whether the 7th argument is a function from an 
   // edge tuple to real
   ListExpr costFun = nl->Sixth(nl->Rest(args));
   if(!listutils::isMap<1>(costFun)){
     return listutils::typeError("cost fun is not a unary function");
   }
   ListExpr edgeTuple = nl->Second(funres);
   if(!nl->Equal(nl->Second(costFun),edgeTuple)){
     return listutils::typeError("edge tuple and cost fun argument differ");
   }
   if(!CcReal::checkType(nl->Third(costFun))){
      return listutils::typeError("result of the cost function is not a real");
   }

   if(listutils::findAttribute(attrList,"IsForward",attrType)){
     return listutils::typeError("edge tuple contains a IsForward attribute");
   }
   ListExpr add = nl->OneElemList(nl->TwoElemList(
                                   nl->SymbolAtom("IsForward"),
                                   listutils::basicSymbol<CcBool>()));
   ListExpr resAttrList = listutils::concat(attrList,add);

   if(!listutils::isAttrList(resAttrList)){
     return listutils::typeError("Unknown error, may be a stupid programmer");
   }   

   return nl->ThreeElemList(
             nl->SymbolAtom(Symbols::APPEND()),
             nl->TwoElemList(
                    nl->IntAtom(index1 - 1),
                    nl->IntAtom(index2 - 1)),
             nl->TwoElemList(
                  listutils::basicSymbol<Stream<Tuple> >(),
                  nl->TwoElemList(
                      listutils::basicSymbol<Tuple>(),
                      resAttrList)));
}


template<class T>
class gbidijkstraInfo{

  public:
     gbidijkstraInfo(
         Supplier _succFun,
         Supplier _predFun,
         int _succPos,
         int _predPos,
         Supplier _costFun,
         T* _source,
         T* _target,
         TupleType* _tt):
        succFun(_succFun), predFun(_predFun),
        succPos(_succPos), predPos(_predPos),
        costFun(_costFun),
        source(_source), target(_target),
        tt(_tt){

           tt->IncReference();
           succArg = qp->Argument(succFun);
           predArg = qp->Argument(predFun);
           costArg = qp->Argument(costFun);
           commonNode = 0;
           Ffront.push(queueEntry<T>(source,0,0));
           Bfront.push(queueEntry<T>(target,0,0));
           dijkstra(); 
           if(commonNode){
             currentNode = commonNode->GetValue();
             fillForwardTuples();
           }
     }

     ~gbidijkstraInfo(){
        tt->DeleteIfAllowed();
        removeEdges(Ftree);
        removeEdges(Btree);
        while(!forwardTuples.empty()){
           forwardTuples.top()->DeleteIfAllowed();
           forwardTuples.pop();
        }
     }

     Tuple* next(){
         if(!commonNode){
           return 0;
         }
         if(!forwardTuples.empty()){
            Tuple* res = forwardTuples.top();
            forwardTuples.pop();
            return res;
         } else {
            typename std::map<ct,treeEntry<ct> >::iterator it;
            it = Btree.find(currentNode);         
            if(it==Btree.end()){ // end reached
               assert(currentNode == target->GetValue());
               commonNode = 0;
               return 0;
            }
            Tuple* res = createResultTuple(it->second,false);
            currentNode = it->second.pred;
            return res; 
         }
     }


  private:

     typedef typename T::ctype ct;

     Supplier succFun;
     Supplier predFun;
     int succPos;
     int predPos;
     Supplier costFun;
     T* source;
     T* target;
     TupleType* tt;
     ArgVectorPointer succArg;
     ArgVectorPointer predArg;
     ArgVectorPointer costArg;
     T* commonNode;

     std::priority_queue<queueEntry<T> > Ffront;
     std::priority_queue<queueEntry<T> > Bfront;
     std::set<ct> Ffinished;
     std::set<ct> Bfinished;
     std::map<ct,treeEntry<ct> > Ftree;
     std::map<ct,treeEntry<ct> > Btree;
     

     ct currentNode;

     // to return the path in a 'normal' order from source
     // to target, we put the tuples from fromward tree to
     // a stack
     std::stack<Tuple*> forwardTuples;



     void removeEdges(std::map<ct,treeEntry<ct> >& tree){
       typename std::map<ct,treeEntry<ct> >::iterator it;
       for(it = tree.begin();it!=tree.end();it++){
         it->second.edge->DeleteIfAllowed();
       }
     }


    void fillForwardTuples(){
       typename std::map<ct,treeEntry<ct> >::iterator it;
       bool dir=true;
       while(dir){
            it = Ftree.find(currentNode);
            if(it==Ftree.end()){ // begin of path reached
               assert(currentNode == source->GetValue());
               dir = false;
               // prepare for back
               currentNode = commonNode->GetValue();
            } else {
               Tuple* res = createResultTuple(it->second,true);
               currentNode = it->second.pred;
               forwardTuples.push(res);
            }
       }
    }
     
     void dijkstra(){

        while(!commonNode){
          if(Ffront.empty()){
             if(Bfront.empty()){
               // no common node found
               return;
             } else {
               // only back front available
               queueEntry<T> e = Bfront.top();
               Bfront.pop();
               processNode(e,false);
             }
          } else {
            if(Bfront.empty()){
               // only forward front not empty
               queueEntry<T> e = Ffront.top();
               Ffront.pop();
               processNode(e,true);  
            } else {
               // both fronts have elements
               if(Ffront.top().costs<=Bfront.top().costs){
                  queueEntry<T> e = Ffront.top();
                  Ffront.pop();
                  processNode(e,true);  
               } else {
                 queueEntry<T> e = Bfront.top();
                 Bfront.pop();
                 processNode(e,false);
               }
            }
          }
        }
     }
 

     void processNode(queueEntry<T>& e, bool isForward){
         // check stop criterion
         ct v = e.nodeValue;
         Supplier fun;
         ArgVectorPointer arg;
         int pos;
         if(isForward){
            if(Ffinished.find(v)!=Ffinished.end()){
               // node already processed
               return;
            }
            Ffinished.insert(v);
            if(Bfinished.find(v)!=Bfinished.end()){ 
               // common node found
               commonNode = e.node;
               return; 
            }
            // further search
            fun = succFun;
            arg = succArg;
            pos = succPos;
         } else {
            if(Bfinished.find(v)!=Bfinished.end()){
               // node already processed 
               return;
            }
            Bfinished.insert(v);
            if(Ffinished.find(v)!=Ffinished.end()){
              // common node found
              commonNode = e.node;
              return; 
            }
            fun = predFun;
            arg = predArg;
            pos = predPos;
         }

         // process all edges
         (*arg)[0] = e.node;
         Word f(fun);
         Stream<Tuple> stream(f);
         Tuple* edge;
         stream.open();
         while( (edge = stream.request()) ){
            processEdge(e.nodeValue, e.costs, edge, isForward, pos, e.depth);  
         }
         stream.close();
     }

     void processEdge(ct pred, double costs, 
                      Tuple* edge, bool isForward, int pos,
                      int depth){


        T* target = (T*) edge->GetAttribute(pos);
        if(!target->IsDefined()){ // ignore nonsense tuples
           edge->DeleteIfAllowed();
           return;
        }
        ct v = target->GetValue();
        std::set<ct>* finished = isForward?&Ffinished:&Bfinished;
        std::map<ct,treeEntry<ct> >* tree = isForward?&Ftree:&Btree;
        std::priority_queue<queueEntry<T> >* front = isForward?&Ffront:&Bfront; 

        if(finished->find(v)!=finished->end()){
           // target node already processed 
           edge->DeleteIfAllowed();
           return;
        }

        double edgeCosts = getCosts(edge);
        if(edgeCosts < 0){ // invalid costs
           edge->DeleteIfAllowed();
           return;
        }


        double newCosts = costs + edgeCosts;
        typename std::map<ct,treeEntry<ct> >::iterator it = tree->find(v);
        if(it==tree->end()){  // node reached the first time
           treeEntry<ct> e(pred,newCosts,depth + 1, edge);
           (*tree)[v] = e;
           queueEntry<T> qe(target, newCosts,depth+1);
           front->push(qe);
           return;
        }

        if(it->second.costs<=newCosts){
           // new path is not shorter than an old one
           edge->DeleteIfAllowed();
           return;
        }

        // update case
        it->second.edge->DeleteIfAllowed(); 
        it->second.edge = edge;
        it->second.pred = pred;
        it->second.costs = newCosts;
        it->second.depth = depth+1;
        queueEntry<T> qe(target, newCosts,depth+1);
        front->push(qe);
     }


     double getCosts(Tuple* edge) const{
        (*costArg)[0] = edge;
        Word value;
        qp->Request(costFun, value);
        CcReal* g = (CcReal*) value.addr;
        if(!g->IsDefined()){
          return -1;
        } 
        return g->GetValue();
     }

     Tuple* createResultTuple(treeEntry<ct>& e, bool forward){
        Tuple* res = new Tuple(tt);
        Tuple* src = e.edge;
        for(int i=0;i<src->GetNoAttributes();i++){
          res->CopyAttribute(i,src,i);
        } 
        res->PutAttribute(src->GetNoAttributes(),new CcBool(true,forward));
        return res;
     }
};



template<class T>
int gbidijkstraVMT(Word* args, Word& result, int message, 
                    Word& local, Supplier s) {

   gbidijkstraInfo<T>* li = (gbidijkstraInfo<T>*) local.addr;
   TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;

   switch(message){
     case INIT : {
        tt = new TupleType(nl->Second(GetTupleResultType(s)));
        qp->GetLocal2(s).addr = tt;
        return 0;
     }
     case FINISH: {
        if(tt){
           tt->DeleteIfAllowed();
           qp->GetLocal2(s).addr=0; 
        }
        return 0;
     }
     case OPEN:{
        if(li){
          delete li;
          local.addr = 0;
        }
        int succPos = ((CcInt*) args[7].addr)->GetValue();
        int predPos = ((CcInt*) args[8].addr)->GetValue();
        T* source = (T*) args[4].addr;
        T* target = (T*) args[5].addr; 
        if(!source->IsDefined() || !target->IsDefined()){
           return 0;
        }
        if(source->Compare(target)==0){
          return 0;
        }
        local.addr = new gbidijkstraInfo<T>(args[0].addr, args[1].addr,succPos, 
                                         predPos, args[6].addr, source, 
                                         target, tt);
        return 0;
     }

     case REQUEST: {
        result.addr = li?li->next():0;
        return result.addr?YIELD:CANCEL;
     }
     case CLOSE: {
        if(li){
           delete li;
           local.addr = 0;
        }
        return 0;
     }

   }
   return -1;
}


template int gbidijkstraVMT<CcInt>(Word* args, Word& result, int message, 
                    Word& local, Supplier s);


template int gbidijkstraVMT<LongInt>(Word* args, Word& result, int message, 
                    Word& local, Supplier s);



} // end of namespace general_dijkstra




