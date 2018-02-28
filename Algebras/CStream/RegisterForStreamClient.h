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

[1] Implementation of RegisterForStreamClient.

[toc]

1 RegisterForStreamClient class implementation
see RegisterForStreamClient.cpp for details.

*/

#ifndef _REGISTER_FOR_STREAM_CLIENT_H_
#define _REGISTER_FOR_STREAM_CLIENT_H_

#include <string>
#include <iostream>
#include "Client.h"
#include "TupleDescr.h"

namespace cstream
{
    class RegisterForStreamClient : public Client 
    {

    public:
        
        RegisterForStreamClient(std::string server, 
            int port,
            TupleDescr* askedTypeTupleDescr, 
            std::string funfilterdescr);

        ~RegisterForStreamClient();
       
        //begins communication with server
        virtual int start();

        bool connectionOK();
        bool requestSupportedType();
        bool sendRequestRegisterForStream();
        Tuple* receiveTuple();

    private:
/*
2 Member Definitions

2.1 streamOK

Checks the state of socket

*/
        bool streamOK();
/*
2.2 io

Stores socketstream

*/
        std::iostream* _io;
/*
2.3 askedTypeTupleDescr

Stores type of requested TupleDescr

*/
        TupleDescr* _askedTypeTupleDescr;
/*

2.4 funfilterdescr

Stores function to filter TupleDescr

*/
        std::string _funfilterdescr;
/*
2.5 messageType

Stores types of supported formates
true - binary formate, false - nested list format

*/
        bool _messageType;
/*
2.6 tupleType

Stores the requested type in nested list formats

*/
        ListExpr _tupleType;
    };

}

#endif // _REGISTER_FOR_STREAM_CLIENT_H_
