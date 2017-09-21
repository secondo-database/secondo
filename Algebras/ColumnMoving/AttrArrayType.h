/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
asize_t with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1. AttrArrayType.h

*/

#pragma once

#include "AttrArray.h"
#include <cstddef>
#include "NestedList.h"
#include "ReadWrite.h"
#include <string>
#include "TypeConstructor.h"
#include "TIUtils.h"
#include "TypeUtils.h"
#include "StandardTypes.h"


using namespace CRelAlgebra;
using namespace std;

namespace ColumnMovingAlgebra
{

/*

1.1 Declaration

~AttrArrayType~ is a generic class for the representation of attribut array
types compatible to the CRel algebra.

*/
  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  class AttrArrayType {
  public:
  
/*
The nested class ~TI~ gives type information for a attribut array.

*/
  
    class TI {
    public:
      static bool Check(ListExpr typeExpr);
      static bool Check(ListExpr typeExpr, std::string &error);
      
      TI(bool numeric);
      ListExpr GetTypeExpr() const;

    private:
      bool m_isNumeric;
    };

/*
The nested class ~TC~ is a generic type constructor.

*/
  
    class TC : public AttrArrayTypeConstructor
    {
    public:
      static const string name;
      
      static ListExpr TypeProperty();
      
      static bool CheckType(ListExpr typeExpr, ListExpr &errorInfo);
      
      static Word In(ListExpr typeExpr, ListExpr value, int errorPos,
                     ListExpr &errorInfo, bool &correct);
      static ListExpr Out(ListExpr typeExpr, Word value);
      
      static Word Create(const ListExpr typeExpr);
      static void Delete(const ListExpr typeExpr, Word &value);
      
      static bool Open(SmiRecord &valueRecord, size_t &offset,
                       const ListExpr typeExpr, Word &value);
      static bool Save(SmiRecord &valueRecord, size_t &offset,
                       const ListExpr typeExpr, Word &value);
                       
      static void Close(const ListExpr typeExpr, Word &w);

      static void *Cast(void *addr);

      static int SizeOf();

      static Word Clone(const ListExpr typeExpr, const Word &w);

      static ListExpr GetAttributeType(ListExpr typeExpr, bool numeric);

      static AttrArrayManager *CreateManager(ListExpr attributeType);

      TC();
    };
    
  private:

/*
The nested class ~Manager~ is a generic AttrArrayManager.

*/
  
      class Manager : public AttrArrayManager
      {
      public:
        virtual AttrArray *Create(SmiFileId flobFileId);
        virtual AttrArray *Load(Reader &source);
        virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header);
      };
    
  };
  
/*
2. Implementation

The following functions connect the types to the CRel algebra.

*/
  
  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  bool AttrArrayType<T,BT,tname,info,example>::TI::Check(ListExpr typeExpr)
  {
    return SimpleTypeCheck(TC::name, typeExpr);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  bool AttrArrayType<T,BT,tname,info,example>::TI::Check(ListExpr typeExpr, 
    string &error)
  {
    return SimpleTypeCheck(TC::name, typeExpr, error);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  AttrArrayType<T,BT,tname,info,example>::TI::TI(bool numeric) :
    m_isNumeric(numeric)
  {
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  ListExpr AttrArrayType<T,BT,tname,info,example>::TI::GetTypeExpr() const
  {
    return SimpleTypeExpr(TC::name, m_isNumeric);
  }



  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  const string AttrArrayType<T,BT,tname,info,example>::TC::name = tname;

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  ListExpr AttrArrayType<T,BT,tname,info,example>::TC::TypeProperty()
  {
    return ConstructorInfo(string(name), "-> " + string(name), "(" + 
      string(name) + ")", string(info), string(example), "").list();
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  bool AttrArrayType<T,BT,tname,info,example>::TC::CheckType(ListExpr typeExpr, 
    ListExpr &errorInfo)
  {
    if (!TI::Check(typeExpr))
    {
      return false;
    }

    return true;
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  Word AttrArrayType<T,BT,tname,info,example>::TC::In(ListExpr typeExpr, 
    ListExpr value, int errorPos, ListExpr &errorInfo, bool &correct)
  {
    return DefaultIn(typeExpr, value, errorPos, errorInfo, correct);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  ListExpr AttrArrayType<T,BT,tname,info,example>::TC::Out(
    ListExpr typeExpr, Word value)
  {
    return DefaultOut(typeExpr, value);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  Word AttrArrayType<T,BT,tname,info,example>::TC::Create(
    const ListExpr typeExpr)
  {
    return DefaultCreate(typeExpr);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  void AttrArrayType<T,BT,tname,info,example>::TC::Delete(
    const ListExpr typeExpr, Word &value)
  {
    return DefaultDelete(typeExpr, value);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  bool AttrArrayType<T,BT,tname,info,example>::TC::Open(SmiRecord &valueRecord, 
    size_t &offset, const ListExpr typeExpr, Word &value)
  {
    return DefaultOpen(valueRecord, offset, typeExpr, value);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  bool AttrArrayType<T,BT,tname,info,example>::TC::Save(SmiRecord &valueRecord, 
    size_t &offset, const ListExpr typeExpr, Word &value)
  {
    return DefaultSave(valueRecord, offset, typeExpr, value);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  void AttrArrayType<T,BT,tname,info,example>::TC::Close(
    const ListExpr typeExpr, Word &value)
  {
    return DefaultClose(typeExpr, value);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  void *AttrArrayType<T,BT,tname,info,example>::TC::Cast(void *addr)
  {
    return DefaultCast(addr);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  int AttrArrayType<T,BT,tname,info,example>::TC::SizeOf()
  {
    return DefaultSizeOf();
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  Word AttrArrayType<T,BT,tname,info,example>::TC::Clone(
    const ListExpr typeExpr, const Word &value)
  {
    return DefaultClone(typeExpr, value);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  ListExpr AttrArrayType<T,BT,tname,info,example>::TC::GetAttributeType(
    ListExpr typeExpr, bool numeric)
  {
    return numeric ? GetNumericType(BT::BasicType()) :
                     nl->SymbolAtom(BT::BasicType());
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  AttrArrayManager *AttrArrayType<T,BT,tname,info,example>::TC::CreateManager(
    ListExpr attributeType)
  {
    return new Manager();
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  AttrArrayType<T,BT,tname,info,example>::TC::TC() :
    AttrArrayTypeConstructor(name, TypeProperty, CheckType, GetAttributeType,
                             CreateManager)
  {
    AssociateKind(Kind::ATTRARRAY());
  }
  
  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  AttrArray *AttrArrayType<T,BT,tname,info,example>::Manager::Create(
    SmiFileId flobFileId)
  {
    return new T();
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  AttrArray *AttrArrayType<T,BT,tname,info,example>::Manager::Load(
    Reader &source)
  {
    return new T(source);
  }

  template<class T, class BT, char const * tname, char const * info, 
    char const * example>
  AttrArray *AttrArrayType<T,BT,tname,info,example>::Manager::Load(
    Reader &source, const AttrArrayHeader &header)
  {
    return new T(source, header.count);
  }

}
