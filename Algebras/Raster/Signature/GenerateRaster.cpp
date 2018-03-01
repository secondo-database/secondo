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

//---------------------------------------------------------------------------
#ifndef GERARASTERSECONDOCPP
#define GERARASTERSECONDOCPP
#define MAX_VECTORS_PART 255

#include "GenerateRaster.h"
//#include "Segment.cpp"
#include "ClipCel.cpp"
#include "../RTree/Plane.cpp"
#include "Signature4CRS.cpp"
#include "../RTree/coord.cpp"
//#include "rotinas.cpp"

#define CoordinateX 0
#define CoordinateY 1
#define ROUNDING_CONSTANT 0.000001
unsigned is_equal(long double a, long double b);

//#define DEBUGMESSAGES

Signature4CRS* GeraRasterSecondo::generateRaster(const long id, 
             const Region *region, const Line *line, const Points *points,
             int potency, SignatureType signatureType)
{
  Rectangle<2> RectMBR;
  if (region != NULL)
    RectMBR = region->BoundingBox();
  else if (line != NULL)
    RectMBR = line->BoundingBox();
  else
    RectMBR = points->BoundingBox();
  MBR mbr(Coordinate((long int)RectMBR.MinD(CoordinateX), 
                     (long int)RectMBR.MinD(CoordinateY)), 
          Coordinate((long int)RectMBR.MaxD(CoordinateX), 
                     (long int)RectMBR.MaxD(CoordinateY)));
  Plane plane = minimumPlane( potency, mbr );
  
  long cellSize = 0x1lu << plane.potency();
  unsigned numberOfCellsX, numberOfCellsY;
  double blockArea=0;
  ClipCel clipCel[PRODUCT];
  HalfSegment hs;
  Point pt(0,0);
  
  //swapMatrix is used to indicate, in the full cell step,
  //when the filling must change from empty to full and vice versa
  static int swapMatrix[ PRODUCT ];
  
  //calculatedGrid is used to store the cells that already have their filling
  //defined in the execution of the algorithm of border filling,
  //so that these cells would not be considered during the determination of 
  //the full cells filling
  static int calculatedGrid[PRODUCT];
  memset(swapMatrix,0, PRODUCT * sizeof( int ));
  memset(calculatedGrid,0, PRODUCT * sizeof( int ));

  Coordinate min,max;

  min.x = mbr.min.x & (0xFFFFFFFFlu << plane.potency());
  min.y = mbr.min.y & (0xFFFFFFFFlu << plane.potency());
  // The min coordinate should be less or equal to the min Coordinate
  // of the plane calculated or will be one cell size to the right.
  // So, when they are different, I just set the min coordinate
  // to plane.min and take off the number of cells in X and/or in Y.
  numberOfCellsX = plane.numberOfCellsX();
  numberOfCellsY = plane.numberOfCellsY();

  if (min.x != plane.min.x)
  {
    plane.min.x = min.x;
    numberOfCellsX--;
  }
  if (min.y != plane.min.y)
  {
     plane.min.y = min.y;
     numberOfCellsY--;
  }

  max.x = min.x + numberOfCellsX * cellSize ;
  max.y = min.y + numberOfCellsY * cellSize ;

  if ( plane.max.x  != max.x)
  {
     numberOfCellsX--;
  }
  if  ( plane.max.y  != max.y )
  {
     numberOfCellsY--;
  }

  Signature4CRS::RasterMap4CRS rasterMap4CRS(id, mbr,
                                         numberOfCellsX, numberOfCellsY,
                                         plane.potency());

  rasterMap4CRS.setGroupOfBits(Signature4CRS::Empty);

  blockArea=cellSize*cellSize;


    Coordinate     currentCell, endCell;
    RealCoordinate entryPoint, coord;
    unsigned       cornerCoordinate;
    long double    xCurrentCell, yCurrentCell;
  Point          lp, rp;
  bool           insideAbove;

    uchar numberOfVectors  = 0;
    int matrixPosition;

  unsigned nCurrentSegment = 0;
  bool endOfParts = false;
  if (region != NULL)
    region->SelectFirst();
  else if (line != NULL)
    line->SelectFirst();
  else
    points->SelectFirst();
  while(!endOfParts)
  {
    #ifdef DEBUGMESSAGES
      cout << "Checking segment " << nCurrentSegment << std::endl;
    #endif
    if (region != NULL){

      region->GetHs(hs);
      lp = hs.GetLeftPoint();
      rp = hs.GetRightPoint();
      insideAbove = hs.GetAttr().insideAbove;
    }
    else if (line != NULL){
      line->GetHs(hs);
      lp = hs.GetLeftPoint();
      rp = hs.GetRightPoint();
    }
    else {
      points->GetPt(pt);
      lp = pt;
      rp = pt;
    }
    if (points != NULL || hs.IsLeftDomPoint()) //works only with left segments
    {
      nCurrentSegment++;
      currentCell = findCell(lp, plane);
      endCell = findCell(rp, plane);

    //visits the floodFill from bottom to top, to be able to use the
    //parameter InsideAbove
      if (nCurrentSegment == 0) // First segment
      {
        if ( ( currentCell.x > MAX_DIMENSION_GRID ) ||
             ( currentCell.y > MAX_DIMENSION_GRID ) )
        {
          cout << "Overflow - Dimention of the grid" << std::endl;
          return NULL;
        }
      }

      entryPoint.x = lp.GetX();
      entryPoint.y = lp.GetY();


      Segment segment(RealCoordinate(lp.GetX(), lp.GetY()),
                      RealCoordinate(rp.GetX(), rp.GetY()),
                      insideAbove,nCurrentSegment);

      while (currentCell != endCell)
      {
        xCurrentCell = currentCell.x * cellSize + plane.min.x;
        yCurrentCell = currentCell.y * cellSize + plane.min.y;

        matrixPosition = currentCell.y * numberOfCellsX + currentCell.x;
        #ifdef DEBUGMESSAGES
          cout << "matrixPosition: " << matrixPosition << std::endl;
        #endif

        Coordinate coordSupDir;
        Segment sInside;
        bool inside,isIntersectionPoint;
        RealCoordinate intersectionPoint;

        coordSupDir.x = (long int)(xCurrentCell+cellSize);
        coordSupDir.y = (long int)(yCurrentCell+cellSize);

        clipCel[matrixPosition].handleSegment(Coordinate(
                      (long int)xCurrentCell,(long int)yCurrentCell),
                      coordSupDir,segment,sInside, inside,
                      isIntersectionPoint,intersectionPoint, signatureType);



        //for lines and points, only inconclusive cells are calculated
        if (region == NULL) 
        {
          if (signatureType == SIGNAT_3CRS)
            rasterMap4CRS.block(currentCell.x,currentCell.y,
                               Signature4CRS::Weak);
          else
            rasterMap4CRS.block(currentCell.x,currentCell.y,
                               clipCel[matrixPosition].evaluateType(blockArea));
        }
        else
          calculatedGrid[matrixPosition] = 1;

        if ( isIntersectionPoint )
          coord = intersectionPoint;
        else
        {
          if ( !(is_equal(sInside.p1.x,entryPoint.x) &&
                 is_equal(sInside.p1.y,entryPoint.y)) )
            coord = sInside.p1;
          else
            coord = sInside.p2;
        }

    if ( is_equal(entryPoint.x, xCurrentCell) || 
             is_equal(entryPoint.x, xCurrentCell + cellSize) )
        {
          if(!is_equal(coord.x, entryPoint.x) && !is_equal(coord.y,
                 entryPoint.y)){
           if(insideAbove)
             swapMatrix[matrixPosition] += 1;
           else
             swapMatrix[matrixPosition] -= 1;
          }
        }
    
        // Find out the next cell

        cornerCoordinate = 0;
        // upper right corner
        if ( is_equal(coord.y,yCurrentCell + cellSize) &&
             is_equal(coord.x,xCurrentCell + cellSize) )
        {
           currentCell.y++;
           currentCell.x++;
           //does not change swapMatrix when it´s a vertical line over
           //the edge of the cell
           if (coord.x != entryPoint.x) {
             if(insideAbove)
               swapMatrix[matrixPosition] += 1;
             else
               swapMatrix[matrixPosition] -= 1;
           }
           cornerCoordinate = 1;
        }
        else
          // upper left corner
          if ( is_equal(coord.y,yCurrentCell + cellSize) &&
               is_equal(coord.x,xCurrentCell) )
          {
            currentCell.y++;
           if (coord.x != entryPoint.x) {
              if(insideAbove)
                swapMatrix[matrixPosition] += 1;
              else
                swapMatrix[matrixPosition] -= 1;
            }
            cornerCoordinate = 1;
          }
          else
            //do not change swapMatrix on the inferior corners, so it
            //will not count the swap twice
            //lower right corner
           if ( is_equal(coord.y,yCurrentCell) &&
                 is_equal(coord.x,xCurrentCell + cellSize) )
           {
              currentCell.x++;
              cornerCoordinate = 1;
              if(insideAbove)
                swapMatrix[matrixPosition] += 1;
              else
                swapMatrix[matrixPosition] -= 1;
           }
           else
             //lower left corner
             if ( is_equal(coord.y,yCurrentCell) &&
                  is_equal(coord.x,xCurrentCell) )
             {
               cornerCoordinate = 1;
               //if ( endPoint.y >= yCurrentCell )
               //{
               //  currentCell.x--;
               //}
               //else
               {
                 //if ( endPoint.x >= xCurrentCell )
                 {
                   currentCell.y--;
//                   if(insideAbove)
//                     swapMatrix[matrixPosition] += 1;
//                   else
//                     swapMatrix[matrixPosition] -= 1;
                 }
                 //else
                 //{
                 //  currentCell.x--;
                 //  currentCell.y--;
                 //  if(insideAbove)
                 //    swapMatrix[matrixPosition] += 1;
                 //  else
                 //    swapMatrix[matrixPosition] -= 1;
                 //}
               }
             }

        if ( !cornerCoordinate )
        {
          //Intersection with cel.top
          if ( is_equal(coord.y,yCurrentCell + plane.sizeOfCell) )
          {
            currentCell.y++;
          }
          //Intersection with cel.left
          else if ( is_equal(coord.x,xCurrentCell))
          {
            currentCell.x--;
            if(insideAbove)
              swapMatrix[matrixPosition] += 1;
            else
              swapMatrix[matrixPosition] -= 1;
          }
          //Intersection with cel.right
          else if ( is_equal(coord.x,xCurrentCell + plane.sizeOfCell) )
          {
            currentCell.x++;
            if(insideAbove)
              swapMatrix[matrixPosition] += 1;
            else
              swapMatrix[matrixPosition] -= 1;
          }
          //Intersection with cel.bottom
          else //if ( is_equal(coord.y,yCurrentCell))
          {
            currentCell.y--;
          }
        }
        entryPoint = coord;

         numberOfVectors++;

        //if the number of vector is larger than the maximum number of vertices
        //than it´s necessary to raise the size of the cell so less vectors will
        //be considered. This increment of the size of the cell resolves,
        //for example, the problem that occurs an infinite loop navigating
        //through the cells. This error ocurred on the generation of the 3cr
        //for the polilyne sequoia11-185839. This error could be resolved,
        //though, since now we are storing the signature 4crs and not 4drs;
        //we are not storing vectors anymore
        
        if ( numberOfVectors >= MAX_VECTORS_PART )
        {
          return NULL;
        }

      } //end of while (currentCell != endCell)
      //calculates the swapMatrix of the endCell
      //checl of the right point is exactly in the border of the cell
      if (!is_equal(currentCell.x * cellSize + plane.min.x, rp.GetX())) 
      {
        xCurrentCell = currentCell.x * cellSize + plane.min.x;
        yCurrentCell = currentCell.y * cellSize + plane.min.y;
        
        if(points != NULL || line != NULL) {
          if (signatureType == SIGNAT_3CRS)
            rasterMap4CRS.block(currentCell.x,currentCell.y,
                   Signature4CRS::Weak);
          else
            rasterMap4CRS.block(currentCell.x,currentCell.y,
                   clipCel[matrixPosition].evaluateType(blockArea));
        }

        matrixPosition = currentCell.y * numberOfCellsX + currentCell.x;

        Coordinate coordSupDir;
        Segment sInside;
        bool inside,isIntersectionPoint;
        RealCoordinate intersectionPoint;

        coordSupDir.x = (long int)(xCurrentCell+cellSize);
        coordSupDir.y = (long int)(yCurrentCell+cellSize);

        clipCel[matrixPosition].handleSegment(Coordinate((long int)xCurrentCell,
                 (long int)yCurrentCell),coordSupDir,segment,
                 sInside, inside, isIntersectionPoint,intersectionPoint, 
                 signatureType);

        calculatedGrid[matrixPosition] = 1;

        if ( isIntersectionPoint )
          coord = intersectionPoint;
        else
        {
          if ( !(is_equal(sInside.p1.x,entryPoint.x) &&
                 is_equal(sInside.p1.y,entryPoint.y)) )
            coord = sInside.p1;
          else
            coord = sInside.p2;
        }

        if (is_equal(entryPoint.x, xCurrentCell) || is_equal(entryPoint.x,
                  xCurrentCell + cellSize))
        {
           if(insideAbove)
             swapMatrix[matrixPosition] += 1;
           else
             swapMatrix[matrixPosition] -= 1;
        }
      }
    }
    if (region != NULL) {
      region->SelectNext();
      endOfParts = region->EndOfHs();
    }
    else if (line != NULL) {
      line->SelectNext();
      endOfParts = line->EndOfHs();
    }
    else {
      points->SelectNext();
      endOfParts = points->EndOfPt();
    }
  }

  #ifdef DEBUGMESSAGES
    for(int posy=numberOfCellsY - 1; posy>=0; posy--)
    {
      for(int posx=0; posx<numberOfCellsX; posx++)
      {
        int matrixPosition = posy * numberOfCellsX + posx;
        cout << swapMatrix[matrixPosition];
     }
      cout<<std::endl;
    }
  #endif
  if (region != NULL)
  {
    for(unsigned posx=0; posx<numberOfCellsX; posx++)
    {
      Signature4CRS::Weight currentFilling = Signature4CRS::Empty;
      int accumulatedSwap = 0;
      for(unsigned posy=0; posy<numberOfCellsY; posy++)
      {
        int matrixPosition = posy * numberOfCellsX + posx;
        accumulatedSwap += swapMatrix[matrixPosition];
        if (calculatedGrid[matrixPosition])
        {
          if (signatureType == SIGNAT_3CRS)
            rasterMap4CRS.block(posx,posy,Signature4CRS::Weak);
          else
            rasterMap4CRS.block(posx,posy,
                    clipCel[matrixPosition].evaluateType(blockArea));
        }
        else
        {
          if (accumulatedSwap == 2)
            currentFilling = Signature4CRS::Full;
          else
            currentFilling = Signature4CRS::Empty;
          if ( (rasterMap4CRS.block(posx,posy) == Signature4CRS::Empty) &&
               (!calculatedGrid[matrixPosition]))
          {
            rasterMap4CRS.block(posx,posy,currentFilling);
          }
        }
      }
    }
  }
  return new Signature4CRS( rasterMap4CRS );
}

