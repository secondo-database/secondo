/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#ifndef TILEALGEBRA_TBOOL_H
#define TILEALGEBRA_TBOOL_H

#include "t.h"
#include "tProperties.h"
#include "../Constants.h"

namespace TileAlgebra
{
/*
typedef of tbool type

*/

typedef t<char> tbool;

/*
declaration of template class tProperties<char>

*/

template <>
class tProperties<char>
{
  public:

  typedef tbool ImplementationType;
  static int GetDimensionSize();
  static int GetFlobElements();
  static SmiSize GetFlobSize();
  static std::string GetTypeName(); 
  static char GetUndefinedValue();
  static char GetValue(const NList& rNList);
  static bool IsUndefinedValue(const char& rchar);
  static bool IsValidValueType(const NList& rNList);
  static NList ToNList(const char& rchar);
};

/*
implementation of template class tProperties<char>

*/

int tProperties<char>::GetDimensionSize()
{
  int dimensionSize = static_cast<unsigned int>
                      (std::sqrt((WinUnix::getPageSize() - sizeof(grid2)) /
                       sizeof(char)));

  return dimensionSize;
}

int tProperties<char>::GetFlobElements()
{
  int nFlobElements = static_cast<unsigned int>
                      (std::pow(GetDimensionSize(), 2));

  return nFlobElements;
}

SmiSize tProperties<char>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(char);

  return flobSize;
}

std::string tProperties<char>::GetTypeName()
{
  return TYPE_NAME_TBOOL;
}

char tProperties<char>::GetUndefinedValue()
{
  return UNDEFINED_BOOL;
}

char tProperties<char>::GetValue(const NList& rNList)
{
  return rNList.boolval();
}

bool tProperties<char>::IsUndefinedValue(const char& rchar)
{
  bool bUndefinedValue = false;
  
  if(rchar == GetUndefinedValue())
  {
    bUndefinedValue = true;
  }

  return bUndefinedValue;
}

bool tProperties<char>::IsValidValueType(const NList& rNList)
{
  bool bValidValueType = rNList.isBool();

  return bValidValueType;
}

NList tProperties<char>::ToNList(const char& rchar)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rchar) == false)
  {
    nList = NList(bool(rchar), true);
  }

  return nList;
}

}

#endif // TILEALGEBRA_TBOOL_H
