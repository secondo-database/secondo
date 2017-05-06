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
#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/ReplicationClient.hpp"
#include "Algebras/DBService/SecondoUtils.hpp"

using namespace std;
using namespace distributed2;

namespace DBService {

ReplicationClient::ReplicationClient(
        string& server,
        int port,
        string& fileName,
        string& databaseName,
        string& relationName)
: FileTransferClient(server, port, true, fileName, fileName),
  fileName(fileName),
  databaseName(databaseName),
  relationName(relationName)
{}

int ReplicationClient::start()
{
    socket = Socket::Connect(server, stringutils::int2str(port),
            Socket::SockGlobalDomain, 3, 1);
    if (!socket) {
        return 1;
    }
    if (!socket->IsOk()) {
        return 2;
    }
    receiveFile(); // TODO error handling
    stringstream query;

    SecondoUtils::adjustDatabaseOnCurrentNode(databaseName);

    query << "let "
          << relationName
          << "_DBS"
            " = \""
          << fileName
          << "\""
          << " getObjectFromFile consume";
    SecondoUtils::executeQueryOnCurrentNode(query.str());
    return 0;
}

} /* namespace DBService */
