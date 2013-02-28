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

#include "fromRegion.h"
#include "fromLine.h"
#include "RobustSetOps.h"

namespace raster2 {

  bool intersection(double x1, double y1, double x2, double y2,
                    double bx1, double by1, double bx2, double by2)
  {
      double z1;
      double z2;

      double d = (y2 - y1) * (bx1 - bx2) - (by2 - by1) * (x1 - x2);

      if ( d == 0 )
      {
          return false;
      }

      z1 = ((x1 - x2) * (bx2 * by1 - bx1 * by2) - (bx1 - bx2) *
           (x2 * y1 - x1 * y2)) / d;
      z2 = ((by2 - by1) * (x2 * y1 - x1 * y2) - (y2 - y1) *
           (bx2 * by1 - bx1 * by2)) / d;

      /*
        wenn y1 > y2 muss die Ueberpruefung des Schnittpunkts fuer
        z2 gedreht sein

       */
      if ( y1 < y2 )
      {
          if ( (z1 > x1 || AlmostEqual(z1, x1))
            && (z1 < x2 || AlmostEqual(z1, x2))
            && (z1 > bx1 || AlmostEqual(z1, bx1))
            && (z1 < bx2 || AlmostEqual(z1, bx2))
            && (z2 > y1 || AlmostEqual(z1, y1))
            && (z2 < y2 || AlmostEqual(z1, y2))
            && (z2 > by1 || AlmostEqual(z1, by1))
            && (z2 < by2 || AlmostEqual(z1, by2)))
          {
              return true;
          }
      }
      else
      {
          if ( (z1 > x1 || AlmostEqual(z1, x1))
            && (z1 < x2 || AlmostEqual(z1, x2))
            && (z1 > bx1 || AlmostEqual(z1, bx1)) 
            && (z1 < bx2 || AlmostEqual(z1, bx2)) 
            && (z2 > y2 || AlmostEqual(z2, y2))
            && (z2 < y1 || AlmostEqual(z2, y1))
            && (z2 > by1 || AlmostEqual(z2, by1)) 
            && (z2 < by2 || AlmostEqual(z2, by2)))
          {
              return true;
          }
      }

      return false;
  }

