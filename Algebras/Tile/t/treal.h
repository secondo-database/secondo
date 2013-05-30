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

#ifndef TILEALGEBRA_TREAL_H
#define TILEALGEBRA_TREAL_H

#include "t.h"
#include "tProperties.h"
#include "../Constants.h"

namespace TileAlgebra
{
/*
typedef of treal type

*/

typedef t<double> treal;

/*
declaration of template class tProperties<double>

*/

template <>
class tProperties<double>
{
  public:

  typedef treal ImplementationType;
  static int GetDimensionSize();
  static int GetFlobElements();
  static SmiSize GetFlobSize();
  static std::string GetTypeName(); 
  static double GetUndefinedValue();
  static double GetValue(const NList& rNList);
  static bool IsUndefinedValue(const double& rdouble);
  static bool IsValidValueType(const NList& rNList);
  static NList ToNList(const double& rdouble);
};

/*
implementation of template class tProperties<double>

*/

int tProperties<double>::GetDimensionSize()
{
  int dimensionSize = static_cast<unsigned int>
                      (std::sqrt((WinUnix::getPageSize() - sizeof(grid2)) /
                      sizeof(double)));

  return dimensionSize;
}

int tProperties<double>::GetFlobElements()
{
  int nFlobElements = static_cast<unsigned int>
                      (std::pow(GetDimensionSize(), 2));

  return nFlobElements;
}

SmiSize tProperties<double>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(double);

  return flobSize;
}

std::string tProperties<double>::GetTypeName()
{
  return TYPE_NAME_TREAL;
}

double tProperties<double>::GetUndefinedValue()
{
  return UNDEFINED_REAL;
}

double tProperties<double>::GetValue(const NList& rNList)
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

bool tProperties<double>::IsUndefinedValue(const double& rdouble)
{
  bool bUndefinedValue = false;
  
  if(rdouble == GetUndefinedValue())
  {
    bUndefinedValue = true;
  }

  return bUndefinedValue;
}

bool tProperties<double>::IsValidValueType(const NList& rNList)
{
  bool bValidValueType = rNList.isReal() || rNList.isInt();

  return bValidValueType;
}

NList tProperties<double>::ToNList(const double& rdouble)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rdouble) == false)
  {
    nList = NList(rdouble);
  }

  return nList;
}

}

#endif // TILEALGEBRA_TREAL_H
