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
#include <sstream>

#include "Profiles.h"
#include "CharTransform.h"
#include "SecParser.h"
#include "NestedList.h"

#include "Algebras/DBService/SecondoUtils.hpp"
#include "Algebras/DBService/DebugOutput.hpp"


using namespace std;
using namespace distributed2;

namespace DBService {

void SecondoUtils::readFromConfigFile(std::string& resultValue,
        const char* section,
        const char* key,
        const char* defaultValue)
{
    printFunction("SecondoUtils::readFromConfigFile");
    string secondoConfig = expandVar("$(SECONDO_CONFIG)");
    resultValue = SmiProfile::GetParameter(section,
            key, defaultValue, secondoConfig);
}

bool SecondoUtils::openDatabaseOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const char* dbName)
{
    printFunction("SecondoUtils::openDatabaseOnRemoteServer");
    return SecondoUtils::handleRemoteDatabase(connectionInfo,
                                                "open",
                                                dbName);
}

bool SecondoUtils::createDatabaseOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const char* dbName)
{
    printFunction("SecondoUtils::createDatabaseOnRemoteServer");
    return SecondoUtils::handleRemoteDatabase(connectionInfo,
                                                "create",
                                                dbName);
}

bool SecondoUtils::closeDatabaseOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo)
{
    printFunction("SecondoUtils::closeDatabaseOnRemoteServer");
    return SecondoUtils::handleRemoteDatabase(connectionInfo,
                                                "close",
                                                "");
}

bool SecondoUtils::handleRemoteDatabase(ConnectionInfo* connectionInfo,
                                          const string& action,
                                          const string& dbName)
{
    printFunction("SecondoUtils::handleRemoteDatabase");
    stringstream query;
    query << action << " database " << dbName;
    print(query.str());
    bool resultOk =
            SecondoUtils::executeQueryOnRemoteServer(connectionInfo,
                    query.str());
    if(!resultOk)
    {
        //throw new SecondoException("could not open database 'dbservice'");
        print("Boo");
    }
    return resultOk;
}

bool SecondoUtils::executeQueryOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const std::string& query)
{
    printFunction("SecondoUtils::executeQueryOnRemoteServer");
    string result;
    return executeQueryOnRemoteServer(connectionInfo,
                                      query,
                                      result);
}

bool SecondoUtils::executeQueryOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const std::string& query,
        std::string& result)
{
    printFunction("SecondoUtils::executeQueryOnRemoteServer");
    int errorCode;
    string errorMessage;
    double runtime;
    distributed2::CommandLog commandLog;
    connectionInfo->simpleCommand(query,
            errorCode, errorMessage, result, false,
            runtime, false, false, commandLog);
    //TODO better error handling
    print(errorCode);
    print(errorMessage.c_str());
    return errorCode == 0;
}

bool SecondoUtils::executeQueryOnCurrentNode(const string& query)
{
    printFunction("SecondoUtils::executeQueryOnCurrentNode");
    SecParser secondoParser;
    string queryAsNestedList;
    if (secondoParser.Text2List(query, queryAsNestedList) != 0)
    {
        // TODO
        return false;
    } else
    {
        Word result;
        print(queryAsNestedList);
        return executeQuery(queryAsNestedList, result, 1024);
    }
}

bool
SecondoUtils::executeQuery(
        const string& queryListStr,
        Word& queryResult,
        const size_t availableMemory)
{
    printFunction("SecondoUtils::executeQuery");
    string typeString(""), errorString("");
    bool success = true;
    bool correct = false, evaluable = false, defined = false,
            isFunction = false;
    ListExpr queryList;
    NestedList* nli = SecondoSystem::GetNestedList();
    success = nli->ReadFromString( queryListStr, queryList );
    if (!success)
    {
        errorString += "Error in operator query. ";
    }
    else {
        try{
            success = QueryProcessor::ExecuteQuery( queryList,
                    queryResult,
                    typeString,
                    errorString,
                    correct,
                    evaluable,
                    defined,
                    isFunction,
                    availableMemory);
        } catch(...){
            cout << "caught exception" << endl;
            success=false;
        }
        cout << "success: " << success << endl;
        cout << "correct: " << correct << endl;
        cout << "evaluable: " << evaluable << endl;
        cout << "defined: " << defined << endl;
        cout << "isFunction: " << isFunction << endl;
    }
    if (errorString != "OK")
    {
        cout << errorString << endl;
    }
    return success;
}

