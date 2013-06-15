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

#include "mtint.h"
#include "../Constants.h"

namespace TileAlgebra
{

/*
implementation of template class mtProperties<int>

*/

int mtProperties<int>::GetXDimensionSize()
{
  int xDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(mtgrid) -
                                  2 * sizeof(int)) /
                                  sizeof(int),
                                  0.5)
                       );

  return xDimensionSize;
}

int mtProperties<int>::GetYDimensionSize()
{
  int yDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(mtgrid) -
                                  2 * sizeof(int)) /
                                  sizeof(int),
                                  0.5)
                       );

  return yDimensionSize;
}

int mtProperties<int>::GetTDimensionSize()
{
  return TIME_DIMENSION_SIZE;
}

int mtProperties<int>::GetFlobElements()
{
  int flobElements = GetXDimensionSize() *
                     GetYDimensionSize() *
                     GetTDimensionSize();

  return flobElements;
}

SmiSize mtProperties<int>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(int);

  return flobSize;
}

std::string mtProperties<int>::GetTypeName()
{
  return TYPE_NAME_MTINT;
}

}
