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

TOOD Functions here contain a lot of repetitive code. Refactor!

*/
#include <iostream>

#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService2/CommunicationClient.hpp"
#include "Algebras/DBService2/CommunicationProtocol.hpp"
#include "Algebras/DBService2/CommunicationUtils.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"

#include <loguru.hpp>

using namespace std;
using namespace distributed2;

namespace DBService {

CommunicationClient::CommunicationClient(
        string& server, int port, Socket* socket)
:Client(server, port, socket)
{
    LOG_SCOPE_FUNCTION(INFO);
    string context("CommunicationClient");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context, port, std::cout));

    traceWriter->writeFunction("CommunicationClient::CommunicationClient");
    traceWriter->write("Connecting to server: ", server);
    traceWriter->write("On port:", port);

    LOG_F(INFO, "Connecting to server: %s on Port %d...", server.c_str(), port);
}

CommunicationClient::~CommunicationClient()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::~CommunicationClient");
}

int CommunicationClient::start()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::start");
    socket = Socket::Connect(server, stringutils::int2str(port),
                Socket::SockGlobalDomain, 3, 1);
        if (!socket) {
            traceWriter->write("Socket initialization failed");
            LOG_F(ERROR, "%s", "Socket initialization failed");
            return 8;
        }
        if (!socket->IsOk()) {
            LOG_F(ERROR, "The socket not ok. Tried to connect to %s port %d.",
                server.c_str(), port);

            traceWriter->write("The socket not ok. Tried to connect to");
            traceWriter->write("Host", server);
            traceWriter->write("Port", stringutils::int2str(port));
            return 9;
        }
        return 0;
}

bool CommunicationClient::triggerReplication(const string& databaseName,
                                            const string& relationName)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::triggerReplication");

    if(!connectionTargetIsDBServiceMaster())
    {
        LOG_F(WARNING, "%s", "Aborting. The target must be the DBService \
            Master.");
        traceWriter->write("Aborting. The target must be the DBService \
            Master.");        
        return false;
    }

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server. start() failed.");
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Connected to a non-CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerReplication());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {
        LOG_F(ERROR, "%s", "Did not receive expected RelationRequest keyword.");
        traceWriter->write("Did not receive expected RelationRequest keyword.");
        return false;
    }
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    string receivedLine;
    CommunicationUtils::receiveLine(io, receivedLine);

    if(receivedLine == CommunicationProtocol::ReplicaExists())
    {
        LOG_F(ERROR, "%s", "Relation already exists in DBService. "
            "Updates are not supported (yet).");
        traceWriter->write("Relation already exists in DBService");
        return false;
    }
    if(receivedLine != CommunicationProtocol::LocationRequest())
    {
        LOG_F(ERROR, "%s", "Did not receive expected LocationRequest keyword.");
        traceWriter->write("Did not receive expected LocationRequest keyword.");
        return false;
    }

    string originalLocation;
    getLocationParameter(originalLocation, "SecondoHost");
    sendBuffer.push(originalLocation);
    getLocationParameter(originalLocation, "SecondoPort");
    sendBuffer.push(originalLocation);
    getLocationParameter(originalLocation, "SecondoHome");
    sendBuffer.push(originalLocation);

    string transferPort;
    SecondoUtilsLocal::readFromConfigFile(transferPort,
                                       "DBService",
                                       "FileTransferPort",
                                       "");
    sendBuffer.push(transferPort);
    CommunicationUtils::sendBatch(io, sendBuffer);

    traceWriter->write("sent original location details");

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::ReplicationTriggered()))
    {
        LOG_F(ERROR, "%s", "Could not trigger the replication.");
        traceWriter->write("Could not trigger replication");
        return false;
    }
    return true;
}

