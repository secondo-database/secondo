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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//[TOC] [\tableofcontents]

[1] Header file of the LineFunctionAlgebra

April 2015 - Rene Steinbrueck

1 Overview

With LineFunctionAlgebra , it is possible to describe the height depending on 
the distance traveled.

[TOC]

2 Defines, includes, and constants

*/
#ifndef LINEFUNCTION_ALGEBRA_H
#define LINEFUNCTION_ALGEBRA_H

#include "LMapping.h"
# include "ConstLengthUnit.h"
#include "LUReal.h"
#include "./../Raster2/sbool.h"
#include "./../Raster2/sint.h"
#include "./../Raster2/sstring.h"
#include "./../Raster2/sreal.h"
#include "./../Raster2/util/types.h"
#include "./../Raster2/Operators/CellIterator.h"
#include "./../Spatial/Point.h"
#include "./../Spatial/HalfSegment.h"
#include "./../Spatial/SpatialAlgebra.h"
#include "./../FText/FTextAlgebra.h"

double DistanceWithHeight(const Point&,const Point&, CcReal&, LReal&,
 const Geoid*);
double HeightDifference(LReal) ;


/*
0 Functions for Operators

0.1 getHeightAtPosition

*/
template <typename T>
double getHeightAtPosition
    (const double xcoord, const double ycoord, const T& raster)
{
    Point P, TL, TC, TR, CL, CC, CR, BL, BC, BR;
    double glength = raster.getGrid().getLength();
    double ox = raster.getGrid().getOriginX();
    double oy = raster.getGrid().getOriginY();

    P.Set (xcoord, ycoord);
    CC.Set (floor((xcoord-ox)/glength) * glength + ox + glength/2,
            floor((ycoord-oy)/glength) * glength + oy + glength/2);

    CL.Set (CC.GetX () - glength, CC.GetY ());
    CR.Set (CC.GetX () + glength, CC.GetY ());
    TL.Set (CC.GetX () - glength, CC.GetY () + glength);
    TC.Set (CC.GetX (), CC.GetY () + glength);
    TR.Set (CC.GetX () + glength, CC.GetY () + glength);
    BL.Set (CC.GetX () - glength, CC.GetY () - glength);
    BC.Set (CC.GetX (), CC.GetY () - glength);
    BR.Set (CC.GetX () + glength, CC.GetY () - glength);

    if (P==CC)
    {
    double res = raster.atlocation(P.GetX (), P.GetY ());
  return (raster.isUndefined(res) ? raster2::UNDEFINED_REAL : res);
    }

    Point A = CC;
    Point B = CL;
    Point C = BC;

    double DistA = P.Distance(A);
    double DistB = P.Distance(B);
    double DistC = P.Distance(C);

    double DistTmp = P.Distance(TL);
    bool undefined =
        raster.isUndefined(raster.atlocation(TL.GetX(), TL.GetY()));

    if (!undefined)
    {
        if (DistTmp < DistB)
        {
            if (DistB < DistC)
            {
                DistC=DistB;
                C=B;
            }
            DistB = DistTmp;
            B = TL;
        }
        else if (DistTmp < DistC)
        {
            DistC = DistTmp;
            C = TL;
        }
    }

    DistTmp = P.Distance(TC);
    undefined = raster.isUndefined(raster.atlocation(TC.GetX(), TC.GetY()));

    if (!undefined)
    {
        if (DistTmp < DistB)
        {
            if (DistB < DistC)
            {
                DistC=DistB;
                C=B;
            }
            DistB = DistTmp;
            B = TC;
        }
        else if (DistTmp < DistC)
        {
            DistC = DistTmp;
            C = TC;
        }
    }

    DistTmp = P.Distance(TR);
    undefined = raster.isUndefined(raster.atlocation(TR.GetX(), TR.GetY()));

    if (!undefined)
    {
        if (DistTmp < DistB)
        {
            if (DistB < DistC)
            {
                DistC=DistB;
                C=B;
            }
            DistB = DistTmp;
            B = TR;
        }
        else if (DistTmp < DistC)
        {
            DistC = DistTmp;
            C = TR;
        }
    }

    DistTmp = P.Distance(BL);
    undefined = raster.isUndefined(raster.atlocation(BL.GetX(),BL.GetY()));

    if (!undefined)
    {
        if (DistTmp < DistB)
        {
            if (DistB < DistC)
            {
                DistC=DistB;
                C=B;
            }
            DistB = DistTmp;
            B = BL;
        }
        else if (DistTmp < DistC)
        {
            DistC = DistTmp;
            C = BL;
        }
    }

    DistTmp = P.Distance(BR);
    undefined = raster.isUndefined(raster.atlocation(BR.GetX(), BR.GetY()));

    if (!undefined)
    {
        if (DistTmp < DistB)
        {
            if (DistB < DistC)
            {
                DistC=DistB;
                C=B;
            }
            DistB = DistTmp;
            B = BR;
        }
        else if (DistTmp < DistC)
        {
            DistC = DistTmp;
            C = BR;
        }
    }

    DistTmp = P.Distance(CR);
    undefined = raster.isUndefined(raster.atlocation(CR.GetX(), CR.GetY()));

    if (!undefined)
    {
        if (DistTmp < DistB)
        {
            if (DistB < DistC)
            {
                DistC=DistB;
                C=B;
            }
            DistB = DistTmp;
            B = CR;
        }
        else if (DistTmp < DistC)
        {
            DistC = DistTmp;
            C = CR;
        }
    }

    double DistAB = A.Distance(B);
    double DistAC = A.Distance(C);
    double DistBC = B.Distance(C);


    //http://www.arndt-bruenner.de/mathe/9/herondreieck.htm
    // whole triangle
    double s = (DistAB + DistAC + DistBC) / 2;
    double A_whole = sqrt (s * (s - DistAB)*(s - DistAC)*(s - DistBC));

    //alpha
    s = (DistB + DistC + DistBC) / 2;
    double A_alpha = sqrt (s * (s - DistB)*(s - DistC)*(s - DistBC));
    double alpha = A_alpha / A_whole;

    //beta
    s = (DistA + DistC + DistAC) / 2;
    double A_beta = sqrt (s * (s - DistA)*(s - DistC)*(s - DistAC));
    double beta = A_beta / A_whole;

    //gamma
    s = (DistA + DistB + DistAB) / 2;
    double A_gamma = sqrt (s * (s - DistA)*(s - DistB)*(s - DistAB));
    double gamma = A_gamma / A_whole;


    double valueA = raster.atlocation(A.GetX (), A.GetY ());
    double valueB = raster.atlocation(B.GetX (), B.GetY ());
    double valueC = raster.atlocation(C.GetX (), C.GetY ());

    bool undefinedA =
        raster.isUndefined(raster.atlocation(A.GetX (), A.GetY ()));
    bool undefinedB =
        raster.isUndefined(raster.atlocation(B.GetX (), B.GetY ()));
    bool undefinedC =
        raster.isUndefined(raster.atlocation(C.GetX (), C.GetY ()));

    if (undefinedA && !undefinedB && !undefinedC)
        return (beta/(beta+gamma)*valueB+gamma/(beta+gamma)*valueC);
    if (!undefinedA && undefinedB && !undefinedC)
        return (alpha/(alpha+gamma)*valueA+gamma/(alpha+gamma)*valueC);
    if (!undefinedA && !undefinedB && undefinedC)
        return (alpha/(alpha+beta)*valueA+beta/(alpha+beta)*valueB);
    if (undefinedA && undefinedB && !undefinedC)
        return (valueC);
    if (undefinedA && !undefinedB && undefinedC)
        return (valueB);
    if (!undefinedA && undefinedB && undefinedC)
        return (valueA);
    if (undefinedA && undefinedB && undefinedC)
        return (raster2::UNDEFINED_REAL);
    return ((alpha * valueA) + (beta * valueB) + (gamma * valueC));
}

/*
1 Operators

1.1 Operator ~heightatposition~

*/

struct heightatpositionInfo : OperatorInfo
{
    heightatpositionInfo ()
    {
        name      = "heightatposition";
        signature =   raster2::sint::BasicType () 
      + " heightatposition "
                      + Point::BasicType ()
                      + " -> real";
        appendSignature (  
      raster2::sreal::BasicType () 
      + " heightatposition "
                         + Point::BasicType ()
                         + " -> real");

        syntax    = "_ heightatposition _ ";
        meaning   =   "This function returns the interpolated "
      "height at the raster position.";
    }
};

template <typename T>
int heightatpositionFun(Word* args, Word& result,
                        int message, Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    CcReal* pResult = static_cast<CcReal*>(result.addr);

    Point* pPoint = static_cast<Point*>(args[1].addr);

    T* praster = static_cast<T*>(args[0].addr);

    if(!pPoint->IsDefined() || !praster->isDefined())
    {
        pResult->SetDefined(false);
    }
    else
    {
        (*pResult) = raster2::sreal::wrap(getHeightAtPosition(pPoint->GetX(),
                                          pPoint->GetY(),
                                          *praster));
    }
    return 0;
}



#endif // LINEFUNCTION_ALGEBRA_H
