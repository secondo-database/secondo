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

#include "Utility.h"

#include <stdexcept>
#include "StringUtils.h"

extern NestedList *nl;

void Append(ListExpr &listBack, ListExpr value)
{
  listBack = nl->Append(listBack, value);
}

bool Rest(ListExpr &list)
{
  list = nl->Rest(list);

  return nl->IsEmpty(list);
}

ListExpr Take(ListExpr &list)
{
  ListExpr result(nl->First(list));

  list = nl->Rest(list);

  return result;
}

size_t Length(ListExpr &list, size_t abort)
{
  ListExpr rest = list;
  size_t length = 0;

  if (!nl->IsAtom(rest))
  {
    while (!nl->IsEmpty(rest) && length < abort)
    {
      rest = nl->Rest(rest);
      ++length;
    }
  }

  return length;
}

ListExpr* ToArray(ListExpr list, size_t &count)
{
  count = nl->ListLength(list);

  ListExpr* array = new ListExpr[count];

  ListExpr rest = list;
  for (size_t i = 0; i < count; i++)
  {
    array[i] = nl->First(rest);
    rest = nl->Rest(rest);
  }

  assert(nl->IsEmpty(rest));

  return array;
}

bool ResolveTypeInfo(ListExpr typeInfo, int &algebraId, int &typeId)
{
  ListExpr firstAtom = typeInfo,
    list = nl->Empty();

  while (!nl->IsAtom(firstAtom))
  {
    if (nl->IsEmpty(firstAtom))
    {
      return false;
    }

    list = firstAtom;
    firstAtom = nl->First(firstAtom);
  }

  const NodeType atomType = nl->AtomType(firstAtom);

  if (atomType == IntType)
  {
    if (!nl->HasLength(list, 2))
    {
      return false;
    }

    const ListExpr secondAtom = nl->Second(list);
    if (!nl->IsNodeType(IntType, secondAtom))
    {
      return false;
    }

    algebraId = nl->IntValue(firstAtom);
    typeId = nl->IntValue(secondAtom);

    return true;
  }

  std::string name;
  switch (atomType)
  {
    case StringType:
      name = nl->StringValue(firstAtom);
      break;
    case SymbolType:
      name = nl->SymbolValue(firstAtom);
      break;
    default:
      return false;
  }

  return SecondoSystem::GetCatalog()->GetTypeId(name, algebraId, typeId);
}

bool ResolveTypeInfo(ListExpr typeInfo, std::string &name)
{
  ListExpr firstAtom = typeInfo,
    list = nl->Empty();

  while (!nl->IsAtom(firstAtom))
  {
    if (nl->IsEmpty(firstAtom))
    {
      return false;
    }

    list = firstAtom;
    firstAtom = nl->First(firstAtom);
  }

  switch (nl->AtomType(firstAtom))
  {
    case IntType:
    {
      if (!nl->HasLength(list, 2))
      {
        return false;
      }

      const ListExpr secondAtom = nl->Second(list);
      if (!nl->IsNodeType(IntType, secondAtom))
      {
        return false;
      }

      name = SecondoSystem::GetCatalog()->GetTypeName(nl->IntValue(firstAtom),
                                                      nl->IntValue(secondAtom));

      return true;
    }
    case StringType:
    {
      name = nl->StringValue(firstAtom);
      return true;
    }
    case SymbolType:
    {
      name = nl->SymbolValue(firstAtom);
      return true;
    }
  }

  return false;
}

bool ResolveTypeInfo(ListExpr typeInfo, std::string &name,
                     int &algebraId, int &typeId)
{
  ListExpr firstAtom = typeInfo,
    list = nl->Empty();

  while (!nl->IsAtom(firstAtom))
  {
    if (nl->IsEmpty(firstAtom))
    {
      return false;
    }

    list = firstAtom;
    firstAtom = nl->First(firstAtom);
  }

  const NodeType atomType = nl->AtomType(firstAtom);

  if (atomType == IntType)
  {
    if (!nl->HasLength(list, 2))
    {
      return false;
    }

    const ListExpr secondAtom = nl->Second(list);
    if (!nl->IsNodeType(IntType, secondAtom))
    {
      return false;
    }

    algebraId = nl->IntValue(firstAtom);
    typeId = nl->IntValue(secondAtom);

    name = SecondoSystem::GetCatalog()->GetTypeName(nl->IntValue(firstAtom),
                                                    nl->IntValue(secondAtom));

    return true;
  }

  switch (atomType)
  {
    case StringType:
      name = nl->StringValue(firstAtom);
      break;
    case SymbolType:
      name = nl->SymbolValue(firstAtom);
      break;
    default:
      return false;
  }

  return SecondoSystem::GetCatalog()->GetTypeId(name, algebraId, typeId);
}

