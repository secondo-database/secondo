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

#include "Propertiesreal.h"

namespace TileAlgebra
{

/*
implementation of template class Properties<double>

*/

double Properties<double>::GetUndefinedValue()
{
  return UNDEFINED_REAL;
}

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

double Properties<double>::GetUnwrappedValue(const CcReal& rCcReal)
{
  double unwrappedValue = GetUndefinedValue();

  if(rCcReal.IsDefined())
  {
    unwrappedValue = rCcReal.GetValue();
  }

  return unwrappedValue;
}

CcReal Properties<double>::GetWrappedValue(const double& rdouble)
{
  return CcReal(!IsUndefinedValue(rdouble), rdouble);
}

bool Properties<double>::IsUndefinedValue(const double& rdouble)
{
  bool bUndefinedValue = false;
  
  if(rdouble == GetUndefinedValue())
  {
    bUndefinedValue = true;
  }

  return bUndefinedValue;
}

bool Properties<double>::IsValidValueType(const NList& rNList)
{
  bool bValidValueType = rNList.isReal() || rNList.isInt();

  return bValidValueType;
}

NList Properties<double>::ToNList(const double& rdouble)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rdouble) == false)
  {
    nList = NList(rdouble);
  }

  return nList;
}

}
