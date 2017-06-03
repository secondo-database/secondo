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
#ifndef ALGEBRAS_DBSERVICE_REPLICATIONCLIENT_HPP_
#define ALGEBRAS_DBSERVICE_REPLICATIONCLIENT_HPP_

#include "Algebras/Distributed2/FileTransferClient.h"

#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/TraceWriter.hpp"

namespace DBService {

class ReplicationClient: public distributed2::FileTransferClient
{
public:
    ReplicationClient(
            std::string& server,
            int port,
            const std::string& fileNameDBS,
            const std::string& fileNameOrigin,
            std::string& databaseName,
            std::string& relationName);
    ~ReplicationClient();
    int start();
    int receiveReplica();
    int requestReplica(
            const std::string& functionAsNestedListString,
            std::string& fileName);
private:
    void receiveFileFromServer();
    const std::string fileNameDBS;
    const std::string fileNameOrigin;
    std::string databaseName;
    std::string relationName;
    std::unique_ptr<TraceWriter> traceWriter;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_REPLICATIONCLIENT_HPP_ */
