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
#include <iostream>

#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"

using namespace std;
using namespace distributed2;

namespace DBService {

CommunicationClient::CommunicationClient(
        string& server, int port, Socket* socket)
:Client(server, port, socket)
{
    string context("CommunicationClient");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context, port, std::cout));

    traceWriter->writeFunction("CommunicationClient::CommunicationClient");
    traceWriter->write("Connecting to server: ", server);
    traceWriter->write("On port:", port);
}

CommunicationClient::~CommunicationClient()
{
    traceWriter->writeFunction("CommunicationClient::~CommunicationClient");
}

int CommunicationClient::start()
{
    traceWriter->writeFunction("CommunicationClient::start");
    socket = Socket::Connect(server, stringutils::int2str(port),
                Socket::SockGlobalDomain, 3, 1);
        if (!socket) {
            traceWriter->write("socket initialization failed");
            return 8;
        }
        if (!socket->IsOk()) {
            traceWriter->write("socket not ok");
            return 9;
        }
        return 0;
}

bool CommunicationClient::triggerReplication(const string& databaseName,
                                            const string& relationName)
{
    traceWriter->writeFunction("CommunicationClient::triggerReplication");

    if(!connectionTargetIsDBServiceMaster())
    {
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
        traceWriter->write("Did not receive expected RelationRequest keyword");
        return false;
    }
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    string receivedLine;
    CommunicationUtils::receiveLine(io, receivedLine);

    if(receivedLine == CommunicationProtocol::ReplicaExists())
    {
        traceWriter->write("Relation already exists in DBService");
        return false;
    }
    if(receivedLine != CommunicationProtocol::LocationRequest())
    {
        traceWriter->write("Did not receive expected LocationRequest keyword");
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
        traceWriter->write("Could not trigger replication");
        return false;
    }
    return true;
}

bool CommunicationClient::giveStartingSignalForReplication(
        const string& databaseName,
        const string& relationName)
{
    traceWriter->writeFunction("CommunicationClient::triggerReplication");

    if(!connectionTargetIsDBServiceMaster())
    {
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
    traceWriter->writeFunction("CommunicationClient::triggerFileTransfer");
    traceWriter->write("transferServerHost", transferServerHost);
    traceWriter->write("transferServerPort", transferServerPort);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
    traceWriter->write("File transfer details sent to worker");
    return 0;
}

void CommunicationClient::getLocationParameter(
        string& location, const char* key)
{
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
    traceWriter->writeFunction("CommunicationClient::getReplicaLocation");
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
    traceWriter->writeFunction(
            "CommunicationClient::reportSuccessfulReplication");

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
        traceWriter->write("Expected RelationRequest");
        return false;
    }

    CommunicationUtils::sendLine(io,
            RelationInfo::getIdentifier(databaseName, relationName));

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
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


bool CommunicationClient::reportSuccessfulDerivation(
        const string& objectId)
{
    traceWriter->writeFunction(
            "CommunicationClient::reportSuccessfulDerivation");

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
        traceWriter->write("Expected ObjectRequest");
        return false;
    }

    CommunicationUtils::sendLine(io, objectId);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
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
    traceWriter->writeFunction("CommunicationClient::requestReplicaDeletion");
    traceWriter->write("databaseName: ", databaseName);
    traceWriter->write("relationName: ", relationName);
    traceWriter->write("derivateName: ", derivateName);

    if(!connectionTargetIsDBServiceMaster())
    {
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
    traceWriter->writeFunction("CommunicationClient::triggerReplicaDeletion");
    traceWriter->write("database", databaseName);
    traceWriter->write("relation", relationName);
    traceWriter->write("derivate", derivateName);

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
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
    string host;
    string commPort;
    if(!SecondoUtilsLocal::lookupDBServiceLocation(
                host, commPort))
    {
        traceWriter->write("DBService not configured");
        return false;
    }
    if(host.compare(server) != 0)
    {
        traceWriter->write("host does not match with DBService host");
        return false;
    }
    if(atoi(commPort.c_str()) != port)
    {
        traceWriter->write("port does not match with DBService commPort");
        return false;
    }

    return true;
}

bool CommunicationClient::pingDBService()
{
    traceWriter->writeFunction("CommunicationClient::pingDBService");

    if(!connectionTargetIsDBServiceMaster())
    {
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
        traceWriter->write("Did not receive ping");
        return false;
    }
    return true;
}

bool CommunicationClient::getRelType(
        const string& relID,
        string& nestedListAsString)
{
    traceWriter->writeFunction("CommunicationClient::getRelType");
    traceWriter->write("relID", relID);

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::RelTypeRequest());
    sendBuffer.push(relID);
    CommunicationUtils::sendBatch(io, sendBuffer);
    traceWriter->write("Sent relID");

    CommunicationUtils::receiveLine(io, nestedListAsString);
    if(nestedListAsString != CommunicationProtocol::None())
    {
        traceWriter->write("Rel type", nestedListAsString);
        return true;
    }
    traceWriter->write("Relation does not exist in DBService");
    return false;
}


bool CommunicationClient::getDerivedType(
        const string& relID,
        const string& derivedName,
        string& nestedListAsString)
{
    traceWriter->writeFunction("CommunicationClient::getDerivedType");
    traceWriter->write("relID", relID);
    traceWriter->write("derivedName", derivedName);

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::DerivedTypeRequest());
    sendBuffer.push(relID);
    sendBuffer.push(derivedName);
    CommunicationUtils::sendBatch(io, sendBuffer);
    traceWriter->write("Sent relID and deriveName");

    CommunicationUtils::receiveLine(io, nestedListAsString);
    if(nestedListAsString != CommunicationProtocol::None())
    {
        traceWriter->write("Derived type", nestedListAsString);
        return true;
    }
    traceWriter->write("Derived object does not exist in DBService");
    return false;
}



bool CommunicationClient::triggerDerivation(const string& databaseName,
                                            const string& targetName,
                                            const string& relName,
                                            const string& fundef)
{
    traceWriter->writeFunction("CommunicationClient::triggerDerivation");

    if(!connectionTargetIsDBServiceMaster())
    {
        traceWriter->write("Aborting due to wrong node specification");
        return false;
    }

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
        traceWriter->write("Object already exists in DBService");
        return false;
    }
    if(receivedLine == CommunicationProtocol::RelationNotExists()){
        traceWriter->write("argument relation does not exists in DBService");
        return false;
    }
    if(receivedLine != CommunicationProtocol::DerivationTriggered())
    {
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
    traceWriter->writeFunction("CommunicationClient::createDerivation");

    if(start() != 0)
    {
        traceWriter->write("Could not connect to Server");
        return false;
    }
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
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
        traceWriter->write("Creation of drivate failed ");
        return false;
    }

    return true;
}



} /* namespace DBService */
