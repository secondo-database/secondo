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

#include "CommunicationUtils.hpp"

#include <queue>

using namespace std;

namespace DBService
{

namespace Test
{

class CommunicationUtilsTest: public ::testing::Test
{
protected:
    bool meetsExpectation(iostream& io, const string& expected)
    {
        string line;
        getline(io, line);
        return line == expected;
    }
};

TEST_F(CommunicationUtilsTest, testReceiveUnexpected)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    string blub("blub");
    io << bla << endl;
    ASSERT_FALSE(CommunicationUtils::receivedExpectedLine(io, blub));
}

TEST_F(CommunicationUtilsTest, testReceiveBatchExpected)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    string blub("blub");

    io << bla << endl;
    io << blub << endl;
    io << bla << endl;

    queue<string> q;
    q.push(bla);
    q.push(blub);
    q.push(bla);

    ASSERT_TRUE(CommunicationUtils::receivedExpectedLines(io, q));
    ASSERT_TRUE(q.empty());
}

TEST_F(CommunicationUtilsTest, testReceiveBatchUnexpected)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    string blub("blub");

    io << bla << endl;
    io << blub << endl;
    io << bla << endl;

    queue<string> q;
    q.push(blub);
    q.push(bla);
    q.push(blub);

    ASSERT_FALSE(CommunicationUtils::receivedExpectedLines(io, q));
    ASSERT_TRUE(q.empty());
}

TEST_F(CommunicationUtilsTest, testReceiveExpected)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    io << bla << endl;
    ASSERT_TRUE(CommunicationUtils::receivedExpectedLine(io, bla));
}

TEST_F(CommunicationUtilsTest, testReceiveLine)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    io << bla << endl;

    string line;
    CommunicationUtils::receiveLine(io, line);
    ASSERT_STREQ(bla.c_str(), line.c_str());
}

TEST_F(CommunicationUtilsTest, testReceiveLines)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    string blub("blub");

    io << bla << endl;
    io << blub << endl;
    io << blub << endl;

    queue<string> q;
    CommunicationUtils::receiveLines(io, 3, q);
    ASSERT_EQ(3, q.size());

    ASSERT_STREQ(bla.c_str(), q.front().c_str());
    q.pop();
    ASSERT_STREQ(blub.c_str(), q.front().c_str());
    q.pop();
    ASSERT_STREQ(blub.c_str(), q.front().c_str());
    q.pop();
}

TEST_F(CommunicationUtilsTest, testSend)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    CommunicationUtils::sendLine(io, bla);
    ASSERT_TRUE(meetsExpectation(io, bla));
}

TEST_F(CommunicationUtilsTest, testSendBatch)
{
    stringbuf buffer;
    iostream io(&buffer);
    string bla("bla");
    string blub("blub");

    queue<string> q;
    q.push(bla);
    q.push(blub);
    q.push(bla);
    ASSERT_EQ(3, q.size());

    CommunicationUtils::sendBatch(io, q);
    ASSERT_TRUE(q.empty());

    ASSERT_TRUE(meetsExpectation(io, bla));
    ASSERT_TRUE(meetsExpectation(io, blub));
    ASSERT_TRUE(meetsExpectation(io, bla));
}

}

}
