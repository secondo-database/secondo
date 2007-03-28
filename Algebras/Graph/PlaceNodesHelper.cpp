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
//[_] [\_]

[1] Implementation of the placenodes operator

[TOC]

1 Overview

This implementation file contains the classes needed to implement the 
placenodes operator

*/

#include <cmath>
#include "GraphAlgebra.h"
#include "PlaceNodesHelper.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif




/*
2 consts

Try to place the nodes in a way, that the angle between two edges is larger 
than c[_]dMinAngleDiff (about 5 Degrees)

*/
double const c_dMinAngleDiff = 0.1;

/*
If too many Edges leave the node, it can't be avoided that the angles are 
very small, so don't check it

*/
int const c_nMaxCheckCount = 7;


/*
3 class NodePriority

*/

bool NodePriority::operator<(NodePriority const & prio) const
{
    bool bRet = false;
    
    if (m_nEdgeCount > prio.GetEdgeCount())
    {
        bRet = true;
    }
    else if (m_nEdgeCount == prio.GetEdgeCount())
    {
        if (m_dDistance < prio.GetDistance())
        {
            bRet = true;
        }
        else
        {
            bRet = m_dDistance == prio.GetDistance() && 
                m_nNodeID < prio.GetNodeID();
        }
    }
    
    return bRet;
}


/*
4 class NodeInfo

*/

bool NodeInfo::InsertEdge(EdgeData const & rEdge)
{
    bool bRet = false;
    set<EdgeData>::const_iterator it = m_setEdgesOut.find(rEdge);
    if (it == m_setEdgesOut.end())
    {
        m_setEdgesOut.insert(rEdge);
        bRet = true;
    }
    else if (rEdge.GetCost() <  (*it).GetCost())
    {
        m_setEdgesOut.erase(it);
        m_setEdgesOut.insert(rEdge);
        bRet = true;
    }
    return bRet;
}


void NodeInfo::PromoteEdge(EdgeData const & rEdge)
{
    m_setEdgesOut.erase(rEdge);
    m_lstEdgesIn.push_back(rEdge);
}

bool NodeInfo::CanUseAngle(double dAngle) const
{
    bool bRet = true;
    list<double>::const_iterator itEnd = m_lstAnglesUsed.end();
    if (m_nOutDegree > c_nMaxCheckCount)
    {
        //If more than c_nMaxCheckCount Edges leave the Node just check for 
        //exact angle
        for (list<double>::const_iterator it = m_lstAnglesUsed.begin(); 
            it != itEnd; ++it)
        {
            if (dAngle == (*it))
            {
                bRet = false;
                break;
            }
        }
    }
    else
    {
        for (list<double>::const_iterator it = m_lstAnglesUsed.begin(); 
            it != itEnd; ++it)
        {
            if (fabs(dAngle - (*it)) < c_dMinAngleDiff || 
                2 * M_PI - fabs(dAngle - (*it)) < c_dMinAngleDiff)
            {
                bRet = false;
                break;
            }
        }
    }
    return bRet;
}


/*
5 class PlaceNodesHelper

*/

PlaceNodesHelper::PlaceNodesHelper() :
    m_pGraph(NULL), m_coordX(0.0)
{
    
}


