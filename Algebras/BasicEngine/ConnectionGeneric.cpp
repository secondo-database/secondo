/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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

*/

#include <vector>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include "ConnectionGeneric.h"

using namespace std;

namespace BasicEngine {

/**

1.1 Limit the given SQL query result

*/
string ConnectionGeneric::limitSQLQuery(const std::string &query) {

    string resultQuery = string(query);

    string sqlQueryUpper = boost::to_upper_copy<std::string>(query);

    // Only select queries can be restrircted
    if(! boost::algorithm::starts_with(sqlQueryUpper, "SELECT")) {
        return resultQuery;    
    }

    // Is query already restricted?
    if(sqlQueryUpper.find("LIMIT") == std::string::npos) {
        
        // Remove existing ";" at the query end, if exists
        if(boost::algorithm::ends_with(resultQuery, ";")) {
            resultQuery.pop_back();
        }

        // Limit query to 1 result tuple
        resultQuery.append(" LIMIT 1;");
    }

    return resultQuery;
}


/**

1.2 Get the SECONDO type of the given SQL query

*/
ListExpr ConnectionGeneric::convertTypeVectorIntoSecondoNL(
    const std::vector<std::tuple<string, string>> &types) {

    if(types.empty()) {
        return nl->TheEmptyList();
    }

    ListExpr attrList = nl->TheEmptyList();
    ListExpr attrListBegin = nl->TheEmptyList();

    for(std::tuple<string, string> type : types) {
        // Attribute name and type
        ListExpr attribute = nl->TwoElemList(
            nl->SymbolAtom(std::get<0>(type)), 
            nl->SymbolAtom(std::get<1>(type))
        );

        if(nl->IsEmpty(attrList)) {
          attrList = nl -> OneElemList(attribute);
          attrListBegin = attrList;
        } else {
          attrList = nl -> Append(attrList, attribute);
        }
    }

    ListExpr resultList = nl->TwoElemList(
        listutils::basicSymbol<Stream<Tuple> >(),
        nl->TwoElemList(
            listutils::basicSymbol<Tuple>(),
            attrListBegin));

    return resultList;
}

/**
 
1.3 Get the projection SQL for a table. 

For a table with the attributes "a1 x .. x an" the string "(a1, ..., an)"
is returned. This string can be used to remove attributes
that are created for internal reasons like the slot id from tables.

*/
std::string ConnectionGeneric::getAttributeProjectionSQLForTable(
        const std::string &table, const std::string &prefix) {


    string selectSQL = "SELECT * FROM " + table + " LIMIT 1";

    std::vector<std::tuple<std::string, std::string>> types =
        getTypeFromSQLQuery(selectSQL);
    
    // Convert tuple<attributename, type> into attributename vector
    vector<string> attributeNames;
    for(auto type : types) {
        string attributeName = std::get<0>(type);

        // Append prefix (e.g., a.attributename)
        if(! prefix.empty()) {
            attributeName.insert(0, prefix + ".");
        }

        attributeNames.push_back(attributeName);
    }
    string projectString = boost::algorithm::join(attributeNames, ", ");

    string resultString = "(";
    resultString.append(projectString);
    resultString.append(")");

    return resultString;
}


/*
1.4 Start a new transaction

*/
bool ConnectionGeneric::beginTransaction() {
    return sendCommand("START TRANSACTION;");
}

/*
1.5 Abort a transaction

*/
bool ConnectionGeneric::abortTransaction() {
    return sendCommand("ROLLBACK;");
}

/*
1.6 Commit a transaction

*/
bool ConnectionGeneric::commitTransaction() {
    return sendCommand("COMMIT;");
}

/*
1.7 ~getPartitionGridSQL~

Creates a table in with grid partitioned data.

*/
std::string ConnectionGeneric::getPartitionGridSQL(
    const std::string &table, const std::string &key,
    const std::string &geo_col, const size_t noOfSlots,
    const ::string &gridName, const std::string &targetTable) {

  string usedKey(key);
  boost::replace_all(usedKey, ",", ",r.");

  string query_exec = "SELECT r." + usedKey +
                      ", g.id AS cellno, g.id % " +
                      to_string(noOfSlots) + " as slot FROM " + gridName +
                      " g INNER JOIN " + table +
                      " r ON ST_INTERSECTS(g.cell, r." + geo_col + ")";

  return getCreateTableFromPredicateSQL(targetTable, query_exec);
}

} // Namespace
