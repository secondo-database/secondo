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

#include <cstdint>
#include <cstring>
#include "SecondoException.h"
#include "SecondoSMI.h"

namespace CRelAlgebra
{
  class Reader
  {
  public:
    virtual uint64_t GetPosition() = 0;
    virtual void SetPosition(uint64_t position) = 0;

    virtual bool Read(char *target, uint64_t count) = 0;

    inline void ReadOrThrow(char *target, uint64_t count)
    {
      if (!Read(target, count))
      {
        throw SecondoException("Reading failed.");
      }
    }

    template<class T>
    inline bool Read(T &value)
    {
      return Read((char*)&value, sizeof(T));
    }

    template<class T>
    inline void ReadOrThrow(T &value)
    {
      if (!Read(value))
      {
        throw SecondoException("Reading failed.");
      }
    }

    template<class T>
    inline T ReadOrThrow()
    {
      T value;
      ReadOrThrow(value);

      return value;
    }
  };

  class Writer
  {
  public:
    virtual uint64_t GetPosition() = 0;
    virtual void SetPosition(uint64_t position) = 0;

    virtual bool Write(char *source, uint64_t count) = 0;

    inline void WriteOrThrow(char *source, uint64_t count)
    {
      if (!Write(source, count))
      {
        throw SecondoException("Writing failed.");
      }
    }

    template<class T>
    inline bool Write(const T &value)
    {
      return Write((char*)&value, sizeof(T));
    }

    template<class T>
    inline void WriteOrThrow(const T &value)
    {
      if (!Write(value))
      {
        throw SecondoException("Writing failed.");
      }
    }
  };

  class BufferReader : public Reader
  {
  public:
    BufferReader(char *buffer, uint64_t offset) :
      m_offset(offset),
      m_buffer(buffer)
    {
    }

    virtual inline uint64_t GetPosition()
    {
      return m_offset;
    }

    virtual inline void SetPosition(uint64_t position)
    {
      m_offset = position;
    }

    virtual inline bool Read(char *target, uint64_t count)
    {
      memcpy(target, m_buffer + m_offset, count);

      m_offset += count;

      return true;
    }

  private:
    uint64_t m_offset;

    char *m_buffer;
  };

  class BufferWriter : public Writer
  {
  public:
    BufferWriter(char *buffer, uint64_t offset) :
      m_offset(offset),
      m_buffer(buffer)
    {
    }

    virtual inline uint64_t GetPosition()
    {
      return m_offset;
    }

    virtual inline void SetPosition(uint64_t position)
    {
      m_offset = position;
    }

    virtual inline bool Write(char *source, uint64_t count)
    {
      memcpy(m_buffer + m_offset, source, count);

      m_offset += count;

      return true;
    }

  private:
    uint64_t m_offset;

    char *m_buffer;
  };

  class BufferReadWriter : public Reader, public Writer
  {
  public:
    BufferReadWriter(char *buffer, uint64_t offset) :
      m_offset(offset),
      m_buffer(buffer)
    {
    }

    virtual inline uint64_t GetPosition()
    {
      return m_offset;
    }

    virtual inline void SetPosition(uint64_t position)
    {
      m_offset = position;
    }

    virtual inline bool Read(char *target, uint64_t count)
    {
      memcpy(target, m_buffer + m_offset, count);

      m_offset += count;

      return true;
    }

    virtual inline bool Write(char *source, uint64_t count)
    {
      memcpy(m_buffer + m_offset, source, count);

      m_offset += count;

      return true;
    }

  private:
    uint64_t m_offset;

    char *m_buffer;
  };

  class SmiReader : public Reader
  {
  public:
    SmiReader(SmiRecord &record, uint64_t offset) :
      m_offset(offset),
      m_record(record)
    {
    }

    virtual inline uint64_t GetPosition()
    {
      return m_offset;
    }

    virtual inline void SetPosition(uint64_t position)
    {
      m_offset = position;
    }

    virtual inline bool Read(char *target, uint64_t count)
    {
      uint64_t read = m_record.Read(target, count, m_offset);

      m_offset += read;

      return read == count;
    }

  private:
    uint64_t m_offset;

    SmiRecord &m_record;
  };

  class SmiWriter : public Writer
  {
  public:
    SmiWriter(SmiRecord &record, uint64_t offset) :
      m_offset(offset),
      m_record(record)
    {
    }

    virtual inline uint64_t GetPosition()
    {
      return m_offset;
    }

    virtual inline void SetPosition(uint64_t position)
    {
      m_offset = position;
    }

    virtual inline bool Write(char *source, uint64_t count)
    {
      uint64_t written = m_record.Write(source, count, m_offset);

      m_offset += written;

      return written == count;
    }

  private:
    uint64_t m_offset;

    SmiRecord &m_record;
  };

  class SmiReadWriter : public Reader, public Writer
  {
  public:
    SmiReadWriter(SmiRecord &record, uint64_t offset) :
      m_offset(offset),
      m_record(record)
    {
    }

    virtual inline uint64_t GetPosition()
    {
      return m_offset;
    }

    virtual inline void SetPosition(uint64_t position)
    {
      m_offset = position;
    }

    virtual inline bool Read(char *target, uint64_t count)
    {
      uint64_t read = m_record.Read(target, count, m_offset);

      m_offset += read;

      return read == count;
    }

    virtual inline bool Write(char *source, uint64_t count)
    {
      uint64_t written = m_record.Write(source, count, m_offset);

      m_offset += written;

      return written == count;
    }

  private:
    uint64_t m_offset;

    SmiRecord &m_record;
  };
}