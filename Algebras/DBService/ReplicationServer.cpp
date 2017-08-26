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
#include <ctime>
#include <chrono>
#include <iostream>
#include <sstream>

#include "FileSystem.h"
#include "SocketIO.h"

#include "Algebras/Distributed2/FileTransferKeywords.h"
#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

ReplicationServer::ReplicationServer(int port) :
 MultiClientServer(port), FileTransferServer(port)
{
    string context("ReplicationServer");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context, port));
    traceWriter->writeFunction("ReplicationServer::ReplicationServer");
    traceWriter->write("port", port);
}

ReplicationServer::~ReplicationServer()
{
    traceWriter->writeFunction("ReplicationServer::~ReplicationServer");
}

int ReplicationServer::start()
{
    traceWriter->writeFunction("ReplicationServer::start");
    return MultiClientServer::start();
}

int ReplicationServer::communicate(iostream& io)
{
    const boost::thread::id tid = boost::this_thread::get_id();
    traceWriter->writeFunction(tid, "ReplicationServer::communicate");
    try
    {
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationServer());

        if(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ReplicationClient()))
        {
            traceWriter->write(tid, "not connected to ReplicationClient");
            return 1;
        }

        queue<string> receiveBuffer;
        CommunicationUtils::receiveLines(io, 2, receiveBuffer);
        string purpose = receiveBuffer.front();
        receiveBuffer.pop();
        string fileName = receiveBuffer.front();
        receiveBuffer.pop();

        if(!FileSystem::FileOrFolderExists(fileName))
        {
            traceWriter->write(tid, "file not found, notifying client");
            CommunicationUtils::sendLine(io,
                    distributed2::FileTransferKeywords::FileNotFound());
        }
        if(purpose == CommunicationProtocol::SendReplicaForStorage())
        {
            sendFileToClient(io, true, tid);
        }else if(purpose == CommunicationProtocol::SendReplicaForUsage())
        {
            CommunicationUtils::sendLine(io,
                    CommunicationProtocol::FunctionRequest());
            string function;
            CommunicationUtils::receiveLine(io, function);
            if(function == CommunicationProtocol::None())
            {
                sendFileToClient(io, true, tid);
            }else
            {
                CommunicationUtils::sendLine(io,
                        CommunicationProtocol::FileName());

                string replicaFileName;
                CommunicationUtils::receiveLine(io, replicaFileName);
                if(!FileSystem::FileOrFolderExists(replicaFileName))
                {
                    traceWriter->write(tid, "file not found, notifying client");
                    CommunicationUtils::sendLine(io,
                            distributed2::FileTransferKeywords::FileNotFound());
                    return 6;
                }

                CommunicationUtils::sendLine(io,
                        CommunicationProtocol::FileName());

                std::time_t currentTime = std::time(0);
                stringstream fileName;
                fileName << currentTime << "_" << replicaFileName;

                CommunicationUtils::sendLine(io,
                        fileName.str());

                applyFunctionAndCreateNewFile(
                        io, function, replicaFileName, fileName.str(), tid);
            }
            sendFileToClient(io, true, tid);
        }else
        {
            traceWriter->write(tid, "unexpected purpose: ", purpose);
            return 1;
        }
    } catch (...)
    {
        traceWriter->write(tid, "ReplicationServer: communication error");
        return 2;
    }
    return 0;
}

