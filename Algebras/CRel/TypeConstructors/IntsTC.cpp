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

#include <exception>
#include "Ints.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "StandardTypes.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::string;
using std::vector;

extern CMsg cmsg;
extern NestedList *nl;

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
  std::string error;
  if (!IntsTI::Check(typeExpr, error))
  {
    cmsg.typeError(error);
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

//Ints2TC-----------------------------------------------------------------------


const string Ints2TC::name= "ints2";

ListExpr Ints2TC::TypeProperty()
{
  return ConstructorInfo(name, "-> " + name,
                         "(" + name + ")",
                         "(ai*) where ai is int",
                         "(2 3 4 5)", "").list();
}

bool Ints2TC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!Ints2TI::Check(typeExpr, error))
  {
    cmsg.typeError(error);
    return false;
  }

  return true;
}

Word Ints2TC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr Ints2TC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word Ints2TC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void Ints2TC::Delete(const ListExpr typeExpr, Word &value)
{
  return DefaultDelete(typeExpr, value);
}

bool Ints2TC::Open(SmiRecord &valueRecord, size_t &offset,
                   const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

bool Ints2TC::Save(SmiRecord &valueRecord, size_t &offset,
                   const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void Ints2TC::Close(const ListExpr typeExpr, Word &value)
{
  return DefaultClose(typeExpr, value);
}

void *Ints2TC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int Ints2TC::SizeOf()
{
  return DefaultSizeOf();
}

Word Ints2TC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr Ints2TC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return numeric ? GetNumericType(CcInt::BasicType()) :
                   nl->SymbolAtom(CcInt::BasicType());
}

class Ints2Manager : public AttrArrayManager
{
public:
  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new Ints2();
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new Ints2(source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new Ints2(source, header.count);
  }
};

AttrArrayManager *Ints2TC::CreateManager(ListExpr attributeType)
{
  return new Ints2Manager();
}

Ints2TC::Ints2TC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
}