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

#include "MapMatchingMHTMJPointCreator.h"
#include "MapMatchingUtil.h"
#include "LogMsg.h"
#include "JNetworkSectionAdapter.h"

using namespace jnetwork;

namespace mapmatch{
/*
1 Implementation of class  MJPointCreator

*/

 MJPointCreator::MJPointCreator(JNetworkAdapter* pJNetAdapter,
                                MJPoint* pResMJPoint) :
    jnet(pJNetAdapter != 0 ? pJNetAdapter->GetNetwork() : 0),
    resMJPoint(pResMJPoint)
{}

 MJPointCreator::~MJPointCreator()
{}

bool  MJPointCreator::CreateResult(
  const std::vector< MHTRouteCandidate* >& rvecRouteCandidates)
{
  if (jnet == NULL || !Init()) return false;

  for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
  {
    MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
    //cout << "rvecRouteCandiate:" << i << endl;
    if (pCandidate != NULL)
    {
      //pCandidate->Print(cout);

      MHTRouteCandidate::PointDataIterator itData =
          pCandidate->PointDataBegin();
      MHTRouteCandidate::PointDataIterator itDataEnd =
          pCandidate->PointDataEnd();

      // Find first defined point
      MHTRouteCandidate::PointDataPtr pData1;
      RouteLocation* startRLoc = 0;
      while(itData != itDataEnd && (pData1 == NULL || startRLoc == NULL))
      {
        pData1 = *itData;
        if (pData1 != NULL)
        {
          startRLoc = GetRouteLocation(pData1);
        }
        ++itData;
      }

      // Process next points
      while (itData != itDataEnd && pData1 != NULL && startRLoc != NULL)
      {
        MHTRouteCandidate::PointDataPtr pData2;
        RouteLocation* endRLoc = 0;

        while (itData != itDataEnd && (pData2 == NULL || endRLoc == NULL))
        {
          pData2 = *itData;
          if (pData2 != NULL)
          {
            endRLoc = GetRouteLocation(pData2);
          }
          ++itData;
        }

        if (endRLoc != NULL)
        {
          jnet->SimulateTrip(*startRLoc, *endRLoc,
                             pData2->GetPointProjection(), pData1->GetTime(),
                             pData2->GetTime(), resMJPoint);
          pData1 = pData2;
          startRLoc->DeleteIfAllowed();
          startRLoc = endRLoc;
          endRLoc = 0;
        }
      }
      if (startRLoc != 0)
      {
        startRLoc->DeleteIfAllowed();
        startRLoc = 0;
      }
    }
  }
  Finalize();
  return resMJPoint->IsDefined();
}

bool  MJPointCreator::Init()
{
  if (resMJPoint == NULL || jnet == NULL || !jnet->IsDefined())
  {
    //cout << "initialisation failed in init of mjpoint creator" << endl;
    return false;
  }
  else
  {
    resMJPoint->Clear();
    resMJPoint->SetDefined(true); // always defined
    resMJPoint->SetNetworkId(*jnet->GetId());
    resMJPoint->StartBulkload();
    //cout << "init of result mjpoint in mjpoint creator successful" << endl;
    return true;
  }
}

void  MJPointCreator::Finalize()
{
    resMJPoint->EndBulkload();
}

RouteLocation* MJPointCreator::GetRouteLocation(
                            const MHTRouteCandidate::PointDataPtr pData) const
{
  RouteLocation* result = 0;
  if (pData != NULL)
  {
    const Point* p = pData->GetPointProjection();
    shared_ptr<IMMNetworkSection> pSect = pData->GetSection();
    if (p != NULL && pSect != NULL)
    {
      JNetworkSectionAdapter* pJNetSect = pSect->CastToJNetworkSection();
      result = pJNetSect->GetRouteLocation(p);
      if (result != 0)
      {
        result->SetSide(pJNetSect->GetSide());
      }
    }
  }
  return result;
}

} // end of namespace mapmatch