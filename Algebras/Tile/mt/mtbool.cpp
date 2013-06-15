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

#include "mtbool.h"
#include "../Constants.h"

namespace TileAlgebra
{

/*
implementation of template class mtProperties<char>

*/

int mtProperties<char>::GetXDimensionSize()
{
  int xDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(mtgrid) -
                                  2 * sizeof(char)) /
                                  sizeof(char),
                                  0.5)
                       );

  return xDimensionSize;
}

int mtProperties<char>::GetYDimensionSize()
{
  int yDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(mtgrid) -
                                  2 * sizeof(char)) /
                                  sizeof(char),
                                  0.5)
                       );

  return yDimensionSize;
}

int mtProperties<char>::GetTDimensionSize()
{
  return TIME_DIMENSION_SIZE;
}

int mtProperties<char>::GetFlobElements()
{
  int flobElements = GetXDimensionSize() *
                     GetYDimensionSize() *
                     GetTDimensionSize();

  return flobElements;
}

SmiSize mtProperties<char>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(char);

  return flobSize;
}

std::string mtProperties<char>::GetTypeName()
{
  return TYPE_NAME_MTBOOL;
}

}
