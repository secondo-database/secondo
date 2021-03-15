/*

1.1 ~DBServicePersistenceAccessor~

The \textit{DBServicePersistenceAccessor} is the interface towards the
\textit{DBService's} persistant storage in SECONDO relations. It provides
functions to create relations as well as inserting, updating and deleting
tuples.

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
#ifndef ALGEBRAS_DBSERVICE_DBSERVICEPERSISTENCEACCESSOR_HPP_
#define ALGEBRAS_DBSERVICE_DBSERVICEPERSISTENCEACCESSOR_HPP_

#include <map>
#include <queue>
#include <vector>

#include "NestedList.h"

#include "Algebras/DBService2/CommandBuilder.hpp"
#include "Algebras/DBService2/DBServiceManager.hpp"
#include "Algebras/DBService2/LocationInfo.hpp"
#include "Algebras/DBService2/RelationInfo.hpp"
#include "Algebras/DBService2/DerivateInfo.hpp"

namespace DBService {

/*

1.1.1 Class Definition

*/

class DBServicePersistenceAccessor {


public:

/*

1.1.1.1 \textit{createDBSchema}

This function creates the DBService database schema if it doesn't exist 
already.

*/

    static bool createDBSchemaIfNotExists();


/*

1.1.1.1 ~getRecordCount~

Retrieve the number of tuples that the specifies relation contains.

*/
    size_t getRecordCount(
            const std::string& databaseName,
            const std::string& relationName);


private:

/*

1.1.1.1 \textit{createOrInsert}

This function creates the given secondo relation (table) if the relation doesn't exist.

*/

    static bool createSecondoRelation(const std::string &relationName,
                                                             const RelationDefinition &rel);

    /*

1.1.1.1 \textit{createOrInsert}

This function creates a relation or inserts the specified tuple if the relation
already exists.

*/
    static bool createOrInsert(
            const std::string& relationName,
            const RelationDefinition& rel,
            const std::vector<std::vector<std::string> >& values);

/*

1.1.1.1 \textit{locations}

This member specifies the attributes of the SECONDO relation that is used for
persisting the \textit{DBService} worker node locations.

*/
    static RelationDefinition locations;


    static RelationDefinition nodes;

/*

1.1.1.1 \textit{relations}

This member specifies the attributes of the SECONDO relation that is used for
persisting \textit{RelationInfo} objects.

*/
    static RelationDefinition relations;

/*

1.1.1.1 \textit{mapping}

This member specifies the attributes of the SECONDO relation that is used for
persisting the mapping of relations to locations.

*/
    static RelationDefinition mapping;

/*
1.1.1.1 ~derivate~

This member specifies the attributes of the SECONDO relation that is 
used for persisting the derivates of a relation.

*/
    static RelationDefinition derivates;

    static const std::string DBSERVICE_DATABASE_NAME;

};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICEPERSISTENCEACCESSOR_HPP_ */
