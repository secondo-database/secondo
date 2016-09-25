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
#ifndef ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_
#define ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_

#include "ConnectionInfo.h"

namespace DBService
{

class DBServiceManager
{
public:

/*
1.1 getInstance

Returns the DBServiceManager instance (singleton).

*/
    static DBServiceManager* getInstance();
/*
1.1 getNodes

Returns pointers to the selected alternative storage locations
when provided with a node.

*/
    void getNodes();
    static void initialize(
            std::vector<distributed2::ConnectionInfo> connections);
private:
/*
1.2 Constructor

Creates a new DBServiceManager instance.

*/
    DBServiceManager();
/*
1.3 Copy Constructor

Does not do anything.

*/
    DBServiceManager(const DBServiceManager&)
    {}
/*
1.3 Destructor

Deallocates and deletes existing DBServiceManager instance.

*/
    ~DBServiceManager();

    static DBServiceManager* _instance;
    static std::vector<distributed2::ConnectionInfo*> connections;
    static bool isInitialized;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_ */
