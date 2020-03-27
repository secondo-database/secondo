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

#include "QueryFilterFields.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "Utils.h"
#include "RelationSchemaInfo.h"

using namespace std;

namespace pgraph {


//-----------------------------------------------------------------------------
bool QueryFilterFields::Matches(string typname,RelationSchemaInfo *schema, 
    Tuple *tuple)
{
   for (auto&& f:Fields)
   {
       if (f->NodeAlias==typname)
       {
           AttrInfo *ai=schema->GetAttrInfo(f->PropertyName);
            if (ai!=NULL)
            {
               string val=ai->GetStringVal(tuple);

               // all types
               if (f->Operator=="=")
                  if (val!=f->FilterValue) return false;
               if (f->Operator=="<>")
                  if (val == f->FilterValue) 
                        return false;

               // type specific
               if (ai->TypeName=="int")
               {
                  if (f->Operator==">")
                     if (!(std::stoi(val) > std::stoi(f->FilterValue))) 
                        return false;
                  if (f->Operator=="<")
                     if (!(std::stoi(val) < std::stoi(f->FilterValue))) 
                        return false;
               }
               // type specific
               if (ai->TypeName=="string")
               {
                  if (f->Operator=="startswith")
                     if (!val.rfind(f->FilterValue,0)==0)
                        return false;
                  if (f->Operator=="contains")
                     if (val.find(f->FilterValue) == string::npos)
                        return false;
               } 
           }
       }

   }
   return true; 
}

//----------------------------------------------------------------------------
void QueryFilterFields::ReadFromList(ListExpr list)
{
    LOGOP(20, "QueryFilterFields::ReadFromList","arg: ",nl->ToString(list));
    int index=0;
    while(true)
    {
        if (nl->IsEmpty(list)) break;
        ListExpr item=nl->First(list);
        list=nl->Rest(list);

        QueryFilterField  *f=new QueryFilterField();
        f->NodeAlias=nl->ToString(nl->First(nl->First(item)));
        f->PropertyName=nl->ToString(nl->Second(nl->First(item)));
        f->Operator=nl->ToString(nl->Second(item));
        f->FilterValue=nl->ToString(nl->Third(item));
        ReplaceStringInPlace(f->FilterValue, "\"","");
        Fields.push_back(f);
        index++;
    }
}


} // namespace