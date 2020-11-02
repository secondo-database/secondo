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

#ifndef QUERYOUTPUTFIELDS_H
#define QUERYOUTPUTFIELDS_H

#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include <list> 


namespace pgraph {

class MemoryGraphObject;
enum AliasType { AliasIsNode, AliasIsEdge };

//-----------------------------------------------------------------------------
class QueryOutputField
{
public:
   std::string NodeAlias="";
   std::string PropertyName="";
   std::string OutputName="";
   int    index;
   std::string datatype;
};

//-----------------------------------------------------------------------------
class QueryAliasInfo
{
public:
   AliasType Type=AliasIsNode;
   std::string AliasName="";
   std::string TypeName="";
};

//-----------------------------------------------------------------------------
class QueryAliasList
{
public:
   std::list<QueryAliasInfo> list;

   void AddNode(std::string aliasname, std::string typename_);
   void AddEdge(std::string aliasname, std::string typename_);
   void Update(std::string aliasname, std::string typename_);
};

//-----------------------------------------------------------------------------
class QueryOutputFields
{
public:
   std::list<QueryOutputField*> Fields;

   void ReadFromList(std::string list);
   void ReadFromList(ListExpr list);
   ListExpr StreamTypeDefinition(QueryAliasList *aliaslist, 
       MemoryGraphObject *pgmem);
};


} // namespace
#endif