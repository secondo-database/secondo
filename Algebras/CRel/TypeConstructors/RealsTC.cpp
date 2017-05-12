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

#include "RealsTC.h"

#include "Reals.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "StandardTypes.h"
#include "TIUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::string;

extern CMsg cmsg;
extern NestedList *nl;

//RealsTI-----------------------------------------------------------------------

bool RealsTI::Check(ListExpr typeExpr)
{
  return SimpleTypeCheck(RealsTC::name, typeExpr);
}

bool RealsTI::Check(ListExpr typeExpr, string &error)
{
  return SimpleTypeCheck(RealsTC::name, typeExpr, error);
}

RealsTI::RealsTI(bool numeric) :
  m_isNumeric(numeric)
{
}

ListExpr RealsTI::GetTypeExpr() const
{
  return SimpleTypeExpr(RealsTC::name, m_isNumeric);
}

//RealsTC-----------------------------------------------------------------------

const string RealsTC::name= "reals";

ListExpr RealsTC::TypeProperty()
{
  return ConstructorInfo(name, "-> " + name,
                         "(" + name + ")",
                         "(ai*) where ai is real",
                         "(2.0 3.0 5.0)", "").list();
}

bool RealsTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!RealsTI::Check(typeExpr, error))
  {
    cmsg.typeError(error);
    return false;
  }

  return true;
}

Word RealsTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr RealsTC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word RealsTC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void RealsTC::Delete(const ListExpr typeExpr, Word &value)
{
  return DefaultDelete(typeExpr, value);
}

bool RealsTC::Open(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

bool RealsTC::Save(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void RealsTC::Close(const ListExpr typeExpr, Word &value)
{
  return DefaultClose(typeExpr, value);
}

void *RealsTC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int RealsTC::SizeOf()
{
  return DefaultSizeOf();
}

Word RealsTC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr RealsTC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return numeric ? GetNumericType(CcReal::BasicType()) :
                   nl->SymbolAtom(CcReal::BasicType());
}

class RealsManager : public AttrArrayManager
{
public:
  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new Reals();
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new Reals(source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new Reals(source, header.count);
  }
};

AttrArrayManager *RealsTC::CreateManager(ListExpr attributeType)
{
  return new RealsManager();
}

RealsTC::RealsTC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
}