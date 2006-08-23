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

1 Header File CacheInfo.h 

August 2006 M. Spiekermann 

1.1 Overview

The class ~CacheInfo~ provides cache statistics which are retrieved by the 
function ~GetCacheStatistics~ of class ~SmiEnvironment~. Statistics are
provided by following function of the Berkeley-DB API 

----
    int
    DbEnv::memp_stat(DB_MPOOL_STAT **gsp,
        DB_MPOOL_FSTAT *(*fsp)[], u_int32_t flags);
----

For details refer to the Berkeley-DB documentation.
 
 

*/


#ifndef CLASS_CACHEINFO_H
#define CLASS_CACHEINFO_H

class CacheInfo {

  public:
  /*
  Fields of the Berkeley-DB struct DB_MPOOL_STAT
  
  size_t st_gbytes;
  Gigabytes of cache (total cache size is st_gbytes + st_bytes). 
  size_t st_bytes;
  Bytes of cache (total cache size is st_gbytes + st_bytes). 
  u_int32_t st_ncache;
  Number of caches. 
  u_int32_t st_regsize;
  Individual cache size. 
  u_int32_t st_map;
  Requested pages mapped into the process' address space 
  (there is no available information about whether or not this request 
  caused disk I/O, although examining the application 
  page fault rate may be helpful). 
  u_int32_t st_cache_hit;
  Requested pages found in the cache. 
  u_int32_t st_cache_miss;
  Requested pages not found in the cache. 
  u_int32_t st_page_create;
  Pages created in the cache. 
  u_int32_t st_page_in;
  Pages read into the cache. 
  u_int32_t st_page_out;
  Pages written from the cache to the backing file. 
  u_int32_t st_ro_evict;
  Clean pages forced from the cache. 
  u_int32_t st_rw_evict;
  Dirty pages forced from the cache. 
  u_int32_t st_page_trickle;
  Dirty pages written using the DbEnv::memp_trickle method. 
  u_int32_t st_pages;
  Pages in the cache. 
  u_int32_t st_page_clean;
  Clean pages currently in the cache. 
  u_int32_t st_page_dirty;
  Dirty pages currently in the cache. 
  u_int32_t st_hash_buckets;
  Number of hash buckets in buffer hash table. 
  u_int32_t st_hash_searches;
  Total number of buffer hash table lookups. 
  u_int32_t st_hash_longest;
  The longest chain ever encountered in buffer hash table lookups. 
  u_int32_t st_hash_examined;
  Total number of hash elements traversed during hash table lookups. 
  u_int32_t st_hash_nowait;
  The number of times that a thread of control was able to obtain 
  a hash bucket lock without waiting. 
  u_int32_t st_hash_wait;
  The number of times that a thread of control was forced to wait
  before obtaining a hash bucket lock. 
  u_int32_t st_hash_max_wait;
  The maximum number of times any hash bucket lock was
  waited for by a thread of control. 
  u_int32_t st_region_wait;
  The number of times that a thread of control was forced to 
  wait before obtaining a region lock. 
  u_int32_t st_region_nowait;
  The number of times that a thread of control was able to 
  obtain a region lock without waiting. 
  u_int32_t st_alloc;
  Number of page allocations. 
  u_int32_t st_alloc_buckets;
  Number of hash buckets checked during allocation. 
  u_int32_t st_alloc_max_buckets;
  Maximum number of hash buckets checked during an allocation. 
  u_int32_t st_alloc_pages;
  Number of pages checked during allocation. 
  u_int32_t st_alloc_max_pages;
  Maximum number of pages checked during an allocation. 
  */ 

  int cstatNr;
  // Sequence number of the statistics  (usead as primary key)
   
  size_t bytes;
  // Bytes of cache (total cache size is st_gbytes + st_bytes). 

  size_t regsize;
  // Individual cache size. 

  size_t cache_hit;
  // Requested pages found in the cache. 
  
  size_t cache_miss;
  // Requested pages not found in the cache. 
  
  size_t page_create;
  // Pages created in the cache. 
  
  size_t page_in;
  //Pages read into the cache. 
  
  size_t page_out;
  //Pages written from the cache to the backing file. 

  size_t pages;
  //Pages in the cache. 
 

  CacheInfo() {}
  ~CacheInfo() {}
    
}; 

class FileInfo {

  /*
  Fields of the Berkeley-DB struct DB_MPOOL_FSTAT
  
  char * file_name;
  The name of the file. 
  size_t st_pagesize;
  Page size in bytes. 
  u_int32_t st_cache_hit;
  Requested pages found in the cache. 
  u_int32_t st_cache_miss;
  Requested pages not found in the cache. 
  u_int32_t st_map;
  Requested pages mapped into the process' address space. 
  u_int32_t st_page_create;
  Pages created in the cache. 
  u_int32_t st_page_in;
  Pages read into the cache. 
  u_int32_t st_page_out;
  Pages written from the cache to the backing file. 
  */


 
  public:
   
  int fstatNr;
  // Sequence counter (used as foreigen key)
 
  string file_name;
  //The name of the file. 
  
  size_t pagesize;
  //Page size in bytes. 
  
  size_t cache_hit;
  //Requested pages found in the cache. 
  
  size_t cache_miss;
  //Requested pages not found in the cache. 
  
  size_t page_create;
  //Pages created in the cache. 
  
  size_t page_in;
  //Pages read into the cache. 
  
  size_t page_out;
  //Pages written from the cache to the backing file. 

  FileInfo() {}
  ~FileInfo() {}

  
}; 


#endif
