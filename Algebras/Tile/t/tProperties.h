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

#ifndef TILEALGEBRA_TPROPERTIES_H
#define TILEALGEBRA_TPROPERTIES_H

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class tProperties represents the properties of t datatypes.

author: Dirk Zacher

*/

template <class Type>
class tProperties
{
  public:

  /*
  typedef of PropertiesType

  */

  typedef Type PropertiesType;

  /*
  typedef of TypeProperties

  */

  typedef Type TypeProperties;

  /*
  typedef of GridType

  */

  typedef Type GridType;

  /*
  typedef of RectangleType

  */

  typedef Type RectangleType;

  /*
  Method GetXDimensionSize returns the size of x dimension of a t datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of x dimension of a t datatype
  exceptions: -

  */

  static int GetXDimensionSize();

  /*
  Method GetYDimensionSize returns the size of y dimension of a t datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of y dimension of a t datatype
  exceptions: -

  */

  static int GetYDimensionSize();

  /*
  Method GetFlobElements returns the number of flob elements of a t datatype.

  author: Dirk Zacher
  parameters: -
  return value: number of flob elements of a t datatype
  exceptions: -

  */

  static int GetFlobElements();

  /*
  Method GetFlobSize returns the size of the flob of a t datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of the flob of a t datatype
  exceptions: -

  */

  static SmiSize GetFlobSize();

  /*
  Method GetTypeName returns the typename of a t datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of a t datatype
  exceptions: -

  */

  static std::string GetTypeName();
};

}

#endif // TILEALGEBRA_TPROPERTIES_H
