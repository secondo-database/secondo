/*
Cache, der gar kein Cache ist.
Lese- und Schreibzugriffe werden direkt auf dem
Dateisystem durchgefuert.

*/

#ifndef NOCACHE_H
#define NOCACHE_H

#include <cstddef>       // size_t

#include "CacheBase.h"

namespace fialgebra {
  namespace cache {

    class NoCache : public CacheBase
    {
    public:
      // ctor
      NoCache( const char* fileName, size_t pageSize, size_t pageCount );
      ~NoCache();
      
      // CacheBase::Read()
      size_t Read( size_t pageNo, size_t length );
      // CacheBase::Write()
      void Write( size_t pageNo, size_t arr, size_t length );
      // CacheBase::Flush()
      void Flush();
      // CacheBase::GetHitRate()
      double GetHitRate();

    }; // class NoCache

  } // namespace cache
} // namespace fialgebra

#endif // NOCACHE_H

















