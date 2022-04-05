/*

1.1.1 Class Definition

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

#include "Algebra.h"
#include "FileSystem.h"
#include "StringUtils.h"

#include "SecondoCatalog.h"
#include "SecondoSystem.h"

#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService2/CommunicationProtocol.hpp"
#include "Algebras/DBService2/CommunicationServer.hpp"
#include "Algebras/DBService2/CommunicationUtils.hpp"
#include "Algebras/DBService2/DBServiceManager.hpp"
#include "Algebras/DBService2/ReplicationClientRunnable.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/TriggerFileTransferRunnable.hpp"
#include "Algebras/DBService2/TriggerReplicaDeletionRunnable.hpp"
#include "Algebras/DBService2/DerivationClient.hpp"
#include "Algebras/DBService2/CreateDerivateRunnable.hpp"

#include "boost/filesystem.hpp"

#include <loguru.hpp>

#include <algorithm>
#include <random>
#include <sstream>

using namespace distributed2;
using namespace std;

namespace fs = boost::filesystem;

extern boost::recursive_mutex nlparsemtx;

namespace DBService {

CommunicationServer::CommunicationServer(int port) :
        MultiClientServer(port)
{
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "CommunicationServer port: %d", port);

    string context("CommunicationServer");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context, port, std::cout));
    traceWriter->writeFunction("CommunicationServer::CommunicationServer");
    traceWriter->write("Initializing CommunicationServer");
    traceWriter->write("port", port);


    lookupMinimumReplicaCount();
}

CommunicationServer::~CommunicationServer()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationServer::~CommunicationServer");
}

int CommunicationServer::start()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationServer::start");

    return MultiClientServer::start();
}

void CommunicationServer::lookupMinimumReplicaCount()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction(
            "CommunicationServer::lookupMinimumReplicaCount");

    string replicaNumber;
    SecondoUtilsLocal::readFromConfigFile(replicaNumber,
                                           "DBService",
                                           "ReplicaNumber",
                                           "");
    minimumReplicaCount = atoi(replicaNumber.c_str());
}

int CommunicationServer::communicate(iostream& io)
{
    const boost::thread::id tid = boost::this_thread::get_id();
    traceWriter->writeFunction(tid, "CommunicationServer::communicate");
    LOG_SCOPE_FUNCTION(INFO);

    try
    {
        traceWriter->write(tid, "Communicating...");

        CommunicationUtils::sendLine(io,
                CommunicationProtocol::CommunicationServer());

        if (!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::CommunicationClient()))
        {
            traceWriter->write(tid,
            "Protocol error: Not connected to CommunicationClient");
            LOG_F(ERROR,
                  "Protocol error: Not connected to CommunicationClient");

            return 1;
        }

        string request;
        CommunicationUtils::receiveLine(io, request);

        traceWriter->write(tid, "request", request);
        LOG_F(INFO, "Received request: %s", request.c_str());

        if(request ==
                CommunicationProtocol::TriggerReplication())
        {
            handleTriggerReplicationRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::StartingSignal())
        {
            handleStartingSignalRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::TriggerFileTransfer())
        {
            handleTriggerFileTransferRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::ReplicaLocationRequest())
        {
            handleProvideReplicaLocationRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::ReplicationSuccessful())
        {
            reportSuccessfulReplication(io, tid);
        }else if(request ==
                CommunicationProtocol::DeleteReplicaRequest())
        {
            handleRequestReplicaDeletion(io, tid);
        }else if(request ==
                CommunicationProtocol::TriggerReplicaDeletion())
        {
            handleTriggerReplicaDeletion(io, tid);
        }else if(request ==
                CommunicationProtocol::Ping())
        {
            handlePing(io, tid);
        }else if(request ==
                CommunicationProtocol::RelTypeRequest())
        {
            handleRelTypeRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::DerivedTypeRequest())
        {
            handleDerivedTypeRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::TriggerDerivation())
        { 
            handleTriggerDerivation(io,tid);
        }else if(request ==
                 CommunicationProtocol::CreateDerivation())
        {
           handleCreateDerivation(io,tid);
        }else if(request ==
                 CommunicationProtocol::CreateDerivateSuccessful()){
           reportSuccessfulDerivation(io,tid);
        } else if (request == CommunicationProtocol::AddNodeRequest()) {
            handleAddNodeRequest(io,tid);
        } else
        {
            traceWriter->write(
                    tid, "Protocol error: invalid request: ", request);
            LOG_F(ERROR, "Protocol error: invalid request: %s",
                  request.c_str());

            return 1;
        }
    } catch (char const* exception) {
        traceWriter->write(tid, "CommunicationServer: communication char "
                                "const* exception:");
        traceWriter->write(tid, exception);

        // LOG_F(ERROR, "CommunicationServer: communication char "
        //     "const* exception: %s", exception);

        return 2;
    } catch (std::string const* exception) {
        traceWriter->write(tid, "CommunicationServer: communication string \
                const* exception:");
        traceWriter->write(tid, exception->c_str());

        // LOG_F(ERROR, "CommunicationServer: communication string "
        //              "exception: %s", exception->c_str());

        return 2;    
    } catch (std::string const exception) {
        traceWriter->write(tid, "CommunicationServer: communication string \
                const exception:");
        traceWriter->write(tid, exception.c_str());

        LOG_F(ERROR, "CommunicationServer: communication const string "
                     "exception: %s", exception.c_str());

        return 2;
    }

    return 0;
}

bool CommunicationServer::handleTriggerReplicationRequest(
        std::iostream& io, const boost::thread::id tid)
{
    bool success = false;
    // The original system (e.g. the master) triggers the actual replication

    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerReplicationRequest");
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);

    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);
    LOG_F(INFO, "databaseName: %s", databaseName.c_str());
    LOG_F(INFO, "relationName: %s", relationName.c_str());

    DBServiceManager* dbService = DBServiceManager::getInstance();

    traceWriter->write(tid, "Got a DBServiceManager instance.");
    LOG_F(INFO, "Got a DBServiceManager instance.");

    // Check if the DBService already has a replica of the given database & 
    //  relation combination ...
    if(dbService->replicaExists(databaseName, relationName))
    {
        /* 
          Updates are not supported. 
          If the DBService already has a replica the replication is cancelled.
        */
        traceWriter->write(tid, 
            "The relation exists and has replicas. Aborting.");

        LOG_F(ERROR, "The relation exists and has replicas."
                     "This is currently not supported. Aborting");

        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicaExists());
        return false;
    }

    traceWriter->write(tid, "The relation doesn't exist. Initiating \
replication. Requesting a target node...");

    LOG_F(INFO, "The relation doesn't exist. Initiating "
                "replication. Requesting a target node...");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    traceWriter->write(tid, "Location request issued.");
    LOG_F(INFO, "Target node requested.");

    CommunicationUtils::receiveLines(io, 4, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();
    string disk = receiveBuffer.front();
    receiveBuffer.pop();
    string transferPort = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "Received the following target node:");
    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);
    traceWriter->write(tid, "disk", disk);
    traceWriter->write(tid, "transferPort", transferPort);

    LOG_F(INFO, "Received the following target node: "
                "Host: %s, Port: %s, Disk: %s, TransferPort: %s",
                host.c_str(), port.c_str(), disk.c_str(), transferPort.c_str());

    // Given the database and relation which needs to replicated, it now needs 
    // to be determined
    // whereto the relation can be replicated. This can be a single or 
    // mulitple worker nodes 
    // as replication targets.
    traceWriter->write(tid, "Determinig replica locations...");
    LOG_F(INFO, "Determinig replica locations...");
    
    /*
        TODO Rename determineReplicaLocations as it also creates the relation,
        original node, do the replica placement and stores all records if 
        a placement can be done.
     */
    success = dbService->determineReplicaLocations(databaseName,
                                 relationName,
                                 host,
                                 port,
                                 disk);

    traceWriter->write(tid, "Done determinig replica locations...");
    LOG_F(INFO, "Done determinig replica locations...");

    if (success) {
        traceWriter->write(tid, "Replica placement successful.");
        LOG_F(INFO, "Replica placement successful.");
        
        CommunicationUtils::sendLine(io,
            CommunicationProtocol::ReplicationTriggered());
        
    } else {
        traceWriter->write(tid, "Replica placement failed.");
        traceWriter->write(tid, dbService->getMessages().c_str());

        LOG_F(ERROR, "The replica placement has failed. "
                     "Placement strategy message is: %s",
                        dbService->getMessages().c_str());

        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationCanceled());
    }

    return true;
}

