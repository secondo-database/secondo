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

#include "LocationInfo.hpp" // TODO

#include <queue>

using namespace std;

namespace DBService
{

namespace Test
{

class LocationInfoTest: public ::testing::Test
{
public:
    LocationInfoTest()
: host("localhost"), port("12345"), config("myConfig"), disk("/opt/sec/myDisk"),
  commPort("98765"), transferPort("65432"), locationInfo(0)
{}

void SetUp()
{
    locationInfo = new LocationInfo(host,
                                    port,
                                    config,
                                    disk,
                                    commPort,
                                    transferPort);    
}

void TearDown()
{
    if(locationInfo)
    {
        delete locationInfo;
        locationInfo = 0;
    }
}

protected:
    string host;
    string port;
    string config;
    string disk;
    string commPort;
    string transferPort;
    LocationInfo* locationInfo;
};

TEST_F(LocationInfoTest, testGetHost)
{
    ASSERT_STREQ(host.c_str(), locationInfo->getHost().c_str());
}

TEST_F(LocationInfoTest, testGetPort)
{
    ASSERT_STREQ(port.c_str(), locationInfo->getPort().c_str());
}

TEST_F(LocationInfoTest, testGetConfig)
{
    ASSERT_STREQ(config.c_str(), locationInfo->getConfig().c_str());
}

TEST_F(LocationInfoTest, testGetDisk)
{
    ASSERT_STREQ(disk.c_str(), locationInfo->getDisk().c_str());
}

TEST_F(LocationInfoTest, testGetCommPort)
{
    ASSERT_STREQ(commPort.c_str(), locationInfo->getCommPort().c_str());
}

TEST_F(LocationInfoTest, testGetTransferPort)
{
    ASSERT_STREQ(transferPort.c_str(), locationInfo->getTransferPort().c_str());
}

TEST_F(LocationInfoTest, testSameWorker)
{
    ASSERT_TRUE(locationInfo->isSameWorker(host, port));
}

TEST_F(LocationInfoTest, testNotSameWorker)
{
    ASSERT_FALSE(
            locationInfo->isSameWorker(string("blaHost"), string("blaPort")));
}

TEST_F(LocationInfoTest, testSameHost)
{
    ASSERT_TRUE(locationInfo->isSameHost(host));
}

TEST_F(LocationInfoTest, testSameResolvedHost)
{
    // Compares DNS resolved equality between localhost and 127.0.0.1
    ASSERT_TRUE(locationInfo->isSameHost("127.0.0.1"));
}

TEST_F(LocationInfoTest, testNotSameHost)
{
    ASSERT_FALSE(locationInfo->isSameHost(string("blaHost")));
}

TEST_F(LocationInfoTest, testNotSameResolvedHost)
{
    ASSERT_FALSE(locationInfo->isSameHost(string("127.0.0.2")));
}

TEST_F(LocationInfoTest, testSameDiskSameHost)
{
    ASSERT_TRUE(locationInfo->isSameDisk(host, disk));
}

TEST_F(LocationInfoTest, testNotReallyDifferentDiskSameHost)
{
    ASSERT_TRUE(
            locationInfo->isSameDisk(
                    host, string("/opt/secondo/myDisk")));
}

TEST_F(LocationInfoTest, testSameDiskDifferentHost)
{
    ASSERT_FALSE(
            locationInfo->isSameDisk(string("blaHost"), disk));
}

TEST_F(LocationInfoTest, testDifferentDiskSameHost)
{
    ASSERT_FALSE(locationInfo->isSameDisk(
            host, string("/disks/secondo/myDisk")));
}

TEST_F(LocationInfoTest, testDifferentDiskDifferentHost)
{
    ASSERT_FALSE(
            locationInfo->isSameDisk(string("blaHost"), string("blaDisk")));
}

TEST_F(LocationInfoTest, testGetIdentifier)
{    
    ASSERT_STREQ("localhostxDBSx12345",
            LocationInfo::getIdentifier(host, port).c_str());
}

TEST_F(LocationInfoTest, testParseIdentifier)
{
    string locID = LocationInfo::getIdentifier(host, port);
    string parsedHost;
    string parsedPort;
    LocationInfo::parseIdentifier(locID, parsedHost, parsedPort);
    ASSERT_STREQ(host.c_str(), parsedHost.c_str());
    ASSERT_STREQ(port.c_str(), parsedPort.c_str());
}

TEST_F(LocationInfoTest, testSetTransferPort)
{
    string newTransferPort("myNewTransferPort");
    ASSERT_STRNE(
            newTransferPort.c_str(), locationInfo->getTransferPort().c_str());
    locationInfo->setTransferPort(newTransferPort);
    ASSERT_STREQ(
            newTransferPort.c_str(), locationInfo->getTransferPort().c_str());
}

}

}
