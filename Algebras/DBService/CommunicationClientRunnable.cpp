/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#include "CommunicationClientRunnable.hpp"

#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"

using namespace std;

namespace DBService {

CommunicationClientRunnable::CommunicationClientRunnable(string sourceHost,
                               int sourceTransferPort,
                               string targetHost,
                               int targetCommPort,
                               std::string localFileName,
                               std::string databaseName,
                               std::string relationName)
:sourceHost(sourceHost), sourceTransferPort(sourceTransferPort),
 targetHost(targetHost), targetCommPort(targetCommPort),
 localFileName(localFileName),
 databaseName(databaseName), relationName(relationName)
{}

CommunicationClientRunnable::~CommunicationClientRunnable()
{}

void CommunicationClientRunnable::run()
{
    if(runner){
        runner->join();
        delete runner;
    }
    runner = new boost::thread(
            &CommunicationClientRunnable::createClient, this);
}
void CommunicationClientRunnable::createClient()
{
    CommunicationClient client(targetHost, targetCommPort, 0);
    client.start();
    client.triggerFileTransfer(sourceHost,
                               stringutils::int2str(sourceTransferPort),
                               localFileName,
                               databaseName,
                               relationName);
}

} /* namespace DBService */
