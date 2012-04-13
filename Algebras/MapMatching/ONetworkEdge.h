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

[1] Header File of ONetworkEdge (Edge of Network based on ordered relation)

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the class ~ONetworkEdge~

2 Defines and includes

*/

#ifndef __ONETWORKEDGE_H_
#define __ONETWORKEDGE_H_

class Tuple;
class SimpleLine;
class CcInt;

#include <Point.h>

#include <stdio.h>
#include <string.h>


/*
3 class ONetworkEdge
  Edge of Network based on ordered relation

*/

class ONetworkEdge
{
public:
    ONetworkEdge();
    ONetworkEdge(Tuple* pTupleEdge,
                 bool bIncReference = true);
    ONetworkEdge(const ONetworkEdge& rEdge);
    ~ONetworkEdge();

    ONetworkEdge& operator=(const ONetworkEdge& rEdge);

    bool operator==(const ONetworkEdge& rEdge) const;

    bool IsDefined(void) const {return m_pTupleEdge != NULL;}

    //int GetSourceId(void) const;

    //int GetTargetId(void) const;

    CcInt* GetSource(void) const;

    CcInt* GetTarget(void) const;

    Point GetSourcePoint(void) const;

    Point GetTargetPoint(void) const;

    SimpleLine* GetCurve(void) const;

    std::string GetRoadName(void) const;

    std::string GetRoadType(void) const;

    std::string GetMaxSpeed(void) const;

    void Print(std::ostream& os) const;

private:

    Tuple* m_pTupleEdge;
};


#endif /* __ONETWORKEDGE_H_ */
