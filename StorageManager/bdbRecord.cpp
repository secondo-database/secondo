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

1 Implementation of SmiRecord using the Berkeley-DB 

April 2002 Ulrich Telle

September 2002 Ulrich Telle, fixed flag (DB\_DIRTY\_READ) in Berkeley DB calls
for system catalog files

November 30, 2002 RHG Added function ~GetKey~

April 2003 Ulrich Telle, implemented temporary SmiFiles

*/

using namespace std;

#include <string>
#include <algorithm>
#include <cctype>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Counter.h"
#include "WinUnix.h"

/* --- Implementation of class SmiRecord --- */

SmiRecord::Implementation::Implementation()
  : bdbFile( 0 ), bdbCursor( 0 ), useCursor( false ), closeCursor( false )
{
}

SmiRecord::Implementation::~Implementation()
{
}

SmiRecord::SmiRecord()
  : smiFile( 0 ), recordKey(), recordSize( 0 ), fixedSize( false ),
    initialized( false ), writable( false )
{
  impl = new Implementation();
}
  
SmiRecord::~SmiRecord()
{
  if ( initialized )
  {
    Finish();
  }
  delete impl;
}

SmiSize
SmiRecord::Read( void* buffer, 
                 const SmiSize numberOfBytes,
                 const SmiSize offset /* = 0 */ )
{
  static const int pageSize = WinUnix::getPageSize();
  static long int& ctr = Counter::getRef("SmiRecord::Read:Calls");
  static long int& byteCtr = Counter::getRef("SmiRecord::Read:Bytes");
  static long int& pageCtr = Counter::getRef("SmiRecord::Read:Pages");
  ctr++;

  int rc = 0;
  int actRead = 0;

  if ( initialized )
  {
    Dbt key;
    Dbt data;
    data.set_data( buffer );
    data.set_ulen( numberOfBytes );
    data.set_flags( DB_DBT_PARTIAL | DB_DBT_USERMEM );
    data.set_dlen( numberOfBytes );
    data.set_doff( offset );

    if ( impl->useCursor )
    {
      rc = impl->bdbCursor->get( &key, &data, DB_CURRENT );
    }
    else
    {
      DbTxn* tid = !smiFile->impl->isTemporaryFile ? 
                        SmiEnvironment::instance.impl->usrTxn : 0;
      u_int32_t flags = (writable && !smiFile->impl->isTemporaryFile) ? 
                           DB_RMW : 0;

      key.set_data( (void*) recordKey.GetAddr() );
      key.set_size( recordKey.keyLength );
      if ( writable || !smiFile->impl->isSystemCatalogFile )
      {
        rc = impl->bdbFile->get( tid, &key, &data, flags );
      }
      else
      {
        flags = (!smiFile->impl->isTemporaryFile) ? DB_DIRTY_READ : 0;
        rc = impl->bdbFile->get( 0, &key, &data, flags );
      }
    }

    if ( rc == 0 )
    {
      actRead = data.get_size();
      byteCtr += actRead;
      pageCtr = byteCtr / pageSize;
      SmiEnvironment::SetError( E_SMI_OK );
    }
    else
    {
      SmiEnvironment::SetBDBError( rc );
    }
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_NOTINIT );
  }

  return (actRead);
}

SmiSize
SmiRecord::Write( const void*   buffer, 
                  const SmiSize numberOfBytes, 
                  const SmiSize offset /* = 0 */ )
{
  static const int pageSize = WinUnix::getPageSize();
  static long int& ctr = Counter::getRef("SmiRecord::Write:Calls");
  static long int& byteCtr = Counter::getRef("SmiRecord::Write:Bytes");
  static long int& pageCtr = Counter::getRef("SmiRecord::Write:Pages");
  ctr++;

  int rc = 0;
  if ( initialized && writable )
  {
    Dbt key;
    Dbt data( (void*) buffer, numberOfBytes );
    data.set_flags( DB_DBT_PARTIAL );
    data.set_dlen( numberOfBytes );
    data.set_doff( offset );

    if ( impl->useCursor )
    {
      rc = impl->bdbCursor->put( &key, &data, DB_CURRENT );
    }
    else
    {
      DbTxn* tid = !smiFile->impl->isTemporaryFile ? 
                        SmiEnvironment::instance.impl->usrTxn : 0;
      key.set_data( (void*) recordKey.GetAddr() );
      key.set_size( recordKey.keyLength );
      rc = impl->bdbFile->put( tid, &key, &data, 0 );
    }
    if ( rc == 0 )
    {
      if ( offset + numberOfBytes > recordSize )
      {
        recordSize = offset + numberOfBytes;
      }
      SmiEnvironment::SetError( E_SMI_OK );
      byteCtr += numberOfBytes;
      pageCtr = byteCtr / pageSize;
    }
    else
    {
      SmiEnvironment::SetBDBError( rc );
    }
  }
  else
  {
    if ( !initialized )
    {
      SmiEnvironment::SetError( E_SMI_RECORD_NOTINIT );
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_RECORD_READONLY );
    }
    rc = -1;
  }

  return ((rc == 0) ? numberOfBytes : 0);
}

SmiSize
SmiRecord::Size()
{
  return (recordSize);
}

SmiKey
SmiRecord::GetKey()
{
  return recordKey;
}
  
bool
SmiRecord::Truncate( const SmiSize newSize )
{
  int rc = 0;

  if ( initialized && !fixedSize )
  {
    if ( newSize < recordSize )
    {
      char buffer;
      Dbt key;
      Dbt data( &buffer, 0 );
      data.set_flags( DB_DBT_PARTIAL );
      data.set_dlen( recordSize-newSize );
      data.set_doff( newSize );
      if ( impl->useCursor )
      {
        rc = impl->bdbCursor->put( &key, &data, DB_CURRENT );
      }
      else
      {
        DbTxn* tid = !smiFile->impl->isTemporaryFile ? 
                          SmiEnvironment::instance.impl->usrTxn : 0;
        key.set_data( (void*) recordKey.GetAddr() );
        key.set_size( recordKey.keyLength );
        rc = impl->bdbFile->put( tid, &key, &data, 0 );
      }

      if ( rc == 0 )
      {
        recordSize = newSize;
        SmiEnvironment::SetError( E_SMI_OK );
      }
      else
      {
        SmiEnvironment::SetBDBError( rc );
      }
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_RECORD_TRUNCATE );
    }
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_NOTINIT );
    rc = -1;
  }

  return (rc == 0);
}

void
SmiRecord::Finish()
{
  if ( impl->useCursor && impl->closeCursor && impl->bdbCursor != 0 )
  {
    impl->bdbCursor->close();
    impl->bdbCursor   = 0;
    impl->useCursor   = false;
    impl->closeCursor = false;
  }
  initialized = false;
  recordSize  = 0;
}

/* --- bdbRecord.cpp --- */

