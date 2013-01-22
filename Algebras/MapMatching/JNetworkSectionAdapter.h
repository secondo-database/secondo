/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012 November Simone Jandt

1 Defines and Includes

*/

#ifndef JNETWORKSECTIONADAPTER_H
#define JNETWORKSECTIONADAPTER_H

#include "MapMatchingNetworkInterface.h"
#include "JNetwork.h"

using namespace jnetwork;

namespace mapmatch{

/*
1 class JNetworkSectionAdapter

Enables mapmatching to JNetworkObjects by access to the requierd section data of
jnet.

*/

class JNetworkSectionAdapter : public IMMNetworkSection
{

public:

/*
1.1. Constructors and Deconstructor

*/

   JNetworkSectionAdapter(JNetwork* jnet, const TupleId sectTupId);
   JNetworkSectionAdapter(JNetwork* jnet, Tuple* tup,
                          const EDirection dDir);
   virtual ~JNetworkSectionAdapter();

/*
1.1. GetSectionInformation

*/
    virtual SimpleLine* GetCurve() const;

    virtual double GetCurveLength(const double dScale) const;

    virtual bool GetCurveStartsSmaller() const;

    virtual Point GetStartPoint() const;

    virtual Point GetEndPoint() const;

    virtual EDirection GetDirection() const;

    virtual bool IsDefined() const;

    virtual bool GetAdjacentSections(const bool bUpDown,
               std::vector<shared_ptr<IMMNetworkSection> >& vecSections) const;

    virtual std::string GetRoadName() const;

    virtual ERoadType GetRoadType() const;

    virtual double GetMaxSpeed() const;

/*
1.1. PrintIdentifier

*/
    virtual void PrintIdentifier(std::ostream& os) const;

/*
1.1. Comparision

*/
    virtual bool operator==(const IMMNetworkSection& rSection) const;

/*
1.1 CastFunctions

*/
 virtual const JNetworkSectionAdapter* CastToJNetworkSection() const;

 virtual JNetworkSectionAdapter* CastToJNetworkSection();

 RouteLocation* GetSectionStartRLoc() const;

private:
  JNetwork* pJNet;
  Tuple* sectTup;
  EDirection driveDir;

};

} // end of namespace mapmatch

#endif //JNETWORKSECTIONADAPTER_H
