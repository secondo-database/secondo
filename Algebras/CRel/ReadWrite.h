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
#include <stdexcept>
#include "Utility.h"

class Reader
{
public:
  virtual size_t GetPosition() = 0;
  virtual void SetPosition(size_t position) = 0;

  virtual bool Read(char *target, size_t count) = 0;

  inline void ReadOrThrow(char *target, size_t count)
  {
    if (!Read(target, count))
    {
      throw std::runtime_error("Reading failed.");
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
      throw std::runtime_error("Reading failed.");
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
  virtual size_t GetPosition() = 0;
  virtual void SetPosition(size_t position) = 0;

  virtual bool Write(char *source, size_t count) = 0;

  inline void WriteOrThrow(char *source, size_t count)
  {
    if (!Write(source, count))
    {
      throw std::runtime_error("Writing failed.");
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
      throw std::runtime_error("Writing failed.");
    }
  }
};

class SmiReader : public Reader
{
public:
  SmiReader(SmiRecord &record, size_t offset) :
    m_offset(offset),
    m_record(record)
  {
  }

  virtual inline size_t GetPosition()
  {
    return m_offset;
  }

  virtual inline void SetPosition(size_t position)
  {
    m_offset = position;
  }

  virtual inline bool Read(char *target, size_t count)
  {
    size_t read = m_record.Read(target, count, m_offset);

    m_offset += read;

    return read == count;
  }

private:
  size_t m_offset;

  SmiRecord &m_record;
};

class SmiWriter : public Writer
{
public:
  SmiWriter(SmiRecord &record, size_t offset) :
    m_offset(offset),
    m_record(record)
  {
  }

  virtual inline size_t GetPosition()
  {
    return m_offset;
  }

  virtual inline void SetPosition(size_t position)
  {
    m_offset = position;
  }

  virtual inline bool Write(char *source, size_t count)
  {
    size_t written = m_record.Write(source, count, m_offset);

    m_offset += written;

    return written == count;
  }

private:
  size_t m_offset;

  SmiRecord &m_record;
};

class SmiReadWriter : public Reader, public Writer
{
public:
  SmiReadWriter(SmiRecord &record, size_t offset) :
    m_offset(offset),
    m_record(record)
  {
  }

  virtual inline size_t GetPosition()
  {
    return m_offset;
  }

  virtual inline void SetPosition(size_t position)
  {
    m_offset = position;
  }

  virtual inline bool Read(char *target, size_t count)
  {
    size_t read = m_record.Read(target, count, m_offset);

    m_offset += read;

    return read == count;
  }

  virtual inline bool Write(char *source, size_t count)
  {
    size_t written = m_record.Write(source, count, m_offset);

    m_offset += written;

    return written == count;
  }

private:
  size_t m_offset;

  SmiRecord &m_record;
};