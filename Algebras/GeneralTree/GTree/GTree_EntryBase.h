/*
\newpage

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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1.1 Headerfile "GTree[_]EntryBase.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_ENTRY_BASE_H__
#define __GTREE_ENTRY_BASE_H__

#include "GTree_Config.h"

namespace gtree
{

/*******************************************************************
Class ~EntryBase~

This class must be the base of all node entries in this framework.

If "[_][_]GTREE[_]DEBUG"[4] is defined, the class will compile with a virtual destructor, which is used for further type checkings in some methods by using "dynamic[_]cast"[4] instead of "static[_]cast"[4] to avoid using incompatible entry and node types, e.g. inserting leaf entries into internal nodes.

*******************************************************************/
class EntryBase
    : public ObjCounter<generalTreeNodes>
{

public:

/*
Default constructor.

*/
    inline EntryBase()
    {}

/*
Default copy constructor.

*/
    inline EntryBase(const EntryBase &source)
    {}

/*
Destructor

*/
#ifdef __GTREE_DEBUG
    inline virtual ~EntryBase()
    {}

#else
    inline ~EntryBase()
    {}

#endif
}; // class EntryBase

} // namespace gtree
#endif // #define __GTREE_ENTRY_BASE_H__
