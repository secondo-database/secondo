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

using namespace std;

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

/* --- bdbHashFile.cpp --- */

