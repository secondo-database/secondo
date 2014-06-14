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
        DbArray<ResultInfo>* clines = new DbArray<ResultInfo>(0);
        local.addr = clines;

        if (clines != 0)
        {
          raster2::grid2 grid = s_in->getGrid();
          Rectangle<2> bbox = s_in->bbox();

          double gridOriginX = grid.getOriginX();
          double gridOriginY = grid.getOriginY();
          double cellsize = grid.getLength();

          double interval = lines->GetValue();

          raster2::RasterIndex<2> from = 
                                  grid.getIndex(bbox.MinD(0), bbox.MinD(1));
          raster2::RasterIndex<2> to = 
                                  grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

          for (raster2::RasterIndex<2> index = from; index < to;
                                       index.increment(from, to))
          {
            // Zellwerte ermitteln
            double e = s_in->get(index);
            double a = s_in->get((int[]){index[0] - 1, index[1] + 1});
            double b = s_in->get((int[]){index[0], index[1] + 1});
            double d = s_in->get((int[]){index[0] - 1, index[1]});
           
            // Koordinaten bestimmen
            double X = index[0] * cellsize + gridOriginX;
            double Y = index[1] * cellsize + gridOriginY;

            // wenn alle vier Zellen gueltige Werte haben
            if (!(s_in->isUndefined(a)) && !(s_in->isUndefined(b)) && 
                !(s_in->isUndefined(d)) && !(s_in->isUndefined(e)))
            {
              ProcessRectangle(a, X, Y + cellsize, 
                               d, X, Y,
                               e, X + cellsize, Y, 
                               b, X + cellsize, Y + cellsize, 
                               interval, clines);       

            }

            // bestimmen welche Zellen Werte enthalten, diese addieren 
            // und durch die Anzahl der gueltigen Zellen teilen
            double sum = 0;
            int good = 0;
            double center = 0;

            if (!(s_in->isUndefined(a)))
            {
              sum += a;
              good++;
            }

            if (!(s_in->isUndefined(b)))
            {
              sum += b;
              good++;
            }

            if (!(s_in->isUndefined(d)))
            {
              sum += d;
              good++;
            }

            if (!(s_in->isUndefined(e)))
            {
              sum += e;
              good++;
            }

            center = sum / good;

            // Alternative Zellwerte berechnen
            double top;
            double left;
            double right;
            double bottom;

            if(!(s_in->isUndefined(a)))
            {
              if(!(s_in->isUndefined(b)))
                top = (a + b) / 2.0;
              else
                top = b;

              if(!(s_in->isUndefined(d)))
                left = (a + d) / 2.0;
              else
                left = a;
            }
            else
            {
              top = b;
              left = d;
            }

            if(!(s_in->isUndefined(e)))
            {
              if(!(s_in->isUndefined(b)))
                right = (e + b) / 2.0;
              else
                right = e;

              if(!(s_in->isUndefined(d)))
                bottom = (e + d) / 2.0;
              else
                bottom = e;
            }
            else
            {
              bottom = d;
              right = b;
            }

            // wenn eine Eckzelle nicht definiert ist
            // -> Berechnung ueber Ersatzwerte
            if (!(s_in->isUndefined(a)))
            {
              ProcessRectangle(a, X, Y + cellsize, 
                               left, X, Y + cellsize/2,
                               center, X + cellsize/2, Y + cellsize/2, 
                               top, X + cellsize/2, Y + cellsize, 
                               interval, clines);
            }
            
            if (!(s_in->isUndefined(d)))
            {
              ProcessRectangle(left, X, Y + cellsize/2, 
                               d, X, Y,
                               bottom, X + cellsize/2, Y, 
                               center, X + cellsize/2, Y + cellsize/2, 
                               interval, clines);
            }
            
            if (!(s_in->isUndefined(e)))
            {
              ProcessRectangle(center, X + cellsize/2, Y + cellsize/2, 
                               bottom, X + cellsize/2, Y,
                               e, X + cellsize, Y, 
                               right, X + cellsize, Y + cellsize/2, 
                               interval, clines);
            }
            
            if (!(s_in->isUndefined(b)))
            {
              ProcessRectangle(top, X + cellsize/2, Y + cellsize, 
                               center, X + cellsize/2, Y + cellsize/2,
                               right, X + cellsize, Y + cellsize/2, 
                               b, X + cellsize, Y + cellsize, 
                               interval, clines);
            }
          }//for
        }//if clines

        return 0;
      }
    
      case REQUEST:
      {
        if(local.addr != 0)
        {
          ListExpr resultType = GetTupleResultType(s);
          TupleType *tupleType = new TupleType(nl->Second(resultType));
          Tuple *clines = new Tuple( tupleType );

          DbArray<ResultInfo>* pResultInfo = (DbArray<ResultInfo>*)local.addr;

          if( clines != 0 )
          {
            if ( pResultInfo != 0 )
            {
              int i = pResultInfo->Size();

              if ( i > 0 )
              {
                ResultInfo temp;
                pResultInfo->Get(i-1,temp);
                pResultInfo->resize(i-1);
                
                CcInt* level = new CcInt(true,temp.level);
                Line* line = new Line(0); 
                line = temp.cline;

                clines->PutAttribute(0,level);
                clines->PutAttribute(1,line);
                result.addr = clines;

                return YIELD;
              }
            }

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
          DbArray<ResultInfo>* pResultInfo = (DbArray<ResultInfo>*)local.addr;
          
          if(pResultInfo != 0)
          {
            delete pResultInfo;
            local.addr = 0;
          }  
        }

        return 0;        
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
                        double interval, DbArray<ResultInfo>* clines)
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
                  double endX, double endY, int leftHigh, 
                  DbArray<ResultInfo>* clines)
  {
    Point p1(true, startX, startY);
    Point p2(true, endX, endY);
    HalfSegment hs(true, p1, p2);
    Line* line = new Line(0);

    line->Put(0,hs);

    int l2 = static_cast<int>(l);

    // Contourlinie fuer level finden
    //if (line->getLevel() == level)
    // wenn existent, dann Segment am richtigen Ende hinzufuegen

    // ansonsten neue Linie anlegen
    //{
      ResultInfo lines;// = new ResultInfo;

      lines.level = l2;
      lines.cline = line;

      clines->Append(lines);

    //}

    return true;
  }

}
