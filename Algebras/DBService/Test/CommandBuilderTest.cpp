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

#include <queue>

#include "SecondoException.h"

#include "CommandBuilder.hpp" //TODO proper path


using namespace std;

namespace DBService
{

namespace Test
{

class TestBuilder : public CommandBuilder
{
public:
    static void addAttributeValue(
            stringstream& stream,
            const AttributeInfo& info,
            const string& value)
    {
        CommandBuilder::addAttributeValue(stream, info, value);
    }
};

class CommandBuilderTest: public ::testing::Test
{
public:
    CommandBuilderTest() : relationName("myRel")
{
    rel.push_back({AttributeType::STRING, string("Col1")});
    values.push_back(string("val1"));
    rel.push_back({AttributeType::INT, string("Col2")});
    values.push_back(string("2"));
    rel.push_back({AttributeType::BOOL, string("Col3")});
    values.push_back(string("FALSE"));
}

protected:
    string relationName;
    RelationDefinition rel;
    vector<string> values;
};

TEST_F(CommandBuilderTest, testGetTypeNameString)
{
    ASSERT_STREQ("string",
            CommandBuilder::getTypeName(AttributeType::STRING).c_str());
}

TEST_F(CommandBuilderTest, testGetTypeNameInt)
{
    ASSERT_STREQ("int",
            CommandBuilder::getTypeName(AttributeType::INT).c_str());
}

TEST_F(CommandBuilderTest, testGetTypeNameBool)
{
    ASSERT_STREQ("bool",
            CommandBuilder::getTypeName(AttributeType::BOOL).c_str());
}

TEST_F(CommandBuilderTest, testGetTypeNameWrongType)
{
    ASSERT_STREQ("ERROR",
            CommandBuilder::getTypeName(AttributeType(99)).c_str());
}

TEST_F(CommandBuilderTest, testAddAttributeValueString)
{
    stringstream stream;
    TestBuilder::addAttributeValue(stream, rel[0], values[0]);
    ASSERT_STREQ("\"val1\"", stream.str().c_str());
}

TEST_F(CommandBuilderTest, testAddAttributeValueInt)
{
    stringstream stream;
    TestBuilder::addAttributeValue(stream, rel[1], values[1]);
    ASSERT_STREQ("2", stream.str().c_str());
}

TEST_F(CommandBuilderTest, testAddAttributeValueBool)
{
    stringstream stream;
    TestBuilder::addAttributeValue(stream, rel[2], values[2]);
    ASSERT_STREQ("FALSE", stream.str().c_str());
}

TEST_F(CommandBuilderTest, testBuildCreateCommand)
{
    string expectedCreateCommand("let myRel = [const rel(tuple([Col1: string,"
            " Col2: int, Col3: bool])) value((\"val1\" 2 FALSE))]");
    ASSERT_STREQ(expectedCreateCommand.c_str(),
            CommandBuilder::buildCreateCommand(
                    relationName, rel, values).c_str());
}

TEST_F(CommandBuilderTest, testBuildInsertCommand)
{
    string expectedInsertCommand("query myRel inserttuple["
            "\"val1\", 2, FALSE] consume");
    ASSERT_STREQ(expectedInsertCommand.c_str(),
            CommandBuilder::buildInsertCommand(
                    relationName, rel, values).c_str());
}

TEST_F(CommandBuilderTest, testBuildCreateCommandThrowsIfLenghtsDoNotMatch)
{
    vector<string> wrongValues;

    ASSERT_THROW(CommandBuilder::buildCreateCommand(
                    relationName, rel, wrongValues), SecondoException*);
}

TEST_F(CommandBuilderTest, testBuildInsertCommandThrowsIfLenghtsDoNotMatch)
{
    vector<string> wrongValues;
    ASSERT_THROW(CommandBuilder::buildInsertCommand(
                    relationName, rel, wrongValues), SecondoException*);
}

TEST_F(CommandBuilderTest, testBuildUpdateCommand)
{
    string expectedUpdateCommand("query myRel feed filter[.Col1 = \"val1\"]"
            " filter[.Col2 = 2] myRel updatedirect [Col3: TRUE] consume");
    FilterConditions filterConditions =
    {
        { {AttributeType::STRING, string("Col1") }, "val1" },
        { {AttributeType::INT, string("Col2") }, "2" }
    };
    AttributeInfoWithValue valueToUpdate =
    { {AttributeType::BOOL, string("Col3") }, "TRUE" };
    ASSERT_STREQ(expectedUpdateCommand.c_str(),
            CommandBuilder::buildUpdateCommand(
                    relationName, filterConditions, valueToUpdate).c_str());
}

TEST_F(CommandBuilderTest, testBuildDeleteCommand)
{
    string expectedDeleteCommand("query myRel feed filter[.Col1 = \"val1\"]"
            " filter[.Col2 = 2] filter[.Col3 = FALSE]"
            " myRel deletedirect consume");
    FilterConditions filterConditions =
    {
        { {AttributeType::STRING, string("Col1") }, "val1" },
        { {AttributeType::INT, string("Col2") }, "2" },
        { {AttributeType::BOOL, string("Col3") }, "FALSE" }
    };
    ASSERT_STREQ(expectedDeleteCommand.c_str(),
                CommandBuilder::buildDeleteCommand(
                        relationName, filterConditions).c_str());
}

}/* namespace Test */

}/* namespace DBService */
