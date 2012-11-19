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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the Graph Algebra

February 2007, G. Barz, M. Stein, A. Ruloffs, A. Martin


[TOC]

1 Overview

This header file essentially contains the definition of the classes ~Vertex~,
~Edge~, ~Path~, and ~Graph~ used in the Graph Algebra. These classes
respectively correspond to the memory representation for the type constructors
~vertex~, ~edge~, ~path~, and ~graph~.

2 Defines and includes

*/
#ifndef GRAPHALGEBRA_H_
#define GRAPHALGEBRA_H_

using namespace std;


#include <vector>
#include <iostream>
#include "../../Tools/Flob/DbArray.h"        //needed in graph and path
#include "SpatialAlgebra.h"    //needed for Points
#include "Algebra.h"        //always needed in Algebras
#include "NestedList.h"        //always needed in Algebras
#include "QueryProcessor.h"    //always needed in Algebras
#include "StandardTypes.h"    //always needed in Algebras


extern NestedList* nl;
extern QueryProcessor *qp;

/*
3 Class Vertex

*/
class Vertex: public Attribute
{
    public:
/*
3.1 Constructors and Destructor

*/
        Vertex();
        Vertex(bool d):Attribute(d), key(0),pos(false,0,0) { }
        Vertex(int nKey, Point const & pntPos):
           Attribute(true),key(nKey), pos(pntPos){ }
        Vertex(int nKey, Coord coordX, Coord coordY):
           Attribute(true),
           key(nKey), pos(true,coordX,coordY){ }

        ~Vertex();
/*
3.2 Get functions

3.2.1 GetKey

*/
        int GetKey() const{return key;}

        int GetIntval() const {return key;}
/*
3.2.2 GetPos

*/
        Point const & GetPos() const{return pos;}

/*
3.3 Set functions

3.3.1 SetKey

*/
        void SetKey(int nKey){key = nKey;}
/*
3.3.2 SetPos

*/
        void SetPos(Point const & pntPos){pos = pntPos;}
        void SetPos(Coord coordX, Coord coordY);

/*
3.4 Functions needed to import the ~vertex~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
          Vertex* Clone() const;
          int Compare(const Attribute*) const;
          bool Adjacent(const Attribute*) const;
          size_t Sizeof() const;

          size_t HashValue() const;
          void CopyFrom(const Attribute* arg);
          inline virtual ostream& Print( ostream& os ) const
          {
            if( IsDefined() )
            {
              os << "Vertex: ( " << key << " @"; pos.Print(os); os << " )";
            }
            else
            {
              os << "Vertex: ( undefined )";
            }
            return os;
          }

          static const string BasicType() { return "vertex"; }
          static const bool checkType(const ListExpr type){
            return listutils::isSymbol(type, BasicType());
          }

    private:
/*
3.5 Attributes

*/
        int key;
        Point pos;
};

/*
4 Class Edge

*/
class Edge: public Attribute
{
    public:
/*
4.1 Constructors and Destructor

*/
                Edge();
        Edge(bool d)  { SetDefined(d);}
                Edge(int nSource, int nTarget, float fCost);
        ~Edge();
/*
4.2 Get functions

4.2.1 GetSource

*/
        int GetSource() const{return source;}
/*
4.2.2 GetTarget

*/
        int GetTarget() const{return target;}
/*
4.2.3 GetCost

*/
        float GetCost() const{return cost;}
/*
4.3 Set funtions

4.3.1 SetSource

*/
        void SetSource(int nSource){source = nSource;}
/*
4.3.2 SetTarget

*/
        void SetTarget(int nTarget){target = nTarget;}
/*
4.3.3 SetCost

*/
        void SetCost(float fCost){cost = fCost;}
/*
4.4 Functions needed to import the ~edge~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
        //Implementations from Attribute:
        Edge* Clone() const;
        int Compare(const Attribute*) const;
        bool Adjacent(const Attribute*) const;
        size_t Sizeof() const;
        size_t HashValue() const;
        void CopyFrom(const Attribute* arg);
        inline virtual ostream& Print( ostream& os ) const
        {
          if(IsDefined())
             os << "Edge: ( " << source << " --> "
                << target <<", " << cost << " )";
          else
            os << "Edge: ( undefined )";
          return os;
        }

        static const string BasicType() { return "edge"; }
        static const bool checkType(const ListExpr type){
          return listutils::isSymbol(type, BasicType());
        }

    private:
/*
4.5 Attributes

*/
        int source;
        int target;
        float cost;
};

/*
5 Class Path

5.1 Struct pathStruct

*/
struct pathStruct
{
        int key;
        Point pos;
        float cost;
        inline ostream& Print( ostream& os ) const
        {
          os << "pathStruct: ( " << key << " @"; pos.Print(os);
          os << ": " << cost << " )";
          return os;
        }
};

class Path: public Attribute
{
    public:
/*
5.2 Constructor and Destructor

*/
        Path();
        Path(bool bDefined);
        ~Path();
/*
5.2 Path query functions

5.2.1 IsEmpty

*/
        bool IsEmpty() const{return myPath.Size() == 0;}


/*
5.2.2 Clear

This function removes all vertices from the path.

*/
  void Clear(){
      myPath.clean();
      cost = 0.0;
      SetDefined(true);
  }

/*
5.2.2 GetCost

*/
        float GetCost() const;
/*
5.2.3 GetNoPathStructs

*/
        int GetNoPathStructs() const {return myPath.Size();}
/*
5.2.4 GetPathStruct

*/
        pathStruct GetPathStruct(int nIndex) const;
/*
5.2.5 GetEdges

*/
        vector<Edge>* GetEdges() const;
/*
5.2.6 GetVertices

*/
        vector<Vertex>* GetVertices() const;
/*
5.3 Path manipulation functions

5.3.1 Append

*/
        void Append(const pathStruct& pathstruct){
          cost += pathstruct.cost ; myPath.Append(pathstruct);}

/*
5.3.2 Destroy

*/
        void Destroy(){myPath.Destroy();}


/*
5.3.3 EqualWay

This function checks the equality of two path instances
ignoring the costs for the edges and positions of the vertices.

*/
        void EqualWay(const Path* P, CcBool& result) const;


/*
5.4   Functions needed to import the ~path~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
        int NumOfFLOBs() const;
        Flob *GetFLOB(const int i);
        Path* Clone() const;
        int Compare(const Attribute*) const;
        bool Adjacent(const Attribute*) const;
        size_t Sizeof() const;

        size_t HashValue() const;
        void CopyFrom(const Attribute* arg);
        inline virtual ostream& Print( ostream& os ) const
        {
          os << "Path: ( ";
          if (IsDefined())
          {
            os << endl;
            for( int i = 0; i < myPath.Size(); i++)
            {
              pathStruct ps;
              myPath.Get(i, ps);
              os << "\t\t"; ps.Print(os); os << endl;
            }
            os << "      Cost = " << cost << " )" << endl;
          }
          else
          {
            os << " undefined )" << endl;
          }
          return os;
        }

        static const string BasicType() { return "path"; }
        static const bool checkType(const ListExpr type){
          return listutils::isSymbol(type, BasicType());
        }

    protected:
/*
5.5 Attributes

*/
        DbArray<pathStruct> myPath;
        float cost;
};


