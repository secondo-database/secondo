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

#include "Algebras/DBService2/CommunicationProtocol.hpp"
#include "Algebras/DBService2/CommunicationUtils.hpp"
#include "Algebras/DBService2/ReplicationServer.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"

#include <loguru.hpp>

#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

using namespace std;
using namespace distributed2;

namespace DBService {

ReplicationServer::ReplicationServer(int port) :
 MultiClientServer(port), FileTransferServer(port)
{
    string context("ReplicationServer");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context, port, std::cout));
    traceWriter->writeFunction("ReplicationServer::ReplicationServer");
    traceWriter->write("port", port);

    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Replication Server Port: %d", port);
}

ReplicationServer::~ReplicationServer()
{
    traceWriter->writeFunction("ReplicationServer::~ReplicationServer");
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "ReplicationServer::~ReplicationServer");
}

int ReplicationServer::start()
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction("ReplicationServer::start");
    
    return MultiClientServer::start();
}

int ReplicationServer::communicate(iostream& io)
{
    LOG_SCOPE_FUNCTION(INFO);
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
            LOG_F(INFO, "not connected to ReplicationClient");
            return 1;
        }

        queue<string> receiveBuffer;
        CommunicationUtils::receiveLines(io, 2, receiveBuffer);
        string purpose = receiveBuffer.front();
        receiveBuffer.pop();
        string fileName = receiveBuffer.front();

        fs::path filepath = ReplicationUtils::expandFilenameToAbsPath(fileName);
        traceWriter->write(tid, "Filepath", filepath.string());
        LOG_F(INFO, "Filepath: %s", filepath.string().c_str());

        receiveBuffer.pop();

        if(purpose == CommunicationProtocol::SendReplicaForStorage())
        {
            if(!FileSystem::FileOrFolderExists(filepath.string()) &&
               !createFileFromRelation(filepath))
            {
               traceWriter->write(tid, "file not found, notifying client");
               LOG_F(INFO, "file not found, notifying client");

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
                traceWriter->write("request file " + filepath.string() 
                                    + " without function");

                LOG_F(INFO, "Requested a file without providing a function %s",
                    filepath.string().c_str());

                if(!FileSystem::FileOrFolderExists(filepath.string()) &&
                   !createFileFromRelation(filepath))
                {
                   traceWriter->write(tid, "file not found, notifying client");
                   LOG_F(WARNING, "File not found, notifying client");

                   CommunicationUtils::sendLine(io,
                   distributed2::FileTransferKeywords::FileNotFound());
                } else {
                   traceWriter->write(
                       "file " + filepath.string() + " found or created");
                    
                    LOG_F(INFO, "File %s found or created.", 
                        filepath.string().c_str());

                }
                sendFileToClient(io, true, tid);
            }else
            {
                // read additional function arguments
                string n;
                CommunicationUtils::receiveLine(io,n);
                bool correct;
                int number = stringutils::str2int<int>(n,correct);
                
                if(number<0) 
                    number = 0;
                
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

                fs::path replicaFilepath = 
                    ReplicationUtils::expandFilenameToAbsPath(replicaFileName);
                
                fs::path newFilepath = 
                    ReplicationUtils::expandFilenameToAbsPath(fileName.str());

                CommunicationUtils::sendLine(io,
                        fileName.str());

                applyFunctionAndCreateNewFile(
                        io, function, otherObjects, replicaFilepath, 
                        newFilepath, tid);
            }
            sendFileToClient(io, true, tid);
        }else
        {
            traceWriter->write(tid, "unexpected purpose: ", purpose);
            LOG_F(ERROR, "Unexpected purpose: %s", purpose.c_str());

            return 1;
        }
    } catch (...)
    {
        traceWriter->write(tid, "ReplicationServer: communication error");
        LOG_F(ERROR, "ReplicationServer: Unknown Exception caught.");

        return 2;
    }
    return 0;
}

void ReplicationServer::sendFileToClient(
        iostream& io,
        bool fileCreated, // TODO remove?! seems not to be used.
        const boost::thread::id tid)
{
    LOG_SCOPE_FUNCTION(INFO);
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
        LOG_F(ERROR, "Got unexpected comands while initiating file transfer.");
    }

    // TODO Misleading: No file is created here.
    traceWriter->write(tid, "Sending file...");
    LOG_F(INFO, "Sending file...");

    std::chrono::steady_clock::time_point begin =
            std::chrono::steady_clock::now();
    
    fs::path filepath;

    int rc = ReplicationServer::sendFile(io, filepath);

    traceWriter->write(tid, "Filepath", filepath.string());
    traceWriter->write(tid, "File has been sent.");

    LOG_F(INFO, "Filepath: %s has been sent.", filepath.string().c_str());

    std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
    if(rc != 0)
    {
        traceWriter->write(tid, ("sending file " + filepath.string() 
                                 + " failed").c_str());
        LOG_F(ERROR, "Sending file %s failed.", filepath.string().c_str());
    } else {
        traceWriter->write(tid, "file sent");
        traceWriter->write("duration of send [microseconds]",
                std::chrono::duration_cast
                <std::chrono::microseconds>(end - begin).count());
        
        LOG_F(INFO, "File has been sent. Duration: %llu ms.", 
            std::chrono::duration_cast
                <std::chrono::microseconds>(end - begin).count());
    }
}