Plane GeraRasterSecondo::minimumPlane(const int potencyInicial, const MBR &mbr)
{
  long factor;

  for(unsigned i = potencyInicial; i < 8 * sizeof( long ); i++ )
  {
    factor = 1l << i;
    Coordinate lmax( mbr.maxD(CoordinateX) < 0 ? mbr.maxD(CoordinateX) 
                     - factor : mbr.maxD(CoordinateX),
                     mbr.maxD(CoordinateY) < 0 ? mbr.maxD(CoordinateY) 
                     - factor : mbr.maxD(CoordinateY) ),
               lmin( mbr.minD(CoordinateX) < 0 ? mbr.minD(CoordinateX) 
               - factor : mbr.minD(CoordinateX),
                     mbr.minD(CoordinateY) < 0 ? mbr.minD(CoordinateY) 
                     - factor : mbr.minD(CoordinateY) );
    double dx = (lmax.x+1) / factor - lmin.x / factor + 1,
           dy = (lmax.y+1) / factor - lmin.y / factor + 1;


    if (( dx * dy <= (double) PRODUCT ) && (dx <= MAX_DIMENSION_GRID) &&
                (dy <= MAX_DIMENSION_GRID))
    {
      long ldx = (long)dx, ldy = (long)dy;
      return Plane( Coordinate( (lmin.x / factor) * factor, 
                    (lmin.y / factor) * factor ),
                    Coordinate( ((lmin.x / factor) + ldx) * factor,
                    ((lmin.y / factor) + ldy) * factor ), factor );
    }
  }
  return Plane( Coordinate( 0,0 ), Coordinate( 0,0 ), 0 );
}

//---------------------------------------------------------------------------

Coordinate GeraRasterSecondo::findCell(const Point& ponto, const Plane& plane)
{
  return Coordinate( (long int)((ponto.GetX() - plane.min.x) /plane.sizeOfCell),
                    (long int)((ponto.GetY() - plane.min.y) /plane.sizeOfCell));
}

unsigned is_equal(long double a, long double b)
{
  if ( fabsl(a - b) <= ROUNDING_CONSTANT ) return 1;
  return 0;
}

#endif
