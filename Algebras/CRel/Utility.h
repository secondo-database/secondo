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

#include "AlgebraTypes.h"
#include "Attribute.h"
#include <cstddef>
#include <cstring>
#include "NestedList.h"
#include "SecondoSMI.h"
#include <string>
#include <vector>

void Append(ListExpr &listBack, ListExpr value);

bool Rest(ListExpr &list);

ListExpr Take(ListExpr &list);

size_t Length(ListExpr &list, size_t abort);

ListExpr* ToArray(ListExpr list, size_t &count);

template<class T>
T *ToArray(T *source, size_t count)
{
  T *copy = new T[count];

  memcpy(copy, source, count);

  return copy;
}

template<class T>
T *ToArray(std::vector<T> &source, size_t count)
{
  T *copy = new T[count];

  for (size_t i = 0; i < count; i++)
  {
    copy[i] = source[i];
  }

  return copy;
}

template<class T>
T *ToArray(std::vector<T> &source)
{
  return ToArray(source, source.size());
}

bool ResolveTypeInfo(ListExpr typeInfo, int &algebraId, int &typeId);

bool ResolveTypeInfo(ListExpr typeInfo, std::string &name);

bool ResolveTypeInfo(ListExpr typeInfo, std::string &name,
                     int &algebraId, int &typeId);

ListExpr GetStreamType(ListExpr typeInfo);

std::string GetTypeName(ListExpr typeInfo);

void WriteOrThrow(SmiRecord &target, char *source, size_t count,
                  size_t &offset);

template<class T>
void WriteOrThrow(SmiRecord &target, T &value, size_t &offset)
{
  WriteOrThrow(target, (char*)&value, sizeof(T), offset);
}

void ReadOrThrow(char *target, SmiRecord &source, size_t count, size_t &offset);

char *ReadOrThrow(SmiRecord &source, size_t count, size_t &offset);

template<class T>
void ReadOrThrow(T &value, SmiRecord &source, size_t &offset)
{
  ReadOrThrow((char*)&value, source, sizeof(T), offset);
}

template<class T>
T ReadOrThrow(SmiRecord &source, size_t &offset)
{
  T value;
  ReadOrThrow((char*)&value, source, sizeof(T), offset);

  return value;
}

void CreateOrThrow(SmiFile &file);

void OpenOrThrow(SmiFile &file, SmiFileId id);

void CloseOrThrow(SmiFile &file, bool sync = true);

void DropOrThrow(SmiFile &file);

void AppendOrThrow(SmiRecordFile &file, SmiRecordId &id);

void AppendOrThrow(SmiRecordFile &file, SmiRecord &record);

void AppendOrThrow(SmiRecordFile &file, SmiRecordId &id, SmiRecord &record);

void SelectOrThrow(SmiRecordFile &file, SmiRecordId id,
                   SmiFile::AccessType accessType, SmiRecord &record);

void DeleteOrThrow(SmiRecordFile &file, SmiRecordId id);

template<class T>
class InFunction
{
public:
    InFunction() :
      m_function(NULL),
      m_typeInfo(nl->Empty())
    {
    }

    InFunction(InObject function, ListExpr typeInfo) :
      m_function(function),
      m_typeInfo(typeInfo)
    {
    }

    ListExpr GetTypeInfo()
    {
      return m_typeInfo;
    }

    void SetTypeInfo(ListExpr typeInfo)
    {
      m_typeInfo = typeInfo;
    }

    inline T operator()(ListExpr value, int errorPos, ListExpr& error,
                        bool& correct) const
    {
      return (T)m_function(m_typeInfo, value, errorPos, error, correct).addr;
    }

    inline T operator()(ListExpr typeInfo, ListExpr value, int errorPos,
                        ListExpr& error, bool& correct) const
    {
      return (T)m_function(typeInfo, value, errorPos, error, correct).addr;
    }

    inline InFunction& operator=(InObject function)
    {
      m_function = function;

      return *this;
    }

private:
  InObject m_function;

  ListExpr m_typeInfo;
};

typedef InFunction<Attribute*> AttributeInFunction;

class AttributeOutFunction
{
public:
    AttributeOutFunction() :
      m_function(NULL),
      m_typeInfo(nl->Empty())
    {
    }

    AttributeOutFunction(OutObject function, ListExpr typeInfo) :
      m_function(function),
      m_typeInfo(typeInfo)
    {
    }

    ListExpr GetTypeInfo()
    {
      return m_typeInfo;
    }

    void SetTypeInfo(ListExpr typeInfo)
    {
      m_typeInfo = typeInfo;
    }

    inline ListExpr operator()(Attribute &value) const
    {
      return m_function(m_typeInfo, Word(&value));
    }

    inline ListExpr operator()(ListExpr typeInfo, Attribute &value) const
    {
      return m_function(typeInfo, Word(&value));
    }

private:
  OutObject m_function;

  ListExpr m_typeInfo;
};

template<class T>
class CastFunction
{
public:
    CastFunction() :
      m_function(NULL)
    {
    }

    CastFunction(ObjectCast function) :
      m_function(function)
    {
    }

    inline T *operator()(void *instance) const
    {
      return (T*)m_function(instance);
    }

    inline ObjectCast& operator=(ObjectCast function)
    {
      m_function = function;

      return *this;
    }

private:
  ObjectCast m_function;
};

typedef CastFunction<Attribute*> AttributeCastFunction;