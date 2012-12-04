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

[1] Header File of the MapMatching Algebra

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the convenience-class ~NetworkSection~

2 Defines and includes

*/
#ifndef __NETWORK_SECTION_H__
#define __NETWORK_SECTION_H__

class Tuple;
class SimpleLine;
class TupleIdentifier;

#include <stdio.h>
#include <Point.h>
#include "NetworkAlgebra.h"

using namespace network;

/*
3 class NetworkSection
  accessing the attributes of a Network-Section

*/
class NetworkSection
{
public:
    NetworkSection();
    NetworkSection(Tuple* pTupleSection,
                   const network::Network* pNetwork,
                   bool bIncReference = true);
    NetworkSection(const NetworkSection& rNetworkSection);
    virtual ~NetworkSection();

    const NetworkSection& operator=(const NetworkSection& rNetworkSection);

    bool operator==(const NetworkSection& rSection) const;

    bool IsDefined(void) const {return m_pTupleSection != NULL;}

    int GetRouteID(void) const;

    const class NetworkRoute& GetRoute(void) const;

    double GetMeas1(void) const;

    double GetMeas2(void) const;

    bool GetDual(void) const;

    const SimpleLine* GetCurve(void) const;

    bool GetCurveStartsSmaller(void) const;

    // calculates the length of the curve in meters
    double GetCurveLength(const double dScale) const;

    TupleIdentifier* GetRRC(void) const;

    int GetSectionID(void) const;

    virtual Point GetStartPoint(void) const;

    virtual Point GetEndPoint(void) const;

    const network::Network* GetNetwork(void) const {return m_pNetwork;}



private:

    Tuple* m_pTupleSection;
    const network::Network* m_pNetwork;
    mutable class NetworkRoute* m_pNetworkRoute;
    mutable double m_dCurveLength;
};


/*
4 class DirectedNetworkSection
  accessing the attributes of a directed Network-Section

*/
class DirectedNetworkSection : public NetworkSection
{
public:

    enum EDirection
    {
        DIR_NONE,
        DIR_UP,
        DIR_DOWN
    };

    DirectedNetworkSection();

    DirectedNetworkSection(Tuple* pTupleSection,
                           const network::Network* pNetwork,
                           bool bIncReference = true,
                           const EDirection eDirection = DIR_NONE);

    DirectedNetworkSection(const DirectedNetworkSection& rNetworkSection);

    DirectedNetworkSection(const NetworkSection& rNetworkSection,
                           const EDirection eDirection);

    virtual ~DirectedNetworkSection();

    const DirectedNetworkSection& operator=
                                (const DirectedNetworkSection& rNetworkSection);

    bool operator==(const DirectedNetworkSection& rSection) const;

    inline void SetDirection(const EDirection eDir) {m_eDirection = eDir;}
    inline EDirection GetDirection(void) const {return m_eDirection;}

    virtual Point GetStartPoint(void) const;

    virtual Point GetEndPoint(void) const;

private:
    EDirection m_eDirection;
};


#endif /* __NETWORK_SECTION_H__ */
