/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

[1] Separate part of persistent data representation

[1] Using Storage Manager Berkeley DB

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

[TOC]

*/

// 30.11.2004 M. Spiekermann. Currently, this flag will never be set and the code does
// not compile since it havent been compiled for a long time. I think
// nobody needs an persistent version of the OldRelationAlgebra. Maybe we can remove
// this file soon.  
#ifdef RELALG_OLD_PERSISTENT  

using namespace std;

#include "RelationAlgebra.h"

extern NestedList* nl;

class TupleAttributesInfo
{

    TupleAttributes* tupleType;
    AttributeType* attrTypes;

  public:

    TupleAttributesInfo (ListExpr typeInfo, ListExpr value);

};

TupleAttributesInfo::TupleAttributesInfo (ListExpr typeInfo, ListExpr value)
{
  ListExpr attrlist, valuelist,first,firstvalue, errorInfo;
  Word attr;
  int algebraId, typeId, noofattrs;
  attrTypes = new AttributeType[nl->ListLength(value)];
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  bool valueCorrect;

  attrlist = nl->Second(typeInfo);
  valuelist = value;
  noofattrs = 0;

  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);

    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    firstvalue = nl->First(valuelist);
    valuelist = nl->Rest(valuelist);
    attr = (algM->InObj(algebraId, typeId))(nl->Second(first),
              firstvalue, 0, errorInfo, valueCorrect);
    if (valueCorrect)
    {
      AttributeType attrtype = { algebraId, typeId, ((Attribute*)attr.addr)->Sizeof() };
      attrTypes[noofattrs] = attrtype;
      noofattrs++;
    }
  }
  tupleType = new TupleAttributes(noofattrs, attrTypes);
};

#endif

