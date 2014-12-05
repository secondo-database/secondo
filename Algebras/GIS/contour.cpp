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

/*
GIS includes

*/
#include "contour.h"

/*
Raster2 and Tile includes

*/
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"
#include "../Tile/t/tint.h"
#include "../Tile/t/treal.h"

/*
declaration of namespace GISAlgebra

*/
namespace GISAlgebra {

/*
Method contourFuns: calculates the contour lines for a given sint or sreal
                    object

Return value: stream of tuple (level, line)

*/
  template <typename T>
  int contourFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    int returnValue = FAILURE;

    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);
    CcInt* lines = static_cast<CcInt*>(args[1].addr);

    switch(message)
    {
      case OPEN:
      {
        // initialize the local storage
        double min = s_in->getMinimum();
        double max = s_in->getMaximum();
        int interval = lines->GetValue();
        
        if ( interval < 1 )
        {
          cmsg.error() << "Interval < 1 not allowed" << endl;
          cmsg.send();
          return CANCEL;
        }

        double diff = max - min;
        int num = ceil(diff / interval) + 1;

        DbArray<ResultInfo>* clines = new DbArray<ResultInfo>(num);

        // initialise array and set level to -32000
        for (int i = 0; i<num; i++)
        {
          ResultInfo lines;
          lines.level = -32000;
          clines->Put(i,lines);
        }

        local.addr = clines;

        if (clines != 0)
        {
          raster2::grid2 grid = s_in->getGrid();
          Rectangle<2> bbox = s_in->bbox();

          double gridOriginX = grid.getOriginX();
          double gridOriginY = grid.getOriginY();
          double cellsize = grid.getLength();

          raster2::RasterIndex<2> from = 
                                  grid.getIndex(bbox.MinD(0), bbox.MinD(1));
          raster2::RasterIndex<2> to = 
                                  grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

          for (raster2::RasterIndex<2> index = from; index < to;
                                       index.increment(from, to))
          {
            // central cell
            double e = s_in->get(index);

            if (!(s_in->isUndefined(e)))
            {
              double a = s_in->get((int[]){index[0] - 1, index[1] + 1});
              double a1 = s_in->get((int[]){index[0] - 1, index[1] + 2});
              double b = s_in->get((int[]){index[0], index[1] + 1});
              double b1 = s_in->get((int[]){index[0], index[1] + 2});
              double c = s_in->get((int[]){index[0] + 1, index[1] + 1});
              double d = s_in->get((int[]){index[0] - 1, index[1]});
              double g = s_in->get((int[]){index[0] - 1, index[1] - 1});
              double h = s_in->get((int[]){index[0], index[1] - 1});
              double f = s_in->get((int[]){index[0] + 1, index[1]});

              // calculate coordinates
              double X = index[0] * cellsize + cellsize/2 + gridOriginX;
              double Y = index[1] * cellsize + cellsize/2 + gridOriginY;

              // if all four cells have valid values
              if (!(s_in->isUndefined(a)) && !(s_in->isUndefined(b)) && 
                  !(s_in->isUndefined(d)) && !(s_in->isUndefined(e)))
              {
                // special case for bottom right cell
                if ((s_in->isUndefined(h)) && (s_in->isUndefined(f)))
                {
                  ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                   d, X - cellsize, Y - cellsize/2,
                                   e, X + cellsize/2, Y - cellsize/2, 
                                   b, X + cellsize/2, Y + cellsize, 
                                   interval, min, clines);       
                }
                // special case for first row
                else if ((s_in->isUndefined(h)) && (s_in->isUndefined(g)))
                {
                  ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                   d, X - cellsize, Y - cellsize/2,
                                   e, X, Y - cellsize/2, 
                                   b, X, Y + cellsize, 
                                   interval, min, clines);       
                }
                // special case for top right cell
                else if ((s_in->isUndefined(f)) && (s_in->isUndefined(a1)) 
                                                && (s_in->isUndefined(b1)))
                {
                  ProcessRectangle(a, X - cellsize, Y + cellsize + cellsize/2,
                                   d, X - cellsize, Y,
                                   e, X + cellsize/2, Y, 
                                   b, X + cellsize/2, Y + cellsize+cellsize/2,
                                   interval, min, clines);       
                }
                // special case for last column
                else if ((s_in->isUndefined(f)) && (s_in->isUndefined(c)))
                {
                  ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                   d, X - cellsize, Y,
                                   e, X + cellsize/2, Y, 
                                   b, X + cellsize/2, Y + cellsize, 
                                   interval, min, clines);       
                }
                // special case for top row
                else if ((s_in->isUndefined(a1)) && (s_in->isUndefined(b1)))
                {
                  ProcessRectangle(a, X - cellsize, Y + cellsize + cellsize/2, 
                                   d, X - cellsize, Y,
                                   e, X, Y, 
                                   b, X, Y + cellsize+cellsize/2,
                                   interval, min, clines);       
                }
                // normal case
                else
                {
                  ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                   d, X - cellsize, Y,
                                   e, X, Y, 
                                   b, X, Y + cellsize, 
                                   interval, min, clines);       
                }
              }
              else
              {
                // determine which cells have defined values, accumulate values
                // and divide through number of valid cells
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
  
                // calculate alternative values
                double top;
                double left;
                double right;
                double bottom;
  
                if(!(s_in->isUndefined(a)))
                {
                  if(!(s_in->isUndefined(b)))
                    top = (a + b) / 2.0;
                  else
                    top = a;
  
                  if(!(s_in->isUndefined(d)))
                    left = (a + d) / 2.0;
                  else
                    left = a;
                }
                else
                {
                  if (!(s_in->isUndefined(b)))
                    top = b;
                  else
                    top = e;

                  if (!(s_in->isUndefined(d)))
                    left = d;
                  else
                    left = e;
                }
  
                if(!(s_in->isUndefined(b)))
                  right = (e + b) / 2.0;
                else
                  right = e;
    
                if(!(s_in->isUndefined(d)))
                  bottom = (e + d) / 2.0;
                else
                  bottom = e;

                // if one cell is not defined
                // -> calculation with alternative values
                if (!(s_in->isUndefined(a)))
                {
                  ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                   left, X - cellsize, Y + cellsize/2,
                                   center, X - cellsize/2, Y + cellsize/2, 
                                   top, X - cellsize/2, Y + cellsize, 
                                   interval, min, clines);
                }
            
                if (!(s_in->isUndefined(d)))
                {
                  if ((s_in->isUndefined(f)) && (s_in->isUndefined(b)))
                  {
                    // special case top right cell
                  }
                  else if (!(s_in->isUndefined(e)) && !(s_in->isUndefined(d)) &&
                            (s_in->isUndefined(a)) && !(s_in->isUndefined(b)))
                  {
                    // special case cell under undefined cell
                    ProcessRectangle(left, X - cellsize, Y + cellsize/2, 
                                     d, X - cellsize, Y,
                                     bottom, X - cellsize/2, Y, 
                                     center, X - cellsize/2, Y + cellsize/2, 
                                     interval, min, clines);
                  }
                  else if (!(s_in->isUndefined(e)) && !(s_in->isUndefined(d)) &&
                           !(s_in->isUndefined(a)) && (s_in->isUndefined(b)))
                  {
                    // special case cell left under undefined cell
                    ProcessRectangle(left, X - cellsize, Y + cellsize/2, 
                                     d, X - cellsize, Y,
                                     bottom, X - cellsize/2, Y, 
                                     center, X - cellsize/2, Y + cellsize/2, 
                                     interval, min, clines);
                  }
                }
            
                if (!(s_in->isUndefined(e)) && (s_in->isUndefined(h))
                                            && (s_in->isUndefined(d)))
                {
                  // special case left bottom cell
                }
                else if (!(s_in->isUndefined(e)) && (s_in->isUndefined(a)) &&
                         !(s_in->isUndefined(b)) && (s_in->isUndefined(b1)))
                {
                  // special case top left cell
                  ProcessRectangle(b, X-cellsize/2, Y+cellsize+cellsize/2,
                                   bottom, X - cellsize/2, Y,
                                   e, X, Y, 
                                   b, X, Y + cellsize + cellsize/2, 
                                   interval, min, clines);
                }
                else if (!(s_in->isUndefined(e)) && (s_in->isUndefined(a))
                                                 && (s_in->isUndefined(b)))
                {
                  // special case top row
                }
                else if (!(s_in->isUndefined(e)) && !(s_in->isUndefined(f)))
                {
                  // special case right top cell
                  ProcessRectangle(center, X - cellsize/2, Y + cellsize/2, 
                                   bottom, X - cellsize/2, Y,
                                   e, X, Y, 
                                   right, X, Y + cellsize/2, 
                                   interval, min, clines);
                }

                if (!(s_in->isUndefined(e)) && (s_in->isUndefined(a)) &&
                         !(s_in->isUndefined(b)) && (s_in->isUndefined(b1)) &&
                          (s_in->isUndefined(a1)))
                {
                  // special case left top cell
                }
                else if (!(s_in->isUndefined(b)) && (s_in->isUndefined(h)))
                {
                  ProcessRectangle(top, X - cellsize/2, Y + cellsize, 
                                   e, X - cellsize/2, Y - cellsize/2,
                                   e, X, Y - cellsize/2, 
                                   b, X, Y + cellsize, 
                                   interval, min, clines);
                }
                else if (!(s_in->isUndefined(b)) && (s_in->isUndefined(d))
                                                 && !(s_in->isUndefined(b1)))
                {
                  ProcessRectangle(top, X - cellsize/2, Y + cellsize, 
                                   center, X - cellsize/2, Y + cellsize/2,
                                   right, X, Y + cellsize/2, 
                                   b, X, Y + cellsize, 
                                   interval, min, clines);
                }
                else if (!(s_in->isUndefined(b)) && !(s_in->isUndefined(e))
                                                 && (s_in->isUndefined(a)))
                {
                  // special case right of undefined
                  ProcessRectangle(top, X - cellsize/2, Y + cellsize, 
                                   center, X - cellsize/2, Y + cellsize/2,
                                   right, X, Y + cellsize/2, 
                                   b, X, Y + cellsize, 
                                   interval, min, clines);
                }
              }
            }//if e def
          }// for 
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

                int j = 2;

                while ((temp.level == -32000) && (j <= i))
                {
                  pResultInfo->Get(i-j,temp);
                  pResultInfo->resize(i-j);

                  j++;
                }

                if ( temp.level == -32000 )
                {
                  result.addr = 0;
                  return CANCEL;
                }

                CcInt* level = new CcInt(true,temp.level);
                Line* line = temp.cline;
                line->EndBulkLoad();

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
        else
        {
          result.addr = 0;
          return CANCEL;
        }      
      }

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

