/*
Basisklasse fuer File-Cache-Implementierung

*/

#include <cstddef>     // size_t
#include <cstring>     // strlen()
#include <fstream>     // ifstream
#include <iostream>    // cout
#include <stdexcept>   // exception

#include "CacheBase.h"

using namespace std;

namespace fialgebra {
  namespace cache {

    // ctor
    CacheBase::CacheBase(
      const char* fileName,
      size_t pageSize,
      size_t pageCount )
      : _pageSize( pageSize ),
        _pageCount( pageCount ) {

      // Stream oeffnen
      _fs.open( fileName, ios::in | ios::out | ios::binary );
      if( !_fs.good() ) {
        _fs.close();
        throw runtime_error(
          "CacheBase.NewPage(): Datei kann nicht geoeffnet werden" );
      } // if
    }
    CacheBase::~CacheBase() {
      // Stream schliessen
      _fs.close();
    }

    // Read
    size_t CacheBase::Read( size_t pageNo ) {
      return Read( pageNo, _pageSize );
    }
    // Write
    void CacheBase::Write( size_t pageNo, size_t arr ) {
      Write( pageNo, arr, _pageSize );
    }

    // NewPage
    size_t CacheBase::NewPage() {
      char* buffer = new char[_pageSize];

      // Cursor ans Ende setzen
      _fs.seekp( 0, ios::end );
      // Neue Seite anhaengen
      _fs.write( buffer, _pageSize );
      // Cursor steht jetzt hinter der neuen Page
      streampos l = _fs.tellp();

      delete[] buffer;

      return ( (size_t)l / _pageSize ) - 1;
    }

    // GetFileSize
    size_t CacheBase::GetFileSize() {
      // Cursor ans Ende setzen
      _fs.seekg( 0, ios::end );
      // Position lesen
      streampos l = _fs.tellg();
      return (size_t)l;
    }
    // GetTotalPages
    size_t CacheBase::GetTotalPages() {
      // Falls mal jemand pageSize auf 0 setzt...
      if( _pageSize == 0 ) return 0;

      size_t size = GetFileSize();
      return size / _pageSize;
    }

    // protected ReadFile
    size_t CacheBase::ReadFile( size_t pageNo, size_t length ) {
      size_t offset = pageNo * _pageSize;

      // Dateigroesse auslesen, gesuchte Page
      // muss in der Datei liegen
      size_t l = GetFileSize();

      if( offset + length > l )
        throw out_of_range(
        "CacheBase.ReadFile(): Die geforderte Seite "
        "uebersteigt die Dateigroesse" );

      char* buffer = new char[length];
      // Position setzen und Seite auslesen
      _fs.seekg( offset, ios::beg );
      _fs.read( buffer, length );

      return (size_t)buffer;
    }
    // protected WriteFile
    void CacheBase::WriteFile(
      size_t pageNo,
      size_t arr,
      size_t length ) {

      // Wenn mehr in eine Seite geschrieben werden soll
      // als Platz ist, geht das nicht...
      if( length > _pageSize )
        throw out_of_range( "CacheBase.WriteFile(): "
        "Daten passen nicht in eine Seite" );

      // Offset im file
      size_t fileOffset = pageNo * _pageSize;

      // Page muss in File liegen
      size_t l = GetFileSize();
      if( fileOffset + length > l )
        throw out_of_range( "CacheBase.WriteFile(): Seite "
        "liegt ausserhalb der Datei" );

      // Position setzen und Seite schreiben
      _fs.seekp( fileOffset, ios::beg );
      _fs.write( (char*)arr, length );
    }

  } // namespace cache
} // namespace fialgebra






