/*
6 Struct AVLTree

This structure provides static functions to store keys and additional
information of a certain type T in AVL trees by using a DBArray.
The DBArray must always be passed over as first argument and consists of node slots
which can either be free or used by a node. Since the index of the root node is
passed as additional argument it is possible to manage several AVL trees in the same
DBArray. There is no mechanism to delete or reuse free node slots, every new node
will be appended to the DBArray.


6.1 A template for the nodes

*/
template<class T>
struct AVLNode
{
     int key;
/*
The key of this node.

*/
     T elem;
/*
The additional informations of this node are stored in this template class.

*/
     int left;
/*
The DBArray index of the left son.
The value must be -1 if this node has no left son.

*/
     int right;
/*
The DBArray index of the right son.
The value must be -1 if this node has no right son.

*/
     int leftDepth;
/*
The depth of the left tree of this node.

*/
     int rightDepth;
/*
The depth of the right tree of this node.

*/
     bool free;
/*
A flag that tells if the DBArray index of this node is no longer used.

*/

     inline int Balance() const;
/*
Returns the balance of the node.

*/
};

template<class T>
ostream& operator <<( ostream& o, const AVLNode<T>& n );

/*
6.2 Static functions for AVL tree management

*/
template<class T>
struct AVLTree
{
      public:
/*
6.2.1 InsertKey

*/
     static int InsertKey(DbArray<AVLNode<T> >& tree,
                   const int key, const T elem, const int index);
/*
Insert a new node with key ~key~ and additional informations ~elem~ in the tree with
the root node at position ~index~ and returns the DBArray index of the new root node.
The operation fails and returns -1 if there was already a node with that key in the tree.

*Precondition*: -1 $\le$ index $<$ tree.Size()

*Complexity*: $C(log n)$, where ~n~ is the number of nodes in the tree


6.2.2 UpdateKey

*/
     static bool UpdateKey(DbArray<AVLNode<T> >& tree,
                    const int key, const T elem, const int index);
/*
Sets the additional informations of the node with the key ~key~ in the tree with the
root node at DBArray position ~index~ to ~elem~ and returns if the operation was
successful.
The operation fails if there is no node with that key in the tree.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(log n)$, where ~n~ is the number of nodes in the tree


6.2.3 ReplaceKey

*/
     static int ReplaceKey(DbArray<AVLNode<T> >& tree,
                  const int key, const int newKey, const int root);
/*
Replaces the key of the node with key ~key~ in the tree with the root node at
DBArray position ~index~ by ~newKey~ and returns the index of the new root node.
The operation fails and returns -2 if there is no node with this key.

*Precondition:* -1 $\le$ root $<$ tree.Size()

*Complexity:* $C(log n)$, where ~n~ is the number of nodes in the tree


6.2.4 DeleteKey

*/
     static int DeleteKey(DbArray<AVLNode<T> >& tree,
                          const int key, const int index);
/*
Deletes the node with the key ~key~ in the tree with the root node at
DBArray position ~index~ and returns the index of the new root node.
The operation fails and returns -2 if there is no node with this key.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(log n)$, where ~n~ is the number of nodes in the tree


6.2.5 DeleteMinKey

*/
     static int DeleteMinKey(DbArray<AVLNode<T> >& tree,
                             const int index, AVLNode<T>& minNode);
/*
Deletes the node with the minimum key in the tree starting at DBArray
position ~index~ and returns the index of the new root node and the node
that contains the minimum key in ~minNode~.

*Precondition:* 0 $\le$ index $<$ tree.Size()

*Complexity:* $C(log n)$, where ~n~ is the number of nodes in the tree


6.2.6 DeleteKeys

*/
     static void DeleteKeys(DbArray<AVLNode<T> >& tree,
                            const int index);
/*
Deletes all nodes of the tree starting at DBArray position ~index~.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(n)$, where ~n~ is the number of nodes in the tree


6.2.7 DeleteNode

*/
     static bool DeleteNode(DbArray<AVLNode<T> >& tree,
                    const int index, const AVLNode<T>* node = 0);
/*
Deletes the node at the DBArray position ~index~ and returns if the operation
was successful. If ~node~ is 0 the node will be loaded from the DBArray,
otherwise ~node~ is regarded to be the value of the node.
The operation fails if the node is already deleted.

*Precondition:* 0 $\le$ index $<$ tree.Size()

*Complexity:* $C(1)$


6.2.8 UpdateNode

*/
     static void UpdateNode(DbArray<AVLNode<T> >& tree,
                            const int index, const T& elem);
/*
Sets the additional informations of the node at DBArray position ~index~ to
~elem~.

*Precondition:* 0 $\le$ index $<$ tree.Size()

*Complexity:* $C(1)$


6.2.9 MapKeys

*/
     static void MapKeys (DbArray<AVLNode<T> >& tree,
                          int& num, vector<int>& v, const int index);

/*
Maps the keys of the tree with the root node at DBArray position ~index~ in
ascending order to ~num~, ... ,~num~+n-1 and returns the mapping that was
used in the vector ~v~.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexitiy:* $C(n)$


6.2.10 HasKey

*/
     static bool HasKey(const DbArray<AVLNode<T> >& tree,
                        const int key, const int index);
/*
Returns whether the tree with the root node at DBArray position ~index~ has a node
with the key ~key~ or not.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(log n)$, where ~n~ is the number of nodes in the tree


6.2.11 ReadKeys

*/
     static int ReadKeys(const DbArray<AVLNode<T> >& tree,
                         vector<int>* v, const int index);
/*
Adds all keys from the tree with the root node at DBArray position ~index~ to the
vector ~v~ and returns the number of elements added to the vector.
The keys are sorted in ascending order.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(n)$, where ~n~ is the number of nodes in the tree


6.2.12 ReadNodes

*/
     static int ReadNodes(const DbArray<AVLNode<T> >& tree,
                          vector<AVLNode<T> >& v, const int index);
/*
Adds all nodes from the tree with the root node at DBArray position ~index~ to the
vector ~v~ and returns the number of elements added to the vector.
The nodes are sorted by their key in ascending order.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(n)$, where ~n~ is the number of nodes in the tree


6.2.13 ReadOptNodes

*/
     static int ReadOptNodes(const DbArray<AVLNode<T> >& tree,
                   vector<AVLNode<T> >& v, const int root);
/*
Adds all nodes from the tree with the root node at DBArray position ~index~ to the
vector ~v~ and returns the number of elements added to the vector.
The nodes will be stored in an optimized order that allows to rebuild the tree
without rebalancing.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(n)$, where ~n~ is the number of nodes in the tree


6.2.14 ReadNode

*/
     static int ReadNode(const DbArray<AVLNode<T> >& tree,
                    AVLNode<T>& n, const int key, const int index);
/*
Copies the node with key ~key~ of the tree with the root node at DBArray position
~index~ to ~n~ and returns the index of this node.
The operation fails and returns -1 if there is no node with this key in the tree.

*Precondition:* -1 $\le$ index $<$ tree.Size()

*Complexity:* $C(log n)$, where ~n~ is the number of nodes in the tree


6.2.15 NumNodes

*/
     static inline int NumNodes(const DbArray<AVLNode<T> >& tree);
/*
Returns the number of nodes stored in the DBArray.


6.2.16 NewNode

*/
     static AVLNode<T> NewNode (const int key, const T elem);
/*
Returns a new node with key ~key~ and additional informations ~elem~.

*/
     static AVLNode<T> NewNode (const AVLNode<T>* n);
/*
Returns a new node which is a copy of the node ~n~.

*/
   private:
/*
6.2.17 Private functions

*/
     static int Rebalance(DbArray<AVLNode<T> >& tree,
                          const int index);
/*
Rebalances the unbalanced node at DBArray position ~index~ and returns the index
of the new root node.

*Precondition:* 0 $\le$ index $<$ tree.Size() \&\& abs(tree[index].Balance()) == 2

*Complexity:* $C(1)$

*/
     static inline int BalancedTree(DbArray<AVLNode<T> >& tree,
                            const AVLNode<T>& node, const int root);
/*
Returns ~root~ if the balance of ~node~ is ok, and otherwise the index of the new
root node after rebalancing the tree.

*Precondition:* 0 $\le$ root $<$ tree.Size()

*Complexity:* $C(1)$

*/
};

