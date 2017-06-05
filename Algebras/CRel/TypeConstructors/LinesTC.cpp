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

#include "LinesTC.h"

#include "Lines.h"
#include "SpatialAlgebra.h"
#include "TIUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using std::string;

extern NestedList *nl;

//LinesTI---------------------------------------------------------------------

bool LinesTI::Check(ListExpr typeExpr)
{
  return SimpleTypeCheck(LinesTC::name, typeExpr);
}

bool LinesTI::Check(ListExpr typeExpr, string &error)
{
  return SimpleTypeCheck(LinesTC::name, typeExpr, error);
}

LinesTI::LinesTI(bool numeric) :
  m_isNumeric(numeric)
{
}

ListExpr LinesTI::GetTypeExpr() const
{
  return SimpleTypeExpr(LinesTC::name, m_isNumeric);
}

//LinesTC---------------------------------------------------------------------

const string LinesTC::name= "lines";

ListExpr LinesTC::TypeProperty()
{
  return ConstructorInfo(name, "-> " + name,
                         "(" + name + ")",
                         "(ai*) where ai is line",
                         "(\"nameA\" \"nameB\" \"nameC\")", "").list();
}

bool LinesTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  if (!LinesTI::Check(typeExpr))
  {
    return false;
  }

  return true;
}

Word LinesTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr LinesTC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word LinesTC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void LinesTC::Delete(const ListExpr typeExpr, Word &value)
{
  return DefaultDelete(typeExpr, value);
}

bool LinesTC::Open(SmiRecord &valueRecord, uint64_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

bool LinesTC::Save(SmiRecord &valueRecord, uint64_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void LinesTC::Close(const ListExpr typeExpr, Word &value)
{
  return DefaultClose(typeExpr, value);
}

void *LinesTC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int LinesTC::SizeOf()
{
  return DefaultSizeOf();
}

Word LinesTC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr LinesTC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return numeric ? GetNumericType(Line::BasicType()) :
                   nl->SymbolAtom(Line::BasicType());
}

class LinesManager : public AttrArrayManager
{
public:
  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new Lines();
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new Lines(source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new Lines(source, header.count);
  }
};

AttrArrayManager *LinesTC::CreateManager(ListExpr attributeType)
{
  return new LinesManager();
}

LinesTC::LinesTC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
  AssociateKind(Kind::SPATIALATTRARRAY2D());
}