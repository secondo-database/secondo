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

[1] Implementation of Graph

February 2007, G. Barz, M. Stein, A. Ruloffs, A. Martin


[TOC]

1 Overview

This file contains the implementation of the class ~Graph~ and all necesarry
functions and classes for the registration of the ~graph~ type constructor.  

2 Defines and includes

*/

#include "PlaceNodesHelper.h"
#include "GraphAlgebra.h"

#include "binTree.cpp"
#include "minTree.cpp"
#include <vector>

/*
3 Implementation of the ~Graph~ class

Read GraphAlgebra.h for more informations about the member functions.

*/

Graph::Graph() { }


Graph::Graph(const bool Defined) : vertices(0), adjlist(0), defined(Defined) {
   
   if (defined)
     Clear();
}


Graph::Graph(const Graph& g) : vertices(0), adjlist(0) {
   
   CopyFrom(&g);
}


Graph::~Graph() { }


inline void Graph::Clear() {
   
   vertices.Clear();
   adjlist.Clear();
   
   verticesRoot = -1;
   numVertices = 0;
   numEdges = 0;
}

bool Graph::EqualsWith(const Graph* other) const
{
        assert(defined && other->defined);    

    if(this->GetNumEdges()!=other->GetNumEdges())
        return false;
    if(this->GetNumVertices()!=other->GetNumVertices())
        return false;
    bool res=true;    
    vector<Vertex>* otherVertices= other->GetVertices();
    vector<Edge>* otherEdges=other->GetEdges();
    for (int i=0; i<this->GetNumVertices()&&res;i++)
    {
        if(!(this->HasVertex(otherVertices->at(i).GetKey())))
        {
            res=false;
        }
    }
    for (int i=0; i<this->GetNumEdges()&&res;i++)
    {
        if(!(this->HasEdge(otherEdges->at(i).GetSource(),
                           otherEdges->at(i).GetTarget())))
        {                    
            res=false;
        }
        else
        {
            Edge e =this->GetEdge(otherEdges->at(i).GetSource(),
                                  otherEdges->at(i).GetTarget());
            if(e.GetCost()!=otherEdges->at(i).GetCost())
            {
                res=false;
            }
        }
    }
    delete otherVertices;
    delete otherEdges;
    return res;
}

inline int Graph::GetNumVertices() const {

   assert(defined);
     
   return numVertices;
}


inline int Graph::GetNumEdges() const {
   
   assert(defined);
     
   return numEdges;
}


inline bool Graph::IsEmpty() const {
   
   assert(defined);
     
   return (numVertices == 0);
}


void Graph::Add(const Graph& g) {

   assert(defined && g.IsDefined());
  
   vector<Vertex>* v = g.GetVertices(true);
     
   // vector returned?
   if (v!=0) {
       
     // add all vertices
     for (unsigned int i=0;i<v->size();i++)
       Add((*v)[i]);
        
     delete v;
   }
     
   vector<Edge>* e = g.GetEdges(true);
     
   // vector returned?
   if (e!=0) { 
       
     // add all edges
     for (unsigned int i=0;i<e->size();i++)
       Add((*e)[i]);     
       
     delete e;
   }   
}


bool Graph::AddVertex (const int key, const Point &pos) {

   assert(defined);
   
   // key already exists?
   if (HasVertex(key))
     return false;
   
   verticesStruct v;
   v.pos = pos;
   v.succ = -1;
   v.inDeg = 0;
   v.outDeg = 0;
   
   int newRoot=AVLTree<verticesStruct>::InsertKey(vertices,
                                           key,v,verticesRoot);
   
   // could not insert key?
   if (newRoot == -1)
     return false;
     
   verticesRoot = newRoot;
   numVertices++;
   return true;
}


inline bool Graph::Add(const Vertex& v) {

   return AddVertex(v.GetKey(),v.GetPos());
}


inline bool Graph::Add(const Edge& e) {

   return AddEdge(e.GetSource(),e.GetTarget(),e.GetCost());
}


inline bool Graph::HasEdge(const int sourceKey, const int targetKey) const {

   assert(defined);
     
   AVLNode<verticesStruct> node;
   
   // sourceKey not found? 
   if (AVLTree<verticesStruct>::ReadNode(vertices,node,
                                         sourceKey,verticesRoot)<0)
     return false;
     
   return AVLTree<adjStruct>::HasKey(adjlist,targetKey,node.elem.succ);
}


inline bool Graph::HasVertex(const int key) const {

   assert(defined);
   
   return AVLTree<verticesStruct>::HasKey(vertices,key,verticesRoot);
}


vector<Vertex>* Graph::GetSuccFrom(const int key) const {
   
   assert(defined);
   
   AVLNode<verticesStruct> n;
   
   // key does not exist?
   if (AVLTree<verticesStruct>::ReadNode(vertices,n,key,verticesRoot)<0)
     return 0;

   vector<AVLNode<adjStruct> > a(0);
   AVLTree<adjStruct>::ReadNodes(adjlist,a,n.elem.succ);
   
   vector<Vertex>* result = new vector<Vertex>(0);
   const AVLNode<verticesStruct>* node;
   
   // create vertices from keys
   for (unsigned int i=0; i<a.size();i++) {
      vertices.Get(a[i].elem.keyInt,node);
      result->push_back(Vertex(node->key,node->elem.pos));
   }
   
   return result;
}


vector<int>* Graph::GetSuccKeysFrom(const int key) const {

   assert(defined);
   
   AVLNode<verticesStruct> node;
   
   // key does not exist?
   if (AVLTree<verticesStruct>::ReadNode(vertices,node,key,verticesRoot)<0)
     return false;

   vector<int>* result = new vector<int>(0);
   AVLTree<adjStruct>::ReadKeys(adjlist,result,node.elem.succ);
   
   return result;
}