bool CommunicationServer::handleStartingSignalRequest(
        std::iostream& io, const boost::thread::id tid)
{

    traceWriter->writeFunction(tid,
            "CommunicationServer::handleStartingSignalRequest");
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());
    
    // relID = {DATABASE}xDBSx{RELATION_NAME}
    string relID;
    CommunicationUtils::receiveLine(io, relID);

    traceWriter->write(tid, "relID", relID);
    LOG_F(INFO, "relID: %s", relID.c_str());

    string relationDatabase;
    string relationName;

    //TODO Refactor -> Eliminate dependency to RelationInfo
    RelationInfo::parseIdentifier(relID, relationDatabase, relationName);
    
    //TODO use shared_ptr
    DBServiceManager* dbService = DBServiceManager::getInstance();

    shared_ptr<DBService::Relation> relation = dbService->getRelation(
        relationDatabase, relationName);
    
    if (relation == nullptr) {        
        stringstream msg;
        msg << "Couldn't find the relation (relationDatabase: ";
        msg << relationDatabase << ", relationName: " << relationName;

        traceWriter->write(msg.str());
        LOG_F(WARNING, "%s", msg.str().c_str());

        return false;
    }

    /* JF:
        Locations have been pretermined as during this step it was also possible
        to reject the replication, e.g. due to missing nodes.

        In this phase it is about starting the file transfers, one for each
        selected replica locations.
    */
    shared_ptr<DBService::Node> originalNode = relation->getOriginalNode();

    traceWriter->write("Triggering file transfers...");
    LOG_F(INFO, "Triggering file transfers...");

    shared_ptr<DBService::Node> replicaTargetNode;

    for( auto& replica : relation->getReplicas()) {
        replicaTargetNode = replica->getTargetNode();
        TriggerFileTransferRunnable clientToDBServiceWorker(
            originalNode->getHost().getHostname(),
            originalNode->getTransferPort(),
            replicaTargetNode->getHost().getHostname(), // DBService Worker
            replicaTargetNode->getComPort(),        
            relation->getRelationDatabase(),
            relation->getName());                    

        clientToDBServiceWorker.run();
    }

    return true;
}

