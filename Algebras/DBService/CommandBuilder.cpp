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
    default:
        return string("ERROR");
    }
}

string CommandBuilder::buildCreateCommand(
        const string& relationName,
        const RelationDefinition& rel,
        const vector<string>& values)
{
    if(rel.size() != values.size())
    {
        throw new SecondoException("rel.size() != values.size()");
    }
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
    for(size_t i = 0; i < rel.size(); i++)
    {
        if(rel[i].type == STRING)
        {
            createCommand << "\"";
        }
        createCommand << values[i];
        if(rel[i].type == STRING)
        {
            createCommand << "\"";
        }
        if(i != rel.size() - 1)
        {
            createCommand << ", ";
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
        if(rel[i].type == STRING)
        {
            insertCommand << "\"";
        }
        insertCommand << values[i];
        if(rel[i].type == STRING)
        {
            insertCommand << "\"";
        }
        if(i != rel.size() - 1)
        {
            insertCommand << ", ";
        }
    }
    insertCommand << "] consume";
    return insertCommand.str();
}

} /* namespace DBService */
