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

#include <string>

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "Symbols.h"

#include "../MainMemory2/MPointer.h"
#include "../MainMemory2/Mem.h"
#include "../MainMemory2/MemoryObject.h"
#include "../MainMemory2/MemCatalog.h"

#include "Utils.h"
#include "PGraph.h"

using namespace std;


extern NestedList* nl;
extern QueryProcessor *qp;

namespace pgraph {

//-----------------------------------------------------------------------------
PGraph::~PGraph()
{
   ClearRelInfos();
}

//-----------------------------------------------------------------------------
Word PGraph::Create( const ListExpr typeInfo )
{
   LOG(20,"PGraph::Create");
   PGraph* pg=new PGraph( 0 );
   return (SetWord( pg ));
}

//-----------------------------------------------------------------------------

Word PGraph::In( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct )
{
   Word result = SetWord(Address(0));
   correct = false;
 
   NList list(instance);
   NList typeinfo(typeInfo);
   
   LOGOP(20,"PGraph::In");
   LOG(20,"instance",list);
   LOG(20,"typeinfo",typeinfo);

   // argument count
   if ( list.length() != 6 )  {
     cout << "PGraph::In - Invalid number of arguments!\n";
     return result;
   }

  // maxsize
   int ms =100;
   NList maxsize = list.second();
   if ( maxsize.isInt() ) 
      ms = maxsize.intval();
   else 
      cout << "PGraph::In - second argument not an integer\n";

   // create PGraph
   PGraph* pg = new PGraph(ms);
   result.addr = pg;

   pg->name=list.third().str();

   // read node relationnames
   NList noderels = list.fourth().second();
   for(Cardinal i=0; i<noderels.length(); i++)
   {
      NList tableinfo=noderels.elem(i+1);
      pg->AddNodesRel( tableinfo.first().str(), tableinfo.second().str() );
   }
   // read node relationnames
   NList edgerels = list.fifth().second();
   for(Cardinal i=0; i<edgerels.length(); i++)
   {
      NList tableinfo=edgerels.elem(i+1);
      pg->AddEdgeRel( tableinfo.first().str(), 
                      tableinfo.second().first().str(),
                      tableinfo.second().second().str(),
                      tableinfo.second().third().str(),
                      tableinfo.third().first().str(),
                      tableinfo.third().second().str(),
                      tableinfo.third().third().str() );
   }
   // read node indexes
   NList indexes = list.sixth().second();
   for(Cardinal i=0; i<indexes.length(); i++)
   {
      NList indexinfo=indexes.elem(i+1);
      pg->AddNodeIndex( indexinfo.first().str(), 
                      indexinfo.second().str(),
                      indexinfo.third().str() );
   }

   //
   correct = true;
   return result;
}

//-----------------------------------------------------------------------------
ListExpr PGraph::Out( ListExpr typeInfo, Word value )
{
   PGraph* pg = static_cast<PGraph*>( value.addr );
   
   NList nodetables=NList();
   NList edgetables=NList();
   NList nodeindexes=NList();
   
   for(std::map<string, NodeRelInfo*>::iterator 
        itr = pg->_nodeRelations.begin(); 
       itr != pg->_nodeRelations.end(); itr++)
   {
      NList tableinfo= NList(  itr->second->name ,itr->second->idattr  );
      nodetables.append( tableinfo );
   }

   for(std::map<string, EdgeRelInfo*>::iterator 
        itr = pg->_edgeRelations.begin(); 
        itr != pg->_edgeRelations.end(); itr++)
   {
      NList tableinfo= NList(  itr->second->EdgeRelName ,
                               NList(itr->second->FromIdName,
                                 itr->second->FromRelName,
                                 itr->second->FromRelKeyName),
                               NList(itr->second->ToIdName,
                               itr->second->ToRelName,
                               itr->second->ToRelKeyName)  );
      edgetables.append( tableinfo );
   }

   for(std::map<string, IndexInfo*>::iterator itr = pg->_nodeIndexes.begin(); 
       itr != pg->_nodeIndexes.end(); itr++)
   {
      NList indexinfo= NList(  itr->second->NodeType ,
                               itr->second->PropName,
                               itr->second->IndexName );
      nodeindexes.append( indexinfo );
   }
   NList res(

      NList(0),
      NList( pg->GetMaxSize()   ) ,
      NList( pg->name  ) ,
      NList( NList("nodetables"),  nodetables  ), 
      NList( NList("edgetables"),  edgetables  ), 
      NList( NList("nodeindexes"),  nodeindexes  ) 
   );

   LOGOP(20, "PGraph::Out ", res);
   return res.listExpr();
}

//-----------------------------------------------------------------------------
bool PGraph::Open( SmiRecord& valueRecord,
                  size_t& offset, const ListExpr typeInfo,
                  Word& value )
{
   LOG(20,"PGraph::Open");
   return DefOpen(PGraph::In, valueRecord, offset, typeInfo, value);
}


//-----------------------------------------------------------------------------
bool PGraph::Save( SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value)
{
  LOG(20,"PGraph::Save");

   ListExpr valueList;
   string valueString;
   int valueLength;

   valueList = Out( nl->First(typeInfo), value );
   valueList = nl->OneElemList( valueList );
   nl->WriteToString( valueString, valueList );
   valueLength = valueString.length();
   valueRecord.Write( &valueLength, sizeof( valueLength ), offset );
   offset += sizeof( valueLength );
   valueRecord.Write( valueString.data(), valueString.length(), offset );
   offset += valueString.length();
   //cout <<"SAVEDATA: "<<valueString.data()<<endl;
   nl->Destroy( valueList );
   return (true);  
}

//-----------------------------------------------------------------------------
void PGraph::AddEdgeRel(string edgerelname, string fieldfrom, string relfrom, 
       string keyrelfrom, string fieldto, string relto, string keyrelto)
{
  EdgeRelInfo *ei = new EdgeRelInfo();
  ei->EdgeRelName = edgerelname;
  ei->FromIdName = fieldfrom;  
  ei->FromRelName = relfrom;
  ei->FromRelKeyName = keyrelfrom;
  ei->ToIdName = fieldto;  
  ei->ToRelName = relto;
  ei->ToRelKeyName = keyrelto;
   _edgeRelations[edgerelname] = ei;
}

//-----------------------------------------------------------------------------
void PGraph::AddNodeIndex(string noderelname, string propfieldname, 
      string indexname)
{
  IndexInfo *ei = new IndexInfo();
  ei->NodeType = noderelname;
  ei->PropName = propfieldname;  
  ei->IndexName = indexname;
   _nodeIndexes[ei->NodeType+"."+ei->PropName] = ei;
}

//-----------------------------------------------------------------------------
void PGraph::AddNodesRel(string relname, string idattrname)
{
  NodeRelInfo *ni = new NodeRelInfo();
  ni->name=relname;
  ni->idattr=idattrname;
  _nodeRelations[relname] = ni;
}

//------------------------------------------------------------------------------
void PGraph::ClearRelInfos()
{
   for(auto&& r : _nodeRelations) 
        delete r.second;
   _nodeRelations.clear();
   for(auto&& r : _edgeRelations) 
        delete r.second;
   _edgeRelations.clear();
}

//-----------------------------------------------------------------------------
string PGraph::DumpInfo(MemoryGraphObject *pgm)
{
   string info="PGRAPH Information\n";
   info += " - name    : "+name + "\n";
   info += " - maxsize : "+std::to_string(GetMaxSize()) + "\n";
   info += " - node relations \n";
   for(std::map<string, NodeRelInfo*>::iterator itr = _nodeRelations.begin(); 
        itr != _nodeRelations.end(); itr++)
   {
      info += "    - " + itr->second->name  + " ("+itr->second->idattr+")"+"\n";
   }
   info += " - edge relations \n";
   for(std::map<string, EdgeRelInfo*>::iterator itr = _edgeRelations.begin(); 
        itr != _edgeRelations.end(); itr++)
   {
      info += "    - " + itr->second->EdgeRelName  + 
         "    (FROM "+itr->second->FromIdName+"=>"+itr->second->FromRelName+
                    "."+itr->second->FromRelKeyName+")"+"\n";
         "    (TO   "+itr->second->ToIdName+"=>"+itr->second->ToRelName+
                "."+itr->second->ToRelKeyName+")"+"\n";
   }
   info += " - node indexes \n";
   for(std::map<string, IndexInfo*>::iterator itr = _nodeIndexes.begin(); 
        itr != _nodeIndexes.end(); itr++)
   {
      info += "    - " + itr->second->NodeType +"."+ itr->second->PropName 
           +" <- "+ itr->second->IndexName +"\n" ;
   }

   // add information dfomfrom memory object
   info += "Memory object information: \n";
   if (pgm==NULL)
   {
      info+="!NOT LOADED\n";
   }
   else
   {
      info += pgm->DumpInfo()+"\n";
   }

   return info;
}

//-----------------------------------------------------------------------------

pgraphInfo pgri;
pgraphFunctions pgrf;
TypeConstructor pgraphTC( pgri, pgrf );

//-----------------------------------------------------------------------------



};
