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

#ifndef RASTER2_FROMLINE_H
#define RASTER2_FROMLINE_H

#include <NList.h>

#include "../sbool.h"
#include "../grid2.h"
#include "SpatialAlgebra.h"

namespace raster2 {
        ListExpr fromLineTypeMap(ListExpr args);
        int fromLineFun(Word* args, Word& result, int message, 
                        Word& local, Supplier s);
        bool intersection(double x1, double y1, double x2, double y2, 
                          double bx1, double by1, double bx2, double by2);

        struct fromLineInfo : OperatorInfo 
        {
          fromLineInfo()
          { 
            name      = "fromline";
            signature = Line::BasicType() + " fromline " + 
                    grid2::BasicType() + " -> " + sbool::BasicType();
            syntax    = "fromline(_,_)";
            meaning   = "Creates sbool from line and grid";
           }              
        };

}

#endif /* #define RASTER2_FROMLINE_H */

