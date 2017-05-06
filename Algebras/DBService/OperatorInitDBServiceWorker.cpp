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


//[$][\$]
//[_][\_]

*/

#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorInitDBServiceWorker.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"

using namespace std;
using namespace distributed2;

namespace DBService {

ListExpr OperatorInitDBServiceWorker::mapType(ListExpr nestedList)
{
    if (!nl->HasLength(nestedList, 0))
    {
        ErrorReporter::ReportError(
                "expected signature: (empty signature)");
        return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
}

int OperatorInitDBServiceWorker::mapValue(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s)
{
    string commPort;
    SecondoUtilsLocal::readFromConfigFile(commPort,
                                       "DBService",
                                       "CommunicationPort",
                                       "0");
    // TODO ErrorHandling

    ServerRunnable commServer(atoi(commPort.c_str()));
    commServer.run<CommunicationServer>();

    string fileTransferPort;
    SecondoUtilsLocal::readFromConfigFile(fileTransferPort,
                                           "DBService",
                                           "FileTransferPort",
                                           "0");
    // TODO ErrorHandling
    ServerRunnable fileServer(atoi(fileTransferPort.c_str()));
    fileServer.run<ReplicationServer>();

    result = qp->ResultStorage(s);
    static_cast<CcBool*>(result.addr)->Set(true,true);
    return 0;
}

} /* namespace DBService */
