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

        if(purpose == CommunicationProtocol::SendReplicaForStorage())
        {
            if(!FileSystem::FileOrFolderExists(fileName) &&
               !createFileFromRelation(fileName))
            {
               traceWriter->write(tid, "file not found, notifying client");
               CommunicationUtils::sendLine(io,
                    distributed2::FileTransferKeywords::FileNotFound());
            }
            sendFileToClient(io, true, tid);
        }else if(purpose == CommunicationProtocol::SendReplicaForUsage())
        {
            CommunicationUtils::sendLine(io,
                    CommunicationProtocol::FunctionRequest());
            string function;
            CommunicationUtils::receiveLine(io, function);
            if(function == CommunicationProtocol::None())
            {
                traceWriter->write("request file " + fileName 
                                    + " without function");
                if(!FileSystem::FileOrFolderExists(fileName) &&
                   !createFileFromRelation(fileName))
                {
                   traceWriter->write(tid, "file not found, notifying client");
                   CommunicationUtils::sendLine(io,
                   distributed2::FileTransferKeywords::FileNotFound());
                } else {
                   traceWriter->write("file " + fileName + " found or created");
                }
                sendFileToClient(io, true, tid);
            }else
            {
                // read additional function arguments
                string n;
                CommunicationUtils::receiveLine(io,n);
                bool correct;
                int number = stringutils::str2int<int>(n,correct);
                if(number<0) number = 0;
                queue<string> otherObjects;
                if(number > 0){
                    CommunicationUtils::receiveLines(io,number,otherObjects);
                }                

                CommunicationUtils::sendLine(io,
                        CommunicationProtocol::FileName());

                string replicaFileName;
                CommunicationUtils::receiveLine(io, replicaFileName);
                CommunicationUtils::sendLine(io,
                        CommunicationProtocol::FileName());

                std::time_t currentTime = std::time(0);
                stringstream fileName;
                fileName << currentTime << "_" << replicaFileName;

                CommunicationUtils::sendLine(io,
                        fileName.str());

                applyFunctionAndCreateNewFile(
                        io, function, otherObjects, replicaFileName, 
                        fileName.str(), tid);
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
    std::string filename;
    int rc = sendFile(io, filename);
    std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
    if(rc != 0)
    {
        traceWriter->write(tid, ("sending file " + filename 
                                 + " failed").c_str());
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
        queue<string>& otherObjects,
        const string& oldFileName,
        const string& newFileName,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(
            tid, "ReplicationServer::applyFunctionAndCreateNewFile");
    traceWriter->write("oldFileName ", oldFileName);
    traceWriter->write("newFileName ", newFileName);


    traceWriter->write(tid, "FunctionList " , function); 

    NestedList* nl = SecondoSystem::GetNestedList();
    ListExpr funlist;
    if(!nl->ReadFromString(function,funlist)){
      traceWriter->write("cannot parse function list");
      return;
    }

    

    if(!nl->HasLength(funlist,3 + otherObjects.size() )){
      traceWriter->write("invalid function definition, not a function"
                         " with correct number of arguments");
      return;
    }
    ListExpr args = nl->Second(funlist);  // relation argument
    if(!nl->HasLength(args,2)){
      traceWriter->write("invalid function arguments");
      return;
    }
    ListExpr argname = nl->First(args);
    if(nl->AtomType(argname) != SymbolType){
      traceWriter->write("invalid name for function argument");
      return;
    }
    ListExpr argtype = nl->Second(args);
    ListExpr funarg1;
    // funarg may be a tuple stream or a relation
    string relName = ReplicationUtils::getRelName(oldFileName);
    if(nl->HasLength(argtype,2) && nl->IsEqual(nl->First(argtype),"stream")){
       funarg1 = nl->TwoElemList(
                            nl->SymbolAtom("feed"),
                            nl->SymbolAtom(relName));
    } else {
       funarg1 = nl->SymbolAtom(relName);
    }

    vector<pair<string,ListExpr> > otherReplacements;
    ListExpr fundef = nl->Rest(nl->Rest(funlist));
    while(!nl->HasLength(fundef,1)){
       ListExpr argx = nl->First(fundef);
       fundef = nl->Rest(fundef);
       if(!nl->HasLength(argx,2)){
          traceWriter->write("invalid function argument");
          return;
       } 
       argx = nl->First(argx); // ignore type
       if(nl->AtomType(argx)!=SymbolType){
          return;
       }
       string argxs = nl->SymbolValue(argx);
       string argName = otherObjects.front();
       otherObjects.pop();
       argName = MetadataObject::getIdentifier(relName, argName);
       otherReplacements.push_back( 
                      pair<string,ListExpr>(argxs,nl->SymbolAtom(argName)));
      
    }    

    fundef = nl->First(fundef);
    
    ListExpr command = listutils::replaceSymbol(fundef, 
                                  nl->SymbolValue(argname), funarg1, nl);



    for( auto p : otherReplacements){
        command = listutils::replaceSymbol(command, p.first, p.second,nl);
    }


    // now, command produces a stream. 
    // write the stream into a file and just count it
   
    command = nl->ThreeElemList(
                     nl->SymbolAtom("fconsume5"),
                     command,
                     nl->TextAtom(newFileName));
 
    command = nl->TwoElemList(
                     nl->SymbolAtom("count"),
                     command);


   
    traceWriter->write(tid, "QueryCommand", command);

 
    Word queryRes;
    std::string typeStr;
    std::string errMsg;
    bool correct;
    bool evaluable;
    bool defined;
    bool isFunction;
 
    traceWriter->write(tid, "Execute Query" );

   bool ok = false;
   try{
      ok = QueryProcessor::ExecuteQuery(
                 command,queryRes,typeStr,errMsg,correct,
                 evaluable,defined,isFunction,DEFAULT_GLOBAL_MEMORY,0, nl);
   } catch(runtime_error qpe){
       ok = false;
       traceWriter->write("Exception during query execution");
       traceWriter->write(qpe.what());
       traceWriter->write("failed command " , command);
   } catch(...){
       ok = false;
       traceWriter->write("Exception during query execution");
       traceWriter->write("failed command " , command);
       return;
   }

   traceWriter->write(tid, "construction successful"); 


    traceWriter->write(tid, "Query executed with result " , ok );


    if(ok){
       CcInt* result = (CcInt*) queryRes.addr;
       result->DeleteIfAllowed();
       traceWriter->write("Creating derived file successful");
       return;
    }

    // query not successful
    traceWriter->write(tid, "Command not successful", command);
    traceWriter->write(tid, "correct", correct);
    traceWriter->write(tid, "evaluable", evaluable);
    traceWriter->write(tid, "defined", defined);
    traceWriter->write(tid, "isFunction", isFunction);
    traceWriter->write(tid, "error message", errMsg);
    
}


bool ReplicationServer::createFileFromRelation(const std::string& filename){
   string relname = ReplicationUtils::getRelName(filename);
   string cmd = "(count (fconsume5 ( feed "+relname+") '"+filename+"'))";
   Word result;
   if(QueryProcessor::ExecuteQuery(cmd,result)){
      traceWriter->write("file created from relation");
      CcInt* res = (CcInt*) result.addr;
      res->DeleteIfAllowed();
      return true;
   } else {
      traceWriter->write("problem in creating file from relation with command'"
                         + cmd + "'");
      return false;
   }
}

} /* namespace DBService */
