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

1 Implementation of Class SmiKey

May 2002 Ulrich Telle

*/

#include <string.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>

#include "SecondoSMI.h"
#include "SmiCodes.h"
#include "IndexableAttribute.h"

using namespace std;

SmiKey::SmiKey()
{
  keyType = SmiKey::Unknown;
  keyLength = 0;
  integerKey = 0;
}


SmiKey::SmiKey( const int32_t key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}

SmiKey::SmiKey( const int64_t key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}


SmiKey::SmiKey( const uint32_t key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}

SmiKey::SmiKey( const uint64_t key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}

SmiKey::SmiKey( const double key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}

SmiKey::SmiKey( const string& key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}

SmiKey::SmiKey( const IndexableAttribute* key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}

SmiKey::SmiKey( const SmiKey& other )
{
  keyType = SmiKey::Unknown;
  SetKey( other.keyType, other.GetAddr(), other.keyLength );
}

SmiKey::~SmiKey()
{
  FreeData();
}

SmiKey&
SmiKey::operator=( const SmiKey& other )
{
  SetKey( other.keyType, other.GetAddr(), other.keyLength );
  return *this;
}

const bool
SmiKey::operator==( const SmiKey& other ) const
{
  if( keyType != other.GetType() )
    return false;

  bool ok = false;
  switch (keyType)
  {
    case SmiKey::Integer:
      ok = (integerKey == other.integerKey);
      break;
    case SmiKey::Longint:
      ok = (longintKey == other.longintKey);
      break;
    case SmiKey::Float:
      ok = (floatKey == other.floatKey);
      break;
    case SmiKey::String:
    case SmiKey::Composite:
      {
        const void* key1 = ( keyLength > SMI_MAX_KEYLEN_LOCAL ) ?
                             longKeyData : shortKeyData;
        const void* key2 = ( other.keyLength > SMI_MAX_KEYLEN_LOCAL ) ?
                             other.longKeyData : other.shortKeyData;
        if ( keyLength == other.keyLength )
        {
          ok = (memcmp( key1, key2, other.keyLength ) == 0);
        }
      }
      break;
    default:
      break;
  }
  return (ok);
}

const bool
SmiKey::operator>( const SmiKey& other ) const
{
  assert( keyType == other.GetType() );
  bool ok;
  switch (keyType)
  {
    case SmiKey::Integer:
      ok = (integerKey > other.integerKey);
      break;
    case SmiKey::Longint:
      ok = (longintKey > other.longintKey);
      break;
    case SmiKey::Float:
      ok = (floatKey > other.floatKey);
      break;
    case SmiKey::String:
    case SmiKey::Composite:
      {
        const void* key1 = ( keyLength > SMI_MAX_KEYLEN_LOCAL ) ?
                             longKeyData : shortKeyData;
        const void* key2 = ( other.keyLength > SMI_MAX_KEYLEN_LOCAL ) ?
                             other.longKeyData : other.shortKeyData;
        if ( keyLength > other.keyLength )
        {
          ok = (memcmp( key1, key2, other.keyLength ) >= 0);
        }
        else
        {
          ok = (memcmp( key1, key2, keyLength ) > 0);
        }
      }
      break;
    case SmiKey::Unknown:
    default:
      ok = false;
      break;
  }
  return (ok);
}

const SmiKey::KeyDataType
SmiKey::GetType() const
{
  return keyType;
}

void
SmiKey::FreeData()
{
  if ( keyType != SmiKey::Unknown  &&
       keyLength > SMI_MAX_KEYLEN_LOCAL
     )
  {
    delete []longKeyData;
  }
}

const void*
SmiKey::GetAddr() const
{
  const void* addr;

  switch ( keyType )
  {
    case SmiKey::Integer:   addr = &integerKey; break;
    case SmiKey::Longint:   addr = &longintKey; break;
    case SmiKey::Float:     addr = &floatKey;   break;
    case SmiKey::String:
    case SmiKey::Composite: addr = (keyLength > SMI_MAX_KEYLEN_LOCAL) ?
                                    longKeyData : shortKeyData; break;
    case SmiKey::Unknown:
    default:                addr = 0; break;
  }

  return addr;
}


void
SmiKey::SetKey( const int32_t key )
{
  FreeData();
  keyType    = SmiKey::Integer;
  keyLength  = sizeof( key );
  integerKey = key;
}

void
SmiKey::SetKey( const int64_t key )
{
  FreeData();
  keyType    = SmiKey::Longint;
  keyLength  = sizeof( key );
  integerKey = key;
}

