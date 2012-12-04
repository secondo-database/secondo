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

[1] Header File containing interfaces to access networks

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains interfaces to access networks from map matching:
~IMMNetwork~, ~IMMNetworkSection~

2 Defines and includes

*/

#ifndef __MAPMATCHINGNETWORKINTERFACE_H_
#define __MAPMATCHINGNETWORKINTERFACE_H_


#include "RectangleAlgebra.h"
#include "Point.h"
#include <vector>
#include <string>
#ifdef SECONDO_WIN32
#include <memory>
#else
#include <tr1/memory>
#endif
using std::tr1::shared_ptr;


namespace mapmatch {

/*
3 interface IMMNetworkSection
  accessing network sections (e.g. edges)

*/
class IMMNetworkSection
{
public:

    virtual ~IMMNetworkSection() {}

    virtual const class SimpleLine* GetCurve(void) const = 0;

    virtual double GetCurveLength(const double dScale) const = 0;

    virtual bool GetCurveStartsSmaller(void) const = 0;

    virtual Point GetStartPoint(void) const = 0;

    virtual Point GetEndPoint(void) const = 0;

    enum EDirection
    {
        DIR_NONE, DIR_UP, DIR_DOWN
    };

    virtual EDirection GetDirection(void) const = 0;

    virtual bool IsDefined(void) const = 0;

    virtual bool GetAdjacentSections(
            const bool bUpDown,
            std::vector<shared_ptr<IMMNetworkSection> >& vecSections) const = 0;

    virtual std::string GetRoadName(void) const {return "";}

    // according to
    // http://wiki.openstreetmap.org/wiki/DE:Key:highway
    enum ERoadType
    {
        RT_UNKNOWN = 0,
        RT_MOTORWAY,
        //RT_MOTORWAY_LINK,
        RT_TRUNK,
        //RT_TRUNK_LINK,
        RT_PRIMARY,
        //RT_PRIMARY_LINK,
        RT_SECONDARY,
        //RT_SECONDARY_LINK,
        RT_TERTIARY,
        //RT_TERTIARY_LINK,
        RT_LIVING_STREET,
        RT_PEDESTRIAN,
        RT_RESIDENTIAL,
        RT_SERVICE,
        RT_TRACK,
        //RT_BUS_GUIDEWAY
        //RT_RACEWAY
        RT_ROAD,
        RT_PATH,
        RT_FOOTWAY,
        RT_CYCLEWAY,
        RT_BRIDLEWAY,
        RT_STEPS,
        RT_PROPOSED,
        RT_CONSTRUCTION,

        RT_OTHER
    };

    virtual ERoadType GetRoadType(void) const {return RT_UNKNOWN;}

    virtual double GetMaxSpeed(void) const {return -1.0;} // km/h

    virtual void PrintIdentifier(std::ostream& os) const = 0;

    virtual bool operator==(const IMMNetworkSection& rSection) const = 0;

    virtual const class NetworkSectionAdapter* CastToNetworkSection(void) const
    {return NULL;}

    virtual class NetworkSectionAdapter* CastToNetworkSection(void)
    {return NULL;}

    virtual const class ONetworkSectionAdapter* CastToONetworkSection(void)const
    {return NULL;}

    virtual class ONetworkSectionAdapter* CastToONetworkSection(void)
    {return NULL;}
    /*
   virtual const class JNetworkSectionAdapter* CastToJNetworkSection(void) const
   {return NULL;}

   virtual class JNetworkSectionAdapter* CastToJNetworkSection(void)
   {return NULL;}*/
};


/*
4 interface IMMNetwork

*/
class IMMNetwork
{
public:

    virtual ~IMMNetwork() {}

    virtual bool GetSections(
            const Rectangle<2>& rBBox,
            std::vector<shared_ptr<IMMNetworkSection> >& vecSections) const = 0;

    virtual Rectangle<2> GetBoundingBox(void) const = 0;

    virtual double GetNetworkScale(void) const {return 1.0;}

    virtual bool IsDefined(void) const = 0;

    virtual bool CanGetRoadType(void) const = 0;
};


} // end of namespace mapmatch


#endif /* __MAPMATCHINGNETWORKINTERFACE_H_ */
