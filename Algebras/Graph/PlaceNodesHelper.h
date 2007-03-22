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

[1] Header File of the graph algebra

[TOC]

1 Overview

This header file contains the definition of the classes used to implement the
placenodes operator in the Graph Algebra.

*/

#ifndef PLACENODESHELPER_H_
#define PLACENODESHELPER_H_

#include <set>
#include <map>
#include <list>
#include "SpatialAlgebra.h"

class Graph;

/* 
2 class NodePriority

This class defines the order, in which the nodes are placed

*/
class NodePriority
{
public:
    explicit NodePriority(int nNodeID, int nEdgeCount, double dDistance) 
        : m_nEdgeCount(nEdgeCount), m_nNodeID(nNodeID), 
        m_dDistance(dDistance){}
    
/*
2.1 Member functions

*/
    int GetNodeID() const {return m_nNodeID;}
    int GetEdgeCount() const {return m_nEdgeCount;}
    double GetDistance() const{return m_dDistance;}
    void SetDistance(double dDistance){m_dDistance = dDistance;}
    void IncEdgeCount(){++m_nEdgeCount;}
    
    bool operator<(NodePriority const & prio) const;
/*
This operator is needed to use this class the stl-classes set and map

*/

private:
/*
2.2 Attributes

*/
    int m_nEdgeCount;
/*
The number of edges leading to the vertex

*/

    int m_nNodeID;
/*
The ID of the verrtex

*/
    
    double m_dDistance;
/*
The distance from the startvertex

*/
};


/* 
3 class EdgeData

This class contains information about an edge

*/
class EdgeData
{
public:
    explicit EdgeData(int nNodeID, float fCost) 
        : m_nNodeID(nNodeID), m_fCost(fCost){}
    
/*
3.1 Member functions

*/
    int GetNodeID() const {return m_nNodeID;}
    float GetCost() const {return m_fCost;}
    
    bool operator<(EdgeData const & edge) const
    {
        return  m_nNodeID < edge.GetNodeID();
    }
/*
This operator is needed to use this class the stl-classes set and map

*/

private:
/*
3.2 Attributes

*/
    int m_nNodeID;
/*
The ID of the target vertex

*/
    
    float m_fCost;
/*
The cost of the edge

*/
};


/* 
4 class NodePriority

This class contains information about a vertex

*/
class NodeInfo
{
public:
    NodeInfo() : m_nOutDegree(0), m_pntPos(false), m_dDistance(0.0) {}
    
/*
4.1 Member functions

*/
    Point const & GetPos() const{return m_pntPos;}
    void SetPos(Point const & pnt){m_pntPos = pnt;}
    
    int GetOutDegree() const{return m_nOutDegree;}
    void SetOutDegree(int nOutDegree){m_nOutDegree = nOutDegree;}
    double GetDistance(){return m_dDistance;}
    void SetDistance(double dDistance){m_dDistance = dDistance;}
    
    std::set<EdgeData> const & GetEdgesOut() const{return m_setEdgesOut;}
    std::list<EdgeData> const & GetEdgesIn() const{return m_lstEdgesIn;}
    std::list<double> const & GetAnglesUsed() const{return m_lstAnglesUsed;}
    void InsertAngle(double dAngle){m_lstAnglesUsed.push_back(dAngle);}
    
    int GetEdgeCount() const
        {return m_setEdgesOut.size() + m_lstEdgesIn.size();}
/*
Returns the sum of the incoming and outgoing edges

*/
    
    void CalculateOutDegree(){m_nOutDegree = m_setEdgesOut.size();}
/*
Saves the number of outgoing edges in m\_nOutDegree

*/
    
    bool InsertEdge(EdgeData const & rEdge);
/*
Inserts an edge to the outgoing edges. Checks, whether the edge already exists
Returns true, if a new edge is inserted or an existing edge is changed

*/

    void PromoteEdge(EdgeData const & rEdge);
/*
Moves an edge from the outgoing edges to the incoming edges

*/
    
    bool CanUseAngle(double dAngle) const;
/*
Checks, whether this angle is already used

*/
    
private:
/*
4.2 Attributes

*/
    int m_nOutDegree;
/*
The number of edges to vertices that havn't been placed yet, when this vertex
is placed

*/

    Point m_pntPos;
/*
The position of the vertex

*/
    
    std::set<EdgeData> m_setEdgesOut;
/*
The edges to vertices that havn't been placed yet

*/

    std::list<EdgeData> m_lstEdgesIn;
/*
This edges to vertices that have already been placed

*/
    
    std::list<double> m_lstAnglesUsed;
/*
The angles between this vertex and connected vertices that have already been 
placed

*/

    double m_dDistance;
/*
This distance to the start vertex

*/
};

/* 
5 class PlaceNodesHelper

This class places the Vertices of a graph according to the distances of the 
edges.

*/
class PlaceNodesHelper
{
public:
    PlaceNodesHelper();
    
/*
5.1 Member functions

*/
    void PlaceNodes(Graph * pGraph);
/*
Places the Vertices of the graph pGraph

*/
    
private:
    void PlaceNode(int nId, Point const & pnt, NodeInfo& rInfo);
/*
Places the Vertex with the ID nId at the point pnt and updates the NodeInfo 
rInfo of the current Vertex and the NodeInfos of the connected vertices

*/

    void GetCandidates(Point const & pnt1, float fRadius1, Point const & pnt2, 
        float fRadius2, Point & pntOut1, Point & pntOut2) const;
/*
Calculates the intersection points of the circles formed by pnt1/fRadius1 and 
pnt2/fRadius2. If the circles don't intersect, it returns a point in the 
middle as pntOut1 and an undefined point as pntOut2.

*/
    
    void CalculatePos(NodeInfo const & rInfo, Point &pnt) const;
/*
Calculates the position where the vertex is placed. rInfo contains the 
information about the vertex to place

*/
    
    double GetAngle(Point const & p1, Point const & p2) const;
/*
Calculates the angle between the points p1 and p2 

*/

/*
5.2 Attributes

*/
    Graph* m_pGraph;
/*
The graph, whichs vertices are placed

*/

    Coord  m_coordX;
/*
X coordinate, which is right of all vertices placed so far. Used to place the
first node of a new component

*/
    
    std::set<NodePriority> m_setNodesToPlace;
/*
This set determines the order, in which the vertices are placed

*/
    
    std::map<int, NodeInfo> m_mapNodeInfo;
/*
This map contains the information about the vertices, which is needed to place 
them

*/
};

#endif
