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

#ifndef TILEALGEBRA_PROPERTIES_H
#define TILEALGEBRA_PROPERTIES_H

/*
SECONDO includes

*/

#include "NList.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class Properties represents the properties of base datatypes
of Tile Algebra t datatypes, mt datatypes and it datatypes.

author: Dirk Zacher

*/

template <class Type>
class Properties
{
  public:

  /*
  typedef of PropertiesType

  */

  typedef Type PropertiesType;

  /*
  typedef of WrapperType

  */

  typedef Type WrapperType;

  /*
  typedef of MType

  */

  typedef Type MType;

  /*
  typedef of UnitType

  */

  typedef Type UnitType;

  /*
  Method GetUndefinedValue returns the undefined value of base datatype.

  author: Dirk Zacher
  parameters: -
  return value: undefined value of base datatype
  exceptions: -

  */

  static Type GetUndefinedValue();

  /*
  Method GetValue returns the value of given NList representation.

  author: Dirk Zacher
  parameters: rNList - reference to a NList object
  return value: value of given NList representation
  exceptions: -

  */

  static Type GetValue(const NList& rNList);

  /*
  Method GetUnwrappedValue returns the unwrapped value of given wrapped value.

  author: Dirk Zacher
  parameters: rWrappedValue - reference to a wrapped value
  return value: unwrapped value
  exceptions: -

  */

  static Type GetUnwrappedValue(const WrapperType& rWrappedValue);

  /*
  Method GetWrappedValue returns the wrapped value of given value.

  author: Dirk Zacher
  parameters: rValue - reference to a value
  return value: wrapped value
  exceptions: -

  */

  static WrapperType GetWrappedValue(const Type& rValue);

  /*
  Method IsUndefinedValue checks if given value is an undefined value.

  author: Dirk Zacher
  parameters: rValue - reference to a value
  return value: true, if rValue is an undefined value, otherwise false
  exceptions: -

  */

  static bool IsUndefinedValue(const Type& rValue);

  /*
  Method IsValidValueType checks if given NList is NList of valid value type.

  author: Dirk Zacher
  parameters: rNList - reference to a NList object
  return value: true, if given NList is NList of valid value type,
                otherwise false
  exceptions: -

  */

  static bool IsValidValueType(const NList& rNList);

  /*
  Method ToNList returns NList representation of given value.

  author: Dirk Zacher
  parameters: rValue - reference to a value
  return value: NList representation of given value
  exceptions: -

  */

  static NList ToNList(const Type& rValue);
};

}

#endif // TILEALGEBRA_PROPERTIES_H