vector<Edge>* Graph::GetSuccEdgesFrom(const int key) const {

   assert(defined);
   
   AVLNode<verticesStruct> n;
   
   // key does not exist?
   if (AVLTree<verticesStruct>::ReadNode(vertices,n,key,verticesRoot)<0)
     return 0;

   vector<AVLNode<adjStruct> > a(0);
   AVLTree<adjStruct>::ReadNodes(adjlist,a,n.elem.succ);
   
   vector<Edge>* result = new vector<Edge>(0);
   
   // create edges to successors
   for (unsigned int i=0; i<a.size();i++) 
      result->push_back(Edge(key,a[i].key,a[i].elem.cost));
   
   return result;
}
          
          
vector<Edge>* Graph::GetPredEdgesFrom(const int key) const {

   assert(defined);

   // key does not exist?
   if (!AVLTree<verticesStruct>::HasKey(vertices,key,verticesRoot))
     return 0;
   
   vector<Edge>* result = new vector<Edge>(0);
   AVLNode<adjStruct> node;
   vector<AVLNode<verticesStruct> > v(0);
   AVLTree<verticesStruct>::ReadNodes(vertices,v,verticesRoot);
   
   for (unsigned int i=0;i<v.size();i++)
     
     // edge with key as target?
     if (AVLTree<adjStruct>::ReadNode(adjlist,node,key,v[i].elem.succ)>=0)
       result->push_back(Edge(v[i].key,node.key,node.elem.cost));

   return result;   
}


vector<Vertex>* Graph::GetPredFrom(const int key) const {
   
   assert(defined);
   
   // key does not exist?
   if (!AVLTree<verticesStruct>::HasKey(vertices,key,verticesRoot))
     return 0;
   
   vector<Vertex>* result = new vector<Vertex>(0);
   vector<AVLNode<verticesStruct> > v(0);
   AVLTree<verticesStruct>::ReadNodes(vertices,v,verticesRoot);
   
   for (unsigned int i=0;i<v.size();i++)
     
     // edge with key as target?
     if (AVLTree<adjStruct>::HasKey(adjlist,key,v[i].elem.succ))
       result->push_back(Vertex(v[i].key,v[i].elem.pos));

   return result;
}


vector<int>* Graph::GetPredKeysFrom(const int key) const {

   assert(defined);
      
   // key does not exist?
   if (!AVLTree<verticesStruct>::HasKey(vertices,key,verticesRoot))
     return 0;
   
   vector<int>* result = new vector<int>(0);
   vector<AVLNode<verticesStruct> > v(0);
   AVLTree<verticesStruct>::ReadNodes(vertices,v,verticesRoot);
   
   for (unsigned int i=0;i<v.size();i++)
     
     // edge with key as target?
     if (AVLTree<adjStruct>::HasKey(adjlist,key,v[i].elem.succ))
       result->push_back(v[i].key);
   
   return result;
}


bool Graph::DeleteVertex (const int key) {
   
   assert(defined);
     
   AVLNode<verticesStruct> node;
   
   // key does not exist?
   if (AVLTree<verticesStruct>::ReadNode(vertices,node,key,verticesRoot)<0)
     return false;
   
   // any edges to delete?
   if ((node.elem.inDeg > 0) || (node.elem.outDeg > 0)) {
     
      // delete all ingoing edges
      vector<int>* v = GetPredKeysFrom(key);
      
      for (unsigned int i=0;i<v->size();i++) 
         DeleteEdge((*v)[i],key);
   
      delete v;
      
      // delete all outgoing edges
      v = GetSuccKeysFrom(key);
      
      for (unsigned int i=0;i<v->size();i++) 
         DeleteEdge(key,(*v)[i]);
         
      delete v;
            
   } 
     
   int newRoot = AVLTree<verticesStruct>::DeleteKey(vertices,
                                              key,verticesRoot);
   
   // delete failed?
   if (newRoot < -1) 
     return false;
   
   verticesRoot = newRoot;
   numVertices--;
   
   return true;
}


inline bool Graph::Delete(const Vertex& v) {

   return DeleteVertex(v.GetKey());
}


inline bool Graph::Delete(const Edge& e) {

   return DeleteEdge(e.GetSource(),e.GetTarget());
}

          
vector<Vertex>* Graph::GetVertices(const bool opt) const {
   
   assert(defined);
     
   vector<Vertex>* v = new vector<Vertex>(0);
   vector<AVLNode<verticesStruct> > n(0);
   
   // return vertices in optimized order?
   if (opt)
     AVLTree<verticesStruct>::ReadOptNodes(vertices,n,verticesRoot);
   else
     AVLTree<verticesStruct>::ReadNodes(vertices,n,verticesRoot);
   
   for (unsigned int i=0; i<n.size(); i++) 
     v->push_back(Vertex(n[i].key,n[i].elem.pos));
   
   
   return v;
}


vector<Edge>* Graph::GetEdges(const bool opt) const {
   
   assert(defined);
   
   vector<Edge>* v = new vector<Edge>(0);
   vector<AVLNode<verticesStruct> > n(0);
   vector<AVLNode<adjStruct> > adj(0);
   
   AVLTree<verticesStruct>::ReadNodes(vertices,n,verticesRoot);
   
   for (unsigned int i=0; i<n.size(); i++) {

      // succ tree not empty?
      if (n[i].elem.succ != -1) {
         
         adj.clear();
         
         // return edges in optimized order?
         if (opt)
           AVLTree<adjStruct>::ReadOptNodes(adjlist,adj,n[i].elem.succ);
         else
           AVLTree<adjStruct>::ReadNodes(adjlist,adj,n[i].elem.succ);
         
         for (unsigned int j=0; j<adj.size(); j++)
           v->push_back(Edge(n[i].key,adj[j].key,adj[j].elem.cost)); 
      }
   }
   return v;
   
}


