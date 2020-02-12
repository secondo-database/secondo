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

namespace pgraph {


//-----------------------------------------------------------------------------
void QueryOutputFields::ReadFromList(ListExpr list)
{
    LOGOP(10, "QueryOutputFields::ReadFromList","arg: ",nl->ToString(list),"#");
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
ListExpr QueryOutputFields::StreamTypeDefinition()
{
    LOGOP(10, "QueryOutputFields::StreamTypeDefinition");            
    
  
    ListExpr attrs = nl->IntAtom(0);
    ListExpr last = attrs;
    int i=0;
    for (auto&& f:Fields) 
    {
         ListExpr attr=nl->TwoElemList(
                nl->SymbolAtom( f->OutputName),
                nl->SymbolAtom(CcString::BasicType()));
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
    LOGOP(10, "QueryOutputFields::StreamTypeDefinition","attrs: ",
        nl->ToString(attrs));                            

    ListExpr res = (nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                        nl->TwoElemList(
                            nl->SymbolAtom(Tuple::BasicType()),
                            attrs)));    

    LOGOP(10, "QueryOutputFields::StreamTypeDefinition","res: ",
        nl->ToString(res));                            

    return res;                            
}


} // namespace