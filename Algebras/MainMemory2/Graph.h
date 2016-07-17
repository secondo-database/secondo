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
#include <limits>
#include <queue>  


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"

namespace graph {


class Edge;
class Vertex;
class Graph;


/*
class Edge

*/

class Edge {

  friend class Vertex;
  friend class Graph;
  
public:
  
  Edge(Tuple* _edge, int _posSource, int _posDest, double _cost) 
    : edge(_edge),posSource(_posSource), posDest(_posDest), cost(_cost) {}
  
private:

  Tuple* edge;
  int posSource;
  int posDest;
  double cost;
};



/*
class Vertex

This class 

*/
class Vertex {
  
  friend class Graph;
  
public:
  
  Vertex(int _nr): 
    nr(_nr), seen(false), dist(std::numeric_limits<double>::max()), prev(0) {
    edges = new std::vector<Edge*>();
  }
  
  int getNr() {
    return nr;
  }
  
//   bool hasEdge(Vertex* v) {
//     return true;
//   }
  
//   bool equals(Vertex* v) {
//     return true;
//   }
  
  
  std::vector<int>* getPath(std::vector<int>* result) {
    if(prev) 
      prev->getPath(result);

    result->push_back(nr);
    return result;
  } 

  
  void print(std::ostream& out) const {
    out << "vertex: " << nr << std::endl;
    out << "adjacency list: ";
    for(size_t i=0; i<edges->size(); i++) {
      Edge* e = edges->at(i);
      out << "-> " 
          << ((CcInt*)e->edge->GetAttribute(e->posDest))->GetIntval() 
          << ", " << e->cost << " ";
    }     
    out << std::endl << "seen: " << seen << std::endl << std::endl;
  } 
  
  void printPath() {
    if(prev) {
      prev->printPath();
      std::cout << " -> ";
    }
    std::cout << nr;
  }
  
  
private:
  int nr;
  bool seen;
  double dist;
  Vertex* prev;
  std::vector<Edge*>* edges;
};




/*
class Graph

This class represents a graph as adjacency lists.

*/
class Graph {
  
  friend class Edge;
  
public:
  Graph() {
    graph = new std::vector<Vertex*>();
    result = new std::vector<int>();
  }
  
  // TODO
  ~Graph() {
    delete graph;
    delete result;
  }
  
  bool isEmpty() {
    return graph->empty();
  }
  
  int size() {
    return graph->size();
  }
 
  
  Vertex* getVertex(int nr) {
    for(size_t i=0; i<graph->size(); i++) {
      if(graph->at(i)->nr == nr) {
        return graph->at(i);
      }
    }
    // vertex not found in graph, add as new vertex
    Vertex* v = new Vertex(nr);
    graph->push_back(v);
    return v;
  }
  

  void addEdge(Tuple* edge, int posSource, int posDest, double cost) {
    Vertex* v = getVertex(((CcInt*)edge->GetAttribute(posSource))->GetIntval());
    getVertex(((CcInt*)edge->GetAttribute(posDest))->GetIntval());
    v->edges->push_back((new Edge(edge,posSource,posDest,cost)));
  }
  
  
  
  // TODO
  void clear(const bool b) {
    std::cout << "TODO: clear()" << std::endl;
  }
  
  
  
  
  void getPath(Vertex* dest) {
    
    if(dest->dist==std::numeric_limits<double>::max()) 
      std::cout << "Error, " << dest->nr << " not reachable\n";
    else 
      dest->getPath(result);
  }
  
  
  //
  // calculates the shortest path from start vertex to all other vertices
  //
  bool shortestPath(Vertex* start) {
    std::queue<Vertex*>* path = new std::queue<Vertex*>();
    
    for(size_t i=0; i<graph->size(); i++) {
      graph->at(i)->dist = std::numeric_limits<double>::max();
      graph->at(i)->seen = false;
      graph->at(i)->prev = 0;
    }
    
    // cost to start node
    start->dist = 0;
    path->push(start);
    
    
    // as long as queue has entries
    while(!path->empty()) {
      Vertex* v = path->front();
      path->pop();
      if(v->seen) continue;
      v->seen = true;
      
      // for every adjacent edge of v
      for(size_t i=0; i<v->edges->size(); i++) {
        Vertex* w = getVertex(((CcInt*)v->edges->at(i)->
                    edge->GetAttribute(v->edges->at(i)->posDest))->GetIntval());
        double cost = v->edges->at(i)->cost;
        
        if(cost<0) {
          std::cout << "Error: cost is negative" << std::endl;
          return false;          
        }
        
        // shortening of path possible
        if(w->dist > v->dist+cost) {
          w->dist = v->dist+cost;
          w->prev = v;
          path->push(w);
        }
      }
    }
    return true;
  }
  
  std::vector<int>* getResult() {
    return result; 
  }
  
  
  Tuple* getEdge(int source, int dest) {
    for(size_t i=0; i<graph->size(); i++) {
      if(graph->at(i)->nr == source) {
        std::cout << "nr: " << source << std::endl; 
        Vertex* v = graph->at(i);
        
        for(size_t j=0; j<v->edges->size(); j++) {
          Edge* e = v->edges->at(j);
          if(((CcInt*)e->edge->GetAttribute(e->posDest))->GetIntval() == dest) {
            std::cout << "dest: " << dest << std::endl; 
            return e->edge;
          }
        }
      }
    }
    return 0;
  }
  
  
  void print(std::ostream& out) {
    out << "------> GRAPH: " << std::endl;
    for(size_t i=0; i<graph->size(); i++) {
      graph->at(i)->print(out);
    }     
  }
  
  void printPath(Vertex* dest) {
    if(dest->dist==std::numeric_limits<double>::max()) {
      std::cout << "Error, " << dest->nr << " not reachable\n";
    }
    else {
      std::cout << "The shortest path, with cost " << dest->dist << " is: ";
      dest->printPath();
      std::cout << std::endl << std::endl;
    }
  }
  
  
private:
  std::vector<Vertex*>* graph;
  std::vector<int>* result;
};


} // end namespace

#endif