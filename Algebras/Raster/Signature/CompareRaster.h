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

May, 2007 Leonardo Azevedo, Rafael Brand

*/

#ifndef COMPARA_RASTER_H
#define COMPARA_RASTER_H

#include "../Signature/Signature4CRS.h"

long compareSignatures4CRS( Signature4CRS *assinat4crs1, 
                  Signature4CRS *assinat4crs2, MBR &MBRintersection);

unsigned intersectionMBR(MBR mbr1,MBR mbr2,
                  long smallerResolutionCellSize,MBR *MBRintersection);

long compareSameSignature(const Signature4CRS &a,const Signature4CRS &b,
                  MBR mbr, long cellSize);

#endif // COMPARA_RASTER_H
