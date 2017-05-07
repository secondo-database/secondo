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

#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ReplicationClient.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

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
{
    printFunction("ReplicationClient::ReplicationClient");
    string context("ReplicationClient");
    traceWriter= auto_ptr<TraceWriter>
    (new TraceWriter(context));
    traceWriter->write("Initializing ReplicationClient");
    traceWriter->write("server", server);
    traceWriter->write("port", port);
    traceWriter->write("fileName", fileName);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);
}

int ReplicationClient::start()
{
    printFunction("ReplicationClient::start");
    socket = Socket::Connect(server, stringutils::int2str(port),
            Socket::SockGlobalDomain, 3, 1);
    if (!socket) {
        return 1;
    }
    if (!socket->IsOk()) {
        return 2;
    }

    try
    {
        iostream& io = socket->GetSocketStream();
        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationServer()))
        {
            print("not connected to ReplicationServer");
            traceWriter->write("not connected to ReplicationServer");
            return 1;
        }
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationClient());

        int receiveOk = receiveFile();
        if(!receiveOk)
        {
            print("receive failed");
            traceWriter->write("receive failed");
        }
        stringstream query;

        SecondoUtilsLocal::adjustDatabase(databaseName);

        query << "let "
              << relationName
              << "_DBS"
                " = \""
              << fileName
              << "\""
              << " getObjectFromFile consume";
        print(query.str());

        string errorMessage;
        bool resultOk =
                SecondoUtilsLocal::createRelation(query.str(), errorMessage);
        if(!resultOk)
        {
            print(errorMessage);
            traceWriter->write("error: ", errorMessage);
            return 3;
        }
    } catch (...)
    {
        cerr << "ReplicationClient: communication error" << endl;
        return 5;
    }
    return 0;
}

} /* namespace DBService */
