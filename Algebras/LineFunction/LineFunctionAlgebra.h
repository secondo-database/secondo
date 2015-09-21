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

With LineFunctionAlgebra , it is possible to describe the height depending on the distance traveled.

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
        signature = 	raster2::sint::BasicType () 
			+ " heightatposition "
                    	+ Point::BasicType ()
                    	+ " -> real";
        appendSignature (	
			raster2::sreal::BasicType () 
			+ " heightatposition "
                       	+ Point::BasicType ()
                       	+ " -> real");

        syntax    = "_ heightatposition _ ";
        meaning   = 	"This function returns the interpolated "
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


ValueMapping heightatpositionFuns[] =
{
    heightatpositionFun<raster2::sint>,
    heightatpositionFun<raster2::sreal>,
    0
};


int heightatpositionSelectFun(ListExpr args)
{
    NList type(args);

    if (type.first().isSymbol(raster2::sint::BasicType()))
    {
        return (0);
    };
    if (type.first().isSymbol(raster2::sreal::BasicType()))
    {
        return (1);
    };
 return (-1);
}

ListExpr heightatpositionTypeMap(ListExpr args)
{
    if(!nl->HasLength(args,2))
    {
        return (listutils::typeError("2 arguments expected"));
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    string err = "stype x point expected";

        if(!raster2::util::isSType(arg1))
            return (listutils::typeError(err + " (first arg is not an stype)"));
        if(!Point::checkType(arg2))
            return (listutils::typeError(err
                + " (second arg is not an point)"));

    return (nl->SymbolAtom(CcReal::BasicType()));
}

/*
1.2 Operator ~lcompose~

*/

struct lcomposeInfo : OperatorInfo
{
    lcomposeInfo()
    {
    name      = "lcompose";
    signature = raster2::sbool::BasicType()
        + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lbool";
    appendSignature(raster2::sreal::BasicType() + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lreal");
    appendSignature(raster2::sint::BasicType() + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lint");
    appendSignature(raster2::sstring::BasicType() + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lstring");

    syntax    = "_ lcompose [_,_]";
    meaning   = "This function merges sline, sT and boolean into lT. "
		"If the bool parameter is TRUE, the Distance between "
		"two points on the line is calculated by orthodrome "
		"distance, otherwise by the euclidean distance.";
    }
};

template <typename ST, typename UT, typename RT>
int constlcomposeFun
    (Word* args, Word& result, int message, Word& local, Supplier s)
{
    // storage for the result
    result = qp->ResultStorage(s);

    // the sT object
    ST* raster = static_cast<ST*>(args[0].addr);

    // the simple Line
    SimpleLine* simpleLine = static_cast<SimpleLine*>(args[1].addr);

    // the computation method for distance
    //(true = Distance Orthodrome, false = Euclidean Distance)
    CcBool* distCalc = static_cast<CcBool*>(args[2].addr);

    // The result of the lcompose
    RT* pResult = static_cast<RT*>(result.addr);


    if (!simpleLine->IsDefined() || !raster->isDefined())
    {
            pResult->SetDefined(false);
            return 0;
    }

    pResult->Clear();
    pResult->StartBulkLoad();

    // get the number of components
    int num = simpleLine->Size();


    HalfSegment unit;
    raster2::grid2 grid = raster->getGrid();
    raster2::grid2::index_type cell1;
    raster2::grid2::index_type cell2;
    double start = 0.0;
    double end = 0.0;
    bool definedGeoID;
    bool distOrthodrome = distCalc->GetBoolval();

    if (simpleLine->StartsSmaller())
    {
        for (int i = 0; i < num; i++)
        {
            simpleLine->Get(i,unit);
            if (unit.IsLeftDomPoint())
            {
                // get the coordinates
                double xStart = unit.GetLeftPoint().GetX();
                double yStart = unit.GetLeftPoint().GetY();
                double xEnd = unit.GetRightPoint().GetX();
                double yEnd = unit.GetRightPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetLeftPoint().DistanceOrthodrome(
                    unit.GetRightPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetLeftPoint().Distance(unit.GetRightPoint());
                end = start + distance;


                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);

                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end),true, (i==(num-2))?true:false);

                    typename ST::cell_type v1 = raster->atlocation(
                        (xEnd-xStart)/2,(yEnd-yStart)/2);
                    if(!raster->isUndefined(v1))
                    {
                        typename ST::wrapper_type v(v1);
                        pResult->MergeAdd(UT(linterval,v));
                    }
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);

                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;
                    while(it.hasNext())
                    {
                        pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);
                        if(e>s)
                        {
                            LInterval linterval(CcReal(s),
                                CcReal(e),true,(i==(num-2))?true:false);
                            double delta  =(p.first + p.second) / 2.0;
                            double x = xStart + delta*dx;
                            double y = yStart + delta*dy;
                            typename ST::cell_type v1 = raster->atlocation(x,y);
                            if(!raster->isUndefined(v1))
                            {
                                typename ST::wrapper_type v(v1);
                                pResult->MergeAdd(UT(linterval,v));
                            }
                        }
                        else
                        assert(e==s);
                    }
                }

            }
        }
    }
    else
    {
        for (int i = num-1; i >=0; i--)
        {
            simpleLine->Get(i,unit);
            if (!unit.IsLeftDomPoint())
            {

                // get the coordinates
                double xStart = unit.GetRightPoint().GetX();
                double yStart = unit.GetRightPoint().GetY();
                double xEnd = unit.GetLeftPoint().GetX();
                double yEnd = unit.GetLeftPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetRightPoint().DistanceOrthodrome(
                    unit.GetLeftPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetRightPoint().Distance(unit.GetLeftPoint());
                end = start + distance;


                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);

                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end),true, (i==0)?true:false);

                    typename ST::cell_type v1 =
                        raster->atlocation((xEnd-xStart)/2,(yEnd-yStart)/2);
                    if(!raster->isUndefined(v1))
                    {
                        typename ST::wrapper_type v(v1);
                        pResult->MergeAdd(UT(linterval,v));
                    }
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);

                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;
                    while(it.hasNext())
                    {
                        pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);
                        if(e>s)
                        {
                            LInterval linterval(CcReal(s),
                                CcReal(e),true,(i==0)?true:false);
                            double delta  =(p.first + p.second) / 2.0;
                            double x = xStart + delta*dx;
                            double y = yStart + delta*dy;
                            typename ST::cell_type v1 = raster->atlocation(x,y);
                            if(!raster->isUndefined(v1))
                            {
                                typename ST::wrapper_type v(v1);
                                pResult->MergeAdd(UT(linterval,v));
                            }
                        }
                        else
                        assert(e==s);
                    }
                }

            }
        }
    }
    pResult->EndBulkLoad();
    return 0;
}



