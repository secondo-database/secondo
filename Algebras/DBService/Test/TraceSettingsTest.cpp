/*

1.1 ~TraceSettingsTest~

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

#include "TraceSettings.hpp"

using namespace std;

namespace DBService
{

namespace Test
{

class TraceSettingsTest: public ::testing::Test
{};

TEST_F(TraceSettingsTest, testInitialTraceLevel)
{
    ASSERT_EQ(TraceLevel::DEBUG, TraceSettings::getInstance()->getTraceLevel());
}

TEST_F(TraceSettingsTest, testTraceOff)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::OFF);
    ASSERT_EQ(TraceLevel::OFF, TraceSettings::getInstance()->getTraceLevel());
}

TEST_F(TraceSettingsTest, testTraceFile)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::FILE);
    ASSERT_EQ(TraceLevel::FILE, TraceSettings::getInstance()->getTraceLevel());
}

TEST_F(TraceSettingsTest, testTraceDebug)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::DEBUG);
    ASSERT_EQ(TraceLevel::DEBUG, TraceSettings::getInstance()->getTraceLevel());
}

TEST_F(TraceSettingsTest, testIsDebugTraceOn_On)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::DEBUG);
    ASSERT_TRUE(TraceSettings::getInstance()->isDebugTraceOn());
}

TEST_F(TraceSettingsTest, testIsDebugTraceOn_Off)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::FILE);
    ASSERT_FALSE(TraceSettings::getInstance()->isDebugTraceOn());
}

TEST_F(TraceSettingsTest, testIsFileTraceOn_On)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::FILE);
    ASSERT_TRUE(TraceSettings::getInstance()->isFileTraceOn());
}

TEST_F(TraceSettingsTest, testIsFileTraceOn_Off)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::OFF);
    ASSERT_FALSE(TraceSettings::getInstance()->isFileTraceOn());
}

TEST_F(TraceSettingsTest, testIsFileTraceOn_Debug)
{
    TraceSettings::getInstance()->setTraceLevel(TraceLevel::DEBUG);
    ASSERT_TRUE(TraceSettings::getInstance()->isFileTraceOn());
}

}

}