inline int Graph::GetInDegFrom(const int key) const {

   assert(defined);
   
   AVLNode<verticesStruct> node;
   
   // key not found?
   if (AVLTree<verticesStruct>::ReadNode(vertices,node,key,verticesRoot)<0)
     return -1;
   
   return node.elem.inDeg;
}


inline int Graph::GetOutDegFrom(const int key) const {

   assert(defined);
   
   AVLNode<verticesStruct> node;
   
   // key not found?
   if (AVLTree<verticesStruct>::ReadNode(vertices,node,key,verticesRoot)<0)
     return -1;
   
   return node.elem.outDeg;
}


inline int Graph::GetMaxDeg(const bool out) const {

   assert(defined);
   
   vector<AVLNode<verticesStruct> > nodes;
   int n = AVLTree<verticesStruct>::ReadNodes(vertices,nodes,verticesRoot);
   int maxOut = 0;
   int maxIn = 0;
   
   for (int i=0;i<n;i++) {
     maxOut = max(maxOut,nodes[i].elem.outDeg);
     maxIn = max(maxIn,nodes[i].elem.inDeg);
   }
   
   return (out ? maxOut : maxIn);
}


inline int Graph::GetMinDeg(const bool out) const {

   assert(defined);
   
   vector<AVLNode<verticesStruct> > nodes;
   int n = AVLTree<verticesStruct>::ReadNodes(vertices,nodes,verticesRoot);
   int minOut = n;
   int minIn = n;
   
   for (int i=0;i<n;i++) {
     minOut = min(minOut,nodes[i].elem.outDeg);
     minIn = min(minIn,nodes[i].elem.inDeg);
   }
   
   return (out ? minOut : minIn);
}


inline bool Graph::SetCost(const int sourceKey, 
                           const int targetKey, 
                           const float cost) {

   assert(defined);
   
   AVLNode<verticesStruct> n;
   
   // sourceKey not found?
   if (AVLTree<verticesStruct>::ReadNode(vertices,n,sourceKey,verticesRoot)<0)
     return false;
     
   AVLNode<adjStruct> a;
   
   int index = AVLTree<adjStruct>::ReadNode(adjlist,a,targetKey,n.elem.succ);
   
   // targetKey not found?
   if (index < 0)
     return false;
   
   a.elem.cost = cost;
   AVLTree<adjStruct>::UpdateNode(adjlist,index,a.elem);
   
   return true;
}


bool Graph::SetPos(const int key, const Point& pos) {

   assert(defined);
   
   AVLNode<verticesStruct> node;
   
   int index = AVLTree<verticesStruct>::ReadNode(vertices,node,
                                                 key,verticesRoot);
   
   // key not found?
   if (index<0)
     return false;
   
   node.elem.pos = pos;
   AVLTree<verticesStruct>::UpdateNode(vertices,index,node.elem);
   
   return true;
}


inline bool Graph::Update(const Vertex& v) {

   return SetPos(v.GetKey(),v.GetPos());
}


inline bool Graph::Update(const Edge& e) {

   return SetCost(e.GetSource(),e.GetTarget(),e.GetCost());
}


inline Vertex Graph::GetVertex(const int key) const {

   assert(defined);
   
   AVLNode<verticesStruct> v;
   
   // key not found?
   if (AVLTree<verticesStruct>::ReadNode(vertices,v,key,verticesRoot)<0)
     return Vertex(false);
   
   return Vertex(v.key,v.elem.pos);
}

inline Edge Graph::GetEdge(const int sourceKey, const int targetKey) const {

   assert(defined);
   
   AVLNode<verticesStruct> v;
   
   // sourceKey not found?
   if (AVLTree<verticesStruct>::ReadNode(vertices,v,sourceKey,verticesRoot)<0)
     return Edge(false);
   
   AVLNode<adjStruct> e;
   
   // targetKey not found?
   if (AVLTree<adjStruct>::ReadNode(adjlist,e,targetKey,v.elem.succ)<0)
     return Edge(false);
        
   return Edge(v.key,e.key,e.elem.cost);
}


bool Graph::AddEdge (const int sourceKey, 
                     const int targetKey, 
                     const float cost) {

   assert(defined);
   
   AVLNode<verticesStruct> source;
   int srcIndex = AVLTree<verticesStruct>::ReadNode(vertices,source,
                                            sourceKey,verticesRoot);
   
   // source key unknown?
   if (srcIndex < 0)
     return false;
   
   AVLNode<verticesStruct> target;
   int tgtIndex = AVLTree<verticesStruct>::ReadNode(vertices,target,
                                               targetKey,verticesRoot);
   
   // target key unknown?
   if (tgtIndex < 0)
     return false;
   
   adjStruct a;
   a.keyInt = tgtIndex;
   a.cost = cost;
   
   int newRoot = AVLTree<adjStruct>::InsertKey(adjlist,targetKey,
                                               a,source.elem.succ);
   
   // insert failed?
   if (newRoot < 0)
     return false;
   
   // update source and target vertex
   source.elem.outDeg++;
   source.elem.succ = newRoot;
   
   // edge is a loop?
   if (sourceKey == targetKey)
     source.elem.inDeg++;
   
   AVLTree<verticesStruct>::UpdateNode(vertices,srcIndex,source.elem);
   
   // edge is no loop?
   if (sourceKey != targetKey) {
     target.elem.inDeg++;
     AVLTree<verticesStruct>::UpdateNode(vertices,tgtIndex,target.elem);
   }
   
   numEdges++;
   return true;
}