/*
6.3 Struct verticesStruct

Used to store additional informations of a vertex.

*/
struct verticesStruct
{
     Point pos;
/*
The position of this vertex.

*/
     int succ;
/*
The DBArray index in adjlist where the root node of the AVL tree that contains the
successors of this vertex is located.
The value must be -1 if this vertex has no successors.

*/
     int inDeg;
/*
The input degree of this vertex.

*/
     int outDeg;
/*
The output degree of this vertex.

*/
};

ostream& operator<< (ostream& o, const verticesStruct& m);


/*
6.4 Struct adjStruct

Used to store additional information of an edge.

*/
struct adjStruct
{
      float cost;
/*
The costs of this edge.

*/
      int keyInt;
/*
The DBArray index in vertices that represents the target vertex of this edge.

*/
};

ostream& operator<< (ostream& o, const adjStruct& m);

/*
7 Class Graph

This class implements the memory representation of the ~graph~ type constructor.
A graph value consists of 2 sets: a set of vertices and a set of directed and
weightened edges between these vertices. Both sets are stored using AVL trees, that
allows a graph construction in $C(n * log n)$. Each vertex has a unique integer as
key, a pointer to the AVL tree that contains all outgoing edges (the adjacent list
of the vertex) and a (optional) position of the vertex for the case that the graph
should be displayed graphically. An edge is determined by its source and target
vertex (resp. by the source and target keys) and has a real value as its costs.
After the usage of delete operations on the graph, the Minimize() function should be
called to minimize the size of the DBArrays that are used for the AVL trees.

*/
class Graph: public Attribute
{
   public:
/*
7.1 Constructors and Destructor

There are three ways of constructing a graph:

*/
     Graph();
/*
This constructor should not be used.

*/
     Graph(const bool Defined);
/*
This constructor createa a new empty graph if ~Defined~ is true, otherwise the graph is
undefined.

*/
     Graph(const Graph& g);
/*
This constructor creates a copy of the graph ~g~.

*/
     void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent graph. It marks the persistent arrays for destroying. The
destructor will perform the real destroying.

*/
     ~Graph();
/*
The destructor.


7.2 Functions for graph construction and manipulation


7.2.1 AddVertex

*/
     bool AddVertex(const int key, const Point &pos);
/*
Adds a new vertex with number ~key~ and the position ~pos~ to the graph.
Returns whether the operation was successful or not.
The operation fails if there is already a key with that number in the
graph.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.2 AddEdge

*/
     bool AddEdge(const int sourceKey, const int targetKey,
                  const float cost);
/*
Adds a new directed edge from ~sourceKey~ to ~targetKey~  and the costs
~cost~ to the graph.
Returns whether the operation was successful or not.
The operation fails if there is already a directed edge from ~sourceKey~
to ~targetKey~ in the graph, or if the graph doesn't have a one of the
vertices with numbers ~targetkey~ or ~sourceKey~

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.3 Add

*/
     bool Add(const Vertex& v);
/*
Adds a new vertex ~v~ to the graph.
Returns whether the operation was successful or not.
The operation fails if there is already a key with the same number as ~v~
in the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)

*/
     bool Add(const Edge& e);
/*
Adds a new directed edge ~e~ to the graph.
Returns whether the operation was successful or not.
The operation fails if there is already a directed edge with the same source
and target key as ~e~ in the graph, or if the graph doesn't have the source
and target key of ~e~.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)

*/
     void Add(const Graph& g);
/*
Adds all missing vertices and edges of the graph ~g~ to the current
graph.

*Precondition:* ~g~ and the current graph are defined.

*Complexity:* $C(n * log n)$, where ~n~ is $|V|+|E|$ of the graph ~g~ = (V,E)


7.2.4 DeleteVertex

*/
     bool DeleteVertex (const int key);
/*
Deletes the vertex with the number ~key~ and all ingoing and outgoing edges
from this vertex in the graph.
Returns whether the operation was successful or not.
The operation fails if there is no vertex with the number ~key~.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.5 DeleteEdge

*/
     bool DeleteEdge (const int sourceKey, const int targetKey);
/*
Deletes the directed edge from ~sourceKey~ to ~targetKey~.
Returns whether the operation was successful or not.
The operation fails if there is no edge from ~sourceKey~ to ~targetKey~ in
the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.6 Delete

*/
     inline bool Delete(const Vertex& v);
/*
Deletes the vertex with the same key as the vertex ~v~ in the graph and all
all ingoing and outgoing edges from this vertex.
Returns whether the operation was successful or not.
The operation fails if there is no vertex with the same key as ~v~.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)

*/
     inline bool Delete(const Edge& e);
/*
Deletes the directed edge with the same source and target as the edge ~e~
in the graph.
Returns whether the operation was successful or not.
The operation fails if there is no edge with the same source and target as
~e~ the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.7 SetPos

*/
     bool SetPos(const int key, const Point &pos);
/*
Sets the vertex with the number ~key~ in the graph to the position of the
point ~pos~.
Returns whether the operation was successful or not.
The operation fails if there is no vertex with number ~key~ in the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.8 SetCost

*/
     bool SetCost(const int sourceKey, const int targetKey,
                  const float cost);
/*
Sets the costs of the directed edge from ~sourceKey~ to ~targetKey~ in the
graph to ~cost~.
Returns whether the operation was successful or not.
The operation fails if there is no edge from ~sourceKey~ to ~targetKey~
in the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.9 Update

*/
     inline bool Update (const Vertex& v);
/*
Sets the position of the vertex with the same key as the vertex ~v~ to the
position of ~v~.
Returns whether the operation was successful or not.
The operation fails if the graph has no vertex with the same key as ~v~.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)

*/
     inline bool Update (const Edge& e);
/*
Sets the costs of the directed edge with the same source and target as the
edge ~e~ in the graph to the costs from ~e~.
Returns whether the operation was successful or not.
The operation fails if there is no edge with the same source and target as
~e~ in the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.2.10 Clear

*/
     inline void Clear();
/*
Clears the sets of vertices and edges of the graph.

7.3 Functions for graph queries


7.3.2 EqualsWith

*/
     bool EqualsWith(const Graph* other) const;
/*
Returns if the graph is equal to the other graph.

*Precondition:* both graphs are defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.3 IsEmpty

*/
     inline bool IsEmpty() const;
/*
Returns wether the graph is empty or not.

*Precondition:* the graph is defined.

*Complexity:* $C(1)$


7.3.1 PartOf

*/
     bool PartOf(const Graph* mayPart) const;
/*



7.3.4 GetVertex

*/
     Vertex GetVertex(const int key) const;
/*
Returns the vertex of the graph with the number ~key~ or an undefined
vertex if the graph has no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.5 GetEdge

*/
     Edge GetEdge(const int sourceKey,
                         const int targetKey) const;
/*
Returns the directed edge of the graph going from ~sourceKey~ to
~targetKey~ or an undefined edge if the graph has no directed edge
between the vertices with these keys.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.6 GetInDegFrom

*/
     int GetInDegFrom(const int key) const;
/*
Returns the input degree of the vertex with the number ~key~ or -1 if
the graph has no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.7 GetOutDegFrom

*/
     int GetOutDegFrom(const int key) const;
/*
Returns the output degree of the vertex with the number ~key~ or -1 if
the graph has no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.8 GetMaxDeg

*/
     int GetMaxDeg(const bool out) const;
/*
Returns the maximum output degree of the graph if ~opt~ is true and otherwise
the maximum input degree.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.9 GetMinDeg

*/
     int GetMinDeg(const bool out) const;
/*
Returns the minimum output degree of the graph if ~opt~ is true and otherwise
the minimum input degree.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.10 HasVertex

*/
     bool HasVertex(const int key) const;
/*
Returns whether the graph has a vertex with the number ~key~ or not.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.11 HasEdge

*/
     bool HasEdge(const int sourceKey,
                         const int targetKey) const;
/*
Returns whether the graph has a directed edge going from ~sourceKey~
to ~targetKey~ or not.

*Precondition:* the graph is defined.

*Complexity:* $C(log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.12 GetVertices

*/
     vector<Vertex>* GetVertices(const bool opt = false) const;
/*
Returns all vertices of the graph as vector.
If ~opt~ is false, the vertices are ordered by their key in ascending
order, and if ~opt~ is true the vertices are stored in an optimized
order that allows to build a graph without rebalancing the AVL tree.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.13 GetEdges

*/
     vector<Edge>* GetEdges(const bool opt = false) const;
/*
Returns all edges of the graph as vector.
If ~opt~ is false, the edges are ordered by their source and target
key in ascending order, and if ~opt~ is true the edges are stored in
an optimized order that allows to build a graph without rebalancing
the AVL tree.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.14 GetSuccFrom

*/
     vector<Vertex>* GetSuccFrom(const int key) const;
/*
Returns a pointer to a vector with all vertices that are direct
successors of the vertex with the number ~key~, or 0 if there is
no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.15 GetPredFrom

*/
     vector<Vertex>* GetPredFrom(const int key) const;
/*
Returns a pointer to a vector with all vertices that are direct
predecessors of the vertex with the number ~key~, or 0 if there
is no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.16 GetSuccKeysFrom

*/
     vector<int>* GetSuccKeysFrom(const int key) const;
/*
Returns a pointer to a vector with all keys that are direct
successors of the vertex with the number ~key~, or 0 if there
is no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.17 GetPredKeysFrom

*/
     vector<int>* GetPredKeysFrom(const int key) const;
/*
Returns a pointer to a vector with all keys that are direct
predecessors of the vertex with the number ~key~, or 0 if there
is no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.18 GetSuccEdgesFrom

*/
     vector<Edge>* GetSuccEdgesFrom(const int key) const;
/*
Returns a pointer to a vector with all outgoing edges of the of the
vertex with the number ~key~, or 0 if there
is no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.19 GetPredEdgesFrom

*/
     vector<Edge>* GetPredEdgesFrom(const int key) const;
/*
Returns a pointer to a vector with all ingoing edges of the of the
vertex with the number ~key~, or 0 if there
is no vertex with this key.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.20 GetNumVertices

*/
     inline int GetNumVertices() const;
/*
Returns the number of the vertices from the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(1)$


7.3.21 GetNumEdges

*/
     inline int GetNumEdges() const;
/*
Returns the number of the edges from the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(1)$


7.3.22 GetStronglyConnectedComponents

*/
     vector<Graph*> GetStronglyConnectedComponents() const;
/*
Returns a vector with all strongly connected components of the graph.

*Precondition:* the graph is defined.

*Complexity:* $C(n * log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.1 GetShortestPath

*/
     Path* GetShortestPath(const int from, const int to) const;
     void GetShortestPath(int start, int target,
                            Path* solution) const;
/*



7.3.23 GetMappedGraph

*/
     Graph* GetMappedGraph(vector<int>& map) const;
/*
Returns a pointer to a new copy of the graph where the vertex keys are mapped to
0..n-1 - the mapping that was used is returned in the vector ~map~.

*Precondition:* the graph is defined.

*Complexity:* $C(n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.24 GetCircle

*/
     Graph* GetCircle(const int startKey,
                      const float maxCost ) const;
/*
Returns all vertices (and connecting edges) whose network distance from the vertex
with key ~startKey~ is not greater than ~maxCost~.
The operation fails and returns an empty graph if ~startKey~ does not exist.

*Precondition:* the graph is defined and maxCost $\ge$ 0

*Complexity:* $C(n^3 * log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)


7.3.25 Minimize

*/
     inline void Minimize();
/*
Minimizes the sizes of the DBArrays by deleting all free nodes.
This function should be called after the execution of delete operations.

*Precondition:* the graph is defined.

*Complexity:* $C(n * log n)$, where ~n~ is $|V|+|E|$ of the graph G=(V,E)

*/

/*
7.4 Functions needed to import the ~graph~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
     Graph* Clone() const;
     int Compare(const Attribute*) const;
     bool Adjacent(const Attribute*) const;
     size_t Sizeof() const;
     int NumOfFLOBs() const;
     Flob *GetFLOB(const int i);

     size_t HashValue() const;
     void CopyFrom(const Attribute* arg);

     static const string BasicType() { return "graph"; }
     static const bool checkType(const ListExpr type){
       return listutils::isSymbol(type, BasicType());
     }

   protected:
/*
7.5 Attributes

*/
     DbArray<AVLNode<verticesStruct> > vertices;
/*
The set of vertices the graph contains.

*/
     DbArray<AVLNode<adjStruct> > adjlist;
/*
The adjacent lists of the graph vertices. Each list element represents an edge.

*/
     int verticesRoot;
/*
The DBArray index in vertices that contains the root node.
verticeRoot must be -1 if the graph is empty.

*/
     int numVertices;
/*
The number of vertices in the graph.

*/
     int numEdges;
/*
The number of edges in the graph.

*/
/*
A flag that tells whether the graph is defined or not.

*/

};