template <typename ST>
int reallcomposeFun
    (Word* args, Word& result, int message, Word& local, Supplier s)
{
    // storage for the result
    result = qp->ResultStorage(s);

    // the sT object
    ST* raster = static_cast<ST*>(args[0].addr);

    // the simple Line
    SimpleLine* simpleLine = static_cast<SimpleLine*>(args[1].addr);

    // the computation method for distance
    //(true = Distance Orthodrome, false = Euclidean Distance)
    CcBool* distCalc = static_cast<CcBool*>(args[2].addr);

    // The result of the lcompose
    LReal* pResult = static_cast<LReal*>(result.addr);


    if (!simpleLine->IsDefined() || !raster->isDefined())
    {
            pResult->SetDefined(false);
            return 0;
    }

    pResult->Clear();
    pResult->StartBulkLoad();

    // get the number of components
    int num = simpleLine->Size();

    HalfSegment unit;
    raster2::grid2 grid = raster->getGrid();
    raster2::grid2::index_type cell1;
    raster2::grid2::index_type cell2;
    double start = 0.0;
    double end = 0.0;
    bool definedGeoID;
    bool distOrthodrome = distCalc->GetBoolval();

    if (simpleLine->StartsSmaller())
    {
        for (int i = 0; i < num; i++)
        {
            simpleLine->Get(i,unit);
            if (unit.IsLeftDomPoint())
            {
                // get the coordinates
                double xStart = unit.GetLeftPoint().GetX();
                double yStart = unit.GetLeftPoint().GetY();
                double xEnd = unit.GetRightPoint().GetX();
                double yEnd = unit.GetRightPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetLeftPoint().DistanceOrthodrome(
                    unit.GetRightPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetLeftPoint().Distance(unit.GetRightPoint());
                end = start + distance;
                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);


                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end), true, (i==(num-2))?true:false);
                    double v1 = getHeightAtPosition<ST>(xStart,yStart,*raster);
                    double v2 = getHeightAtPosition<ST>(xEnd,yEnd,*raster);
                    if(!raster->isUndefined(v1) && !raster->isUndefined(v2))
                        pResult->MergeAdd(LUReal(linterval,v1,v2,false));
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);
                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;

                    while(it.hasNext())
                    {
                        pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);

                        if(e>s)
                        {
                            LInterval linterval(CcReal(s),
                                CcReal(e),true,(i==(num-2))?true:false);
                            double xs = xStart + p.first * dx;
                            double ys = yStart + p.first * dy;
                            double xe = xStart + p.second * dx;
                            double ye = yStart + p.second * dy;
                            double v1 = getHeightAtPosition<ST>(xs,ys,*raster);
                            double v2 = getHeightAtPosition<ST>(xe,ye,*raster);
                            if(!raster->isUndefined(v1) &&
                                !raster->isUndefined(v2))
                                pResult->MergeAdd(
                                    LUReal(linterval,v1,v2,false));
                        }
                        else
                        assert(e==s);
                    }
                }
            }
        }
    }
    else
    {
        for (int i = num-1; i >=0; i--)
        {
            simpleLine->Get(i,unit);
            if (!unit.IsLeftDomPoint())
            {
                // get the coordinates
                double xStart = unit.GetLeftPoint().GetX();
                double yStart = unit.GetLeftPoint().GetY();
                double xEnd = unit.GetRightPoint().GetX();
                double yEnd = unit.GetRightPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetLeftPoint().DistanceOrthodrome(
                    unit.GetRightPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetLeftPoint().Distance(unit.GetRightPoint());
                end = start + distance;
                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);


                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end), true, (i==0)?true:false);
                    double v1 = getHeightAtPosition<ST>(xStart,yStart,*raster);
                    double v2 = getHeightAtPosition<ST>(xEnd,yEnd,*raster);
                    if(!raster->isUndefined(v1) && !raster->isUndefined(v2))
                        pResult->MergeAdd(LUReal(linterval,v1,v2,false));
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);
                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;

                    while(it.hasNext())
                    {
                        pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);

                        if(e>s)
                        {
                            LInterval linterval(
                                CcReal(s),CcReal(e),true,(i==0)?true:false);
                            double xs = xStart + p.first * dx;
                            double ys = yStart + p.first * dy;
                            double xe = xStart + p.second * dx;
                            double ye = yStart + p.second * dy;
                            double v1 = getHeightAtPosition<ST>(xs,ys,*raster);
                            double v2 = getHeightAtPosition<ST>(xe,ye,*raster);
                            if(!raster->isUndefined(v1)
                                && !raster->isUndefined(v2))
                                pResult->MergeAdd(
                                    LUReal(linterval,v1,v2,false));
                        }
                        else
                        assert(e==s);
                    }
                }
            }
        }
    }

    pResult->EndBulkLoad();

