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

[1] Implementation of the class Edge

[TOC]

1 Overview

This implementation file contains the implementation of the class Edge

*/

#include "GraphAlgebra.h"


/*
2 Implementation of the class Edge

*/


Edge::Edge()
{
}

Edge::Edge(int nSource, int nTarget, float fCost) :
  Attribute(true),
  source(nSource), target(nTarget), cost(fCost)
{
  SetDefined(true);
}


Edge::~Edge()
{

}


Edge* Edge::Clone()const
{
  Edge* pRet;
  if (IsDefined())
  {
    pRet = new Edge(source, target, cost);
  }
  else
  {
    pRet = new Edge;
    pRet->SetDefined(false);
  }
  return pRet;
}

size_t Edge::Sizeof() const
{
  return sizeof(*this);
}

int Edge::Compare(const Attribute* pAttr) const
{
  int nRet = 0;
  Edge const * pEdge = dynamic_cast<Edge const *>(pAttr);
  if (pEdge != NULL)
  {
    if (pEdge->IsDefined())
    {
      if (!IsDefined())
      {
        nRet = -1;
      }
      else if (source > pEdge->GetSource())
      {
        nRet = 1;
      }
      else if (source < pEdge->GetSource())
      {
        nRet = -1;
      }
      else if (target > pEdge->GetTarget())
      {
        nRet = 1;
      }
      else if (target < pEdge->GetTarget())
      {
        nRet = -1;
      }
    }
    else if (IsDefined())
    {
      nRet = 1;
    }
  }
  else
  {
    nRet = -1;
  }
  return nRet;
}


bool Edge::Adjacent(const Attribute*) const
{
  return false;
}

size_t Edge::HashValue() const
{
  size_t nRet = 0;
  if (IsDefined()) {
    nRet = source + target;
  }
  return nRet;
}

void Edge::CopyFrom(const Attribute* arg)
{
  Edge const * pEdge = dynamic_cast<Edge const *>(arg);
  if (pEdge != NULL)
  {
    SetDefined(pEdge->IsDefined());
    source = pEdge->GetSource();
    target = pEdge->GetTarget();
    cost = pEdge->GetCost();
  }
}

