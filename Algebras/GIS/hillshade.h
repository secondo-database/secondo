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

#ifndef GIS_HILLSHADE_H
#define GIS_HILLSHADE_H

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
declaration of hillshadeFuns array

*/
        extern ValueMapping hillshadeFuns[];

/*
Type Mapping

*/
        ListExpr hillshadeTypeMap(ListExpr args);

/*
Value Mapping

*/
        int hillshadeSelectFun(ListExpr args);

/*
Struct hillshadeInfo describes name, syntax, meaning and signature
of operator hillshade.

*/
        struct hillshadeInfo : OperatorInfo 
        {
          hillshadeInfo()
          { 
            name      = "hillshade";
            signature = "sType x double x double x double -> sType";
            appendSignature(
                        "tType x double x double x double -> stream(tType)");
            syntax    = "_ hillshade[_,_,_]";
            meaning   = "Creates hillshade for raster or tile";
           }              
        };
}

#endif /* #define GIS_HILLSHADE_H */