void
SmiKey::SetKey( const uint32_t key )
{
  FreeData();
  keyType    = SmiKey::Integer;
  keyLength  = sizeof( key );
  integerKey = key;
}

void
SmiKey::SetKey( const uint64_t key )
{
  FreeData();
  keyType    = SmiKey::Longint;
  keyLength  = sizeof( key );
  integerKey = key;
}


void
SmiKey::SetKey( const double key )
{
  FreeData();
  keyType   = SmiKey::Float;
  keyLength = sizeof( key );
  floatKey  = key;
}

void
SmiKey::SetKey( const string& key )
{
  FreeData();
  keyType = SmiKey::String;
  keyLength  = (key.length() <= SMI_MAX_KEYLEN) ? key.length() : SMI_MAX_KEYLEN;
  if ( keyLength > SMI_MAX_KEYLEN_LOCAL )
  {
    longKeyData = new char[keyLength+1];
    key.copy( (char*) longKeyData, keyLength );
    longKeyData[keyLength] = 0;
  }
  else
  {
    key.copy( shortKeyData, keyLength );
    shortKeyData[keyLength] = 0;
  }
}

void
SmiKey::SetKey( const IndexableAttribute* key )
{
  static char mapdata[SMI_MAX_KEYLEN];
  char* data;

  FreeData();
  keyType = SmiKey::Composite;

  keyLength = key->SizeOfChars();
  assert( keyLength < SMI_MAX_KEYLEN );
  key->WriteTo( mapdata );

  if ( keyLength > SMI_MAX_KEYLEN_LOCAL )
  {
    longKeyData = new char[keyLength+1];
    data = longKeyData;
  }
  else
  {
    data = shortKeyData;
  }
  memcpy( data, mapdata, keyLength );
  data[keyLength] = 0;
}

void
SmiKey::SetKey( const KeyDataType kdt,
                const void* key, const SmiSize keyLen )
{
  switch (kdt)
  {
    case SmiKey::Integer:
      SetKey( *((int32_t*) key) );
      break;
    case SmiKey::Longint:
      SetKey( *((int64_t*) key) );
      break;
    case SmiKey::Float:
      SetKey( *((double*) key) );
      break;
    case SmiKey::String:
    case SmiKey::Composite:
      FreeData();
      keyType   = kdt;
      keyLength = keyLen;
      if ( keyLength > SMI_MAX_KEYLEN_LOCAL )
      {
        longKeyData = new char[keyLength+1];
        memcpy( longKeyData, key, keyLength );
        longKeyData[keyLength] = 0;
      }
      else
      {
        memcpy( shortKeyData, key, keyLength );
        shortKeyData[keyLength] = 0;
      }
      break;
    case SmiKey::Unknown:
    default:
      FreeData();
      keyType = kdt;
      break;
  }
}


bool
SmiKey::GetKey( int32_t& key ) const
{
  bool ok = false;

  if ( keyType == SmiKey::Integer )
  {
    ok = true;
    key = integerKey;
  }

  return ok;
}

bool
SmiKey::GetKey( int64_t& key ) const
{
  bool ok = false;

  if ( keyType == SmiKey::Longint )
  {
    ok = true;
    key = longintKey;
  }

  return ok;
}

bool
SmiKey::GetKey( uint32_t& key ) const
{
  bool ok = false;

  if ( keyType == SmiKey::Integer )
  {
    ok = true;
    key = integerKey;
  }

  return ok;
}

bool
SmiKey::GetKey( uint64_t& key ) const
{
  bool ok = false;

  if ( keyType == SmiKey::Longint )
  {
    ok = true;
    key = longintKey;
  }

  return ok;
}

bool
SmiKey::GetKey( double& key ) const
{
  bool ok = false;

  if ( keyType == SmiKey::Float )
  {
    ok = true;
    key = floatKey;
  }

  return ok;
}

bool
SmiKey::GetKey( string& key ) const
{
  bool ok = false;

  if ( keyType == SmiKey::String )
  {
    ok = true;
    if ( keyLength > SMI_MAX_KEYLEN_LOCAL )
    {
      key.assign( (char*) longKeyData, keyLength );
    }
    else
    {
      key.assign( shortKeyData, keyLength );
    }
  }

  return ok;
}

