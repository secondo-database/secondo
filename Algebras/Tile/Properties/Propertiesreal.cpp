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

#include "Propertiesreal.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method GetUndefinedValue returns the undefined value of base datatype real.

author: Dirk Zacher
parameters: -
return value: undefined value of base datatype real
exceptions: -

*/

double Properties<double>::GetUndefinedValue()
{
  return UNDEFINED_REAL;
}

/*
Method GetValue returns the value of given NList representation.

author: Dirk Zacher
parameters: rNList - reference to a NList object
return value: value of given NList representation
exceptions: -

*/

double Properties<double>::GetValue(const NList& rNList)
{
  double value = 0.0;

  if(rNList.isInt())
  {
    value = rNList.intval();
  }

  else
  {
    value = rNList.realval();
  }

  return value;
}

/*
Method GetUnwrappedValue returns the unwrapped value of given wrapped value.

author: Dirk Zacher
parameters: rWrappedValue - reference to a wrapped value
return value: unwrapped value
exceptions: -

*/

double Properties<double>::GetUnwrappedValue(const CcReal& rWrappedValue)
{
  double unwrappedValue = GetUndefinedValue();

  if(rWrappedValue.IsDefined())
  {
    unwrappedValue = rWrappedValue.GetValue();
  }

  return unwrappedValue;
}

/*
Method GetWrappedValue returns the wrapped value of given value.

author: Dirk Zacher
parameters: rValue - reference to a value
return value: wrapped value
exceptions: -

*/

CcReal Properties<double>::GetWrappedValue(const double& rValue)
{
  return CcReal(!IsUndefinedValue(rValue), rValue);
}

/*
Method IsUndefinedValue checks if given value is an undefined value.

author: Dirk Zacher
parameters: rValue - reference to a value
return value: true, if rValue is an undefined value, otherwise false
exceptions: -

*/

bool Properties<double>::IsUndefinedValue(const double& rValue)
{
  bool bUndefinedValue = (rValue != rValue);

  return bUndefinedValue;
}

/*
Method IsValidValueType checks if given NList is NList of type real or int.

author: Dirk Zacher
parameters: rNList - reference to a NList object
return value: true, if given NList is NList of type real or int,
              otherwise false
exceptions: -

*/

bool Properties<double>::IsValidValueType(const NList& rNList)
{
  bool bValidValueType = rNList.isReal() || rNList.isInt();

  return bValidValueType;
}

/*
Method ToNList returns NList representation of given value.

author: Dirk Zacher
parameters: rValue - reference to a value
return value: NList representation of given value
exceptions: -

*/

NList Properties<double>::ToNList(const double& rValue)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rValue) == false)
  {
    nList = NList(rValue);
  }

  return nList;
}

}
