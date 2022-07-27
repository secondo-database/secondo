/*
----
This file is part of SECONDO.

Copyright (C) 2022,
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

#ifndef BASIC_ENGINE_H
#define BASIC_ENGINE_H

#include "BasicEngineControl.h"

namespace BasicEngine {

/*
0 Declaring variables

dbs\_con is a pointer to a connection, for example to postgres

*/
extern BasicEngineControl *be_control;

/*
noMaster is just a default string for an error massage.

*/
std::string const noMaster = "\nPlease use at first an init-Operator "
                             "before using this operator!\n";

/*
noWorker is just a default string for an error massage.

*/
std::string const noWorker = "\nPlease use at first an init-Worker-Operator "
                             "before using this operator!\n";

/*
negSlots is just a default string for an error massage.

*/
std::string const negSlots =
    "\nThe number of slots have to be greater than 0."
    "The number should be a multiple of your number of workers.\n";

/*
1.1.2 Generic database connection factory

*/
ConnectionGeneric *getAndInitDatabaseConnection(const std::string &dbType,
                                                const std::string &dbUser,
                                                const std::string &dbPass,
                                                const int dbPort,
                                                const std::string &dbName);
} // namespace BasicEngine

#endif