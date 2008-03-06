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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Headerfile "DistDataId.h"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "DistDataId"[4] struct, which could be used retrieve the respective "DistDataInfo"[4] object from the "DistDataReg"[4]. It stores the algebra- and type-id of the assigned type constructor together with the unique id of the distdata type.

*/
#ifndef __DISTDATA_ID_H
#define __DISTDATA_ID_H

namespace generalTree {

/*
1.1 Struct "DistDataId"[4]

*/
struct DistDataId
{
/*
Default constructor (should not be used - needed, since this struct is stored within the "DistDataAttribute"[4] class, which needs this constructor).

*/
    DistDataId()
    {}

/*
Constructor (createas an undefined id)

*/
    DistDataId(bool _defined) :
            defined(_defined)
    {}

/*
Constructor.

*/
    DistDataId(int _algebraId, int _typeId, int _distdataId) :
            defined(true),
            algebraId(_algebraId),
            typeId(_typeId),
            distdataId(_distdataId)
    {}

    bool inline operator == (const DistDataId& rhs)
    {
        return
            (algebraId == rhs.algebraId) &&
            (typeId == rhs.typeId) &&
            (distdataId == rhs.distdataId);
    }

    bool inline operator != (const DistDataId& rhs)
    { return !operator == (rhs); }

    bool defined;
    int algebraId; // algebra-id of the assigned type constructor
    int typeId;    // type-id of the assigned type constructor
    int distdataId; // id of the distdata object
};

} // namespace generalTree
#endif // ifndef __DISTDATA_ID_H
