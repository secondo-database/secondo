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
#include <random>
#include "Algebra.h"
#include "FileSystem.h"
#include "StringUtils.h"

#include "SecondoCatalog.h"
#include "SecondoSystem.h"

#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/ReplicationClientRunnable.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/TriggerFileTransferRunnable.hpp"
#include "Algebras/DBService/TriggerReplicaDeletionRunnable.hpp"
#include "Algebras/DBService/DerivationClient.hpp"
#include "Algebras/DBService/CreateDerivateRunnable.hpp"

using namespace distributed2;
using namespace std;

namespace DBService {

CommunicationServer::CommunicationServer(int port) :
        MultiClientServer(port)
{
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
    traceWriter->writeFunction("CommunicationServer::~CommunicationServer");
}

int CommunicationServer::start()
{
    traceWriter->writeFunction("CommunicationServer::start");
    return MultiClientServer::start();
}

void CommunicationServer::lookupMinimumReplicaCount()
{
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
            return 1;
        }

        string request;
        CommunicationUtils::receiveLine(io, request);

        traceWriter->write(tid, "request", request);

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
        }else
        {
            traceWriter->write(
                    tid, "Protocol error: invalid request: ", request);
            return 1;
        }
    } catch (...)
    {
        traceWriter->write(tid, "CommunicationServer: communication error");
        return 2;
    }
    return 0;
}

bool CommunicationServer::handleTriggerReplicationRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerReplicationRequest");
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

    DBServiceManager* dbService = DBServiceManager::getInstance();
    if(dbService->replicaExists(databaseName, relationName))
    {
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicaExists());
        return false;
    }

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    CommunicationUtils::receiveLines(io, 4, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();
    string disk = receiveBuffer.front();
    receiveBuffer.pop();
    string transferPort = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);
    traceWriter->write(tid, "disk", disk);
    traceWriter->write(tid, "transferPort", transferPort);

    dbService->determineReplicaLocations(databaseName,
                                 relationName,
                                 host,
                                 port,
                                 disk);
    vector<pair<ConnectionID, bool> > locations;
    dbService->getReplicaLocations(RelationInfo::getIdentifier(
            databaseName, relationName),
            locations);
    dbService->setOriginalLocationTransferPort(
            RelationInfo::getIdentifier(databaseName, relationName),
            transferPort);

    if(locations.size() < (size_t)minimumReplicaCount)
    {
        dbService->deleteRelationLocations(databaseName, relationName);
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationCanceled());
    }else
    {
        dbService->persistReplicaLocations(databaseName, relationName);
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationTriggered());
    }
    return true;
}

bool CommunicationServer::handleStartingSignalRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleStartingSignalRequest");
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    string relID;
    CommunicationUtils::receiveLine(io, relID);
    traceWriter->write(tid, "relID", relID);

    vector<pair<ConnectionID, bool> > locations;
    DBServiceManager* dbService = DBServiceManager::getInstance();
    dbService->getReplicaLocations(relID, locations);
    RelationInfo& relationInfo = dbService->getRelationInfo(relID);

    string originalLocationHost = relationInfo.
            getOriginalLocation().getHost();
    traceWriter->write("originalLocationHost", originalLocationHost);
    string originalLocationTransferPort = relationInfo.
            getOriginalLocation().
            getTransferPort();
    traceWriter->write("originalLocationTransferPort",
            originalLocationTransferPort);

    traceWriter->write("Triggering file transfers");
    for(vector<pair<ConnectionID, bool> >::const_iterator it
                = locations.begin(); it != locations.end(); it++)
        {
            LocationInfo locationInfo = dbService->getLocation((*it).first);
            traceWriter->write(locationInfo);
            TriggerFileTransferRunnable clientToDBServiceWorker(
                    originalLocationHost, /*original location*/
                    atoi(originalLocationTransferPort.
                            c_str()), /*original location*/
                    locationInfo.getHost(), /*DBService*/
                    atoi(locationInfo.getCommPort().c_str()), /*DBService*/
                    relationInfo.getDatabaseName(),
                    relationInfo.getRelationName());

            clientToDBServiceWorker.run();
        }
    return true;
}

bool CommunicationServer::handleTriggerFileTransferRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerFileTransferRequest");
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::ReplicationDetailsRequest());
    traceWriter->write(tid, "sent ReplicationDetailsRequest");

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
    traceWriter->write(tid, "received replication details");
    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);
    // TODO remove filename, is determined automatically
    traceWriter->write(tid, "fileName", fileName);
    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);

    ReplicationClientRunnable replicationClient(
            host,
            atoi(port.c_str()),
            databaseName,
            relationName);
    replicationClient.run();
    return true;
}

