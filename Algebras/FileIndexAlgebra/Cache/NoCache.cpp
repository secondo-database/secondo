/*
Cache, der gar kein Cache ist.
Lese- und Schreibzugriffe werden direkt auf dem
Dateisystem durchgefuert.

*/

#include <iostream>     // cout
#include <fstream>      // ifstream
#include <stdexcept>    // exception
#include <stdio.h>    // fopen

#include "NoCache.h"

using namespace std;

namespace fialgebra {
  namespace cache {

    // ctor
    NoCache::NoCache( const char* fileName, size_t pageSize, size_t pageCount )
      : CacheBase( fileName, pageSize, pageCount ) { }
    NoCache::~NoCache() {
      Flush();
    }

    // Read()
    size_t NoCache::Read( size_t pageNo, size_t length ) {
      // Hier wird einfach direkt aus der Datei gelesen.
      return ReadFile( pageNo, length );
    }
    // Write()
    void NoCache::Write( size_t pageNo, size_t arr, size_t length ) {
      // Hier wird einfach direkt in die Datei geschrieben.
      WriteFile( pageNo, arr, length );
    }

    // Flush()
    void NoCache::Flush() {
      // Kein Cache, kein flush..
    }
    // GetHitRate()
    double NoCache::GetHitRate() {
      // Ohne Cache gibt es auch keine HitRate
      return 0.0;
    }

  } // namespace cache
} // namespace fialgebra



