    int fromRegionFun
            (Word* args, Word& result, int message, Word& local, Supplier s)
    {
        result = qp->ResultStorage(s);
                
        sbool* ergebnis = static_cast<sbool*>(result.addr);
        Region* region = static_cast<Region*>(args[0].addr);
        grid2* grid = static_cast<grid2*>(args[1].addr);   

        if(!region->IsDefined() || 
           grid->getLength()<=0){
           ergebnis->setDefined(false);
           return 0; 
        } 

        ergebnis->clear();
                                                                       
        double gridOriginX = grid->getOriginX();
        double gridOriginY = grid->getOriginY();
        double length = grid->getLength();

        ergebnis->setGrid(*grid);

        bool setFields = false;

        int regionLength = region->Size();

        HalfSegment hs;
        Point mittelpunkt(true, 0.0, 0.0);

        region->SelectFirst();

        for ( int i = 0; i < regionLength; i++ )
        {
            region->GetHs(hs);
            region->SelectNext();

            /*
             Start- und Endpunkt des Segment bestimmen
        
             */
            Point point1 = hs.GetLeftPoint();
            Point point2 = hs.GetRightPoint();

            double x1 = point1.GetX();
            double y1 = point1.GetY();
            double x2 = point2.GetX();
            double y2 = point2.GetY();

            /*
             Raster bestimmen, damit die korrekten Felder gesetzt werden
        
            */
            double gridX = gridOriginX;
            double gridY = gridOriginY;
    
            if ( gridX < x1 )
            {
                while ( gridX < x1 )
                {
                    gridX = gridX + length;
                }
            
                if ( gridX > x1 )
                {
                    gridX = gridX - length;
                }
            }
            else if ( gridX > x1 )
            {
                while ( gridX > x1 )
                {
                    gridX = gridX - length;
                }
            }
        
            if ( gridY < y1 )
            {
                while ( gridY < y1 )
                {
                    gridY = gridY + length;
                }
        
                if ( gridY > y1 )
                {
                    gridY = gridY - length;
                }
            }
            else if ( gridY > y1 )
            {
                while ( gridY > y1 )
                {
                    gridY = gridY - length;
                }
            }
        
            /*
             Alle Felder die von der Begrenzungslinie geschnitten werden
             und deren Mittelpunkt in der Region liegt, auf true setzen
        
            */

            /*
             Sonderfall: senkrechte Linie
         
             */
            if ( x1 == x2 && gridY < y2 )
            {
                while ( gridY < y2 )
                {
                  mittelpunkt.Set(gridX + length/2, gridY + length/2);
    
                  if ( robust::contains(*region, mittelpunkt) == true )
                  {
                    ergebnis->setatlocation(gridX,gridY,true);
                    setFields = true;
                  }
                  else
                  {
                    mittelpunkt.Set(gridX + length + length/2, 
                                            gridY + length/2);
    
                    if ( robust::contains(*region, mittelpunkt) == true )
                    {
                      ergebnis->setatlocation(gridX + length,gridY,true);
                      setFields = true;
                    }
                    else
                    {
                      mittelpunkt.Set(gridX - length + length/2, 
                                              gridY + length/2);
     
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(gridX - length,gridY,true);
                        setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX + length/2, 
                                                gridY + length + length/2);
    
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX,gridY + length,true);
                          setFields = true;
                        }
                        else
                        {
                          mittelpunkt.Set(gridX + length/2, 
                                                  gridY - length + length/2);
    
                          if ( robust::contains(*region, mittelpunkt) == true )
                          {
                            ergebnis->setatlocation(gridX,gridY - length,true);
                            setFields = true;
                          }
                        }
                      }
                    }
                  }