/*
Method contourFuns: calculates the contour lines for a given stream of 
                    tint or treal objects

Return value: stream of tuple (level, line)

*/
  template <typename T, typename SourceTypeProperties>
  int contourFunTile(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    int returnValue = FAILURE;

    CcInt* lines = static_cast<CcInt*>(args[1].addr);

    vector<Tuple*> current;
    vector<Tuple*> last;
    vector<Tuple*> next;
    Tuple* nextElement;
    double fromX;
    double fromY;
    double toX;
    double toY;
    double tileSize;
    double cellSize;
    bool firstTuple = true;
    bool readNextElement = true;
    bool newLine = false;
    bool skipNextRow;
    bool skipLastRow;
    int currentSize;
    int nextSize;
    int lastSize;
    int currentTuple = 0;

    int xDimensionSize = TileAlgebra::tProperties<char>::GetXDimensionSize();
    int yDimensionSize = TileAlgebra::tProperties<char>::GetYDimensionSize();

    int maxX = xDimensionSize - 1;
    int maxY = yDimensionSize - 1;


    switch(message)
    {
      case OPEN:
      {
        int interval = lines->GetValue();

        // Check for valid interval        
        if ( interval < 1 )
        {
          cmsg.error() << "Interval < 1 not allowed" << endl;
          cmsg.send();
          return CANCEL;
        }

        Word elem;
        Tuple* tuple;
        T* s_in;

        typename SourceTypeProperties::TypeProperties::PropertiesType min;
        typename SourceTypeProperties::TypeProperties::PropertiesType max;

        qp->Open(args[0].addr);
        qp->Request(args[0].addr, elem);

        bool firstValue = true;

        // Get minimum and maximum values of complete tile
        while(qp->Received(args[0].addr))
        {
          tuple = (Tuple*)elem.addr;
          s_in = static_cast<T*>(tuple->GetAttribute(0));

          typename SourceTypeProperties::TypeProperties::PropertiesType minTemp;
          s_in->minimum(minTemp);
          typename SourceTypeProperties::TypeProperties::PropertiesType maxTemp;
          s_in->maximum(maxTemp);

          if ((minTemp < min) || (firstValue == true))
          {
            min = minTemp;
          }

          if ((maxTemp > max) || (firstValue == true))
          {
            max = maxTemp;
          }

          firstValue = false;

          qp->Request(args[0].addr, elem);
        }

        qp->Close(args[0].addr);

        double diff = max - min;
        int num = ceil(diff / interval) + 1;

        DbArray<ResultInfo>* clines = new DbArray<ResultInfo>(num);

        // initialise array and set level to -32000
        for (int i = 0; i<num; i++)
        {
          ResultInfo lines;
          lines.level = -32000;
          clines->Put(i,lines);
        }

        local.addr = clines;

        qp->Open(args[0].addr);

        double gridOriginX;
        double gridOriginY;
        double lastOriginX;
        double lastOriginY;

        while (readNextElement == true)
        {
          qp->Request(args[0].addr, elem);

          if(qp->Received(args[0].addr))
          {
            tuple = (Tuple*)elem.addr;

            s_in = static_cast<T*>(tuple->GetAttribute(0));

            TileAlgebra::tgrid grid;
            s_in->getgrid(grid);
            gridOriginX = grid.GetX();
            gridOriginY = grid.GetY();
            cellSize = grid.GetLength();
            tileSize = cellSize * xDimensionSize;

            // read cells until Y coordinate changes
            if ((gridOriginY == lastOriginY) || (firstTuple == true))
            {
              if (!(firstTuple == true))
              {
                // if there is a gap between two read tiles, fill vector
                // with dummy tile
                while ((gridOriginX - lastOriginX) - tileSize > cellSize)
                {
                  TupleType *tupleType = tuple->GetTupleType();
                  Tuple* dummy = new Tuple( tupleType );
                  T* s_out = new T(true);
                  dummy->PutAttribute(0,s_out);

                  current.push_back(dummy);
                  lastOriginX = lastOriginX + tileSize;
                }
              }

              current.push_back(tuple);
              lastOriginX = gridOriginX;
              lastOriginY = gridOriginY;
              firstTuple = false;
            }
            else
            {
              readNextElement = false;
              firstTuple = true;
              next.push_back(tuple);
              lastOriginX = gridOriginX;
            }
          }
          else
          {
            readNextElement = false;
          }
        } // while currentLine


        currentSize = current.size();

        readNextElement = true;

        while (currentSize != 0)
        {
          while (readNextElement == true)
          {
            qp->Request(args[0].addr, elem);

            if(qp->Received(args[0].addr))
            {
              tuple = (Tuple*)elem.addr;

              s_in = static_cast<T*>(tuple->GetAttribute(0));

              TileAlgebra::tgrid grid;
              s_in->getgrid(grid);
              gridOriginX = grid.GetX();
              gridOriginY = grid.GetY();
              cellSize = grid.GetLength();

              // read cells until Y coordinate changes
              if ((gridOriginY == lastOriginY) || (firstTuple == true))
              {
                // if there is a gap between two read tiles, fill vector
                // with dummy tile
                while ((gridOriginX-lastOriginX) - tileSize > cellSize)
                {
                  TupleType *tupleType = tuple->GetTupleType();
                  Tuple* dummy = new Tuple( tupleType );
                  T* s_out = new T(true);
                  dummy->PutAttribute(0,s_out);

                  next.push_back(dummy);
                  lastOriginX = lastOriginX + tileSize;
                }

                next.push_back(tuple);
                lastOriginX = gridOriginX;
                lastOriginY = gridOriginY;
                firstTuple = false;
              }
              else
              {
                readNextElement = false;
                firstTuple = true;
                newLine = true;

                nextElement = tuple;
                lastOriginX = gridOriginX;
              }
            }
            else
            {
              readNextElement = false;
            }
          } // while NextLine

          int factorNext = 0;
          int factorLast = 0;

          skipNextRow = false;
          skipLastRow = false;

          nextSize = next.size();
          lastSize = last.size();

          Tuple* cTuple = current[0];
          T* c = static_cast<T*>(cTuple->GetAttribute(0));
          TileAlgebra::tgrid cGrid;
          c->getgrid(cGrid);
          double cGridOriginX = cGrid.GetX();
          double cGridOriginY = cGrid.GetY();

          if (nextSize > 0)
          {
            Tuple* nTuple = next[0];
            T* n = static_cast<T*>(nTuple->GetAttribute(0));
            TileAlgebra::tgrid nGrid;
            n->getgrid(nGrid);
            double nGridOriginX = nGrid.GetX();
            double nGridOriginY = nGrid.GetY();

            // calculate factor if tile rows starts at different coordinates
            if ((cGridOriginX - nGridOriginX) > 0)
            {
              while ((cGridOriginX - nGridOriginX) > 0)
              {
                factorNext++;
                nGridOriginX = nGridOriginX + tileSize;              
              }
            }
            else if ((cGridOriginX - nGridOriginX) < 0)
            {
              while ((cGridOriginX - nGridOriginX) < 0)
              {
                factorNext--;
                cGridOriginX = cGridOriginX + tileSize;              
              }
            }

            // check if one or more rows are missing
            if ((nGridOriginY - cGridOriginY) > tileSize)
            {
              skipNextRow = true;
            }          
          }

          if (lastSize > 0)
          {
            Tuple* lTuple = last[0];
            T* l = static_cast<T*>(lTuple->GetAttribute(0));
            TileAlgebra::tgrid lGrid;
            l->getgrid(lGrid);
            double lGridOriginX = lGrid.GetX();
            double lGridOriginY = lGrid.GetY();
            cGridOriginX = cGrid.GetX();
            cGridOriginY = cGrid.GetY();

            // calculate factor if tile rows starts at different coordinates
            if ((cGridOriginX - lGridOriginX) > 0)
            {
              while ((cGridOriginX - lGridOriginX) > 0)
              {
                factorLast++;
                lGridOriginX = lGridOriginX + tileSize;              
              }
            }
            else if ((cGridOriginX - lGridOriginX) < 0)
            {
              while ((cGridOriginX - lGridOriginX) < 0)
              {
                factorLast--;
                cGridOriginX = cGridOriginX + tileSize;              
              }
            }

            // check if one or more rows are missing
            if ((cGridOriginY - lGridOriginY) > tileSize)
            {
              skipLastRow = true;
            }          
          }

          while (currentTuple < currentSize)
          {
            tuple = current[currentTuple];

            if ((tuple == 0) || (tuple->GetNoAttributes() != 1))
            {
              while (tuple == 0)
              {
                currentTuple++;
                tuple = current[currentTuple];
              }

              while (tuple->GetNoAttributes() != 1)
              {
                currentTuple++;
                tuple = current[currentTuple];
              }
            }

            s_in = static_cast<T*>(tuple->GetAttribute(0));

            TileAlgebra::Index<2> from;
            TileAlgebra::Index<2> to;

            s_in->GetBoundingBoxIndexes(from, to);
            fromX = from[0];
            fromY = from[1];
            toX = to[0];
            toY = to[1];      

            TileAlgebra::tgrid grid;
            s_in->getgrid(grid);
              
            cellSize = grid.GetLength();

            gridOriginX = grid.GetX();
            gridOriginY = grid.GetY();

            for(int row = fromY; row <= toY; row++)
            {
              for(int column = fromX; column <= toX; column++)
              {
                TileAlgebra::Index<2> index((int[]){column, row});
  
                // central cell
                double e = s_in->GetValue(index);
 
                if(!(SourceTypeProperties::TypeProperties::IsUndefinedValue(e)))
                {
                  double a =
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double a1 = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double b = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double b1 = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double c = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double d = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double f = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double g = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double h = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();

                  GetValuesContour<T, SourceTypeProperties>
                    (&a, &a1, &b, &b1, &c, &d, &f, &g, &h, row, column, 
                     currentTuple, s_in,
                     maxX, maxY, factorNext, factorLast, 
                     skipNextRow, skipLastRow,
                     current, next, last,
                     currentSize, nextSize, lastSize);

                  // calculate coordinates
                  double X = index[0] * cellSize + cellSize/2 + gridOriginX;
                  double Y = index[1] * cellSize + cellSize/2 + gridOriginY;

                  // if all four cells have valid values
                  if (!(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(a)) && 
                      !(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(b)) && 
                      !(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(d)) && 
                      !(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(e)))
                  {
                    // special case for bottom right cell
                    if ((SourceTypeProperties::TypeProperties::
                         IsUndefinedValue(h)) && 
                        (SourceTypeProperties::TypeProperties::
                         IsUndefinedValue(f)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y - cellSize/2,
                                       e, X + cellSize/2, Y - cellSize/2, 
                                       b, X + cellSize/2, Y + cellSize, 
                                       interval, min, clines);       
                    }
                    // special case for first row
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(h)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(g)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y - cellSize/2,
                                       e, X, Y - cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval, min, clines);       
                    }
                    // special case for top right cell
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(f)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(a1)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(b1)))
                    {
                      ProcessRectangle(a, X - cellSize, Y+cellSize+cellSize/2,
                                       d, X - cellSize, Y,
                                       e, X + cellSize/2, Y, 
                                       b, X + cellSize/2, Y+cellSize+cellSize/2,
                                       interval, min, clines);       
                    }
                    // special case for last column
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(f)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(c)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y,
                                       e, X + cellSize/2, Y, 
                                       b, X + cellSize/2, Y + cellSize, 
                                       interval, min, clines);       
                    }
                    // special case for top row
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(a1)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(b1)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize+cellSize/2,
                                       d, X - cellSize, Y,
                                       e, X, Y, 
                                       b, X, Y + cellSize + cellSize/2,
                                       interval, min, clines);       
                    }
                    // normal case
                    else
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y,
                                       e, X, Y, 
                                       b, X, Y + cellSize, 
                                       interval, min, clines);       
                    }
                  }
                  else
                  {  
                    // determine which cells have defined values, accumulate 
                    // values and divide through number of valid cells
                    double sum = 0;
                    int good = 0;
                    double center = 0;
      
                    if (!(SourceTypeProperties::TypeProperties::
                                                IsUndefinedValue(a)))
                    {
                      sum += a;
                      good++;
                    }
    
                    if (!(SourceTypeProperties::TypeProperties::
                                                 IsUndefinedValue(b)))
                    {
                      sum += b;
                      good++;
                    }
    
                    if (!(SourceTypeProperties::TypeProperties::
                                                IsUndefinedValue(d)))
                    {
                      sum += d;
                      good++;
                    }
    
                    if (!(SourceTypeProperties::TypeProperties::
                                                IsUndefinedValue(e)))
                    {
                      sum += e;
                      good++;
                    }
    
                    center = sum / good;
  
                    // calculate alternative values
                    double top;
                    double left;
                    double right;
                    double bottom;
  
                    if(!(SourceTypeProperties::TypeProperties::
                                               IsUndefinedValue(a)))
                    {
                      if(!(SourceTypeProperties::TypeProperties::
                                                 IsUndefinedValue(b)))
                        top = (a + b) / 2.0;
                      else
                        top = a;
  
                      if(!(SourceTypeProperties::TypeProperties::
                                                 IsUndefinedValue(d)))
                        left = (a + d) / 2.0;
                      else
                        left = a;
                    }
                    else
                    {
                      if (!(SourceTypeProperties::TypeProperties::
                                                  IsUndefinedValue(b)))
                        top = b;
                      else
                        top = e;

                      if (!(SourceTypeProperties::TypeProperties::
                                                  IsUndefinedValue(d)))
                        left = d;
                      else
                        left = e;
                    }
  
                    if(!(SourceTypeProperties::TypeProperties::
                                               IsUndefinedValue(b)))
                      right = (e + b) / 2.0;
                    else
                      right = e;
      
                    if(!(SourceTypeProperties::TypeProperties::
                                               IsUndefinedValue(d)))
                      bottom = (e + d) / 2.0;
                    else
                      bottom = e;
  
                    // if one cell is not defined
                    // -> calculation with alternative values
                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(a)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       left, X - cellSize, Y + cellSize/2,
                                       center, X - cellSize/2, Y + cellSize/2,
                                       top, X - cellSize/2, Y + cellSize, 
                                       interval, min, clines);
                    }
              
                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(d)))
                    {
                      if ((SourceTypeProperties::TypeProperties::
                           IsUndefinedValue(f)) && 
                          (SourceTypeProperties::TypeProperties::
                           IsUndefinedValue(b)))
                      {
                        // special case top right cell
                      }
                      else if (!(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(e)) && 
                                !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(d)) &&
                                (SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(a)) && 
                               !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(b)))
                      {
                        // special case cell under undefined cell
                        ProcessRectangle(left, X - cellSize, Y + cellSize/2, 
                                         d, X - cellSize, Y,
                                         bottom, X - cellSize/2, Y, 
                                         center, X - cellSize/2, Y + cellSize/2,
                                         interval, min, clines);
                      }
                      else if (!(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(e)) && 
                               !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(d)) &&
                               !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(a)) && 
                                (SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(b)))
                      {
                        // special case cell left under undefined cell
                        ProcessRectangle(left, X - cellSize, Y + cellSize/2, 
                                         d, X - cellSize, Y,
                                         bottom, X - cellSize/2, Y, 
                                         center, X - cellSize/2, Y + cellSize/2,
                                         interval, min, clines);
                      }
                    }
            
                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(e)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(h)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(d)))
                    {
                      // special case left bottom cell
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(a)) &&
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b1)))
                    {
                      // special case top left cell
                      ProcessRectangle(b, X-cellSize/2, Y+cellSize+cellSize/2,
                                       bottom, X - cellSize/2, Y,
                                       e, X, Y, 
                                       b, X, Y + cellSize + cellSize/2, 
                                       interval, min, clines);
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(a)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)))
                    {
                      // special case top row
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(f)))
                    {
                      // special case right top cell
                      ProcessRectangle(center, X - cellSize/2, Y + cellSize/2, 
                                       bottom, X - cellSize/2, Y,
                                       e, X, Y, 
                                       right, X, Y + cellSize/2,   
                                       interval, min, clines);
                    }

                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(e)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(a)) &&
                        !(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(b)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(b1)) &&
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(a1)))
                    {
                      // special case left top cell
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(h)))
                    {
                      ProcessRectangle(top, X - cellSize/2, Y + cellSize, 
                                       e, X - cellSize/2, Y - cellSize/2,
                                       e, X, Y - cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval, min, clines);
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(d)) && 
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b1)))
                    {
                      ProcessRectangle(top, X - cellSize/2, Y + cellSize, 
                                       center, X - cellSize/2, Y + cellSize/2,
                                       right, X, Y + cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval, min, clines);
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(a)))
                    {
                      // special case right of undefined
                      ProcessRectangle(top, X - cellSize/2, Y + cellSize, 
                                       center, X - cellSize/2, Y + cellSize/2,
                                       right, X, Y + cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval, min, clines);
                    }
                  }
                }//if e def
              }// for
            }// for 

            currentTuple++;
          }

          // change of tile rows
          last = current;
          current = next;
          next.clear();

          currentSize = current.size();

          if (newLine == true)
          {
            next.push_back(nextElement);
            newLine = false;
          }

          currentTuple = 0;
          readNextElement = true;
        }

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

                int j = 2;

                while ((temp.level == -32000) && (j <= i))
                {
                  pResultInfo->Get(i-j,temp);
                  pResultInfo->resize(i-j);

                  j++;
                }

                if ( temp.level == -32000 )
                {
                  result.addr = 0;
                  return CANCEL;
                }

                CcInt* level = new CcInt(true,temp.level);
                Line* line = temp.cline;
                line->EndBulkLoad();

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
        else
        {
          result.addr = 0;
          return CANCEL;
        }      
      }

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

        qp->Close(args[0].addr);

        return 0;        
      }

      default:
      {
        assert(false);
      }
    }
 
    return returnValue;
  }
