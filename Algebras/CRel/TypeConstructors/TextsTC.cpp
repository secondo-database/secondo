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

#include "TextsTC.h"

#include "FTextAlgebra.h"
#include "Strings.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "TIUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::string;

extern CMsg cmsg;
extern NestedList *nl;

//TextsTI-----------------------------------------------------------------------

bool TextsTI::Check(ListExpr typeExpr)
{
  return SimpleTypeCheck(TextsTC::name, typeExpr);
}

bool TextsTI::Check(ListExpr typeExpr, string &error)
{
  return SimpleTypeCheck(TextsTC::name, typeExpr, error);
}

TextsTI::TextsTI(bool numeric) :
  m_isNumeric(numeric)
{
}

ListExpr TextsTI::GetTypeExpr() const
{
  return SimpleTypeExpr(TextsTC::name, m_isNumeric);
}

//TextsTC-----------------------------------------------------------------------

const string TextsTC::name= "texts";

ListExpr TextsTC::TypeProperty()
{
  return ConstructorInfo(name, "-> " + name,
                         "(" + name + ")",
                         "(ai*) where ai is text",
                         "(\'nameA\' \'nameB\' \'nameC\')", "").list();
}

bool TextsTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!TextsTI::Check(typeExpr, error))
  {
    cmsg.typeError(error);
    return false;
  }

  return true;
}

Word TextsTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr TextsTC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word TextsTC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void TextsTC::Delete(const ListExpr typeExpr, Word &value)
{
  return DefaultDelete(typeExpr, value);
}

bool TextsTC::Open(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

bool TextsTC::Save(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void TextsTC::Close(const ListExpr typeExpr, Word &value)
{
  return DefaultClose(typeExpr, value);
}

void *TextsTC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int TextsTC::SizeOf()
{
  return DefaultSizeOf();
}

Word TextsTC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr TextsTC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return numeric ? GetNumericType(FText::BasicType()) :
                   nl->SymbolAtom(FText::BasicType());
}

class TextsManager : public AttrArrayManager
{
public:
  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new Texts();
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new Texts(source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new Texts(source, header.count);
  }
};

AttrArrayManager *TextsTC::CreateManager(ListExpr attributeType)
{
  return new TextsManager();
}

TextsTC::TextsTC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
}