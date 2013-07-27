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

#ifndef TILEALGEBRA_ITINT_H
#define TILEALGEBRA_ITINT_H

/*
TileAlgebra includes

*/

#include "it.h"
#include "itProperties.h"
#include "../Properties/Propertiesint.h"
#include "../t/tint.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
typedef of datatype itint

*/

typedef it<int> itint;

/*
Class itProperties<int> represents the properties of datatype itint.

author: Dirk Zacher

*/

template <>
class itProperties<int>
{
  public:

  /*
  typedef of PropertiesType

  */

  typedef itint PropertiesType;

  /*
  typedef of TypeProperties

  */

  typedef Properties<int> TypeProperties;

  /*
  typedef of tType

  */

  typedef tint tType;

  /*
  Method GetXDimensionSize returns the size of x dimension of datatype itint.

  author: Dirk Zacher
  parameters: -
  return value: size of x dimension of datatype itint
  exceptions: -

  */

  static int GetXDimensionSize();

  /*
  Method GetYDimensionSize returns the size of y dimension of datatype itint.

  author: Dirk Zacher
  parameters: -
  return value: size of y dimension of datatype itint
  exceptions: -

  */

  static int GetYDimensionSize();

  /*
  Method GetTypeName returns the typename of datatype itint.

  author: Dirk Zacher
  parameters: -
  return value: typename of datatype itint
  exceptions: -

  */

  static std::string GetTypeName();
};

}

#endif // TILEALGEBRA_ITINT_H