/*

The CommunicationClient triggering this is located on the DBService Master.
This CommunicationServer is running on a DBService Worker Node.
It will trigger a ReplicationClientRunnable which in turn creates a
ReplicationClient which then will retrieve the Relation from the original
Node.

*/
bool CommunicationServer::handleTriggerFileTransferRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerFileTransferRequest");
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::ReplicationDetailsRequest());

    traceWriter->write(tid, "Sent ReplicationDetailsRequest");
    LOG_F(INFO, "Sent the ReplicationDetailsRequest");

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 5, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();
    string fileName = receiveBuffer.front();
    receiveBuffer.pop();
    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    //TODO Explain what these details are about. From or to location?
    traceWriter->write(tid, "received replication details");
    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);

    // TODO remove filename, is determined automatically
    traceWriter->write(tid, "fileName", fileName);
    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);

    LOG_F(INFO, "Received replication details. Host: %s, Port: %s, "
                "filename: %s, database: %s, relation: %s",
          host.c_str(), port.c_str(), fileName.c_str(), databaseName.c_str(),
          relationName.c_str());

    LOG_F(INFO, "%s", "Creating ReplicationClientRunnable...");
    ReplicationClientRunnable replicationClient(
            host,
            atoi(port.c_str()),
            databaseName,
            relationName);

    LOG_F(INFO, "%s", "Running ReplicationClientRunnable...");
    replicationClient.run();
    LOG_F(INFO, "%s", "Done running ReplicationClientRunnable.");

    return true;
}

