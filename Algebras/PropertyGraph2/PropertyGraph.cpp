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

#include "MPointer.h"
#include "Mem.h"
#include "MemoryObject.h"
#include "MemCatalog.h"

#include "Utils.h"
#include "PropertyGraph.h"

using namespace std;


extern NestedList* nl;
extern QueryProcessor *qp;

namespace pgraph {

// forward deklatation: part of PropertyGraphAlgebra.cpp
void removePGraphMemoryParts(PGraph *pg);

//-----------------------------------------------------------------------------
PGraph::~PGraph()
{
   ClearRelInfos();
}

//-----------------------------------------------------------------------------
Word PGraph::Create( const ListExpr typeInfo )
{
   LOG(30,"PGraph::Create");
   PGraph* pg=new PGraph(  );
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
   
   LOGOP(30,"PGraph::In");
   LOG(30,"instance",list);
   LOG(30,"typeinfo",typeinfo);

   // argument count
   if ( list.length() != 6 )  {
     cout << "PGraph::In - Invalid number of arguments!\n";
     return result;
   }

   // create PGraph
   PGraph* pg = new PGraph();
   result.addr = pg;

   pg->name=list.second().str();

   // read node relationnames
   NList noderels = list.third().second();
   for(Cardinal i=0; i<noderels.length(); i++)
   {
      NList tableinfo=noderels.elem(i+1);
      NodeRelInfo*nri= pg->AddNodesRel( tableinfo.first().str(), 
           tableinfo.second().str() );
      nri->StatCardinality=tableinfo.third().intval();
   }
   // read node relationnames
   NList edgerels = list.fourth().second();
   for(Cardinal i=0; i<edgerels.length(); i++)
   {
      NList tableinfo=edgerels.elem(i+1);
      EdgeRelInfo *eri = pg->AddEdgeRel( tableinfo.first().str(), 
                      tableinfo.second().first().str(),
                      tableinfo.second().second().str(),
                      tableinfo.second().third().str(),
                      tableinfo.third().first().str(),
                      tableinfo.third().second().str(),
                      tableinfo.third().third().str() );
      eri->StatAvgForward =tableinfo.fourth().first().realval();
      eri->StatAvgBackward =tableinfo.fourth().second().realval();
   }
   // read indexes
   NList indexes = list.fifth().second();
   for(Cardinal i=0; i<indexes.length(); i++)
   {
      NList indexinfo=indexes.elem(i+1);
      pg->AddIndex( 
            indexinfo.first().str(), 
            indexinfo.second().str());
   }   
   //
   pg->dumpGraph=false;
   pg->dumpQueryTree=false;
   NList options = list.sixth().second();
   for(Cardinal i=0; i<options.length(); i++)
   {
      NList option=options.elem(i+1);  

      if (option.first().str()=="log")
         debugLevel=option.second().intval();
      if (option.first().str()=="dotquery")
         pg->dumpQueryTree=option.second().intval()==1;
      if (option.first().str()=="dotgraph")
         pg->dumpGraph=option.second().intval()==1;
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
   NList options=NList();
   NList indexes=NList();
   
   for(auto&& item : pg->_nodeRelations)
   {
      NList tableinfo= NList(  item.second->name ,item.second->idattr, 
          item.second->StatCardinality  );
      nodetables.append( tableinfo );
   }

   for(auto&& item : pg->_edgeRelations)
   {
      NList tableinfo= NList(  item.second->EdgeRelName ,
                               NList(item.second->FromIdName,
                                 item.second->FromRelName,
                                 item.second->FromRelKeyName),
                               NList(item.second->ToIdName,
                               item.second->ToRelName,
                               item.second->ToRelKeyName),
                               NList( NList(item.second->StatAvgForward),
                                  NList(item.second->StatAvgBackward))
                               );
      edgetables.append( tableinfo );
   }

   for(auto&& item : pg->_Indexes)
   {
      NList indexinfo= NList(  item.second->name ,item.second->attr);
      indexes.append( indexinfo );
   }


   //
   options.append( NList("log", to_string(debugLevel)));
   if (pg->dumpQueryTree) options.append(NList(string("dotquery"),string("1")));
   if (pg->dumpGraph) options.append( NList(string("dotgraph"),string("1")));

   //  
   NList res(
      NList(0),
      NList( pg->name  ) ,
      NList( NList("nodetables"),  nodetables  ), 
      NList( NList("edgetables"),  edgetables  ), 
      NList( NList("indexes"),  indexes  ), 
      NList( NList("options"),  options  )
   );

   LOGOP(30, "PGraph::Out ", res);
   return res.listExpr();
}

//-----------------------------------------------------------------------------
bool PGraph::Open( SmiRecord& valueRecord,
                  size_t& offset, const ListExpr typeInfo,
                  Word& value )
{
   LOG(30,"PGraph::Open");
   return DefOpen(PGraph::In, valueRecord, offset, typeInfo, value);
}

//-----------------------------------------------------------------------------
void  PGraph::Deletion(const ListExpr typeInfo,  Word& object )
{
   PGraph* pg = static_cast<PGraph*>( object.addr );
   LOG(30,"PGraph::Deletion");
   removePGraphMemoryParts(pg);
}
//-----------------------------------------------------------------------------
bool PGraph::Save( SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value)
{
  LOG(30,"PGraph::Save");

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
EdgeRelInfo *PGraph::AddEdgeRel(string edgerelname, string fieldfrom, 
     string relfrom,string keyrelfrom, string fieldto, string relto, 
     string keyrelto)
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
   return ei;
}

//-----------------------------------------------------------------------------
NodeRelInfo* PGraph::AddNodesRel(string relname, string idattrname)
{
  NodeRelInfo *ni = new NodeRelInfo();
  ni->name=relname;
  ni->idattr=idattrname;
  _nodeRelations[relname] = ni;
  return ni;
}

//-----------------------------------------------------------------------------
IndexInfo* PGraph::AddIndex(std::string relname, std::string attrname)
{
  IndexInfo *ni = new IndexInfo();
  ni->name=relname;
  ni->attr=attrname;
  _Indexes[relname+"."+attrname] = ni;
  return ni;
}

//------------------------------------------------------------------------------
void PGraph::ClearStat()
{
   for(auto&& r : _nodeRelations) 
      r.second->StatCardinality=-1;
   for(auto&& r : _edgeRelations)  {
      r.second->StatAvgForward=-1;
      r.second->StatAvgBackward=-1;
   }
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
   info += " - node relations \n";
   for(auto && item:_nodeRelations)
   {
      info += "    - " + item.second->name  + " ("+item.second->idattr+")";
      if (item.second->StatCardinality>-1) 
                   info+= " [Stat:"+to_string(item.second->StatCardinality)+"]";
      info += "\n";
   }
   info += " - edge relations \n";
   for(auto&& item: _edgeRelations)
   {
      info += "    - " + item.second->EdgeRelName  + 
         "    (FROM "+item.second->FromIdName+"=>"+item.second->FromRelName+
                    "."+item.second->FromRelKeyName+"; "+
         "TO   "+item.second->ToIdName+"=>"+item.second->ToRelName+
                "."+item.second->ToRelKeyName+")";
      if (item.second->StatAvgForward>-1 && item.second->StatAvgBackward>-1) 
         info+= "\n      [Stat:"+to_string(item.second->StatAvgForward)+
                ";"+to_string(item.second->StatAvgBackward)+"]";
      info += "\n";
   }
   info += " - indexes \n";
   for(auto && item:_Indexes)
   {
      info += "    - " + item.second->name  + "."+ item.second->attr;
      info += "\n";
   }

   // add information dfomfrom memory object
   info += "\nConfiguration: \n";
   info += " - Loglevel "+to_string(debugLevel)+"\n";
   info += string(" - dump dot file:  querytree:") 
           + (dumpQueryTree ? "yes":"-") 
           + " ; graph:"+(dumpGraph?"yes":"-")+"\n";

   // add information dfomfrom memory object
   info += "\nMemory object information: \n";
   if (pgm==NULL || !pgm->IsLoaded())
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
