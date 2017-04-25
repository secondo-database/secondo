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

#include "GSpatialAttrArrayTC.h"

#include "AlgebraManager.h"
#include "AttrArray.h"
#include <exception>
#include "GAttrArray.h"
#include "GSpatialAttrArrayTI.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "ReadWrite.h"
#include "SecondoSystem.h"
#include "StringUtils.h"
#include "Symbols.h"
#include "TypeUtils.h"
#include "TypeConstructor.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::string;
using stringutils::any2str;

extern NestedList *nl;
extern AlgebraManager *am;
extern CMsg cmsg;

template <int dim>
const std::string &GSpatialAttrArrayTC<dim>::Name()
{
  static const string name = "gattrarray" + any2str(dim) + "d";

  return name;
}

template <int dim>
ListExpr GSpatialAttrArrayTC<dim>::TypeProperty()
{
  string kind;

  switch (dim)
  {
    case 1:
      kind = Kind::SPATIALATTRARRAY1D();
      break;
    case 2:
      kind = Kind::SPATIALATTRARRAY2D();
      break;
    case 3:
      kind = Kind::SPATIALATTRARRAY3D();
      break;
    case 4:
      kind = Kind::SPATIALATTRARRAY4D();
      break;
    case 8:
      kind = Kind::SPATIALATTRARRAY8D();
      break;
  }

  return ConstructorInfo(Name(), "DATA -> " + kind,
                         "(" + Name() + " int)", "(a0 a1 ... an)",
                         "(1 2 3 4)", "").list();
}

template <int dim>
bool GSpatialAttrArrayTC<dim>::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!GSpatialAttrArrayTI<dim>::Check(typeExpr, error))
  {
    cmsg.typeError(error);
    return false;
  }

  return true;
}

template <int dim>
Word GSpatialAttrArrayTC<dim>::In(ListExpr typeExpr, ListExpr value,
                                  int errorPos, ListExpr &errorInfo,
                                  bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

template <int dim>
ListExpr GSpatialAttrArrayTC<dim>::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

template <int dim>
Word GSpatialAttrArrayTC<dim>::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

template <int dim>
void GSpatialAttrArrayTC<dim>::Delete(const ListExpr typeExpr, Word &value)
{
  DefaultDelete(typeExpr, value);
}

template <int dim>
bool GSpatialAttrArrayTC<dim>::Open(SmiRecord &valueRecord, size_t &offset,
                                    const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

template <int dim>
bool GSpatialAttrArrayTC<dim>::Save(SmiRecord &valueRecord, size_t &offset,
                                    const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

template <int dim>
void GSpatialAttrArrayTC<dim>::Close(const ListExpr typeExpr, Word &value)
{
  DefaultClose(typeExpr, value);
}

template <int dim>
void *GSpatialAttrArrayTC<dim>::Cast(void *addr)
{
  return DefaultCast(addr);
}

template <int dim>
int GSpatialAttrArrayTC<dim>::SizeOf()
{
  return DefaultSizeOf();
}

template <int dim>
Word GSpatialAttrArrayTC<dim>::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

template <int dim>
ListExpr GSpatialAttrArrayTC<dim>::GetAttributeType(ListExpr typeExpr,
                                                    bool numeric)
{
  return GSpatialAttrArrayTI<dim>(typeExpr, numeric).GetAttributeType();
}

template<int dim>
class GSpatialAttrArrayManager : public AttrArrayManager
{
public:
  GSpatialAttrArrayManager(ListExpr attributeType) :
    m_info(GAttrArrayInfo(attributeType))
  {
  }

  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new GSpatialAttrArray<dim>(m_info, flobFileId);
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new GSpatialAttrArray<dim>(m_info, source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new GSpatialAttrArray<dim>(m_info, source, header);
  }

private:
  PGAttrArrayInfo m_info;
};

template<int dim>
AttrArrayManager *GSpatialAttrArrayTC<dim>::CreateManager(
  ListExpr attributeType)
{
  TypeConstructor *typeConstructor = GetTypeConstructor(attributeType);

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
      return new GSpatialAttrArrayManager<dim>(attributeType);
    }
  }

  return nullptr;
}

template<int dim>
GSpatialAttrArrayTC<dim>::GSpatialAttrArrayTC() :
  AttrArrayTypeConstructor(Name(), TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
  switch (dim)
  {
    case 1:
      AssociateKind(Kind::SPATIALATTRARRAY1D());
      break;
    case 2:
      AssociateKind(Kind::SPATIALATTRARRAY2D());
      break;
    case 3:
      AssociateKind(Kind::SPATIALATTRARRAY3D());
      break;
    case 4:
      AssociateKind(Kind::SPATIALATTRARRAY4D());
      break;
    case 8:
      AssociateKind(Kind::SPATIALATTRARRAY8D());
      break;
  }
}

template class GSpatialAttrArrayTC<1>;
template class GSpatialAttrArrayTC<2>;
template class GSpatialAttrArrayTC<3>;
template class GSpatialAttrArrayTC<4>;
template class GSpatialAttrArrayTC<8>;