return 0;
}


ValueMapping lcomposeFuns[] =
{
    constlcomposeFun<raster2::sbool, LUBool, LBool>,
    constlcomposeFun<raster2::sstring, LUString, LString>,
    reallcomposeFun<raster2::sint>,
    reallcomposeFun<raster2::sreal>,
    0
};


int lcomposeSelectFun(ListExpr args)
{
string errmsg = "lcomposeSelectFun started";

    NList type(args);

        if (type.first().isSymbol(raster2::sbool::BasicType()))
            return 0;
        if (type.first().isSymbol(raster2::sstring::BasicType()))
            return 1;
        if (type.first().isSymbol(raster2::sint::BasicType()))
            return 2;
        if(type.first().isSymbol(raster2::sreal::BasicType()))
            return 3;
        return -1; //this point should never be reached
}

ListExpr lcomposeTypeMap(ListExpr args)
{
string errmsg = "lcomposeTypeMap started";

    if(!nl->HasLength(args,3))
        return listutils::typeError("3 arguments expected");

    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    string err = "stype x sline x bool expected";
    if(!raster2::util::isSType(arg1))
        return listutils::typeError(err + " (first arg is not an stype)");
    if(!SimpleLine::checkType(arg2))
        return listutils::typeError(err + " (second arg is not an sline)");
    if(!CcBool::checkType(arg3))
        return listutils::typeError(err + " (third arg is not an bool)");

    std::string sname = nl->SymbolValue(arg1);
    if (sname==raster2::sbool::BasicType())
        return (nl->SymbolAtom(LBool::BasicType()));
    if (sname==raster2::sstring::BasicType())
        return (nl->SymbolAtom(LString::BasicType()));
    if (sname==raster2::sreal::BasicType())
        return (nl->SymbolAtom(LReal::BasicType()));
    if (sname==raster2::sint::BasicType())
        return (nl->SymbolAtom(LReal::BasicType()));
    return listutils::typeError();
}


