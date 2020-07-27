/*

1.1 ~ReplicationUtils~

This class provides some functions that are useful for the replication.

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
#ifndef ALGEBRAS_DBSERVICE_REPLICATIONUTILS_HPP_
#define ALGEBRAS_DBSERVICE_REPLICATIONUTILS_HPP_

#include <string>

namespace DBService {

/*

1.1.1 Class Definition

*/

class ReplicationUtils {

/*

1.1.1.1 ~getFileName~

This function constructs a file name, applicable in the original system,
from a database name and a relation name.

*/
public:
    static const std::string getFileName(
            const std::string& databaseName,
            const std::string& relationName);

/*

1.1.1.1 ~getFileNameOnDBServiceWorker~

This function constructs a file name, applicable in the ~DBService~ system,
from a database name and a relation name.

*/
    static const std::string getFileNameOnDBServiceWorker(
            const std::string& databaseName,
            const std::string& relationName);

/*

1.1.1.1 ~parseFileName~

This function parses a file name and returns the corresponding database and
relation name.

*/
    static void parseFileName(
            const std::string& fileName,
            std::string& databaseName,
            std::string& relationName);


/*
1.1.1.2 getRelName

Returns a relation name derived from a filename;

*/
   static std::string getRelName(
              const std::string& filename);


/*
1.1.1.2 ~getRelnameonDBServiceWorker~

*/
   static const std::string getRelNameOnDBServiceWorker(
             const std::string& databaseName,
             const std::string& relationName);

   static const std::string getRelId(
        const std::string& databaseName,
        const std::string& relationName);


   static std::string getDerivedName(
             const std::string& databaseName,
             const std::string& relationName,
             const std::string& derivedName);



   static bool extractRelationInfo(
            const std::string& relationID,
            std::string& dbName,
            std::string& relationName);

   static bool extractDerivateInfo(
       const std::string& derivedID,
       std::string& databaseName,
       std::string& relationName,
       std::string& derivedName);
  

/*
1.1.1.1 ~getDBStart~

Returns the string that is the start of all objects stored in 
the given database.

*/
   static inline std::string getDBStart(const std::string& database){
     return database + separator;
   }
 

/*

1.1.1.1 ~separator~

This separator is used when constructing a file name.

*/
private:
    static std::string separator;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_REPLICATIONUTILS_HPP_ */