bool SecondoUtils::adjustDatabaseOnCurrentNode(const std::string& databaseName)
{
    printFunction("SecondoUtils::adjustDatabaseOnCurrentNode");
    //TODO check correctness
    if(SecondoSystem::GetInstance()->GetDatabaseName()
            != databaseName)
    {
        string queryClose("close database");
        SecondoUtils::executeQueryOnCurrentNode(queryClose);
        stringstream queryCreate;

        queryCreate << "create database "
                    << databaseName;
        SecondoUtils::executeQueryOnCurrentNode(queryCreate.str());

        stringstream queryOpen;
        queryOpen << "open database "
                    << databaseName;
        SecondoUtils::executeQueryOnCurrentNode(queryOpen.str());
        return true;
    }
    return false;
}

bool
SecondoUtils::createRelationOnCurrentNode(const string& queryAsString,
        string& errorMessage)
{
    printFunction("SecondoUtils::createRelationOnCurrentNode");
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    QueryProcessor* queryProcessor = SecondoSystem::GetQueryProcessor();
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    Word result = SetWord(Address(0));
    OpTree tree = 0;

    ListExpr resultType = nl->TheEmptyList();

    SecParser secondoParser;
    string queryAsNestedListString;
    if (secondoParser.Text2List(queryAsString, queryAsNestedListString) != 0) {
        // TODO
        print("could not parse query");
        return false;
    }
    ListExpr queryAsNestedList;
    if (!nl->ReadFromString(queryAsNestedListString, queryAsNestedList)) {
        print("could not convert string to list");
    }

    //TODO check open database?
    //TODO transaction?
    // SecondoSystem::BeginTransaction();
    string objectName = nl->SymbolValue(nl->Second(queryAsNestedList));
    ListExpr valueExpr = nl->Fourth(queryAsNestedList);

    try {
        queryProcessor->Construct(valueExpr, correct, evaluable, defined,
                isFunction, tree, resultType);

        if (evaluable) {
            string typeName = "";
            catalog->CreateObject(objectName, typeName, resultType, 0);
            queryProcessor->EvalP(tree, result, 1);
            catalog->UpdateObject(objectName, result);
            queryProcessor->Destroy(tree, false);
        } else {
            return false;
        }
    } catch (...) {

        print("caught error");
        queryProcessor->Destroy(tree, true);
    }

    // TODO
    //SecondoSystem::CommitTransaction(true);

    return true;
}

bool SecondoUtils::createRelationFromConsumeResult(
        const string& relationName,
        Word& result)
{
    printFunction("SecondoUtils::createRelationFromConsumeResult");
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();
    string typeName = "";
    catalog->CreateObject(relationName, typeName,
            nl->SymbolAtom(Relation::BasicType()), 0);
    catalog->UpdateObject(relationName, result);
    return true;
}

bool SecondoUtils::excuteQueryOnCurrentNode(const string& queryAsString,
        ListExpr& resultList, string& errorMessage) {
    printFunction("SecondoUtils::excuteQueryOnCurrentNode");
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    QueryProcessor* queryProcessor = SecondoSystem::GetQueryProcessor();
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    Word result = SetWord(Address(0));
    OpTree tree = 0;

    ListExpr resultType = nl->TheEmptyList();

    SecParser secondoParser;
    string queryAsNestedListString;
    if (secondoParser.Text2List(queryAsString, queryAsNestedListString) != 0) {
        // TODO
        print("could not parse query");
        return false;
    }
    ListExpr queryAsNestedList;
    if (!nl->ReadFromString(queryAsNestedListString, queryAsNestedList)) {
        print("could not convert string to list");
    }

    // database open?

    // transaction?

    try {

        queryProcessor->Construct(nl->Second(queryAsNestedList), correct,
                evaluable, defined, isFunction, tree, resultType);

        if (evaluable) {
            print(evaluable);
            queryProcessor->EvalP(tree, result, 1);

            ListExpr valueList = catalog->OutObject(resultType, result);
            resultList = nl->TwoElemList(resultType, valueList);

            queryProcessor->Destroy(tree, true);

        }

    } catch (SI_Error err) {

        print("caught error");
    }

  queryProcessor->Destroy( tree, true );
  return true;
}

} /* namespace DBService */