template<class T>
ostream& operator <<( ostream& o, const AVLNode<T>& n ) {

   if (n.free)
     o << "undefined";
   else
     o << "Node " << n.key << "->" << n.elem 
       << " : Left[" << n.leftDepth << "]: " << n.left 
       << " / Right[" << n.rightDepth << "]: " << n.right 
       << " / Balance = " << n.Balance();
   return o;
}



template<class T>
inline string Print(DbArray<AVLNode<T> >& tree) {

   string s;
   const AVLNode<T>* node;
   
   s = "--------------------------------------\n";
   for (int i=0;i<tree.Size();i++) {
     tree.Get(i,node);
     s = s + (string)i + ". " + *node + "\n"; 
   }
   s += "--------------------------------------\n";
   
   return s;
}



template<class T>
inline int AVLNode<T>::Balance() const {
   
   return rightDepth-leftDepth;
}


template<class T>
AVLNode<T> AVLTree<T>::NewNode (const int key, const T elem) {
   
   AVLNode<T> n;
   n.key = key;
   n.elem = elem;
   n.left = -1;
   n.right = -1;
   n.leftDepth = 0;
   n.rightDepth = 0;
   n.free = false;
   return n;
}

template<class T>
AVLNode<T> AVLTree<T>::NewNode (const AVLNode<T>* n) {
   
   AVLNode<T> node;
   node.key = n->key;
   node.elem = n->elem;
   node.left = n->left;
   node.right = n->right;
   node.leftDepth = n->leftDepth;
   node.rightDepth = n->rightDepth;
   node.free = n->free;
   return node;
}