/*
declaration of contourFuns array

*/
  ValueMapping contourFuns[] =
  {
    contourFun<raster2::sint>,
    contourFun<raster2::sreal>,
    contourFunTile<TileAlgebra::tint, TileAlgebra::tProperties<int> >,
    contourFunTile<TileAlgebra::treal, TileAlgebra::tProperties<double> >,
    0
  };

/*
Value Mapping

*/
  int contourSelectFun(ListExpr args)
  {
    int nSelection = -1;
    
    NList type(args);

    if (type.first() == NList(raster2::sint::BasicType()))
    {
      return 0;
    }
    
    else if (type.first() == NList(raster2::sreal::BasicType()))
    {
      return 1;
    }

    ListExpr stream = nl->First(args);
    NList list = nl->Second(nl->First(nl->Second(nl->Second(stream))));

    if (list == TileAlgebra::tint::BasicType())
    {
      return 2;
    }

    if (list == TileAlgebra::treal::BasicType())
    {
      return 3;
    }

    return nSelection;
  }

/*
Type Mapping

*/
  ListExpr contourTypeMap(ListExpr args)
  {

    string error = "Expecting an sint, sreal or a stream of "
                   "tint or treal and an integer (interval).";

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

    if ((type == NList(raster2::sint::BasicType(), CcInt::BasicType())) ||
        (type == NList(raster2::sreal::BasicType(), CcInt::BasicType())))
    {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                             attrList));
    }

    ListExpr stream = nl->First(args);
    NList attr = nl->Second(args);

    if(!listutils::isTupleStream(stream))
    {
      return listutils::typeError(error);
    }

    NList list = nl->Second(nl->First(nl->Second(nl->Second(stream))));

    if((list == TileAlgebra::tint::BasicType() && attr == CcInt::BasicType()) ||
       (list == TileAlgebra::treal::BasicType() && attr == CcInt::BasicType()))
    {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                             attrList));
    }

    return listutils::typeError(error);
  }

