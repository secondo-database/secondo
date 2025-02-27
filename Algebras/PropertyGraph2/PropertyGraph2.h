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

#ifndef PGRAPH_H
#define PGRAPH_H

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"

#include "PropertyGraphMem.h"

namespace pgraph2 {

class StatInfo
{
   public:
      double value;
};

class NodeRelInfo
{
   public:
      std::string name;
      std::string idattr;
      int StatCardinality=-1;
};

class IndexInfo
{
   public:
      std::string name;
      std::string attr;
};

class EdgeRelInfo
{
public:
   std::string EdgeRelName;
   std::string FromIdName;
   std::string FromRelName;
   std::string FromRelKeyName;
   std::string ToIdName;
   std::string ToRelName;
   std::string ToRelKeyName;
   double StatAvgForward=-1;
   double StatAvgBackward=-1;
   int StatCardinality=-1;
};


//-----------------------------------------------------------------------------
class PGraph2
{
   public:
      // constructors and destructors
   PGraph2() {}

   ~PGraph2();


   // lifecycle
   static Word     Create( const ListExpr typeInfo );

   static Word     In( const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct );

   static ListExpr Out( ListExpr typeInfo, Word value );


   // Persistence Functions
   static bool     Open( SmiRecord& valueRecord,
                        size_t& offset, const ListExpr typeInfo,
                        Word& value );

   static bool     Save( SmiRecord& valueRecord, size_t& offset,
                        const ListExpr typeInfo, Word& w );

    static void    Deletion(const ListExpr typeInfo,  Word& object );


   // type name and checking
   static const std::string BasicType() { return "pgraph"; }

   static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
   }

   static const bool checkType(const NList type){
      return type.isSymbol(BasicType());
   }

public:

   // private data
   bool dumpQueryTree=false;
   bool dumpGraph=false;
   std::string structure="placeholder";
   std::string name;
   std::map<std::string, NodeRelInfo*> _nodeRelations;
   std::map<std::string, EdgeRelInfo*> _edgeRelations;
   std::map<std::string, IndexInfo*> _Indexes;

public:

   // define some friends
   friend struct  ConstructorFunctions<PGraph2>;

   // auxiliary functions
   NodeRelInfo* AddNodesRel(std::string relname, std::string idattrname="");
   EdgeRelInfo *AddEdgeRel(std::string edgerelname, std::string fieldfrom,
         std::string relfrom,
         std::string keyrelfrom,
         std::string fieldto, std::string relto, std::string keyrelto);
   IndexInfo* AddIndex(std::string relname, std::string attrname);

   void   ClearRelInfos();
   void   ClearStat();

   std::string DumpInfo(MemoryGraphObject* pgm = NULL);
};


//-----------------------------------------------------------------------------

struct pgraphInfo : ConstructorInfo {

  pgraphInfo() {

    name         = PGraph2::BasicType();
    signature    = "-> " + Kind::SIMPLE();
    typeExample  = PGraph2::BasicType();
    listRep      =  "(name)";
    valueExample = "(\"p2\")";
    remarks      = "name of property graph";
  }
};

//------------------------------------------------------------------------------
struct pgraphFunctions : ConstructorFunctions<PGraph2> {

  pgraphFunctions()
  {
    // re-assign some function pointers
    create = PGraph2::Create;
    in = PGraph2::In;
    out = PGraph2::Out;
    deletion=PGraph2::Deletion;

    open = PGraph2::Open;
    save = PGraph2::Save;
  }
};

extern pgraphInfo pgri2;
extern pgraphFunctions pgrf2;
extern TypeConstructor pgraph2TC;

}


#endif
