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
#include <chrono>

#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/ReplicationClient.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"

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
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context, port));

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
    return 0;
}

int ReplicationClient::receiveReplica()
{
    traceWriter->writeFunction("ReplicationClient::receiveReplica");
    try
    {
        if(start() != 0)
        {
            traceWriter->write("Could not connect to Server");
            return false;
        }

        iostream& io = socket->GetSocketStream();
        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationServer()))
        {
            traceWriter->write("not connected to ReplicationServer");
            return 1;
        }
        queue<string> sendBuffer;
        sendBuffer.push(CommunicationProtocol::ReplicationClient());
        sendBuffer.push(CommunicationProtocol::SendReplicaForStorage());
        sendBuffer.push(fileNameOrigin);
        traceWriter->write("fileNameOrigin", fileNameOrigin);
        traceWriter->write("fileNameDBS", fileNameDBS);
        CommunicationUtils::sendBatch(io, sendBuffer);

        if(receiveFileFromServer())
        {
            traceWriter->write("file received, create relation");
            ListExpr command = nl->TwoElemList(
                                 nl->SymbolAtom("consume"),
                                 nl->TwoElemList(
                                   nl->SymbolAtom("ffeed5"),
                                   nl->TextAtom(fileNameDBS)));

            Word result((void*)0);
            string typeString,errorString;
            bool correct,evaluable,defined,isFunction;
            SecondoSystem::BeginTransaction();

            try{
                QueryProcessor::ExecuteQuery(command, result, typeString,
                                  errorString,correct,evaluable,defined,
                                  isFunction);
            } catch(...){
                correct = false;
            }
            if(!correct){
               traceWriter->write("Error in creating relation from file");
               SecondoSystem::AbortTransaction(true);
               return 1;
            }
            ListExpr typeExpr;
            nl->ReadFromString(typeString,typeExpr);
            SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
            string relName = ReplicationUtils::getRelName(fileNameDBS);

            bool ok = ctlg->InsertObject (relName,"",typeExpr,result,true);
            if(!ok){
              traceWriter->write("Insertion relation " + relName + " failed");
              SecondoSystem::AbortTransaction(true);
              return 1;
            }
            ok = ctlg->CleanUp(false,true);
            if(!ok){
               traceWriter->write("catalog->CleanUp failed");
               SecondoSystem::AbortTransaction(true);
            } else {
               traceWriter->write("Inserting relation " + relName 
                                  + " successful");
               SecondoSystem::CommitTransaction(true);
            }
            traceWriter->write(
                    "Replication successful, notifying DBService master");
            if(ok){
               reportSuccessfulReplication();
            }
        }
    } catch (...)
    {
        cerr << "ReplicationClient: communication error" << endl;
        return 5;
    }
    return 0;
}

int ReplicationClient::requestReplica(const string& functionAsNestedListString,
                                 std::string& fileName,
                                 const std::vector<std::string>& otherObjects)
{
    traceWriter->writeFunction("ReplicationClient::requestReplica");
    try
    {
        if(start() != 0)
        {
            traceWriter->write("Could not connect to Server");
            return 1;
        }

        iostream& io = socket->GetSocketStream();
        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationServer()))
        {
            traceWriter->write("not connected to ReplicationServer");
            return 2;
        }
        queue<string> sendBuffer;
        sendBuffer.push(CommunicationProtocol::ReplicationClient());
        sendBuffer.push(CommunicationProtocol::SendReplicaForUsage());
        sendBuffer.push(fileNameOrigin);
        CommunicationUtils::sendBatch(io, sendBuffer);
        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::FunctionRequest()))
        {
            traceWriter->write("expected FunctionRequest");
            return 3;
        }
        if(functionAsNestedListString.empty())
        {
            CommunicationUtils::sendLine(io, CommunicationProtocol::None());
        }else
        {
            CommunicationUtils::sendLine(io, functionAsNestedListString);

            queue<string> sendBuffer2;
            sendBuffer2.push(stringutils::int2str(otherObjects.size()));
            for(auto & o : otherObjects)
            {
               sendBuffer2.push(o);
            }
            CommunicationUtils::sendBatch(io,sendBuffer2);

            if(!CommunicationUtils::receivedExpectedLine(io,
                    CommunicationProtocol::FileName()))
            {
                traceWriter->write("expected file name keyword");
                return 4;
            }
            CommunicationUtils::sendLine(io, fileNameOrigin);
            traceWriter->write("sent original filename");

            if(!CommunicationUtils::receivedExpectedLine(io,
                    CommunicationProtocol::FileName()))
            {
                traceWriter->write("expected file name keyword");
                return 5;
            }

            string newFileName;
            CommunicationUtils::receiveLine(io, newFileName);
            traceWriter->write("new filename is", newFileName);
            fileNameOrigin = remoteName = newFileName;
        }

        if(receiveFileFromServer())
        {
            fileName = fileNameDBS;
        }
    } catch (...)
    {
        cerr << "ReplicationClient: communication error" << endl;
        return 4;
    }
    return 0;
}

bool ReplicationClient::receiveFileFromServer()
{
    traceWriter->writeFunction("ReplicationClient::receiveFileFromServer");
    traceWriter->write("requesting file", remoteName);

    std::chrono::steady_clock::time_point begin =
            std::chrono::steady_clock::now();
    int rc = receiveFile();
    std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
    if(rc != 0)
    {
        traceWriter->write("receive failed");
        traceWriter->write("rc=", rc);
        return false;
    }else
    {
        traceWriter->write("received file");
        traceWriter->write("duration of receive [microseconds]",
                std::chrono::duration_cast
                <std::chrono::microseconds>(end - begin).count());
        return true;
    }
}

void ReplicationClient::reportSuccessfulReplication()
{
    traceWriter->writeFunction(
            "ReplicationClient::reportSuccessfulReplication");

    string dbServiceHost;
    string dbServicePort;
    if(!SecondoUtilsLocal::lookupDBServiceLocation(
            dbServiceHost,
            dbServicePort))
    {
        throw new SecondoException("Unable to connect to DBService");
    }

    CommunicationClient clientToDBServiceMaster(
            dbServiceHost,
            atoi(dbServicePort.c_str()),
            0);
    bool reported = clientToDBServiceMaster.reportSuccessfulReplication(
            databaseName,
            relationName);
    if(!reported)
    {
        traceWriter->write("Unable to report successful replication");
    }
}

} /* namespace DBService */
