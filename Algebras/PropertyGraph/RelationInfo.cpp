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

#include "../Relation-C++/RelationAlgebra.h"

#include <string>
#include <Utils.h>
#include <RelationInfo.h>
using namespace std;

//------------------------------------------------------------------
namespace pgraph
{

//----------------------------------------------------------------------------
string AttrInfo::GetStringVal(Tuple *tuple)
{
      if (TypeName=="string")  {
         return ((CcString*)tuple->GetAttribute(Index))->GetValue();
      }
      if (TypeName=="int")  {
         return to_string(((CcInt*)tuple->GetAttribute(Index))->GetValue());
      }
      return "";
}

//----------------------------------------------------------------------------
RelationSchemaInfo::RelationSchemaInfo()
{
}

//----------------------------------------------------------------------------
RelationSchemaInfo::RelationSchemaInfo(ListExpr list)
{
   LoadFromList(list);
}

//----------------------------------------------------------------------------
void RelationSchemaInfo::Dump()
{
   cout << "Attributes:" <<endl;
    for (auto&& f:Attributes) 
    {
       cout << " - "<< f->Name<<endl;
    }
}

//----------------------------------------------------------------------------
void RelationSchemaInfo::LoadFromList(ListExpr list)
{
   if (nl->ToString( nl->First(list))!="rel")
      throw  PGraphException("no relation!");
   
   list=nl->Second(nl->Second(list));
   int i=0;
   while (nl->ListLength(list)>0)
   {
      ListExpr item=nl->First(list);
      list=nl->Rest(list);

      AttrInfo *a=new AttrInfo();
      a->Name=nl->ToString(nl->First(item));
      a->TypeName=nl->ToString(nl->Second(item));
      a->Index=i++;

      Attributes.push_back(a);
   }
}

//----------------------------------------------------------------------------
AttrInfo* RelationSchemaInfo::GetAttrInfo(string name)
{
   
    for(unsigned int i=0; i<Attributes.size();i++)
    {
       if (Attributes[i]->Name==name)
          return Attributes[i];
    }
    return NULL;
}

//----------------------------------------------------------------------------
AttrInfo* RelationSchemaInfo::GetAttrInfo(int index)
{
   return Attributes[index];
}

//----------------------------------------------------------------------------
int RelationSchemaInfo::GetAttrCount()
{
   return Attributes.size();
}
    



} // namespace pgraph
