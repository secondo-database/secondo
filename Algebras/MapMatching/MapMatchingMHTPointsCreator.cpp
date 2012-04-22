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

[1] Implementation File containing class ~PointsCreator~

April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of
the class ~PointsCreator~.

2 Defines and includes

*/

#include "MapMatchingMHTPointsCreator.h"

#include <SpatialAlgebra.h>


namespace mapmatch {


/*
3 class PointsCreator
  Creates a Points-object with projected points

*/
PointsCreator::PointsCreator(Points* pResPoints)
:m_pResPoints(pResPoints)
{
}

PointsCreator::~PointsCreator()
{
    m_pResPoints = NULL;
}

bool PointsCreator::CreateResult(const std::vector<MHTRouteCandidate*>&
                                                           rvecRouteCandidates)
{
    if (!Init())
        return false;

    for (size_t i = 0; i < rvecRouteCandidates.size(); ++i)
    {
        MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate != NULL)
        {
            MHTRouteCandidate::PointDataIterator itData =
                                                   pCandidate->PointDataBegin();
            MHTRouteCandidate::PointDataIterator itDataEnd =
                                                     pCandidate->PointDataEnd();

            // Find first defined point
            const MHTRouteCandidate::PointData* pData = NULL;
            while(itData != itDataEnd)
            {
                pData = *itData;

                if (pData != NULL)
                {
                    const Point* pPtProjection = pData->GetPointProjection();
                    if (pPtProjection != NULL)
                    {
                        m_pResPoints->operator +=(*pPtProjection);
                    }
                }

                ++itData;
            }
        }
    }

    Finalize();

    return true;
}

bool PointsCreator::Init(void)
{
    if (m_pResPoints == NULL)
        return false;
    else
    {
        m_pResPoints->Clear();

        m_pResPoints->SetDefined(true); // always defined

        m_pResPoints->StartBulkLoad();

        return true;
    }
}

void PointsCreator::Finalize(void)
{
    if (m_pResPoints != NULL)
    {
        m_pResPoints->EndBulkLoad(false, false, false);
    }
}

} // end of namespace mapmatch

