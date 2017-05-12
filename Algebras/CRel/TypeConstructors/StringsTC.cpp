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

#include "StringsTC.h"

#include <exception>
#include "Strings.h"
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

const string StringsTC::name= "strings";

ListExpr StringsTC::TypeProperty()
{
  return ConstructorInfo(name, "-> " + name,
                         "(" + name + ")",
                         "(ai*) where ai is string",
                         "(\"nameA\" \"nameB\" \"nameC\")", "").list();
}

bool StringsTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!StringsTI::Check(typeExpr, error))
  {
    cmsg.typeError(error);
    return false;
  }

  return true;
}

Word StringsTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr StringsTC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word StringsTC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void StringsTC::Delete(const ListExpr typeExpr, Word &value)
{
  return DefaultDelete(typeExpr, value);
}

bool StringsTC::Open(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

bool StringsTC::Save(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void StringsTC::Close(const ListExpr typeExpr, Word &value)
{
  return DefaultClose(typeExpr, value);
}

void *StringsTC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int StringsTC::SizeOf()
{
  return DefaultSizeOf();
}

Word StringsTC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr StringsTC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return numeric ? GetNumericType(CcString::BasicType()) :
                   nl->SymbolAtom(CcString::BasicType());
}

class StringsManager : public AttrArrayManager
{
public:
  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new Strings<false>();
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new Strings<false>(source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new Strings<false>(source, header.count);
  }
};

AttrArrayManager *StringsTC::CreateManager(ListExpr attributeType)
{
  return new StringsManager();
}

StringsTC::StringsTC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
}