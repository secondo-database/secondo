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

[1] Header File containing class to access ONetwork via IMMNetwork interface

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the class ~ONetworkAdapter~

2 Defines and includes

*/

#ifndef __ONETWORKADAPTER_H_
#define __ONETWORKADAPTER_H_

#include "MapMatchingNetworkInterface.h"

class ONetwork;
class ONetworkEdge;


namespace mapmatch {


/*
3 ~ONetworkAdapter~

*/
class ONetworkAdapter : public IMMNetwork
{
public:
    ONetworkAdapter(ONetwork* pNetwork = NULL);
    ONetworkAdapter(const ONetworkAdapter& rONetworkAdapter);

    virtual ~ONetworkAdapter();

    virtual bool GetSections(
               const Rectangle<2>& rBBox,
               std::vector<shared_ptr<IMMNetworkSection> >& rvecSections) const;

    virtual Rectangle<2> GetBoundingBox(void) const;

    virtual double GetNetworkScale(void) const;

    virtual bool IsDefined(void) const;


    // Only used by MapMatchingOEdgeTupleStreamCreator
    Tuple* GetUndefEdgeTuple(void) const;

private:

    ONetwork* m_pNetwork;
};


/*
4 ~ONetworkSectionAdapter~

*/
class ONetworkSectionAdapter : public IMMNetworkSection
{
public:
    ONetworkSectionAdapter();
    ONetworkSectionAdapter(const ONetworkEdge& rEdge,
                           const ONetwork* pONetwork);
    ONetworkSectionAdapter(const ONetworkSectionAdapter&
                                                       rONetworkSectionAdapter);
    virtual ~ONetworkSectionAdapter();

    virtual const class SimpleLine* GetCurve(void) const;

    virtual double GetCurveLength(const double dScale) const;

    virtual bool GetCurveStartsSmaller(void) const;

    virtual Point GetStartPoint(void) const;

    virtual Point GetEndPoint(void) const;

    virtual EDirection GetDirection(void) const;

    virtual bool IsDefined(void) const;

    virtual bool GetAdjacentSections(
                const bool _bUpDown,
                std::vector<shared_ptr<IMMNetworkSection> >& vecSections) const;

    virtual std::string GetRoadName(void) const;

    virtual ERoadType GetRoadType(void) const;

    virtual double GetMaxSpeed(void) const;

    virtual bool operator==(const IMMNetworkSection& rSection) const;

    virtual const ONetworkSectionAdapter* CastToONetworkSection(void) const;

    virtual ONetworkSectionAdapter* CastToONetworkSection(void);


    virtual void PrintIdentifier(std::ostream& os) const;

    const ONetworkEdge* GetNetworkEdge(void) const {return m_pEdge;}

private:
    ONetworkEdge* m_pEdge;
    const ONetwork* m_pONetwork;
    mutable ERoadType m_eRoadType;
};


} // end of namespace mapmatch

#endif /* __ONETWORKADAPTER_H_ */
