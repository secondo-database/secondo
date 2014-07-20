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

*/

#ifndef GIS_CONTOUR_H
#define GIS_CONTOUR_H

/*
SECONDO includes

*/
#include <NList.h>

/*
Raster2 includes

*/
#include "../Raster2/stype.h"

/*
declaration of namespace GISAlgebra

*/
namespace GISAlgebra 
{
/*
declaration of struct ResultInfo

*/
        struct ResultInfo
        {
          int level;
          Line* cline;
        };

/*
declaration of contourFuns array

*/
        extern ValueMapping contourFuns[];

/*
Type Mapping

*/
        ListExpr contourTypeMap(ListExpr args);

/*
Value Mapping

*/
        int contourSelectFun(ListExpr args);

/*
Method ProcessRectangle returns true if processing was successful

Parameters: values and coordinates of four points, the wanted interval, 
            the minimum value and DbArray ResultInfo to store the found 
            segments

*/
        bool ProcessRectangle(double, double, double,
                              double, double, double,
                              double, double, double,
                              double, double, double, 
                              int, double, DbArray<ResultInfo>*);

/*
Method Intersects calculates if a line between two points intersects the level

Parameters: values and coordinates of two points, value of a third point, 
            the level value, a pointer for a counter to store the number 
            of intersect and two pointer two store the point where the line is
            intersected

*/
        void Intersect(double, double, double,
                       double, double, double,
                       double, double, int*, double*, double*);

/*
Method AddSegment addes the found segments to the ResultInfo DbArray

Parameters: level value, coordinates of segments start and stop point, 
            the minimum value, the interval value and DbArray ResultInfo 
            to store the segments

*/
        bool AddSegment(int, double, double, 
                        double, double, double, int,
                        DbArray<ResultInfo>*);

/*
Struct contourInfo describes name, syntax, meaning and signature
of operator contour.

*/
        struct contourInfo : OperatorInfo 
        {
          contourInfo()
          { 
            name      = "contourlines";
            signature = "sType x double -> lines";
            syntax    = "contourlines(_,_)";
            meaning   = "Creates contour lines";
           }              
        };
}

#endif /* #define GIS_CONTOUR_H */