template<class T>
inline int AVLTree<T>::BalancedTree(DbArray<AVLNode<T> >& tree, 
                             const AVLNode<T>& node, const int root) {

   assert((0 <= root) && (root < tree.Size()));
   
   return (abs(node.Balance())>1) ? AVLTree<T>::Rebalance(tree,root) : root;
}


template<class T>
inline int AVLTree<T>::NumNodes(const DbArray<AVLNode<T> >& tree) {

   return tree.Size();
}


template<class T>
bool AVLTree<T>::DeleteNode(DbArray<AVLNode<T> >& tree, 
                      const int index, const AVLNode<T>* n) {
   
   assert((index >= 0) && (index < tree.Size()));
   
   AVLNode<T> root;
   
   // no node parameter?
   if (n==0) {
     AVLNode<T> node;
     tree.Get(index,node);
     root = AVLTree<T>::NewNode(&node);
   }
   else
     root = AVLTree<T>::NewNode(n);
   
   // node already deleted?
   if (root.free)
     return false;
     
   root.free = true;
   tree.Put(index,root);
   
   return true;
}


template<class T>
void AVLTree<T>::UpdateNode(DbArray<AVLNode<T> >& tree, 
                                const int index, const T& elem) {
  
   assert((index >= 0) && (index < tree.Size()));
   
   AVLNode<T> node;
   tree.Get(index,node);
   
   AVLNode<T> root = AVLTree<T>::NewNode(&node);
   root.elem = elem;
   tree.Put(index,root);
}