bool CommunicationClient::giveStartingSignalForReplication(
        const string& databaseName,
        const string& relationName)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::triggerReplication");

    if(!connectionTargetIsDBServiceMaster())
    {
        LOG_F(ERROR, "%s", "Command must be send to the DBService Master!");
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to server (start() failed).");
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {

        LOG_F(ERROR, "%s", "Not connected to CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::StartingSignal());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {
        LOG_F(ERROR, "%s", "Did not receive expected RelationRequest keyword.");
        traceWriter->write("Did not receive expected RelationRequest keyword");
        return false;
    }
    CommunicationUtils::sendLine(io,
            RelationInfo::getIdentifier(databaseName, relationName));
    return true;
}

int CommunicationClient::triggerFileTransfer(const string& transferServerHost,
                                             const string& transferServerPort,
                                             const string& databaseName,
                                             const string& relationName)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::triggerFileTransfer");

    LOG_F(INFO, "{ TransferServer: {host: %s, port: %s}, "
        "db: %s, relation: %s }",
        transferServerHost.c_str(),
        transferServerPort.c_str(),
        databaseName.c_str(),
        relationName.c_str()
    );

    traceWriter->write("transferServerHost", transferServerHost);
    traceWriter->write("transferServerPort", transferServerPort);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer");
        return 1;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerFileTransfer());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::ReplicationDetailsRequest()))
    {
        LOG_F(ERROR, "Did not receive expected ReplicationDetailsRequest "
            "keyword");

        traceWriter->write(
                "Did not receive expected ReplicationDetailsRequest keyword");
        return 2;
    }
    sendBuffer.push(transferServerHost);
    sendBuffer.push(transferServerPort);
    sendBuffer.push(
            ReplicationUtils::getFileName(
                    *(const_cast<string*>(&databaseName)),
                    *(const_cast<string*>(&relationName))));
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    LOG_F(INFO, "%s", "File transfer details sent to worker.");
    traceWriter->write("File transfer details sent to worker");
    return 0;
}

void CommunicationClient::getLocationParameter(
        string& location, const char* key)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::getLocationParameter");
    SecondoUtilsLocal::readFromConfigFile(location,
                                       "Environment",
                                       key,
                                       "");
}

bool CommunicationClient::getReplicaLocation(const string& databaseName,
                                             const string& relationName,
                                             const vector<string>& otherObjects,
                                             string& host,
                                             string& transferPort,
                                             string& commPort)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::getReplicaLocation");
    LOG_F(INFO, "Database: %s, Relation: %s", 
        databaseName.c_str(), relationName.c_str());

    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::ReplicaLocationRequest());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {
        
        LOG_F(ERROR, "%s", "Did not receive expected RelationRequest.");
        traceWriter->write("Did not receive expected RelationRequest");
        return false;
    }

    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    sendBuffer.push(stringutils::int2str(otherObjects.size()));
    for( auto s: otherObjects)
    {
      sendBuffer.push(s);
    }    

    CommunicationUtils::sendBatch(io, sendBuffer);

    LOG_F(INFO, "%s", "Sent relation details to DBService master.");
    traceWriter->write("Sent relation details to DBService master");

    queue<string> receivedLines;
    CommunicationUtils::receiveLines(io, 3, receivedLines);
    host = receivedLines.front();
    receivedLines.pop();
    transferPort = receivedLines.front();
    receivedLines.pop();
    commPort = receivedLines.front();
    receivedLines.pop();

    traceWriter->write("host", host);
    traceWriter->write("transferPort", transferPort);
    
    LOG_F(INFO, "Host: %s, TransferPort: %s", 
        host.c_str(), transferPort.c_str());
    

    if(!host.empty() && host != CommunicationProtocol::None()
    && !transferPort.empty() && transferPort != CommunicationProtocol::None()
    && !commPort.empty() && commPort != CommunicationProtocol::None())
    {
        return true;
    }
    return false;
}

bool CommunicationClient::reportSuccessfulReplication(
        const string& databaseName,
        const string& relationName)
{
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Notifying the DBS-M about the successful replication of "
        "relation: %s in db: %s...", 
        relationName.c_str(), databaseName.c_str());

    traceWriter->writeFunction(
            "CommunicationClient::reportSuccessfulReplication");

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server.");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer");

        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::ReplicationSuccessful());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {        
        LOG_F(ERROR, "%s", "Expected RelationRequest.");
        traceWriter->write("Expected RelationRequest");
        return false;
    }

    CommunicationUtils::sendLine(io,
            RelationInfo::getIdentifier(databaseName, relationName));

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
        LOG_F(ERROR, "%s", "Expected LocationRequest.");
        traceWriter->write("Expected LocationRequest");
        return false;
    }

    string replicaLocation;
    getLocationParameter(replicaLocation, "SecondoHost");
    sendBuffer.push(replicaLocation);
    getLocationParameter(replicaLocation, "SecondoPort");
    sendBuffer.push(replicaLocation);
    CommunicationUtils::sendBatch(io, sendBuffer);

    LOG_F(INFO, "%s", "Successfully reported to the DBService.");
    return true;
}


bool CommunicationClient::reportSuccessfulDerivation(
        const string& objectId)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction(
            "CommunicationClient::reportSuccessfulDerivation");

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server.");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::CreateDerivateSuccessful());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::ObjectRequest()))
    {
        LOG_F(ERROR, "%s", "Expected ObjectRequest.");
        traceWriter->write("Expected ObjectRequest");
        return false;
    }

    CommunicationUtils::sendLine(io, objectId);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
        LOG_F(ERROR, "%s", "Expected LocationRequest.");
        traceWriter->write("Expected LocationRequest");
        return false;
    }

    string replicaLocation;
    getLocationParameter(replicaLocation, "SecondoHost");
    sendBuffer.push(replicaLocation);
    getLocationParameter(replicaLocation, "SecondoPort");
    sendBuffer.push(replicaLocation);
    CommunicationUtils::sendBatch(io, sendBuffer);
    return true;
}


