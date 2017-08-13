/*

1.1.1 Class Implementation

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

*/
#include <sstream>

#include "SecondoException.h"

#include "Algebras/DBService/CommandBuilder.hpp"

using namespace std;

namespace DBService
{

string CommandBuilder::getTypeName(AttributeType type)
{
    switch(type)
    {
    case STRING:
        return string("string");
    case INT:
        return string("int");
    case BOOL:
        return string("bool");
    default:
        return string("ERROR");
    }
}

void CommandBuilder::addAttributeValue(
        stringstream& stream,
        const AttributeInfo& info,
        const string& value)
{
    if(info.type == STRING)
    {
        stream << "\"";
    }
    stream << value;
    if(info.type == STRING)
    {
        stream << "\"";
    }
}

string CommandBuilder::buildCreateCommand(
        const string& relationName,
        const RelationDefinition& rel,
        const vector<vector<string> >& values)
{
    stringstream createCommand;
    createCommand << "let " << relationName << " = [const rel(tuple([";
    for(size_t i = 0; i < rel.size(); i++)
    {
        createCommand << rel[i].name << ": "
                      << getTypeName(rel[i].type);
        if(i != rel.size() - 1)
        {
            createCommand << ", ";
        }
    }
    createCommand << "])) value((";

    for(size_t j = 0; j < values.size(); j++)
    {
        if(rel.size() != values[j].size())
        {
            throw new SecondoException("wrong number of attributes");
        }
        for(size_t i = 0; i < values[j].size(); i++)
        {
            addAttributeValue(
                    createCommand, rel[i], values[j][i]);
            if(i != rel.size() - 1)
            {
                createCommand << " ";
            }
        }
        if(j != values.size() - 1)
        {
            createCommand << ")\n(";
        }
    }
    createCommand << "))]";
    return createCommand.str();
}

string CommandBuilder::buildInsertCommand(const string& relationName,
                                          const RelationDefinition& rel,
                                          const vector<string>& values)
{
    if(rel.size() != values.size())
    {
        throw new SecondoException("rel.size() != values.size()");
    }
    stringstream insertCommand;
    insertCommand << "query " << relationName << " inserttuple[";
    for(size_t i = 0; i < rel.size(); i++)
    {
        addAttributeValue(insertCommand, rel[i], values[i]);
        if(i != rel.size() - 1)
        {
            insertCommand << ", ";
        }
    }
    insertCommand << "] consume";
    return insertCommand.str();
}

string CommandBuilder::buildUpdateCommand(
        const string& relationName,
        const FilterConditions& filterConditions,
        const AttributeInfoWithValue& valueToUpdate)
{
    stringstream updateCommand;
    updateCommand << "query " << relationName << " feed ";
    for(auto condition : filterConditions)
    {
        updateCommand << "filter[." << condition.attributeInfo.name << " = ";
        addAttributeValue(
                updateCommand,
                condition.attributeInfo,
                condition.value);
        updateCommand << "] ";
    }
    updateCommand << relationName << " updatedirect [";
    updateCommand << valueToUpdate.attributeInfo.name << ": ";
    addAttributeValue(
            updateCommand,
            valueToUpdate.attributeInfo,
            valueToUpdate.value);
    updateCommand << "] consume";
    return updateCommand.str();
}

string CommandBuilder::buildDeleteCommand(
        const string& relationName,
        const vector<AttributeInfoWithValue>& filterConditions)
{
    stringstream deleteCommand;
    deleteCommand << "query " << relationName << " feed ";
    for(auto condition : filterConditions)
    {
        deleteCommand << "filter[." << condition.attributeInfo.name << " = ";
        addAttributeValue(
                deleteCommand,
                condition.attributeInfo,
                condition.value);
        deleteCommand << "] ";
    }
    deleteCommand << relationName << " deletedirect consume";
    return deleteCommand.str();
}


} /* namespace DBService */