//TODO Refactor. Unreadable. Way too complex.
void ReplicationServer::applyFunctionAndCreateNewFile(
        iostream& io,
        const string& function,
        queue<string>& otherObjects,
        const fs::path& oldFileName,
        const fs::path& newFileName,
        const boost::thread::id tid)
{
    LOG_SCOPE_FUNCTION(INFO);
    traceWriter->writeFunction(
            tid, "ReplicationServer::applyFunctionAndCreateNewFile");
    traceWriter->write("oldFileName ", oldFileName.string());
    traceWriter->write("newFileName ", newFileName.string());
    traceWriter->write(tid, "FunctionList ", function);

    LOG_F(INFO, "oldFileName: %s", oldFileName.string().c_str());
    LOG_F(INFO, "newFileName: %s", newFileName.string().c_str());
    LOG_F(INFO, "FunctionList: %s", function.c_str());


    NestedList* nl = SecondoSystem::GetNestedList();
    ListExpr funlist;
    if(!nl->ReadFromString(function,funlist)){
      
      traceWriter->write("cannot parse function list");
      LOG_F(WARNING, "Cannot parse function list.");

      return;
    }

    

    if(!nl->HasLength(funlist,3 + otherObjects.size() )){

        traceWriter->write("Invalid function definition, not a function"
            " with correct number of arguments.");

        LOG_F(WARNING, "Invalid function definition, not a function"
            " with correct number of arguments.");

      return;
    }

    ListExpr args = nl->Second(funlist);  // relation argument

    if(!nl->HasLength(args,2)){
        traceWriter->write("Invalid function arguments.");
        LOG_F(WARNING, "Invalid function arguments.");

        return;
    }

    ListExpr argname = nl->First(args);

    if(nl->AtomType(argname) != SymbolType){

        traceWriter->write("Invalid name for function argument.");
        LOG_F(WARNING, "Invalid name for function argument.");

        return;
    }
    ListExpr argtype = nl->Second(args);
    ListExpr funarg1;

    // funarg may be a tuple stream or a relation
    string relName = ReplicationUtils::getRelName(
        oldFileName.filename().string());

    if(nl->HasLength(argtype,2) && nl->IsEqual(nl->First(argtype),"stream")){
        funarg1 = nl->TwoElemList(
            nl->SymbolAtom("feed"),
            nl->SymbolAtom(relName));

    } else {
       funarg1 = nl->SymbolAtom(relName);
    }

    if(stringutils::endsWith(relName, "xRPLCTD")){
       relName = relName.substr(0,relName.size()-7);
    }

    vector<pair<string,ListExpr> > otherReplacements;
    ListExpr fundef = nl->Rest(nl->Rest(funlist));
    
    while(!nl->HasLength(fundef,1)){
        ListExpr argx = nl->First(fundef);
        fundef = nl->Rest(fundef);

        if(!nl->HasLength(argx,2)){
            traceWriter->write("invalid function argument");
            LOG_F(WARNING, "Invalid function argument.");

            return;
        } 

        argx = nl->First(argx); // ignore type
        
        if(nl->AtomType(argx)!=SymbolType){
            return;
        }
        string argxs = nl->SymbolValue(argx);
        string argName = otherObjects.front();
        otherObjects.pop();
        argName = DerivateInfo::getIdentifier(relName, argName);
        otherReplacements.push_back( 
            pair<string,ListExpr>(argxs,nl->SymbolAtom(argName)));      
    }

    while(nl->HasLength(fundef,1)){ // unpack
        fundef = nl->First(fundef);
    }
    
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
                     nl->TextAtom(newFileName.string()));
 
    command = nl->TwoElemList(
                     nl->SymbolAtom("count"),
                     command);


   
    traceWriter->write(tid, "QueryCommand", command);
    LOG_F(INFO, "Query command: %s",  nl->ToString(command).c_str());

    Word queryRes;
    std::string typeStr;
    std::string errMsg;
    bool correct;
    bool evaluable;
    bool defined;
    bool isFunction;
 
    traceWriter->write(tid, "Execute Query" );
    LOG_F(INFO, "Executing query...");

    bool ok = false;
    try{
        ok = QueryProcessor::ExecuteQuery(
                    command,queryRes,typeStr,errMsg,correct,
                    evaluable,defined,isFunction,DEFAULT_GLOBAL_MEMORY,0, nl);
    } catch(SI_Error err){
        ok = false;

        traceWriter->write("Exception during query execution");
        traceWriter->write("ErrorCode", err);
        traceWriter->write("failed command " , command);

        LOG_F(ERROR, "Query execution exception. ErrorCode: %d", err);

    } catch(runtime_error& qpe){
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
    LOG_F(INFO, "Construction successful.");

    traceWriter->write(tid, "Query executed with result ", ok);
    LOG_F(INFO, "Query executed with result (bool): %d", ok);

    if(ok){
       CcInt* result = (CcInt*) queryRes.addr;
       result->DeleteIfAllowed();
       traceWriter->write("Successfully created file from derivative");
       LOG_F(INFO, "Successfully created file from derivative.");

       return;
    }

    // query not successful
    traceWriter->write(tid, "Command not successful", command);
    traceWriter->write(tid, "correct", correct);
    traceWriter->write(tid, "evaluable", evaluable);
    traceWriter->write(tid, "defined", defined);
    traceWriter->write(tid, "isFunction", isFunction);
    traceWriter->write(tid, "error message", errMsg);

    LOG_F(WARNING, "Command was not successful.");
    LOG_F(WARNING, "Query command: %s", nl->ToString(command).c_str());
    LOG_F(WARNING, "correct: %d", correct);
    LOG_F(WARNING, "evaluable: %d", evaluable);
    LOG_F(WARNING, "defined: %d ", defined);
    LOG_F(WARNING, "isFunction: %d", isFunction);
    LOG_F(WARNING, "error message: %s", errMsg.c_str());
}