bool Graph::DeleteEdge (const int sourceKey, const int targetKey ) {

   assert(defined);

   AVLNode<verticesStruct> source;
   int srcIndex = AVLTree<verticesStruct>::ReadNode(vertices,source,
                                                sourceKey,verticesRoot);
   
   // source key unknown?
   if (srcIndex < 0)
     return false;
   
   AVLNode<verticesStruct> target;
   int tgtIndex = AVLTree<verticesStruct>::ReadNode(vertices,target,
                                                  targetKey,verticesRoot);
   
   // target key unknown?
   if (tgtIndex < 0)
     return false;
   
   int newRoot = AVLTree<adjStruct>::DeleteKey(adjlist,targetKey,
                                                       source.elem.succ);
   
   // delete failed?
   if (newRoot < -1)
     return false;
   
   // update source and target vertex
   source.elem.outDeg--;
   source.elem.succ = newRoot;
   
   // edge is a loop?
   if (sourceKey == targetKey)
     source.elem.inDeg--;
   
   AVLTree<verticesStruct>::UpdateNode(vertices,srcIndex,source.elem);
   
   // edge is no loop?
   if (sourceKey != targetKey) {
     target.elem.inDeg--;
     AVLTree<verticesStruct>::UpdateNode(vertices,tgtIndex,target.elem);
   }
   
   numEdges--;  
   return true;      
}


Graph* Graph::GetMappedGraph(vector<int>& map) const {

   assert(defined);
   
   Graph* g = new Graph(true);
   
   // copy vertices
   const AVLNode<verticesStruct>* n;
   int i;
   
   for (i=0;i<vertices.Size();i++) {
    
     vertices.Get(i,n);
     g->vertices.Append(*n);
   }
   g->verticesRoot = verticesRoot;
   g->numVertices = numVertices;
   g->numEdges = numEdges;
   
   // map vertex keys to 0,..,n-1
   int num = 0;
   AVLTree<verticesStruct>::MapKeys(g->vertices,num,map,verticesRoot);
   
   // map target keys to new keys
   const AVLNode<adjStruct>* e;
   
   for (i=0;i<adjlist.Size();i++) {
     
     adjlist.Get(i,e);
     g->vertices.Get(e->elem.keyInt,n);
     AVLNode<adjStruct> a = AVLTree<adjStruct>::NewNode(e);
     a.key = n->key;
     g->adjlist.Append(a);
   }
   
   return g;
}          


struct circleEnv {
   float maxDist;
   int startKey;
   Graph* graph;
   vector<Vertex>* vertices;
   vector<float> distance;
   vector<int> map;
   Graph* circle;
};


void GraphCircle(circleEnv& env, const int key, const float dist) {

   env.circle->AddVertex(env.map[key],(*env.vertices)[key].GetPos());
   env.distance[key] = dist;
   
   Path* p;
   vector<Edge>* succ = env.graph->GetSuccEdgesFrom(key);
   
   // examine all outgoing edges of the vertex
   for (unsigned int i=0;i<succ->size();i++) {
     
     Edge& e = (*succ)[i];
     float& targetDist = env.distance[e.GetTarget()];
     
     // distance from start vertex to target vertex unknown?
     if (targetDist<0.0f) {
        
        p = env.graph->GetShortestPath(env.startKey,e.GetTarget());
        assert(p->IsDefined());
        targetDist = p->GetCost();        
        delete p;
        
        // vertex outside circle?
        if (targetDist > env.maxDist)
          continue;
        
        // add vertex to circle and continue in target
        GraphCircle(env,e.GetTarget(),targetDist);
     }
     
     // edge part of circle?
     if ((dist+e.GetCost()) <= env.maxDist) 
         env.circle->AddEdge(env.map[e.GetSource()],
                             env.map[e.GetTarget()],e.GetCost());
   }
   
   delete succ;
}


Graph* Graph::GetCircle(const int startKey, const float maxDist) const {
   
   assert(defined && (maxDist >= 0.0f));
   
   circleEnv env;
   env.maxDist = maxDist;
   env.distance.clear();
   env.map.clear();
   env.circle = new Graph(true);
   env.graph = GetMappedGraph(env.map);
   env.vertices = env.graph->GetVertices();
   
   env.startKey = -1;
   
   // initialize distances and check if startKey exists
   for(int i=0;i<env.graph->GetNumVertices();i++) {
     
     env.distance.push_back(-1.0f);
     
     // startKey found?
     if (env.map[i] == startKey)
       env.startKey = i;
   }
   
   // startKey exists?
   if (env.startKey != -1)
     GraphCircle(env,env.startKey,0);
     
   delete env.graph;
   
   return env.circle; 
   
}


struct tarjanStruct {
   
   Point pos;
   int dfs;
   int lowlink;
   int sccIndex;
   bool inStack;
   bool visited;
};

struct tarjanEnv {

   int maxDfs;
   Graph* graph;
   vector<tarjanStruct> tVertices;
   stack<int> tStack;
   vector<int> map;
   vector<Graph*> SCC;
};