void PlaceNodesHelper::PlaceNodes(Graph* pGraph)
{
    m_pGraph = pGraph;
    if (m_pGraph->IsDefined())
    {
        vector<Vertex>* pVertices = m_pGraph->GetVertices(false);
        if (pVertices != NULL)
        {
            vector<Vertex>::const_iterator itEnd = pVertices->end();
            for (vector<Vertex>::const_iterator it = pVertices->begin(); 
                it != itEnd; ++it)
            {
                int nKey = (*it).GetKey();
                NodePriority prio(nKey, 0, 0.0);
                m_setNodesToPlace.insert(prio);
                NodeInfo info;
                m_mapNodeInfo.insert(pair<int, NodeInfo>(nKey, info));
            }
            delete pVertices;
            pVertices = NULL;
        }
        
        vector<Edge>* pEdges = m_pGraph->GetEdges(false);
        if (pEdges != NULL)
        {
            vector<Edge>::const_iterator itEnd = pEdges->end();
            for (vector<Edge>::const_iterator it = pEdges->begin(); 
                it != itEnd; ++it)
            {
                Edge const & rEdge = *it;
                if (rEdge.GetSource() != rEdge.GetTarget())
                {
                    NodeInfo &rInfoSource = 
                        (*(m_mapNodeInfo.find(rEdge.GetSource()))).second;
                    if (rInfoSource.InsertEdge(EdgeData(
                        rEdge.GetTarget(), rEdge.GetCost())))
                    {
                        NodeInfo &rInfoTarget = 
                            (*(m_mapNodeInfo.find(rEdge.GetTarget()))).second;
                        rInfoTarget.InsertEdge(EdgeData(
                            rEdge.GetSource(), rEdge.GetCost()));
                    }
                }
            }
            delete pEdges;
            pEdges = NULL;
        }
        
        while (!m_setNodesToPlace.empty())
        {
            set<NodePriority>::const_iterator it = m_setNodesToPlace.begin();
            NodePriority const & prio = *it;
            int nNodeId = prio.GetNodeID();
            m_setNodesToPlace.erase(it);
            
            NodeInfo &rInfo =(*(m_mapNodeInfo.find(nNodeId))).second;
            Point pnt(false);
            CalculatePos(rInfo, pnt);
            PlaceNode(nNodeId, pnt, rInfo);
        }
    }
}


void PlaceNodesHelper::PlaceNode(int nId, Point const & pnt, NodeInfo& rInfo)
{
    m_pGraph->SetPos(nId, pnt);
    double dNextX = ceil(pnt.GetX() + 1.0);
    if (dNextX > m_coordX)
    {
        m_coordX = dNextX;
    }
    rInfo.SetPos(pnt);
    rInfo.CalculateOutDegree();
    
    list<EdgeData>::const_iterator itEndIn = rInfo.GetEdgesIn().end();
    for (list<EdgeData>::const_iterator it = rInfo.GetEdgesIn().begin(); 
        it != itEndIn; ++it)
    {
        NodeInfo& rInfoSourceNode = 
            (*m_mapNodeInfo.find((*it).GetNodeID())).second;
        rInfoSourceNode.InsertAngle(GetAngle(rInfoSourceNode.GetPos(), pnt));
    }
    
    set<EdgeData>::const_iterator itEndOut = rInfo.GetEdgesOut().end();
    for (set<EdgeData>::const_iterator it = rInfo.GetEdgesOut().begin(); 
        it != itEndOut; ++it)
    {
        EdgeData const & rEdge = *it;
        int nTargetNodeID = rEdge.GetNodeID();
        NodeInfo& rInfoTargetNode = 
            (*m_mapNodeInfo.find(nTargetNodeID)).second;
        NodePriority prio(nTargetNodeID, rInfoTargetNode.GetEdgesIn().size(), 
            rInfoTargetNode.GetDistance());
        if (m_setNodesToPlace.erase(prio) > 0)
        {
            prio.IncEdgeCount();
            if (prio.GetEdgeCount() == 1)
            {
                //Set Distance
                double dDistance = rInfo.GetDistance() + rEdge.GetCost();
                prio.SetDistance(dDistance);
                rInfoTargetNode.SetDistance(dDistance);
            }
            m_setNodesToPlace.insert(prio);
        }
        EdgeData edge(nId, rEdge.GetCost());
        rInfoTargetNode.PromoteEdge(edge);
    }
}


