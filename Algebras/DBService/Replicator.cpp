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
#include <sstream>
#include <vector>

#include "SecParser.h"
#include "Algebra.h"
#include "ConnectionInfo.h"
#include "CommandLog.h"

#include "Replicator.hpp"
#include "DBServiceManager.hpp"
#include "DebugOutput.hpp"

//#include <boost/thread.hpp>

using namespace std;
using namespace distributed2;

namespace DBService
{

Replicator::Replicator(string ext) : fileExtension(ext)
{}

Replicator::~Replicator()
{
    // TODO Auto-generated destructor stub
}

std::string& Replicator::getFileExtension()
{
    return fileExtension;
}

void Replicator::replicateRelation(const RelationInfo& relationInfo) const
{
    createFileOnCurrentNode(relationInfo.getRelationName());
    runReplication(relationInfo);
}

void Replicator::createFileOnCurrentNode(const std::string& relationName) const
{
    stringstream query;
    query << "query saveObjectToFile " << relationName << "[\"" << relationName
            << "." << fileExtension << "\"]";
    SecParser secondoParser;
    string queryAsNestedList;
    if (secondoParser.Text2List(query.str(), queryAsNestedList) != 0)
    {
        // TODO
    } else
    {
        Word result;
        QueryProcessor::ExecuteQuery(queryAsNestedList, result, 1024);
    }
}

void Replicator::runReplication(const RelationInfo& relationInfo) const
{
    string fileName = relationInfo.getRelationName() + "." + fileExtension;
    stringstream sendFileToRemoteServerCommand;
    sendFileToRemoteServerCommand << "query sendFile(0, '" << fileName
            << "', \"" << fileName << "\")";
    cout << sendFileToRemoteServerCommand.str() << endl;
    for (vector<ConnectionID>::const_iterator i = relationInfo.nodesBegin();
            i != relationInfo.nodesEnd(); ++i)
    {
        stringstream createObjectFromFileCommand;
        ConnectionInfo* connection = DBServiceManager::getConnection(*i);
        createObjectFromFileCommand << "let " << relationInfo.getRelationName()
                << " =  '" << connection->getSendFolder() << fileName
                << "' getObjectFromFile consume";

        cout << createObjectFromFileCommand.str() << endl;

        // TODO
        //boost::thread replicationThread;

        int errorCode;
        string errorMessage;
        string result;
        double runtime;
        CommandLog commandLog;
        connection->simpleCommand(sendFileToRemoteServerCommand.str(),
                                  errorCode, errorMessage, result, false,
                                  runtime, false, false, commandLog);
        connection->simpleCommand(createObjectFromFileCommand.str(), errorCode,
                                  errorMessage, result, false, runtime, false,
                                  false, commandLog);
    }
}

} /* namespace DBService */
