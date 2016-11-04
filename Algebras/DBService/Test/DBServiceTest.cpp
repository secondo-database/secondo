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
#include "Google/googletest/include/gtest/gtest.h"

#include "DBService.hpp"
#include "DBServiceManager.hpp"

using namespace std;

namespace DBService
{

namespace Test
{

class TestDBService : public DBService
{
public:
    TestDBService(const int argc, char* argv[]) :
            DBService(argc, argv)
    {}

    void getWorkerDetails(string& input,
                          string& host,
                          string& config,
                          string& port)
    {
        DBService::getWorkerDetails(input, host, config, port);
    }
};

class DBServiceTest: public ::testing::Test
{
public:
    void SetUp()
    {
        host.clear();
        config.clear();
        port.clear();
    }

protected:
    string host;
    string config;
    string port;
};

TEST_F(DBServiceTest, DBServiceInitializesDBServiceManager)
{
    char* args[] = { "myHost:myConfig:myPort" };
    DBService::DBService dbService(1, args);
    ASSERT_NO_THROW(DBServiceManager::getInstance());
}

TEST_F(DBServiceTest, getWorkerDetails)
{
    char* args[] = { 0 };
    TestDBService dbService(0, args);
    std::string input("myHost:myConfig:myPort");
    dbService.getWorkerDetails(input, host, config, port);
    ASSERT_STREQ("myHost", host.c_str());
    ASSERT_STREQ("myConfig", config.c_str());
    ASSERT_STREQ("myPort", port.c_str());
}

TEST_F(DBServiceTest, getWorkerDetailsSecondColonButNoPort)
{
    char* args[] = { 0 };
    TestDBService dbService(0, args);
    std::string input("myHost:myConfig:");
    dbService.getWorkerDetails(input, host, config, port);
    ASSERT_STREQ("myHost", host.c_str());
    ASSERT_STREQ("myConfig", config.c_str());
    ASSERT_STREQ("", port.c_str());
}

TEST_F(DBServiceTest, getWorkerDetailsNoPort)
{
    char* args[] = { 0 };
    TestDBService dbService(0, args);
    std::string input("myHost:myConfig");
    dbService.getWorkerDetails(input, host, config, port);
    ASSERT_STREQ("myHost", host.c_str());
    ASSERT_STREQ("myConfig", config.c_str());
    ASSERT_STREQ("", port.c_str());
}

TEST_F(DBServiceTest, getWorkerDetailsNoConfig)
{
    char* args[] = { 0 };
    TestDBService dbService(0, args);
    std::string input("myHost:");
    try{
    dbService.getWorkerDetails(input, host, config, port);
    }catch(const std::exception& e)
    {
        cout << e.what() << endl;
    }
    ASSERT_THROW(dbService.getWorkerDetails(input, host, config, port),
                 SecondoException);
}

TEST_F(DBServiceTest, getWorkerDetailsNoConfigNoColon)
{
    char* args[] = { 0 };
    TestDBService dbService(0, args);
    std::string input("myHost");
    ASSERT_THROW(dbService.getWorkerDetails(input, host, config, port),
                 SecondoException);
}

TEST_F(DBServiceTest, getWorkerDetailsEmptyString)
{
    char* args[] = { 0 };
    TestDBService dbService(0, args);
    std::string input("");
    ASSERT_THROW(dbService.getWorkerDetails(input, host, config, port),
                 SecondoException);
}




}
}