bool
SmiKey::GetKey( IndexableAttribute* key ) const
{
  bool ok = false;
  const char* data;

  if ( keyType == SmiKey::Composite )
  {
    if ( keyLength > SMI_MAX_KEYLEN_LOCAL )
    {
      data = (const char*) longKeyData;
    }
    else
    {
      data = shortKeyData;
    }

    key->ReadFrom( data );
    ok = true;
  }

  return ok;
}

SmiSize SmiKey::GetLength() const {
  if(keyType == SmiKey::Composite)
    return keyLength;
  return -1;
}

void
SmiKey::Map( const int32_t inData, void* outData )
{
  const unsigned char* s = (unsigned char*) &inData;
        unsigned char* t = (unsigned char*) outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[sizeof( inData )-j-1];
  }
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
#endif
  t[0] ^= 0x80;
}

void
SmiKey::Map( const int64_t inData, void* outData )
{
  const unsigned char* s = (unsigned char*) &inData;
        unsigned char* t = (unsigned char*) outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[sizeof( inData )-j-1];
  }
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
#endif
  t[0] ^= 0x80;
}

void
SmiKey::Map( const uint32_t inData, void* outData )
{
  const unsigned char* s = (unsigned char*) &inData;
        unsigned char* t = (unsigned char*) outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[sizeof( inData )-j-1];
  }
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
#endif
  t[0] ^= 0x80;
}

void
SmiKey::Map( const uint64_t inData, void* outData )
{
  const unsigned char* s = (unsigned char*) &inData;
        unsigned char* t = (unsigned char*) outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[sizeof( inData )-j-1];
  }
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
#endif
  t[0] ^= 0x80;
}


void
SmiKey::Map( const double inData, void* outData )
{
  const unsigned char* s = (unsigned char*) &inData;
        unsigned char* t = (unsigned char*) outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[sizeof( inData )-j-1];
    if ( inData < 0.0 )
    {
      t[j] ^= 0xff;
    }
  }
#else
  for ( int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
#endif
  if ( inData >= 0.0 )
  {
    t[0] ^= 0x80;
  }
}

void
SmiKey::Unmap( const void* inData, int32_t& outData )
{
  const unsigned char* s = (unsigned char*) inData;
        unsigned char* t = (unsigned char*) &outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( outData ); j++ )
  {
    t[j] = s[sizeof( outData )-j-1];
  }
  t[sizeof( outData )-1] ^= 0x80;
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
  t[0] ^= 0x80;
#endif
}

void
SmiKey::Unmap( const void* inData, int64_t& outData )
{
  const unsigned char* s = (unsigned char*) inData;
        unsigned char* t = (unsigned char*) &outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( outData ); j++ )
  {
    t[j] = s[sizeof( outData )-j-1];
  }
  t[sizeof( outData )-1] ^= 0x80;
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
  t[0] ^= 0x80;
#endif
}

void
SmiKey::Unmap( const void* inData, uint32_t& outData )
{
  const unsigned char* s = (unsigned char*) inData;
        unsigned char* t = (unsigned char*) &outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( outData ); j++ )
  {
    t[j] = s[sizeof( outData )-j-1];
  }
  t[sizeof( outData )-1] ^= 0x80;
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
  t[0] ^= 0x80;
#endif
}

void
SmiKey::Unmap( const void* inData, uint64_t& outData )
{
  const unsigned char* s = (unsigned char*) inData;
        unsigned char* t = (unsigned char*) &outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( outData ); j++ )
  {
    t[j] = s[sizeof( outData )-j-1];
  }
  t[sizeof( outData )-1] ^= 0x80;
#else
  for ( unsigned int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
  }
  t[0] ^= 0x80;
#endif
}

void
SmiKey::Unmap( const void* inData, double& outData )
{
  const unsigned char* s = (unsigned char*) inData;
        unsigned char* t = (unsigned char*) &outData;
#ifdef SECONDO_LITTLE_ENDIAN
  for ( unsigned int j = 0; j < sizeof( outData ); j++ )
  {
    t[j] = s[sizeof( outData )-j-1];
    if ( !(s[0] & 0x80) )
    {
      t[j] ^= 0xff;
    }
  }
  if ( s[0] & 0x80 )
  {
    t[sizeof( outData )-1] ^= 0x80;
  }
#else
  for ( int j = 0; j < sizeof( inData ); j++ )
  {
    t[j] = s[j];
    if ( !(s[0] & 0x80) )
    {
      t[j] ^= 0xff;
    }
  }
  if ( s[0] & 0x80 )
  {
    t[0] ^= 0x80;
  }
#endif
}

/* --- smiKey.cpp --- */

