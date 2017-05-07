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
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/Replicator.hpp"
#include "Algebras/DBService/ReplicatorRunnable.hpp"

using namespace std;

namespace DBService {

ReplicatorRunnable::ReplicatorRunnable(std::string databaseName,
                                       const std::string relationName,
                                       vector<LocationInfo>& locations)
: databaseName(databaseName), relationName(relationName), locations(locations),
  runner(0)
{
    printFunction("ReplicatorRunnable::ReplicatorRunnable");
    print("databaseName", databaseName);
    print("relationName", relationName);
}

ReplicatorRunnable::~ReplicatorRunnable()
{}

void ReplicatorRunnable::run()
{
    printFunction("ReplicatorRunnable::run");
    if(runner){
        runner->join();
        delete runner;
    }
    runner = new boost::thread(
            boost::bind(&ReplicatorRunnable::create,
                        this,
                        databaseName,
                        relationName,
                        locations));
}


void ReplicatorRunnable::create(std::string& databaseName,
                                std::string& relationName,
                                vector<LocationInfo>& locations)
{
    printFunction("ReplicatorRunnable::create");
    print("databaseName", databaseName);
    print("relationName", relationName);
    Replicator replicator(databaseName, relationName);
    replicator.replicateRelation(locations);
}

} /* namespace DBService */
