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

2012, November Simone Jandt

1 Defines and Includes

*/

#include "JNetworkSectionAdapter.h"

using namespace jnetwork;

namespace mapmatch{

/*
1 Implementation of JNetworkSectionAdapter

1.1. Constructors and Deconstructor

*/


JNetworkSectionAdapter::JNetworkSectionAdapter(JNetwork* jnet,
                                               const TupleId stid):
  IMMNetworkSection(),
  pJNet(jnet),
  sectTup((jnet != 0 && jnet->IsDefined())?jnet->GetSectionTupleWithId(stid):0)
{}

JNetworkSectionAdapter::JNetworkSectionAdapter(JNetwork* jnet,
                                               Tuple* tup) :
  IMMNetworkSection(),
  pJNet(jnet),
  sectTup(tup)
{}

JNetworkSectionAdapter::~JNetworkSectionAdapter()
{}

/*
1.1. Get JNetworkSectionInformation

*/

bool JNetworkSectionAdapter::GetAdjacentSections(const bool bUpDown,
                  vector< shared_ptr< IMMNetworkSection > >& vecSections) const
{
  if (sectTup != 0)
  {
    vecSections.clear();
    JListInt* listSID = 0;
    if (bUpDown)
      listSID =
        (JListInt*) sectTup->GetAttribute(JNetwork::SEC_LIST_ADJ_SECTIONS_UP);
    else
      listSID =
        (JListInt*) sectTup->GetAttribute(JNetwork::SEC_LIST_ADJ_SECTIONS_DOWN);
    if (listSID != 0 && listSID->IsDefined())
    {
      Tuple* curSectTup = 0;
      CcInt nextSID;
      for (int i = 0; i < listSID->GetNoOfComponents(); i++)
      {
        listSID->Get(i,nextSID);
        curSectTup = pJNet->GetSectionTupleWithId(nextSID.GetIntval());
        if (curSectTup != 0)
        {
          shared_ptr<IMMNetworkSection>
          insSect(new JNetworkSectionAdapter(pJNet, curSectTup->Clone()));
          vecSections.push_back(insSect);
          curSectTup->DeleteIfAllowed();
          curSectTup = 0;
        }
      }
      return true;
    }
  }
  return false;
}

SimpleLine* JNetworkSectionAdapter::GetCurve() const
{
  if (sectTup != 0)
  {
    return (SimpleLine*) sectTup->GetAttribute(JNetwork::SEC_CURVE)->Copy();
  }
  else
    return 0;
}

double JNetworkSectionAdapter::GetCurveLength(const double dScale) const
{
  if (sectTup != 0)
    return ((CcReal*)sectTup->GetAttribute(JNetwork::SEC_LENGTH))->GetRealval();
  else
    return 0.0;
}

bool JNetworkSectionAdapter::GetCurveStartsSmaller() const
{
  if (IsDefined())
    return GetCurve()->StartsSmaller();
  else
    return true;
}

IMMNetworkSection::EDirection JNetworkSectionAdapter::GetDirection() const
{
  if (IsDefined())
  {
    Direction* dir =
      (Direction*) sectTup->GetAttribute(JNetwork::SEC_DIRECTION);
    Direction compD(Down);
    int test = compD.Compare(dir);
    if ( test == 0)
      return DIR_DOWN;
    else
      if (test < 0)
        return DIR_UP;
  }
  return DIR_NONE;
}

Point JNetworkSectionAdapter::GetStartPoint() const
{
  if (IsDefined())
  {
    Point* res = pJNet->GetSectionStartPoint(sectTup);
    Point result(*res);
    res->DeleteIfAllowed();
    return Point(result);
  }
  return Point(false);
}

Point JNetworkSectionAdapter::GetEndPoint() const
{
  if (IsDefined())
  {
    Point* res = pJNet->GetSectionEndPoint(sectTup);
    Point result(*res);
    res->DeleteIfAllowed();
    return Point(result);
  }
  return Point(false);
}

double JNetworkSectionAdapter::GetMaxSpeed() const
{
  if (sectTup != 0)
    return ((CcReal*) sectTup->GetAttribute(JNetwork::SEC_VMAX))->GetRealval();
  else
    return 0.1;
}

string JNetworkSectionAdapter::GetRoadName() const
{
  return "";
}

IMMNetworkSection::ERoadType JNetworkSectionAdapter::GetRoadType() const
{
  return RT_UNKNOWN;
}

bool JNetworkSectionAdapter::IsDefined() const
{
  return (pJNet != 0 && pJNet->IsDefined() && sectTup != 0);
}


const JNetworkSectionAdapter*
  JNetworkSectionAdapter::CastToJNetworkSection() const
{
  return this;
}

JNetworkSectionAdapter* JNetworkSectionAdapter::CastToJNetworkSection()
{
  return this;
}

bool JNetworkSectionAdapter::operator==(const
                                mapmatch::IMMNetworkSection& rSection) const
{
  const JNetworkSectionAdapter* other = rSection.CastToJNetworkSection();
  if (sectTup != 0 && other != 0 &&
      sectTup->GetNoAttributes() == other->sectTup->GetNoAttributes())
  {
    Attribute* lhs;
    Attribute* rhs;
    for (int i = 0; i < sectTup->GetNoAttributes(); i++)
    {
      lhs = sectTup->GetAttribute(i);
      rhs = other->sectTup->GetAttribute(i);
      if (lhs->Compare(rhs) != 0)
        return false;
    }
    return true;
  }
  else
    return false;
}

RouteLocation* JNetworkSectionAdapter::GetSectionStartRLoc() const
{
  JRouteInterval* rint = pJNet->GetSectionFirstRouteInterval(sectTup);
  if (rint != NULL)
  {
    JListRLoc* list = pJNet->GetSectionStartJunctionRLocs(sectTup);
    if (list != 0)
    {
      int i = 0;
      RouteLocation rloc;
      while ( i < list->GetNoOfComponents())
      {
        list->Get(i,rloc);
        if (rint->Contains(rloc))
        {
          rint->DeleteIfAllowed();
          list->DeleteIfAllowed();
          return new RouteLocation(rloc);
        }
        i++;
      }
      list->DeleteIfAllowed();
      list = 0;
    }
    rint->DeleteIfAllowed();
    rint = 0;
  }
  return 0;
}

void JNetworkSectionAdapter::PrintIdentifier(ostream& os) const
{
  sectTup->Print(os);
}

} // end of namespace maptmatch
