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
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#pragma once

#include "Attribute.h"
#include <cstddef>
#include "HashMap.h"
#include "NestedList.h"
#include "ReadWrite.h"
#include "SecondoSMI.h"
#include "Shared.h"
#include <string>
#include "TypeConstructor.h"

class ArrayAttribute;
class AttrArrayIterator;

class AttrArray : public RefCounter
{
public:
  virtual ~AttrArray();

  ArrayAttribute GetAt(size_t row) const;

  ArrayAttribute operator[](size_t row) const;


  virtual void Append(const AttrArray &block, size_t row) = 0;

  virtual void Append(ListExpr value) = 0;

  virtual void Append(Attribute &value) = 0;

  void Append(const ArrayAttribute &value);


  virtual void Remove() = 0;


  //Anzahl der Elemente im Block
  virtual size_t GetCount() const = 0;

  //Byte-Groesse des Blocks
  virtual size_t GetSize() const = 0;


  //Vergleich eines Eintrags des Blocks mit dem Eintrag eines anderen
  virtual int Compare(size_t rowA, const AttrArray &blockB,
                      size_t rowB) const = 0;

  //Vergleich eines Eintrags des Blocks mit einer Attribute-Instanz
  virtual int Compare(size_t row, Attribute &value) const = 0;

  int Compare(size_t row, const ArrayAttribute &value) const;


  //Vergleich eines Eintrags des Blocks mit dem Eintrag eines anderen
  virtual int Equals(size_t rowA, const AttrArray &blockB, size_t rowB) const;

  //Vergleich eines Eintrags des Blocks mit einer Attribute-Instanz
  virtual int Equals(size_t row, Attribute &value) const;

  int Equals(size_t row, const ArrayAttribute &value) const;


  //Funktion zum Berechnen des Hash-Werts eines Eintrags des Blocks
  virtual size_t GetHash(size_t row) const = 0;

  virtual ListExpr GetListExpr(size_t row) const = 0;


  //Schreibt die Daten serialisiert in ein Zielobjekt
  virtual void Save(Writer &target) const = 0;

  virtual void DeleteRecords();

  virtual void CloseFiles();

  AttrArrayIterator GetIterator() const;
};

class ArrayAttribute
{
public:
  ArrayAttribute();

  ArrayAttribute(const AttrArray *block, size_t row);


  const AttrArray *GetBlock() const;

  size_t GetRow() const;


  int Compare(const AttrArray &block, size_t row) const;

  int Compare(const ArrayAttribute &value) const;

  int Compare(size_t row, Attribute &value) const;

  bool operator < (const ArrayAttribute& value);

  bool operator <= (const ArrayAttribute& value);

  bool operator > (const ArrayAttribute& value);

  bool operator >= (const ArrayAttribute& value);

  int Equals(const AttrArray &block, size_t row) const;

  int Equals(const ArrayAttribute &value) const;

  int Equals(Attribute &value) const;

  bool operator == (const ArrayAttribute& value);

  bool operator != (const ArrayAttribute& value);


  size_t GetHash() const;

  ListExpr GetListExpr() const;

protected:
  const AttrArray *m_block;

  size_t m_row;

  friend class AttrArray;
  friend class AttrArrayIterator;
};

class AttrArrayIterator
{
public:
  AttrArrayIterator();

  AttrArrayIterator(const AttrArray *instance);

  bool IsValid() const;

  bool MoveToNext();

  ArrayAttribute &GetAttribute();

private:
  const AttrArray *m_instance;

  size_t m_count;

  ArrayAttribute m_current;
};

class AttrArrayFactory : public RefCounter
{
public:
  virtual AttrArray *Create(SmiFileId fileId) = 0;

  virtual AttrArray *Load(Reader &source) = 0;
};

class AttrArrayRegistration
{
public:
  virtual AttrArrayFactory *Create(ListExpr attributeType) = 0;

  virtual bool CheckAttributeType(ListExpr attributeType) = 0;

  virtual std::string GetAttributeTypeName() = 0;
};

class AttrArrayCatalog
{
public:
  static AttrArrayCatalog &GetInstance();

  AttrArrayFactory *CreateFactory(ListExpr attributeType);

  void Register(AttrArrayRegistration *registration);

private:
  static AttrArrayCatalog instance;

  static size_t HashString(const std::string &value);

  static int CompareString(const std::string &a, const std::string &b);

  typedef SortedHashMap<std::string, AttrArrayRegistration*,
                        HashString, CompareString> FactoryMap;

  FactoryMap m_registrations;

  AttrArrayCatalog();
};
