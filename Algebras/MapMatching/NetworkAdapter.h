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

[1] Header File containing class to access Network via INetwork interface

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the class ~NetworkAdapter~

2 Defines and includes

*/

#ifndef __NETWORKADAPTER_H_
#define __NETWORKADAPTER_H_

#include "MapMatchingNetworkInterface.h"
#include <RectangleAlgebra.h>

class Network;
class NetworkRoute;
class DirectedNetworkSection;
class Region;


namespace mapmatch {


/*
3 ~NetworkAdapter~

*/
class NetworkAdapter : public IMMNetwork
{
public:
    NetworkAdapter(Network* pNetwork = NULL);
    NetworkAdapter(const NetworkAdapter& rNetworkAdapter);

    virtual ~NetworkAdapter();

    virtual bool GetSections(
               const Rectangle<2>& rBBox,
               std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const;

    virtual Rectangle<2> GetBoundingBox(void) const;

    virtual double GetNetworkScale(void) const;

    virtual bool IsDefined(void) const;

private:

    void GetSectionsOfRoute(
                const NetworkRoute& rNetworkRoute,
                const Region& rRegion,
                std::vector<shared_ptr<IMMNetworkSection> >& rVecSectRes) const;

    Network* m_pNetwork;
};


/*
4 ~NetworkSectionAdapter~

*/
class NetworkSectionAdapter : public IMMNetworkSection
{
public:
    NetworkSectionAdapter();
    NetworkSectionAdapter(const DirectedNetworkSection& rSection);
    NetworkSectionAdapter(const NetworkSectionAdapter& rNetworkSectionAdapter);
    virtual ~NetworkSectionAdapter();

    virtual const class SimpleLine* GetCurve(void) const;

    virtual double GetCurveLength(const double dScale) const;

    virtual bool GetCurveStartsSmaller(void) const;

    virtual Point GetStartPoint(void) const;

    virtual Point GetEndPoint(void) const;

    virtual EDirection GetDirection(void) const;

    virtual bool IsDefined(void) const;

    virtual bool GetAdjacentSections(
                const bool bUpDown,
                std::vector<shared_ptr<IMMNetworkSection> >& vecSections) const;

    virtual bool operator==(const IMMNetworkSection& rSection) const;

    virtual const NetworkSectionAdapter* CastToNetworkSection(void) const;

    virtual NetworkSectionAdapter* CastToNetworkSection(void);

    virtual void PrintIdentifier(std::ostream& os) const;

    const DirectedNetworkSection* GetSection(void) const {return m_pSection;}

private:
    DirectedNetworkSection* m_pSection;
};


} // end of namespace mapmatch

#endif /* __NETWORKADAPTER_H_ */
