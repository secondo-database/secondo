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
#include <sstream>

#include "CharTransform.h"
#include "NestedList.h"
#include "Profiles.h"
#include "SecParser.h"

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"


using namespace std;
using namespace distributed2;

namespace DBService {

void SecondoUtilsLocal::readFromConfigFile(std::string& resultValue,
        const char* section,
        const char* key,
        const char* defaultValue)
{
    printFunction("SecondoUtilsLocal::readFromConfigFile");
    string secondoConfig = expandVar("$(SECONDO_CONFIG)");
    resultValue = SmiProfile::GetParameter(section,
            key, defaultValue, secondoConfig);
}

bool SecondoUtilsLocal::executeQuery(const string& query)
{
    printFunction("SecondoUtilsLocal::executeQuery");
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
SecondoUtilsLocal::executeQuery(
        const string& queryListStr,
        Word& queryResult,
        const size_t availableMemory)
{
    printFunction("SecondoUtilsLocal::executeQuery");
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

bool SecondoUtilsLocal::adjustDatabase(const std::string& databaseName)
{
    printFunction("SecondoUtilsLocal::adjustDatabase");
    //TODO check correctness
    if(SecondoSystem::GetInstance()->GetDatabaseName()
            != databaseName)
    {
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
    }
    return false;
}

bool
SecondoUtilsLocal::createRelation(const string& queryAsString,
        string& errorMessage)
{
    printFunction("SecondoUtilsLocal::createRelation");
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    QueryProcessor* queryProcessor = SecondoSystem::GetQueryProcessor();
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    // TODO check for existing catalog object

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

bool SecondoUtilsLocal::createRelationFromConsumeResult(
        const string& relationName,
        Word& result)
{
    printFunction("SecondoUtilsLocal::createRelationFromConsumeResult");
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();
    string typeName = "";
    catalog->CreateObject(relationName, typeName,
            nl->SymbolAtom(Relation::BasicType()), 0);
    catalog->UpdateObject(relationName, result);
    return true;
}

bool SecondoUtilsLocal::excuteQueryCommand(const string& queryAsString,
        ListExpr& resultList, string& errorMessage) {
    printFunction("SecondoUtilsLocal::excuteQuery");
    print("queryAsString", queryAsString);
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