bool CommunicationServer::handleProvideReplicaLocationRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleProvideReplicaLocationRequest");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);

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
    
    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    ConnectionID randomReplicaLocation = 0;

    try
    {
       vector<ConnectionID> possibleLocations;
       string relName = RelationInfo::getIdentifier(databaseName, relationName);
       RelationInfo relInfo =dbService->getRelationInfo(relName);
       relInfo.getAllLocations(possibleLocations);
       std::sort(possibleLocations.begin(), possibleLocations.end());

       traceWriter->write("check for presence of derived objects");
       while(!otherObjects.empty() && !possibleLocations.empty())
       {
          string n = otherObjects.front();
          otherObjects.pop();
          traceWriter->write(tid, "look for derivate " , n);
          string oname = DerivateInfo::getIdentifier(relName, n);
          traceWriter->write("derivateID : " , oname);
          DerivateInfo derInfo = dbService->getDerivateInfo(oname);
          traceWriter->write("derivateInfo found");

          vector<ConnectionID> derLocs;
          derInfo.getAllLocations(derLocs);
          sort(derLocs.begin(), derLocs.end());


          vector<ConnectionID> inter;
          set_intersection(possibleLocations.begin(), possibleLocations.end(),
                           derLocs.begin(), derLocs.end(), 
                           back_inserter(inter));
          inter.swap(possibleLocations);
       }
       if(!possibleLocations.empty()){
          traceWriter->write(tid, "choose random location"); 
          shuffle(possibleLocations.begin(), possibleLocations.end(), 
            std::mt19937(std::random_device()()));
          randomReplicaLocation = possibleLocations.front(); 
       } else {
           traceWriter->write(tid, "No locations available");
           dbService->printDerivates( std::cout);
       }
       
    } catch(...)
    {
       traceWriter->write(tid, "Exception: No location found");
       dbService->printDerivates(std::cout);
    }

    queue<string> sendBuffer;
    if(randomReplicaLocation == 0)
    {
        sendBuffer.push(CommunicationProtocol::None());
        sendBuffer.push(CommunicationProtocol::None());
        sendBuffer.push(CommunicationProtocol::None());
        CommunicationUtils::sendBatch(io, sendBuffer);
        return false;
    }
    LocationInfo location = dbService->getLocation(randomReplicaLocation);

    sendBuffer.push(location.getHost());
    sendBuffer.push(location.getTransferPort());
    sendBuffer.push(location.getCommPort());
    CommunicationUtils::sendBatch(io, sendBuffer);

    return true;
}

bool CommunicationServer::reportSuccessfulReplication(
        iostream& io,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::reportSuccessfulReplication");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    string relID;
    CommunicationUtils::receiveLine(io, relID);
    traceWriter->write(tid, "relID", relID);

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

    DBServiceManager::getInstance()->maintainSuccessfulReplication(
            relID, host, port);
    return true;
}



void CommunicationServer::deleteRemoteDerivate(
        const string& databaseName,
        const string& relationName,
        const string& derivateName){

   string derId = DerivateInfo::getIdentifier(databaseName,relationName, 
                                              derivateName);
   DBServiceManager* dbService = DBServiceManager::getInstance();
   DerivateInfo& derInfo = dbService->getDerivateInfo(derId);
   for(ReplicaLocations::const_iterator it =
         derInfo.nodesBegin(); it != derInfo.nodesEnd(); it++)
   {
       if(it->second)
       {
          LocationInfo& locationInfo =dbService->getLocation(it->first);
          TriggerReplicaDeletionRunnable replicaEraser(
              locationInfo.getHost(),
               atoi(locationInfo.getCommPort().c_str()),
               databaseName, relationName, derivateName);
          replicaEraser.run();
      }
  }
}

void CommunicationServer::deleteRemoteRelation(
        const string& databaseName,
        const string& relationName)
{
   string relId = RelationInfo::getIdentifier(databaseName, relationName);
   DBServiceManager* dbService = DBServiceManager::getInstance();
   RelationInfo& relationInfo = dbService->getRelationInfo(relId);
   // remove the relation itselfs 
   for(ReplicaLocations::const_iterator it =
       relationInfo.nodesBegin(); it!=relationInfo.nodesEnd(); it++)
   {
       if(it->second)
       {
          LocationInfo& locationInfo = dbService->getLocation(it->first);
          TriggerReplicaDeletionRunnable replicaEraser(
                locationInfo.getHost(),
                atoi(locationInfo.getCommPort().c_str()),
                databaseName, relationName,"");
          replicaEraser.run();
       }
   }
   // delete all derivates that depend on relation
   vector<string> derivates;
   dbService->findDerivates(relId, derivates);
   for(auto& t: derivates)
   {
       string db, rel, der;
       if(!ReplicationUtils::extractDerivateInfo(t, db, rel, der)){
                traceWriter->write("problem in extracting derivate info " 
                                   "from id");
       } else {
           deleteRemoteDerivate(db,rel,der);
       }
   }
}

void CommunicationServer::deleteRemoteDatabase(const string& databaseName){

    DBServiceManager* dbService = DBServiceManager::getInstance();
    vector<string> relations;
    dbService->findRelations(databaseName, relations);
    for(auto& r: relations){
        string db;
        string rel;
        if(ReplicationUtils::extractRelationInfo(r,db,rel)){
          deleteRemoteRelation(db,rel);
        }
    }
}