void PlaceNodesHelper::CalculatePos(NodeInfo const & rInfo, Point& pnt) const
{
    switch(rInfo.GetEdgesIn().size())
    {
        case 0:
            pnt.Set(m_coordX, 0.0);
        break;
        case 1:
        {
            EdgeData const & rEdge = *(rInfo.GetEdgesIn().begin());
            NodeInfo const & rInfoSource = 
                (*m_mapNodeInfo.find(rEdge.GetNodeID())).second;
            int nDiv = rInfoSource.GetOutDegree() - 1;
            if (nDiv == 0)
            {
                nDiv += rInfoSource.GetEdgeCount() - 1;
                if (nDiv == 0)
                {
                    nDiv = 1;
                }
            }
            double dStep = M_PI_2 / nDiv;
            double dAngle = 0.0;
            while (!rInfoSource.CanUseAngle(dAngle))
            {
                dAngle += dStep;
            }
            pnt.Set(
                rInfoSource.GetPos().GetX() + rEdge.GetCost() * cos(dAngle), 
                rInfoSource.GetPos().GetY() + rEdge.GetCost() * sin(dAngle));
        }
        break;
        case 2:
        {
            list<EdgeData>::const_iterator itEdgeData = 
                rInfo.GetEdgesIn().begin();
            EdgeData const & rEdge1 = *itEdgeData;
            ++itEdgeData;
            EdgeData const & rEdge2 = *itEdgeData;
            NodeInfo const & rInfoSource1 = 
                (*m_mapNodeInfo.find(rEdge1.GetNodeID())).second;
            NodeInfo const & rInfoSource2= 
                (*m_mapNodeInfo.find(rEdge2.GetNodeID())).second;
            
            Point p1(false);
            Point p2(false);
            GetCandidates(rInfoSource1.GetPos(), rEdge1.GetCost(), 
                rInfoSource2.GetPos(), rEdge2.GetCost(), p1, p2);
            if (p2.IsDefined())
            {
                int nUsedAngles1 = 0;
                double dAngle = GetAngle(rInfoSource1.GetPos(), p1);
                if (!rInfoSource1.CanUseAngle(dAngle))
                {
                    ++nUsedAngles1;
                }
                dAngle = GetAngle(rInfoSource2.GetPos(), p1);
                if (!rInfoSource2.CanUseAngle(dAngle))
                {
                    ++nUsedAngles1;
                }
                if (nUsedAngles1 == 0)
                {
                    pnt = p1;
                }
                else
                {
                    int nUsedAngles2 = 0;
                    dAngle = GetAngle(rInfoSource1.GetPos(), p2);
                    if (!rInfoSource1.CanUseAngle(dAngle))
                    {
                        ++nUsedAngles2;
                    }
                    dAngle = GetAngle(rInfoSource2.GetPos(), p2);
                    if (!rInfoSource2.CanUseAngle(dAngle))
                    {
                        ++nUsedAngles2;
                    }
                    if (nUsedAngles1 <= nUsedAngles2)
                    {
                        pnt = p1;
                    }
                    else
                    {
                        pnt = p2;
                    }
                }
            }
            else
            {
                pnt = p1;
            }
        }
        break;
        default:
        {
            list<EdgeData>::const_iterator itEdgeData = 
                rInfo.GetEdgesIn().begin();
            EdgeData const & rEdge1 = *itEdgeData;
            ++itEdgeData;
            EdgeData const & rEdge2 = *itEdgeData;
            NodeInfo const & rInfoSource1 = 
                (*m_mapNodeInfo.find(rEdge1.GetNodeID())).second;
            NodeInfo const & rInfoSource2 = 
                (*m_mapNodeInfo.find(rEdge2.GetNodeID())).second;
            
            Point p1(false);
            Point p2(false);
            GetCandidates(rInfoSource1.GetPos(), rEdge1.GetCost(), 
                rInfoSource2.GetPos(), rEdge2.GetCost(), p1, p2);
            if (p2.IsDefined())
            {
                ++itEdgeData;
                EdgeData const & rEdge3 = *itEdgeData;
                NodeInfo const & rInfoSource3 = 
                    (*m_mapNodeInfo.find(rEdge3.GetNodeID())).second;
                
                Point p3(false);
                Point p4(false);
                GetCandidates(rInfoSource1.GetPos(), rEdge1.GetCost(), 
                    rInfoSource3.GetPos(), rEdge3.GetCost(), p3, p4);
                if (p4.IsDefined())
                {
                    double dDistance1 = p1.Distance(p3);
                    double dDistance2 = p1.Distance(p4);
                    double dDistance3 = p2.Distance(p3);
                    double dDistance4 = p2.Distance(p4);
                    double dMinDistance = min(min(dDistance1, dDistance2), 
                        min(dDistance3, dDistance4));
                    if (dMinDistance == dDistance1 || 
                        dMinDistance == dDistance2)
                    {
                        pnt = p1;
                    }
                    else
                    {
                        pnt = p2;
                    }
                }
                else
                {
                    pnt = p1.Distance(p3) <= p2.Distance(p3) ? p1 : p2;
                }

            }
            else
            {
                pnt = p1;
            }
        }
        break;
    }
}


