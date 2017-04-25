/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
----

*/

#include "GSpatialAttrArrayTI.h"

#include "AlgebraManager.h"
#include "GSpatialAttrArrayTC.h"
#include "ListUtils.h"
#include "SecondoSystem.h"
#include "StringUtils.h"
#include "Symbols.h"
#include "TypeConstructor.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using listutils::isStream;
using std::string;
using stringutils::any2str;

extern AlgebraManager *am;
extern NestedList *nl;

template <int dim>
bool GSpatialAttrArrayTI<dim>::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "GSpatialAttrArrayTI's length != 2.";
    return false;
  }

  const ListExpr firstArg = nl->First(typeExpr);
  if (isStream(typeExpr))
  {
    return Check(GetStreamType(typeExpr), error);
  }

  if (!nl->IsEqual(firstArg, GSpatialAttrArrayTC<dim>::Name()))
  {
    error = "GSpatialAttrArrayTI's first element != " +
            GSpatialAttrArrayTC<dim>::Name() + ".";

    return false;
  }

  TypeConstructor *typeConstructor = GetTypeConstructor(nl->Second(typeExpr));

  if (typeConstructor != nullptr)
  {
    string kind;

    switch (dim)
    {
      case 1:
        kind = Kind::SPATIAL1D();
        break;
      case 2:
        kind = Kind::SPATIAL2D();
        break;
      case 3:
        kind = Kind::SPATIAL3D();
        break;
      case 4:
        kind = Kind::SPATIAL4D();
        break;
      case 8:
        kind = Kind::SPATIAL8D();
        break;
    }

    if (typeConstructor->MemberOf(kind))
    {
      return true;
    }
  }

  error = "GSpatialAttrArrayTI's argument (attribute type) is not of kind "
          "SPATIAL" + any2str(dim) + "D.";

  return false;
}

template <int dim>
GSpatialAttrArrayTI<dim>::GSpatialAttrArrayTI(bool numeric) :
  m_typeInfo(numeric)
{
}

template <int dim>
GSpatialAttrArrayTI<dim>::GSpatialAttrArrayTI(ListExpr typeExpr, bool numeric) :
  m_typeInfo(typeExpr, numeric)
{
}

template <int dim>
bool GSpatialAttrArrayTI<dim>::IsNumeric() const
{
  return m_typeInfo.IsNumeric();
}

template <int dim>
ListExpr GSpatialAttrArrayTI<dim>::GetAttributeType() const
{
  return m_typeInfo.GetAttributeType();
}

template <int dim>
void GSpatialAttrArrayTI<dim>::SetAttributeType(ListExpr value)
{
  m_typeInfo.SetAttributeType(value);
}

template <int dim>
const PGAttrArrayInfo &GSpatialAttrArrayTI<dim>::GetPInfo() const
{
  return m_typeInfo.GetPInfo();
}

template <int dim>
ListExpr GSpatialAttrArrayTI<dim>::GetTypeExpr() const
{
  return nl->TwoElemList(
    m_typeInfo.IsNumeric() ? GetNumericType(GSpatialAttrArrayTC<dim>::Name()) :
                             nl->SymbolAtom(GSpatialAttrArrayTC<dim>::Name()),
    m_typeInfo.GetAttributeType());
}

template class GSpatialAttrArrayTI<1>;
template class GSpatialAttrArrayTI<2>;
template class GSpatialAttrArrayTI<3>;
template class GSpatialAttrArrayTI<4>;
template class GSpatialAttrArrayTI<8>;