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
#include "Algebras/DBService/CommunicationProtocol.hpp"

using namespace std;

namespace DBService {

string CommunicationProtocol::CommunicationServer()
{
    return "<DBSERVICECOMMSERV>";
}

string CommunicationProtocol::CommunicationClient()
{
    return "<DBSERVICECOMMCLI>";
}

string CommunicationProtocol::ReplicationCanceled()
{
    return "<CANCELED>";
}

string CommunicationProtocol::ReplicationTriggered()
{
    return "<TRIGGERED>";
}

string CommunicationProtocol::RelationRequest()
{
    return "<RELATIONREQUEST>";
}

string CommunicationProtocol::LocationRequest()
{
    return "<LOCATIONREQUEST>";
}

string CommunicationProtocol::ReplicaLocationRequest()
{
    return "<REPLICALOCATIONREQUEST>";
}

string CommunicationProtocol::ReplicationServer()
{
    return "<REPLICATIONSERVER>";
}

string CommunicationProtocol::ReplicationClient()
{
    return "<REPLICATIONCLIENT>";
}

string CommunicationProtocol::ReplicationClientRequest()
{
    return "<REPLICATIONCLIENTREQUEST>";
}

string CommunicationProtocol::TriggerReplication()
{
    return "<TRIGGERREPLICATION>";
}

string CommunicationProtocol::TriggerFileTransfer()
{
    return "<TRIGGERRFILETRANSFER>";
}

string CommunicationProtocol::ReplicationDetailsRequest()
{
    return "<REPLICATIONDETAILSREQUEST>";
}

string CommunicationProtocol::SendReplicaForStorage()
{
    return "<SENDREPLICAFORSTORAGE>";
}
string CommunicationProtocol::SendReplicaForUsage()
{
    return "<SENDREPLICAFORUSAGE>";
}

string CommunicationProtocol::FunctionRequest()
{
    return "<FUNCTIONREQUEST>";
}

string CommunicationProtocol::None()
{
    return "<NONE>";
}

string CommunicationProtocol::ReplicationSuccessful()
{
    return "<REPLICATIONSUCCESSFUL>";
}

string CommunicationProtocol::DeleteReplicaRequest()
{
    return "<DELETEREPLICAREQUEST>";
}

string CommunicationProtocol::TriggerReplicaDeletion()
{
    return "<TRIGGERREPLICADELETION>";
}

string CommunicationProtocol::ReplicaExists()
{
    return "<REPLICAEXISTS>";
}

string CommunicationProtocol::StartingSignal()
{
    return "<STARTINGSIGNAL>";
}

string CommunicationProtocol::Ping()
{
    return "<PING>";
}

string CommunicationProtocol::RelTypeRequest()
{
    return "<RELTYPEREQUEST>";
}

string CommunicationProtocol::FileName()
{
    return "<FILENAME>";
}


string CommunicationProtocol::TriggerDerivation()
{
    return "<TRIGGERDERIVATION>";
}

string CommunicationProtocol::DerivationRequest()
{
   return "<DERIVATIONREQUEST>";
}

string CommunicationProtocol::ObjectExists()
{
  return "<OBJECTEXISTS>";
}

string CommunicationProtocol::RelationNotExists() 
{
  return "<RELATIONNOTEXISTS>";
}

string CommunicationProtocol::DerivationTriggered()
{
  return "<DERIVATIONTRIGGERED>";
}

string CommunicationProtocol::CreateDerivation()
{
  return "<CREATEDERIVATION>";
}

} /* namespace DBService */
