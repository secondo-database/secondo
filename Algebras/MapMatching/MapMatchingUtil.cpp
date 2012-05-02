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
#include <SpatialAlgebra.h>
#include <NetworkAlgebra.h>
#include "NetworkAdapter.h"
#include "MapMatchingNetworkInterface.h"

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
                                        const double dScale)
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
            return MMUtil::CalcDistance(rPt, rPtRes, dScale);
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
            return MMUtil::CalcDistance(rPt, rPtRes, dScale);
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
            return MMUtil::CalcDistance(rPt, rPtRes, dScale);
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
                                       const double dScale)
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
                                                        ResPointSeg, dScale);
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
                              const double dScale)
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
            return MMUtil::CalcDistance(rPt, rPtRes, dScale);
        }
    }
    else if (AlmostEqual(yl, yr)) // horizontal
    {
        if ((xl <= X && X <= xr) || (xr <= X && X <= xl))
            // if (xl <= X && X <= xr)
        {
            bIsOrthogonal = true;

            rPtRes.Set(X, yl);
            return MMUtil::CalcDistance(rPt, rPtRes, dScale);
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
            return MMUtil::CalcDistance(rPt, rPtRes, dScale);
        }
    }

    // No orthogonal projection possible
    // -> Calc shortest distance to left or right point

    bIsOrthogonal = false;
    double dDistanceLeft = MMUtil::CalcDistance(rPt,
                                                rHalfSegment.GetLeftPoint(),
                                                dScale);
    double dDistanceRight = MMUtil::CalcDistance(rPt,
                                                 rHalfSegment.GetRightPoint(),
                                                 dScale);
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
                             const double dScale,
                             HalfSegment* pResHS)
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
                                              bOrthogonal, dScale);
            if (ResPointSeg.IsDefined() && dDistance < dShortestDistance)
            {
                dShortestDistance = dDistance;
                ResPoint = ResPointSeg;
                bIsOrthogonal = bOrthogonal;
                if (pResHS != NULL)
                    *pResHS = hs;
            }
        }
    }

    rdDistanceRes = dShortestDistance;
    return ResPoint;
}

static double CalcDistanceSimple(const Point& rPoint1, const Point& rPoint2)
{
    // TODO http://www.movable-type.co.uk/scripts/latlong.html

    const double dRadiusEarth = 6371.00; // km

    Point Point1Rad(true, degToRad(rPoint1.GetX()), degToRad(rPoint1.GetY()));
    Point Point2Rad(true, degToRad(rPoint2.GetX()), degToRad(rPoint2.GetY()));

    // Distance in meters
    return 1000 * acos(sin(Point1Rad.GetY()) * sin(Point2Rad.GetY()) +
                       cos(Point1Rad.GetY()) * cos(Point2Rad.GetY()) *
                       cos(Point1Rad.GetX() - Point2Rad.GetX())) * dRadiusEarth;
}