void PlaceNodesHelper::GetCandidates(Point const & pnt1, float fRadius1, 
    Point const & pnt2, float fRadius2, Point & pntOut1, Point & pntOut2) const
{
    double dDistance = pnt1.Distance(pnt2);
    if (dDistance == 0.0)
    {
        double dRadius = (fRadius1 + fRadius2) / 2.0;
        pntOut1.Set(pnt1.GetX() + dRadius, pnt1.GetY());
        pntOut2.Set(pnt1.GetX(), pnt1.GetY() + dRadius);
    }
    else if (fRadius1 + fRadius2 == 0.0)
    {
        pntOut2.Set((pnt1.GetX() + pnt2.GetX()) / 2.0, 
            (pnt1.GetY() + pnt2.GetY()) / 2.0);
    }
    else if (dDistance >= fRadius1 + fRadius2)
    {
        double dRatio = fRadius1 / (fRadius1 + fRadius2);
        double x = pnt1.GetX() + (pnt2.GetX() - pnt1.GetX()) * dRatio;
        double y = pnt1.GetY() + (pnt2.GetY() - pnt1.GetY()) * dRatio;
        pntOut1.Set(x, y);
        pntOut2.SetDefined(false); 
    }
    else if (fRadius1 >= dDistance + fRadius2)
    {
        double dRatio = (fRadius1 + fRadius2 + dDistance)  / (2 * dDistance);
        double x = pnt1.GetX() + (pnt2.GetX() - pnt1.GetX()) * dRatio;
        double y = pnt1.GetY() + (pnt2.GetY() - pnt1.GetY()) * dRatio;
        pntOut1.Set(x, y);
        pntOut2.SetDefined(false);
    }
    else if (fRadius2 >= dDistance + fRadius1)
    {
        double dRatio = (fRadius1 + fRadius2 + dDistance)  / (2 * dDistance);
        double x = pnt2.GetX() + (pnt1.GetX() - pnt2.GetX()) * dRatio;
        double y = pnt2.GetY() + (pnt1.GetY() - pnt2.GetY()) * dRatio;
        pntOut1.Set(x, y);
        pntOut2.SetDefined(false);
    }
    else
    {
        double dRadius1_2 = fRadius1 * fRadius1;
        double dBaseLen = (dDistance * dDistance + dRadius1_2 - 
            fRadius2 * fRadius2) / (2.0 * dDistance);
        double dHeight = sqrt(dRadius1_2 - dBaseLen * dBaseLen);
        double dDeltaX = (pnt2.GetX() - pnt1.GetX()) / dDistance;
        double dDeltaY = (pnt2.GetY() - pnt1.GetY()) / dDistance;
        double dBaseX = pnt1.GetX() + dDeltaX * dBaseLen;
        double dBaseY = pnt1.GetY() + dDeltaY * dBaseLen;
        double dX1 = dBaseX - dDeltaY * dHeight;
        double dY1 = dBaseY + dDeltaX * dHeight;
        double dX2 = dBaseX + dDeltaY * dHeight;
        double dY2 = dBaseY - dDeltaX * dHeight;
        if (dX1 > dX2 || dX1 == dX2 && dY1 > dY2)
        {
            pntOut1.Set(dX1, dY1);
            pntOut2.Set(dX2, dY2);
        }
        else
        {
            pntOut1.Set(dX2, dY2);
            pntOut2.Set(dX1, dY1);
        }
    }
}


double PlaceNodesHelper::GetAngle(Point const & p1, Point const & p2) const
{
    double dRet;
    double dDistance = p1.Distance(p2);
    if (dDistance != 0.0)
    {
        dRet = acos((p2.GetX() - p1.GetX()) / p1.Distance(p2));
        if (p2.GetY() < p1.GetY())
        {
            dRet += M_PI;
        }
    }
    else
    {
        dRet = 2.0 * M_PI;
    }
    return dRet;
}