ListExpr GetStreamType(ListExpr typeInfo)
{
  return nl->Second(typeInfo);
}

std::string GetTypeName(ListExpr typeInfo)
{
  ListExpr tmp = typeInfo;
  while(!nl->IsAtom(tmp) && !nl->IsEmpty(tmp))
  {
    tmp = Take(tmp);
  }

  if(nl->AtomType(tmp) == SymbolType)
  {
    return nl->SymbolValue(tmp);
  }

  return "";
}

void WriteOrThrow(SmiRecord &target, char *source, size_t count,
                         size_t &offset)
{
  size_t writtenCount = target.Write(source, count, offset);

  if (writtenCount != count)
  {
    throw std::runtime_error("Writing to SmiRecord failed.");
  }
  else
  {
    offset += writtenCount;
  }
}

void ReadOrThrow(char *target, SmiRecord &source, size_t count,
                         size_t &offset)
{
  size_t readCount = source.Read(target, count, offset);

  if (readCount != count)
  {
    throw std::runtime_error("Reading from SmiRecord failed.");
  }
  else
  {
    offset += readCount;
  }
}

char *ReadOrThrow(SmiRecord &source, size_t count, size_t &offset)
{
  char *target = new char[count];
  size_t readCount = source.Read(target, count, offset);

  if (readCount != count)
  {
    delete[] target;

    throw std::runtime_error("Reading from SmiRecord failed.");
  }
  else
  {
    offset += readCount;

    return target;
  }
}

void CreateOrThrow(SmiFile &file)
{
  if (!file.Create())
  {
    throw std::runtime_error("Creating file failed.");
  }
}

size_t closedFiles = 0,
  openedFiles = 0;

void OpenOrThrow(SmiFile &file, SmiFileId id)
{
  ++openedFiles;

  if (!file.Open(id))
  {
    throw std::runtime_error("Opening file failed. fileId: " +
                             stringutils::any2str(id) + ".");
  }
}

void CloseOrThrow(SmiFile &file, bool sync)
{
  ++closedFiles;

  if (!file.Close(sync))
  {
    throw std::runtime_error("Closing file failed. fileId: " +
                             stringutils::any2str(file.GetFileId()) + ".");
  }
}

void DropOrThrow(SmiFile &file)
{
  if (!file.Drop())
  {
    throw std::runtime_error("Droping file failed. fileId: " +
                             stringutils::any2str(file.GetFileId()) + ".");
  }
}

void AppendOrThrow(SmiRecordFile &file, SmiRecordId &id)
{
  SmiRecord record;
  if (!file.AppendRecord(id, record))
  {
    throw std::runtime_error("Appending record failed. fileId: " +
                             stringutils::any2str(file.GetFileId()) + ".");
  }
}

void AppendOrThrow(SmiRecordFile &file, SmiRecord &record)
{
  SmiRecordId id;
  if (!file.AppendRecord(id, record))
  {
    throw std::runtime_error("Appending record failed. fileId: " +
                             stringutils::any2str(file.GetFileId()) + ".");
  }
}

void AppendOrThrow(SmiRecordFile &file, SmiRecordId &id,
                          SmiRecord &record)
{
  if (!file.AppendRecord(id, record))
  {
    throw std::runtime_error("Appending record failed. fileId: " +
                             stringutils::any2str(file.GetFileId()) + ".");
  }
}

void SelectOrThrow(SmiRecordFile &file, SmiRecordId id,
                          SmiFile::AccessType accessType, SmiRecord &record)
{
  if (!file.SelectRecord(id, record, accessType))
  {
    throw std::runtime_error("Selecting record failed. fileId: " +
                             stringutils::any2str(file.GetFileId()) +
                             ", recordId: " + stringutils::any2str(id) +
                             ", accessType: " +
                             stringutils::any2str(accessType) + ".");
  }
}

void DeleteOrThrow(SmiRecordFile &file, SmiRecordId id)
{
  if (!file.DeleteRecord(id))
  {
    throw std::runtime_error("Deleting record failed. fileId: " +
                             stringutils::any2str(file.GetFileId()) +
                             ", recordId: " + stringutils::any2str(id) + ".");
  }
}