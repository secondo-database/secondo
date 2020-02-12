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

#include "PGraphMem.h"

namespace pgraph {

 class NodeRelInfo
{
   public: 
      string name;
      string idattr;
};

class EdgeRelInfo
{
public:
   string EdgeRelName;
   string FromIdName;
   string FromRelName;
   string FromRelKeyName;
   string ToIdName;
   string ToRelName;
   string ToRelKeyName;
};


//-----------------------------------------------------------------------------
class PGraph 
{
   public:
      // constructors and destructors
   PGraph() {}

   PGraph(int maxsize){ _maxsize=maxsize; }

   PGraph( const PGraph& pg )
   {
      _maxsize = pg._maxsize;
   }

   ~PGraph(); 


   // auxiliary functions
   int GetMaxSize() const { return _maxsize; }


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

   // type name and checking
   static const string BasicType() { return "pgraph"; }

   static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
   }

   static const bool checkType(const NList type){
      return type.isSymbol(BasicType());
   }

public:

   // private data
   int _maxsize;
   string name;
   std::map<string, NodeRelInfo*> _nodeRelations;
   std::map<string, EdgeRelInfo*> _edgeRelations;
   std::map<string, IndexInfo*> _nodeIndexes;

public:

   // define some friends 
   friend struct  ConstructorFunctions<PGraph>;

   // auxiliary functions
   void AddNodesRel(string relname, string idattrname="");
   void AddEdgeRel(string edgerelname, string fieldfrom, string relfrom, 
         string keyrelfrom, 
         string fieldto, string relto, string keyrelto);
   void AddNodeIndex(string noderelname, string propfieldname, 
        string indexname);

   void   ClearRelInfos();
   string DumpInfo(MemoryGraphObject* pgm = NULL);
};


//-----------------------------------------------------------------------------

struct pgraphInfo : ConstructorInfo {

  pgraphInfo() {

    name         = PGraph::BasicType();
    signature    = "-> " + Kind::SIMPLE();
    typeExample  = PGraph::BasicType();
    listRep      =  "(<maxsize>)";
    valueExample = "(1000)";
    remarks      = "max size of adjacency list";
  }
};

//------------------------------------------------------------------------------
struct pgraphFunctions : ConstructorFunctions<PGraph> {

  pgraphFunctions()
  {
    // re-assign some function pointers
    create = PGraph::Create;
    in = PGraph::In;
    out = PGraph::Out;

    open = PGraph::Open;
    save = PGraph::Save;
  }
};

extern pgraphInfo pgri;
extern pgraphFunctions pgrf;
extern  TypeConstructor pgraphTC;

}


#endif