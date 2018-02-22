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

#include "LongIntsTC.h"

#include "Ints.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "TIUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using std::string;

extern NestedList *nl;

//LongIntsTI--------------------------------------------------------------------

bool LongIntsTI::Check(ListExpr typeExpr)
{
  return SimpleTypeCheck(LongIntsTC::name, typeExpr);
}

bool LongIntsTI::Check(ListExpr typeExpr, string &error)
{
  return SimpleTypeCheck(LongIntsTC::name, typeExpr, error);
}

LongIntsTI::LongIntsTI(bool numeric) :
  m_isNumeric(numeric)
{
}

ListExpr LongIntsTI::GetTypeExpr() const
{
  return SimpleTypeExpr(LongIntsTC::name, m_isNumeric);
}

//LongIntsTC--------------------------------------------------------------------

const string LongIntsTC::name= "longints";

ListExpr LongIntsTC::TypeProperty()
{
  return ConstructorInfo(name, "-> " + name,
                         "(" + name + ")",
                         "(ai*) where ai is longint",
                         "(2 3 (0 4) 5)", "").list();
}

bool LongIntsTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  if (!LongIntsTI::Check(typeExpr))
  {
    return false;
  }

  return true;
}

Word LongIntsTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr LongIntsTC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word LongIntsTC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void LongIntsTC::Delete(const ListExpr typeExpr, Word &value)
{
  return DefaultDelete(typeExpr, value);
}

bool LongIntsTC::Open(SmiRecord &valueRecord, uint64_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

bool LongIntsTC::Save(SmiRecord &valueRecord, uint64_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void LongIntsTC::Close(const ListExpr typeExpr, Word &value)
{
  return DefaultClose(typeExpr, value);
}

void *LongIntsTC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int LongIntsTC::SizeOf()
{
  return DefaultSizeOf();
}

Word LongIntsTC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr LongIntsTC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return numeric ? GetNumericType(LongInt::BasicType()) :
                   nl->SymbolAtom(LongInt::BasicType());
}

class LongIntsManager : public AttrArrayManager
{
public:
  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new LongInts();
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new LongInts(source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new LongInts(source, header.count);
  }
};

AttrArrayManager *LongIntsTC::CreateManager(ListExpr attributeType)
{
  return new LongIntsManager();
}

LongIntsTC::LongIntsTC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
}
