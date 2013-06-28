/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

#include "fromregion.h"
#include "RobustSetOps.h"

namespace TileAlgebra
{

/*
definition of intersection function

*/

bool intersectionFunction(const double& rX1,
                          const double& rY1,
                          const double& rX2,
                          const double& rY2,
                          const double& rbX1,
                          const double& rbY1,
                          const double& rbX2,
                          const double& rbY2)
{
  bool bRetVal = false;

  double d = (rY2 - rY1) * (rbX1 - rbX2) - (rbY2 - rbY1) * (rX1 - rX2);

  if(d != 0.0)
  {
    double z1 = ((rX1 - rX2) * (rbX2 * rbY1 - rbX1 * rbY2) -
                 (rbX1 - rbX2) * (rX2 * rY1 - rX1 * rY2)) / d;
    double z2 = ((rbY2 - rbY1) * (rX2 * rY1 - rX1 * rY2) -
                 (rY2 - rY1) * (rbX2 * rbY1 - rbX1 * rbY2)) / d;

    /*
    wenn rY1 > rY2 muss die Ueberpruefung des Schnittpunkts
    fuer z2 gedreht sein

    */

    if(rY1 < rY2)
    {
      if((z1 > rX1 || AlmostEqual(z1, rX1)) &&
         (z1 < rX2 || AlmostEqual(z1, rX2)) &&
         (z1 > rbX1 || AlmostEqual(z1, rbX1)) &&
         (z1 < rbX2 || AlmostEqual(z1, rbX2)) &&
         (z2 > rY1 || AlmostEqual(z1, rY1)) &&
         (z2 < rY2 || AlmostEqual(z1, rY2)) &&
         (z2 > rbY1 || AlmostEqual(z1, rbY1)) &&
         (z2 < rbY2 || AlmostEqual(z1, rbY2)))
      {
        bRetVal = true;
      }
    }

    else
    {
      if((z1 > rX1 || AlmostEqual(z1, rX1)) &&
         (z1 < rX2 || AlmostEqual(z1, rX2)) &&
         (z1 > rbX1 || AlmostEqual(z1, rbX1)) &&
         (z1 < rbX2 || AlmostEqual(z1, rbX2)) &&
         (z2 > rY2 || AlmostEqual(z2, rY2)) &&
         (z2 < rY1 || AlmostEqual(z2, rY1)) &&
         (z2 > rbY1 || AlmostEqual(z2, rbY1)) &&
         (z2 < rbY2 || AlmostEqual(z2, rbY2)))
      {
        bRetVal = true;
      }
    }
  }

  return bRetVal;
}

/*
definition of fromregion function

*/

int fromregionFunction(Word* pArguments,
                       Word& rResult,
                       int message,
                       Word& rLocal,
                       Supplier supplier)
{
  int nRetVal = 0;

  if(qp != 0 &&
     pArguments != 0)
  {
    Region* pRegion = static_cast<Region*>(pArguments[0].addr);
    tgrid* pGrid = static_cast<tgrid*>(pArguments[1].addr);

    if(pRegion != 0 &&
       pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        tbool* pResult = static_cast<tbool*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pRegion->IsDefined() &&
             pGrid->IsDefined())
          {
            pResult->SetDefined(true);
            pResult->SetGrid(*pGrid);

            double gridOriginX = pGrid->GetX();
            double gridOriginY = pGrid->GetY();
            double gridLength = pGrid->GetLength();
            double halfGridLength = gridLength / 2;

            bool setFields = false;
            Point centerPoint(true, 0.0, 0.0);

            pRegion->SelectFirst();
            int regionLength = pRegion->Size();

            for(int i = 0; i < regionLength; i++)
            {
              HalfSegment halfSegment;
              pRegion->GetHs(halfSegment);
              pRegion->SelectNext();

              /*
              Startpunkt und Endpunkt des Segment bestimmen

              */

              Point point1 = halfSegment.GetLeftPoint();
              Point point2 = halfSegment.GetRightPoint();

              double x1 = point1.GetX();
              double y1 = point1.GetY();
              double x2 = point2.GetX();
              double y2 = point2.GetY();

              /*
              Raster bestimmen, damit die korrekten Felder gesetzt werden

              */

              double gridX = gridOriginX;
              double gridY = gridOriginY;

              if(gridX < x1)
              {
                while(gridX < x1)
                {
                  gridX = gridX + gridLength;
                }

                if(gridX > x1)
                {
                  gridX = gridX - gridLength;
                }
              }

              else if(gridX > x1)
              {
                while(gridX > x1)
                {
                  gridX = gridX - gridLength;
                }
              }

              if(gridY < y1)
              {
                while(gridY < y1)
                {
                  gridY = gridY + gridLength;
                }

                if(gridY > y1)
                {
                  gridY = gridY - gridLength;
                }
              }

              else if(gridY > y1)
              {
                while(gridY > y1)
                {
                  gridY = gridY - gridLength;
                }
              }

              /*
              Alle Felder die von der Begrenzungslinie geschnitten werden
              und deren Mittelpunkt in der Region liegt, auf true setzen

              */

              /*
              Sonderfall: senkrechte Linie

              */

              if(x1 == x2 &&
                 gridY < y2)
              {
                while(gridY < y2)
                {
                  centerPoint.Set(gridX + halfGridLength,
                                  gridY + halfGridLength);

                  if(robust::contains(*pRegion, centerPoint) == true)
                  {
                    pResult->SetValue(gridX, gridY, true);
                    setFields = true;
                  }

                  else
                  {
                    centerPoint.Set(gridX + gridLength + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                      pResult->SetValue(gridX + gridLength, gridY, true);
                      setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX - gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if(robust::contains(*pRegion, centerPoint) == true)
                      {
                        pResult->SetValue(gridX - gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX + halfGridLength,
                                        gridY + gridLength + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX, gridY + gridLength, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY - gridLength + halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY - gridLength, true);
                            setFields = true;
                          }
                        }
                      }
                    }
                  }

                  gridY = gridY + gridLength;
                }
              }

              else if(x1 == x2 &&
                      gridY > y2)
              {
                while(gridY > y2)
                {
                  centerPoint.Set(gridX + halfGridLength,
                                  gridY + halfGridLength);

                  if(robust::contains(*pRegion, centerPoint) == true)
                  {
                    pResult->SetValue(gridX, gridY, true);
                    setFields = true;
                  }

                  else
                  {
                    centerPoint.Set(gridX + gridLength + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                      pResult->SetValue(gridX + gridLength, gridY, true);
                      setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX - gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if(robust::contains(*pRegion, centerPoint) == true)
                      {
                        pResult->SetValue(gridX - gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX + halfGridLength,
                                        gridY + gridLength + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX, gridY + gridLength, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY - gridLength + halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY - gridLength, true);
                            setFields = true;
                          }
                        }
                      }
                    }
                  }

                  gridY = gridY - gridLength;
                }
              }