double MMUtil::CalcDistance(const Point& rPt1,
                            const Point& rPt2,
                            const double dScale)
{
//#define USE_GEOID
//#define USE_GEOID_PRECISE
#define USE_SIMPLE_DISTANCE

#ifdef USE_GEOID

    static Geoid s_Geoid(Geoid::WGS1984);

    if (AlmostEqual(dScale, 1.0))
    {
        bool bValid = true;
        double dDistance = rPt1.DistanceOrthodrome(rPt2,
                                                   s_Geoid,
                                                   bValid);
        assert(bValid);
        return dDistance;
    }
    else
    {
        Point Point1(rPt1);
        Point1.Scale(1.0 / dScale);
        Point Point2(rPt2);
        Point2.Scale(1.0 / dScale);
        bool bValid = true;
        double dDistance = Point1.DistanceOrthodrome(Point2,
                                                     s_Geoid,
                                                     bValid);
        assert(bValid);
        return dDistance;
    }

#elif defined USE_GEOID_PRECISE

    static Geoid s_Geoid(Geoid::WGS1984);

    if (AlmostEqual(dScale, 1.0))
    {
        bool bValid = true;
        double dInitBearing = 0.;
        double dFinalBearing = 0.;
        double dDistance = rPt1.DistanceOrthodromePrecise(rPt2,
                                                          s_Geoid,
                                                          bValid,
                                                          dInitBearing,
                                                          dFinalBearing);
        assert(bValid);
        return dDistance;
    }
    else
    {
        Point Point1(rPt1);
        Point1.Scale(1.0 / dScale);
        Point Point2(rPt2);
        Point2.Scale(1.0 / dScale);
        bool bValid = true;
        double dInitBearing = 0.;
        double dFinalBearing = 0.;
        double dDistance = Point1.DistanceOrthodromePrecise(Point2,
                                                            s_Geoid,
                                                            bValid,
                                                            dInitBearing,
                                                            dFinalBearing);
        assert(bValid);
        return dDistance;
    }

#elif defined USE_SIMPLE_DISTANCE

    if (AlmostEqual(dScale, 1.0))
    {
        return CalcDistanceSimple(rPt1, rPt2);
    }
    else
    {
        Point Point1(rPt1);
        Point1.Scale(1.0 / dScale);
        Point Point2(rPt2);
        Point2.Scale(1.0 / dScale);

        return CalcDistanceSimple(Point1, Point2);
    }

#else

    return rPt1.Distance(rPt2);

#endif
}

double MMUtil::CalcDistance(const std::vector<Point>& rvecPoints,
                            const double dScale)
{
    double dDistance = 0.0;

    const size_t nSize = rvecPoints.size();
    if (nSize > 0)
    {
        Point PtPrev = rvecPoints[0];

        for (size_t i = 1; i < nSize; ++i)
        {
            const Point& rPtAct = rvecPoints[i];
            if (rPtAct.IsDefined())
            {
                dDistance += MMUtil::CalcDistance(PtPrev, rPtAct, dScale);
                PtPrev = rPtAct;
            }
        }
    }

    return dDistance;
}

double MMUtil::CalcDistance(const Point& rPt1,
                            const Point& rPt2,
                            const SimpleLine& rCurve,
                            const double dScale)
{
    double dPos1 = -1.0;

    MMUtil::GetPosOnSimpleLine(rCurve,
                               rPt1,
                               rCurve.GetStartSmaller(),
                               0.000001 * dScale,
                               dPos1);

    double dPos2 = -1.0;
    MMUtil::GetPosOnSimpleLine(rCurve,
                               rPt2,
                               rCurve.GetStartSmaller(),
                               0.000001 * dScale,
                               dPos2);

    AttributePtr<SimpleLine> pSubline(new SimpleLine(0));
    rCurve.SubLine(dPos1, dPos2, *pSubline);

    return MMUtil::CalcLengthCurve(pSubline.get(),
                                   dScale);
}

double MMUtil::CalcLengthCurve(const GLine* pCurve,
                               const Network* pNetwork,
                               const double dScale)
{
    if (pCurve == NULL || !pCurve->IsDefined() ||
        pNetwork == NULL || !pNetwork->IsDefined())
        return 0.0;

    double dLen = 0.0;
    RouteInterval rI;
    AttributePtr<SimpleLine> pSubline(new SimpleLine(0));
    const int nNoOfComponents = pCurve->NoOfComponents();
    for (int i = 0; i < nNoOfComponents; ++i)
    {
        pCurve->Get(i,rI);
        pNetwork->GetLineValueOfRouteInterval(&rI, pSubline.get());
        if (pSubline->IsDefined())
        {
            dLen += MMUtil::CalcLengthCurve(pSubline.get(), dScale);
        }

        pSubline->Clear();
    }

    return dLen;
}

double MMUtil::CalcLengthCurve(const SimpleLine* pCurve,
                               const double dScale)
{
    if (pCurve == NULL || !pCurve->IsDefined())
        return 0.0;

#if 0

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

#else

    double dLength = 0.0;
    const int nHalfSegments = pCurve->Size();
    bool bValid = true;
    for (int i=0; bValid && i < nHalfSegments; ++i)
    {
        HalfSegment hs;
        pCurve->Get(i, hs);
        if( hs.IsLeftDomPoint())
        {
            dLength += MMUtil::CalcDistance(hs.GetLeftPoint(),
                                            hs.GetRightPoint(),
                                            dScale);
        }
    }
    return dLength;

#endif
}

