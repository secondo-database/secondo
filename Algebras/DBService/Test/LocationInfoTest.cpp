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

#include "LocationInfo.hpp"

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
: host("myHost"), port("12345"), disk("myDisk"),
  commPort("98765"), transferPort("65432")
{}

protected:
    string host;
    string port;
    string disk;
    string commPort;
    string transferPort;
};

TEST_F(LocationInfoTest, testGetHost)
{
    LocationInfo locationInfo(host,
                              port,
                              disk,
                              commPort,
                              transferPort);
    ASSERT_STREQ(host.c_str(), locationInfo.getHost().c_str());
}

TEST_F(LocationInfoTest, testGetPort)
{
    LocationInfo locationInfo(host,
                              port,
                              disk,
                              commPort,
                              transferPort);
    ASSERT_STREQ(port.c_str(), locationInfo.getPort().c_str());
}

TEST_F(LocationInfoTest, testGetDisk)
{
    LocationInfo locationInfo(host,
                              port,
                              disk,
                              commPort,
                              transferPort);
    ASSERT_STREQ(disk.c_str(), locationInfo.getDisk().c_str());
}

TEST_F(LocationInfoTest, testGetCommPort)
{
    LocationInfo locationInfo(host,
                              port,
                              disk,
                              commPort,
                              transferPort);
    ASSERT_STREQ(commPort.c_str(), locationInfo.getCommPort().c_str());
}

TEST_F(LocationInfoTest, testGetTransferPort)
{
    LocationInfo locationInfo(host,
                              port,
                              disk,
                              commPort,
                              transferPort);
    ASSERT_STREQ(transferPort.c_str(), locationInfo.getTransferPort().c_str());
}

}

}
