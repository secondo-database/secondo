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
#include <algorithm>
#include <cstring>
#include <sstream>

#include "Algebras/DBService/ReplicationUtils.hpp"

using namespace std;

namespace DBService {

const std::string ReplicationUtils::getFileName(
        const std::string& databaseName,
        const std::string& relationName)
{
    stringstream fileName;
    fileName << databaseName << separator << relationName << ".bin";
    return fileName.str();
}

const std::string ReplicationUtils::getFileNameOnDBServiceWorker(
        const std::string& databaseName,
        const std::string& relationName)
{
    stringstream fileName;
    fileName << databaseName
             << separator
             << relationName
             << "xRPLCTD" << ".bin";
    return fileName.str();
}

void ReplicationUtils::parseFileName(const std::string& fileName,
        std::string& databaseName,
        std::string& relationName)
{
    size_t dbNameEndPos = fileName.find(separator, 0);
    databaseName = fileName.substr(0, dbNameEndPos);

    size_t relNameStartPos = dbNameEndPos+separator.length();
    size_t relNameLength = fileName.length() - strlen(".bin") - relNameStartPos;

    relationName = fileName.substr(relNameStartPos, relNameLength);
}

void ReplicationUtils::removeQuotes(std::string& relationName)
{
    relationName.erase(
            remove(relationName.begin(), relationName.end(), '\"'),
            relationName.end());
}

string ReplicationUtils::separator("xDBSx");

} /* namespace DBService */
