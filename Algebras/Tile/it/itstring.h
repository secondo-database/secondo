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

#ifndef TILEALGEBRA_ITSTRING_H
#define TILEALGEBRA_ITSTRING_H

/*
system includes

*/

#include <string>

/*
TileAlgebra includes

*/

#include "it.h"
#include "itProperties.h"
#include "../Properties/Propertiesstring.h"
#include "../t/tstring.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
typedef of datatype itstring

*/

typedef it<std::string> itstring;

/*
Class itProperties<std::string> represents the properties of datatype itstring.

author: Dirk Zacher

*/

template <>
class itProperties<std::string>
{
  public:

  /*
  typedef of PropertiesType

  */

  typedef itstring PropertiesType;

  /*
  typedef of TypeProperties

  */

  typedef Properties<std::string> TypeProperties;

  /*
  typedef of tType

  */

  typedef tstring tType;

  /*
  Method GetXDimensionSize returns the size of x dimension of datatype itstring.

  author: Dirk Zacher
  parameters: -
  return value: size of x dimension of datatype itstring
  exceptions: -

  */

  static int GetXDimensionSize();

  /*
  Method GetYDimensionSize returns the size of y dimension of datatype itstring.

  author: Dirk Zacher
  parameters: -
  return value: size of y dimension of datatype itstring
  exceptions: -

  */

  static int GetYDimensionSize();

  /*
  Method GetTypeName returns the typename of datatype itstring.

  author: Dirk Zacher
  parameters: -
  return value: typename of datatype itstring
  exceptions: -

  */

  static std::string GetTypeName();
};

}

#endif // TILEALGEBRA_ITSTRING_H