              /*
              Linie von links oben nach rechts unten

              */

              if(y1 > y2)
              {
                while(gridX < x2)
                {
                  if(intersectionFunction(x1, y1, x2, y2,
                                          gridX + gridLength,
                                          gridY,
                                          gridX + gridLength,
                                          gridY + gridLength))
                  {
                    centerPoint.Set(gridX + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                      pResult->SetValue(gridX, gridY, true);
                      setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX + gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if ( robust::contains(*pRegion, centerPoint) == true )
                      {
                        pResult->SetValue(gridX + gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX - gridLength + halfGridLength,
                                        gridY + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX - gridLength, gridY, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY + gridLength + halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY + gridLength, true);
                            setFields = true;
                          }

                          else
                          {
                            centerPoint.Set(gridX + halfGridLength,
                                            gridY - gridLength +
                                            halfGridLength);

                            if(robust::contains(*pRegion, centerPoint) == true)
                            {
                              pResult->SetValue(gridX, gridY - gridLength,
                                                true);
                              setFields = true;
                            }
                          }
                        }
                      }
                    }

                    gridX = gridX + gridLength;
                  }

                  else if(intersectionFunction(x1, y1, x2, y2,
                                               gridX,
                                               gridY,
                                               gridX + gridLength,
                                               gridY))
                  {
                    centerPoint.Set(gridX + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                      pResult->SetValue(gridX, gridY, true);
                      setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX + gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if(robust::contains(*pRegion, centerPoint) == true)
                      {
                        pResult->SetValue(gridX + gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX - gridLength + halfGridLength,
                                        gridY + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX - gridLength, gridY, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY + gridLength + halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY + gridLength, true);
                            setFields = true;
                          }

                          else
                          {
                            centerPoint.Set(gridX + halfGridLength, 
                                            gridY - gridLength +
                                            halfGridLength);

                            if(robust::contains(*pRegion, centerPoint) == true)
                            {
                              pResult->SetValue(gridX, gridY - gridLength,
                                                true);
                              setFields = true;
                            }
                          }
                        }
                      }
                    }

                    gridY = gridY - gridLength;
                  }

                  else
                  {
                    /*
                    Endfeld erreicht

                    */
                    centerPoint.Set(gridX + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                      pResult->SetValue(gridX, gridY, true);
                      setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX + gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if(robust::contains(*pRegion, centerPoint) == true)
                      {
                        pResult->SetValue(gridX + gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX - gridLength + halfGridLength,
                                        gridY + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX - gridLength, gridY, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY + gridLength + halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY + gridLength, true);
                            setFields = true;
                          }

                          else
                          {
                            centerPoint.Set(gridX + halfGridLength,
                                            gridY - gridLength +
                                            halfGridLength);

                            if(robust::contains(*pRegion, centerPoint) == true)
                            {
                              pResult->SetValue(gridX, gridY - gridLength,
                                                true);
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
                while(gridX < x2)
                {
                  if(intersectionFunction(x1, y1, x2, y2,
                                          gridX,
                                          gridY + gridLength,
                                          gridX + gridLength,
                                          gridY + gridLength))
                  {
                    centerPoint.Set(gridX + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                        pResult->SetValue(gridX, gridY, true);
                        setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX + gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if(robust::contains(*pRegion, centerPoint) == true)
                      {
                        pResult->SetValue(gridX + gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX - gridLength + halfGridLength,
                                        gridY + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX - gridLength, gridY, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY + gridLength + halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY + gridLength,
                                              true);
                            setFields = true;
                          }

                          else
                          {
                            centerPoint.Set(gridX + halfGridLength,
                                            gridY - gridLength +
                                            halfGridLength);

                            if(robust::contains(*pRegion, centerPoint) ==
                               true)
                            {
                              pResult->SetValue(gridX, gridY - gridLength,
                                                true);
                              setFields = true;
                            }
                          }
                        }
                      }
                    }

                    gridY = gridY + gridLength;
                  }

                  else if(intersectionFunction(x1, y1, x2, y2,
                                               gridX + gridLength,
                                               gridY,
                                               gridX + gridLength,
                                               gridY + gridLength))
                  {
                    centerPoint.Set(gridX + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                      pResult->SetValue(gridX, gridY, true);
                      setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX + gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if(robust::contains(*pRegion, centerPoint) == true)
                      {
                        pResult->SetValue(gridX + gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX - gridLength + halfGridLength,
                                        gridY + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX - gridLength, gridY, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY + gridLength + halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY + gridLength,
                                              true);
                            setFields = true;
                          }

                          else
                          {
                            centerPoint.Set(gridX + halfGridLength,
                                            gridY - gridLength +
                                            halfGridLength);

                            if(robust::contains(*pRegion, centerPoint) ==
                               true)
                            {
                              pResult->SetValue(gridX, gridY - gridLength,
                                                true);
                              setFields = true;
                            }
                          }
                        }
                      }
                    }

                    gridX = gridX + gridLength;
                  }

                  else
                  {
                    /*
                    Endfeld erreicht

                    */

                    centerPoint.Set(gridX + halfGridLength,
                                    gridY + halfGridLength);

                    if(robust::contains(*pRegion, centerPoint) == true)
                    {
                      pResult->SetValue(gridX, gridY, true);
                      setFields = true;
                    }

                    else
                    {
                      centerPoint.Set(gridX + gridLength + halfGridLength,
                                      gridY + halfGridLength);

                      if(robust::contains(*pRegion, centerPoint) == true)
                      {
                        pResult->SetValue(gridX + gridLength, gridY, true);
                        setFields = true;
                      }

                      else
                      {
                        centerPoint.Set(gridX - gridLength + halfGridLength,
                                        gridY + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(gridX - gridLength, gridY, true);
                          setFields = true;
                        }

                        else
                        {
                          centerPoint.Set(gridX + halfGridLength,
                                          gridY + gridLength +
                                          halfGridLength);

                          if(robust::contains(*pRegion, centerPoint) == true)
                          {
                            pResult->SetValue(gridX, gridY + gridLength,
                                              true);
                            setFields = true;
                          }

                          else
                          {
                            centerPoint.Set(gridX + halfGridLength,
                                            gridY - gridLength +
                                            halfGridLength);

                            if(robust::contains(*pRegion, centerPoint) ==
                               true)
                            {
                              pResult->SetValue(gridX, gridY - gridLength,
                                                true);
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

            if(setFields == true)
            {
              /*
              BoundingBox und Teilgitter ermitteln

              */

              Rectangle<2> setFieldsBoundingBox;
              pResult->bbox(setFieldsBoundingBox);

              double setFieldsX1 = setFieldsBoundingBox.MinD(0);
              double setFieldsY1 = setFieldsBoundingBox.MinD(1);
              double setFieldsX2 = setFieldsBoundingBox.MaxD(0);
              double setFieldsY2 = setFieldsBoundingBox.MaxD(1);

              int size = tProperties<char>::GetXDimensionSize() * gridLength;

              int minX = gridOriginX;
              int minY = gridOriginY;

              if(setFieldsX1 > minX)
              {
                while(setFieldsX1 > minX + size)
                {
                  minX += size;
                }
              }

              else
              {
                while(setFieldsX1 < minX)
                {
                  minX -= size;
                }
              }

              if(setFieldsY1 > minY)
              {
                while(setFieldsY1 > minY + size)
                {
                  minY += size;
                }
              }

              else
              {
                while(setFieldsY1 < minY)
                {
                    minY -= size;
                }
              }

              int maxX = minX + size;
              int maxY = minY + size;

              while(setFieldsX2 > maxX)
              {
                maxX += size;
              }

              while(setFieldsY2 > maxY)
              {
                maxY += size;
              }

              for(int i = (minX - gridOriginX) / size;
                  i < (maxX - gridOriginX) / size; i++)
              {
                for(int l = (minY - gridOriginY) / size;
                    l < (maxY - gridOriginY) / size; l++)
                {
                  bool set = false;

                  /*
                  Zellen ohne Schnittkanten innerhalb der Region auf true setzen

                  */

                  for(double j = (gridOriginX + i * size);
                      j < (gridOriginX + size + i * size);
                      j += gridLength)
                  {
                    for(double k = (gridOriginY + l * size);
                        k < (gridOriginY + size + l * size);
                        k += gridLength)
                    {
                      CcBool value;
                      pResult->atlocation(j + halfGridLength,
                                          k + halfGridLength,
                                          value);

                      if(value.IsDefined() == false)
                      {
                        centerPoint.Set(j + halfGridLength,
                                        k + halfGridLength);

                        if(robust::contains(*pRegion, centerPoint) == true)
                        {
                          pResult->SetValue(j + halfGridLength,
                                            k + halfGridLength,
                                            true);
                        }
                      }
                    }
                  }

                  /*
                  Feststellen welche Teilgitter belegt sind

                  */

                  for(double j = (gridOriginX + i * size);
                      j < (gridOriginX + size + i * size);
                      j += gridLength)
                  {
                    for(double k = (gridOriginY + l * size);
                        k < (gridOriginY + size + l * size);
                        k += gridLength)
                    {
                      CcBool value;
                      pResult->atlocation(j, k, value);

                      if(value.IsDefined() &&
                         value.GetBoolval() == true)
                      {
                        set = true;
                        j = gridOriginX + size + i * size;
                        k = gridOriginY + size + l * size;
                      }
                    }
                  }

                  /*
                  alle anderen Felder auf false setzen

                  */

                  if(set == true)
                  {
                    for(double j = (gridOriginX + i * size);
                        j < (gridOriginX + size + i * size);
                        j += gridLength)
                    {
                      for(double k = (gridOriginY + l * size);
                          k < (gridOriginY + size + l * size);
                          k += gridLength)
                      {
                        CcBool value;
                        pResult->atlocation(j + halfGridLength,
                                            k + halfGridLength,
                                            value);

                        if(value.IsDefined() == false)
                        {
                          pResult->SetValue(j + halfGridLength,
                                            k + halfGridLength,
                                            false);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of fromregion type mapping function

*/

ListExpr fromregionTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator fromregion expects "
                                   "a region and a tgrid.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();

    if(argument1 == Region::BasicType() &&
       argument2 == tgrid::BasicType())
    {
      type = NList(tbool::BasicType()).listExpr();
    }
  }

  return type;
}

}
