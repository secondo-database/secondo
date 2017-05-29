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
#include "Google/googletest/include/gtest/gtest.h"

#include "ReplicationUtils.hpp" //TODO proper path

#include <queue>

using namespace std;

namespace DBService
{

namespace Test
{

class ReplicationUtilsTest: public ::testing::Test
{
};

TEST_F(ReplicationUtilsTest, testGetFileName)
{
    string databaseName("myDB");
    string relationName("myRel");
    ASSERT_STREQ("myDBxDBSxmyRel.bin",
            ReplicationUtils::getFileName(databaseName, relationName).c_str());
}

TEST_F(ReplicationUtilsTest, testGetFileNameOnDBServiceWorker)
{
    string databaseName("myDB");
    string relationName("myRel");
    ASSERT_STREQ("myDBxDBSxmyRelxRPLCTD.bin",
            ReplicationUtils::getFileNameOnDBServiceWorker(
                    databaseName, relationName).c_str());
}

TEST_F(ReplicationUtilsTest, testParseFileName)
{
    string fileName("myDBxDBSxmyRel.bin");
    string databaseName;
    string relationName;
    ReplicationUtils::parseFileName(fileName, databaseName, relationName);
    ASSERT_STREQ("myDB", databaseName.c_str());
    ASSERT_STREQ("myRel", relationName.c_str());
}

}

}