template<class T>
bool AVLTree<T>::HasKey(const DbArray<AVLNode<T> >& tree, 
                               const int key, const int index) {
   
   assert((index >= -1) && (index < tree.Size()));
   
   // key not found?
   if (index == -1)
     return false;
     
   AVLNode<T> node;
   tree.Get(index,node);
   
   // key found?
   if (key == node.key)
     return true;
   
   // key in left tree?
   if (key < node.key)
     // continue search in left tree
     return AVLTree<T>::HasKey(tree,key,node.left);
   
   // continue search in right tree
   return AVLTree<T>::HasKey(tree,key,node.right);
}


template<class T>
bool AVLTree<T>::UpdateKey(DbArray<AVLNode<T> >& tree, const int key, 
                                            const T elem, const int index) {

   assert((index >= -1) && (index < tree.Size()));
   
   // key not found?
   if (index == -1)
     return false;
   
   const AVLNode<T>* node;
   tree.Get(index,node);
   
   // key found?
   if (key == node->key) {
     
     // update node
     AVLNode<T> root = AVLTree<T>::NewNode(node);
     root.elem = elem;
     tree.Put(index,root);
     
     return true;
   }
   
   // key in left tree?
   if (key < node->key)
     // continue search in left tree
     return AVLTree<T>::UpdateKey(tree,key,elem,node->left);
   
   // continue search in right tree
   return AVLTree<T>::UpdateKey(tree,key,elem,node->right);
}


template<class T>
int AVLTree<T>::Rebalance(DbArray<AVLNode<T> >& tree, const int index) {
   
   assert((index >= 0) && (index < tree.Size()));
   
   AVLNode<T> bNode;
   AVLNode<T> son1;
   AVLNode<T> son2;
   
   tree.Get(index,bNode);
   AVLNode<T> rootNode = AVLTree<T>::NewNode(&bNode);
   
   // node really unbalanced ?
   assert(abs(bNode.Balance())==2);
   
   int root = index;
   int sonIndex;
   
   if (bNode.Balance()==2) {
     
     // too much nodes on the right hand side
     sonIndex = bNode.right;
     tree.Get(sonIndex,son1);
     AVLNode<T> rootSon = AVLTree<T>::NewNode(&son1);
     
     // too much nodes on the right hand side of the son?
     if (son1.Balance()==1) {
       
       // start rotation       
       rootNode.right = son1.left;
       rootNode.rightDepth = son1.leftDepth;
       rootSon.left = index;
       rootSon.leftDepth = max(bNode.leftDepth,son1.leftDepth)+1;
       root = sonIndex;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,rootSon);
     }
     else {
       
       // start double rotation
       tree.Get(son1.left,son2);
       AVLNode<T> newRoot = AVLTree<T>::NewNode(&son2);
       
       root = son1.left;
       newRoot.left = index;
       newRoot.leftDepth = max(bNode.leftDepth,son2.leftDepth)+1;
       newRoot.right = sonIndex;
       newRoot.rightDepth = max(son1.rightDepth,son2.rightDepth)+1;
       rootNode.right = son2.left;
       rootNode.rightDepth = son2.leftDepth;
       rootSon.left = son2.right;
       rootSon.leftDepth = son2.rightDepth;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,newRoot);
       tree.Put(sonIndex,rootSon);
     }
   }
   else {
   
     // too much nodes on the left hand side
     sonIndex = bNode.left;
     tree.Get(sonIndex,son1);
     AVLNode<T> rootSon = AVLTree<T>::NewNode(&son1);
     
     // too much nodes on the left hand side of the son?
     if (son1.Balance()==-1) {
       
       // start rotation       
       rootNode.left = son1.right;
       rootNode.leftDepth = son1.rightDepth;
       rootSon.right = index;
       rootSon.rightDepth = max(bNode.rightDepth,son1.rightDepth)+1;
       root = sonIndex;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,rootSon);
     }
     else {
       
       // start double rotation
       tree.Get(son1.right,son2);
       AVLNode<T> newRoot = AVLTree<T>::NewNode(&son2);
       
       root = son1.right;
       newRoot.right = index;
       newRoot.rightDepth = max(bNode.rightDepth,son2.rightDepth)+1;
       newRoot.left = sonIndex;
       newRoot.leftDepth = max(son1.leftDepth,son2.leftDepth)+1;
       rootNode.left = son2.right;
       rootNode.leftDepth = son2.rightDepth;
       rootSon.right = son2.left;
       rootSon.rightDepth = son2.leftDepth;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,newRoot);
       tree.Put(sonIndex,rootSon);
     }
   }
   return root;
}