bool CommunicationServer::handleRequestReplicaDeletion(
        iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
                "CommunicationServer::handleRequestReplicaDeletion");

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
    
    DBServiceManager* dbService = DBServiceManager::getInstance();
    try{
      if(relationName.empty()){
          deleteRemoteDatabase(databaseName);
      } else if(derivateName.empty()){
          deleteRemoteRelation(databaseName, relationName);
      } else {
          deleteRemoteDerivate(databaseName, relationName, derivateName);
      }
      dbService->deleteReplicaMetadata(databaseName,relationName,
                                         derivateName);
    }catch(...)
    {
        traceWriter->write(tid, "Relation does not exist");
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

    string databaseName;
    string relationName;
    string derivateName;
    CommunicationUtils::receiveLine(io, databaseName);
    CommunicationUtils::receiveLine(io, relationName);
    CommunicationUtils::receiveLine(io, derivateName);
    
    string victim;

    if(derivateName.empty())
    {
       // remove File
       string filename = ReplicationUtils::getFileNameOnDBServiceWorker(
                                  databaseName,
                                  relationName);
       FileSystem::DeleteFileOrFolder( filename );
       victim = ReplicationUtils::getRelName(filename);
    } 
    else
    {
       string relId = RelationInfo::getIdentifier(databaseName, relationName);
       victim = DerivateInfo::getIdentifier(relId, derivateName);
    }
   
    // ensure to have only one access to the catalog
    static boost::mutex mtx;
    boost::lock_guard<boost::mutex> guard(mtx);
 

    traceWriter->write("database", databaseName);
    traceWriter->write("relation", relationName),
    traceWriter->write("derivate", derivateName);
    traceWriter->write("victim", victim);

    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    SecondoSystem::BeginTransaction();
    ctlg->DeleteObject(victim);    
    ctlg->CleanUp(true);
    SecondoSystem::CommitTransaction(false);

    //TODO check return code etc
    //TODO tracing
    return true;
}

bool CommunicationServer::handlePing(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handlePing");
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::Ping());
    return true;
}

bool CommunicationServer::handleRelTypeRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleRelTypeRequest");
    string relID;
    CommunicationUtils::receiveLine(io, relID);

    string databaseName;
    string relationName;
    RelationInfo::parseIdentifier(relID, databaseName, relationName);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    string fileName = ReplicationUtils::getFileNameOnDBServiceWorker(
                        databaseName,
                        relationName);

    traceWriter->write("fileName", fileName);
    string relname = ReplicationUtils::getRelName(fileName);
    traceWriter->write("relName ", relname);
    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    if(!ctlg->IsObjectName(relname)){
        traceWriter->write(relname + " is not a database object");
        CommunicationUtils::sendLine(io, CommunicationProtocol::None());
    } else {
        traceWriter->write(relname + " is a database object");
        ListExpr type = ctlg->GetObjectTypeExpr(relname);
        traceWriter->write("relType is " , nl->ToString(type));
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

    string objectName = ReplicationUtils::getDerivedName(
                                           databaseName,
                                           relationName,
                                           derivedName);
    traceWriter->write("ObjectName ", objectName);



    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    if(!ctlg->IsObjectName(objectName)){
        traceWriter->write(objectName + " is not a database object");
        CommunicationUtils::sendLine(io, CommunicationProtocol::None());
    } else {
        traceWriter->write(objectName + " is a database object");
        ListExpr type = ctlg->GetObjectTypeExpr(objectName);
        traceWriter->write("objectName is " , nl->ToString(type));
        CommunicationUtils::sendLine(io, nl->ToString(type));
    }
    return true;
}

bool CommunicationServer::handleTriggerDerivation(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerDerivation");

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

    // for all workers holding a replica of the relation
    // send command to the worker to derive the object
    string relId = RelationInfo::getIdentifier(databaseName, relName);
    ReplicaLocations rl;
    string  derId = dbService->determineDerivateLocations(targetName, relId,
                                                          fundef);
    dbService->persistDerivateLocations(derId);
    

    dbService->getReplicaLocations(relId, rl);
    ReplicaLocations::iterator it;
    for(it = rl.begin(); it!=rl.end();it++){
       if(it->second){ // relation is replicated
          ConnectionID cid = it->first;
          LocationInfo& li = dbService->getLocation(cid);
          CreateDerivateRunnable cdr(li.getHost(),
                                     atoi(li.getCommPort().c_str()),
                                     databaseName,
                                     targetName,
                                     relName,
                                     fundef);
          cdr.run();
                                     
       }
    }
    CommunicationUtils::sendLine(io, 
              CommunicationProtocol::DerivationTriggered());
    return true;
}


bool CommunicationServer::handleCreateDerivation(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleCreateDerivation");
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

    DerivationClient dc(databaseName, targetName, relName, fundef);
    dc.start();
    return true;
}

bool CommunicationServer::reportSuccessfulDerivation(
       std::iostream& io, const boost::thread::id tid){

    traceWriter->writeFunction(tid,
            "CommunicationServer::reportSuccessfulDerivation");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::ObjectRequest());

    string objectID;
    CommunicationUtils::receiveLine(io, objectID);
    traceWriter->write(tid, "objectID", objectID);

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

    DBServiceManager::getInstance()->maintainSuccessfulDerivation(
            objectID, host, port);
    return true;
}




} /* namespace DBService */
