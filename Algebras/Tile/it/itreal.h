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

#ifndef TILEALGEBRA_ITREAL_H
#define TILEALGEBRA_ITREAL_H

#include "it.h"
#include "itProperties.h"
#include "../Properties/Propertiesreal.h"
#include "../t/treal.h"

namespace TileAlgebra
{

/*
typedef of itreal type

*/

typedef it<double> itreal;

/*
declaration of template class itProperties<double>

*/

template <>
class itProperties<double>
{
  public:

  typedef itreal PropertiesType;
  typedef Properties<double> TypeProperties;
  typedef treal tType;
  static int GetXDimensionSize();
  static int GetYDimensionSize();
  static std::string GetTypeName();
};

}

#endif // TILEALGEBRA_ITREAL_H
