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
#include <algorithm>
#include <sstream>

#include <boost/thread/mutex.hpp>

#include "CharTransform.h"
#include "NestedList.h"
#include "Profiles.h"
#include "SecParser.h"

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/TraceSettings.hpp"

using namespace std;
using namespace distributed2;

namespace DBService {

void SecondoUtilsLocal::readFromConfigFile(string& resultValue,
        const char* section,
        const char* key,
        const char* defaultValue)
{
    printFunction("SecondoUtilsLocal::readFromConfigFile", std::cout);
    string secondoConfig = expandVar("$(SECONDO_CONFIG)");
    resultValue = SmiProfile::GetParameter(section,
            key, defaultValue, secondoConfig);
}

bool SecondoUtilsLocal::executeQuery(const string& queryAsString)
{
    Word queryResult;
    return executeQuery(queryAsString, queryResult);
}

bool SecondoUtilsLocal::prepareQueryForProcessing(
            const string& queryAsString,
            string& queryAsPreparedNestedListString)
{
    printFunction("SecondoUtilsLocal::prepareQueryForProcessing", std::cout);
    print("queryAsString", queryAsString, std::cout);
    SecParser secondoParser;
    string queryAsNestedListString;
    if (secondoParser.Text2List(queryAsString, queryAsNestedListString) != 0)
    {
        print("could not parse query", std::cout);
        return false;
    }
    print("query converted to nested list string", std::cout);
    print("queryAsNestedListString 1", queryAsNestedListString, std::cout);

    if(queryAsNestedListString.find("(query ") == 0)
    {
        queryAsNestedListString.erase(0, strlen("(query "));
        print("queryAsNestedListString 2", queryAsNestedListString, std::cout);
        queryAsPreparedNestedListString =
                queryAsNestedListString.substr(
                        0, queryAsNestedListString.rfind(")"));
    }else
    {
        queryAsPreparedNestedListString = queryAsNestedListString;
    }
    print("queryAsPreparedNestedListString",
            queryAsPreparedNestedListString, std::cout);
    return true;
}

bool SecondoUtilsLocal::executeQuery2(const string& queryAsString)
{
    printFunction("SecondoUtilsLocal::executeQuery2", std::cout);
    Word queryResult;
    string queryAsPreparedNestedListString;
    boost::lock_guard<boost::mutex> lock(utilsMutex);
    if(!prepareQueryForProcessing(
            queryAsString,
            queryAsPreparedNestedListString))
    {
        return false;
    }
    int traceLevel =
            TraceSettings::getInstance()->isDebugTraceOn() ? 3 : 0;
    return QueryProcessor::ExecuteQuery(
            queryAsPreparedNestedListString,
            queryResult,
            DEFAULT_GLOBAL_MEMORY,
            traceLevel);
}

bool SecondoUtilsLocal::executeQuery(const string& queryAsString,
                                     Word& queryResult)
{
    printFunction("SecondoUtilsLocal::executeQuery", std::cout);
    boost::lock_guard<boost::mutex> lock(utilsMutex);
    print("queryAsString", queryAsString, std::cout);
    SecParser secondoParser;
    string queryAsNestedListString;
    if (secondoParser.Text2List(queryAsString, queryAsNestedListString) != 0) {
        print("could not parse query", std::cout);
        return false;
    }
    print("query converted to nested list string", std::cout);

    ListExpr queryAsNestedList;
    NestedList* nli = SecondoSystem::GetNestedList();
    if(!nli->ReadFromString(queryAsNestedListString,
            queryAsNestedList))
    {
        print("could not read nested list from string", std::cout);
        return false;
    }

    print("nested list string converted to nested list", std::cout);

    print("queryAsNestedListString", queryAsNestedListString, std::cout);

    bool success = true;
    bool correct, evaluable, defined, isFunction = false;
    string typeString(""), errorString("");
    try
    {
        success = QueryProcessor::ExecuteQuery(
                queryAsNestedList,
                queryResult,
                typeString,
                errorString,
                correct,
                evaluable,
                defined,
                isFunction
                /*availableMemory*/);
    } catch(...)
    {
        print("Caught exception during query execution", std::cout);
        success = false;
        print("errorString", errorString, std::cout);
    }
    print("success", success, std::cout);
    print("correct", correct, std::cout);
    print("evaluable", evaluable, std::cout);
    print("defined", defined, std::cout);
    print("isFunction", isFunction, std::cout);
    return success;
}

bool SecondoUtilsLocal::adjustDatabase(const string& databaseName)
{
    printFunction("SecondoUtilsLocal::adjustDatabase", std::cout);
    const string currentDB = SecondoSystem::GetInstance()->GetDatabaseName();
    print("current database name", currentDB, std::cout);
    print("requested database name", databaseName, std::cout);

    string databaseNameUppered(databaseName);

    transform(
            databaseNameUppered.begin(),
            databaseNameUppered.end(),
            databaseNameUppered.begin(),
            ::toupper);

    if(currentDB
            != databaseNameUppered)
    {
        boost::lock_guard<boost::mutex> lock(utilsMutex);
        print("need to adjust database", std::cout);
        string queryClose("close database");
        SecondoUtilsLocal::executeQuery(queryClose);

        stringstream queryCreate;
        queryCreate << "create database "
                    << databaseName;
        SecondoUtilsLocal::executeQuery(queryCreate.str());

        stringstream queryOpen;
        queryOpen << "open database "
                    << databaseName;
        SecondoUtilsLocal::executeQuery(queryOpen.str());
        return true;
    }else
    {
        print("database name matches, no need to adjust", std::cout);
    }
    return false;
}

bool
SecondoUtilsLocal::createRelation(const string& queryAsString,
        string& errorMessage)
{
    printFunction("SecondoUtilsLocal::createRelation", std::cout);
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    boost::lock_guard<boost::mutex> lock(utilsMutex);
    NestedList* nli = SecondoSystem::GetNestedList();
    QueryProcessor* queryProcessor = new QueryProcessor( nli,
            SecondoSystem::GetAlgebraManager(),
            DEFAULT_GLOBAL_MEMORY);
    //queryProcessor->SetDebugLevel(3);
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    Word result = SetWord(Address(0));
    OpTree tree = 0;

    ListExpr resultType = nl->TheEmptyList();

    print("queryAsString", queryAsString, std::cout);
    SecParser secondoParser;
    string queryAsNestedListString;
    if (secondoParser.Text2List(queryAsString, queryAsNestedListString) != 0) {
        print("could not parse query", std::cout);
        return false;
    }
    print("query converted to nested list string", std::cout);
    ListExpr queryAsNestedList;
    if (!nl->ReadFromString(queryAsNestedListString, queryAsNestedList)) {
        print("could not convert string to list", std::cout);
    }
    print("nested list string converted to nested list", std::cout);

    string objectName = nl->SymbolValue(nl->Second(queryAsNestedList));
    ListExpr valueExpr = nl->Fourth(queryAsNestedList);

    try {
        queryProcessor->Construct(valueExpr, correct, evaluable, defined,
                isFunction, tree, resultType);

        if (evaluable) {
            string typeName = "";
            catalog->CreateObject(objectName, typeName, resultType, 0);
            queryProcessor->EvalS(tree, result, 1);
            catalog->UpdateObject(objectName, result);
            queryProcessor->Destroy(tree, false);
        } else {
            return false;
        }
    } catch (...) {

        print("caught error", std::cout);
        queryProcessor->Destroy(tree, true);
    }

    cout << "correct: " << correct << endl;
    cout << "evaluable: " << evaluable << endl;
    cout << "defined: " << defined << endl;
    cout << "isFunction: " << isFunction << endl;
    return true;
}

bool SecondoUtilsLocal::executeQueryCommand(const string& queryAsString)
{
    printFunction("SecondoUtilsLocal::executeQueryCommand (1 arg)", std::cout);
    ListExpr resultList;
    string errorMessage;
    bool resultOk = SecondoUtilsLocal::executeQueryCommand(
                                                    queryAsString,
                                                    resultList,
                                                    errorMessage);
    if(resultOk)
    {
        print("query executed successfully", std::cout);
        print("resultList", resultList, std::cout);
    }else
    {
        print("failed to execute query", std::cout);
        print("errorMessage", errorMessage, std::cout);
    }
    return resultOk;
}

bool SecondoUtilsLocal::executeQueryCommand(const string& queryAsString,
        ListExpr& resultList, string& errorMessage) {
    printFunction("SecondoUtilsLocal::executeQueryCommand (2 args)", std::cout);
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    boost::lock_guard<boost::mutex> lock(utilsMutex);
    NestedList* nli = SecondoSystem::GetNestedList();
    QueryProcessor* queryProcessor = new QueryProcessor( nli,
            SecondoSystem::GetAlgebraManager(),
            DEFAULT_GLOBAL_MEMORY);
    //queryProcessor->SetDebugLevel(3);
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    Word result = SetWord(Address(0));
    OpTree tree = 0;

    ListExpr resultType = nl->TheEmptyList();

    print("queryAsString", queryAsString, std::cout);
    SecParser secondoParser;
    string queryAsNestedListString;
    if (secondoParser.Text2List(queryAsString, queryAsNestedListString) != 0) {
        print("could not parse query", std::cout);
        return false;
    }
    print("query converted to nested list string", std::cout);
    print("queryAsNestedListString", queryAsNestedListString, std::cout);

    ListExpr queryAsNestedList;
    if (!nl->ReadFromString(queryAsNestedListString, queryAsNestedList)) {
        print("could not convert string to list", std::cout);
    }
    print("nested list string converted to nested list", std::cout);
    print("queryAsNestedList", queryAsNestedList, std::cout);

    try {
        print("queryProcessor->Construct", std::cout);
        queryProcessor->Construct(nl->Second(queryAsNestedList), correct,
                evaluable, defined, isFunction, tree, resultType);
        print("trying queryProcessor->Construct done", std::cout);
        cout << "correct: " << correct << endl;
        cout << "evaluable: " << evaluable << endl;
        cout << "defined: " << defined << endl;
        cout << "isFunction: " << isFunction << endl;

        if (evaluable) {
            queryProcessor->EvalS(tree, result, 1);
            print("queryProcessor->EvalP done", std::cout);

            ListExpr valueList = catalog->OutObject(resultType, result);
            print("valueList done", std::cout);

            resultList = nl->TwoElemList(resultType, valueList);
            print("resultList done", std::cout);

            queryProcessor->Destroy(tree, true);
            print("queryProcessor->Destroy done", std::cout);
        }

    } catch (...) {
        print("caught error", std::cout);
        queryProcessor->Destroy(tree, true);
        return false;
    }
    return true;
}

bool SecondoUtilsLocal::lookupDBServiceLocation(
            string& host,
            string& commPort)
{
    printFunction("DBServiceConnector::lookupDBServiceLocation", std::cout);
    SecondoUtilsLocal::readFromConfigFile(host,
                                           "DBService",
                                           "DBServiceHost",
                                           "");
    if(host.length() == 0)
    {
        print("could not find DBServiceHost in config file", std::cout);
        return false;
    }

    SecondoUtilsLocal::readFromConfigFile(commPort,
                                       "DBService",
                                       "DBServicePort",
                                       "");
    if(commPort.length() == 0)
    {
        print("could not find DBServicePort in config file", std::cout);
        return false;
    }
    return true;
}

boost::mutex SecondoUtilsLocal::utilsMutex;

} /* namespace DBService */
