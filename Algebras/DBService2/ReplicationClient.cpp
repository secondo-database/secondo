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

#include "Algebras/DBService2/CommunicationClient.hpp"
#include "Algebras/DBService2/CommunicationProtocol.hpp"
#include "Algebras/DBService2/CommunicationUtils.hpp"
#include "Algebras/DBService2/ReplicationClient.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"

#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

using namespace std;
using namespace distributed2;

namespace DBService {


/*

Hypothesis

Use Case 1
The ReplicationClient is used when transferring files from the
O-M to the DBS-W.
In this scenario the O-M runs the ReplicationServer and the DBS-W runs
the ReplicationClient.

Use Case 2
However, there is another use case in which the DBS-W sends a replica to
lets say a O-W. In this scenario the DBS-W runs the ReplicationServer
and the O-W runs the ReplicationClient.

*/
ReplicationClient::ReplicationClient(
        string& server,
        int port,
        const fs::path& localPath, // local to the replication client
        const std::string& remoteFilename,
        string& databaseName,
        string& relationName)
: FileTransferClient(server,
                     port,
                     true,
                     *(const_cast<string*>(&localPath.string())), // local
                     *(const_cast<string*>(&remoteFilename))), // remote
  localPath(localPath.string()),
  remoteFilename(remoteFilename),
  databaseName(databaseName),
  relationName(relationName)
{
    string context("ReplicationClient");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context, port, std::cout));

    traceWriter->writeFunction("ReplicationClient::ReplicationClient");
    traceWriter->write("server", server);
    traceWriter->write("port", port);
    traceWriter->write("localPath", localPath.string());
    traceWriter->write("remoteFilename", remoteFilename);
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
        sendBuffer.push(remoteFilename);
        traceWriter->write("remoteFilename", remoteFilename);
        traceWriter->write("localPath", localPath.string());
        CommunicationUtils::sendBatch(io, sendBuffer);

        if(receiveFileFromServer())
        {
            traceWriter->write("file received, create relation");
            ListExpr command = nl->TwoElemList(
                nl->SymbolAtom("consume"),
                nl->TwoElemList(
                    nl->SymbolAtom("ffeed5"),
                    nl->TextAtom(localPath.string()))); //  // remoteFilename


            //TODO Does this read the entire memory before writing it?
            //  If so, wouldn't it be more efficient to stream read and 
            //  stream write? Is this possible?
            Word result((void*)0);
            string typeString,errorString;
            bool correct,evaluable,defined,isFunction;
            SecondoSystem::BeginTransaction();

            // Generate a stream of tuples from ffeed5 and store into "result"
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
            string relName = ReplicationUtils::getRelName(
                localPath.filename().string());

            // Create the relation by writing the "result"
            bool ok = ctlg->InsertObject(relName,"",typeExpr,result,true);
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
                                 fs::path& fileName,
                                 const std::vector<std::string>& otherObjects)
{
    traceWriter->writeFunction("ReplicationClient::requestReplica");
    traceWriter->write("remoteFilename is", remoteFilename);


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
        sendBuffer.push(remoteFilename);
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
            CommunicationUtils::sendLine(io, remoteFilename);
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
            remoteFilename = remoteName = newFileName;
        }

        if(receiveFileFromServer())
        {
            fileName = localPath;
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

        //TODO Resolve error codes and explain them.
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