/*
1.3 Operator ~lfdistance~

*/

struct lfdistanceInfo : OperatorInfo
    {
		lfdistanceInfo()
		{
        name      = "lfdistance";
        signature = "lfdistance ("
        	+ Point::BasicType()
        	+ " , "
        	+ Point::BasicType()
            	+ ", "
        	+ raster2::sint::BasicType()
        	+ " , "
        	+ CcInt::BasicType()
        	+ "-> Real";
        appendSignature("lfdistance ("
            	+ Point::BasicType()
            	+ " , "
            	+ Point::BasicType()
                + " , "
            	+ raster2::sreal::BasicType()
            	+ " , "
                + CcInt::BasicType()
                + "-> Real");

        syntax    = 	"lfdistance ( _ , _ , _ , _ )";
        meaning   = 	"This function computes the distance "
			"between two points including their height. "
			"The First two parameters are "
		    	"the two points, between the distance "
			"should be calculated "
		    	"and the third parameter is a raster, "
			"from which the height "
			"could be get. "
			"The fourth parameter is 1 or 2: "
			"1 for computing the pure "
			"driving distance or "
			"2 for computing the driving distance, "
			"depending on the gradient.";
      }
    };

template<typename Raster> int lfdistanceFun
(Word* args, Word& result, int message, Word& local, Supplier s)
{
	result = qp->ResultStorage(s);
	Point* a = static_cast<Point*>(args[0].addr);
	Point* b = static_cast<Point*>(args[1].addr);
	Raster* raster = static_cast<Raster*>(args[2].addr);
	CcInt* suchart = static_cast<CcInt*>(args[3].addr);
	CcReal* res = static_cast<CcReal*>(result.addr);

    bool checkCoord;

	if(!a->IsDefined() || !b->IsDefined() || a->IsEmpty() || b->IsEmpty() ||
       !suchart->IsDefined() || !( suchart->GetIntval() == 1 ||
            suchart->GetIntval() == 2 ) )
	{
		res->SetDefined(false);
	}
	else
	{
        if (suchart->GetIntval() == 1)
        {
            double airDistance =
                a->DistanceOrthodrome(*b,Geoid(true),checkCoord);
            double heightA=getHeightAtPosition(a->GetX(),a->GetY(),*raster);
            double heightB=getHeightAtPosition(b->GetX(),b->GetY(),*raster);
            double dheight=heightB-heightA;
            double drivingDistance=sqrt(pow(dheight,2)+pow(airDistance,2));
            res->Set(true, drivingDistance);
        }
        if (suchart->GetIntval() == 2)
        {
            double airDistance =
                a->DistanceOrthodrome(*b,Geoid(true),checkCoord);
            double heightA=getHeightAtPosition(a->GetX(),a->GetY(),*raster);
            double heightB=getHeightAtPosition(b->GetX(),b->GetY(),*raster);
            double dheight=heightB-heightA;
            double drivingDistance=sqrt(pow(dheight,2)+pow(airDistance,2));
            double climb = dheight/airDistance;
            double dist;


            if ((climb < (-1.0))||(climb > 1.0))
                res->SetDefined(false);
            else if ((climb >= -1.00)&&(climb <= -0.10))
                dist = drivingDistance *  0.30;
			else if ((climb >  -0.10)&&(climb <  -0.01))
                dist = drivingDistance *  0.60;
			else if ((climb >= -0.01)&&(climb <=  0.01))
                dist = drivingDistance *  1.00;
			else if ((climb >   0.01)&&(climb <   0.02))
                dist = drivingDistance *  1.25;
			else if ((climb >=  0.02)&&(climb <   0.05))
                dist = drivingDistance *  1.75;
			else if ((climb >=  0.05)&&(climb <   0.10))
                dist = drivingDistance *  2.75;
			else if ((climb >=  0.10)&&(climb <=  1.00))
                dist = drivingDistance * 20.00;

            res->Set(true,dist);
        }
    }
	return (0);
}