                  gridY = gridY + length;
                }
            }
            else if ( x1 == x2 && gridY > y2 )
            {
              while ( gridY > y2 )
              {
                mittelpunkt.Set(gridX + length/2, gridY + length/2);
   
                if ( robust::contains(*region, mittelpunkt) == true )
                {
                  ergebnis->setatlocation(gridX,gridY,true);
                  setFields = true;
                }
                else
                {
                  mittelpunkt.Set(gridX + length + length/2, 
                                          gridY + length/2);
    
                  if ( robust::contains(*region, mittelpunkt) == true )
                  {
                    ergebnis->setatlocation(gridX + length,gridY,true);
                    setFields = true;
                  }
                  else
                  {
                    mittelpunkt.Set(gridX - length + length/2, 
                                            gridY + length/2);
     
                    if ( robust::contains(*region, mittelpunkt) == true )
                    {
                      ergebnis->setatlocation(gridX - length,gridY,true);
                      setFields = true;
                    }
                    else
                    {
                      mittelpunkt.Set(gridX + length/2, 
                                              gridY + length + length/2);
    
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(gridX,gridY + length,true);
                        setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX + length/2, 
                                                gridY - length + length/2);
    
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX,gridY - length,true);
                          setFields = true;
                        }
                      }
                    }
                  }
                }
                    
                gridY = gridY - length;
              }
            }
        
            /*
             Linie von links oben nach rechts unten
         
             */
            if ( y1 > y2 )
            {
                while ( gridX < x2 )
                {
                  if( raster2::intersection(x1, y1, x2, y2, gridX + length,
                             gridY, gridX + length, gridY + length) )
                  {
                    mittelpunkt.Set(gridX + length/2, gridY + length/2);
    
                    if ( robust::contains(*region, mittelpunkt) == true )
                    {
                      ergebnis->setatlocation(gridX,gridY,true);
                      setFields = true;
                    }
                    else
                    {
                      mittelpunkt.Set(gridX + length + length/2, 
                                              gridY + length/2);
    
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(gridX + length,gridY,true);
                        setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX - length + length/2, 
                                                gridY + length/2);
     
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX - length,gridY,true);
                          setFields = true;
                        }
                        else
                        {
                          mittelpunkt.Set(gridX + length/2, 
                                                  gridY + length + length/2);
    
                          if ( robust::contains(*region, mittelpunkt) == true )
                          {
                            ergebnis->setatlocation(gridX,gridY + length,true);
                            setFields = true;
                          }
                          else
                          {
                            mittelpunkt.Set(gridX + length/2, 
                                                    gridY - length + length/2);

                            if (robust::contains(*region, mittelpunkt) == true)
                            {
                              ergebnis->setatlocation(gridX,gridY-length,true);
                              setFields = true;
                            }
                          }
                        }
                      }
                    }

                    gridX = gridX + length;
                  }
                  else if( raster2::intersection(x1, y1, x2, y2, gridX,
                                  gridY, gridX + length, gridY) )
                  {
                    mittelpunkt.Set(gridX + length/2, gridY + length/2);
    
                    if ( robust::contains(*region, mittelpunkt) == true )
                    {
                      ergebnis->setatlocation(gridX,gridY,true);
                      setFields = true;
                    }
                    else
                    {
                      mittelpunkt.Set(gridX + length + length/2, 
                                              gridY + length/2);
    
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(gridX + length,gridY,true);
                        setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX - length + length/2, 
                                                gridY + length/2);
     
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX - length,gridY,true);
                          setFields = true;
                        }
                        else
                        {
                          mittelpunkt.Set(gridX + length/2, 
                                                  gridY + length + length/2);
    
                          if ( robust::contains(*region, mittelpunkt) == true )
                          {
                            ergebnis->setatlocation(gridX,gridY + length,true);
                            setFields = true;
                          }
                          else
                          {
                            mittelpunkt.Set(gridX + length/2, 
                                                    gridY - length + length/2);

                            if (robust::contains(*region, mittelpunkt) == true)
                            {
                              ergebnis->setatlocation(gridX,gridY-length,true);
                              setFields = true;
                            }
                          }
                        }
                      }
                    }
                        
                    gridY = gridY - length;
                  }
                  else 
                  {
                    /*
                     Endfeld erreicht
                     
                    */
                    mittelpunkt.Set(gridX + length/2, gridY + length/2);
    
                    if ( robust::contains(*region, mittelpunkt) == true )
                    {
                      ergebnis->setatlocation(gridX,gridY,true);
                      setFields = true;
                    }
                    else
                    {
                      mittelpunkt.Set(gridX + length + length/2, 
                                              gridY + length/2);
    
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(gridX + length,gridY,true);
                        setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX - length + length/2, 
                                                gridY + length/2);
     
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX - length,gridY,true);
                          setFields = true;
                        }
                        else
                        {
                          mittelpunkt.Set(gridX + length/2, 
                                                  gridY + length + length/2);
    
                          if ( robust::contains(*region, mittelpunkt) == true )
                          {
                            ergebnis->setatlocation(gridX,gridY + length,true);
                            setFields = true;
                          }
                          else
                          {
                            mittelpunkt.Set(gridX + length/2, 
                                                    gridY - length + length/2);

                            if (robust::contains(*region, mittelpunkt) == true)
                            {
                              ergebnis->setatlocation(gridX,gridY-length,true);
                              setFields = true;
                            }
                          }
                        }
                      }
                    }
                        
                    gridX = x2;
                  }
                }    
            }
            /*
             Linie von links unten nach rechts oben
         
             */
            else
            {
                while ( gridX < x2 )
                {
                    if ( raster2::intersection(x1, y1, x2, y2, gridX,
                         gridY + length, gridX + length, gridY + length) )
                    {
                      mittelpunkt.Set(gridX + length/2, gridY + length/2);

                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                          ergebnis->setatlocation(gridX,gridY,true);
                          setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX + length + length/2, 
                                                gridY + length/2);
    
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX + length,gridY,true);
                          setFields = true;
                        }
                        else
                        {
                          mittelpunkt.Set(gridX - length + length/2, 
                                                  gridY + length/2);
     
                          if ( robust::contains(*region, mittelpunkt) == true )
                          {
                            ergebnis->setatlocation(gridX - length,gridY,true);
                            setFields = true;
                          }
                          else
                          {
                            mittelpunkt.Set(gridX + length/2, 
                                                    gridY + length + length/2);
    
                            if ( robust::contains(*region, mittelpunkt)==true)
                            {
                              ergebnis->setatlocation(gridX,
                                                      gridY + length, true);
                              setFields = true;
                            }
                            else
                            {
                              mittelpunkt.Set(gridX + length/2, 
                                              gridY - length + length/2);

                              if (robust::contains(*region, mittelpunkt)==true)
                              {
                                ergebnis->setatlocation(gridX,
                                                        gridY-length, true);
                                setFields = true;
                              }
                            }
                          }
                        }
                      }
                        
                      gridY = gridY + length;
                    }
                    else if( raster2::intersection(x1, y1, x2, y2, gridX 
                             + length, gridY, gridX + length, gridY + length) )
                    {
                      mittelpunkt.Set(gridX + length/2, gridY + length/2);
    
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(gridX,gridY,true);
                        setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX + length + length/2, 
                                                gridY + length/2);
    
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX + length,gridY,true);
                          setFields = true;
                        }
                        else
                        {
                          mittelpunkt.Set(gridX - length + length/2, 
                                                  gridY + length/2);
     
                          if ( robust::contains(*region, mittelpunkt) == true )
                          {
                            ergebnis->setatlocation(gridX - length,gridY,true);
                            setFields = true;
                          }
                          else
                          {
                            mittelpunkt.Set(gridX + length/2, 
                                                    gridY + length + length/2);
    
                            if ( robust::contains(*region, mittelpunkt)==true)
                            {
                              ergebnis->setatlocation(gridX,
                                                      gridY + length, true);
                              setFields = true;
                            }
                            else
                            {
                              mittelpunkt.Set(gridX + length/2, 
                                              gridY - length + length/2);

                              if (robust::contains(*region, mittelpunkt)==true)
                              {
                                ergebnis->setatlocation(gridX, 
                                                        gridY-length, true);
                                setFields = true;
                              }
                            }
                          }
                        }
                      }
                      
                      gridX = gridX + length;
                    }
                    else 
                    {
                      /*
                       Endfeld erreicht
                     
                       */
                      mittelpunkt.Set(gridX + length/2, gridY + length/2);
    
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(gridX,gridY,true);
                        setFields = true;
                      }
                      else
                      {
                        mittelpunkt.Set(gridX + length + length/2, 
                                                gridY + length/2);
     
                        if ( robust::contains(*region, mittelpunkt) == true )
                        {
                          ergebnis->setatlocation(gridX + length,gridY,true);
                          setFields = true;
                        }
                        else
                        {
                          mittelpunkt.Set(gridX - length + length/2, 
                                                  gridY + length/2);
     
                          if ( robust::contains(*region, mittelpunkt) == true )
                          {
                            ergebnis->setatlocation(gridX - length,gridY,true);
                            setFields = true;
                          }
                          else
                          {
                            mittelpunkt.Set(gridX + length/2, 
                                                    gridY + length + length/2);
    
                            if ( robust::contains(*region, mittelpunkt)==true)
                            {
                              ergebnis->setatlocation(gridX,
                                                      gridY + length, true);
                              setFields = true;
                            }
                            else
                            {
                              mittelpunkt.Set(gridX + length/2, 
                                              gridY - length + length/2);

                              if (robust::contains(*region, mittelpunkt)==true)
                              {
                                ergebnis->setatlocation(gridX,
                                                        gridY-length, true);
                                setFields = true;
                              }
                            }
                          }
                        }
                      }
                        
                      gridX = x2;
                    }
                }
            }
        }

        if ( setFields == true )
        {
            /*
             BoundingBox und Teilgitter ermitteln
        
             */
        
            Rectangle<2> belegteFelder = ergebnis->bbox();
        
            double belegteFelderX1 = belegteFelder.MinD(0);
            double belegteFelderY1 = belegteFelder.MinD(1);
            double belegteFelderX2 = belegteFelder.MaxD(0);
            double belegteFelderY2 = belegteFelder.MaxD(1);

            int size = sbool::storage_type::tile_size * length;

            int minX = gridOriginX;
            int minY = gridOriginY;

            if ( belegteFelderX1 > minX )
            {
                while ( belegteFelderX1 > minX + size )
                {
                    minX = minX + size;
                }
            }
            else
            {
                while ( belegteFelderX1 < minX )
                {
                    minX = minX - size;
                }
            }
        
            if ( belegteFelderY1 > minY )
            {
                while ( belegteFelderY1 > minY + size )
                {
                    minY = minY + size;
                }
            }
            else
            {
                while ( belegteFelderY1 < minY )
                {
                    minY = minY - size;
                }
            }

            int maxX = minX + size;
            int maxY = minY + size;

            while ( belegteFelderX2 > maxX )
            {
                maxX = maxX + size;
            }
        
            while ( belegteFelderY2 > maxY )
            {
                maxY = maxY + size;
            }
        
            
            
            for ( int i = (minX - gridOriginX)/size; 
                                  i < (maxX - gridOriginX)/size; i++ )
            {
              for ( int l = (minY - gridOriginY)/size; 
                                    l < (maxY - gridOriginY)/size; l++ )
              {
                bool gesetzt = false;

                /*
                 Zellen ohne Schnittkanten innerhalb der Region auf true setzen
        
                 */
                for (double j = (gridOriginX + i*size); 
                         j < (gridOriginX + size + i*size); j = j + length)
                  for (double k = (gridOriginY + l*size); 
                           k < (gridOriginY + size + l*size); k = k + length)
                  {
                    if ( ergebnis->atlocation(j + length/2, k + length/2) 
                                                            == UNDEFINED_BOOL )
                    {
                      mittelpunkt.Set(j + length/2, k + length/2);
    
                      if ( robust::contains(*region, mittelpunkt) == true )
                      {
                        ergebnis->setatlocation(j + length/2, 
                                                k + length/2, true);
                      }
                    }
                  }
      
                /*
                 Feststellen welche Teilgitter belegt sind
        
                 */
                for (double j = (gridOriginX + i*size); 
                         j < (gridOriginX + size + i*size); j = j + length)
                  for (double k = (gridOriginY + l*size); 
                           k < (gridOriginY + size + l*size); k = k + length)
                  {
                    if ( ergebnis->atlocation(j,k) == true )
                    {
                      gesetzt = true;
                      j = gridOriginX + size + i*size;
                      k = gridOriginY + size + l*size;
                    }
                  }
              
                /*
                 alle anderen Felder auf false setzen
        
                 */
                if ( gesetzt == true )  
                {
                  for (double j = (gridOriginX + i*size); 
                           j < (gridOriginX + size + i*size); j = j + length)
                    for  (double k = (gridOriginY + l*size); 
                              k < (gridOriginY + size + l*size); k = k + length)
                    {
                      if ( ergebnis->atlocation(j + length/2, k + length/2) 
                                                            == UNDEFINED_BOOL )
                      {
                        ergebnis->setatlocation(j + length/2, 
                                                k + length/2, false);
                      }
                    }
                }
              }
            }
        }

        return 0;
    }
    
    ListExpr fromRegionTypeMap(ListExpr args)
    {
        if(!nl->HasLength(args,2)){
          return listutils::typeError("2 arguments expected");
        }
        if(!Region::checkType(nl->First(args)) ||
           !grid2::checkType(nl->Second(args))){
          return listutils::typeError("region x grid2 expected");
        }
        return listutils::basicSymbol<sbool>();

    }
}
