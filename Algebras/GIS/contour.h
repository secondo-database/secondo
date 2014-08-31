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
Raster2 and Tile includes

*/
#include "../Raster2/stype.h"
#include "../Tile/t/tint.h"
#include "../Tile/t/treal.h"

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
Method GetValuesContour reads the values for 3x3 cells

parameters: a - reference to top left cell \\
            a1 - reference to top left cell + 1 \\
            b - reference to top middle cell \\
            b1 - reference to top middle cell + 1 \\
            c - reference to top right cell \\
            d - reference to middle left cell \\
            f - reference to middle right cell \\
            g - reference to bottom left cell \\
            h - reference to bottom right cell \\
            row - number of current row \\
            column - number of current column \\
            currentTuple - number of current tuple \\
            s\_in - current tuple \\
            maxX - maximum X in a tuple \\
            maxY - maximum Y in a tuple \\
            factorNext - if vector current and next have different start points \\
            factorlast - if vector current and last have different start points \\
            skipNextRow - if difference between next and current is more 
                          than one tile \\
            skipLastRow - if difference between last and current is more 
                          than one tile \\
            current - current vector \\
            next - next vector \\
            last - last vector \\
            currentSize - size of current vector \\
            nextSize - size of next vector \\
            lastSize - size of last vector \\
return value: -
exceptions: -

*/

        template <typename T, typename SourceTypeProperties>
        void GetValuesContour(double* a, double* a1, double* b, double* b1,
               double* c, double* d, double* f, double* g, double* h,
               int row, int column, int currentTuple, T* s_in,
               int maxX, int maxY,
               int factorNext, int factorLast,
               bool skipNextRow, bool skipLastRow,
               vector<Tuple*> current, vector<Tuple*> next,
               vector<Tuple*> last, 
               int currentSize, int nextSize, int lastSize);


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

