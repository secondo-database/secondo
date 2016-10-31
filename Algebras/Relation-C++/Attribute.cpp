/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

1 Header File: Attribute

May 1998 Stefan Dieker

April 2002 Ulrich Telle Adjustments for the new Secondo version

Oct 2004 M. Spiekermann. Adding some more detailed documentation and some
thoughts about redesign and performance.

January 2006, M. Spiekermann. Some template functions which could be used as default
for some type constructor functions were moved to ConstructorTemplates.h

May 2006, M. Spiekermann. Documentation for the ~Compare~ function extended. Template
functions ~GenericCompare~ and ~GetValue~ added.

1.1 Overview

Classes implementing attribute data types have to be subtypes of class
attribute. Whatever the shape of such derived attribute classes might be,
their instances can be aggregated and made persistent via instances of class
~Tuple~, while the user is (almost) not aware of the additional management
actions arising from persistence.

1.1 Class "Attribute"[1]

The class ~Attribute~ defines several pure virtual methods which every
derived attribute class must implement.

*/

#include <sstream>

#include "Attribute.h"
#include "SecondoSystem.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "../Tools/Flob/Flob.h"
#include "WinUnix.h"
#include "AlmostEqual.h"
#include <limits>


Attribute* Attribute::Create(char* state,
                              size_t& offset, const ListExpr typeInfo ) {
  NestedList *nl = SecondoSystem::GetNestedList();
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  int algId, typeId;
  size_t size;
  if (!nl->IsAtom(nl->First(typeInfo)))
  {
    ListExpr type = nl->First(typeInfo);
    algId = nl->IntValue( nl->First( type ) );
      typeId = nl->IntValue( nl->Second( type ) );
      size = (algMgr->SizeOfObj(algId, typeId))();
      }
  else
   {
       algId = nl->IntValue( nl->First( typeInfo ) );
      typeId = nl->IntValue( nl->Second( typeInfo ) );
      size = (algMgr->SizeOfObj(algId, typeId))();
      }

  Attribute*
    elem = (Attribute*)(algMgr->CreateObj(algId, typeId))( typeInfo ).addr;
  // Read the element
  elem->Rebuild(state, offset, algMgr->Cast(algId,typeId));
  elem->del.refs = 1;
  elem->del.SetDelete();
  offset += size;
  return elem;
}


    void Attribute::Save( SmiRecord& valueRecord, size_t& offset,
                             const ListExpr typeInfo, Attribute *elem )
    {
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId, typeId, size;
      if (!nl->IsAtom(nl->First(typeInfo)))
      {
        ListExpr type = nl->First(typeInfo);
        algId = nl->IntValue( nl->First( type ) );
          typeId = nl->IntValue( nl->Second( type ) );
          size = (algMgr->SizeOfObj(algId, typeId))();
          }
      else
       {
           algId = nl->IntValue( nl->First( typeInfo ) );
          typeId = nl->IntValue( nl->Second( typeInfo ) );
          size = (algMgr->SizeOfObj(algId, typeId))();
          }

      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        Flob *tmpFlob = elem->GetFLOB(i);
        SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
        SmiRecordFile* rf = ctlg->GetFlobFile();
        //cerr << "FlobFileId = " << fileId << endl;
        tmpFlob->saveToFile(rf, *tmpFlob);
      }

      // Write the element
      valueRecord.Write( elem, size, offset );
      offset += size;
    }
/*
Default save function.

*/
    Attribute *Attribute::Open( SmiRecord& valueRecord,
                                   SmiSize& offset, const ListExpr typeInfo )
    {
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId=0;
      int typeId=0;
      size_t size=0;
      if (!nl->IsAtom(nl->First(typeInfo)))
      {
        ListExpr type = nl->First(typeInfo);
        algId = nl->IntValue( nl->First( type ) );
          typeId = nl->IntValue( nl->Second( type ) );
          size = (algMgr->SizeOfObj(algId, typeId))();
          }
      else
       {
           algId = nl->IntValue( nl->First( typeInfo ) );
          typeId = nl->IntValue( nl->Second( typeInfo ) );
          size = (algMgr->SizeOfObj(algId, typeId))();
          }

      Attribute*
        elem = static_cast<Attribute*>(
              (algMgr->CreateObj(algId, typeId))( typeInfo ).addr );
      // Read the element
      valueRecord.Read( elem, size, offset );
      elem = static_cast<Attribute*>(
           (algMgr->Cast(algId, typeId))( elem ) );
      elem->del.refs = 1;
      elem->del.SetDelete();
      offset += size;

      return elem;
    }
/*
Default open function.

*/

    bool Attribute::DeleteIfAllowed( const bool destroyFlobs /*=true*/ )
    {
      assert( del.refs > 0 );
      del.refs--;
      if( del.refs == 0 )
      {
        Finalize();
        if(destroyFlobs && !IsPinned()){ // destroy if unpinned
          for( int i = 0; i < NumOfFLOBs(); i++) {
            GetFLOB(i)->destroyIfNonPersistent();
          }
        }
        if( del.IsDelete() )
          delete this;
        else {
          free( this );
        }
        return true;
      }
      return false;
    }
/*
Deletes an attribute if allowed, i.e. if ~refs~ = 0.

*/

    void Attribute::DestroyFlobs()
    {
      for( int i = 0; i < NumOfFLOBs(); i++) {
        GetFLOB(i)->destroy();
      }
    }
/*
Destroys all Flobs of the Attribute, regardless of the reference counter.
Call this prior to deletion of an automatic attribute variable.
Otherwise, the Flobs belonging to the Attribute are not destroyed and may
persist without being referenced any more.

*/

    Attribute* Attribute::Copy()
    {
      if( del.refs == std::numeric_limits<uint16_t>::max() )
        return Clone();
      del.refs++;
      return this;
    }
/*
Copies this element if it is possible, otherwise
clones it.

*/

    std::string Attribute::AttrDelete2string()
    {
      std::string Result, str;
      std::stringstream ss;
      ss << ((int) del.refs);
      ss >> str;

      Result += " del.Refs=";
      Result += str;
      Result += ", del.IsDelete=";
      if (del.IsDelete())
        Result += "true";
      else
        Result +="false";
      return Result;
    }
/*
Print the delete reference info to a string (for debugging)

*/


std::ostream& operator<<(std::ostream& os, const Attribute& attr)
{
  return attr.Print(os);
}


/*
Generalized output operator

*/