template<class T>
int AVLTree<T>::InsertKey(DbArray<AVLNode<T> >& tree, 
                          const int key, const T elem, 
                          const int index) {
  
   assert((index >= -1) && (index < tree.Size()));
   
   AVLNode<T> node;
   AVLNode<T> son;
      
   // tree empty?
   if (index == -1) {
      tree.Append(AVLTree<T>::NewNode(key,elem));  
      return tree.Size()-1;
   }
   
   tree.Get(index,node);
      
   // key already in tree?
   if (key == node.key)
     return -1;
     
   AVLNode<T> thisNode = AVLTree<T>::NewNode(&node);
   int newSon;
   int newDepth;
   
   // continue in left son?
   if (key < node.key) {
     
     // no left son?
     if (node.left == -1) {
       
       // insert key as new left son
       tree.Append(AVLTree<T>::NewNode(key,elem));
       thisNode.left = tree.Size()-1;
       thisNode.leftDepth = 1;
       
       tree.Put(index,thisNode);
       return index;
     }
     else {
       
       // try to insert key in left tree
       newSon = AVLTree<T>::InsertKey(tree,key,elem,node.left);
       
       // insert failed?
       if (newSon == -1)
         return -1;
         
       // check left son
       tree.Get(newSon,son);
       newDepth = max(son.rightDepth,son.leftDepth)+1;
       
       // update required?  
       if ((newSon != thisNode.left) || (newDepth != thisNode.leftDepth)) {
         thisNode.leftDepth = newDepth;
         thisNode.left = newSon;
         tree.Put(index,thisNode);
         
         // Node not balanced?
         return AVLTree<T>::BalancedTree(tree,thisNode,index);
       }
       
       return index;
     }
   }
   
   // continue in right son
   else {
     
     // no right son?
     if (node.right == -1) {
       
       // insert key as new right son
       tree.Append(AVLTree<T>::NewNode(key,elem));
       thisNode.right = tree.Size()-1;
       thisNode.rightDepth = 1;
       
       tree.Put(index,thisNode);
       return index;
     }
     else {
       
       // try to insert key in right tree
       newSon = AVLTree<T>::InsertKey(tree,key,elem,node.right);
       
       // insert failed?
       if (newSon == -1)
         return -1;
         
       // check right son
       tree.Get(newSon,son);
       newDepth = max(son.rightDepth,son.leftDepth)+1;
       
       // update required?  
       if ((newSon != thisNode.right) || (newDepth != thisNode.rightDepth)) {
         thisNode.rightDepth = newDepth;
         thisNode.right = newSon;
         tree.Put(index,thisNode);
         
         // Node not balanced?
         return AVLTree<T>::BalancedTree(tree,thisNode,index);
       }
       
       return index;
     }
   
   }
 
}


template<class T>
int AVLTree<T>::DeleteMinKey (DbArray<AVLNode<T> >& tree, 
                             const int index, AVLNode<T>& minNode) {
   
   assert((index >= 0) && (index < tree.Size()));
   
   AVLNode<T> node;
   tree.Get(index,node);
   AVLNode<T> root = AVLTree<T>::NewNode(&node);
   
   // minimum in current node?
   if (node.left == -1) {
     
     // save and delete the minimum node
     minNode = root;
     AVLTree<T>::DeleteNode(tree,index,&root);
     
     return minNode.right;
   } 
   
   // continue in left son
   root.left = AVLTree<T>::DeleteMinKey(tree,node.left,minNode);
   
   // left son not deleted?
   if (root.left != -1) {
     
     tree.Get(root.left,node);
     root.leftDepth = max(node.leftDepth,node.rightDepth)+1;
   }
   else
     root.leftDepth = 0;
     
   // update current node
   tree.Put(index,root);
   
   return AVLTree<T>::BalancedTree(tree,root,index);   
}


template<class T>
int AVLTree<T>::DeleteKey (DbArray<AVLNode<T> >& tree, 
                                     const int key, const int index) {
   
   assert((index >= -1) && (index < tree.Size()));
   
   // tree empty?
   if (index == -1)
     return -2;
   
   AVLNode<T> node;
   AVLNode<T> son;
   int newSon;
   int newDepth;
   
   tree.Get(index,node);
   AVLNode<T> root = AVLTree<T>::NewNode(&node);
  
   // key not in current node?
   if (key != node.key) {
     
     // continue search in left tree?
     if ((key < node.key) && (node.left != -1)) {
       
       newSon = AVLTree<T>::DeleteKey(tree,key,node.left);
       
       // key not found?
       if (newSon == -2)
         return -2;
       
       // son not deleted?
       if (newSon != -1) {
         tree.Get(newSon,son);
         newDepth = max(son.leftDepth,son.rightDepth)+1;
       }
       else
         newDepth = 0;
         
       // update required?
       if ((newDepth != node.leftDepth) || (newSon != node.left)) {
       
         root.leftDepth = newDepth;
         root.left = newSon;
         
         tree.Put(index,root);  
          
         // node not balanced?
         return AVLTree<T>::BalancedTree(tree,root,index);
       }
       
       return index;
     }
     
     // continue search in right tree?
     if ((key > node.key) && (node.right != -1)) {
     
       newSon = AVLTree<T>::DeleteKey(tree,key,node.right);
       
       // key not found?
       if (newSon == -2)
         return -2;
         
       // son not deleted?
       if (newSon != -1) {
         tree.Get(newSon,son);
         newDepth = max(son.rightDepth,son.leftDepth)+1;
       }
       else
         newDepth = 0;
         
       // update required?
       if ((newDepth != node.rightDepth) || (newSon != node.right)) {
       
         root.rightDepth = newDepth;
         root.right = newSon;
         
         tree.Put(index,root);  
          
         // node not balanced?
         return AVLTree<T>::BalancedTree(tree,root,index);
       }
       
       return index;
     }
     
     // error: key not found
     return -2; 
   }
   
   // key found in current node
      
   // node is a leaf?
   if ((node.left == -1) && (node.right == -1)) {
   
      // delete leaf
      AVLTree<T>::DeleteNode(tree,index,&root);
      
      return -1;
   }
   
   // node has 2 sons?
   if ((node.left != -1) && (node.right != -1)) {
   
      AVLNode<T> min;
      tree.Get(node.right,son);
      
      root.right = AVLTree<T>::DeleteMinKey(tree,node.right,min);
      
      // right son not deleted by DeleteMinKey?
      if (root.right != -1) {
        tree.Get(root.right, son);
        root.rightDepth = max(son.leftDepth,son.rightDepth)+1;
      }
      else
        root.rightDepth = 0;
        
      root.key = min.key;
      root.elem = min.elem;
      tree.Put(index,root);      

      return index;
   }
   
   // node has only one son
   AVLTree<T>::DeleteNode(tree,index,&root);
   
   return ((node.left != -1) ? node.left : node.right);
}