bool CommunicationClient::requestReplicaDeletion(
        const string& databaseName,
        const string& relationName,
        const string& derivateName)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::requestReplicaDeletion");

    LOG_F(INFO, "DeletionRequest: {db: %s, relation: %s, derivative: %s}",
        databaseName.c_str(), relationName.c_str(), derivateName.c_str());

    traceWriter->write("databaseName: ", databaseName);
    traceWriter->write("relationName: ", relationName);
    traceWriter->write("derivateName: ", derivateName);

    if(!connectionTargetIsDBServiceMaster())
    {
        LOG_F(ERROR, "%s", "Aborting due to wrong node specification");
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::DeleteReplicaRequest());
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    sendBuffer.push(derivateName);
    CommunicationUtils::sendBatch(io, sendBuffer);
    return true;
}

bool CommunicationClient::triggerReplicaDeletion(
        const string& databaseName,
        const string& relationName,
        const string& derivateName)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::triggerReplicaDeletion");

    LOG_F(INFO, "TriggerReplication: {db: %s, relation: %s, derivative: %s}",
        databaseName.c_str(), relationName.c_str(), derivateName.c_str());

    traceWriter->write("database", databaseName);
    traceWriter->write("relation", relationName);
    traceWriter->write("derivate", derivateName);

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server.");
        traceWriter->write("Could not connect to Server.");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer.");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerReplicaDeletion());
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    sendBuffer.push(derivateName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    return true;
}

bool CommunicationClient::connectionTargetIsDBServiceMaster()
{
    LOG_SCOPE_FUNCTION(INFO);
    string host;
    string commPort;
    if(!SecondoUtilsLocal::lookupDBServiceLocation(
                host, commPort))
    {
        LOG_F(ERROR, "%s", "DBService is not configured. "
            "Please adapt the Secondo config file.");

        traceWriter->write("DBService not configured");
        return false;
    }
    if(host.compare(server) != 0)
    {
        LOG_F(ERROR, "%s", "Host does not match with DBService host.");
        traceWriter->write("host does not match with DBService host");
        return false;
    }
    if(atoi(commPort.c_str()) != port)
    {
        LOG_F(ERROR, "%s", "Port does not match with DBService commPort");
        traceWriter->write("port does not match with DBService commPort");
        return false;
    }

    return true;
}

bool CommunicationClient::pingDBService()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::pingDBService");

    if(!connectionTargetIsDBServiceMaster())
    {
        LOG_F(ERROR, "%s", "Aborting due to wrong node specification");
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer.");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::Ping());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::Ping()))
    {
        LOG_F(ERROR, "%s", "Did not receive the expected PING.");
        traceWriter->write("Did not receive ping");
        return false;
    }
    return true;
}

bool CommunicationClient::getRelType(
        const string& relID,
        string& nestedListAsString)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::getRelType");
    traceWriter->write("relID", relID);
    LOG_F(INFO, "relID: %s", relID.c_str());

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::RelTypeRequest());
    sendBuffer.push(relID);
    CommunicationUtils::sendBatch(io, sendBuffer);
    traceWriter->write("Sent relID");

    LOG_F(INFO, "%s", "Sent reldID.");

    CommunicationUtils::receiveLine(io, nestedListAsString);
    if(nestedListAsString != CommunicationProtocol::None())
    {
        LOG_F(INFO,"Relation type: %s", nestedListAsString.c_str());
        traceWriter->write("Rel type", nestedListAsString);
        return true;
    }

    LOG_F(ERROR, "%s", "The Relation does not exist in DBService. Aborting.");

    traceWriter->write("Relation does not exist in DBService");
    return false;
}


bool CommunicationClient::getDerivedType(
        const string& relID,
        const string& derivedName,
        string& nestedListAsString)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::getDerivedType");
    traceWriter->write("relID", relID);
    traceWriter->write("derivedName", derivedName);

    LOG_F(INFO, "relID: %s, derivative: %s",
        relID.c_str(), derivedName.c_str());

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::DerivedTypeRequest());
    sendBuffer.push(relID);
    sendBuffer.push(derivedName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    LOG_F(INFO, "%s", "Sent relID and derivative name.");
    traceWriter->write("Sent relID and derivative name");

    CommunicationUtils::receiveLine(io, nestedListAsString);
    if(nestedListAsString != CommunicationProtocol::None())
    {
        traceWriter->write("Derived type", nestedListAsString);
        LOG_F(INFO, "Derived type: %s", nestedListAsString.c_str());
        return true;
    }
    LOG_F(INFO, "%s", "The derivation does not exist.");
    traceWriter->write("Derived object does not exist in DBService");
    return false;
}