void Tarjan(const int index, tarjanEnv& env  ) {
   
   tarjanStruct& v = env.tVertices[index];
   
   v.dfs = env.maxDfs;
   v.lowlink = env.maxDfs;
   env.maxDfs++;
   env.tStack.push(index);
   v.inStack = true;
   v.visited = true;
   
   vector<int>* succ = env.graph->GetSuccKeysFrom(index);
   
   // check all successors
   for (unsigned int i=0;i<succ->size();i++) {
     
     tarjanStruct& vs = env.tVertices[(*succ)[i]];
     
     // successor is new vertex? 
     if (!vs.visited) {
     
       Tarjan((*succ)[i],env);
       v.lowlink = min(v.lowlink, vs.lowlink);
     }
     // stack contains successor?
     else if (vs.inStack)
       v.lowlink = min(v.lowlink, vs.dfs);
   }
   delete succ;
   
   // current vertex is the root of a SCC?   
   if (v.lowlink == v.dfs) {
      
      Graph* scc = new Graph(true);
      int i;
      
      // build SCC from all vertices on stack above the root vertex
      do {
        i = env.tStack.top();
        env.tStack.pop();
        env.tVertices[i].inStack = false;
        env.tVertices[i].sccIndex = env.SCC.size();
        scc->AddVertex(env.map[i],env.tVertices[i].pos);
        
      } while (i != index);
      
      env.SCC.push_back(scc);
   }   
}


vector<Graph*> Graph::GetStronglyConnectedComponents() const {

   assert(defined);
   
   tarjanEnv env;
   env.SCC.clear();
   env.map.clear();
   
   // work on the mapped version
   env.graph = GetMappedGraph(env.map);
   
   // add additional informations to the vertices
   
   env.tVertices.clear();
   vector<Vertex>* vt = GetVertices();
   
   for (unsigned int i=0;i<vt->size();i++) {
     
     tarjanStruct t;
     t.pos = (*vt)[i].GetPos();
     t.visited = false;
     t.dfs = 0;
     t.lowlink = 0;
     t.inStack = false;
     env.tVertices.push_back(t);
   }
   delete vt;
   
   // execute the Tarjan algorithm on all connected components
   
   env.maxDfs = 0;
   
   for (unsigned int i=0;i<env.tVertices.size();i++) 
     
     // vertex part of new connected component?
     if (!env.tVertices[i].visited) {
       
       Tarjan(i,env);
     }
   
   // add edges to the SCCs  
   
   vector<Edge>* ve = env.graph->GetEdges();
   
   int src,tgt;
   
   for (unsigned int i=0;i<ve->size();i++) {
    
     src = (*ve)[i].GetSource();
     tgt = (*ve)[i].GetTarget();
     
     // source and target of current edge in the same SCC?
     if (env.tVertices[src].sccIndex == env.tVertices[tgt].sccIndex)
       env.SCC[env.tVertices[src].sccIndex]->AddEdge(env.map[src],
                                        env.map[tgt],(*ve)[i].GetCost());
   }    
   
   // free memory
   delete ve;    
   delete env.graph;
   
   return env.SCC;
}

int FirstSourceEdge(vector<Edge>* graphEdges, int count, int key) {
    if (count == 0)
        return 0;
    int pos = count / 2;
    int diff = pos;

    while (diff > 0) {
        if ((*graphEdges) [pos].GetSource() == key) break;
        if ((*graphEdges) [pos].GetSource() > key)
            pos -= diff;
        else
            pos += diff;
        if (pos < 0)
            pos = 0;
        if (pos >= count)
            pos = count - 1;
        diff = (diff==1) ? 0 : (diff + 1) / 2;
    }
    while( pos > 0 && (*graphEdges) [pos - 1].GetSource() == key)
        pos--;
    return pos;
}

