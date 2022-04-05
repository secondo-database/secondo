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
#include "Algebras/DBService2/LockKeeper.hpp"

#include <loguru.hpp>

#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

using namespace std;
using namespace distributed2;

extern boost::recursive_mutex nlparsemtx;

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
    LOG_SCOPE_FUNCTION(INFO);
    
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
    LOG_SCOPE_FUNCTION(INFO);
}

int ReplicationClient::start()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("ReplicationClient::start");
    socket = Socket::Connect(server, stringutils::int2str(port),
            Socket::SockGlobalDomain, 3, 1);

    if (!socket)
    {
        traceWriter->write("socket initialization failed");
        LOG_F(ERROR, "%s", "Socket initialization failed.");

        return 1;
    }
    if (!socket->IsOk())
    {
        LOG_F(ERROR, "%s", "Socket not ok.");
        traceWriter->write("socket not ok");
        return 2;
    }
    return 0;
}

int ReplicationClient::receiveReplica()
{
    traceWriter->writeFunction("ReplicationClient::receiveReplica");
    LOG_SCOPE_FUNCTION(INFO);

    // try
    // {
        if(start() != 0)
        {
            traceWriter->write("Could not connect to Server");
            LOG_F(ERROR, "%s", "Could not connect to the ReplicationServer.");

            return false;
        }

        iostream& io = socket->GetSocketStream();
        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationServer()))
        {
            traceWriter->write("Not connected to ReplicationServer.");
            LOG_F(ERROR, "%s", "Not connected to ReplicationServer.");

            return 1;
        }
        queue<string> sendBuffer;
        sendBuffer.push(CommunicationProtocol::ReplicationClient());
        sendBuffer.push(CommunicationProtocol::SendReplicaForStorage());
        sendBuffer.push(remoteFilename);

        traceWriter->write("remoteFilename", remoteFilename);
        traceWriter->write("localPath", localPath.string());

        LOG_F(INFO, "remoteFilename: %s", remoteFilename.c_str());
        LOG_F(INFO, "localPath: %s", localPath.string().c_str());

        CommunicationUtils::sendBatch(io, sendBuffer);


        // Lock access to the nested list.
        
        if(receiveFileFromServer())
        {
            traceWriter->write("File received. Now creating relation...");
            LOG_F(INFO, "%s", "File received. Now creating relation...");

            LOG_F(INFO, "%s", "Acquiring the nlparsemtx...");
            boost::unique_lock<boost::recursive_mutex> nlLock(nlparsemtx);
            LOG_F(INFO, "%s", "Successfully acquired the nlparsemtx.");
            
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

            //LOG_F(INFO, "%s", "Acquiring the QueryProcessorMutex...");
            
            // // Establishing a timeout for locks
            // std::shared_ptr<boost::timed_mutex> qpMutex =
            //     LockKeeper::getInstance()->getQueryProcessorMutex();

            // boost::unique_lock<boost::timed_mutex>
            //     lock{ *qpMutex, boost::try_to_lock };

        // if(lock.owns_lock() || 
        //     lock.try_lock_for(boost::chrono::seconds{ 360 })) {
        //     LOG_F(INFO, "%s", "Successfully acquired QueryProcessorMutex.");
        // }
        // else {
        //     LOG_F(ERROR, "%s", "Acquisition of QueryProcessorMutex "
        //         "failed due to timeout.");
        //     return 1;
        // }
        
        // boost::lock_guard<boost::recursive_mutex> queryProcessorGuard(
        //     // Dereference the shared_ptr to the mutex
        //     *LockKeeper::getInstance()->getQueryProcessorMutex()
        // );
        // LOG_F(INFO, "%s", "Successfully acquired the QueryProcessorMutex.");

            // Generate a stream of tuples from ffeed5 and store into "result"
            try{
                std::chrono::steady_clock::time_point begin =
                    std::chrono::steady_clock::now();
                
                QueryProcessor::ExecuteQuery(command, result, typeString,
                                  errorString,correct,evaluable,defined,
                                  isFunction);
                
                std::chrono::steady_clock::time_point end =
                    std::chrono::steady_clock::now();
                
                LOG_F(INFO, "feed5 successful. Duration: %ld ms", 
                    std::chrono::duration_cast
                    <std::chrono::milliseconds>(end - begin).count());
            } catch(...){
                LOG_F(ERROR, "%s", "Exception during feed5.");
                correct = false;
            }

            if(!correct){
                traceWriter->write("Error in creating relation from file.");
                LOG_F(ERROR, "%s", "Error in creating relation from file.");

                SecondoSystem::AbortTransaction(true);
                return 1;
            } else {
                LOG_F(INFO, "%s", 
                    "Successfully created the relation from file.");
            }

            ListExpr typeExpr;
            
            //nlLock.lock();
            nl->ReadFromString(typeString,typeExpr);
            //nlLock.unlock();

            SecondoCatalog* ctlg = SecondoSystem::GetCatalog();

            string relName = ReplicationUtils::getRelName(
                localPath.filename().string());

            LOG_F(INFO, "%s", "Inserting the relation...");

            std::chrono::steady_clock::time_point begin =
                std::chrono::steady_clock::now();

            // Create the relation by writing the "result"
            bool ok = ctlg->InsertObject(relName,"",typeExpr,result,true);
            
            if(!ok){
              traceWriter->write("Insertion relation " + relName + " failed");
              LOG_F(ERROR, "Insertion of relation %s failed", 
                relName.c_str());            

              SecondoSystem::AbortTransaction(true);
              return 1;
            }

            ok = ctlg->CleanUp(true);

            if(!ok){
                traceWriter->write("catalog->CleanUp failed");
                LOG_F(ERROR, "%s", "catalog->CleanUp failed.");

                SecondoSystem::AbortTransaction(true);
                return 1;
            } else {
                traceWriter->write("Inserting relation " + relName 
                                  + " successful");

                LOG_F(INFO, "Insertion of the relation %s was successful.", 
                    relName.c_str());
                SecondoSystem::CommitTransaction(true);
            }

            std::chrono::steady_clock::time_point end =
                std::chrono::steady_clock::now();

            LOG_F(INFO, "Inserted relation. Duration: %ld ms", 
                std::chrono::duration_cast
                <std::chrono::milliseconds>(end - begin).count());

            traceWriter->write(
                    "Replication successful, notifying DBService master");
            LOG_F(INFO, "%s", "Replication successful, notifying "
                "the DBService master...");

            nlLock.unlock();

            if(ok){
               reportSuccessfulReplication();
            }
        }
    // } catch (...)
    // {
    //     cerr << "ReplicationClient: communication error" << endl;
    //     LOG_F(ERROR, "%s", "ReplicationClient: communication error.");
    //     return 5;
    // }
    return 0;
}