bool CommunicationClient::triggerDerivation(const string& databaseName,
                                            const string& targetName,
                                            const string& relName,
                                            const string& fundef)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::triggerDerivation");

    if(!connectionTargetIsDBServiceMaster())
    {
        LOG_F(ERROR, "%s", "Aborting due to wrong node specification");
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerDerivation());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::DerivationRequest()))
    {
        LOG_F(ERROR, "%s", "Did not receive expected DerivationRequest "
            "keyword");

        traceWriter->write("Did not receive expected DerivationRequest "
                           "keyword");
        return false;
    }
    sendBuffer.push(databaseName);
    sendBuffer.push(targetName);
    sendBuffer.push(relName);
    // remove line ends from function definition
    string fundef1 = stringutils::replaceAll(fundef,"\n","");
    sendBuffer.push(fundef1);
    CommunicationUtils::sendBatch(io, sendBuffer);

    string receivedLine;
    CommunicationUtils::receiveLine(io, receivedLine);

    if(receivedLine == CommunicationProtocol::ObjectExists())
    {
        LOG_F(ERROR, "%s", "Object already exists in DBService.");
        traceWriter->write("Object already exists in DBService");

        return false;
    }
    if(receivedLine == CommunicationProtocol::RelationNotExists()){
        LOG_F(ERROR, "%s", "The given relation does not exists in DBService.");
        traceWriter->write("The given relation does not exists in DBService.");
        return false;
    }
    if(receivedLine != CommunicationProtocol::DerivationTriggered())
    {
        LOG_F(ERROR, "%s", "Did not receive expected DerivationTriggered"
            " keyword");
        traceWriter->write("Did not receive expected DerivationTriggered"
                           " keyword");
        return false;
    }

    return true;
}


bool CommunicationClient::createDerivation(const string& databaseName,
                                            const string& targetName,
                                            const string& relName,
                                            const string& fundef)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::createDerivation");

    if(start() != 0)
    {

        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::CreateDerivation());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::DerivationRequest()))
    {
        LOG_F(ERROR, "%s", "Did not receive expected DerivationRequest "
            "keyword");
        traceWriter->write("Did not receive expected DerivationRequest "
                           "keyword");
        return false;
    }
    sendBuffer.push(databaseName);
    sendBuffer.push(targetName);
    sendBuffer.push(relName);
    // remove line ends from function definition
    string fundef1 = stringutils::replaceAll(fundef,"\n","");
    sendBuffer.push(fundef1);
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CreateDerivateSuccessful()))
    {
        LOG_F(ERROR, "%s", "The creation of the derivative failed.");
        traceWriter->write("Creation of drivate failed ");
        return false;
    }

    return true;
}

bool CommunicationClient::addNode(
    const std::string& nodeHost, const int& nodePort,
    const std::string& pathToNodeConfig) {

    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("CommunicationClient::addNode");
    
    if(!connectionTargetIsDBServiceMaster())
    {
        LOG_F(ERROR, "%s", "Aborting due to wrong node specification");
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        LOG_F(ERROR, "%s", "Could not connect to Server");
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
        CommunicationProtocol::CommunicationServer()))
    {
        LOG_F(ERROR, "%s", "Not connected to CommunicationServer");
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::AddNodeRequest());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
        CommunicationProtocol::AddNodeRequest()))
    {
        LOG_F(ERROR, "%s", "Did not receive expected AddNodeRequest keyword");
        traceWriter->write("Did not receive expected AddNodeRequest keyword");
        return false;
    }

    //TODO Look at this function. UP to this point nothing really happened!
    sendBuffer.push(nodeHost);
    sendBuffer.push(to_string(nodePort));
    sendBuffer.push(pathToNodeConfig);

    CommunicationUtils::sendBatch(io, sendBuffer);

    if(CommunicationUtils::receivedExpectedLine(io,
        CommunicationProtocol::NodeAdded())) {

        LOG_F(INFO, "%s", "The node has been added successfully.");
        traceWriter->write("The node has been added successfully.");

        return true;

    } else {

        LOG_F(ERROR, "%s", "Adding of the node has failed.");
        traceWriter->write("Adding of the node has failed.");

        return false;
    }

    return false;
}



} /* namespace DBService */