Path* Graph::GetShortestPath(int start, int target) const { 

    BinTree parts;
    MinTree edges;
    vector<pathStruct> wayPoint;
    vector<Edge>* graphEdges = GetEdges(); 
    vector<Vertex>* graphVertex = GetVertices(); 
    Path* solution = new Path(true);


    bool found = false;
    float foundWeight = -1.0f;
    Edge iEdge;
    Edge hitTarget;

    // if start and targetvertex are equal, ...
    if (start == target) {
        for (int i = 0; i < GetNumVertices(); i++)
            // return path of length 0 if start is an edge of the graph
            if ((*graphVertex) [i].GetKey() == start) {
                solution->SetDefined(true);
                pathStruct loneSome;
                loneSome.key = start;
                loneSome.pos = (*graphVertex) [i].GetPos();
                loneSome.cost = 0.0f;
                solution->Append(loneSome);
                delete graphEdges;
                delete graphVertex;
                return solution;
            }
        // if not in graph return an undefined path
        solution->SetDefined(false);
        delete graphEdges;
        delete graphVertex;
        return solution;
    } else { // search for all edges beginning at the start key
        for(int i = 0; i < GetNumEdges(); i++) { // EdgeSize of Graph !!!
            if ((*graphEdges) [i].GetSource() == start) { 
                   // found a connected edge // Of Graph !!!
                if ((*graphEdges) [i].GetTarget() == start) // reflective edge
                    continue;
                iEdge = (*graphEdges) [i];
                if ((parts.getCostOfEdge(iEdge.GetTarget()) == -1.0f) ||
                       // to-point first time reached ...
                    (parts.getCostOfEdge(iEdge.GetTarget()) 
                                 > iEdge.GetCost())) { // or with less weight
                    edges.put((Edge)iEdge);
                    parts.insertElem(iEdge);
                    if (iEdge.GetTarget() == target) {
                        found = true;
                        foundWeight = iEdge.GetCost();
                        hitTarget = iEdge;
                    }
                }
            }
        }
        bool searchOn = true;
        Edge actual(false);

        while(searchOn) { // After have searched from the start vertex,
                          // search from the further vertices on the way
            actual = (Edge) edges.pull();
            if (actual.IsDefined() == false) { 
                   // no more further edges to examine
                searchOn = false;
                break;
            }
            if (found && actual.GetCost() >= foundWeight) { 
                 // No Path may avaiable with lower weight
                searchOn = false;
                break;
            }
            if (parts.getCostOfEdge(actual.GetTarget()) != -1.0f) 
                  // if to-point not first time reached ...
                if (parts.getCostOfEdge(actual.GetTarget()) < 
                         actual.GetCost()) { // ... with less weight
                    continue; // then try next edge
                }

            int start = FirstSourceEdge(graphEdges, GetNumEdges(), 
                                        actual.GetTarget());
            for(int i = start; i < GetNumEdges(); i++) {
                if ((*graphEdges) [i].GetSource() != actual.GetTarget()) 
                    break;
                if ((*graphEdges) [i].GetSource() == 
                      (*graphEdges) [i].GetTarget()) // reflective edge
                    continue;
                if ((parts.getCostOfEdge((*graphEdges)[i].GetTarget())
                               ==-1.0f)|| // to-point first time reached ...
                        (parts.getCostOfEdge((*graphEdges) [i].GetTarget()) > 
                         actual.GetCost() + (*graphEdges) [i].GetCost())) {
                          // or with less weight
                    iEdge = (*graphEdges) [i];
                    iEdge.SetCost(actual.GetCost() + iEdge.GetCost());
                    edges.put(iEdge);
                    parts.insertElem(iEdge);
                    if (iEdge.GetTarget() == target) {
                        found = true;
                        if (foundWeight < 0 || (foundWeight >= 0 && 
                            foundWeight > iEdge.GetCost())) {
                            foundWeight = iEdge.GetCost();
                            hitTarget = iEdge;
                        }
                    }
                }
            }
        }
    }
    if (found) { // After examine the graph 
                 // gather backwards the path to the start vertex
        solution->SetDefined(true);
        pathStruct pstmp;

        pstmp.key = hitTarget.GetTarget();
        pstmp.pos = (GetVertex(hitTarget.GetTarget())).GetPos();
        pstmp.cost = hitTarget.GetCost();
        wayPoint.push_back(pstmp); // Push target vertex on vector

        iEdge = hitTarget;
        float costHelper = iEdge.GetCost();

        while (iEdge.GetSource() != start) {
            iEdge = (Edge) parts.getElemAT(iEdge.GetSource());

            pstmp.key = iEdge.GetTarget();
            pstmp.pos = (GetVertex(iEdge.GetTarget())).GetPos();
            pstmp.cost = costHelper;
            costHelper = iEdge.GetCost();
            pstmp.cost -= costHelper;
            wayPoint.push_back(pstmp); // Push vertex of the way on the vector

        }

        pstmp.key = iEdge.GetSource();
        pstmp.pos = (GetVertex(iEdge.GetSource())).GetPos();
        pstmp.cost = costHelper;
        wayPoint.push_back(pstmp); // Push start-vertex on the vector

        while(!wayPoint.empty()) {
            solution->Append(wayPoint.back());
            wayPoint.pop_back();
        }
    }
    else {
        solution->SetDefined(false);
    }
    delete graphEdges;
    delete graphVertex;
    return solution;
};

bool Graph::PartOf(const Graph* part) const {
    int debugLevel = 0;
    if (debugLevel >= 1) 
        cout << " BigVertices " << GetNumVertices() 
             << "  PartVertices " << part->GetNumVertices() << endl;
    if (GetNumVertices() < part->GetNumVertices())
        return false;
    if (debugLevel >= 1) 
        cout << " BigEdges " << GetNumEdges() 
             << "  PartEdges " << part->GetNumEdges() << endl;
    if (GetNumEdges() < part->GetNumEdges())
        return false;
    vector<Vertex>* bigVertex = GetVertices();
    vector<Vertex>* partVertex = part->GetVertices();
    if (debugLevel >= 1) cout << " Line 1120" << endl; 

    // O(nlogn) Implementation of vertices comparison
    bool found = true;
    for (int i = 0; i < part->GetNumVertices(); i++) {
        if (debugLevel >=2) 
             cout << " i " << i << " SearchKey " 
                  << (*partVertex) [i].GetKey() << endl;
        found = false;
        int searchPos = GetNumVertices() / 2;
        int delta = searchPos;
        bool goon = true;
        bool goon2 = true;
        while (goon2) {
            if (!goon) goon2 = false;
            if (delta == 1) goon = false;
            if (debugLevel >=2) 
               cout << " searchpos " << searchPos 
                    << " TestKey " 
                    << (*bigVertex) [searchPos].GetKey() << endl;
            if (searchPos >= 0 && searchPos < GetNumVertices())
                if ((*partVertex) [i].GetKey() == 
                    (*bigVertex)[searchPos].GetKey()) {
                    found = true;
                    break;
                }
            delta = (delta + 1) / 2;
            if ((*partVertex) [i].GetKey() < (*bigVertex)[searchPos].GetKey())
                searchPos -= delta;
            else
                searchPos += delta;
        }
    }
    delete bigVertex;
    delete partVertex;
    if (!found)
        return false;

    vector<Edge>* bigEdges = GetEdges();
    vector<Edge>* partEdges = part->GetEdges();
    if (debugLevel >= 1) cout << " Line 1137" << endl; 

    // O(nlogn) Implementation of edges comparison
    found = true;
    for (int i = 0; i < part->GetNumEdges(); i++) {
        found = false;
        if (debugLevel >= 2) 
            cout << " Search edge from " 
                 << (*partEdges)[i].GetSource() 
                 << "  to " 
                 << (*partEdges) [i].GetTarget() << endl;

        int searchPos = GetNumEdges() / 2;
        int delta = searchPos;
        bool goon = true;
        bool goon2 = true;
        while (goon2) {
            if (!goon) goon2 = false;
            if (delta == 1) goon = false;
            if (debugLevel >=2) 
                  cout << " searchpos " << searchPos 
                       << " TestKey " 
                       << (*bigEdges) [searchPos].GetSource() 
                       << endl;
            if (searchPos >= 0 && searchPos < GetNumEdges())
                if ((*partEdges) [i].GetSource() == 
                    (*bigEdges)[searchPos].GetSource() && 
                    (*partEdges) [i].GetTarget() == 
                    (*bigEdges)[searchPos].GetTarget() && 
                    (*partEdges) [i].GetCost() == 
                    (*bigEdges)[searchPos].GetCost() ) {
                    found = true;
                    break;
                }
            delta = (delta + 1) / 2;
            if ((*partEdges) [i].GetSource() < 
                (*bigEdges)[searchPos].GetSource() || 
                (*partEdges) [i].GetSource() == 
                (*bigEdges)[searchPos].GetSource() && 
                (*partEdges) [i].GetTarget() < 
                (*bigEdges)[searchPos].GetTarget() || 
                (*partEdges) [i].GetSource() == 
                (*bigEdges)[searchPos].GetSource() && 
                (*partEdges) [i].GetTarget()== 
                (*bigEdges)[searchPos].GetTarget() && 
                (*partEdges) [i].GetCost() < 
                (*bigEdges)[searchPos].GetCost())
                searchPos -= delta;
            else
                searchPos += delta;
        }
    }
    delete bigEdges;
    delete partEdges;
    if (!found)
        return false;

    return true;
}

