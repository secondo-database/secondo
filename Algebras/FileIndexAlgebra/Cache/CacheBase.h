/*
Basisklasse fuer File-Cache-Implementierung

*/

#ifndef CACHEBASE_H
#define CACHEBASE_H

#include <cstddef>    // size_t
#include <stdexcept>  // logic_error


namespace fialgebra {
  namespace cache {

    // Basisklasse saemtliche File-Cache-Implementierung
    class CacheBase
    {

    public:
      // ctor
      // fileName  : Dateiname, Datei muss existieren
      // pageSize  : Seitengroesse
      // pageCount : Anzahl Seiten, die maximal im Cache gehalten werden
      CacheBase( const char* fileName, size_t pageSize, size_t pageCount );
      virtual ~CacheBase();

      // 
      // Virtual methods
      // 
      // Liest die ersten 'length' Bytes der Seite 'pageNo'.
      // Liefert Zeiger auf char*-Array.
      virtual size_t Read( size_t pageNo, size_t length ) = 0;
      // Schreibt die ersten 'length' Bytes des char*-Array
      // auf die Seite 'pageNo'.
      virtual void Write( size_t pageNo, size_t arr, size_t length ) = 0;
      // Schreibt ausstehende Aenderungen in die Datei.
      virtual void Flush() = 0;
      // Anteil der Cache-Treffer (Anfragen, die aus dem Cache bedient werden
      // konnten) an der Gesamtanzahl an Anfragen. Liegt im Intervall [0,1].
      // Dient nur zur Info.
      virtual double GetHitRate() = 0;

      // Liest die komplette Seite 'pageNo'. Liefert Zeiger auf char*-Array.
      size_t Read( size_t pageNo );
      // Schreibt die ersten 'pageSize' Bytes des char*-Array
      // auf die Seite 'pageNo'.
      void Write( size_t pageNo, size_t arr );
      // Liefert den Index (die Nummer) der neuen Seite
      size_t NewPage();

      // Gibt die Gesamtanzahl an Seiten zurueck, die die
      // Cache-Datei aktuell aufnehmen kann. ( Dateigroesse / PageSize )
      size_t GetTotalPages();
      // Liefert die Groesse der Cache-Datei in Bytes.
      size_t GetFileSize();

    protected:
      // Liest die ersten 'length' Bytes der Seite 'pageNo'
      // aus der Cache-Datei. Liefert Zeiger auf char*-Array.
      size_t ReadFile( size_t pageNo, size_t length );
      // Schreibt die ersten 'length' Bytes des char*-Array auf
      // die Seite 'pageNo' in der Cache-Datei.
      void WriteFile( size_t pageNo, size_t arr, size_t length );

      // Datei-Stream
      std::fstream _fs;
      // Seitengroesse
      size_t _pageSize;
      // Max. Anzahl Seiten im Cache
      size_t _pageCount;

    }; // class CacheBase
  } // namespace cache
} // namespace fialgebra

#endif // CACHEBASE_H































