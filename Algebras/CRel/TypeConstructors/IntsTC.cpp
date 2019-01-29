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

#include "IntsTC.h"

#include "Ints.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "TIUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::string;

extern NestedList *nl;

//IntsTI------------------------------------------------------------------------

bool IntsTI::Check(ListExpr typeExpr)
{
  return SimpleTypeCheck(IntsTC::name, typeExpr);
}

bool IntsTI::Check(ListExpr typeExpr, string &error)
{
  return SimpleTypeCheck(IntsTC::name, typeExpr, error);
}

IntsTI::IntsTI(bool numeric) :
  m_isNumeric(numeric)
{
}

ListExpr IntsTI::GetTypeExpr() const
{
  return SimpleTypeExpr(IntsTC::name, m_isNumeric);
}

//IntsTC------------------------------------------------------------------------

const string IntsTC::name= "ints";

ListExpr IntsTC::TypeProperty()
{
  return ConstructorInfo(name, "-> " + name,
                         "(" + name + ")",
                         "(ai*) where ai is int",
                         "(2 3 5)", "").list();
}

bool IntsTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  if (!IntsTI::Check(typeExpr))
  {
    return false;
  }

  return true;
}

Word IntsTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr IntsTC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word IntsTC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void IntsTC::Delete(const ListExpr typeExpr, Word &value)
{
  return DefaultDelete(typeExpr, value);
}

bool IntsTC::Open(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset,
                                                      typeExpr, value);
}

bool IntsTC::Save(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void IntsTC::Close(const ListExpr typeExpr, Word &value)
{
  return DefaultClose(typeExpr, value);
}

void *IntsTC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int IntsTC::SizeOf()
{
  return DefaultSizeOf();
}

Word IntsTC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr IntsTC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return numeric ? GetNumericType(CcInt::BasicType()) :
                   nl->SymbolAtom(CcInt::BasicType());
}

class IntsManager : public AttrArrayManager
{
public:
  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new Ints();
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new Ints(source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new Ints(source, header.count);
  }
};

AttrArrayManager *IntsTC::CreateManager(ListExpr attributeType)
{
  return new IntsManager();
}

IntsTC::IntsTC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
  AssociateKind(Kind::ATTRARRAY());
}