inline void Graph::Minimize() {
   
   assert(defined);
   
   // empty space in DBArrays?
   if ((vertices.Size() > numVertices) || (adjlist.Size() > numEdges)) {
   
      Graph g(*this);
     
      Clear();
      
      Add(g);
   }  
}


void Graph::SetDefined(bool def) {
   
   defined=def;
}


bool Graph::IsDefined() const {
   
   return defined;
}


Graph* Graph::Clone() const { 
   
   return new Graph(*this); 
}


size_t Graph::Sizeof() const {
   
   return sizeof(Graph);
}


int Graph::Compare(const Attribute*) const {
   
   return 0;
}


bool Graph::Adjacent(const Attribute*) const {
   
   return 0;
}


size_t Graph::HashValue() const {
   
   return 0;
}


void Graph::CopyFrom(const StandardAttribute* arg) {
   
   const Graph* g = (const Graph*)arg;
   
   vertices.Clear();
   const AVLNode<verticesStruct>* v;
   for (int i=0;i<g->vertices.Size();i++) {
     g->vertices.Get(i,v);
     vertices.Append(*v);
   }
   
   adjlist.Clear();
   const AVLNode<adjStruct>* a;
   for (int i=0;i<g->adjlist.Size();i++) {
     g->adjlist.Get(i,a);
     adjlist.Append(*a);
   }
      
   defined = g->defined;
   numVertices = g->numVertices;
   numEdges = g->numEdges;
   verticesRoot = g->verticesRoot;
}


int Graph::NumOfFLOBs() const {

   return 2;
}


FLOB* Graph::GetFLOB(const int i) {
  
   assert( i >= 0 && i < NumOfFLOBs() );
   
   if (i==1)
     return &adjlist;
   
   return &vertices;
}


inline void Graph::Destroy() {
   
   vertices.Destroy();
   adjlist.Destroy();
}


/*
4 Implementation of the ~graph~ type constructor

*/



/*
4.1 List Representation

The list representation of a graph is

----  ((vertex*) (edge*))
----

conditions: vertex keys are unique, no parallel edges, source and target 
keys of the edges exist in the vertex list

4.2 ~Out~-function

*/
ListExpr OutGraph( ListExpr typeInfo, Word value ) {  
   

   Graph* graph = (Graph*)(value.addr);

   if( !graph->IsDefined() )
     return nl->SymbolAtom("undef");    
     
   // create ListExpr for vertices
   
   vector<Vertex>* v = graph->GetVertices(true);
   ListExpr last;
   ListExpr verticesList;
   Point p;
   
   if ((*v).size() > 0) {
     p = (*v)[0].GetPos();
     verticesList = 
       nl->OneElemList(
           nl->TwoElemList(
               nl->IntAtom((*v)[0].GetKey()),
               OutPoint( nl->TheEmptyList(), SetWord( (void*)(&p)))
           )
       );  
   
    last = verticesList;
    for (unsigned int i=1;i<(*v).size();i++) {
       p = (*v)[i].GetPos();
       last =
         nl->Append(
             last,
             nl->TwoElemList(
                 nl->IntAtom((*v)[i].GetKey()),
                 OutPoint( nl->TheEmptyList(), SetWord( (void*)(&p)))
            )
         );
     }
   }
   else
     verticesList = nl->TheEmptyList();
     
   // create ListExpr for edges
   
   vector<Edge>* e = graph->GetEdges(true);
   ListExpr edgesList;
   
   if ((*e).size() > 0) {
     
     edgesList = 
       nl->OneElemList(
           nl->ThreeElemList(
               nl->IntAtom((*e)[0].GetSource()),
               nl->IntAtom((*e)[0].GetTarget()),
               nl->RealAtom((*e)[0].GetCost())
           )
       );  
   
     last = edgesList;
     for (unsigned int i=1;i<(*e).size();i++) {
       last =
         nl->Append(
             last,
             nl->ThreeElemList(
               nl->IntAtom((*e)[i].GetSource()),
               nl->IntAtom((*e)[i].GetTarget()),
               nl->RealAtom((*e)[i].GetCost())
             )
         );
     }
   }
   else
     edgesList = nl->TheEmptyList();
     
   return (nl->TwoElemList(verticesList,edgesList));
}

