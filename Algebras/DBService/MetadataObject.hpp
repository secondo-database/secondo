/*
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
#ifndef ALGEBRAS_DBSERVICE_METADATAOBJECT_HPP_
#define ALGEBRAS_DBSERVICE_METADATAOBJECT_HPP_

namespace DBService {

class MetadataObject {
public:
    MetadataObject(){};
    static std::string getIdentifier(const std::string& prefix,
                                     const std::string& suffix);
    static void parseIdentifier(
            const std::string& identifier,
            std::string& prefix,
            std::string& suffix);
/*

1.1.1 \textit{separator}

Is the string used for composing the identifier.

*/
protected:
    static std::string separator;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_METADATAOBJECT_HPP_ */
