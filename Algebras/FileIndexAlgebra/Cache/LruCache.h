/*
Least recently used cache

*/

#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <cstddef>   // size_t

#include "CacheBase.h"
#include "IncLruCache.h"


namespace fialgebra {
  namespace cache {

    class LruCache : public CacheBase
    {
    public:
      // ctor
      // fileName  : Dateiname, Datei muss existieren
      // pageSize  : Seitengroesse
      // pageCount : Anzahl Seiten, die maximal im Cache gehalten werden
      LruCache( const char* fileName, size_t pageSize, size_t pageCount );
      ~LruCache();

      // CacheBase::Read()
      // Liest die ersten 'length' Bytes der Seite 'pageNo'.
      // Liefert Zeiger auf char*-Array.
      size_t Read( size_t pageNo, size_t length );
      // CacheBase::Write()
      // Schreibt die ersten 'length' Bytes des char*-Array
      // auf die Seite 'pageNo'.
      void Write( size_t pageNo, size_t arr, size_t length );
      // CacheBase::Flush()
      // Schreibt ausstehende Aenderungen in die Datei.
      void Flush();
      // CacheBase::GetHitRate()
      // Anteil der Cache-Treffer (Anfragen, die aus dem Cache bedient werden
      // konnten) an der Gesamtanzahl an Anfragen. Liegt im Intervall [0,1].
      // Dient nur zur Info.
      double GetHitRate();

    private:
      // Nested class
      // Element, das im Cache landet und neben den eigentlichen Daten der 
      // entsprechenden Page noch ein paar Verwaltungs-Infos enthaellt.
      class CacheElement
      {
      public:
        // ctor
        CacheElement( size_t value, size_t length, bool changed );
        ~CacheElement();

        // Value, hier Adresse auf char* der Page
        size_t GetValue();
        // Gruesse der Page
        size_t GetLength();
        // Element hat sich geaendert
        bool HasChanged();

      private:
        size_t _value;
        size_t _length;
        bool   _changed;
      };


      // Schreibt eine Page in den Cache
      void WriteCache( size_t pageNo, size_t arr, 
        size_t length, bool changed );
      // Liesst eine Page aus dem Cache
      size_t ReadCache( size_t pageNo, size_t length );

      // Delete callback, wird vom Cache aufgerufen, wenn ein
      // Element aus diesem entfernt wird.
      void DeleteCallback( size_t key, CacheElement elem );

      // Eigentlicher Cache
      include::IncLruCache<size_t, CacheElement> _incCache;
      // Gesamtzahl Page-Misses
      size_t totalMiss = 0;
      // Gesamtzahl Page-Hits
      size_t totalHit = 0;

    }; // class LruCache

  } // namespace cache
} // namespace fialgebra

#endif // LRUCACHE_H






















