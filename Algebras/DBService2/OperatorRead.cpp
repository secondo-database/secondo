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
#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/Relation-C++/OperatorFeed.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/OperatorRead.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "DBServiceClient.hpp"

#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

using namespace std;

extern boost::mutex nlparsemtx;

namespace DBService {

//TODO Refactor. Code is too complex and barely readable.
ListExpr OperatorRead::mapType(ListExpr nestedList)
{
    print(nestedList, std::cout);

    boost::unique_lock<boost::mutex> nlLock(nlparsemtx);

    if(!nl->HasLength(nestedList, 1))
    {
        ErrorReporter::ReportError(
                "expected one argument");
                return nl->TypeError();
    }

    ListExpr feedTypeMapResult = OperatorFeed::FeedTypeMap(nestedList);

    bool relationLocallyAvailable = (feedTypeMapResult != nl->TypeError());
    print("relationLocallyAvailable",
            string(relationLocallyAvailable ? "TRUE" : "FALSE"), std::cout);
    
    fs::path fileName;

    if(!relationLocallyAvailable)
    {
        print("Trying to retrieve relation from DBService", std::cout);
        const string databaseName =
                SecondoSystem::GetInstance()->GetDatabaseName();
        const string relationName = nl->ToString(nl->First(nestedList));
        print("databaseName", databaseName, std::cout);
        print("relationName", relationName, std::cout);

        vector<string> otherObjects;
        DBServiceClient* client = DBServiceClient::getInstance();

        if(!client){
          print("could not create client", std::cout);
          return listutils::typeError("Could not create client, "
                                      "check dbs configuration");
        }

        fileName = client->retrieveReplicaAndGetFileName(
                        databaseName,
                        relationName,
                        otherObjects,
                        string(""));

        if(fileName.empty())
        {
            print("Did not receive file", std::cout);
            return listutils::typeError("File does not exist");
        }

        print("fileName (path)", fileName.string(), std::cout);
        
        // ffeed5Info also needs to access the nested list        
        nlLock.unlock();

        ffeed5Info info(fileName.string());


        if(info.isOK()){
            
            // nlLock.lock(); 
            // Not locking here may be risky
            // TODO How to release the lock if the call is nested?
            feedTypeMapResult = nl->TwoElemList(
                   nl->SymbolAtom(Symbol::STREAM()),
                   nl->Second(info.getRelType()));

            // Reaquire the lock to protect the nl from other threads
            nlLock.lock(); 
        }else
        {
            print("Could not determine relation type from file", std::cout);
            return listutils::typeError("Unreadable file");
        }
    } else {
        print("Found relation locally.", std::cout);
    }

    print("feedTypeMapResult", feedTypeMapResult, std::cout);

    string fileNameStr = fileName.filename().string();
    
    ListExpr readTypeMapResult = nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList((relationLocallyAvailable ?
                    nl->StringAtom("") : nl->StringAtom(
                        as_const(fileNameStr)))),
                    feedTypeMapResult);
    print("readTypeMapResult", readTypeMapResult, std::cout);
    return readTypeMapResult;
}

int OperatorRead::mapValue(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s)
{
    print("READ mapValue", std::cout);
    
    //TODO Add fs::path and retrieve file from database dir.
    string fileName =
            static_cast<CcString*>(args[1].addr)->GetValue();

    fs::path filePath = ReplicationUtils::expandFilenameToAbsPath(fileName);

    print("Filename", fileName, std::cout);
    print("Filepath", filePath.string(), std::cout);

    if(fileName.empty())
    {
        print("Filename was empty.", std::cout);
        return OperatorFeed::Feed(args, result,
                message, local, s);
    }

    ffeed5Info* info = (ffeed5Info*) local.addr;

    switch(message){
    case OPEN:{
        print("Case OPEN", std::cout);
        if(info){
            delete info;
            local.addr = 0;
        }

        print("Reading tuple stream from file", filePath.string(), std::cout);
        info = new ffeed5Info(filePath.string());

        if(!info->isOK())
        {
            print("Could not read file", std::cout);
            delete info;
            return 0;
        }

        ListExpr relType = info->getRelType();
        print("relType", relType, std::cout);
        
        if(!Relation::checkType(relType))
        {
            print("Deleting info. checkType was false.", std::cout);
            delete info;
            return 0;
        }

        local.addr = info;
        return 0;
    }
    case REQUEST:
        print("Case REQUEST", std::cout);
        result.addr = info ? info->next() : 0;
        return result.addr? YIELD : CANCEL;
    case CLOSE:
        print("Case CLOSE", std::cout);
        if(info)
        {
            delete info;
            local.addr = 0;
        }
        return 0;
    }
    return -1;
}

} /* namespace DBService */
