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

[1] Implementation of utilities for map matching

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of
utilities for map matching

2 Defines and includes

*/
#include "MapMatchingUtil.h"
#include "../Spatial/SpatialAlgebra.h"

#include <stdio.h>
#include <limits>


namespace mapmatch {


bool MMUtil::Intersects(const Region& rRegion, const SimpleLine& rSLine)
{
#if 0
    Line LineFromSLine(0);
    rSLine.toLine(LineFromSLine);
#if 1
    Line Result(0);
    rRegion.Intersection(LineFromSLine, Result);
    return Result.IsDefined() && !Result.IsEmpty();
#else
    return AlmostEqual(rRegion.Distance(LineFromSLine), 0.0);
#endif
#else

    int nSize = rSLine.Size();
    for (int i = 0; i< nSize; ++i)
    {
        HalfSegment hs; // TODO falscher Konstruktor
        rSLine.Get(i, hs);
        if(hs.IsLeftDomPoint() && rRegion.Intersects(hs))
            return true;
    }
    return false;

#endif
}

double MMUtil::CalcOrthogonalProjection(const HalfSegment& rHalfSegment,
                                        const Point& rPt, Point& rPtRes,
                                        const Geoid* pGeoid)
{
    // Modified copy of HalfSegment::Distance(Point)
    // We need distance and projected point

    Coord xl = rHalfSegment.GetLeftPoint().GetX();
    Coord yl = rHalfSegment.GetLeftPoint().GetY();
    Coord xr = rHalfSegment.GetRightPoint().GetX();
    Coord yr = rHalfSegment.GetRightPoint().GetY();
    Coord X = rPt.GetX();
    Coord Y = rPt.GetY();

    if (AlmostEqual(xl, xr)) // vertical
    {
        if ((yl <= Y && Y <= yr) || (yr <= Y && Y <= yl))
        {
            rPtRes.Set(xl, Y);
            if (pGeoid != NULL)
                return rPt.Distance(rPtRes, pGeoid);
            else
                return fabs(X - xl);
        }
        else
        {
            rPtRes.SetDefined(false);
            return std::numeric_limits<double>::max();
        }
    }
    else if (AlmostEqual(yl, yr)) // horizontal
    {
        if ((xl <= X && X <= xr) || (xr <= X && X <= xl))
            // if (xl <= X && X <= xr)
        {
            rPtRes.Set(X, yl);
            if (pGeoid != NULL)
                return rPt.Distance(rPtRes, pGeoid);
            else
                return fabs(Y - yl);
        }
        else
        {
            rPtRes.SetDefined(false);
            return std::numeric_limits<double>::max();
        }
    }
    else
    {
        Coord k = (yr - yl) / (xr - xl);
        Coord a = yl - k * xl;
        Coord xx = (k * (Y - a) + X) / (k * k + 1);
        Coord yy = k * xx + a;

        if (xl <= xx && xx <= xr)
        {
            rPtRes.Set(xx, yy);
            return rPt.Distance(rPtRes, pGeoid);
        }
        else
        {
            rPtRes.SetDefined(false);
            return std::numeric_limits<double>::max();
        }
    }
}

Point MMUtil::CalcOrthogonalProjection(const SimpleLine& rLine,
                                       const Point& rPt,
                                       double& rdDistanceRes,
                                       const Geoid* pGeoid)
{
    Point ResPoint(false /*not defined*/);
    double dShortestDistance = std::numeric_limits<double>::max();

    for (int i = 0; i < rLine.Size(); ++i)
    {
        HalfSegment hs;
        rLine.Get(i, hs);
        if (hs.IsLeftDomPoint())
        {
            Point ResPointSeg(false /*not defined*/);
            double dDistance = CalcOrthogonalProjection(hs, rPt,
                                                        ResPointSeg, pGeoid);
            if (ResPointSeg.IsDefined() && dDistance < dShortestDistance)
            {
                dShortestDistance = dDistance;
                ResPoint = ResPointSeg;
            }
        }
    }

    rdDistanceRes = dShortestDistance;
    return ResPoint;
}

double MMUtil::CalcProjection(const HalfSegment& rHalfSegment,
                              const Point& rPt, Point& rPtRes,
                              bool& bIsOrthogonal,
                              const Geoid* pGeoid)
{
    // Modified copy of HalfSegment::Distance(Point)
    // We need distance and projected point

    Coord xl = rHalfSegment.GetLeftPoint().GetX();
    Coord yl = rHalfSegment.GetLeftPoint().GetY();
    Coord xr = rHalfSegment.GetRightPoint().GetX();
    Coord yr = rHalfSegment.GetRightPoint().GetY();
    Coord X = rPt.GetX();
    Coord Y = rPt.GetY();

    bIsOrthogonal = false;

    if (AlmostEqual(xl, xr)) // vertical
    {
        if ((yl <= Y && Y <= yr) || (yr <= Y && Y <= yl))
        {
            bIsOrthogonal = true;

            rPtRes.Set(xl, Y);
            if (pGeoid != NULL)
                return rPt.Distance(rPtRes, pGeoid);
            else
                return fabs(X - xl);
        }
    }
    else if (AlmostEqual(yl, yr)) // horizontal
    {
        if ((xl <= X && X <= xr) || (xr <= X && X <= xl))
            // if (xl <= X && X <= xr)
        {
            bIsOrthogonal = true;

            rPtRes.Set(X, yl);
            if (pGeoid != NULL)
                return rPt.Distance(rPtRes, pGeoid);
            else
                return fabs(Y - yl);
        }
    }
    else
    {
        Coord k = (yr - yl) / (xr - xl);
        Coord a = yl - k * xl;
        Coord xx = (k * (Y - a) + X) / (k * k + 1);
        Coord yy = k * xx + a;

        if (xl <= xx && xx <= xr)
        {
            bIsOrthogonal = true;

            rPtRes.Set(xx, yy);
            return rPt.Distance(rPtRes, pGeoid);
        }
    }

    // No orthogonal projection possible
    // -> Calc shortest distance to left or right point

    bIsOrthogonal = false;
    double dDistanceLeft = rPt.Distance(rHalfSegment.GetLeftPoint(), pGeoid);
    double dDistanceRight = rPt.Distance(rHalfSegment.GetRightPoint(), pGeoid);
    if (dDistanceLeft <= dDistanceRight)
    {
        rPtRes.Set(rHalfSegment.GetLeftPoint());
        return dDistanceLeft;
    }
    else
    {
        rPtRes.Set(rHalfSegment.GetRightPoint());
        return dDistanceRight;
    }
}

Point MMUtil::CalcProjection(const SimpleLine& rLine,
                             const Point& rPt,
                             double& rdDistanceRes,
                             bool& bIsOrthogonal,
                             const Geoid* pGeoid)
{
    Point ResPoint(false /*not defined*/);
    double dShortestDistance = std::numeric_limits<double>::max();

    for (int i = 0; i < rLine.Size(); ++i)
    {
        HalfSegment hs;
        rLine.Get(i, hs);
        if (hs.IsLeftDomPoint())
        {
            Point ResPointSeg(false /*not defined*/);
            bool bOrthogonal = false;
            double dDistance = CalcProjection(hs, rPt, ResPointSeg,
                                              bOrthogonal, pGeoid);
            if (ResPointSeg.IsDefined() && dDistance < dShortestDistance)
            {
                dShortestDistance = dDistance;
                ResPoint = ResPointSeg;
                bIsOrthogonal = bOrthogonal;
            }
        }
    }

    rdDistanceRes = dShortestDistance;
    return ResPoint;
}

double MMUtil::CalcDistance(const std::vector<const Point*>& rvecPoints,
                            const Geoid* pGeoid)
{
    double dDistance = 0.0;

    const size_t nSize = rvecPoints.size();
    if (nSize > 0)
    {
        const Point* pPrevPoint = rvecPoints[0];

        for (size_t i = 1; i < nSize; ++i)
        {
            const Point* pActPoint = rvecPoints[i];
            if (pActPoint != NULL)
            {
                if (pActPoint != NULL)
                    dDistance += pPrevPoint->Distance(*pActPoint, pGeoid);

                pPrevPoint = pActPoint;
            }
        }
    }

    return dDistance;
}

double MMUtil::CalcLengthCurve(const SimpleLine* pCurve,
                               const Geoid* pGeoid)
{
    if (pGeoid != NULL)
    {
        bool bValid = true;
        double dLengthCurve = pCurve->Length(*pGeoid, bValid);
        if (bValid)
            return dLengthCurve;
        else
            return pCurve->Length();
    }
    else
    {
        return pCurve->Length();
    }
}

// modified copy of SimpleLine::AtPoint
bool MMUtil::GetPosOnSimpleLine(const SimpleLine& rLine,
                                const Point& p,
                                bool startsSmaller,
                                double tolerance,
                                double& result)
{
    if (rLine.IsEmpty() || !p.IsDefined())
    {
        return false;
    }

    bool found = false;
    HalfSegment hs;
    const Rectangle<2> rectBounding(true,
                                    p.GetX() - tolerance,
                                    p.GetX() + tolerance,
                                    p.GetY() - tolerance,
                                    p.GetY() + tolerance);

    for (int nPos = 0; nPos < rLine.Size(); ++nPos)
    {
        rLine.Get(nPos, hs);

        double dDistance = hs.Distance(rectBounding);

        if (hs.IsLeftDomPoint() && AlmostEqual(dDistance, 0.0))
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        LRS lrs;
        rLine.Get(hs.attr.edgeno, lrs);
        rLine.Get(lrs.hsPos, hs);
        result = lrs.lrsPos + p.Distance(hs.GetDomPoint());

        if (!startsSmaller)
            result = rLine.Length() - result;

        /*if (tolerance != 0.0)
        {
            if (AlmostEqualAbsolute(result, 0.0, tolerance))
                result = 0;
            else if (AlmostEqualAbsolute(result, rLine.Length(), tolerance))
                result = rLine.Length();
        }
        else*/
        {
            if (AlmostEqual(result, 0.0))
                result = 0.0;
            else if (AlmostEqual(result, rLine.Length()))
                result = rLine.Length();
        }

        return true;
    }
    return false;
}



} // end of namespace mapmatch


