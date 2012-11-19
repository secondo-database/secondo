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

[1] Implementation of the class Vertex

[TOC]

1 Overview

This implementation file contains the implementation of the class Vertex

*/


#include "GraphAlgebra.h"

/*
2 Implementation of the class Vertex

*/


Vertex::Vertex()
{
}


Vertex::~Vertex()
{

}


Vertex* Vertex::Clone() const
{
    Vertex* pRet;
    if (IsDefined())
    {
        pRet = new Vertex(key, pos);
    }
    else
    {
        pRet = new Vertex;
        pRet->SetDefined(false);
    }
    return pRet;
}

size_t Vertex::Sizeof() const
{
    return sizeof(*this);
}

int Vertex::Compare(const Attribute* pAttr) const
{
    int nRet = 0;
    Vertex const * pVertex = dynamic_cast<Vertex const *>(pAttr);
    if (pVertex != NULL)
    {
        if (pVertex->IsDefined())
        {
            if (!IsDefined())
            {
                nRet = -1;
            }
            else if (key > pVertex->GetKey())
            {
                nRet = 1;
            }
            else if (key < pVertex->GetKey())
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


bool Vertex::Adjacent(const Attribute* pAttr) const
{
    bool bRet = false;
    Vertex const * pVertex = dynamic_cast<Vertex const *>(pAttr);
    if (pVertex != NULL)
    {
        bRet = abs(key - pVertex->GetKey()) == 1;
    }
    return bRet;
}

size_t Vertex::HashValue() const
{
    size_t nRet = 0;
    if (IsDefined())
    {
        nRet = key;
    }
    return nRet;
}

void Vertex::CopyFrom( const Attribute* arg)
{
    Vertex const * pVertex = dynamic_cast<Vertex const *>(arg);
    if (pVertex != NULL)
    {
        SetDefined(pVertex->IsDefined());
        key = pVertex->GetKey();
        pos = pVertex->GetPos();
    }
}


void Vertex::SetPos(Coord coordX, Coord coordY)
{
    pos.Set(coordX, coordY);
}


