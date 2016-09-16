/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <set>
#include <limits>
#include <queue>
#include <stack>
#include <algorithm>


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"



namespace graph {

using namespace std;
class Edge;
class Vertex;
class Graph;


/*
class Edge

This class represents an edge in a graph.

*/

class Edge {

    friend class Vertex;
    friend class Graph;

public:

    Edge(Tuple* _edge, int _posSource, int _posDest,
         double _cost, double _dist)
        : edge(_edge),posSource(_posSource), posDest(_posDest),
          cost(_cost), dist(_dist) {}
          
    ~Edge() {
      edge->DeleteIfAllowed();
      edge = 0;
    }

    Tuple* getEdge() {
      return edge;
    }
    
    int getPosDest() {
      return posDest;
    }
    
/* 
~memSize~

Returns the memory size of this edge.

*/    
    size_t memSize() const {
      size_t res = sizeof(*this);
      return res;  
    }
    
private:

    Tuple* edge;
    int posSource;
    int posDest;
    double cost;
    double dist;
};



/*
class Vertex

This class represents a vertex in a graph.

*/

class Vertex {

    friend class Graph;
    friend class Queue;
    friend class Comp;

public:
  
/*
struct EqualVertex

This stuct is used to compare to vertices. 

*/  
    struct EqualVertex {
      
      /*      
      The operator() returns true if the node number of the first 
      vertex is smaller than the node number of the second vertex.
        
      */
      bool operator()(const Vertex* v1, const Vertex* v2) const {
        return v1->nr < v2->nr;
      }
      
    };

    Vertex(int _nr):
        nr(_nr), seen(false), 
        cost(std::numeric_limits<double>::max()), 
        dist(std::numeric_limits<double>::max()), prev(0), 
        index(0), lowlink(0), inStack(false), compNo(0) {
      
      edges = new std::vector<Edge*>();
    }
    
    ~Vertex() {
      for(size_t i=0; i<edges->size(); i++) {
        delete edges->at(i);
      }
      delete edges; 
    }
    
    std::size_t operator()(const Vertex &v) const {
      return v.nr;
    }
    
    int getNr() {
        return nr;
    }
    
    Vertex* getPrev() {
      return prev; 
    }
    
    void setPrev(Vertex* v) {
      prev = v; 
    }
    
    void setCost(double i) {
      cost = i; 
    }
    
    void setDist(double i) {
      dist = i;
    }
    
    double getCost() {
      return cost; 
    }
    
    double getDist() {
      return dist; 
    }
    
    void isSeen(bool b) {
      seen = b; 
    }
    
    bool wasSeen() {
      return seen; 
    }

    std::vector<Edge*>* getEdges() {
      return edges;
    }

    int getCompNo() {
      return compNo;
    }

/* 
~getPath~

Adds all node numbers of the predecessors of this node to the
std::vector result.

*/
    std::vector<int>* getPath(std::vector<int>* result) {
      if(prev) {
          prev->getPath(result);
        }

        result->push_back(nr);
        return result;
    }

/* 
~print~

Prints this node including its adjacent nodes to the console.

*/
    void print(std::ostream& out) const {
        out << "vertex: " << nr << std::endl;
        out << "adjacency list: ";
        for(size_t i=0; i<edges->size(); i++) {
            Edge* e = edges->at(i);
            out << "-> "
                << ((CcInt*)e->edge->GetAttribute(e->posDest))->GetIntval()
                << ", " << e->cost << " "
                << ", " << e->dist << " ";
        }
        if(prev != 0)
          out << std::endl << "prev: " << prev->nr;
        out << std::endl << "seen: " << seen;
        out << std::endl << "CompNo: " << compNo << std::endl << std::endl;
    }

