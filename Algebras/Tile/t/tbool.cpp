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

#include "tbool.h"

namespace TileAlgebra
{

/*
implementation of template class tProperties<char>

*/

int tProperties<char>::GetDimensionSize()
{
  int dimensionSize = static_cast<unsigned int>
                      (std::pow((WinUnix::getPageSize() -
                                 sizeof(tgrid) -
                                 2 * sizeof(char)) /
                                 sizeof(char),
                                 0.5)
                      );

  return dimensionSize;
}

int tProperties<char>::GetFlobElements()
{
  int nFlobElements = static_cast<unsigned int>
                      (std::pow(GetDimensionSize(), 2));

  return nFlobElements;
}

SmiSize tProperties<char>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(char);

  return flobSize;
}

std::string tProperties<char>::GetTypeName()
{
  return TYPE_NAME_TBOOL;
}

}
