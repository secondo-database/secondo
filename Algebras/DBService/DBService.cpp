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
#include <iostream>

#include "DBService.hpp"
#include "DBServiceManager.hpp"

#include "SecondoException.h"
#include "Profiles.h"

using namespace std;

namespace DBService
{

DBService::DBService(const int argc, char* argv[])
{
    cout << "DBService started" << endl;
    cout << argc << endl;
    for (size_t i = 0; i < argc; ++i)
    {
        std::string input(argv[i], strlen(argv[i]));
        cout << input << endl;
        string host;
        string config;
        string port;
        getWorkerDetails(input, host, config, port);
        //DBServiceManager::addNode(host, port, config);
        //TODO DBServiceManager::addNode()
        DBServiceManager::initialize();
    }
}

DBService::~DBService()
{
    std::cout << "DBService terminated" << std::endl;
}

void DBService::getWorkerDetails(string& input,
                                 string& host,
                                 string& config,
                                 string& port)
{
    if (input.length() == 0)
    {
        throw SecondoException("Empty string");
    }
    size_t firstColon = input.find(":");
    if ((firstColon == string::npos) || (input.length() == firstColon + 1))
    {
        throw SecondoException("No config file specified");
    }
    size_t secondColon = input.find(":", firstColon + 1);

    host = input.substr(0, firstColon);
    config = input.substr(firstColon + 1, secondColon - firstColon - 1);

    if (input.length() > secondColon)
    {
        port = input.substr(secondColon + 1);
    }
}

} /* namespace DBService */

