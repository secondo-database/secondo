/*

1 Implementation of Class SmiKey 

May 2002 Ulrich Telle

*/
using namespace std;

#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>

#include "SecondoSMI.h"
#include "SmiCodes.h"

SmiKey::SmiKey( MapKeyFunc mapKey /* = 0 */ )
{
  keyType = SmiKey::Unknown;
  mapFunc = mapKey;
}

SmiKey::SmiKey( const SmiRecordId key )
{
  keyType = SmiKey::Unknown;
  SetKey( key );
}

SmiKey::SmiKey( const long key )
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

SmiKey::SmiKey( const void* key, const SmiSize keyLen, MapKeyFunc mapKey )
{
  keyType = SmiKey::Unknown;
  SetKey( key, keyLen, mapKey );
}

SmiKey::SmiKey( const SmiKey& other )
{
  keyType   = other.keyType;
  keyLength = other.keyLength;
  mapFunc   = other.mapFunc;
  SetKey( other.keyType, other.GetAddr(), other.keyLength, other.mapFunc );
}

SmiKey::~SmiKey()
{
  FreeData();
}

SmiKey&
SmiKey::operator=( const SmiKey& other )
{
  SetKey( other.keyType, other.GetAddr(), other.keyLength, other.mapFunc );
  return *this;
}

const bool
SmiKey::operator>( const SmiKey& other )
{
  assert( keyType == other.GetType() );
  bool ok;
  switch (keyType)
  {
    case SmiKey::RecNo:
      ok = (recnoKey > other.recnoKey);
      break;
    case SmiKey::Integer:
      ok = (integerKey > other.integerKey);
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
       keyLength > SMI_MAX_KEYLEN_LOCAL )
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
    case SmiKey::RecNo:     addr = &recnoKey;   break;
    case SmiKey::Integer:   addr = &integerKey; break;
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
SmiKey::SetKey( const SmiRecordId key )
{
  FreeData();
  keyType   = SmiKey::RecNo;
  keyLength = sizeof( key );
  recnoKey = key;
}

void
SmiKey::SetKey( const long key )
{
  FreeData();
  keyType    = SmiKey::Integer;
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
SmiKey::SetKey( const void* key, const SmiSize keyLen, MapKeyFunc mapKey )
{
  static char mapdata[SMI_MAX_KEYLEN];
  char* data;

  FreeData();
  keyType = SmiKey::Composite;
  mapFunc = mapKey;
  if ( mapFunc != 0 )
  {
    (*mapFunc)( key, keyLen, mapdata, SMI_MAX_KEYLEN, keyLength, true );
  }
  else
  {
    keyLength = (keyLen <= SMI_MAX_KEYLEN) ? keyLen : SMI_MAX_KEYLEN;
    memcpy( mapdata, key, keyLength );
  }

  if ( keyLength > SMI_MAX_KEYLEN )
    keyLength = SMI_MAX_KEYLEN;
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
                const void* key, const SmiSize keyLen,
                MapKeyFunc mapKey /* = 0 */ )
{
  switch (kdt)
  {
    case SmiKey::RecNo:
      SetKey( *((SmiRecordId*) key) );
      break;
    case SmiKey::Integer:
      SetKey( *((long*) key) );
      break;
    case SmiKey::Float:
      SetKey( *((double*) key) );
      break;
    case SmiKey::String:
    case SmiKey::Composite:
      FreeData();
      keyType   = kdt;
      keyLength = keyLen;
      mapFunc   = mapKey;
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
SmiKey::GetKey( SmiRecordId& key )
{
  bool ok = false;

  if ( keyType == SmiKey::RecNo )
  {
    ok = true;
    key = recnoKey;
  }

  return ok;
}

bool
SmiKey::GetKey( long& key )
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
SmiKey::GetKey( double& key )
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
SmiKey::GetKey( string& key )
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
SmiKey::GetKey( void* key, const SmiSize maxKeyLen, SmiSize& keyLen )
{
  bool ok = false;
  char* data;

  if ( keyType == SmiKey::Composite )
  {
    if ( keyLength > SMI_MAX_KEYLEN_LOCAL )
    {
      data = (char*) longKeyData;
    }
    else
    {
      data = shortKeyData;
    }
    if ( mapFunc != 0 )
    {
      (*mapFunc)( data, keyLength, key, maxKeyLen, keyLen, false );
    }
    else
    {
      keyLen = (keyLength <= maxKeyLen) ? keyLength : maxKeyLen;
      memcpy( key, data, keyLen );
    }
    ok = true;
  }

  return ok;
}

void
SmiKey::Map( const long inData, void* outData )
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
SmiKey::Unmap( const void* inData, long& outData )
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

