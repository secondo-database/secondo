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

#include "RelationInfo.hpp"

#include <queue>

using namespace std;

namespace DBService
{

namespace Test
{

class RelationInfoTest: public ::testing::Test
{
public:
    RelationInfoTest()
: dbName("myDatabase"), relName("myRelation"),
  host("myHost"), port("12345"), disk("myDisk"),
  relationInfo(0)
{}
    void SetUp()
    {
        relationInfo = new RelationInfo(
                                    dbName,
                                    relName,
                                    host,
                                    port,
                                    disk);
    }

    void TearDown()
    {
        if(relationInfo)
        {
            delete relationInfo;
            relationInfo = 0;
        }
    }

protected:
    string dbName;
    string relName;
    string host;
    string port;
    string disk;
    RelationInfo* relationInfo;
};

TEST_F(RelationInfoTest, testGetDatabaseName)
{
    ASSERT_STREQ(dbName.c_str(), relationInfo->getDatabaseName().c_str());
}

TEST_F(RelationInfoTest, testGetRelationName)
{
    ASSERT_STREQ(relName.c_str(), relationInfo->getRelationName().c_str());
}

TEST_F(RelationInfoTest, testGetHost)
{
    ASSERT_STREQ(host.c_str(),
            relationInfo->getOriginalLocation().getHost().c_str());
}

TEST_F(RelationInfoTest, testGetPort)
{
    ASSERT_STREQ(port.c_str(),
            relationInfo->getOriginalLocation().getPort().c_str());
}

TEST_F(RelationInfoTest, testGetDisk)
{
    ASSERT_STREQ(disk.c_str(),
            relationInfo->getOriginalLocation().getDisk().c_str());
}

TEST_F(RelationInfoTest, testAddNode)
{
    ASSERT_EQ(0u, relationInfo->getNodeCount());
    ConnectionID connID = 5;
    relationInfo->addNode(connID);
    ASSERT_EQ(1u, relationInfo->getNodeCount());
    map<ConnectionID, bool>::const_iterator it =
            relationInfo->nodesBegin();
    ASSERT_EQ(connID, it->first);
    ASSERT_FALSE(it->second);
    ASSERT_EQ(relationInfo->nodesEnd(), ++it);
}

TEST_F(RelationInfoTest, testAddNodeReplicatedTrue)
{
    ASSERT_EQ(0u, relationInfo->getNodeCount());
    ConnectionID connID = 5;
    relationInfo->addNode(connID, true);
    ASSERT_EQ(1u, relationInfo->getNodeCount());
    map<ConnectionID, bool>::const_iterator it =
            relationInfo->nodesBegin();
    ASSERT_EQ(connID, it->first);
    ASSERT_TRUE(it->second);
    ASSERT_EQ(relationInfo->nodesEnd(), ++it);
}

TEST_F(RelationInfoTest, testAddNodeReplicatedFalse)
{
    ASSERT_EQ(0u, relationInfo->getNodeCount());
    ConnectionID connID = 5;
    relationInfo->addNode(connID, false);
    ASSERT_EQ(1u, relationInfo->getNodeCount());
    map<ConnectionID, bool>::const_iterator it =
            relationInfo->nodesBegin();
    ASSERT_EQ(connID, it->first);
    ASSERT_FALSE(it->second);
    ASSERT_EQ(relationInfo->nodesEnd(), ++it);
}

TEST_F(RelationInfoTest, testAddNodes)
{
    ASSERT_EQ(0u, relationInfo->getNodeCount());
    vector<ConnectionID> nodesToAdd;
    nodesToAdd.push_back(3);
    nodesToAdd.push_back(9);
    relationInfo->addNodes(nodesToAdd);
    ASSERT_EQ(2u, relationInfo->getNodeCount());
    map<ConnectionID, bool>::const_iterator it =
            relationInfo->nodesBegin();
    ASSERT_EQ(3u, it->first);
    ASSERT_FALSE(it->second);
    ASSERT_EQ(9u, (++it)->first);
    ASSERT_FALSE(it->second);
    ASSERT_EQ(relationInfo->nodesEnd(), ++it);
}

TEST_F(RelationInfoTest, testToString)
{
    string dbName("myDB");
    ASSERT_STREQ("myDatabasexDBSxmyRelation",
            relationInfo->toString().c_str());
}

TEST_F(RelationInfoTest, testGetIdentifier)
{
    string dbName("myDB");
    string relName("myRel");
    ASSERT_STREQ("myDBxDBSxmyRel",
            RelationInfo::getIdentifier(dbName, relName).c_str());
}

TEST_F(RelationInfoTest, testUpdateReplicationStatus)
{
    relationInfo->addNode(1);
    ASSERT_FALSE(relationInfo->nodesBegin()->second);
    relationInfo->updateReplicationStatus(1, true);
    ASSERT_TRUE(relationInfo->nodesBegin()->second);
}

TEST_F(RelationInfoTest, testParseIdentifier)
{
    string dbName("myDB");
    string relName("myRel");
    string relID = RelationInfo::getIdentifier(dbName, relName);
    string parsedDBName;
    string parsedRelName;
    RelationInfo::parseIdentifier(relID, parsedDBName, parsedRelName);
    ASSERT_STREQ(dbName.c_str(), parsedDBName.c_str());
    ASSERT_STREQ(parsedRelName.c_str(), parsedRelName.c_str());
}

}

}
