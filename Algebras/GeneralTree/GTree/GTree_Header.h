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

1.1 Headerfile "GTree[_]Header.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_HEADER_H__
#define __GTREE_HEADER_H__

#include "SecondoSMI.h" // for SmiRecordId

namespace gtree
{

/********************************************************************
Struct ~Header~

Default tree header class (provides some often needed members).

********************************************************************/
struct Header
{
    Header()
        : root(0), height(0), entryCount(0),
          internalCount(0), leafCount(0)
    {}

    ~Header()
    {}

    SmiRecordId root;       // page of the root node
    unsigned height;        // height of the tree
    unsigned entryCount;    // count of the entries in the tree
    unsigned internalCount; // count of Internal nodes in the tree
    unsigned leafCount;     // count of leaf nodes in the tree
};

} // namespace gtree
#endif // #define __GTREE_HEADER_H__


