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

#include "Propertiesbool.h"

namespace TileAlgebra
{

/*
implementation of template class Properties<char>

*/

char Properties<char>::GetUndefinedValue()
{
  return UNDEFINED_BOOL;
}

char Properties<char>::GetValue(const NList& rNList)
{
  return rNList.boolval();
}

char Properties<char>::GetUnwrappedValue(const CcBool& rCcBool)
{
  char unwrappedValue = GetUndefinedValue();

  if(rCcBool.IsDefined())
  {
    unwrappedValue = rCcBool.GetValue();
  }

  return unwrappedValue;
}

CcBool Properties<char>::GetWrappedValue(const char& rchar)
{
  return CcBool(!IsUndefinedValue(rchar), int(rchar));
}

bool Properties<char>::IsUndefinedValue(const char& rchar)
{
  bool bUndefinedValue = false;
  
  if(rchar == GetUndefinedValue())
  {
    bUndefinedValue = true;
  }

  return bUndefinedValue;
}

bool Properties<char>::IsValidValueType(const NList& rNList)
{
  bool bValidValueType = rNList.isBool();

  return bValidValueType;
}

NList Properties<char>::ToNList(const char& rchar)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rchar) == false)
  {
    nList = NList(bool(rchar), true);
  }

  return nList;
}

}
