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

/*
TileAlgebra includes

*/

#include "mtbool.h"
#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method GetXDimensionSize returns the size of x dimension of datatype mtbool.

author: Dirk Zacher
parameters: -
return value: size of x dimension of datatype mtbool
exceptions: -

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

/*
Method GetYDimensionSize returns the size of y dimension of datatype mtbool.

author: Dirk Zacher
parameters: -
return value: size of y dimension of datatype mtbool
exceptions: -

*/

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

/*
Method GetTDimensionSize returns the size of time dimension of datatype mtbool.

author: Dirk Zacher
parameters: -
return value: size of time dimension of datatype mtbool
exceptions: -

*/

int mtProperties<char>::GetTDimensionSize()
{
  return TIME_DIMENSION_SIZE;
}

/*
Method GetFlobElements returns the number of flob elements of datatype mtbool.

author: Dirk Zacher
parameters: -
return value: number of flob elements of datatype mtbool
exceptions: -

*/

int mtProperties<char>::GetFlobElements()
{
  int flobElements = GetXDimensionSize() *
                     GetYDimensionSize() *
                     GetTDimensionSize();

  return flobElements;
}

/*
Method GetFlobSize returns the size of the flob of datatype mtbool.

author: Dirk Zacher
parameters: -
return value: size of the flob of datatype mtbool
exceptions: -

*/

SmiSize mtProperties<char>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(char);

  return flobSize;
}

/*
Method GetTypeName returns the typename of datatype mtbool.

author: Dirk Zacher
parameters: -
return value: typename of datatype mtbool
exceptions: -

*/

std::string mtProperties<char>::GetTypeName()
{
  return TYPE_NAME_MTBOOL;
}

}