/*
Method ProcessRectangle returns true if processing was successful

Parameters: values and coordinates of four points, the wanted interval, 
            the minimum value and DbArray ResultInfo to store the found 
            segments

*/
  bool ProcessRectangle(double a, double aX, double aY,
                        double g, double gX, double gY,
                        double i, double iX, double iY,
                        double c, double cX, double cY, 
                        int interval, double min, 
                        DbArray<ResultInfo>* clines)
  {
    // calculate minimum and maximum
    double Min = MIN(MIN(a,c),MIN(g,i));
    double Max = MAX(MAX(a,c),MAX(g,i));

    int startLevel = (int) floor(Min / interval);
    int endLevel = (int) ceil(Max / interval);

    // calculate intersection
    for(int iLevel = startLevel; iLevel <= endLevel; iLevel++)
    {
      int level = iLevel * interval;

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

      if( nPoints == 2 )
      {
        // left and bottom
        if ( nPoints1 == 1 && nPoints2 == 2)
        {
          if ( !(g == level && i == level) )
            AddSegment( level, pointsX[0], pointsY[0], 
                        pointsX[1], pointsY[1], min, interval, clines);
        }
        // left and right
        else if ( nPoints1 == 1 && nPoints3 == 2 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], min, interval, clines);
        }
        // left and top
        else if ( nPoints1 == 1 && nPoints == 2 )
        { 
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], min, interval, clines);
        }
        // bottom and right
        else if(  nPoints2 == 1 && nPoints3 == 2)
        {
          if ( !(c == level && i == level) )
            AddSegment( level, pointsX[0], pointsY[0], 
                        pointsX[1], pointsY[1], min, interval, clines);
        }
        // bottom and top
        else if ( nPoints2 == 1 && nPoints == 2 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], min, interval, clines);
        }
        // right and top
        else if ( nPoints3 == 1 && nPoints == 2 )
        { 
           AddSegment( level, pointsX[0], pointsY[0], 
                       pointsX[1], pointsY[1], min, interval, clines);
        }
        else
        {
          return error;
        }
      }

      if( nPoints == 3 )
      {
        // left, bottom and right
        if ( nPoints1 == 1 && nPoints2 == 2 && nPoints3 == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], min, interval, clines);

          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], min, interval, clines);
        }
        // left, bottom and top
        else if ( nPoints1 == 1 && nPoints2 == 2 && nPoints == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], min, interval, clines);

          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[2], pointsY[2], min, interval, clines);
        }
        // bottom, right and top
        else if ( nPoints2 == 1 && nPoints3 == 2 && nPoints == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], min, interval, clines);

          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], min, interval, clines);
        }
        // left, right and top
        else if ( nPoints1 == 1 && nPoints3 == 2 && nPoints == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], min, interval, clines);

          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], min, interval, clines);
        }
      }

      if( nPoints == 4 )
      {
        if ( !(c == level && a == level) )
        {
          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], min, interval, clines);
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[3], pointsY[3], min, interval, clines);
        }
      }
    } 
    return false;
  }

