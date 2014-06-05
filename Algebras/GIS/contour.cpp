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

#include "contour.h"
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"

namespace GISAlgebra {

  template <typename T>
  int contourFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    int returnValue = FAILURE;

    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);
    CcReal* lines = static_cast<CcReal*>(args[1].addr);

    switch(message)
    {
      case OPEN:
      {
        // initialize the local storage
        ListExpr resultType = GetTupleResultType(s);
        TupleType *tupleType = new TupleType(nl->Second(resultType));
        local.addr = tupleType;

        return 0;
      }
    
      case REQUEST:
      {
        if(local.addr != 0)
        {
          TupleType *tupleType = (TupleType *)local.addr;
          Tuple *clines = new Tuple( tupleType );

          if(clines != 0)
          {
            raster2::grid2 grid = s_in->getGrid();

            double gridOriginX = grid.getOriginX();
            double gridOriginY = grid.getOriginY();
            double cellsize = grid.getLength();

            double interval = lines->GetValue();

            Rectangle<2> bbox = s_in->bbox();

            raster2::RasterIndex<2> from = 
                               grid.getIndex(bbox.MinD(0), bbox.MinD(1));
            raster2::RasterIndex<2> to = 
                               grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

            for (raster2::RasterIndex<2> index=from; index < to; 
                                         index.increment(from, to))
            {
              // Zellwerte ermitteln
              double e = s_in->get(index);
              double a = s_in->get((int[]){index[0] - 1, index[1] + 1});
              double b = s_in->get((int[]){index[0], index[1] + 1});
              double c = s_in->get((int[]){index[0] + 1, index[1] + 1});
              double d = s_in->get((int[]){index[0] - 1, index[1]});
              double f = s_in->get((int[]){index[0] + 1, index[1]});
              double g = s_in->get((int[]){index[0] - 1, index[1] - 1});
              double h = s_in->get((int[]){index[0], index[1] - 1});
              double i = s_in->get((int[]){index[0] + 1, index[1] - 1});
            
              // Koordinaten bestimmen
              double X = index[0] * cellsize + gridOriginX;
              double Y = index[1] * cellsize + gridOriginY;
              
              double aX = X + cellsize/2 - cellsize;
              double aY = Y + cellsize/2 + cellsize;
              double bX = X + cellsize/2;
              double bY = Y + cellsize/2 + cellsize;
              double cX = X + cellsize/2 + cellsize;
              double cY = Y + cellsize/2 + cellsize;
              double dX = X + cellsize/2 - cellsize;
              double dY = Y + cellsize/2;
              double eX = X + cellsize/2;
              double eY = Y + cellsize/2;
              double fX = X + cellsize/2 + cellsize;
              double fY = Y + cellsize/2;
              double gX = X + cellsize/2 - cellsize;
              double gY = Y + cellsize/2 - cellsize;
              double hX = X + cellsize/2;
              double hY = Y + cellsize/2 - cellsize;
              double iX = X + cellsize/2 + cellsize;
              double iY = Y + cellsize/2 - cellsize;
            
              // wenn alle vier Eckzellen gueltige Werte haben
              // -> Berechnung uber Eckzellen
              if (!(s_in->isUndefined(a)) && !(s_in->isUndefined(c)) && 
                  !(s_in->isUndefined(g)) && !(s_in->isUndefined(i)))
              {
                ProcessRectangle(a, aX, aY, g, gX, gY,
                                 i, iX, iY, c, cX, cY, interval, clines);
              }
            
              // wenn eine Eckzelle nicht definiert ist
              // -> Berechnung ueber 2x2 Quadrat
              if (!(s_in->isUndefined(a)) && !(s_in->isUndefined(d)) &&
                  !(s_in->isUndefined(e)) && !(s_in->isUndefined(b)))
              {
                ProcessRectangle(a, aX, aY, d, dX, dY,
                                 e, eX, eY, b, bX, bY, interval, clines);
              }
            
              if (!(s_in->isUndefined(d)) && !(s_in->isUndefined(g)) &&
                  !(s_in->isUndefined(h)) && !(s_in->isUndefined(e)))
              {
                ProcessRectangle(d, dX, dY, g, gX, gY,
                                 h, hX, hY, e, eX, eY, interval, clines);
              }
            
              if (!(s_in->isUndefined(e)) && !(s_in->isUndefined(h)) &&
                  !(s_in->isUndefined(i)) && !(s_in->isUndefined(f)))
              {
                ProcessRectangle(e, eX, eY, h, hX, hY,
                                 i, iX, iY, f, fX, fY, interval, clines);
              }
            
              if (!(s_in->isUndefined(b)) && !(s_in->isUndefined(e)) &&
                  !(s_in->isUndefined(f)) && !(s_in->isUndefined(c)))
              {
                ProcessRectangle(b, bX, bY, e, eX, eY,
                                 f, fX, fY, c, cX, cY, interval, clines);
              }

              result.addr = clines;
              return YIELD;
            }//for

            result.addr = 0;
            return CANCEL;

          }  
          else
          {
            result.addr = 0;
            return CANCEL;
          }      
        }
      }
      break;

      case CLOSE:
      {
        if(local.addr != 0)
        {
          ((TupleType *)local.addr)->DeleteIfAllowed();
          return 0;          
        }

        returnValue = 0;
      }

      default:
      {
        assert(false);
      }
    }
 
    return returnValue;
  }

  ValueMapping contourFuns[] =
  {
    contourFun<raster2::sint>,
    contourFun<raster2::sreal>,
    0
  };

  int contourSelectFun(ListExpr args)
  {
    int nSelection = -1;
    
    NList type(args);

    if (type.first() == NList(raster2::sint::BasicType()))
    {
      nSelection = 0;
    }
    
    else if (type.first() == NList(raster2::sreal::BasicType()))
    {
      nSelection = 1;
    }

    return nSelection;
  }

  ListExpr contourTypeMap(ListExpr args)
  {
    NList type(args);

    ListExpr attrList=nl->TheEmptyList();
    
    ListExpr attr1 = nl->TwoElemList( nl->SymbolAtom("Height"),
                                     nl->SymbolAtom(CcInt::BasicType()));

    ListExpr attr2 = nl->TwoElemList( nl->SymbolAtom("Contour"),
                                     nl->SymbolAtom(Line::BasicType()));

    attrList = nl->TwoElemList( attr1, attr2 );

    if(type.length() != 2)
    {
      return NList::typeError("two arguments required");
    }

    if (type == NList(raster2::sint::BasicType(), CcReal::BasicType())) 
    {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                             attrList));
    }

    else if(type == NList(raster2::sreal::BasicType(), 
                          CcReal::BasicType())) 
    {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                             attrList));
    }
    
    return NList::typeError
           ("Expecting an sint or sreal and a double (interval).");
  }

  bool ProcessRectangle(double a, double aX, double aY,
                        double g, double gX, double gY,
                        double i, double iX, double iY,
                        double c, double cX, double cY, 
                        double interval, Tuple* clines)
  {
    // Rechteck verarbeiten (Minimum, Maximum bestimmen)
    double Min = MIN(MIN(a,c),MIN(g,i));
    double Max = MAX(MAX(a,c),MAX(g,i));

    int startLevel = (int) ceil(Min / interval);
    int endLevel = (int) floor(Max / interval);

    // Schnittpunkte bestimmen
    for(int iLevel = startLevel; iLevel < endLevel; iLevel++)
    {
      double level = iLevel * interval;

      int nPoints = 0; 
      int nPoints1 = 0, nPoints2 = 0, nPoints3 = 0;
      double pointsX[4], pointsY[4];

      Intersect( a, aX, aY, g, gX, gY, i, 
                 level, &nPoints, pointsX, pointsY );
      nPoints1 = nPoints;

      Intersect( g, gX, gY, i, iX, iY, c,
                 level, &nPoints, pointsX, pointsY );
      nPoints2 = nPoints;

      Intersect( i, iX, iY, c, cX, cY, a,
                 level, &nPoints, pointsX, pointsY );
      nPoints3 = nPoints;

      Intersect( c, cX, cY, a, aX, aY, g,
                 level, &nPoints, pointsX, pointsY );

      if( nPoints >= 2 )
      {
        // links und unten
        if ( nPoints1 == 1 && nPoints2 == 2)
        {
          return AddSegment( level, pointsX[0], pointsY[0], 
                                    pointsX[1], pointsY[1], c > g, clines);
        }
        // links und rechts
        else if ( nPoints1 == 1 && nPoints3 == 2 )
        {
          return AddSegment( level, pointsX[0], pointsY[0], 
                                    pointsX[1], pointsY[1], a > i, clines);
        }
        // links und oben
        else if ( nPoints1 == 1 && nPoints == 2 )
        { 
          if ( !(a == level && g == level) )
            return AddSegment( level, pointsX[0], pointsY[0], 
                                      pointsX[1], pointsY[1], a > i, clines);
        }
        // unten und rechts
        else if(  nPoints2 == 1 && nPoints3 == 2)
        {
          return AddSegment( level, pointsX[0], pointsY[0], 
                                    pointsX[1], pointsY[1], a > i, clines);
        }
        // unten und oben
        else if ( nPoints2 == 1 && nPoints == 2 )
        {
          return AddSegment( level, pointsX[0], pointsY[0], 
                                    pointsX[1], pointsY[1], g > c, clines);
        }
        // rechts und oben
        else if ( nPoints3 == 1 && nPoints == 2 )
        { 
          if ( !(c == level && a == level) )
             return AddSegment( level, pointsX[0], pointsY[0], 
                                       pointsX[1], pointsY[1], g > c, clines);
        }
        else
        {
          return error;
        }
      }

      if( nPoints == 4 )
      {
        if ( !(c == level && a == level) )
        {
          return AddSegment( level, pointsX[2], pointsY[2], 
                                    pointsX[3], pointsY[3], i > c, clines);

        }
      }
    } 
    return false;
  }

  void Intersect(double val1, double val1X, double val1Y,
                 double val2, double val2X, double val2Y,
                 double val3, double level, int *pnPoints,
                 double *ppointsX, double *ppointsY )
  {
    if( val1 < level && val2 >= level )
    {
      double diff = (level - val1) / (val2 - val1);

      ppointsX[*pnPoints] = val1X * (1.0 - diff) + val2X * diff;
      ppointsY[*pnPoints] = val1Y * (1.0 - diff) + val2Y * diff;
      (*pnPoints)++;
    }
    else if( val1 > level && val2 <= level )
    {
      double diff = (level - val2) / (val1 - val2);

      ppointsX[*pnPoints] = val2X * (1.0 - diff) + val1X * diff;
      ppointsY[*pnPoints] = val2Y * (1.0 - diff) + val1Y * diff;
      (*pnPoints)++;
    }
    else if( val1 == level && val2 == level && val3 != level )
    {
      ppointsX[*pnPoints] = val2X;
      ppointsY[*pnPoints] = val2Y;
      (*pnPoints)++;
    }
  }

  bool AddSegment(double l, double startX, double startY,
                  double endX, double endY, int leftHigh, Tuple* clines)
  {
    Point p1(true, startX, startY);
    Point p2(true, endX, endY);
    HalfSegment hs(true, p1, p2);
    Line* line = new Line(0);

    line->Put(0,hs);

    int l2 = static_cast<int>(l);
    CcInt* level = new CcInt(true,l2);



    // Contourlinie fuer level finden
    //if (line->getLevel() == level)
    // wenn existent, dann Segment am richtigen Ende hinzufuegen

    // ansonsten neue Linie anlegen
    //{
      clines->PutAttribute(0,level);
      clines->PutAttribute(1,line);

    //}

    return true;
  }

}
