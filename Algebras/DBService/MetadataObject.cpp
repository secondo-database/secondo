/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
Faculty of Mathematics and Computer Science,
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

*/
#include <string>

#include "Algebras/DBService/MetadataObject.hpp"

using namespace std;

namespace DBService {

string MetadataObject::getIdentifier(const string& prefix,
                                     const string& suffix)
{
    return prefix + separator + suffix;
}

void MetadataObject::parseIdentifier(
        const string& identifier,
        string& prefix,
        string& suffix)
{
    size_t prefixEndPos = identifier.find(separator, 0);
    prefix = identifier.substr(0, prefixEndPos);

    size_t suffixStartPos = prefixEndPos+separator.length();
    suffix = identifier.substr(suffixStartPos, identifier.length());
}

string MetadataObject::separator("xDBSx");

} /* namespace DBService */
