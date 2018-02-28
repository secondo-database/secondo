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

[1] Implementation of class RequestTupleTypesClient.

[toc]

1 RequestTupleTypesClient implementation

2 Includes

*/

#include <iosfwd>
#include <fstream>
#include <string>

#include "Algebras/Distributed2/FileTransferKeywords.h"
#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationUtils.hpp"
#include "RequestTupleTypesClient.h"
#include "ProvideTupleTypesProtocol.h"
#include "DistributeStreamProtocol.h"
#include "VTHelpers.h"
#include "TupleDescr.h"

using namespace std;

namespace cstream
{
/*
1.1 Constructor

Creates a new object of the RequestTupleTypesClient with an existing 
connection, a requested number of TupleDescr types.

*/
RequestTupleTypesClient::RequestTupleTypesClient(string server, int port,
        int maxNumberTypeTupleDescr) :
        Client(server, port, 0),
        _maxNumberTypeTupleDescr(maxNumberTypeTupleDescr) {
}

/*
1.2 Function Definitions

The functions provided by the RequestTupleTypesClient class are explained
below.

1.2.1 connectionOK

This function checks if connection to server exists for sendig and receiving sting messages.

*/
bool RequestTupleTypesClient::connectionOK() {
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

1.2.2 streamOK


This function checks the state of socket.

*/
bool RequestTupleTypesClient::streamOK() {

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

1.2.3 start


This function starts communication with server

*/
int RequestTupleTypesClient::start() {
    socket = Socket::Connect(server, stringutils::int2str(port),
        Socket::SockGlobalDomain, 3, 1);
    
    if(!socket)
        return 8;

    if(!socket->IsOk())
        return 9;
    
    _io = &(socket->GetSocketStream());

    if(!streamOK())
        return 7;

    if(!sendRequestTupleTypes())
        return 6;

    return 0;
}

/*

1.2.4 sendRequestTupleTypes


This function builds request string message to send to server 

*/
bool RequestTupleTypesClient::sendRequestTupleTypes() {
    if(!connectionOK()) {
        return false;
    }

    DBService::CommunicationUtils::sendLine(*_io,
        ProvideTupleTypesProtocol::requestTupleTypes() + "(" + 
        to_string(_maxNumberTypeTupleDescr) + ")");
    return true;
}

/*

1.2.5 receiveTupleDescr


This function receives TupleDescr from server 

*/
TupleDescr* RequestTupleTypesClient::receiveTupleDescr() {
    if(!connectionOK())
        return 0;

    string tupdescrline;
    DBService::CommunicationUtils::receiveLine(*_io, tupdescrline);
    LOG << tupdescrline << ENDL;
    if(tupdescrline==ProvideTupleTypesProtocol::requestDone())
        return 0;

    return new TupleDescr(tupdescrline);
}

}