int ReplicationClient::requestReplica(const string& functionAsNestedListString,
                                 fs::path& fileName,
                                 const std::vector<std::string>& otherObjects)
{
    traceWriter->writeFunction("ReplicationClient::requestReplica");
    LOG_SCOPE_FUNCTION(INFO);

    traceWriter->write("remoteFilename is", remoteFilename);
    LOG_F(INFO, "The remoteFilename is: %s", remoteFilename.c_str());

    try
    {
        if(start() != 0)
        {
            traceWriter->write("Could not connect to ReplicationServer.");
            LOG_F(ERROR, "%s", "Could not connect to ReplicationServer.");

            return 1;
        }

        iostream& io = socket->GetSocketStream();
        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationServer()))
        {
            traceWriter->write("not connected to ReplicationServer");
            LOG_F(ERROR, "%s", "Connected to a wrong server. "
                "This is not a ReplicationServer.");

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
            LOG_F(ERROR, "%s", "Expected FunctionRequest.");

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
                LOG_F(ERROR, "%s", "Expected file name keyword.");

                return 4;
            }
            CommunicationUtils::sendLine(io, remoteFilename);
            traceWriter->write("sent original filename");
            LOG_F(INFO, "Sent original filename: %s", remoteFilename.c_str());

            if(!CommunicationUtils::receivedExpectedLine(io,
                    CommunicationProtocol::FileName()))
            {
                traceWriter->write("expected file name keyword");
                LOG_F(ERROR, "%s", "Expected file name keyword.");

                return 5;
            }

            string newFileName;
            CommunicationUtils::receiveLine(io, newFileName);
            traceWriter->write("new filename is", newFileName);
            LOG_F(INFO, "The new filename is: %s", newFileName.c_str());

            remoteFilename = remoteName = newFileName;
        }

        if(receiveFileFromServer())
        {
            fileName = localPath;
        }
    } catch (...)
    {
        cerr << "ReplicationClient: communication error" << endl;
        LOG_F(ERROR, "%s", "ReplicationClient: communication error.");

        return 4;
    }
    return 0;
}

bool ReplicationClient::receiveFileFromServer()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("ReplicationClient::receiveFileFromServer");
    traceWriter->write("requesting file", remoteName);
    LOG_F(INFO, "Requesting file: %s", remoteName.c_str());

    std::chrono::steady_clock::time_point begin =
            std::chrono::steady_clock::now();
    int rc = receiveFile();
    std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();

    if(rc != 0) {
        traceWriter->write("receive failed");
        traceWriter->write("rc=", rc);

        LOG_F(ERROR, "%s", "Failed to receive the replica file. "
            "The return code was: %d", rc);

        //TODO Resolve error codes and explain them.
        return false;

    } else {
        traceWriter->write("received file");
        traceWriter->write("duration of receive [microseconds]",
                std::chrono::duration_cast
                <std::chrono::microseconds>(end - begin).count());

        LOG_F(INFO, "Received the file successfully. "
            "The file transfer duration: %ld ms", std::chrono::duration_cast
            <std::chrono::milliseconds>(end - begin).count());

        return true;
    }
}

void ReplicationClient::reportSuccessfulReplication()
{
    traceWriter->writeFunction(
            "ReplicationClient::reportSuccessfulReplication");
    
    LOG_SCOPE_FUNCTION(INFO);

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