ValueMapping lfdistanceFuns[] =
{
		lfdistanceFun<raster2::sint>,
		lfdistanceFun<raster2::sreal>,
		0
};


int lfdistanceSelectFun(ListExpr args)
{
	NList type(args);

	if (type.third().isSymbol(raster2::sint::BasicType()))
	{
		return (0);
	}
	if(type.third().isSymbol(raster2::sreal::BasicType()))
	{
		return (1);
	}
	return (-1);
}

ListExpr lfdistanceTypeMap(ListExpr args)
{
	if(!nl->HasLength(args,4)){
		return (listutils::typeError("4 arguments expected"));
	}
	ListExpr arg1 = nl->First(args);
	ListExpr arg2 = nl->Second(args);
	ListExpr arg3 = nl->Third(args);
	ListExpr arg4 = nl->Fourth(args);

	string err = "Point x Point x stype x int expected";
	if(!Point::checkType(arg1))
	{
		return (listutils::typeError(err
            + " (first arg is not an point)"));
	}
	if(!Point::checkType(arg2))
	{
		return (listutils::typeError(err
            + " (second arg is not an point)"));
	}
	if(!raster2::util::isSType(arg3)){
		return (listutils::typeError(err
            + " (third arg is not an stype)"));
	}
	if(!CcInt::checkType(arg4))
	{
		return (listutils::typeError(err
            + " (fourth arg is not an int)"));
	}
	return (listutils::basicSymbol<CcReal>());
}

/*
1.4 Operator ~lfdistanceparam~

*/

struct lfdistanceparamInfo : OperatorInfo
    {
		lfdistanceparamInfo()
		{
        name      = "lfdistanceparam";
        signature = "lfdistanceparam ("
        	+ Point::BasicType()
        	+ " , "
        	+ Point::BasicType()
            	+ " , "
        	+ raster2::sint::BasicType()
            	+ " , "
            	+ CcString::BasicType()
            	+ " , "
            	+ CcString::BasicType()
            	+ " , "
            	+ CcString::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ "-> Real )";
        appendSignature("lfdistanceparam ("
            	+ Point::BasicType()
            	+ " , "
            	+ Point::BasicType()
            	+ " , "
            	+ raster2::sreal::BasicType()
            	+ " , "
            	+ CcString::BasicType()
            	+ " , "
            	+ CcString::BasicType()
            	+ " , "
            	+ CcString::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ " , "
            	+ CcReal::BasicType()
            	+ "-> Real )");

        syntax    = "lfdistanceparam ( _ , _ , _ , _ , _ , _ , "
            "_ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ )";
        meaning   = 	"This function computes the distance "
			"between two points including their height. "
                   	"The First two parameters are the two points, "
                   	"between the distance should be calculated "
			"and the third parameter is a raster, "
			"from which the height could be get. "
                   	"Parameter fith, six and seven is used "
                   	"to pass the sort of way, the dependence to "
			"a cycle route and the surface. "
                   	"With the last parameters the dependence "
			"on the sort of way, "
                   	"the gradient and the surface can be set. "
                   	"Put in Zero for default value.";

      }
    };