    void printPath() {
        if(prev) {
            prev->printPath();
            std::cout << " -> ";
        }
        std::cout << nr;
    }

/* 
~memSize~

Returns the memory size of this vertex.

*/       
    size_t memSize() const {
      size_t res = sizeof(*this) + (sizeof(void*) * edges->size());
      return res;  
    }

private:
    int nr;
    bool seen;
    double cost;
    double dist;
    Vertex* prev;
    int index;
    int lowlink;
    bool inStack;
    int compNo;
    std::vector<Edge*>* edges;
};




/* 
class Comp

This class is used to compare entries in a priority queue ~Queue~.

*/
class Comp {
  public:
    
    bool operator() (const Vertex* lhs, const Vertex* rhs) const {
      if(lhs->dist > rhs->dist) {
        return true;
      }
      return false;
    }
};


/* 
class Queue

The class implements a priority queue in which the Vertex v with
the smallest ~dist~ value is at the front of the queue.

*/
struct Queue {
 
  Queue() {
    queue = new std::vector<Vertex*>();    
    std::make_heap(queue->begin(),queue->end(),Comp());
  }
  
  ~Queue() {
    clear();
    delete queue;
  }
  
/*
~clear~

Deletes all elements in the queue.

*/   
  void clear() {
    while(queue->size() > 0){
      Vertex* v = queue->front();
      delete v;
      queue->pop_back();  
    }
  }

/*
~empty~

*/ 
  bool empty() {
    return queue->empty();
  }
  
/*
~size~

Returns the size of this queue.

*/ 
  size_t size() {
    return queue->size();
  }
  
/*
~top~

Returns the top element of this queue.

*/ 
  Vertex* top() {
    return queue->front();
  }
  
/*
~push~

Adds a new element to the queue.

*/ 
  void push(Vertex* entry) {
    queue->push_back(entry);
    std::push_heap(queue->begin(),queue->end(),Comp());
  }

/*
~pop~

Removes the top element from the queue.

*/ 
  void pop() {
    std::pop_heap(queue->begin(),queue->end(),Comp());
    queue->pop_back();
  }

/*
~print~

Prints all elements of the queue to the console.

*/   
  void print(std::ostream& out) {
    out << std::endl << "QUEUE:" << std::endl;
    for(size_t i=0; i<queue->size(); i++) {
      out << "NodeNumber: " << queue->at(i)->nr;
      if(queue->at(i)->prev != 0)
        out << " previous: " << queue->at(i)->prev->nr;
      out << " Cost: " << queue->at(i)->cost;
      out << " Distanz: " << queue->at(i)->dist << std::endl;
    }
  }
  
private:
  std::vector<Vertex*>* queue;
  std::greater<Vertex*> comp;
};



/*
class Graph

This class represents a graph as adjacency lists.

*/
class Graph {

    friend class Edge;
    

public:
    Graph() {
        graph = new set<Vertex*, Vertex::EqualVertex>();
        result = new vector<int>();
    }

    ~Graph() {
      set<Vertex*,Vertex::EqualVertex>::iterator it = graph->begin();
      while(it!=graph->end()) {
        Vertex* v = *it;
        delete v;
        it++;
      }
      delete graph;
      result->clear();
      delete result;
    }

/*
~reset~

Removes all entries from graph

*/
    void reset() {
       result->clear();
    }

/*
~isEmpty~

Returns true if this graph is empty.

*/
    bool isEmpty() {
        return graph->empty();
    }

/*
~size~

Returns the number of nodes in this graph.

*/    
    int size() {
        return graph->size();
    }

/*
~getGraph~

*/
    std::set<Vertex*,Vertex::EqualVertex>* getGraph() {
      return graph; 
    }

/*
~getVertex~

If the nodenumber is already known in the graph, the node is returned,
otherwise a new node with that number is created and returned.

*/
    Vertex* getVertex(int nr) {     
      Vertex* v = new Vertex(nr);
      std::set<Vertex*,Vertex::EqualVertex>::iterator it;
      it = graph->find(v);
      if(it == graph->end()) {
        graph->insert(v);
        return v;
      }
      delete v;
      return *it;
    }

/*
~addEdge~

Adds a new edge to the graph.

*/
    void addEdge(Tuple* edge, int posSource, 
                 int posDest, double cost, double dist) {

      Vertex* v = getVertex(((CcInt*)edge->
                              GetAttribute(posSource))->GetIntval());
      
      getVertex(((CcInt*)edge->GetAttribute(posDest))->GetIntval());
      v->edges->push_back((new Edge(edge,posSource,posDest,cost,dist)));
    }

/*
~clear~

Deletes all nodes from this graph. 

*/    
    void clear() {
      std::set<Vertex*,Vertex::EqualVertex>::iterator it = graph->begin();
      while(it!=graph->end()) {
        Vertex* v = *it;
        delete v;
        it++;
      }
    }

/*
~getPath~

Recursively adds all predecessors of a given node to the std::vector result.

*/
    void getPath(Vertex* dest) {

        if(dest->cost==std::numeric_limits<double>::max())
          std::cout << "Error, " << dest->nr << " not reachable\n";
        else
          dest->getPath(result);
    }

/*
~getResult~

Returns a pointer to the std::vector result.

*/    
    std::vector<int>* getResult() {
        return result;
    }