bool ReplicationServer::createFileFromRelation(const fs::path& filepath){
    LOG_SCOPE_FUNCTION(INFO);

    string databaseName;
    string relname;

    ReplicationUtils::parseFileName(
        filepath.filename().string(),
        databaseName,
        relname
    );
    
    traceWriter->write("createFileFromRelation - Filepath", filepath.string());
    LOG_F(INFO, "Filepath: %s", filepath.string().c_str());
    

    // consume5: store tuple stream into a file
    string cmd = "(count (fconsume5 ( feed " + relname + "\
) '" + filepath.string() + "'))";

    Word result;

    if(QueryProcessor::ExecuteQuery(cmd,result)){
        traceWriter->write("file created from relation");
        LOG_F(INFO, "File created from relation.");
    
        CcInt* res = (CcInt*) result.addr;
        res->DeleteIfAllowed();

        return true;
    } else {
        traceWriter->write(
            "problem in creating file from relation with command'"
                         + cmd + "'");
        
        LOG_F(ERROR, "problem in creating file from relation with command: %s",
            cmd.c_str());

        return false;
    }
}

///*
    Replaces the function from the FileTransferServer to fix an issue with 
    bufsize (crashed with the default value of 1048576).
    Note that this function takes a path instead of a filename.
//*/
int ReplicationServer::sendFile(iostream& io, fs::path& outfilepath) {
    LOG_SCOPE_FUNCTION(INFO);

    string outfilename;

    // client ask for a file
    getline(io, outfilename);

    traceWriter->write("ReplicationServer::sendFile. Outfilename",
        outfilename);
    
    LOG_F(INFO, "ReplicationServer::sendFile. Outfilename: %s",
        outfilename.c_str());

    outfilepath = ReplicationUtils::expandFilenameToAbsPath(
        outfilename);

    traceWriter->write("ReplicationServer::sendFile. Outfilepath: ", 
        outfilepath.string());

    LOG_F(INFO, "Outfilepath: %s",
        outfilepath.string().c_str());

    ifstream in(outfilepath.string().c_str(), ios::binary);
    if(!in) {
        io << FileTransferKeywords::FileNotFound() << endl;
        io.flush();

        return 6;
    }
    in.seekg(0, in.end);
    size_t length = in.tellg();
    in.seekg(0, in.beg);
    io << FileTransferKeywords::Data() << endl;
    io << stringutils::any2str(length) << endl;
    io.flush();
    size_t bufsize = 8192; //1048576;
    char buffer[bufsize];
    while(!in.eof() && in.good()) {
        in.read(buffer, bufsize);
        size_t r = in.gcount();
        io.write(buffer, r);
    }
    in.close();
    io << FileTransferKeywords::EndData() << endl;
    io.flush();

    return 0;
}

} /* namespace DBService */
