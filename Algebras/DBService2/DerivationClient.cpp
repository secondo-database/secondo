/*

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


1 Implementation of class DerivationClient

*/

#include "Algebras/DBService2/DerivationClient.hpp"
#include "Algebras/DBService2/MetadataObject.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/CommunicationClient.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"
#include "SecondoSystem.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "Algebras/DBService2/LockKeeper.hpp"

#include <loguru.hpp>

extern boost::recursive_mutex nlparsemtx;

namespace DBService{

    DerivationClient::DerivationClient(
            const std::string& _DBName,
            const std::string& _targetName,
            const std::string& _relName,
            const std::string& _fundef): DBName(_DBName), 
                targetName(_targetName),
                relName(_relName), fundef(_fundef){
        
        LOG_SCOPE_FUNCTION(INFO);
        printFunction(__PRETTY_FUNCTION__, std::cout);

        relIdOnWorker = ReplicationUtils::getRelNameOnDBServiceWorker(DBName,
                                                                    relName);
        rid = RelationInfo::getIdentifier(DBName, relName); 
        targetId = DerivateInfo::getIdentifier(rid, targetName);
    }


    void DerivationClient::start() {

    LOG_SCOPE_FUNCTION(INFO);
    printFunction(__PRETTY_FUNCTION__, std::cout);
            
    LOG_F(INFO, "%s", "Acquiring lock for nlparsemtx...");
    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for nlparsemtx...");

    try{
        SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
        // some basic checks
        if(ctlg->IsObjectName(targetId)){
            derivationFailed("target " + targetName + " already exists");
            return;
        }
        if(!ctlg->IsObjectName(relIdOnWorker)){
            derivationFailed("argument relation " + relName 
                + " does not exist");
            return;
        }
    
        // convert function string into nested list
        // and check for valid format
        ListExpr funlist;
        if(!nl->ReadFromString(fundef, funlist)){
            derivationFailed("cannot parse function definition");
            return;
        }  
        if(!nl->HasLength(funlist,3)){
            derivationFailed("invalid function description");
            return;
        }
        ListExpr funarg = nl->Second(funlist);
        if(!nl->HasLength(funarg,2)){
            derivationFailed("invalid function description");
            return;
        }
        ListExpr argNameL = nl->First(funarg);
        if(nl->AtomType(argNameL)!=SymbolType){
            derivationFailed("invalid argument name");
        }

        // replace the argument's name in the function 
        // definition by the relation's id
        std::string argName = nl->SymbolValue(argNameL);
        ListExpr fundeflist = nl->Third(funlist);
        ListExpr relSymb = nl->SymbolAtom(relIdOnWorker);
        fundeflist = listutils::replaceSymbol(fundeflist,argName, relSymb,nl);
    
        // try to evaluate the function
        Word QueryResult;
        std::string typeString, errorString;
        bool correct, evaluable, defined, isFunction;
    
        // LOG_F(INFO, "%s", "Acquiring lock for queryProcessorGuard...");
        // boost::lock_guard<boost::recursive_mutex> queryProcessorGuard(
        //     // Dereference the shared_ptr to the mutex
        //     *LockKeeper::getInstance()->getQueryProcessorMutex()
        // );
        // LOG_F(INFO, "%s", 
        //     "Successfully acquired lock for queryProcessorGuard.");


        // //TODO Make lock timeout 300configurable
        // if(!qpMutex->try_lock_for(boost::chrono::seconds(300))) {
        //     LOG_F(ERROR, "%s", "Acquisition of QueryProcessorMutex "
        //         "failed due to timeout.");
        // }

        // // Whenever qpMutex is dereferenced, a copy is created which isn't 
        // possible as the object is marked as non-copyable.

        // boost::timed_mutex& qpMutex2 = *qpMutex;

        // // Create a lockguard for the timed_mutex
        // boost::lock_guard<boost::mutex>
        //     lock(qpMutex2, boost::adopt_lock_t());
        
        // Establishing a timeout for locks
        // https://dieboostcppbibliotheken.de/boost.thread-synchronisation
        
        
        // std::shared_ptr<boost::timed_mutex> qpMutex = 
        //     LockKeeper::getInstance()->getQueryProcessorMutex();

        // boost::unique_lock<boost::timed_mutex> 
        //     lock{ *qpMutex, boost::try_to_lock };

        // if(lock.owns_lock() || 
        //     lock.try_lock_for(boost::chrono::seconds{ 360 })) {
        //     LOG_F(INFO, "%s", "Successfully acquired QueryProcessorMutex.");
        // } else {
        //     LOG_F(ERROR, "%s", "Acquisition of QueryProcessorMutex "
        //                  "failed due to timeout.");
        //     return;
        // }

        LOG_F(INFO, "%s", "Starting transaction to create derivative...");

        SecondoSystem::BeginTransaction();
        try{
            QueryProcessor::ExecuteQuery(fundeflist, QueryResult, typeString, 
                                        errorString, correct, 
                                        evaluable,defined, isFunction);
        } catch(...){
            correct = false;
            errorString = "exception during executeQuery";
        }
        if(!correct){
            derivationFailed(errorString);
            return;
        }

        // insert result into the database
        ListExpr typeExpr;
        nl->ReadFromString(typeString, typeExpr);

        LOG_F(INFO, "%s", "Inserting derivative into the database...");

        if(!ctlg->InsertObject(targetId, "",typeExpr, QueryResult, true)) {
            derivationFailed("could not insert target object");
            return;
        }

        ctlg->CleanUp(false,true);
        SecondoSystem::CommitTransaction(true);

        LOG_F(INFO, "%s", "Derivative has been created. "
            "Notifying the DBService...");        

        // report success of operation
        derivationSuccessful();
    } catch(...){
        derivationFailed("Exception during start function");
    }
    }
         

    /*
        TODO While success is reported to the DBService, failure isn't.
            A failed creation of a derivative should be communicated to the 
            DBService so that the DBServiceManager can mark the Derivation
            as "failed". Also the reporting back could be used to provide
            metadata about the failure helping the DBServiceManager to decide
            on whether to re-attempt the creation later.
    */
    void DerivationClient::derivationFailed(const std::string& error){
        LOG_SCOPE_FUNCTION(INFO);
        LOG_F(ERROR, "Failed to created derivative: %s", error.c_str());
        print("derivation failed: " + error, std::cout);
    }

    void DerivationClient::derivationSuccessful(){
        LOG_SCOPE_FUNCTION(INFO);
        printFunction(__PRETTY_FUNCTION__, std::cout);

        std::string dbServiceHost;
        std::string dbServicePort;
        if(!SecondoUtilsLocal::lookupDBServiceLocation(
                dbServiceHost,
                dbServicePort))
        {
            LOG_F(ERROR, "%s", "Error during lookupDBServiceLocation");
            print("Error during lookupDBServiceLocation", std::cout);
            throw new SecondoException("Unable to connect to DBService");
        }

        CommunicationClient clientToDBServiceMaster(
                dbServiceHost,
                atoi(dbServicePort.c_str()),
                0);
        clientToDBServiceMaster.reportSuccessfulDerivation(
                targetId);
        
        LOG_F(INFO, "%s", "Successfully created and reported the derivative.");
        print("derivationSuccessful reported", std::cout);
    }


}

