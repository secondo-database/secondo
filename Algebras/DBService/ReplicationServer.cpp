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
#include <iostream>

#include <boost/filesystem.hpp>

#include "SocketIO.h"

#include "Algebras/Distributed2/FileTransferKeywords.h"

#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

ReplicationServer::ReplicationServer(int port) :
 MultiClientServer(port), FileTransferServer(port)
{
    string context("ReplicationServer");
    traceWriter= auto_ptr<TraceWriter>
    (new TraceWriter(context));
    traceWriter->writeFunction("ReplicationServer::ReplicationServer");
    traceWriter->write("port", port);
}

ReplicationServer::~ReplicationServer()
{
    traceWriter->writeFunction("ReplicationServer::~ReplicationServer");
}

int ReplicationServer::start()
{
    traceWriter->writeFunction("ReplicationServer::start");
    return MultiClientServer::start();
}

int ReplicationServer::communicate(iostream& io)
{
    traceWriter->writeFunction("ReplicationServer::communicate");
    try
    {
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationServer());

        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationClient()))
        {
            traceWriter->write("not connected to ReplicationClient");
            return 1;
        }
        string fileName;
        CommunicationUtils::receiveLine(io, fileName);

        bool fileCreated = true;
        if(!boost::filesystem::exists(fileName))
        {
            traceWriter->write("file does not exist");
            fileCreated = createFile(fileName);
        }

        // expected by receiveFile function of FileTransferClient,
        // but not sent by sendFile function of FileTransferServer
        CommunicationUtils::sendLine(io,
                distributed2::FileTransferKeywords::FileTransferServer());

        if(fileCreated)
        {
            traceWriter->write("file created, sending file");
            if(sendFile(io) != 0)
            {
                traceWriter->write("send failed");
            }else
            {
                traceWriter->write("file sent");
            }
            // TODO delete file
        }else
        {
            traceWriter->write("notifying client");
            CommunicationUtils::sendLine(io,
                            distributed2::FileTransferKeywords::FileNotFound());
        }
    } catch (...)
    {
        traceWriter->write("ReplicationServer: communication error");
        return 5;
    }
    return 0;
}

bool ReplicationServer::createFile(string fileName) const
{
    traceWriter->writeFunction("ReplicationServer::createFile");

    string databaseName;
    string relationName;
    ReplicationUtils::parseFileName(fileName, databaseName, relationName);

    SecondoUtilsLocal::adjustDatabase(databaseName);

    stringstream query;
    query << "query "
          << relationName
          << " saveObjectToFile[\""
          << fileName
          << "\"]";
    traceWriter->write("query", query.str());

//    SecondoUtilsLocal::executeQuery(query.str());
    bool resultOk = SecondoUtilsLocal::excuteQueryCommand(query.str());
    if(resultOk)
    {
        traceWriter->write("file created");
    }else
    {
        traceWriter->write("could not create file");
    }
    return resultOk;
}

} /* namespace DBService */