/*
Method Intersects calculates if a line between two points intersects the level

Parameters: values and coordinates of two points, value of a third point, 
            the level value, a pointer for a counter to store the number 
            of intersect and two pointer two store the point where the line is
            intersected

*/
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

/*
Method AddSegment addes the found segments to the ResultInfo DbArray

Parameters: level value, coordinates of segments start and stop point, 
            the minimum value, the interval value and DbArray ResultInfo 
            to store the segments

*/
  bool AddSegment(int l, double startX, double startY,
                  double endX, double endY, 
                  double min, int interval, DbArray<ResultInfo>* clines)
  {
    Point p1(true, startX, startY);
    Point p2(true, endX, endY);
    HalfSegment hs(true, p1, p2);

    // calculate array element
    int i = floor((l - min) / interval);

    // find correct contour line
    ResultInfo temp;
    clines->Get(i,temp);

    // if contour line exists, add segment
    if (!(temp.level == -32000))
    {            
      Line* line = temp.cline;

      int s = line->Size();

      hs.attr.edgeno = s/2;
      (*line) += hs;
      hs.SetLeftDomPoint(false);
      (*line) += hs;
    }
    else
    // if not, create new line
    {
      Line* line = new Line(0);

      line->StartBulkLoad();

      hs.attr.edgeno = 0;
      (*line) += hs;
      hs.SetLeftDomPoint(false);
      (*line) += hs;

      int l2 = static_cast<int>(l);

      ResultInfo lines;

      lines.level = l2;
      lines.cline = line;

      clines->Put(i,lines);
    }

    return true;
  }

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
  void GetValuesContour(double* a, double* a1, double* b, double* b1, double* c,
               double* d, double* f, double* g, double* h,
               int row, int column, int currentTuple, T* s_in,
               int maxX, int maxY,
               int factorNext, int factorLast,
               bool skipNextRow, bool skipLastRow,
               vector<Tuple*> current, vector<Tuple*> next,
               vector<Tuple*> last, 
               int currentSize, int nextSize, int lastSize)
  {
    Tuple* tuple_help;
    T* s_in_help;

    // left lower corner
    if ((column == 0) && (row == 0))
    {
      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *a = s_in_help->GetValue((int[]){maxX, 1});
          *a1 = s_in_help->GetValue((int[]){maxX, 2});
          *d = s_in_help->GetValue((int[]){maxX, 0});
        }
      }

      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *f = s_in->GetValue((int[]){column + 1, row});
  
      if ((lastSize > 0) && (skipLastRow == false))
      {
        if ((currentTuple + factorLast > 0) &&
            (currentTuple + factorLast - 1 < lastSize))
        {  
          tuple_help = last[currentTuple - 1 + factorLast];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *g = s_in_help->GetValue((int[]){maxX, maxY});
          }
        }
  
        if ((currentTuple + factorLast >= 0) &&
            (currentTuple + factorLast < lastSize))
        {
          tuple_help = last[currentTuple + factorLast];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *h = s_in_help->GetValue((int[]){0, maxY});
          }
        }
      }
    }
    // left upper corner - 1
    else if ((column == 0) && (row == maxY - 1))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext > 0) &&
            (currentTuple + factorNext - 1 < nextSize))
        {  
          tuple_help = next[currentTuple - 1 + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *a1 = s_in_help->GetValue((int[]){maxX, 0});
          }
        }
  
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *b1 = s_in_help->GetValue((int[]){0, 0});
          }
        }
      }

      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];
  
        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
          *a = s_in_help->GetValue((int[]){maxX, maxY});
          *d = s_in_help->GetValue((int[]){maxX, maxY - 1});
          *g = s_in_help->GetValue((int[]){maxX, maxY - 2});
        }
      }
  
      *b = s_in->GetValue((int[]){column, row + 1});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *f = s_in->GetValue((int[]){column + 1, row});
      *h = s_in->GetValue((int[]){column, row - 1});
    }
    // left upper corner  
    else if ((column == 0) && (row == maxY))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext > 0) &&
            (currentTuple + factorNext - 1 < nextSize))
        {  
          tuple_help = next[currentTuple - 1 + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *a = s_in_help->GetValue((int[]){maxX, 0});
            *a1 = s_in_help->GetValue((int[]){maxX, 1});
          }
        }
  
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *b = s_in_help->GetValue((int[]){0, 0});
            *b1 = s_in_help->GetValue((int[]){0, 1});
            *c = s_in_help->GetValue((int[]){1, 0});
          }
        }
      }

      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *d = s_in_help->GetValue((int[]){maxX, maxY});
          *g = s_in_help->GetValue((int[]){maxX, maxY - 1});
        }
      }

      *f = s_in->GetValue((int[]){column + 1, row});
      *h = s_in->GetValue((int[]){column, row - 1});
    }
    // right lower corner
    else if ((column == maxX) && (row == 0))
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *d = s_in->GetValue((int[]){column - 1, row});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *c = s_in_help->GetValue((int[]){0, 1});
          *f = s_in_help->GetValue((int[]){0, 0});
        }
      }

      if ((lastSize > 0) && (skipLastRow == false))
      {
        if ((currentTuple + factorLast >= 0) &&
            (currentTuple + factorLast < lastSize))
        {
          tuple_help = last[currentTuple + factorLast];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *g = s_in_help->GetValue((int[]){maxX - 1, maxY});
            *h = s_in_help->GetValue((int[]){maxX, maxY});
          }
        }
      }
    }
    // right upper corner - 1
    else if ((column == maxX) && (row == maxY - 1))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {  
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *a1 = s_in_help->GetValue((int[]){maxX - 1, 0});
            *b1 = s_in_help->GetValue((int[]){maxX, 0});
          }
        }
      }

      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *b = s_in->GetValue((int[]){column, row + 1});
      *d = s_in->GetValue((int[]){column - 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
          *c = s_in_help->GetValue((int[]){0, maxY});
          *f = s_in_help->GetValue((int[]){0, maxY - 1});
        }
      }
    }
    // right upper corner
    else if ((column == maxX) && (row == maxY))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *a = s_in_help->GetValue((int[]){maxX - 1, 0});
            *a1 = s_in_help->GetValue((int[]){maxX - 1, 1});
            *b = s_in_help->GetValue((int[]){maxX, 0});
            *b1 = s_in_help->GetValue((int[]){maxX, 1});
          }  
        }

        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext + 1 < nextSize))
        {  
          tuple_help = next[currentTuple + 1 + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *c = s_in_help->GetValue((int[]){0, 0});
          }
        }
      }

      *d = s_in->GetValue((int[]){column - 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *f = s_in_help->GetValue((int[]){0, maxY});
        }
      }
    }
    // left column
    else if (column == 0)
    {
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *f = s_in->GetValue((int[]){column + 1, row});  
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *a = s_in_help->GetValue((int[]){maxX, row + 1});
          *a1 = s_in_help->GetValue((int[]){maxX, row + 2});
          *d = s_in_help->GetValue((int[]){maxX, row});
          *g = s_in_help->GetValue((int[]){maxX, row - 1});
        }
      }
    }
    // lower row
    else if (row == 0)
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});

      if ((lastSize > 0)  && (skipLastRow == false))
      {
        if ((currentTuple + factorLast >= 0) &&
            (currentTuple + factorLast < lastSize))
        {
          tuple_help = last[currentTuple + factorLast];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *g = s_in_help->GetValue((int[]){column - 1, maxY});
            *h = s_in_help->GetValue((int[]){column, maxY});
          }
        }
      }
    }
    // right column
    else if (column == maxX)
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *d = s_in->GetValue((int[]){column - 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *c = s_in_help->GetValue((int[]){0, row + 1});
          *f = s_in_help->GetValue((int[]){0, row});
        }
      }
    }
    // upper row - 1
    else if (row == maxY - 1)
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *b = s_in->GetValue((int[]){column, row + 1});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *a1 = s_in_help->GetValue((int[]){column - 1, 0});
            *b1 = s_in_help->GetValue((int[]){column, 0});
          }
        }
      }
    }
    // upper row
    else if (row == maxY)
    {
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *a = s_in_help->GetValue((int[]){column - 1, 0});
            *a1 = s_in_help->GetValue((int[]){column - 1, 1});
            *b = s_in_help->GetValue((int[]){column, 0});
            *b1 = s_in_help->GetValue((int[]){column, 1});
            *c = s_in_help->GetValue((int[]){column + 1, 0});
          }
        }
      }
    }
    // no border cells
    else
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});  
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});
    }
  }
}
