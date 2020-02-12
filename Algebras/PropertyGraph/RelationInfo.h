/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

#ifndef RELATIONINFO_H_
#define RELATIONINFO_H_

#include <string>

#include "NestedList.h"

//------------------------------------------------------------------
namespace pgraph
{

class IndexInfo
{
public:
   std::string NodeType;
   std::string PropName;
   std::string IndexName;
};


//------------------------------------------------------------------
class AttrInfo
{
public:
    int    Index=-1;
    std::string Name;    
    std::string TypeName;
    std::string GetStringVal(Tuple *tuple);
};

//------------------------------------------------------------------
class RelationSchemaInfo
{
    vector<AttrInfo*> Attributes;
public:    
    RelationSchemaInfo();
    RelationSchemaInfo(ListExpr relation);
    void LoadFromList(ListExpr relation);
    AttrInfo* GetAttrInfo(std::string Name);
    AttrInfo* GetAttrInfo(int index);
    int GetAttrCount();
    void Dump();
};


} // namespace

#endif // PGRAPH_UTILS_H_