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

#include "QueryOutputFields.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "Utils.h"
#include "PropertyGraphMem.h"

using namespace std;

namespace pgraph {


//-----------------------------------------------------------------------------
void QueryOutputFields::ReadFromList(ListExpr list)
{
    LOGOP(20, "QueryOutputFields::ReadFromList","arg: ",nl->ToString(list),"#");
    int index=0;
    while(true)
    {
        if (nl->IsEmpty(list)) break;
        ListExpr item=nl->First(list);
        list=nl->Rest(list);

        QueryOutputField *f=new QueryOutputField();
        f->NodeAlias=nl->ToString(nl->First(nl->First(item)));
        f->OutputName=nl->ToString(nl->Second(item));
        f->index=index;
        f->PropertyName=nl->ToString(nl->Second(nl->First(item)));
        Fields.push_back(f);
        index++;
    }

}

//----------------------------------------------------------------------------
void QueryOutputFields::ReadFromList(string text)
{
    ListExpr list;
    nl->ReadFromString(text, list);
    ReadFromList(list);
}

//----------------------------------------------------------------------------
ListExpr QueryOutputFields::StreamTypeDefinition(QueryAliasList *aliaslist, 
   MemoryGraphObject *pgmem)
{
   LOGOP(20, "QueryOutputFields::StreamTypeDefinition");            
   

   ListExpr attrs = nl->IntAtom(0);
   ListExpr last = attrs;
   int i=0;
   for (auto&& f:Fields) 
   {
      if (f->datatype=="")
      {
         // search aliasinfo
         QueryAliasInfo *ai=NULL;
         for(auto&& alias:aliaslist->list)
            if (alias.AliasName==f->NodeAlias)
               ai=&alias;

         if (ai==NULL)     
            throw PGraphException("no node alias defined for "+f->NodeAlias);
         if (ai->TypeName=="")   
            throw PGraphException("no type found for alias "+f->NodeAlias);

         RelationInfo *ri= pgmem->RelRegistry.GetRelationInfo(ai->TypeName);
         if (ri==NULL)
               throw PGraphException("unknown typename"+ai->TypeName);

         AttrInfo *ati = ri->RelSchema.GetAttrInfo(f->PropertyName);
         if (ati==NULL)
            throw PGraphException("propertyname not found: "+f->PropertyName);

         f->datatype=ati->TypeName;   
      }



      ListExpr attr=nl->TwoElemList(
               nl->SymbolAtom( f->OutputName),
               nl->SymbolAtom(f->datatype));
      if (i==0)
      {
         attrs = nl->OneElemList(attr);
         last = attrs;
      }
      else
      {
         last = nl->Append(last,attr);
      }
      i++;
   }
   LOGOP(20, "QueryOutputFields::StreamTypeDefinition","attrs: ",
      nl->ToString(attrs));                            

   ListExpr res = (nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                     nl->TwoElemList(
                           nl->SymbolAtom(Tuple::BasicType()),
                           attrs)));    

   LOGOP(20, "QueryOutputFields::StreamTypeDefinition","res: ",
      nl->ToString(res));                            

   return res;                            
}

//------------------------------------------------------------------------------
void QueryAliasList::Update(std::string aliasname, std::string typename_)
{
   if (aliasname=="") return;
   
   for(auto&& item : list)
   {
      if ((item.AliasName!="") && (item.AliasName==aliasname))
         item.TypeName=typename_;
   }
}

//------------------------------------------------------------------------------
void QueryAliasList::AddNode(std::string aliasname, std::string typename_)
{
   for(auto&& item : list)
   {
      if ((item.AliasName!="") && (item.AliasName==aliasname))
         throw PGraphException("alias redeclared !");
   }

   QueryAliasInfo aliasinfo;
   aliasinfo.AliasName=aliasname;
   aliasinfo.TypeName=typename_;
   aliasinfo.Type=AliasIsNode;
   list.push_back(aliasinfo);
}

//------------------------------------------------------------------------------
void QueryAliasList::AddEdge(std::string aliasname, std::string typename_)
{
   for(auto&& item : list)
   {
      if ((item.AliasName!="") && (item.AliasName==aliasname))
         throw PGraphException("alias redeclared !");
   }

   QueryAliasInfo aliasinfo;
   aliasinfo.AliasName=aliasname;
   aliasinfo.TypeName=typename_;
   aliasinfo.Type=AliasIsEdge;
   list.push_back(aliasinfo);
}

} // namespace