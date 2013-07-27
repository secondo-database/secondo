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

#ifndef TILEALGEBRA_MTINT_H
#define TILEALGEBRA_MTINT_H

/*
SECONDO includes

*/

#include "RectangleAlgebra.h"
#include "TemporalAlgebra.h"

/*
TileAlgebra includes

*/

#include "mt.h"
#include "mtProperties.h"
#include "../Properties/Propertiesint.h"
#include "../it/itint.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
typedef of datatype mtint

*/

typedef mt<int> mtint;

/*
Class mtProperties<int> represents the properties of datatype mtint.

author: Dirk Zacher

*/

template <>
class mtProperties<int>
{
  public:

  /*
  typedef of PropertiesType

  */

  typedef mtint PropertiesType;

  /*
  typedef of TypeProperties

  */

  typedef Properties<int> TypeProperties;

  /*
  typedef of GridType

  */

  typedef mtgrid GridType;

  /*
  typedef of RectangleType

  */

  typedef Rectangle<3> RectangleType;

  /*
  typedef of itType

  */

  typedef itint itType;

  /*
  typedef of tType

  */

  typedef tint tType;

  /*
  Method GetXDimensionSize returns the size of x dimension of datatype mtint.

  author: Dirk Zacher
  parameters: -
  return value: size of x dimension of datatype mtint
  exceptions: -

  */

  static int GetXDimensionSize();

  /*
  Method GetYDimensionSize returns the size of y dimension of datatype mtint.

  author: Dirk Zacher
  parameters: -
  return value: size of y dimension of datatype mtint
  exceptions: -

  */

  static int GetYDimensionSize();

  /*
  Method GetTDimensionSize returns the size of time dimension of datatype mtint.

  author: Dirk Zacher
  parameters: -
  return value: size of time dimension of datatype mtint
  exceptions: -

  */

  static int GetTDimensionSize();

  /*
  Method GetFlobElements returns the number of flob elements of datatype mtint.

  author: Dirk Zacher
  parameters: -
  return value: number of flob elements of datatype mtint
  exceptions: -

  */

  static int GetFlobElements();

  /*
  Method GetFlobSize returns the size of the flob of datatype mtint.

  author: Dirk Zacher
  parameters: -
  return value: size of the flob of datatype mtint
  exceptions: -

  */

  static SmiSize GetFlobSize();

  /*
  Method GetTypeName returns the typename of datatype mtint.

  author: Dirk Zacher
  parameters: -
  return value: typename of datatype mtint
  exceptions: -

  */

  static std::string GetTypeName(); 
};

}

#endif // TILEALGEBRA_MTINT_H
