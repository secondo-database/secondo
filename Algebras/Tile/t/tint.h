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

#ifndef TILEALGEBRA_TINT_H
#define TILEALGEBRA_TINT_H

#include "t.h"
#include "tProperties.h"
#include "../Constants.h"

namespace TileAlgebra
{
/*
typedef of tint type

*/

typedef t<int> tint;

/*
declaration of template class tProperties<int>

*/

template <>
class tProperties<int>
{
  public:

  typedef tint ImplementationType;
  static int GetDimensionSize();
  static int GetFlobElements();
  static SmiSize GetFlobSize();
  static std::string GetTypeName(); 
  static int GetUndefinedValue();
  static int GetValue(const NList& rNList);
  static bool IsUndefinedValue(const int& rint);
  static bool IsValidValueType(const NList& rNList);
  static NList ToNList(const int& rint);
};

/*
implementation of template class tProperties<int>

*/

int tProperties<int>::GetDimensionSize()
{
  int dimensionSize = static_cast<unsigned int>
                      (std::sqrt((WinUnix::getPageSize() -
                       GRID2_SIZE) / sizeof(int)));

  return dimensionSize;
}

int tProperties<int>::GetFlobElements()
{
  int nFlobElements = static_cast<unsigned int>
                      (std::pow(GetDimensionSize(), 2));

  return nFlobElements;
}

SmiSize tProperties<int>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(int);

  return flobSize;
}

std::string tProperties<int>::GetTypeName()
{
  return TYPE_NAME_TINT;
}

int tProperties<int>::GetUndefinedValue()
{
  return UNDEFINED_INT;
}

int tProperties<int>::GetValue(const NList& rNList)
{
  return rNList.intval();
}

bool tProperties<int>::IsUndefinedValue(const int& rint)
{
  bool bUndefinedValue = false;
  
  if(rint == tProperties<int>::GetUndefinedValue())
  {
    bUndefinedValue = true;
  }

  return bUndefinedValue;
}

bool tProperties<int>::IsValidValueType(const NList& rNList)
{
  return rNList.isInt();
}

NList tProperties<int>::ToNList(const int& rint)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rint) == false)
  {
    nList = NList(rint);
  }

  return nList;
}

}

#endif // TILEALGEBRA_TINT_H
