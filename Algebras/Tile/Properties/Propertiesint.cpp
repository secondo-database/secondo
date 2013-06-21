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

#include "Propertiesint.h"

namespace TileAlgebra
{

/*
implementation of template class Properties<int>

*/

int Properties<int>::GetUndefinedValue()
{
  return UNDEFINED_INT;
}

int Properties<int>::GetValue(const NList& rNList)
{
  return rNList.intval();
}

int Properties<int>::GetUnwrappedValue(const CcInt& rCcInt)
{
  int unwrappedValue = GetUndefinedValue();

  if(rCcInt.IsDefined())
  {
    unwrappedValue = rCcInt.GetValue();
  }

  return unwrappedValue;
}

CcInt Properties<int>::GetWrappedValue(const int& rint)
{
  return CcInt(!IsUndefinedValue(rint), rint);
}

bool Properties<int>::IsUndefinedValue(const int& rint)
{
  bool bUndefinedValue = false;
  
  if(rint == GetUndefinedValue())
  {
    bUndefinedValue = true;
  }

  return bUndefinedValue;
}

bool Properties<int>::IsValidValueType(const NList& rNList)
{
  bool bValidValueType = rNList.isInt();

  return bValidValueType;
}

NList Properties<int>::ToNList(const int& rint)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rint) == false)
  {
    nList = NList(rint);
  }

  return nList;
}

}