template<typename Raster> int lfdistanceparamFun
(Word* args, Word& result, int message, Word& local, Supplier s)
{
	result = qp->ResultStorage(s);
	Point* a = static_cast<Point*>(args[0].addr);
	Point* b = static_cast<Point*>(args[1].addr);
	Raster* raster = static_cast<Raster*>(args[2].addr);
	CcString* roadType = static_cast<CcString*>(args[3].addr);
	CcString* routePart = static_cast<CcString*>(args[4].addr);
	CcString* surface = static_cast<CcString*>(args[5].addr);
	CcReal* w1 = static_cast<CcReal*>(args[6].addr);
	CcReal* w2 = static_cast<CcReal*>(args[7].addr);
	CcReal* w3 = static_cast<CcReal*>(args[8].addr);
	CcReal* w4 = static_cast<CcReal*>(args[9].addr);
	CcReal* s1 = static_cast<CcReal*>(args[10].addr);
	CcReal* s2 = static_cast<CcReal*>(args[11].addr);
	CcReal* s3 = static_cast<CcReal*>(args[12].addr);
	CcReal* s4 = static_cast<CcReal*>(args[13].addr);
	CcReal* s5 = static_cast<CcReal*>(args[14].addr);
	CcReal* s6 = static_cast<CcReal*>(args[15].addr);
	CcReal* s7 = static_cast<CcReal*>(args[16].addr);
	CcReal* o1 = static_cast<CcReal*>(args[17].addr);
	CcReal* o2 = static_cast<CcReal*>(args[18].addr);
	CcReal* o3 = static_cast<CcReal*>(args[19].addr);
	CcReal* o4 = static_cast<CcReal*>(args[20].addr);
	CcReal* res = static_cast<CcReal*>(result.addr);

    bool checkCoord;

    double wayFactor;
    if ((routePart->toText()=="yes") && (w1->GetRealval()!=0.0))
        wayFactor = w1->GetRealval();
    else if ((routePart->toText()=="yes") && (w1->GetRealval()==0.0))
        wayFactor = 0.5;
    else if ((roadType->toText()=="cycleway") && (w2->GetRealval()!=0.0))
        wayFactor = w2->GetRealval();
    else if ((roadType->toText()=="cycleway") && (w2->GetRealval()==0.0))
        wayFactor = 0.5;
    else if ((roadType->toText()=="track" ||
        roadType->toText() == "path") && (w3->GetRealval()!=0.0))
        wayFactor = w3->GetRealval();
    else if ((roadType->toText()=="trunk" ||
            roadType->toText()=="primary" ||
            roadType->toText()=="secondary" ||
            roadType->toText()=="tertiary" ||
            roadType->toText()=="unclassified" ||
            roadType->toText()=="residential" ||
            roadType->toText()=="trunk")
            && (w4->GetRealval()!=0.0))
            wayFactor = w4->GetRealval();
    else if ((roadType->toText()=="trunk" ||
            roadType->toText()=="primary" ||
            roadType->toText()=="secondary" ||
            roadType->toText()=="tertiary" ||
            roadType->toText()=="unclassified" ||
            roadType->toText()=="residential" ||
            roadType->toText()=="trunk") && (w4->GetRealval()==0.0))
            wayFactor = 2.0;
    else wayFactor = 1.0;

    double surfaceFactor;
    if ((surface->toText()=="paved" || surface->toText()=="asphalt" ||
         surface->toText()=="concrete") && (o1->GetRealval() != 0.0))
         surfaceFactor = o1->GetRealval();
    else if ((surface->toText()=="cobblestone" ||
            surface->toText()=="cobblestone:flattened" ||
            surface->toText()=="sett" ||
            surface->toText()=="concrete:lanes" ||
            surface->toText()=="concrete:plates" ||
            surface->toText()=="paving_stones" ||
            surface->toText()=="metal" ||
            surface->toText()=="wood" ||
            surface->toText()=="paving_stones:30" ||
            surface->toText()=="paving_stones:20" )
            && (o2->GetRealval()!=0.0))
            surfaceFactor = o2->GetRealval();
    else if ((surface->toText()=="unpaved" ||
            surface->toText()=="compacted" ||
            surface->toText()=="dirt" ||
            surface->toText()=="earth" ||
            surface->toText()=="grass" ||
            surface->toText() == "grass_paver" ||
            surface->toText()=="ground" ||
            surface->toText()=="mud" ||
            surface->toText()=="ice" ||
            surface->toText()=="salt" ||
            surface->toText()=="sand" ||
            surface->toText()=="snow"||
            surface->toText()=="woodchips" ) && (o3->GetRealval()!=0.0))
            surfaceFactor = o3->GetRealval();
        else if ((surface->toText()=="gravel" ||
            surface->toText()=="pebblestone" )
            && (o4->GetRealval()!=0.0))
              surfaceFactor = o4->GetRealval();
    else if ((surface->toText()=="gravel" ||
            surface->toText()=="pebblestone" )
            && (o4->GetRealval()==0.0))
              surfaceFactor = 3.0;
    else surfaceFactor = 1.0;


	if(!a->IsDefined() || !b->IsDefined() || a->IsEmpty() || b->IsEmpty() )
	{
		res->SetDefined(false);
	}
	else
	{
        double airDistance = a->DistanceOrthodrome(*b,Geoid(true),checkCoord);
        double heightA=getHeightAtPosition(a->GetX(),a->GetY(),*raster);
        double heightB=getHeightAtPosition(b->GetX(),b->GetY(),*raster);
        double dheight=heightB-heightA;
        double drivingDistance=sqrt(pow(dheight,2)+pow(airDistance,2));
        double climb = dheight/airDistance;
        double dist;


            if ((climb < (-1.0))||(climb > 1.0)) res->SetDefined(false);
            else if ((climb >= -1.00)&&(climb <= -0.10))
                dist = drivingDistance *
                    (s1->GetRealval()!=0 ? s1->GetRealval() : 0.30);
			else if ((climb >  -0.10)&&(climb <  -0.01))
                dist = drivingDistance *
                    (s2->GetRealval()!=0 ? s2->GetRealval() : 0.60);
			else if ((climb >= -0.01)&&(climb <=  0.01))
                dist = drivingDistance *
                    (s3->GetRealval()!=0 ? s3->GetRealval() : 1.00);
			else if ((climb >   0.01)&&(climb <   0.02))
                dist = drivingDistance *
                    (s4->GetRealval()!=0 ? s4->GetRealval() : 1.25);
			else if ((climb >=  0.02)&&(climb <   0.05))
                dist = drivingDistance *
                    (s5->GetRealval()!=0 ? s5->GetRealval() : 1.75);
			else if ((climb >=  0.05)&&(climb <   0.10))
                dist = drivingDistance *
                    (s6->GetRealval()!=0 ? s6->GetRealval() : 2.75);
			else if ((climb >=  0.10)&&(climb <=  1.00))
                dist = drivingDistance *
                    (s7->GetRealval()!=0 ? s7->GetRealval() : 20.00);

            dist = dist * wayFactor * surfaceFactor;
            res->Set(true,dist);
        }
	return (0);
}


