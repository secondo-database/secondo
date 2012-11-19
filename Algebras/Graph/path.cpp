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

[1] Implementation of the class Path

[TOC]

1 Overview

This implementation file contains the implementation of the class Path

*/


#include "GraphAlgebra.h"




/*
3 Implementation of class Path

*/


Path::Path() { }

Path::Path(bool bDefined)
    : Attribute(bDefined), myPath(0), cost(0.0)
{ }

Path::~Path() { }



float Path::GetCost() const
{
    if (!IsDefined() || (myPath.Size() == 0))
        return -1.0f;

    pathStruct s;

    myPath.Get(myPath.Size()-1,s);
    return cost-s.cost;
};


Path* Path::Clone() const
{
    Path* p = new Path(IsDefined());
    for (int i = 0; i < GetNoPathStructs(); ++i)
    {
        p->Append(GetPathStruct(i));
    }
    p->cost = cost;
    return p;
}

size_t Path::Sizeof() const
{
    return sizeof(*this);
}

/*
~EqualWay~

This function checks the equality of this path with the
argument with respect to the key in the path, i.e. ignoring
costs and positions.

*/

void Path::EqualWay(const Path* p, CcBool& result) const{
  if(!IsDefined() || !p->IsDefined()){
      result.SetDefined(false);
      return;
  }
  int mysize = this->GetNoPathStructs();
  int psize = p->GetNoPathStructs();
  if(mysize!=psize){
     result.Set(true,false);
     return;
  }
  pathStruct mps;
  pathStruct pps;
  for(int i=0; i<mysize;i++){
     this->myPath.Get(i,mps);
     p->myPath.Get(i,pps);
     if(mps.key != pps.key){
        result.Set(true,false);
        return;
     }
  }
  result.Set(true,true);
}

int Path::Compare(const Attribute* arg) const
{
    Path const * p = dynamic_cast<Path const *>(arg);
    if(!IsDefined() || !p->IsDefined()){
      return -1;
    }
    // first criterion is the length of the path
    int mlength = myPath.Size();
    int plength = p->myPath.Size();
    if(mlength<plength){
       return -1;
    }
    if(mlength>plength){
       return 1;
    }
    // second criterion are the costs
    if(this->cost < p->cost){
       return -1;
    }
    if(this->cost > p->cost){
       return 1;
    }

    // both paths have the same length
    // and the same costs
    // scan both path in parallel
    pathStruct mps;
    pathStruct pps;
    for(int i=0;i<mlength;i++){
       myPath.Get(i,mps);
       p->myPath.Get(i,pps);
       if(mps.key < pps.key){
          return -1;
       }
       if(mps.key > pps.key){
          return 1;
       }
       int cmp = mps.pos.Compare(&(pps.pos));
       if(cmp!=0){
          return cmp;
       }
       if(!AlmostEqual(mps.cost,pps.cost)){
          if(mps.cost < pps.cost){
             return -1;
          } else {
             return 1;
          }
       }
    }
    return 0;
}


bool Path::Adjacent(const Attribute*) const
{
    return false;
}

size_t Path::HashValue() const
{
    size_t nRet = 0;
    if (IsDefined()) {
      nRet += GetNoPathStructs();
      int nCount = GetNoPathStructs();
      for (int i = 0; (i < nCount) && (i <= 5); ++i){
        pathStruct p = GetPathStruct(i);
        nRet += p.key;
      }
    }
    return 0;
}

void Path::CopyFrom(const Attribute* arg)
{
    Path const * pArg = dynamic_cast<Path const *>(arg);
    myPath.clean();
    int nCount = pArg->GetNoPathStructs();
    for (int i = 0; i < nCount; ++i)
    {
        Append(pArg->GetPathStruct(i));
    }
        cost = pArg->cost;
        SetDefined(pArg->IsDefined());
}


int Path::NumOfFLOBs() const
{
    return 1;
}

Flob* Path::GetFLOB(const int i)
{
    return i == 0 ? &myPath : NULL;
}

pathStruct Path::GetPathStruct(int nIndex) const
{
    pathStruct pRet;
    if (nIndex >= 0 && nIndex < GetNoPathStructs())
    {
        myPath.Get(nIndex, pRet);
    }

    return pRet;
}

/*
3.1.1 Function for Operator ~edges~ of GraphAlgebra

*/
vector<Edge>* Path::GetEdges() const
{
    vector<Edge>* vEdges = new vector<Edge>(0);
    pathStruct pStruct;
    int source;
    int target;
    float cost;

    if (myPath.Size() < 2)
        return vEdges;
    pStruct = GetPathStruct(0);
    source = pStruct.key;
    cost = pStruct.cost;
    for (int i = 1; i < myPath.Size(); i++) {
        pStruct = GetPathStruct(i);
        target = pStruct.key;
        vEdges->push_back(Edge(source,target,cost));
        source = target;
        cost = pStruct.cost;
    }

    return vEdges;
}


/*
3.1.2 Function for Operator ~vertices~ of GraphAlgebra

*/

vector<Vertex>* Path::GetVertices() const
{

    vector<Vertex>* vVertices = new vector<Vertex>(0);
    pathStruct pStruct;

    for (int i = 0; i < myPath.Size(); i++) {
        pStruct = GetPathStruct(i);
        vVertices->push_back(Vertex(pStruct.key, pStruct.pos));
    }

    return vVertices;
}