bool CommunicationServer::handleProvideReplicaLocationRequest(
        std::iostream& io, const boost::thread::id tid)
{

    traceWriter->writeFunction(tid,
            "CommunicationServer::handleProvideReplicaLocationRequest");
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);

    // START receving derivate information
    // read number of other objects
    string n;
    CommunicationUtils::receiveLine(io, n);

    bool correct;
    int number = stringutils::str2int<int>(n,correct); //TODO: error handling
    if(number<0) number = 0;
    
    queue<string> otherObjects;
    if(number>0){
      CommunicationUtils::receiveLines(io,number,otherObjects);
    }
    // END receving derivate information
    
    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);
    LOG_F(INFO, "Database: %s, Relation: %s",
          databaseName.c_str(), relationName.c_str());

    DBServiceManager* dbService = DBServiceManager::getInstance();

    /*    
    - Retrieve replicas of the requestion relation, 
    - Select a random replica
    - Return its targetNode
    */
    
    shared_ptr<DBService::Node> node = dbService->getRandomNodeWithReplica(
            databaseName, relationName);

    /*
      CommunicationProtocol
      Didn't find a node

      This is a non-success case.
    */
    queue<string> sendBuffer;

    if(node == nullptr)
    {        
        traceWriter->write(tid, "Didn't find a targetNode. Maybe relation "
                                "doesn't exist or relation has no replicas.");


        traceWriter->write(tid, "My data:");
        stringstream msg;
        dbService->printMetadata(msg);
        traceWriter->write(tid, msg.str().c_str());

        LOG_F(WARNING, "Didn't find a targetNode. Maybe relation "
                       "doesn't exist or relation has no replicas. My data: %s",
              msg.str().c_str());

        sendBuffer.push(CommunicationProtocol::None());
        sendBuffer.push(CommunicationProtocol::None());
        sendBuffer.push(CommunicationProtocol::None());
        CommunicationUtils::sendBatch(io, sendBuffer);
        return false;
    }

    traceWriter->write(tid, "Found a target Node: ");
    traceWriter->write(tid, node->str().c_str());

    LOG_F(INFO, "Found target node: %s", node->str().c_str());

    /*
      CommunicationProtocol
      Sending the location by providing:
      - host
      - transferPort
      - comPort

      This is the success case.
    */
    sendBuffer.push(node->getHost().getHostname());
    sendBuffer.push(to_string(node->getTransferPort()));
    sendBuffer.push(to_string(node->getComPort()));
    CommunicationUtils::sendBatch(io, sendBuffer);

    return true;
}

bool CommunicationServer::reportSuccessfulReplication(
        iostream& io,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::reportSuccessfulReplication");
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    string relID;
    CommunicationUtils::receiveLine(io, relID);
    traceWriter->write(tid, "relID", relID);
    LOG_F(INFO, "reldID: %s", relID.c_str());

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);
    LOG_F(INFO, "Host: %s, Port: %s", host.c_str(), port.c_str());

    DBServiceManager::getInstance()->maintainSuccessfulReplication(
            relID, host, port);
    return true;
}

bool CommunicationServer::handleRequestReplicaDeletion(
        iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
                "CommunicationServer::handleRequestReplicaDeletion");
    LOG_SCOPE_FUNCTION(INFO);

    string databaseName;
    string relationName;
    string derivateName; 
    CommunicationUtils::receiveLine(io, databaseName);
    CommunicationUtils::receiveLine(io, relationName);
    CommunicationUtils::receiveLine(io, derivateName);
    // if the relation and all derivates should be removed,
    // the derivateName will be empty
    traceWriter->write("database", databaseName);
    traceWriter->write("relation", relationName);
    traceWriter->write("derived", derivateName);
    LOG_F(INFO, "Database: %s, Relation: %s, Derivative: %s",
          databaseName.c_str(), relationName.c_str(), derivateName.c_str());
    
    DBServiceManager* dbService = DBServiceManager::getInstance();
    try{

      dbService->deleteReplicaMetadata(databaseName,relationName,
                                         derivateName);
    }catch(...)
    {
        traceWriter->write(tid, "Relation does not exist");
        LOG_F(WARNING, "The relation does not exist.");
        return false;
    }

    return true;
}

