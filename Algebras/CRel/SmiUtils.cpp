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

#include "SmiUtils.h"

#include "SecondoException.h"
#include "StringUtils.h"

using namespace CRelAlgebra;

using stringutils::any2str;

void CRelAlgebra::WriteOrThrow(SmiRecordFile &target, SmiRecordId recordId,
                               char *source, size_t count, size_t &offset)
{
  size_t writtenCount;

  target.Write(recordId, source, count, offset, writtenCount);

  if (writtenCount != count)
  {
    throw SecondoException("Writing to SmiRecordFile failed.");
  }
  else
  {
    offset += writtenCount;
  }
}

void CRelAlgebra::WriteOrThrow(SmiRecord &target, char *source, size_t count,
                               size_t &offset)
{
  size_t writtenCount = target.Write(source, count, offset);

  if (writtenCount != count)
  {
    throw SecondoException("Writing to SmiRecord failed.");
  }
  else
  {
    offset += writtenCount;
  }
}

void CRelAlgebra::ReadOrThrow(char *target, SmiRecordFile &source,
                              SmiRecordId recordId, size_t count,
                              size_t &offset)
{
  size_t readCount;

  source.Read(recordId, target, count, offset, readCount);

  if (readCount != count)
  {
    throw SecondoException("Reading from SmiRecordFile failed.");
  }
  else
  {
    offset += readCount;
  }
}

void CRelAlgebra::ReadOrThrow(char *target, SmiRecord &source, size_t count,
                              size_t &offset)
{
  size_t readCount = source.Read(target, count, offset);

  if (readCount != count)
  {
    throw SecondoException("Reading from SmiRecord failed.");
  }
  else
  {
    offset += readCount;
  }
}

char *CRelAlgebra::ReadOrThrow(SmiRecordFile &source, SmiRecordId recordId,
                               size_t count, size_t &offset)
{
  size_t readCount;

  char *target = new char[count];

  source.Read(recordId, target, count, offset, readCount);

  if (readCount != count)
  {
    delete[] target;

    throw SecondoException("Reading from SmiRecord failed.");
  }
  else
  {
    offset += readCount;

    return target;
  }
}

char *CRelAlgebra::ReadOrThrow(SmiRecord &source, size_t count,
                               size_t &offset)
{
  char *target = new char[count];
  size_t readCount = source.Read(target, count, offset);

  if (readCount != count)
  {
    delete[] target;

    throw SecondoException("Reading from SmiRecord failed.");
  }
  else
  {
    offset += readCount;

    return target;
  }
}

void CRelAlgebra::CreateOrThrow(SmiFile &file)
{
  if (!file.Create())
  {
    throw SecondoException("Creating file failed.");
  }
}

void CRelAlgebra::OpenOrThrow(SmiFile &file, SmiFileId id)
{
  if (!file.Open(id))
  {
    throw SecondoException("Opening file failed. fileId: " + any2str(id) + ".");
  }
}

void CRelAlgebra::CloseOrThrow(SmiFile &file, bool sync)
{
  if (!file.Close(sync))
  {
    throw SecondoException("Closing file failed. fileId: " +
                           any2str(file.GetFileId()) + ".");
  }
}

void CRelAlgebra::DropOrThrow(SmiFile &file)
{
  if (!file.Drop())
  {
    throw SecondoException("Droping file failed. fileId: " +
                           any2str(file.GetFileId()) + ".");
  }
}

void CRelAlgebra::AppendOrThrow(SmiRecordFile &file, SmiRecordId &id)
{
  SmiRecord record;
  if (!file.AppendRecord(id, record))
  {
    throw SecondoException("Appending record failed. fileId: " +
                           any2str(file.GetFileId()) + ".");
  }
}

void CRelAlgebra::AppendOrThrow(SmiRecordFile &file, SmiRecord &record)
{
  SmiRecordId id;
  if (!file.AppendRecord(id, record))
  {
    throw SecondoException("Appending record failed. fileId: " +
                           any2str(file.GetFileId()) + ".");
  }
}

void CRelAlgebra::AppendOrThrow(SmiRecordFile &file, SmiRecordId &id,
                                SmiRecord &record)
{
  if (!file.AppendRecord(id, record))
  {
    throw SecondoException("Appending record failed. fileId: " +
                           any2str(file.GetFileId()) + ".");
  }
}

void CRelAlgebra::SelectOrThrow(SmiRecordFile &file, SmiRecordId id,
                   SmiFile::AccessType accessType, SmiRecord &record)
{
  if (!file.SelectRecord(id, record, accessType))
  {
    throw SecondoException("Selecting record failed. fileId: " +
                           any2str(file.GetFileId()) + ", recordId: " +
                           any2str(id) + ", accessType: " +
                           any2str(accessType) + ".");
  }
}

void CRelAlgebra::DeleteOrThrow(SmiRecordFile &file, SmiRecordId id)
{
  if (!file.DeleteRecord(id))
  {
    throw SecondoException("Deleting record failed. fileId: " +
                           any2str(file.GetFileId()) + ", recordId: " +
                           any2str(id) + ".");
  }
}
