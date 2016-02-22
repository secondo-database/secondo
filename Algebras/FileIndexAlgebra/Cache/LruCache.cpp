/*
Least recently used cache

*/

#include <fstream>    // ifstream
#include <functional> // std::placeholders
#include <iostream>   // cout
#include <list>       // list
#include <stdexcept>  // exception
#include <stdio.h>    // fopen
#include <string.h>   // memcpy
#include <utility>    // pair

#include "LruCache.h"

using namespace std;
using namespace std::placeholders;

namespace fialgebra {
  namespace cache {

    // Nested class
    // CacheElement::CacheElement()
    LruCache::CacheElement::CacheElement(
      size_t value,
      size_t length,
      bool changed )
      : _value( value ),
        _length( length ),
        _changed( changed ) { }
    // CacheElement::~CacheElement()
    LruCache::CacheElement::~CacheElement() { }
    // CacheElement::GetValue()
    size_t LruCache::CacheElement::GetValue() { return _value; }
    // CacheElement::GetLength()
    size_t LruCache::CacheElement::GetLength() { return _length; }
    // CacheElement::HasChanged()
    bool LruCache::CacheElement::HasChanged() { return _changed; }


    // ctor
    LruCache::LruCache(
      const char* fileName,
      size_t pageSize,
      size_t pageCount )
      : CacheBase( fileName, pageSize, pageCount ),
        _incCache( pageCount,
          std::bind( &LruCache::DeleteCallback, this, _1, _2 ) )  { }
    LruCache::~LruCache() {
      Flush();
    }

    // Read
    size_t LruCache::Read( size_t pageNo, size_t length ) {
      if( _incCache.Contains( pageNo ) ) {
        totalHit++;
        return ReadCache( pageNo, length );
      } // if

      totalMiss++;

      size_t arr = ReadFile( pageNo, length );
      // Das Element wird in den Cache geschrieben,
      // hat sich aber ja noch nicht geänder. Deshalb
      // ist der letzte Parameter false.
      WriteCache( pageNo, arr, length, false );

      return arr;
    }
    // Write
    void LruCache::Write( size_t pageNo, size_t arr, size_t length ) {
      // Page hat sich potentiell geaender, deshalb ist
      // der letzte Parameter true.
      WriteCache( pageNo, arr, length, true );
    }

    // private WriteCache()
    void LruCache::WriteCache( size_t pageNo, size_t arr,
      size_t length, bool changed ) {
      // Wir arbeiten im Cache mit Kopien der Daten, sonst wird
      // der Cache inkonsistent, sobald jemand eins der Objekte
      // aus dem Speicher löscht.
      char* buffer = new char[length];
      memcpy( buffer, (char*)arr, length );

      _incCache.Put( pageNo,
        CacheElement( (size_t)buffer, length, changed ) );
    }
    // private ReadCache()
    size_t LruCache::ReadCache( size_t pageNo, size_t length ) {
      CacheElement elem = _incCache.Get( pageNo );
      size_t arr = elem.GetValue();

      // Auch hier: Wir erstellen eine Kopie der Page.
      char* buffer = new char[length];
      memcpy( buffer, (char*)arr, length );

      return (size_t)buffer;
    }

    // private DeleteCallback()
    void LruCache::DeleteCallback( size_t key, CacheElement elem ) {
      // Write back
      if( elem.HasChanged() )
        WriteFile( key, elem.GetValue(), elem.GetLength() );

      // Objekte, die aus dem Cache fliegen, werden auch
      // aus dem Speicher gelöscht.
      char* arr = (char*)elem.GetValue();
      delete[] arr;
    }

    // Flush
    void LruCache::Flush() {
      _incCache.Clear();
    }
    // GetHitRate
    double LruCache::GetHitRate() {
      size_t total = totalHit + totalMiss;
      if( total == 0 ) return 0.0;

      return (double)totalHit / total;
    }

  } // namespace cache
} // namespace fialgebra





































