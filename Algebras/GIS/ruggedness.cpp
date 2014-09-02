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
#include "ruggedness.h"
#include "getValues.h"

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
Method ruggednessFuns: calculates the ruggedness value for each cell of a sint 
                       or sreal. For each cell the value and the values of the 
                       eight neighbour cells is read. 

Return value: sint or sreal

*/
  template <typename T>
  int ruggednessFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    typename T::this_type* s_out =
          static_cast<typename T::this_type*>(result.addr);
    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);

    raster2::grid2 grid = s_in->getGrid();

    s_out->setGrid(grid);

    Rectangle<2> bbox = s_in->bbox();

    raster2::RasterIndex<2> from = grid.getIndex(bbox.MinD(0), bbox.MinD(1));
    raster2::RasterIndex<2> to = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

    // sometimes for small cellsize the index is calculated wrong
    to[0]++;
    to[1]++;

    for (raster2::RasterIndex<2> index=from; index < to; 
                                             index.increment(from, to))
    {
        // central cell
        double e = s_in->get(index);

        if(!(s_in->isUndefined(e)))
        {
          double a = s_in->get((int[]){index[0] - 1, index[1] + 1});
          if (s_in->isUndefined(a)){a = e;}
          double b = s_in->get((int[]){index[0], index[1] + 1});
          if (s_in->isUndefined(b)){b = e;}
          double c = s_in->get((int[]){index[0] + 1, index[1] + 1});
          if (s_in->isUndefined(c)){c = e;}
          double d = s_in->get((int[]){index[0] - 1, index[1]});
          if (s_in->isUndefined(d)){d = e;}
          double f = s_in->get((int[]){index[0] + 1, index[1]});
          if (s_in->isUndefined(f)){f = e;}
          double g = s_in->get((int[]){index[0] - 1, index[1] - 1});
          if (s_in->isUndefined(g)){g = e;}
          double h = s_in->get((int[]){index[0], index[1] - 1});
          if (s_in->isUndefined(h)){h = e;}
          double i = s_in->get((int[]){index[0] + 1, index[1] - 1});
          if (s_in->isUndefined(i)){i = e;}

          // calculate difference between central and surrounding cells, 
          // square values
          double diff1 = (a - e) * (a - e);
          double diff2 = (b - e) * (b - e);
          double diff3 = (c - e) * (c - e);
          double diff4 = (d - e) * (d - e);
          double diff5 = (f - e) * (f - e);
          double diff6 = (g - e) * (g - e);
          double diff7 = (h - e) * (h - e);
          double diff8 = (i - e) * (i - e);

          // accumulate values and take square root
          double ruggedness = sqrt(diff1 + diff2 + diff3 + diff4 + 
                                   diff5 + diff6 + diff7 + diff8);

          s_out->set(index, ruggedness);
        }
    }

    return 0;
  }

