/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and 
Computer Science, Database Systems for New Applications.

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

//[_] [\_]

1 Implementation of a cached SmiRecordFile 

October 2009, M. Spiekermann. This implementation
offers the possibility to define a cachesize of the underlying
persistent data structures. If the caching mechanisms of the underlying
storage manager (e.g. berkeley db) are not efficient enough other caches
and displacement strategies may be implemented here.

*/


#include <db_cxx.h>
//#include <exception>
#include <stdexcept>


#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"

using namespace std;

/* --- Implementation of class CachedFile --- */


SmiCachedFile::SmiCachedFile( const bool temp, 
			      const uint64_t cs /*=0*/)
  : SmiFile( temp ), cacheSize(cs)
{
}


SmiCachedFile::SmiCachedFile(const SmiCachedFile& f)
  : SmiFile( f ), cacheSize(f.cacheSize)
{
  // the Berkeley-DB cache size cannot be set here.
  // All opened files within a berkeley-db environment
  // share a global cache.

}


SmiCachedFile::~SmiCachedFile()
{
}


uint64_t
SmiCachedFile::GetCacheSize() const
{
  return cacheSize;
  /*	
  int nCaches = 0;
  u_int32_t gb;
  u_int32_t bytes;  
  int rc = impl->bdbFile->get_cachesize( &gb, &bytes, &nCaches ); 
  SmiEnvironment::SetError(rc);
  if (rc != 0)
    throw runtime_error("Error: could not get cache size!");

  uint64_t sz = gb * (1024 * 1024 * 1024); 
  return sz + bytes;
  */
}

