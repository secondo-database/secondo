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


//[$][\$]
//[_][\_]

*/
#include "Algebras/DBService/RelationInfo.hpp"

using namespace std;

namespace DBService
{

RelationInfo::RelationInfo(const string& dbName,
                           const string& relName,
                           const string& host,
                           const string& port,
                           const string& disk) :
        databaseName(dbName), relationName(relName),
        originalLocation(host, port, "", disk, "", "")
{}

const string& RelationInfo::getDatabaseName() const
{
    return databaseName;
}

const string& RelationInfo::getRelationName() const
{
    return relationName;
}

void RelationInfo::addNode(ConnectionID id)
{
    nodes.push_back(id);
}

void RelationInfo::addNodes(std::vector<ConnectionID>& nodesToAdd)
{
    for (std::vector<ConnectionID>::const_iterator i = nodesToAdd.begin();
            i != nodesToAdd.end(); ++i)
    {
        nodes.push_back(*i);
    }
}

const vector<ConnectionID>::const_iterator RelationInfo::nodesBegin() const
{
    return nodes.begin();
}

const vector<ConnectionID>::const_iterator RelationInfo::nodesEnd() const
{
    return nodes.end();
}

const size_t RelationInfo::getNodeCount()
{
    return nodes.size();
}

const string RelationInfo::toString() const
{
    return RelationInfo::getIdentifier(databaseName, relationName);
}

string RelationInfo::getIdentifier(const string dbName,
                                 const std::string relName)
{
    return dbName + "xDBSx" + relName;
}

const LocationInfo& RelationInfo::getOriginalLocation() const
{
return originalLocation;
}

} /* namespace DBService */
