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

#include "GAttrArrayTC.h"

#include "AlgebraManager.h"
#include "AttrArray.h"
#include <exception>
#include "GAttrArray.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "ReadWrite.h"
#include "SecondoSystem.h"
#include "Symbols.h"
#include "TypeConstructor.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using listutils::isDATA;
using listutils::isStream;
using std::exception;
using std::string;

extern NestedList *nl;
extern AlgebraManager *am;
extern CMsg cmsg;

//GAttrArrayTI------------------------------------------------------------------

bool GAttrArrayTI::Check(ListExpr typeExpr)
{
  string error;

  return Check(typeExpr, error);
}

bool GAttrArrayTI::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "GAttrArrayTI's length != 2.";
    return false;
  }

  const ListExpr firstArg = nl->First(typeExpr);
  if (isStream(typeExpr))
  {
    return Check(GetStreamType(typeExpr), error);
  }

  if (!nl->IsEqual(firstArg, GAttrArrayTC::name))
  {
    error = "GAttrArrayTI's first element != " + GAttrArrayTC::name + ".";
    return false;
  }

  if (!isDATA(nl->Second(typeExpr)))
  {
    error = "GAttrArrayTI's argument (attribute type) is not of kind DATA.";
    return false;
  }

  return true;
}

GAttrArrayTI::GAttrArrayTI(bool numeric) :
  m_attributeType(nl->Empty()),
  m_isNumeric(numeric),
  m_info(nullptr)
{
}

GAttrArrayTI::GAttrArrayTI(ListExpr typeExpr, bool numeric) :
  m_attributeType(isStream(typeExpr) ?
                    nl->Second(GetStreamType(typeExpr)) :
                    nl->Second(typeExpr)),
  m_isNumeric(numeric),
  m_info(nullptr)
{
}

bool GAttrArrayTI::IsNumeric() const
{
  return m_isNumeric;
}

ListExpr GAttrArrayTI::GetAttributeType() const
{
  return m_attributeType;
}

void GAttrArrayTI::SetAttributeType(ListExpr value)
{
  m_attributeType = value;
  m_info = nullptr;
}

const PGAttrArrayInfo &GAttrArrayTI::GetPInfo() const
{
  ListExpr attributeType = m_attributeType;

  if (m_info.IsNull() && !nl->IsEmpty(attributeType))
  {
    if (!m_isNumeric)
    {
      attributeType = SecondoSystem::GetCatalog()->NumericType(attributeType);
    }

    m_info = new GAttrArrayInfo(attributeType);
  }

  return m_info;
}

ListExpr GAttrArrayTI::GetTypeExpr() const
{
  return nl->TwoElemList(m_isNumeric ? GetNumericType(GAttrArrayTC::name) :
                                       nl->SymbolAtom(GAttrArrayTC::name),
                         m_attributeType);
}

//GAttrArrayTC------------------------------------------------------------------

const string GAttrArrayTC::name = "gattrarray";

ListExpr GAttrArrayTC::TypeProperty()
{
  return ConstructorInfo(name,
    "DATA -> " + Kind::ATTRARRAY(),
    "(" + name + " int)",
    "(a0 a1 ... an)",
    "(1 2 3 4)", "").list();
}

bool GAttrArrayTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!GAttrArrayTI::Check(typeExpr, error))
  {
    cmsg.typeError(error);
    return false;
  }

  return true;
}

Word GAttrArrayTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                      ListExpr &errorInfo, bool &correct)
{
  return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
}

ListExpr GAttrArrayTC::Out(ListExpr typeExpr, Word value)
{
  return DefaultOut(typeExpr, value);
}

Word GAttrArrayTC::Create(const ListExpr typeExpr)
{
  return DefaultCreate(typeExpr);
}

void GAttrArrayTC::Delete(const ListExpr typeExpr, Word &value)
{
  DefaultDelete(typeExpr, value);
}

bool GAttrArrayTC::Open(SmiRecord &valueRecord, size_t &offset,
                        const ListExpr typeExpr, Word &value)
{
  return DefaultOpen(valueRecord, offset, typeExpr, value);
}

bool GAttrArrayTC::Save(SmiRecord &valueRecord, size_t &offset,
                        const ListExpr typeExpr, Word &value)
{
  return DefaultSave(valueRecord, offset, typeExpr, value);
}

void GAttrArrayTC::Close(const ListExpr typeExpr, Word &value)
{
  DefaultClose(typeExpr, value);
}

void *GAttrArrayTC::Cast(void *addr)
{
  return DefaultCast(addr);
}

int GAttrArrayTC::SizeOf()
{
  return DefaultSizeOf();
}

Word GAttrArrayTC::Clone(const ListExpr typeExpr, const Word &value)
{
  return DefaultClone(typeExpr, value);
}

ListExpr GAttrArrayTC::GetAttributeType(ListExpr typeExpr, bool numeric)
{
  return GAttrArrayTI(typeExpr, numeric).GetAttributeType();
}

class GAttrArrayManager : public AttrArrayManager
{
public:
  GAttrArrayManager(ListExpr attributeType) :
    m_info(GAttrArrayInfo(attributeType))
  {
  }

  virtual AttrArray *Create(SmiFileId flobFileId)
  {
    return new GAttrArray(m_info, flobFileId);
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new GAttrArray(m_info, source);
  }

  virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header)
  {
    return new GAttrArray(m_info, source, header);
  }

private:
  PGAttrArrayInfo m_info;
};

AttrArrayManager *GAttrArrayTC::CreateManager(ListExpr attributeType)
{
  TypeConstructor *typeConstructor = GetTypeConstructor(attributeType);

  if (typeConstructor != nullptr && typeConstructor->MemberOf(Kind::DATA()))
  {
    return new GAttrArrayManager(attributeType);
  }

  return nullptr;
}

GAttrArrayTC::GAttrArrayTC() :
  AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                           CreateManager)
{
}