bool CommunicationServer::handleTriggerReplicaDeletion(
        std::iostream& io,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
                "CommunicationServer::handleTriggerReplicaDeletion");
    LOG_SCOPE_FUNCTION(INFO);

    string databaseName;
    string relationName;
    string derivateName;
    CommunicationUtils::receiveLine(io, databaseName);
    CommunicationUtils::receiveLine(io, relationName);
    CommunicationUtils::receiveLine(io, derivateName);
    
    string victim;
    bool success;

    if(derivateName.empty())
    {
        // remove File
        fs::path filename = ReplicationUtils::getFilePathOnDBServiceWorker(
            databaseName,
            relationName);

       traceWriter->write("filename", filename.string());
       LOG_F(INFO, "Filename: %s", filename.string().c_str());

       success = FileSystem::DeleteFileOrFolder( filename.string() );

       if (success) {
           traceWriter->write("Successfully deleted file.");
           LOG_F(INFO, "Successfully deleted file.");
       } else {
           traceWriter->write("Couldn't delete file.");
           LOG_F(ERROR, "Couldn't delete file");
       }

       victim = ReplicationUtils::getRelName(filename.filename().string());
    } else {
       string relId = RelationInfo::getIdentifier(databaseName, relationName);
       victim = DerivateInfo::getIdentifier(relId, derivateName);
    }
   
    // ensure to have only one access to the catalog
    static boost::mutex mtx;

    LOG_F(INFO, "%s", "Acquiring lock for SecondoCatalog...");
    boost::lock_guard<boost::mutex> guard(mtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for SecondoCatalog.");

    traceWriter->write("database", databaseName);
    traceWriter->write("relation", relationName),
    traceWriter->write("derivate", derivateName);
    traceWriter->write("victim", victim);
    LOG_F(INFO, "Database: %s, Relation: %s, Derivative: %s, Victim: %s",
          databaseName.c_str(), relationName.c_str(), derivateName.c_str(),
          victim.c_str());

    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    SecondoSystem::BeginTransaction();
    ctlg->DeleteObject(victim);    
    ctlg->CleanUp(true);
    SecondoSystem::CommitTransaction(false);

    traceWriter->write("Deleted object (victim).");
    LOG_F(INFO, "Deleted object (victim).");

    //TODO check return code etc
    //TODO tracing
    return true;
}

bool CommunicationServer::handleAddNodeRequest(
        std::iostream& io,
        const boost::thread::id tid) {

    traceWriter->writeFunction(tid,
            "CommunicationServer::handleAddNodeRequest");
    LOG_SCOPE_FUNCTION(INFO);
    
    // Confirm the AddNodeRequest
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::AddNodeRequest());
    
    string nodeHost;
    string nodePort;
    string nodeConfigPath;

    CommunicationUtils::receiveLine(io, nodeHost);
    CommunicationUtils::receiveLine(io, nodePort);
    CommunicationUtils::receiveLine(io, nodeConfigPath);

    traceWriter->write("nodeHost", nodeHost);
    traceWriter->write("nodePort", nodePort);
    traceWriter->write("nodeConfigPath", nodeConfigPath);
    LOG_F(INFO, "Node: (Host: %s, Port: %s, ConfigPath: %s",
          nodeHost.c_str(), nodePort.c_str(), nodeConfigPath.c_str());

    // TODO Talk to DBServiceManager then return
    DBServiceManager* dbService = DBServiceManager::getInstance();

    bool success = dbService->addNode(nodeHost, stoi(nodePort), nodeConfigPath);
    
    if (success) {
        // Success
        traceWriter->write("Successfully added node.");
        LOG_F(INFO, "Successfully added node.");

        CommunicationUtils::sendLine(io,
            CommunicationProtocol::NodeAdded());
        return true;

    } else {
        traceWriter->write("dbServiceManager::addNode failed.");
        LOG_F(WARNING, "dbServiceManager::addNode failed.");

        CommunicationUtils::sendLine(io,
            CommunicationProtocol::AddNodeFailed());
        return false;
    }

    return false;
}

