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
#include "OperatorStartDBService.hpp"

#include "Dist2Helper.h"
#include "DBServiceManager.hpp"

#include "DebugOutput.hpp"

using namespace std;

namespace DBService
{

ListExpr OperatorStartDBService::mapType(ListExpr nestedList)
{
    throw SecondoException("not implemented");
    //TODO usesargsintypemapping

    print(nestedList);
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
    print("checking worker relation");
    if(!distributed2::isWorkerRelDesc(nl->First(nestedList),
                                      positions,
                                      types,
                                      errorMessage))
    {
        ErrorReporter::ReportError("argument is not a worker relation: "
                                    + errorMessage);
        return nl->TypeError();
    }
    print("creating result list");
    print(nl->First(positions));
    print(nl->Second(positions));
    print(nl->Third(positions));

    /*ListExpr workerPos = nl->ThreeElemList(
            nl->First(positions),
            nl->Second(positions),
            nl->Third(positions));*/

    //ListExpr result =  nl->TwoElemList(nl->First(nestedList), workerPos);
    //print(result);
    //return result;

/*    ListExpr appendList = listutils::concat(positions,
                                     nl->OneElemList(nl->IntAtom(1)));

    ListExpr res = nl->TwoElemList(
            nl->IntAtom(1),
                          nl->First(nestedList));*/

    ListExpr appendList = nl->FourElemList(nl->First(positions),
                                           nl->Second(positions),
                                           nl->Third(positions),
                                           nl->Rest(nestedList));

    ListExpr result = listutils::basicSymbol<CcBool>();

    ListExpr tmResult = nl->ThreeElemList(
                    nl->SymbolAtom(Symbols::APPEND()),
                    appendList,
                    result);

    print(tmResult);
    return tmResult;
}

int OperatorStartDBService::mapValue(Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s)
{
    print("ValueMapping");
    print(message);
    Relation* relation = reinterpret_cast<Relation*>(args[0].addr);
    int hostPos = reinterpret_cast<CcInt*>(args[1].addr)->GetValue();
    int portPos = reinterpret_cast<CcInt*>(args[2].addr)->GetValue();
    int confPos = reinterpret_cast<CcInt*>(args[3].addr)->GetValue();

    print("Before Scan");
    GenericRelationIterator* it = relation->MakeScan();
    Tuple* tuple;

    print("Before While");
    while((tuple = it->GetNextTuple())){
        CcString* hostAttr =
                static_cast<CcString*>(tuple->GetAttribute(hostPos));
       if(!hostAttr->IsDefined())
       {
           //TODO
           print("host not defined");
       }
       CcInt* portAttr = static_cast<CcInt*>(tuple->GetAttribute(portPos));
       if(!portAttr->IsDefined())
       {
           //TODO
           print("port not defined");
       }
        CcString* configAttr =
                static_cast<CcString*>(tuple->GetAttribute(confPos));
       if(!configAttr->IsDefined())
       {
           //TODO
           print("config not defined");
       }
       string host = hostAttr->GetValue();
       int port = portAttr->GetValue();
       string config = configAttr->GetValue();
       tuple->DeleteIfAllowed();

       print("add node");
       DBServiceManager::getInstance()->addNode(host, port, config);

    }
    delete it;
    return 0;
}

} /* namespace DBService */
