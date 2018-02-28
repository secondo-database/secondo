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

[1] Implementation of RequestTupleTypesClient.

[toc]

1 RequestTupleTypesClient class implementation
see RequestTupleTypesClient.cpp for details.

*/

#ifndef _REQUEST_TUPLE_TYPES_CLIENT_H_
#define _REQUEST_TUPLE_TYPES_CLIENT_H_

#include <string>
#include <iostream>
#include "Client.h"
#include "TupleDescr.h"

namespace cstream
{
    class RequestTupleTypesClient : public Client 
    {

    public:
        RequestTupleTypesClient(std::string server, int port,
            int maxNumberTypeTupleDescr);

        virtual int start();

        bool connectionOK();
        bool sendRequestTupleTypes();
        TupleDescr* receiveTupleDescr();

    private:
        
/*
2 Member Definitions

2.1 streamOK

Checks the state of socket

*/
        bool streamOK();
        
/*

2.2 maxNumberTypeTupleDescr

Stores the number of requested types of TupleDescr

*/  

        int _maxNumberTypeTupleDescr;
        
/*

2.3 io

Stores socketstream

*/  
        std::iostream* _io;

    };

}

#endif // _REQUEST_TUPLE_TYPES_CLIENT_H_
