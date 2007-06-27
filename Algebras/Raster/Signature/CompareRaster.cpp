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
#include "CompareRaster.h"
//#include "../Signature/Polilinha.h"
//#include "TerceiroPasso.h"

//#define DEBUGMESSAGES

long compareSignatures4CRS( Signature4CRS *assinat4crs1, 
                 Signature4CRS *assinat4crs2, MBR &MBRintersection)
{
  long comparisonCellSize;
  long result = -1;

  #ifdef DEBUGMESSAGES
  cout << "comparing signatures..." << std::endl;
  cout << "potency1 = " << assinat4crs1->map->potency << std::endl;
  cout << "potency2 = " << assinat4crs2->map->potency << std::endl;
  #endif
  if ( assinat4crs1->map->potency > assinat4crs2->map->potency )
    comparisonCellSize = 1l << assinat4crs1->map->potency;
  else
    comparisonCellSize = 1l << assinat4crs2->map->potency;

  
  //MBRintersection is the MBR of the intersection considering the size of the
  //cell with smaller resolution, the larger cell size.
  //the MBRintersection is calculated only to obtain the signature 5CDRS and
  //to the operation of intersection. To others operations, the MBRintersection
  //must be calculated in a different way.
  intersectionMBR( assinat4crs1->map->mbr, assinat4crs2->map->mbr, 
                   comparisonCellSize, &MBRintersection);

  result = compareSameSignature(*assinat4crs1,*assinat4crs2,
                  MBRintersection,comparisonCellSize);

  return result;
}

unsigned intersectionMBR(MBR mbr1,MBR mbr2,long smallerResolutionCellSize,
                         MBR *MBRintersection)
{
  int displacementX, displacementY;
  
  if ( ( mbr1.max.x < mbr2.min.x ) ||
       ( mbr2.max.x < mbr1.min.x ) ||
       ( mbr1.max.y < mbr2.min.y ) ||
       ( mbr2.max.y < mbr1.min.y ) ) return 0;

  MBRintersection->max.x = smaller(mbr1.max.x,mbr2.max.x);
  MBRintersection->max.y = smaller(mbr1.max.y,mbr2.max.y);
  MBRintersection->min.x = larger(mbr1.min.x,mbr2.min.x);
  MBRintersection->min.y = larger(mbr1.min.y,mbr2.min.y);

  // smallerResolutionCellSize-1 --> don´t displace one more cell to
  // the left/down if the point is in the limit of the cell

  MBRintersection->max.x % smallerResolutionCellSize == 0 ? 
                displacementX = 0: displacementX = 1;
  MBRintersection->max.y % smallerResolutionCellSize == 0 ? 
                displacementY = 0: displacementY = 1;

  MBRintersection->max.x = MBRintersection->max.x < 0 ? MBRintersection->max.x 
          - (smallerResolutionCellSize-1) : MBRintersection->max.x;
  MBRintersection->max.y = MBRintersection->max.y < 0 ? MBRintersection->max.y 
          - (smallerResolutionCellSize-1) : MBRintersection->max.y;
  MBRintersection->min.x = MBRintersection->min.x < 0 ? MBRintersection->min.x 
          - (smallerResolutionCellSize-1) : MBRintersection->min.x;
  MBRintersection->min.y = MBRintersection->min.y < 0 ? MBRintersection->min.y 
          - (smallerResolutionCellSize-1) : MBRintersection->min.y;

  MBRintersection->max.x = ( MBRintersection->max.x / smallerResolutionCellSize 
          + displacementX ) * smallerResolutionCellSize;
  MBRintersection->max.y = ( MBRintersection->max.y / smallerResolutionCellSize 
          + displacementY ) * smallerResolutionCellSize;
  MBRintersection->min.x = ( MBRintersection->min.x / smallerResolutionCellSize)
          * smallerResolutionCellSize;
  MBRintersection->min.y = ( MBRintersection->min.y / smallerResolutionCellSize)
          * smallerResolutionCellSize;

  return 1;
}

long compareSameSignature(const Signature4CRS &a,const Signature4CRS &b, 
                          MBR mbr, long cellSize)
{
  long result = FALSE_HIT;
  for( long i=mbr.min.x; i <= mbr.max.x; i+=cellSize)
    for(long j=mbr.min.y; j <= mbr.max.y ; j+=cellSize)
    {
      Signature4CRS::Weight blockA = a.block(i,j,cellSize);
      Signature4CRS::Weight blockB = b.block(i,j,cellSize);

      if( blockA == Signature4CRS::Empty || blockB == Signature4CRS::Empty )
      continue; // Does not have intersection.
      else if( (blockA==Signature4CRS::Full || blockB==Signature4CRS::Full) ||
          (blockA == Signature4CRS::Strong && blockB == Signature4CRS::Strong) )
        return HIT;
      else {
        result = INCONCLUSIVE;
      }
    }
  return result;
}

