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
    jnet(pJNetAdapter->GetNetwork()),
    resMJPoint(pResMJPoint)
{}

 MJPointCreator::~MJPointCreator()
{}

bool  MJPointCreator::CreateResult(
  const std::vector< MHTRouteCandidate* >& rvecRouteCandidates)
{
  resMJPoint->Clear();
  if (jnet == NULL || !Init())
  {
    return false;
  }

  MHTRouteCandidate::PointDataPtr pLastPointOfPrevSection;
  bool bPrevCandidateFailed = false; // -> matching failed
                                     // -> don't connect by shortest path

  for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
  {
    bool bCalcShortestPath = ((i > 0) && !bPrevCandidateFailed);

    MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
    if (pCandidate != NULL)
    {
      bPrevCandidateFailed = pCandidate->GetFailed();

      MHTRouteCandidate::PointDataIterator itData =
          pCandidate->PointDataBegin();
      MHTRouteCandidate::PointDataIterator itDataEnd =
          pCandidate->PointDataEnd();

      // Find first defined point
      MHTRouteCandidate::PointDataPtr pData1;
      AttributePtr<RouteLocation> pRLStart(NULL);
      while(itData != itDataEnd &&
            (pData1 == NULL ||
            !(pRLStart = GetRLoc(pData1, jnet->GetTolerance()))))
      {
        pData1 = *itData;
        ++itData;
      }

      // Process next points
      while (itData != itDataEnd && pData1 != NULL && pRLStart)
      {
        MHTRouteCandidate::PointDataPtr pData2;
        AttributePtr<RouteLocation> pRLEnd(NULL);

        while (itData != itDataEnd &&
               (pData2 == NULL ||
                  !(pRLEnd = GetRLoc(pData2, jnet->GetTolerance()))))
        {
          pData2 = *itData;
          ++itData;
        }
        if (bCalcShortestPath && pLastPointOfPrevSection != NULL)
        {
          // Calculate ShortestPath between last point of previous
          // candidate and first point of this candidate

          // ShortestPath calculation only when time difference
          // is less than 4 minutes
          const DateTime MaxTimeDiff(durationtype, 240000);

          if (pData1->GetTime() - pLastPointOfPrevSection->GetTime() <
              MaxTimeDiff)
          {
            AttributePtr<RouteLocation>
              pPrevEnd(GetRLoc(pLastPointOfPrevSection,
                               jnet->GetTolerance()));
            jnet->SimulateTrip(*pPrevEnd, *pRLStart,
                                pLastPointOfPrevSection->GetPointProjection(),
                                pData1->GetPointProjection(),
                                pLastPointOfPrevSection->GetTime(),
                                pData1->GetTime(), resMJPoint);
          }
          bCalcShortestPath = false;
        }

        if (pData2 != NULL && pRLEnd)
        {
           jnet->SimulateTrip(*pRLStart, *pRLEnd, pData1->GetPointProjection(),
                               pData2->GetPointProjection(),
                               pData1->GetTime(), pData2->GetTime(),
                               resMJPoint);
           pLastPointOfPrevSection = pData2;
        }

        pData1 = pData2;
        pRLStart = pRLEnd;
      }
    }
  }
  Finalize();
  return true;
}

bool  MJPointCreator::Init()
{
  if (resMJPoint == NULL || jnet == NULL || !jnet->IsDefined())
    return false;
  else
  {
    resMJPoint->Clear();
    resMJPoint->SetDefined(true); // always defined
    resMJPoint->SetNetworkId(*jnet->GetId());
    resMJPoint->StartBulkload();
    return true;
  }
}

void  MJPointCreator::Finalize()
{
    resMJPoint->EndBulkload();
}

const mapmatch::AttributePtr< RouteLocation >
   MJPointCreator::GetRLoc(
      const mapmatch::MHTRouteCandidate::PointDataPtr& pData,
      const double& dNetworkScale) const
{
  if (pData == NULL)
    return NULL;

  const shared_ptr<IMMNetworkSection>& pSection = pData->GetSection();
  if (pSection == NULL)
    return NULL;

  const JNetworkSectionAdapter* pAdapter = pSection->CastToJNetworkSection();
  if (pAdapter == NULL)
    return NULL;

  const Point* pPointProjection = pData->GetPointProjection();

  if (pPointProjection == NULL || !pPointProjection->IsDefined())
  {
    return NULL;
  }

  const bool RouteStartsSmaller = pAdapter->GetCurveStartsSmaller();
  const SimpleLine* pSectCurve(pAdapter->GetCurve());
  double dPos = 0.0;
  if (pSectCurve != NULL &&
    MMUtil::GetPosOnSimpleLine(*pSectCurve,
                               *pPointProjection,
                               RouteStartsSmaller,
                               dNetworkScale,
                               dPos))
    //pRouteCurve->AtPoint(*pPointProjection, RouteStartsSmaller, dPos))
  {
    RouteLocation* rlocStart = pAdapter->GetSectionStartRLoc();
    AttributePtr<RouteLocation> res(new RouteLocation(rlocStart->GetRouteId(),
                                               rlocStart->GetPosition() + dPos,
                                               rlocStart->GetSide()));
    return res;
  }
  else
  {
    // Projected point could not be matched onto route
    assert(false);
    return NULL;
  }
}


} // end of namespace mapmatch