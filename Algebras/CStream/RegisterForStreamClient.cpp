/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of class RegisterForStreamClient.

[toc]

1 RegisterForStreamClient implementation

*/

#include <iosfwd>
#include <fstream>
#include <string>

#include "Algebras/Distributed2/FileTransferKeywords.h"
#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationUtils.hpp"
#include "RegisterForStreamClient.h"
#include "ProvideTupleTypesProtocol.h"
#include "DistributeStreamProtocol.h"
#include "VTHelpers.h"
#include "TupleDescr.h"
#include "ListUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

using namespace std;

namespace cstream
{

/*
1.1 Constructor

Creates a new object of the RegisterForStreamClient with an existing 
connection, a requested TupleTypeDescr and filterfunction.

*/
    
RegisterForStreamClient::RegisterForStreamClient(string server, 
            int port,
            TupleDescr* askedTypeTupleDescr, 
            string funfilterdescr) :
        Client(server, port, 0),_askedTypeTupleDescr(askedTypeTupleDescr),
        _funfilterdescr(funfilterdescr) {
    _tupleType = nl->OneElemList(SecondoSystem::GetCatalog()->NumericType(
                    nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                        _askedTypeTupleDescr->GetTupleTypeExpr())));
}

/*
1.2 Constructor

*/
RegisterForStreamClient::~RegisterForStreamClient() {
    _askedTypeTupleDescr->DeleteIfAllowed();
}

/*
1.3 Function Definitions

The functions provided by the RegisterForStreamClient class are explained
below.

1.3.1 connectionOK

This function checks if connection to server exists for sendig and receiving sting messages.

*/
        
bool RegisterForStreamClient::connectionOK() {
    if(!socket)  {
        return false;
    }
    if(!socket->IsOk()) {
        return false;
    }
    if(!streamOK()) {
        return false;
    }

    return true;
}

/*
1.3.2 streamOK

This function checks the state of socket.

*/
bool RegisterForStreamClient::streamOK() {

    if(!_io)
        return false;

    try{
        if(!_io->good())
        {
            LOG << " eof()=" << _io->eof();
            LOG << " fail()=" << _io->fail();
            LOG << " bad()=" << _io->bad();
            return false;
        }
        return true;
    }catch(...)
    {
        LOG << "caught exception while trying to use stream" << ENDL;
        return false;
    }
}

/*
1.3.3 start


This function starts communication with server

*/
int RegisterForStreamClient::start() {
    socket = Socket::Connect(server, stringutils::int2str(port),
        Socket::SockGlobalDomain, 3, 1);
    
    if(!socket)
        return 8;

    if(!socket->IsOk())
        return 9;
    
    _io = &(socket->GetSocketStream());

    if(!streamOK())
        return 7;

    if(!requestSupportedType())
        return 5;

    if(!sendRequestRegisterForStream())
        return 6;

    return 0;
}

/*
1.3.4 requestSupportedType

This function checks supported formats:binary and nested list 

*/

bool RegisterForStreamClient::requestSupportedType() {
    if(!connectionOK()) {
        return false;
    }
    LOG << " requestSupportedType: RequestSupportedTypes() " << ENDL;
    DBService::CommunicationUtils::sendLine(*_io,
        DistributeStreamProtocol::requestSupportedTypes());
    
    LOG << DistributeStreamProtocol::requestSupportedTypes() << ENDL;

    string supportedType;
    DBService::CommunicationUtils::receiveLine(*_io, supportedType);

    if(supportedType == DistributeStreamProtocol::sendSupportedTypes(true)) {
        LOG << " binary Format supported " << ENDL;
        _messageType = true;
        return true;
    }
    if(supportedType == DistributeStreamProtocol::sendSupportedTypes(false)) {
       LOG << " nested list format supported " << ENDL;
        _messageType = false;
        return true;
    }
    
    return false;
}

/*
1.3.5 sendRequestRegisterForStream

This function builds request string message to send to server 

*/
 
bool RegisterForStreamClient::sendRequestRegisterForStream() {
    if(!connectionOK()) {
        return false;
    }
    LOG << " sendRequestRegisterForStream: connectionOK " << ENDL;

    if(_messageType) {
        DBService::CommunicationUtils::sendLine(*_io,
            DistributeStreamProtocol::requestStream() + "(" + 
            _askedTypeTupleDescr->GetString() + "," 
            + _funfilterdescr + "," + "true" + ")" );
        
        LOG << DistributeStreamProtocol::requestStream() + "(" + 
            _askedTypeTupleDescr->GetString() + "," 
            + _funfilterdescr + "," + "true" + ")"<< ENDL;
    }
    else {
        DBService::CommunicationUtils::sendLine(*_io,
            DistributeStreamProtocol::requestStream() + "(" + 
            _askedTypeTupleDescr->GetString() + "," 
            + _funfilterdescr + "," + "false" + ")" );
        
        LOG << DistributeStreamProtocol::requestStream() + "(" + 
            _askedTypeTupleDescr->GetString() + "," 
            + _funfilterdescr + "," + "false" + ")"<< ENDL;
    }    

    if(DBService::CommunicationUtils::receivedExpectedLine(*_io,
        DistributeStreamProtocol::confirmStreamOK())){
        LOG << " receiveTupleDescr: confirmStreamOK " << ENDL;
    } else {
       LOG << "receiveTupleDescr:confirmStream is not OK" << ENDL;
        return false; 
    }
    
    return true;
}

/*
1.3.6 receiveTuple

This function receives tuples from server 

*/

Tuple* RegisterForStreamClient::receiveTuple() {
    if(!connectionOK())
        return 0;
    
    string tupleMessage;
    DBService::CommunicationUtils::receiveLine(*_io, tupleMessage);

    LOG << tupleMessage << ENDL;
    
    if(tupleMessage.compare(DistributeStreamProtocol::streamDone()) == 0){
        LOG << "End of stream. No more Tuple." << ENDL;
        return 0;
    }
    
    if( tupleMessage.length() < 
            DistributeStreamProtocol::tupleMessage().length() + 2 ) {
        return 0;
    }

    if(!(tupleMessage.substr(0, 
        DistributeStreamProtocol::tupleMessage().length()+1)
        == DistributeStreamProtocol::tupleMessage() + "(")
    || !(tupleMessage.substr(tupleMessage.length()-1, 1) == ")")) {
        return 0;
    }

    Tuple* tuple;

    if(_messageType) {
        tupleMessage.erase(0, 
            DistributeStreamProtocol::tupleMessage().length()+1);
        tupleMessage.erase(tupleMessage.length()-1, 1);

        tuple = new Tuple(_askedTypeTupleDescr->CreateTupleType());
        tuple->ReadFromBinStr(0,tupleMessage);
    }
    else {
        tupleMessage.erase(0, 
            DistributeStreamProtocol::tupleMessage().length());

        ListExpr value;
        if(!nl->ReadFromString(tupleMessage, value))
            return 0;
        
        ListExpr errorInfo;
        bool correct = false;
        tuple = Tuple::In( _tupleType, value, 0,
                  errorInfo, correct );
        if(!correct) {
            return 0;
        }
    }

    return tuple;
}

}
 
