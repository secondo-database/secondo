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
#include "Algebras/DBService/ReplicationClient.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;
using namespace distributed2;

namespace DBService {

ReplicationClient::ReplicationClient(
        string& server,
        int port,
        const std::string& fileNameDBS,
        const std::string& fileNameOrigin,
        string& databaseName,
        string& relationName)
: FileTransferClient(server,
                     port,
                     true,
                     *(const_cast<string*>(&fileNameDBS)),
                     *(const_cast<string*>(&fileNameOrigin))),
  fileNameDBS(fileNameDBS),
  fileNameOrigin(fileNameOrigin),
  databaseName(databaseName),
  relationName(relationName)
{
    string context("ReplicationClient");
    traceWriter= auto_ptr<TraceWriter>
    (new TraceWriter(context));

    traceWriter->writeFunction("ReplicationClient::ReplicationClient");
    traceWriter->write("server", server);
    traceWriter->write("port", port);
    traceWriter->write("fileNameDBS", fileNameDBS);
    traceWriter->write("fileNameOrigin", fileNameOrigin);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);
}

ReplicationClient::~ReplicationClient()
{
    traceWriter->writeFunction("ReplicationClient::~ReplicationClient");
}

int ReplicationClient::start()
{
    traceWriter->writeFunction("ReplicationClient::start");
    socket = Socket::Connect(server, stringutils::int2str(port),
            Socket::SockGlobalDomain, 3, 1);
    if (!socket)
    {
        traceWriter->write("socket initialization failed");
        return 1;
    }
    if (!socket->IsOk())
    {
        traceWriter->write("socket not ok");
        return 2;
    }

    try
    {
        iostream& io = socket->GetSocketStream();
        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationServer()))
        {
            traceWriter->write("not connected to ReplicationServer");
            return 1;
        }
        queue<string> sendBuffer;
        sendBuffer.push(CommunicationProtocol::ReplicationClient());
        sendBuffer.push(fileNameOrigin);
        CommunicationUtils::sendBatch(io, sendBuffer);

        if(receiveFile() != 0)
        {
            traceWriter->write("receive failed");
        }
        traceWriter->write("received file");

        // TODO currently not necessary
        SecondoUtilsLocal::adjustDatabase(databaseName);

        stringstream query;
        query << "let "
              << relationName
              << "_DBS"
                " = \""
              << fileNameDBS
              << "\""
              << " getObjectFromFile consume";
        traceWriter->write(query.str());

        string errorMessage;
        bool resultOk =
                SecondoUtilsLocal::createRelation(query.str(), errorMessage);
        if(!resultOk)
        {
            traceWriter->write(errorMessage);
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