template<class T>
void AVLTree<T>::DeleteKeys(DbArray<AVLNode<T> >& tree, const int index) {

   assert((index >= -1) && (index < tree.Size()));   

   // tree empty?
   if (index == -1)
     return;
     
   const AVLNode<T>* node;
   tree.Get(index,node);
   AVLNode<T> root = AVLTree<T>::NewNode(node);
   
   // nodes in left tree?
   if (node->left != -1)
     AVLTree<T>::DeleteKeys(tree,node->left);
     
   // nodes in right tree?
   if (node->right != -1)
     AVLTree<T>::DeleteKeys(tree,node->right);
   
   // delete current node
   AVLTree<T>::DeleteNode(tree,index,&root);
}


template<class T>
int AVLTree<T>::ReadKeys (const DbArray<AVLNode<T> >& tree,
                          vector<int>* v, const int index) {

   
   assert((index >= -1) && (index < tree.Size()));
   
   // tree empty?
   if (index == -1)
     return 0;
     
   AVLNode<T> node;
   tree.Get(index,node);
   
   int n = 0;
   if (node.left != -1)
     n += AVLTree<T>::ReadKeys(tree,v,node.left);
   v->push_back(node.key);
   n++;
   if (node.right != -1)
     n += AVLTree<T>::ReadKeys(tree,v,node.right);
     
   return n;
}


template<class T>
void AVLTree<T>::MapKeys (DbArray<AVLNode<T> >& tree, 
                             int& num, vector<int>& v, 
                             const int index) {
   
   assert((index >= -1) && (index < tree.Size()) && (num >= 0));
   
   // tree empty?
   if (index == -1)
     return;
     
   AVLNode<T> node;   
   tree.Get(index,node);
   
   if (node.left != -1)
     AVLTree<T>::MapKeys(tree,num,v,node.left);
   
   AVLNode<T> n = AVLTree<T>::NewNode(&node);
   v.push_back(n.key);
   n.key = num;
   tree.Put(index,n);
   
   num++;
   
   if (node.right != -1)
     AVLTree<T>::MapKeys(tree,num,v,node.right);
}


template<class T>
int AVLTree<T>::ReadOptNodes(const DbArray<AVLNode<T> >& tree, 
                             vector<AVLNode<T> >& v, const int root) {

   assert((root >= -1) && (root < tree.Size()));
   
   // tree empty?
   if (root == -1)
     return 0;
     
   AVLNode<T> node;
   vector<int> source(0);
   vector<int> target(0);
   
   target.push_back(root);
   
   int n = 0;
   
   // any nodes on next level?
   while (target.size()>0) {
     
     source = target;
     target.clear();
     
     // add all sons of source nodes to target
     for (unsigned int i = 0;i<source.size();i++) {
       
       tree.Get(source[i],node);
       v.push_back(AVLTree<T>::NewNode(&node));
       n++;
       
       // left son?
       if (node.left != -1)
         target.push_back(node.left);
       
       // right son?
       if (node.right != -1)
         target.push_back(node.right);
     }
   }
   return n;
}


template<class T>
int AVLTree<T>::ReadNodes (const DbArray<AVLNode<T> >& tree, 
                            vector<AVLNode<T> >& v, const int index) {
   
   assert((index >= -1) && (index < tree.Size()));
   
   // tree empty?
   if (index == -1)
     return 0;
     
   AVLNode<T> node;
   tree.Get(index,node);
   
   int n = 0;
   if (node.left != -1)
     n += AVLTree<T>::ReadNodes(tree,v,node.left);
   v.push_back(AVLTree<T>::NewNode(&node));
   n++;
   if (node.right != -1)
     n += AVLTree<T>::ReadNodes(tree,v,node.right);
     
   return n;
}


template<class T>
int AVLTree<T>::ReadNode(const DbArray<AVLNode<T> >& tree, AVLNode<T>& n, 
                         const int key, const int index) {

   assert((index >= -1) && (index < tree.Size()));
   
   // key not found?
   if (index == -1)
     return -1;
   
   AVLNode<T> node;
   tree.Get(index,node);
   
   // key found?
   if (key == node.key) {
      
      // copy node and return index
      n = node;
      return index;
   }
   
   // key in left tree?
   if (key < node.key)
     // continue search in left tree
     return AVLTree<T>::ReadNode(tree,n,key,node.left);
     
   // continue search in right tree
   return AVLTree<T>::ReadNode(tree,n,key,node.right);
}


template<class T>
int AVLTree<T>::ReplaceKey(DbArray<AVLNode<T> >& tree, 
                           const int key, const int newKey, const int root) {

   assert((root >= -1) && (root < tree.Size()));
   
   AVLNode<T> node;
   
   // key and newKey ok?
   if (AVLTree<T>::HasKey(tree,newKey,root) || 
       !AVLTree<T>::ReadNode(tree,node,key,root))
     return -2;
   
   int newRoot = AVLTree<T>::DeleteKey(tree,key,root);
   
   // delete failed?
   if (newRoot == -2)
     return -2;
     
   return AVLTree<T>::InsertKey(tree,newKey,node.elem,root);
}


/*
2  class EdgeDirection

this local class is needed to check for parallel edges in a path

*/


class EdgeDirection
{
public:
    explicit EdgeDirection(int nSource, int nTarget)
        : m_nSource(nSource), m_nTarget(nTarget){}

    int GetSource() const{return m_nSource;}
    int GetTarget() const{return m_nTarget;}

    bool operator<(EdgeDirection const & edge) const
    {
        return m_nSource < edge.GetSource() ||
            (m_nSource == edge.GetSource() && m_nTarget < edge.GetTarget());
    }

private:
    int m_nSource;
    int m_nTarget;
};


#endif /*GRAPHALGEBRA_H_*/
