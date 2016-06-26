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


namespace graph{

class Vertex;
class Edge;
class Graph;

// TODO change vector to arrays, i.e. Vertex**



/*
class Edge

*/
class Edge {

  friend class Vertex;
  friend class Graph;
  
public:

  Edge(Vertex* _dest, double _cost) :
    dest(_dest), cost(_cost) {}
  
private:
  Vertex* dest;
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
  
  
  bool hasEdge(Vertex* v) {
    return true;
  }
  
  bool equals(Vertex* v) {
    return true;
  }
  
  void print(std::ostream& out) const {
    out << "vertex: " << nr << std::endl;
    out << "adjacency list: ";
    for(size_t i=0; i<edges->size(); i++) {
      Edge* e = edges->at(i);
      out << "-> " << e->dest->nr << ", " << e->cost << " ";
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
  std::vector<Edge*>* edges;
  double dist;
  bool seen;
  Vertex* prev;
};




/*
class Graph

This class represents a graph as adjacency lists.

*/
class Graph {
  
public:
  Graph(){
    graph = new std::vector<Vertex*>();
  }
  
  // TODO
  ~Graph() {
    
  }
  
  bool isEmpty() {
    return graph->empty();
  }
  
  int size() {
    return graph->size();
  }
 
  
  Vertex* getVertex(int nr) {
    //std::cout << "sgetVertex" << std::endl;
    for(size_t i=0; i<graph->size(); i++) {
      if(graph->at(i)->nr == nr) {
        return graph->at(i);
      }
    }
    // vertex not found in graph, add as new vertex
    std::cout << "getVertex: add new vertex" << std::endl;
    Vertex* v = new Vertex(nr);
    v->print(std::cout);
    graph->push_back(v);
    return v;
  }
  
  
  void addEdge(int source, int dest, double cost) {
    std::cout << "addEdge" << std::endl;
    Vertex* v = getVertex(source);
    Vertex* w = getVertex(dest);
    v->edges->push_back((new Edge(w,cost)));
  }
  
  void clear(const bool b) {
    std::cout << "TODO: clear()" << std::endl;
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
  
  //
  // calculates the shortest path from start vertex to all other vertices
  //
  void shortestPath(Vertex* start) {
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
        Vertex* w = v->edges->at(i)->dest;
        double cost = v->edges->at(i)->cost;
//         std::cout << "w: " << w->nr << " cost: " << cost << std::endl;
        
        if(cost<0) {
          std::cout << "Error: cost is negative" << std::endl;
          continue;          
        }
        
        // shortening of path possible
        if(w->dist > v->dist+cost) {
//           std::cout << "w->dist > v->dist+cost: w dist: " << w->dist 
//                     << " v dist: " << v->dist << std::endl;
          w->dist = v->dist+cost;
          w->prev = v;
          path->push(w);
//           start = v;
//           start->dist = w->dist;
//           std::cout << "Distance: " << start->dist << std::endl;
        }
      }
    }
//     printPath(start);
  }
  
private:
  std::vector<Vertex*>* graph;
};


} // end namespace

#endif