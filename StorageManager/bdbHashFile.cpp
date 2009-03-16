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

1 Implementation of SmiHashFile using the Berkeley-DB

April 2002 Ulrich Telle

September 2002 Ulrich Telle, fixed flag (DB\_DIRTY\_READ) in Berkeley DB calls for system catalog files

April 2003 Ulrich Telle, implemented temporary SmiFiles

May 2008, Victor Almeida created the two sons of the ~SmiKeyedFile~ class, namely ~SmiBtreeFile~ and
~SmiHashFile~, for B-Tree and hash access methods, respectively.

*/

using namespace std;

//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <cassert>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"

extern string lu_2_s(uint32_t value); // defined in bdbFile.cpp

/* --- Implementation of class SmiHashFile --- */

SmiHashFile::SmiHashFile( const SmiKey::KeyDataType keyType,
                            const bool hasUniqueKeys /* = true */,
                            const bool isTemporary /* = false */ )
  : SmiKeyedFile( KeyedHash, keyType, hasUniqueKeys, isTemporary )
{
}

SmiHashFile::~SmiHashFile()
{
}

SmiStatResultType
  SmiHashFile::GetFileStatistics(const SMI_STATS_MODE mode)
{ int getStatReturnValue = 0;
  u_int32_t flags = 0;
  DB_HASH_STAT *sRS = 0;
  SmiStatResultType result;
  // set flags according to ~mode~
  switch(mode){
    case SMI_STATS_LAZY: {
        flags = DB_FAST_STAT;
        break;
      }
    case SMI_STATS_EAGER: {
        flags = 0;
        break;
      }
    default: {
        cout << "Error in SmiHashFile::GetFileStatistics: Unknown "
             << "SMI_STATS_MODE" << mode << endl;
//         assert( false );
      }
  }
  // call bdb stats method
#if DB_VERSION_MAJOR >= 4
#if DB_VERSION_MINOR >= 3
  getStatReturnValue = impl->bdbFile->stat(0, &sRS, flags);
#endif
#else
  getStatReturnValue = impl->bdbFile->stat( &sRS, flags);
#endif
  // check for errors
  if(getStatReturnValue != 0){
    cout << "Error in SmiHashFile::GetFileStatistics: stat(...) returned != 0"
         << getStatReturnValue << endl;
    string error;
    SmiEnvironment::GetLastErrorCode( error );
    cout << error << endl;
//  assert( false );
    return result;
  }
  // translate result structure to vector<pair<string,string> >
  result.push_back(pair<string,string>("FileName",fileName));
  result.push_back(pair<string,string>("FileType","HashFile"));
  result.push_back(pair<string,string>("StatisticsMode",
      (mode == SMI_STATS_LAZY) ? "Lazy" : "Eager" ));
  result.push_back(pair<string,string>("FileTypeVersion",
      lu_2_s(sRS->hash_version)));
  result.push_back(pair<string,string>("NoUniqueKeys",
      lu_2_s(sRS->hash_nkeys)));
  result.push_back(pair<string,string>("NoEntries",
      lu_2_s(sRS->hash_ndata)));
  result.push_back(pair<string,string>("PageSize",
      lu_2_s(sRS->hash_pagesize)));
  result.push_back(pair<string,string>("NoDesiredItemsPerBucket",
      lu_2_s(sRS->hash_ffactor)));
  result.push_back(pair<string,string>("NoBuckets",
      lu_2_s(sRS->hash_buckets)));
  result.push_back(pair<string,string>("NoFreeListPages",
      lu_2_s(sRS->hash_free)));
  result.push_back(pair<string,string>("NoBigItemPages",
      lu_2_s(sRS->hash_bigpages)));
  result.push_back(pair<string,string>("NoOverflowPages",
      lu_2_s(sRS->hash_overflows)));
  result.push_back(pair<string,string>("NoDuplicatePages",
      lu_2_s(sRS->hash_dup)));
  result.push_back(pair<string,string>("NoBytesFreeBucketPages",
      lu_2_s(sRS->hash_bfree)));
  result.push_back(pair<string,string>("NoBytesFreeBigItemPages",
      lu_2_s(sRS->hash_big_bfree)));
  result.push_back(pair<string,string>("NoBytesFreeOverflowPages",
      lu_2_s(sRS->hash_ovfl_free)));
  result.push_back(pair<string,string>("NoBytesFreeDuplicatePages",
      lu_2_s(sRS->hash_dup_free)));

  free(sRS); // free result structure
  return result;
}

/* --- bdbHashFile.cpp --- */