void ReplicationServer::sendFileToClient(
        iostream& io,
        bool fileCreated,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(tid, "ReplicationServer::sendFileToClient");
    // expected by receiveFile function of FileTransferClient,
    // but not sent by sendFile function of FileTransferServer
    CommunicationUtils::sendLine(io,
            distributed2::FileTransferKeywords::FileTransferServer());

    queue<string> expectedLines;
    expectedLines.push(
            distributed2::FileTransferKeywords::FileTransferClient());
    expectedLines.push(distributed2::FileTransferKeywords::SendFile());

    if(!CommunicationUtils::receivedExpectedLines(io, expectedLines))
    {
        traceWriter->write(tid,
                "communication error while initiating file transfer");
    }

    traceWriter->write(tid, "file created, sending file");

    std::chrono::steady_clock::time_point begin =
            std::chrono::steady_clock::now();
    int rc = sendFile(io);
    std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
    if(rc != 0)
    {
        traceWriter->write(tid, "send failed");
    }else
    {
        traceWriter->write(tid, "file sent");
        traceWriter->write("duration of send [microseconds]",
                std::chrono::duration_cast
                <std::chrono::microseconds>(end - begin).count());
    }
}

void ReplicationServer::applyFunctionAndCreateNewFile(
        iostream& io,
        const string& function,
        const string& oldFileName,
        const string& newFileName,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(
            tid, "ReplicationServer::applyFunctionAndCreateNewFile");
    ffeed5Info* oldFileInfo = new ffeed5Info(oldFileName);
    if(!oldFileInfo->isOK())
    {
        traceWriter->write("Could not read file");
        delete oldFileInfo;
        return;
    }
    traceWriter->write("File opened");
    ListExpr relType = oldFileInfo->getRelType();
    traceWriter->write(tid, "relType", relType);
    if(!Relation::checkType(relType))
    {
        traceWriter->write("file does not contain a relation");
        delete oldFileInfo;
        return;
    }
    traceWriter->write("relType ok");

    NestedList* nli = SecondoSystem::GetNestedList();
    QueryProcessor* queryProcessor = new QueryProcessor(nli,
            SecondoSystem::GetAlgebraManager(),
            DEFAULT_GLOBAL_MEMORY);

    ofstream out(newFileName.c_str(),ios::out|ios::binary);

    Tuple* tuple = oldFileInfo->next();

    if(tuple)
    {
        traceWriter->write("found tuple in file");
        ListExpr functionAsNestedList;
        if(!nli->ReadFromString(function, functionAsNestedList))
        {
            traceWriter->write("not a valid function");
            return;
        }
        traceWriter->write(tid, "functionAsNestedList", functionAsNestedList);
        Word fun(functionAsNestedList);
        Word funResult;
        Tuple* resultTuple = nullptr;
        applyFunction(queryProcessor, tuple, fun, funResult, resultTuple);
        if(resultTuple)
        {
            // TODO determine new rel type depending on resulting tuple type

//            TupleType* tupleType = resultTuple->GetTupleType();
//            stringstream ss;
//            ss << "rel( " << tupleType << ")";
//            traceWriter->write("new rel type", ss.str());
//            if(!nli->ReadFromString(ss.str(), relType))
//            {
//                traceWriter->write("file does not contain a relation");
//                return;
//            }

            traceWriter->write("relType", relType);
            BinRelWriter::writeHeader(out, relType);

            while ((tuple = oldFileInfo->next()))
            {
                traceWriter->write("next tuple");
                applyFunction(
                        queryProcessor, tuple, fun, funResult, resultTuple);
                if(resultTuple)
                {
                    BinRelWriter::writeNextTuple(out, resultTuple);
                }
            }
        }else
        {
            traceWriter->write("resultTuple is nullptr");
        }
        out.close();
    }else
    {
        traceWriter->write("empty rel file");
    }

    delete oldFileInfo;
    oldFileInfo = 0;
}

void ReplicationServer::applyFunction(
        QueryProcessor* qp,
        Tuple* input,
        Word function,
        Word funResult,
        Tuple* output)
{
    traceWriter->writeFunction("ReplicationServer::applyFunction");
    output = input;
//    ArgVectorPointer funArg = qp->Argument(function.addr);
//    (*funArg)[0].addr = input;
//    qp->Request(function.addr, funResult);
//    if(funResult.addr)
//    {
//        output = (Tuple*)funResult.addr;
//    }else
//    {
//        output = nullptr;
//    }
}

} /* namespace DBService */
