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

#include <cstddef>
#include "SecondoSMI.h"
#include "Shared.h"
#include <unordered_map>

namespace CRelAlgebra
{
  void WriteOrThrow(SmiRecordFile &target, SmiRecordId recordId, char *source,
                    size_t count, size_t &offset);

  void WriteOrThrow(SmiRecord &target, char *source, size_t count,
                    size_t &offset);

  template<class T>
  void WriteOrThrow(SmiRecord &target, T &value, size_t &offset)
  {
    WriteOrThrow(target, (char*)&value, sizeof(T), offset);
  }

  void ReadOrThrow(char *target, SmiRecordFile &source, SmiRecordId recordId,
                   size_t count, size_t &offset);

  void ReadOrThrow(char *target, SmiRecord &source, size_t count,
                   size_t &offset);

  char *ReadOrThrow(SmiRecordFile &source, SmiRecordId recordId, size_t count,
                    size_t &offset);

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


  class SharedRecordFile : public SmiRecordFile
  {
  public:
    SharedRecordFile(bool fixedSize = false) :
      SmiRecordFile(fixedSize)
    {
    }

    virtual ~SharedRecordFile()
    {
    }

    void AddRef() const
    {
      ++m_refCount;
    }

    void DecRef() const
    {
      if (--m_refCount == 0)
      {
        delete this;
      }
    }

    size_t GetRefCount() const
    {
      return m_refCount;
    }

  private:
    mutable size_t m_refCount;
  };

  class SharedRecordFiles
  {
  public:
    SharedRecordFiles()
    {
    }

    SharedRecordFile *GetFile(SmiFileId id, bool fixedSize = false)
    {
      auto i = m_files.find(id);

      if (i == m_files.end())
      {
        i = m_files.insert({ id, new Entry(id, fixedSize, this) }).first;
      }

      return i->second;
    };

    SharedRecordFile *operator[](SmiFileId id)
    {
      return GetFile(id);
    };

    void AddRef() const
    {
      ++m_refCount;
    }

    void DecRef() const
    {
      if (--m_refCount == 0)
      {
        delete this;
      }
    }

    size_t GetRefCount() const
    {
      return m_refCount;
    }

  private:
    class Entry : public SharedRecordFile
    {
    public:
      Entry(SmiFileId id, bool fixedSize, SharedRecordFiles *files) :
        SharedRecordFile(fixedSize),
        m_id(id),
        m_files(files)
      {
        OpenOrThrow(*this, id);

        files->AddRef();
      }

      virtual ~Entry()
      {
        m_files->m_files.erase(m_id);

        m_files->DecRef();

        CloseOrThrow(*this);
      }

    private:
      size_t m_id;

      SharedRecordFiles *m_files;
    };

    std::unordered_map<SmiFileId, SharedRecordFile*> m_files;

    mutable size_t m_refCount;
  };
}