ValueMapping lfdistanceparamFuns[] =
{
		lfdistanceparamFun<raster2::sint>,
		lfdistanceparamFun<raster2::sreal>,
		0
};


int lfdistanceparamSelectFun(ListExpr args)
{
	NList type(args);

	if (type.third().isSymbol(raster2::sint::BasicType()))
	{
		return (0);
	}
	if(type.third().isSymbol(raster2::sreal::BasicType()))
	{
		return (1);
	}
	return (-1);
}

ListExpr lfdistanceparamTypeMap(ListExpr args)
{
	if(!nl->HasLength(args,21)){
		return (listutils::typeError("21 arguments expected"));
	}
    ListExpr arg1 = nl->First(args);
	ListExpr arg2 = nl->Second(args);
	ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);
    ListExpr arg5 = nl->Fifth(args);
    ListExpr arg6 = nl->Sixth(args);

	string err = "Point x Point x stype x 3 string x 15 real expected ";
	if(!Point::checkType(arg1))
	{
		return (listutils::typeError(err + " (arg 1 is not an point)"));
	}
	if(!Point::checkType(arg2))
	{
		return (listutils::typeError(err + " (arg 2 is not an point)"));
	}
	if(!raster2::util::isSType(arg3))
	{
		return (listutils::typeError(err + " (arg 3 is not an stype)"));
	}
	if(!CcString::checkType(arg4))
	{
            return (listutils::typeError(err
                + " (arg 4 arg is not an string)"));
	}
	if(!CcString::checkType(arg5))
	{
		return (listutils::typeError(err
                + " (arg 5 is not an string)"));
	}
	if(!CcString::checkType(arg6))
	{
            return (listutils::typeError(err + " (arg 6 is not an string)"));
	}

	NList type(args);

    if(!CcReal::checkType(type.elem(7).listExpr()))
        {
            return (listutils::typeError(err + "arg 7 is not an real "));
        }
    if(!CcReal::checkType(type.elem(8).listExpr()))
        {
            return (listutils::typeError(err + "arg 8 is not an real "));
        }
    if(!CcReal::checkType(type.elem(9).listExpr()))
        {
            return (listutils::typeError(err + "arg 9 is not an real "));
        }
    if(!CcReal::checkType(type.elem(10).listExpr()))
        {
            return (listutils::typeError(err + "arg 10 is not an real "));
        }
    if(!CcReal::checkType(type.elem(11).listExpr()))
        {
            return (listutils::typeError(err + "arg 11 is not an real "));
        }
    if(!CcReal::checkType(type.elem(12).listExpr()))
        {
            return (listutils::typeError(err + "arg 12 is not an real "));
        }
    if(!CcReal::checkType(type.elem(13).listExpr()))
        {
            return (listutils::typeError(err + "arg 13 is not an real "));
        }
    if(!CcReal::checkType(type.elem(14).listExpr()))
        {
            return (listutils::typeError(err + "arg 14 is not an real "));
        }
    if(!CcReal::checkType(type.elem(15).listExpr()))
        {
            return (listutils::typeError(err + "arg 15 is not an real "));
        }
    if(!CcReal::checkType(type.elem(16).listExpr()))
        {
            return (listutils::typeError(err + "arg 16 is not an real "));
        }
    if(!CcReal::checkType(type.elem(17).listExpr()))
        {
            return (listutils::typeError(err + "arg 17 is not an real "));
        }
    if(!CcReal::checkType(type.elem(18).listExpr()))
        {
            return (listutils::typeError(err + "arg 18 is not an real "));
        }
    if(!CcReal::checkType(type.elem(19).listExpr()))
        {
            return (listutils::typeError(err + "arg 19 is not an real "));
        }
    if(!CcReal::checkType(type.elem(20).listExpr()))
        {
            return (listutils::typeError(err + "arg 20 is not an real "));
        }
    if(!CcReal::checkType(type.elem(21).listExpr()))
        {
            return (listutils::typeError(err + "arg 21 is not an real "));
        }
    return (listutils::basicSymbol<CcReal>());
}
#endif // LINEFUNCTION_ALGEBRA_H
