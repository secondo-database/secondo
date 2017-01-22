/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_DBSERVICE_RELATIONINFO_HPP_
#define ALGEBRAS_DBSERVICE_RELATIONINFO_HPP_

#include <string>
#include <vector>

namespace DBService
{

typedef size_t ConnectionID;

class RelationInfo
{
public:
    RelationInfo(const std::string& name);
    const std::string& getRelationName() const;
    void addNode(ConnectionID id);
    void addNodes(std::vector<ConnectionID>& nodes);
    const std::vector<ConnectionID>::const_iterator nodesBegin() const;
    const std::vector<ConnectionID>::const_iterator nodesEnd() const;

private:
    std::string relationName;
    std::vector<ConnectionID> nodes;

};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_RELATIONINFO_HPP_ */
