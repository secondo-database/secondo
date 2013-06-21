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

#ifndef TILEALGEBRA_PROPERTIESREAL_H
#define TILEALGEBRA_PROPERTIESREAL_H

#include "StandardTypes.h"
#include "Properties.h"
#include "../Constants.h"

namespace TileAlgebra
{

/*
declaration of template class Properties<double>

*/

template <>
class Properties<double>
{
  public:

  typedef double PropertiesType;
  typedef CcReal WrapperType;
  static double GetUndefinedValue();
  static double GetValue(const NList& rNList);
  static double GetUnwrappedValue(const CcReal& rCcReal);
  static CcReal GetWrappedValue(const double& rdouble);
  static bool IsUndefinedValue(const double& rdouble);
  static bool IsValidValueType(const NList& rNList);
  static NList ToNList(const double& rdouble);
};

}

#endif // TILEALGEBRA_PROPERTIESREAL_H