/*
Method ruggednessFunsTile: calculates the ruggedness value for each cell of a 
                           stream of tint or treal. For each cell the value and
                           the values of the eight neighbour cells is read

Return value: stream of tint or treal

*/
  template <typename T, typename SourceTypeProperties>
  int ruggednessFunTile(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    int returnValue = FAILURE;

    struct ResultInfo
    {
      T* s_in;
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
      bool firstTuple;
      bool readNextElement;
      bool newLine;
      bool skipNextRow;
      bool skipLastRow;
      int currentSize;
      int nextSize;
      int lastSize;
      int currentTuple;
      double lastOriginX;
    };

    int xDimensionSize = TileAlgebra::tProperties<char>::GetXDimensionSize();
    int yDimensionSize = TileAlgebra::tProperties<char>::GetYDimensionSize();

    int maxX = xDimensionSize - 1;
    int maxY = yDimensionSize - 1;

    switch(message)
    {
      case OPEN:
      {
        // initialize the local storage
        ResultInfo* info = new ResultInfo;

        info->firstTuple = true;
        info->readNextElement = true;

        info->skipNextRow = false;
        info->skipLastRow = false;

        info->currentTuple = 0;

        local.addr = info;

        qp->Open(args[0].addr);

        Word elem;
        Tuple* tuple;

        T* s_in;

        double gridOriginX;
        double gridOriginY;
        double lastOriginY;

        bool readElement = true;
        info->newLine = false;

        while (readElement == true)
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
            info->cellSize = grid.GetLength();
            info->tileSize = info->cellSize * xDimensionSize;

            // read cells until Y coordinate changes
            if ((gridOriginY == lastOriginY) || (info->firstTuple == true))
            {
              if (!(info->firstTuple == true))
              {
                // if there is a gap between two read tiles, fill vector
                // with dummy tile
                while ((gridOriginX - info->lastOriginX) - info->tileSize 
                                                             > info->cellSize)
                {
                  TupleType *tupleType = tuple->GetTupleType();
                  Tuple* dummy = new Tuple( tupleType );
                  T* s_out = new T(true);
                  dummy->PutAttribute(0,s_out);

                  info->current.push_back(dummy);
                  info->lastOriginX = info->lastOriginX + info->tileSize;
                }
              }
              info->current.push_back(tuple);
              info->lastOriginX = gridOriginX;
              lastOriginY = gridOriginY;
              info->firstTuple = false;
            }
            else
            {
              readElement = false;
              info->firstTuple = true;
              info->next.push_back(tuple);
              info->lastOriginX = gridOriginX;
            }
          }
          else
          {
            readElement = false;
          }
        } // while

        return 0;
      }

      case REQUEST:
      {
        if(local.addr != 0)
        {
          ResultInfo* info = static_cast<ResultInfo*>(local.addr);

          Word elem;
          Tuple* tuple;

          T* s_in;

          double gridOriginX;
          double gridOriginY;
          double lastOriginY;

          while (info->readNextElement == true)
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
              info->cellSize = grid.GetLength();

              // read cells until Y coordinate changes
              if ((gridOriginY == lastOriginY) || (info->firstTuple == true))
              {
                // if there is a gap between two read tiles, fill vector
                // with dummy tile
                while ((gridOriginX - info->lastOriginX) - info->tileSize 
                                                             > info->cellSize)
                {
                  TupleType *tupleType = tuple->GetTupleType();
                  Tuple* dummy = new Tuple( tupleType );
                  T* s_out = new T(true);
                  dummy->PutAttribute(0,s_out);
                  info->next.push_back(dummy);
                  info->lastOriginX = info->lastOriginX + info->tileSize;
                }

                info->next.push_back(tuple);
                info->lastOriginX = gridOriginX;
                lastOriginY = gridOriginY;
                info->firstTuple = false;
              }
              else
              {
                info->readNextElement = false;
                info->firstTuple = true;
                info->newLine = true;

                info->nextElement = tuple;
                info->lastOriginX = gridOriginX;
              }
            }
            else
            {
              info->readNextElement = false;
            }
          } // while

          T* s_out = new T(true);

          info->currentSize = info->current.size();
          info->nextSize = info->next.size();
          info->lastSize = info->last.size();

          if (info->currentSize == 0)
          {
            return CANCEL;
          }

          int factorNext = 0;
          int factorLast = 0;

          info->skipNextRow = false;
          info->skipLastRow = false;

          Tuple* cTuple = info->current[0];
          T* c = static_cast<T*>(cTuple->GetAttribute(0));
          TileAlgebra::tgrid cGrid;
          c->getgrid(cGrid);
          double cGridOriginX = cGrid.GetX();
          double cGridOriginY = cGrid.GetY();

          if (info->nextSize > 0)
          {
            Tuple* nTuple = info->next[0];
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
                nGridOriginX = nGridOriginX + info->tileSize;              
              }
            }
            else if ((cGridOriginX - nGridOriginX) < 0)
            {
              while ((cGridOriginX - nGridOriginX) < 0)
              {
                factorNext--;
                cGridOriginX = cGridOriginX + info->tileSize;              
              }
            }

            // check if one or more rows are missing
            if ((nGridOriginY - cGridOriginY) > info->tileSize)
            {
              info->skipNextRow = true;
            }
          }

          if (info->lastSize > 0)
          {
            Tuple* lTuple = info->last[0];
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
                lGridOriginX = lGridOriginX + info->tileSize;              
              }
            }
            else if ((cGridOriginX - lGridOriginX) < 0)
            {
              while ((cGridOriginX - lGridOriginX) < 0)
              {
                factorLast--;
                cGridOriginX = cGridOriginX + info->tileSize;              
              }
            }

            // check if one or more rows are missing
            if ((cGridOriginY - lGridOriginY) > info->tileSize)
            {
              info->skipLastRow = true;
            }          
          }

          // read tile from vector
          while (info->currentTuple < info->currentSize)
          {
            tuple = info->current[info->currentTuple];

            if ((tuple == 0) || (tuple->GetNoAttributes() != 1))
            {
              while (tuple == 0)
              {
                info->currentTuple++;
                tuple = info->current[info->currentTuple];
              }

              while (tuple->GetNoAttributes() != 1)
              {
                info->currentTuple++;
                tuple = info->current[info->currentTuple];
              }
            }

            s_in = static_cast<T*>(tuple->GetAttribute(0));

            TileAlgebra::Index<2> from;
            TileAlgebra::Index<2> to;

            s_in->GetBoundingBoxIndexes(from, to);
            info->fromX = from[0];
            info->fromY = from[1];
            info->toX = to[0];
            info->toY = to[1];      

            TileAlgebra::tgrid grid;
            s_in->getgrid(grid);
            s_out->SetGrid(grid);

            info->cellSize = grid.GetLength();

            bool bHasDefinedValue = false;

            for(int row = info->fromY; row <= info->toY; row++)
            {
              for(int column = info->fromX; column <= info->toX; column++)
              {
                TileAlgebra::Index<2> index((int[]){column, row});

                // central cell
                double e = s_in->GetValue(index);

                if(SourceTypeProperties::TypeProperties::
                                         IsUndefinedValue(e) == false)
                {
                  bHasDefinedValue = true;

                  double a = 0;
                  double b = 0;
                  double c = 0;
                  double d = 0;
                  double f = 0;
                  double g = 0;
                  double h = 0;
                  double i = 0;

                  GetValues<T, SourceTypeProperties>
                    (&a, &b, &c, &d, &e, &f, &g, &h, &i, row, column, 
                     info->currentTuple, s_in,
                     maxX, maxY, factorNext, factorLast, 
                     info->skipNextRow, info->skipLastRow,
                     info->current, info->next, info->last,
                     info->currentSize, info->nextSize, info->lastSize);

                  // calculate difference between central and surrounding 
                  // cells, square values
                  double diff1 = (a - e) * (a - e);
                  double diff2 = (b - e) * (b - e);
                  double diff3 = (c - e) * (c - e);
                  double diff4 = (d - e) * (d - e);
                  double diff5 = (f - e) * (f - e);
                  double diff6 = (g - e) * (g - e);
                  double diff7 = (h - e) * (h - e);
                  double diff8 = (i - e) * (i - e);

                  // accumulate values and take square root
                  double ruggedness = sqrt(diff1 + diff2 + diff3 + diff4 + 
                                           diff5 + diff6 + diff7 + diff8);

                  s_out->SetValue(index, ruggedness, true);
                }
              }
            }

            info->currentTuple++;

            // change of tile rows
            if (!(info->currentTuple < info->currentSize))
            {
              info->last = info->current;
              info->current = info->next;
              info->next.clear();

              if (info->newLine == true)
              {
                info->next.push_back(info->nextElement);
                info->newLine = false;
              }

              info->currentTuple = 0;
              info->readNextElement = true;
            }                  

            if(bHasDefinedValue == true)
            {
              // return the next stream element
              ListExpr resultType = GetTupleResultType(s);
              TupleType *tupleType = new TupleType(nl->Second(resultType));
              Tuple *slope_out = new Tuple( tupleType );
              slope_out->PutAttribute(0,s_out);
              result.addr = slope_out;
              return YIELD;
            }
          } // while currentTuple
        } // if local.addr
        else
        {
          // always set the result to null before return CANCEL
          result.addr = 0;
          return CANCEL;
        }
      } //REQUEST

      case CLOSE:
      {
        if(local.addr != 0)
        {
          ResultInfo* info = static_cast<ResultInfo*>(local.addr);

          if(info != 0)
          {
            delete info;
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
definition of ruggednessFuns array

*/
  ValueMapping ruggednessFuns[] =
  {
    ruggednessFun<raster2::sint>,
    ruggednessFun<raster2::sreal>,
    ruggednessFunTile<TileAlgebra::tint, TileAlgebra::tProperties<int>>,
    ruggednessFunTile<TileAlgebra::treal, TileAlgebra::tProperties<double>>,
    0
  };

/*
Value Mapping 

*/
  int ruggednessSelectFun(ListExpr args)
  {
    int nSelection = -1;
    
    NList type(args);

    if (type.first() == NList(raster2::sint::BasicType()))
    {
      return 0;
    }

    if (type.first() == NList(raster2::sreal::BasicType()))
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
  ListExpr ruggednessTypeMap(ListExpr args)
  {
    string error = "Expecting an sint, sreal or a stream of tint or treal";

    NList type(args);

    if(type.length() != 1)
    {
      return NList::typeError("one argument required");
    }

    if (type.first() == NList(raster2::sint::BasicType()))
    {
      return NList(raster2::sint::BasicType()).listExpr();
    }

    else if(type.first() == NList(raster2::sreal::BasicType())) 
    {
      return NList(raster2::sreal::BasicType()).listExpr();
    }

    ListExpr stream = nl->First(args);

    if(!listutils::isTupleStream(stream))
    {
      return listutils::typeError(error);
    }

    NList list = nl->Second(nl->First(nl->Second(nl->Second(stream))));

    ListExpr attrList=nl->TheEmptyList();

    if(list == TileAlgebra::tint::BasicType())
    {
      ListExpr attr1 = nl->TwoElemList( nl->SymbolAtom("Ruggedness"),
                       nl->SymbolAtom(TileAlgebra::tint::BasicType()));

      attrList = nl->OneElemList( attr1 );

      return nl->TwoElemList(
             nl->SymbolAtom(Stream<Tuple>::BasicType()),
             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList));
    }

    else if(list == TileAlgebra::treal::BasicType())
    {
      ListExpr attr1 = nl->TwoElemList( nl->SymbolAtom("Ruggedness"),
                       nl->SymbolAtom(TileAlgebra::treal::BasicType()));

      attrList = nl->OneElemList( attr1 );

      return nl->TwoElemList(
             nl->SymbolAtom(Stream<Tuple>::BasicType()),
             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList));
    }

    return listutils::typeError(error);
  }
}