bool CommunicationServer::handlePing(
        std::iostream& io, const boost::thread::id tid)
{
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::Ping());
    return true;
}

bool CommunicationServer::handleRelTypeRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleRelTypeRequest");
    LOG_SCOPE_FUNCTION(INFO);


    string relID;
    CommunicationUtils::receiveLine(io, relID);

    string databaseName;
    string relationName;
    RelationInfo::parseIdentifier(relID, databaseName, relationName);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);
    LOG_F(INFO, "Database: %s, Relation: %s",
          databaseName.c_str(), relationName.c_str());

    // Here filename is enough. No no need to use a path as all that mattes
    // is extracting the relationName. 
    // JF: Strange enough as the relationName is also passed as a parameter?!s
    string fileName = ReplicationUtils::getFileNameOnDBServiceWorker(
                        databaseName,
                        relationName);

    traceWriter->write("fileName", fileName);
    string relname = ReplicationUtils::getRelName(fileName);
    traceWriter->write("relName ", relname);

    LOG_F(INFO, "Filename: %s, Relation: %s",
            fileName.c_str(), relname.c_str());

    LOG_F(INFO, "%s", "Acquiring lock for nlparsemtx...");
    // Restrict access to the nested list
    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for nlparsemtx...");
    
    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    if(!ctlg->IsObjectName(relname)){
        traceWriter->write(relname + " is not a database object");
        LOG_F(WARNING, "%s is not a database object.", relname.c_str());

        CommunicationUtils::sendLine(io, CommunicationProtocol::None());
    } else {
        traceWriter->write(relname + " is a database object");
        LOG_F(INFO, "%s is a database object.", relname.c_str());

        ListExpr type = ctlg->GetObjectTypeExpr(relname);

        traceWriter->write("relType is " , nl->ToString(type));
        LOG_F(INFO, "Relation type is %s.", nl->ToString(type).c_str());

        type = nl->TwoElemList(
               nl->SymbolAtom(Symbol::STREAM()),
               nl->Second(type));
        CommunicationUtils::sendLine(io, nl->ToString(type));
    }
    return true;
}

bool CommunicationServer::handleDerivedTypeRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleDerivedTypeRequest");
    LOG_SCOPE_FUNCTION(INFO);

    // ensure to have only one access to the catalog
    

    string relID;
    CommunicationUtils::receiveLine(io, relID);
    string derivedName;
    CommunicationUtils::receiveLine(io, derivedName);
    string databaseName;
    string relationName;
    RelationInfo::parseIdentifier(relID, databaseName, relationName);

    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);
    traceWriter->write("derivedName ", derivedName);

    LOG_F(INFO, "Database: %s, Relation: %s, Derivative: %s",
          databaseName.c_str(), relationName.c_str(), derivedName.c_str());

    string objectName = ReplicationUtils::getDerivedName(
                                           databaseName,
                                           relationName,
                                           derivedName);

    traceWriter->write("ObjectName ", objectName);
    LOG_F(INFO, "ObjectName: %s", objectName.c_str());

    LOG_F(INFO, "%s", "Acquiring lock for nlparsemtx...");
    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for nlparsemtx...");

    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    if(!ctlg->IsObjectName(objectName)){
        traceWriter->write(objectName + " is not a database object");
        LOG_F(WARNING, "%s is not a database object.", objectName.c_str());

        CommunicationUtils::sendLine(io, CommunicationProtocol::None());
    } else {
        traceWriter->write(objectName + " is a database object");
        LOG_F(INFO, "%s is a database object.", objectName.c_str());

        ListExpr type = ctlg->GetObjectTypeExpr(objectName);

        traceWriter->write("Object type is " , nl->ToString(type));
        LOG_F(INFO, "Object type is: %s.", nl->ToString(type).c_str());

        CommunicationUtils::sendLine(io, nl->ToString(type));
    }
    return true;
}

