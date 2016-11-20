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
#include "OperatorSetNodes.hpp"

#include "Dist2Helper.h"
#include "DBServiceManager.hpp"

using namespace std;

namespace DBService
{

ListExpr OperatorSetNodes::mapType(ListExpr nestedList)
{
    cout << listutils::stringValue(nestedList) << endl;
    if (nl->ListLength(nestedList) != 1)
    {
        ErrorReporter::ReportError(
                "expected signature: rel");
        return nl->TypeError();
    }
    if(!Relation::checkType(nl->First(nestedList)))
    {
        ErrorReporter::ReportError(
                "argument must be: rel");
        return nl->TypeError();
    }
    ListExpr positions;
    ListExpr types;
    string errorMessage;
    if(!distributed2::isWorkerRelDesc(nestedList,
                                      positions,
                                      types,
                                      errorMessage))
    {
        ErrorReporter::ReportError("argument is not a worker relation: "
                                    + errorMessage);
        return nl->TypeError();
    }

    ListExpr result = nl->FourElemList(nestedList,
                                       nl->First(positions),
                                       nl->Second(positions),
                                       nl->Third(positions));
    cout << listutils::stringValue(result) << endl;
    return result;
}

int OperatorSetNodes::mapValue(Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s)
{
    Relation* relation = reinterpret_cast<Relation*>(&args[0]);
    int hostPos = reinterpret_cast<CcInt*>(&args[1])->GetValue();
    int portPos = reinterpret_cast<CcInt*>(&args[2])->GetValue();
    int confPos = reinterpret_cast<CcInt*>(&args[3])->GetValue();

    GenericRelationIterator* it = relation->MakeScan();
    Tuple* tuple;
    while((tuple = it->GetNextTuple())){
        CcString* hostAttr =
                static_cast<CcString*>(tuple->GetAttribute(hostPos));
       if(!hostAttr->IsDefined())
       {
           //TODO
       }
       CcInt* portAttr = static_cast<CcInt*>(tuple->GetAttribute(portPos));
       if(!portAttr->IsDefined())
       {
           //TODO
       }
        CcString* configAttr =
                static_cast<CcString*>(tuple->GetAttribute(confPos));
       if(!configAttr->IsDefined())
       {
           //TODO
       }
       string host = hostAttr->GetValue();
       int port = portAttr->GetValue();
       string config = configAttr->GetValue();
       tuple->DeleteIfAllowed();

       DBServiceManager::addNode(host, port, config);

    }
    DBServiceManager::initialize();
    delete it;
    return 0;
}

} /* namespace DBService */
