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
                                               const TupleId stid) :
  IMMNetworkSection(),  pJNet(jnet),
  sectTup(0)
{
  if (jnet != 0 && jnet->IsDefined())
    sectTup = jnet->GetSectionTupleWithTupleId(stid);
  if (sectTup != 0)
  {
    Direction* sDir = pJNet->GetSectionDirection(sectTup);
    Direction cDir(Down);
    int test = sDir->Compare(cDir);
    if (test < 0)
      driveDir = DIR_UP;
    else if (test == 0)
      driveDir = DIR_DOWN;
    else
      driveDir = DIR_NONE;
    sDir->DeleteIfAllowed();
  }
}

JNetworkSectionAdapter::JNetworkSectionAdapter(JNetwork* jnet,
                                               Tuple* tup,
                                               const EDirection dDir):
  IMMNetworkSection(), pJNet(jnet), sectTup(0), driveDir(dDir)
{
  if (tup != 0)
  {
    sectTup = tup;
    sectTup->IncReference();
  }

}

JNetworkSectionAdapter::~JNetworkSectionAdapter()
{
  sectTup->DeleteIfAllowed();
}

/*
1.1. Get JNetworkSectionInformation

*/

bool JNetworkSectionAdapter::GetAdjacentSections(const bool bUpDown,
                  vector< shared_ptr< IMMNetworkSection > >& vecSections) const
{
  //cout << "get Adjacent Sections: ";
  if (sectTup != 0)
  {
    int idEndNode = -1;
    //cout << bUpDown << ", of: "  << pJNet->GetSectionId(sectTup) << endl;
    vecSections.clear();
    JListInt* listSID = 0;
    if (bUpDown)
    {
      listSID =
        (JListInt*)sectTup->GetAttribute(JNetwork::SEC_LIST_ADJ_SECTIONS_UP);
      idEndNode =
        ((CcInt*)sectTup->GetAttribute(JNetwork::SEC_ENDNODE_ID))->GetIntval();
    }
    else
    {
      listSID =
        (JListInt*)sectTup->GetAttribute(JNetwork::SEC_LIST_ADJ_SECTIONS_DOWN);
      idEndNode =
       ((CcInt*)sectTup->GetAttribute(JNetwork::SEC_STARTNODE_ID))->GetIntval();
    }
    if (listSID != 0)
    {
      if (listSID->IsDefined() && listSID->GetNoOfComponents() > 0)
      {
        Tuple* curSectTup = 0;
        CcInt nextSID;
        for (int i = 0; i < listSID->GetNoOfComponents(); i++)
        {
          listSID->Get(i,nextSID);
          curSectTup = pJNet->GetSectionTupleWithId(nextSID.GetIntval());
          if (curSectTup != 0)
          {
            EDirection drDir(DIR_NONE);
            if (idEndNode == pJNet->GetSectionStartJunctionID(curSectTup))
              drDir = DIR_UP;
            else if (idEndNode == pJNet->GetSectionEndJunctionID(curSectTup))
              drDir = DIR_DOWN;
            else if (!bUpDown)
              drDir = DIR_DOWN;
            else
              drDir = DIR_UP;
            shared_ptr<IMMNetworkSection>
              insSect(new JNetworkSectionAdapter(pJNet, curSectTup, drDir));
            vecSections.push_back(insSect);
            curSectTup->DeleteIfAllowed();
            curSectTup = 0;
          }
        }
      }
      listSID = 0;
      return true;
    }
  }
  return false;
}

SimpleLine* JNetworkSectionAdapter::GetCurve() const
{
  if (sectTup != 0)
  {
    return (SimpleLine*) sectTup->GetAttribute(JNetwork::SEC_CURVE);
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
  bool result = false;
  SimpleLine* sectCurve = GetCurve();
  if (sectCurve != 0)
  {
    result = sectCurve->GetStartSmaller();
  }
  return result;
}

IMMNetworkSection::EDirection JNetworkSectionAdapter::GetDirection() const
{
  if (IsDefined())
    return driveDir;
  else
    return DIR_NONE;
}

Point JNetworkSectionAdapter::GetStartPoint() const
{
  if (IsDefined())
  {
    Point* res = pJNet->GetSectionStartPoint(sectTup);
    if (res != 0)
    {
      Point result(*res);
      res->DeleteIfAllowed();
      return Point(result);
    }
  }
  return Point(false);
}

Point JNetworkSectionAdapter::GetEndPoint() const
{
  if (IsDefined())
  {
    Point* res = pJNet->GetSectionEndPoint(sectTup);
     if (res != 0)
    {
      Point result(*res);
      res->DeleteIfAllowed();
      return Point(result);
    }
  }
  return Point(false);
}

double JNetworkSectionAdapter::GetMaxSpeed() const
{
  if (sectTup != 0)
    return ((CcReal*)sectTup->GetAttribute(JNetwork::SEC_VMAX))->GetRealval();
  else
    return 0.0;
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
  return (sectTup != 0 && other != 0 &&
          pJNet->GetSectionId(sectTup) == pJNet->GetSectionId(other->sectTup) &&
          driveDir == other->driveDir);
}

void JNetworkSectionAdapter::PrintIdentifier(ostream& os) const
{
  JRouteInterval* jrint = pJNet->GetSectionFirstRouteInterval(sectTup);
  os << "sid: " << pJNet->GetSectionId(sectTup)
     << ", RouteInterval: " << *jrint
     << ", driveDir: ";
  if (driveDir == DIR_NONE) os << "None" << endl;
  else if (driveDir == DIR_UP) os << "Up" << endl;
  else os << "Down" << endl;
  jrint->DeleteIfAllowed();
}

RouteLocation* JNetworkSectionAdapter::GetRouteLocation(const Point*& p) const
{
  if (IsDefined() && p != 0 && p->IsDefined())
    return pJNet->GetNetworkValueOfOn(p,sectTup);
  else
    return 0;
}

Direction JNetworkSectionAdapter::GetSide() const
{
  if (driveDir == DIR_DOWN)
    return Direction(Down);
  else if (driveDir == DIR_UP)
    return Direction(Up);
  else
    return Direction(Both);
}

} // end of namespace maptmatch