double MMUtil::CalcHeading(const Point& rPt1,
                           const Point& rPt2,
                           bool bAtPt2,
                           double dScale)
{
    static Geoid s_Geoid(Geoid::WGS1984);

    if (AlmostEqual(dScale, 1.0))
    {
        return rPt1.Direction(rPt2, true, /*ReturnHeading*/
                              &s_Geoid, bAtPt2);
    }
    else
    {
        Point Point1(rPt1);
        Point1.Scale(1.0 / dScale);
        Point Point2(rPt2);
        Point2.Scale(1.0 / dScale);

        return Point1.Direction(Point2, true, /*ReturnHeading*/
                                &s_Geoid, bAtPt2);
    }
}

double MMUtil::CalcHeading(const IMMNetworkSection* pSection,
                           const HalfSegment& rHS,
                           double dScale)
{
    if (pSection == NULL)
    {
        assert(false);
        return 0.0;
    }

    Point Pt1 = rHS.GetLeftPoint();
    Point Pt2 = rHS.GetRightPoint();

    const SimpleLine* pCurve = pSection->GetCurve();

    LRS lrs;
    pCurve->Get(rHS.attr.edgeno, lrs);
    HalfSegment HS;
    pCurve->Get(lrs.hsPos, HS);

    double dPos1 = lrs.lrsPos + Pt1.Distance(HS.GetDomPoint());
    double dPos2 = lrs.lrsPos + Pt2.Distance(HS.GetDomPoint());

    if (!pSection->GetCurveStartsSmaller())
    {
        dPos1 = pCurve->Length() - dPos1;
        dPos2 = pCurve->Length() - dPos2;
    }

    if ((pSection->GetDirection() == IMMNetworkSection::DIR_UP &&
         dPos1 < dPos2) ||
         (pSection->GetDirection() == IMMNetworkSection::DIR_DOWN &&
         dPos2 < dPos1))
    {
        return MMUtil::CalcHeading(Pt1,
                                   Pt2,
                                   false /*AtEndPoint*/,
                                   dScale);
    }
    else
    {
        return MMUtil::CalcHeading(Pt2,
                                   Pt1,
                                   false /*AtEndPoint*/,
                                  dScale);
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

Point MMUtil::CalcDestinationPoint(const Point& rPoint,
                                   double dBearing,
                                   double dDistanceKM)
{
    const double dRadiusEarth = 6378.137;

    double dDistance = dDistanceKM / dRadiusEarth;  // angular distance
    dBearing = degToRad(dBearing);

    double dLat1 = degToRad(rPoint.GetY());
    double dLon1 = degToRad(rPoint.GetX());

    double dLat2 = asin(sin(dLat1) * cos(dDistance) +
                        cos(dLat1) * sin(dDistance) * cos(dBearing));

    double dLon2 = dLon1 + atan2(sin(dBearing) * sin(dDistance) * cos(dLat1),
                                 cos(dDistance) - sin(dLat1) * sin(dLat2));

    return Point(true, radToDeg(dLon2), radToDeg(dLat2));
}


void MMUtil::SubLine(const SimpleLine* pLine,
                     const Point& rPoint1,
                     const Point& rPoint2,
                     bool bStartsSmaller,
                     double dScale,
                     SimpleLine& rSubLine)
{
    if (pLine == NULL)
    {
        rSubLine.SetDefined(false);
        return;
    }

    double dPos1 = -1.0;
    MMUtil::GetPosOnSimpleLine(*pLine,
                               rPoint1,
                               bStartsSmaller,
                               0.000001 * dScale,
                               dPos1);

    double dPos2 = -1.0;
    MMUtil::GetPosOnSimpleLine(*pLine,
                               rPoint2,
                               bStartsSmaller,
                               0.000001 * dScale,
                               dPos2);

    pLine->SubLine(dPos1, dPos2,bStartsSmaller, rSubLine);
}



} // end of namespace mapmatch


