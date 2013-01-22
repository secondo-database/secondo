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
    //cout << "rvecRouteCandiate:" << i << endl;
    if (pCandidate != NULL)
    {
      //pCandidate->Print(cout);
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
            !(pRLStart = GetRLoc(pData1, 1.0))))
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
                  !(pRLEnd = GetRLoc(pData2, 1.0))))
        {
          pData2 = *itData;
          ++itData;
        }
        if (pData2 != NULL && pRLEnd)
        {
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
                pPrevEnd(GetRLoc(pLastPointOfPrevSection, 1.0));
              CalcShortestPath(*pPrevEnd, *pRLStart,
                               pLastPointOfPrevSection, pData1);
            }
            bCalcShortestPath = false;
          }

          if (pData2 != NULL && pRLEnd)
          {
            CalcShortestPath(*pRLStart, *pRLEnd, pData1, pData2);
            pLastPointOfPrevSection = pData2;
          }

          pData1 = pData2;
          pRLStart = pRLEnd;
        }
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
    //cout << "initialisation of result mjpoint in mjpoint creator successful" << endl;
    return true;
  }
}

void  MJPointCreator::Finalize()
{
    resMJPoint->EndBulkload(false);
}

const mapmatch::AttributePtr< RouteLocation >
   MJPointCreator::GetRLoc(
      const mapmatch::MHTRouteCandidate::PointDataPtr& pData,
      const double& dNetworkScale) const
{
  //cout << "Called GetRLoc for" << endl;
  if (pData == NULL)
    return NULL;

  const shared_ptr<IMMNetworkSection>& pSection = pData->GetSection();
  if (pSection == NULL)
    return NULL;
  const JNetworkSectionAdapter* pAdapter = pSection->CastToJNetworkSection();
  if (pAdapter == NULL)
    return NULL;
  const Point* pPointProjection = pData->GetPointProjection();
  //cout <<  "Point: " << *pPointProjection << endl;
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
  {
    //cout << "found: " << dPos << endl;
    Direction dir(Both);
    RouteLocation* rlocStart = pAdapter->GetSectionStartRLoc();
    //cout << "startLocationOfSection: " << *rlocStart << endl;
    AttributePtr<RouteLocation> res(new RouteLocation(rlocStart->GetRouteId(),
                                               rlocStart->GetPosition() + dPos,
                                               dir));
    return res;
  }
  else
  {
    // Projected point could not be matched onto route
    assert(false);
    return NULL;
  }
}

bool MJPointCreator::CalcShortestPath(const jnetwork::RouteLocation& from,
                                      const jnetwork::RouteLocation& to,
                                  const MHTRouteCandidate::PointDataPtr ptrFrom,
                                  const MHTRouteCandidate::PointDataPtr ptrTo)
    const
{
  if (!from.IsDefined() || !to.IsDefined() || !ptrFrom || !ptrTo)
    return false;

  double length = 0.0;
  DbArray<JRouteInterval>* sp =
    jnet->ShortestPath(from, to, ptrTo->GetPointProjection(), length);
  if (sp == 0)
    return false;
  if (sp->Size() <= 0)
  {
    sp->Destroy();
    delete sp;
    return false;
  }
  double lengthmeter = CalcLengthCurveMeter(sp);
  if (!MMUtil::CheckSpeed(lengthmeter,
                          ptrFrom->GetTime(), ptrTo->GetTime(),
                          *ptrFrom->GetPointProjection(),
                          *ptrTo->GetPointProjection(),
                          ptrFrom->GetSection()->GetRoadType(),
                       max(ptrFrom->GetSection()->GetMaxSpeed(),
                           ptrTo->GetSection()->GetMaxSpeed())))
    return false;
  jnet->ComputeAndAddUnits(sp, ptrFrom->GetTime(), ptrTo->GetTime(), true,
                           false, length, resMJPoint);
  return true;
}

double MJPointCreator::CalcLengthCurveMeter(const DbArray<JRouteInterval>* path)
    const
{
  double result = 0.0;
  JRouteInterval rint;
  SimpleLine* sline = new SimpleLine(0);
  for (int i = 0; i < path->Size(); i++)
  {
    path->Get(i,rint);
    jnet->GetSpatialValueOf(rint, *sline);
    result += MMUtil::CalcLengthCurve( sline, 1.0);
  }
  return result;
}

} // end of namespace mapmatch