bool CommunicationServer::handleTriggerDerivation(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerDerivation");
    LOG_SCOPE_FUNCTION(INFO);

    LOG_F(INFO, "%s", "Acquiring lock for SecondoCatalog...");
    // ensure to have only one access to the catalog    
    static boost::mutex mtx;
    boost::lock_guard<boost::mutex> guard(mtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for SecondoCatalog...");

    // request derivation information
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::DerivationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 4, receiveBuffer);

    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string targetName = receiveBuffer.front();
    receiveBuffer.pop();
    string relName = receiveBuffer.front();
    receiveBuffer.pop();
    string fundef = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "targetName", targetName);
    traceWriter->write(tid, "relationName", relName);
    traceWriter->write(tid, "fundef", fundef);

    LOG_F(INFO, "Database: %s, TargetName: %s, Relation: %s, Fundef: %s",
          databaseName.c_str(), targetName.c_str(), relName.c_str(),
          fundef.c_str());

    DBServiceManager* dbService = DBServiceManager::getInstance();

    if(!dbService->replicaExists(databaseName, relName))
    {
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::RelationNotExists());
        return false;
    }

    if(dbService->derivateExists(targetName)){
        CommunicationUtils::sendLine(io, 
                 CommunicationProtocol::ObjectExists());
        return false;
    }    

    dbService->addDerivative(databaseName, relName, targetName, fundef);

    CommunicationUtils::sendLine (
        io, CommunicationProtocol::DerivationTriggered ());
    return true;
}


bool CommunicationServer::handleCreateDerivation(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleCreateDerivation");

    LOG_SCOPE_FUNCTION(INFO);

    LOG_F(INFO, "%s", "Acquiring lock for SecondoCatalog...");
    // ensure to have only one access to the catalog
    static boost::mutex mtx;
    boost::lock_guard<boost::mutex> guard(mtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for SecondoCatalog...");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::DerivationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 4, receiveBuffer);

    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string targetName = receiveBuffer.front();
    receiveBuffer.pop();
    string relName = receiveBuffer.front();
    receiveBuffer.pop();
    string fundef = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "targetName", targetName);
    traceWriter->write(tid, "relationName", relName);
    traceWriter->write(tid, "fundef", fundef);

    LOG_F(INFO, "Database: %s, TargetName: %s, Relation: %s, Fundef: %s",
          databaseName.c_str(), targetName.c_str(), relName.c_str(),
          fundef.c_str());

    DerivationClient dc(databaseName, targetName, relName, fundef);
    dc.start();
    return true;
}

bool CommunicationServer::reportSuccessfulDerivation(
       std::iostream& io, const boost::thread::id tid){

    traceWriter->writeFunction(tid,
            "CommunicationServer::reportSuccessfulDerivation");

    LOG_SCOPE_FUNCTION(INFO);

    LOG_F(INFO, "%s", "Acquiring lock for SecondoCatalog...");
    // ensure to have only one access to the catalog
    static boost::mutex mtx;
    boost::lock_guard<boost::mutex> guard(mtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for SecondoCatalog...");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::ObjectRequest());

    string objectID;
    CommunicationUtils::receiveLine(io, objectID);

    traceWriter->write(tid, "objectID", objectID);
    LOG_F(INFO, "ObjectID: %s", objectID.c_str());

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);
    LOG_F(INFO, "Host: %s, Port: %s", host.c_str(), port.c_str());

    DBServiceManager::getInstance()->maintainSuccessfulDerivation(
            objectID, host, port);
    return true;
}

} /* namespace DBService */