/*
4.3 ~In~-function

*/
Word InGraph( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct ) {
   
   Graph* graph;
   graph = new Graph(true);
   
   correct = false;
   
   if (nl->ListLength(instance) == 2) {
     
     ListExpr verticesList = nl->First(instance);
     ListExpr edgesList = nl->Second(instance);
     
     if (!(nl->IsAtom(verticesList) || nl->IsAtom(edgesList))) {
       
       correct = true;
       Point* p;
       ListExpr first, second, third;
       ListExpr firstElem = nl->Empty();
       ListExpr rest = verticesList;
       
       // parse values of vertices
       while (correct && !nl->IsEmpty(rest)) {
         firstElem = nl->First(rest);
         rest = nl->Rest(rest);
         
         if (nl->ListLength(firstElem) != 2)
           correct = false;
         else {
           first = nl->First(firstElem);
           second = nl->Second(firstElem);
           
           if (!(nl->IsAtom(first) && (nl->AtomType(first) == IntType)))
             correct = false;
           else {
             p = (Point*)InPoint( nl->TheEmptyList(), 
                                  second, 0, errorInfo, correct ).addr;
             if (correct) {
               correct = graph->AddVertex(nl->IntValue(first),*p);
             }
           }
         }       
       }
       
       // parse values of edges
       firstElem = nl->Empty();
       rest = edgesList;
       
       while (correct && !nl->IsEmpty(rest)) {
         firstElem = nl->First(rest);
         rest = nl->Rest(rest);
         
         if (nl->ListLength(firstElem) != 3)
           correct = false;
         else {
           first = nl->First(firstElem);
           second = nl->Second(firstElem);
           third = nl->Third(firstElem);
           
           if (!(nl->IsAtom(first) && (nl->AtomType(first) == IntType) 
                && nl->IsAtom(second) && (nl->AtomType(second) == IntType) 
                && nl->IsAtom(third) && (nl->AtomType(third) == RealType) && 
                   (nl->RealValue(third) >= 0)))
             correct = false;
           else 
             correct = graph->AddEdge(nl->IntValue(first), 
                                      nl->IntValue(second), 
                                      nl->RealValue(third));
         }
       }
       
       if (!correct)
         cout << "Graph is invalid!" << endl;
     }
       
   }
   else if (nl->IsEqual(instance,"undef")) {
     
     graph->SetDefined(false);
     correct = true;
   }
   
   if (correct) 
     return SetWord(graph);
   
   delete graph;
   return SetWord(Address(0));
}

/*
4.5 ~Create~-function

*/
Word CreateGraph( const ListExpr typeInfo ) {

    return SetWord(new Graph(true));
}


/*
4.6 ~Delete~-function

*/
void DeleteGraph( const ListExpr typeInfo, Word& w ) {
   
   Graph* graph = (Graph*)w.addr;

   graph->Destroy();
   graph->DeleteIfAllowed();
   w.addr = 0;
}

/*
4.7 ~Close~-function

*/
void CloseGraph( const ListExpr typeInfo, Word& w ) {
   
  ((Graph*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.8 ~Clone~-function

*/
Word CloneGraph( const ListExpr typeInfo, const Word& w ) {
   
   return SetWord( ((Graph*)w.addr)->Clone() );
}

/*
4.9 ~SizeOf~-function

*/
int SizeOfGraph() {
   
   return sizeof(Graph);
}
/*
4.10 Function describing the signature of the type constructor

*/
ListExpr GraphProperty() {

   return (
     nl->TwoElemList(
         nl->FiveElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List"),
             nl->StringAtom("Remarks")
             ),
         nl->FiveElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom("graph"),
             nl->TextAtom("((<vertex>*) (<edge>*))"),
             nl->TextAtom("(((2 (2.0 -3.5)) (5 (-1.2 5.0)))"
                          " ((2 2 4.2) (5 2 0.2)))"),
             nl->TextAtom("The vertices must have unique keys, the edges use"
                          " source and target keys from the vertex list, the"
                          " edge costs must be positive and "
                          "there are no parallel edges.")
             )
         )
     );
}

/*
4.11 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~graph~ does not have arguments, this is trivial.

*/

bool CheckGraph( ListExpr type, ListExpr& errorInfo ) {
   
   return (nl->IsEqual(type,"graph"));
}
/*
4.12 ~Cast~-function

*/
void* CastGraph (void* addr) {
   
   return (new (addr) Graph);
}

/*
4.13 Creation of the type constructor instance

*/
TypeConstructor graphCon(
    "graph",              //name
    GraphProperty,               //property function describing signature
    OutGraph, InGraph,            //Out and In functions
    0, 0,                      //SaveToList and RestoreFromList functions
    CreateGraph, DeleteGraph,      //object creation and deletion
    0, 0, CloseGraph, CloneGraph, //object open, save, close, and clone
    CastGraph,              //cast function
    SizeOfGraph,           //sizeof function
    CheckGraph                    //kind checking function
);