    void tarjan() {

      int index = 0;                            
      int compNo = 1;                
      std::stack<Vertex*>* stack = new std::stack<Vertex*>();       
      std::set<Vertex*,Vertex::EqualVertex>::iterator it = graph->begin();
      while(it!=graph->end()) {
        tarjan(*it,index,stack,compNo);     
        it++;
      }
      delete stack;
    }
 
/*
~tarjan~

Calculates the scc for the given vertex.

*/   

     void tarjan(Vertex* v, int& index, 
                 std::stack<Vertex*>* stack, int& compNo) {

      v->index = index;                 
      v->lowlink = index;      
      index++;                          
      stack->push(v);                  
      v->inStack = true;
      v->seen = true;                 
      Vertex* w;
      // visit all adjacent nodes
      for(size_t i=0; i<v->edges->size(); i++) {     
        w = getVertex(((CcInt*)v->edges->at(i)->
            edge->GetAttribute(v->edges->at(i)->posDest))->GetIntval());
        if(!w->seen) {
          tarjan(w,index,stack,compNo);    // recursive call
          v->lowlink = std::min(v->lowlink,w->lowlink);
        }
        else if (w->inStack)
          v->lowlink = std::min(v->lowlink,w->index);
      }
      // root of scc
      if (v->lowlink == v->index) { 
        v->compNo = compNo;
        while(true) {
          w = stack->top();
          stack->pop();
          if(v->nr == w->nr)
            break;
          w->compNo = compNo;
        }
        compNo++;
      }
      
    }
    
/*
~getEdge~

Returns the tuple with start node ~source~ and end node ~dest~.

*/ 
    Tuple* getEdge(int source, int dest) {
      
      std::set<Vertex*,Vertex::EqualVertex>::iterator it;
      Vertex* w = new Vertex(source);
      
      it = graph->find(w);
      Vertex* v = *it;
      delete w;
      
      for(size_t j=0; j<v->edges->size(); j++) {
        Edge* e = v->edges->at(j);
        if(((CcInt*)e->edge->GetAttribute(e->posDest))->GetIntval() 
                                                              == dest) {
            return e->edge;
        }
      }
      return 0;
    }

/*
~print~

Prints this graph to the console.

*/ 
    void print(std::ostream& out) {
      out << "------> GRAPH: " << std::endl;
      out << "GRAPH size: " << this->memSize() << endl;
      std::set<Vertex*,Vertex::EqualVertex>::iterator it = graph->begin();
      while(it!=graph->end()) {
        Vertex* v = *it;     
        v->print(out);
        it++;
      }
    }
    
/* 
~memSize~

Returns the memory size of this graph.

*/   
    size_t memSize() const {
      size_t res = sizeof(*this) + (sizeof(void*) * graph->size());
      return res;  
    }


private:
    std::set<Vertex*, Vertex::EqualVertex>* graph;
    std::vector<int>* result;
};


} // end namespace

#endif
