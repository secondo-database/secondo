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
#include "FileSystem.h"
#include "QueryProcessor.h"

#include "Algebras/DBService2/DBServiceClient.hpp"
#include "Algebras/DBService2/OperatorAddNode.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"

#include <loguru.hpp>

extern QueryProcessor* qp;
extern boost::recursive_mutex nlparsemtx;

namespace DBService
{

ListExpr OperatorAddNode::mapType(ListExpr nestedList)
{
    print(nestedList, std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    // ensure to have only one access to the catalog
    static boost::mutex mtx;
    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);

    if (!nl->HasLength(nestedList, 3))
    {
        ErrorReporter::ReportError(
                "expected signature: host x port x config");
                return nl->TypeError();
    }

    if(!CcString::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "first argument must be: string");
        return nl->TypeError();
    }

    if(!CcInt::checkType(nl->Second(nestedList)))
    {
        ErrorReporter::ReportError(
                "second argument must be: int");
        return nl->TypeError();
    }

    if(!CcString::checkType(nl->Third(nestedList)))
    {
        ErrorReporter::ReportError(
                "third argument must be: string");
        return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
}

int OperatorAddNode::mapValue(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s)
{
    LOG_SCOPE_FUNCTION(INFO);
    
    // If invoked using a TTY, the TTY may create a transaction but we need
    // full control over transactions. So therefore we'll close any given 
    // transaction to ensure that no transaction is running.
    if(!SecondoSystem::CommitTransaction(true))
    {
        LOG_F(INFO, "%s", "There was no transaction running.");
    }
    
    CcString* host = static_cast<CcString*>(args[0].addr);
    CcInt* port = static_cast<CcInt*>(args[1].addr);
    CcString* config = static_cast<CcString*>(args[2].addr);

    DBServiceClient* dbsClient = DBServiceClient::getInstance();

    print(host->GetValue(), std::cout);
    print(port->GetValue(), std::cout);
    print(config->GetValue(), std::cout);
        
    // if (!FileSystem::FileOrFolderExists(config->GetValue()))
    // {
    //    print("The given DBService config file does not exist.", std::cout);
    //    static_cast<CcBool *>(result.addr)->SetDefined(false);
    //    return 0;
    // }

    print("Adding node...", std::cout);
    LOG_F(INFO, "%s", "Adding node...");
    
    /* TODO Invoking the DBServiceManager directly from an Operator has 
      drawbacks. It prevents the operator from being executable from
      SecondoClients which do not run the DBService processes.

      This may be confusing to users and makes the usage of the DBService
      alegbra in distributed environments harder as it requires a strong
      tie between a particular Secondo TTY - the one which has been used to
      start the DBService - and its corresponding SecondoBDB process.
      
      Due to the fact that the Secondo server process will die if the TTY 
      disconnects, the lifecycle of the DBService is tied to the SecondoTTY.
      This is not desirable. The SecondoBDB processes may run on a 
      reliable server computer but the TTY may be executed on a personal 
      computer which may have to be put into standby or switched off from time 
      to time, e.g. during transportation.

      Instead of using the DBServiceManager, the operator should enhance
      the DBServiceClient / CommunicationServer client to talk to the
      one process running the DBService. As the CommunictionServer is 
      - by besign - colocated to this process - the CommServer will
      always have access to the DBServiceManager.

      Making addNode remote executable will allow self-registration of 
      dbs workers. This is important in Kubernetes environments, for example
      where the number of workers can be easily adjusted by adapting the
      replica number in the dbs-worker statefulset.
    */
    bool success = false;

    result = qp->ResultStorage(s);

    if(dbsClient) {
        success = dbsClient->addNode(
                    host->GetValue(),
                    port->GetValue(),
                    config->getCsvStr());
        
        if (!success) {
            print("Couldn't add node using DBServiceManager.", std::cout);
            LOG_F(ERROR, "%s", "Couldn't add node using DBServiceManager.");
            static_cast<CcBool*>(result.addr)->Set(false, false);
            return 1;
        }
    } else {
        print("Couldn't get DBService Client instance.", std::cout);
        LOG_F(ERROR, "%s", "Couldn't get DBService Client instance.");

        static_cast<CcBool*>(result.addr)->Set(false, false);
        return 1;
    }

    print("Done adding node.", std::cout);
    LOG_F(INFO, "%s", "Done adding node.");

    //TODO find out if there's a more elegant way...
    /* The test suite will commit many transactions but Secondo will attempt 
        to close
     * the transaction for the given operator. Therefore, here a transaction 
     * will be opened
     * to satisfy this requirement for a successful operator execution.
     */
    SecondoSystem::BeginTransaction();

    // Sets defined?=yes, success=true
    static_cast<CcBool*>(result.addr)->Set(true,success);

    return 0;
}

} /* namespace DBService */
