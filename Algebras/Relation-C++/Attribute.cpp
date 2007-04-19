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
#include "FLOB.h"
#include "WinUnix.h"

const double FACTOR = 0.00000001; // Precision factor, used within AlmostEqual


    void Attribute::Save( SmiRecord& valueRecord, size_t& offset, 
                             const ListExpr typeInfo, Attribute *elem )
    {
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

      int algId = nl->IntValue( nl->First( typeInfo ) ),
          typeId = nl->IntValue( nl->Second( typeInfo ) ),
          size = (algMgr->SizeOfObj(algId, typeId))();

      // Calculate the extension size
      int extensionSize = 0;
      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        FLOB *tmpFLOB = elem->GetFLOB(i);
        extensionSize += tmpFLOB->Size();
      }

      // Move FLOB data to extension tuple
      char *extensionElement = 0;
      if( extensionSize > 0 )
        extensionElement = (char *)malloc( extensionSize );

      char *extensionPtr = extensionElement;
      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        FLOB *tmpFLOB = elem->GetFLOB(i);
        unsigned int size = tmpFLOB->WriteTo( extensionPtr );
        extensionPtr += size;
      }

      // Write the element
      valueRecord.Write( elem, size, offset );
      offset += size;

      // Write the extension element
      if( extensionSize > 0 )
      {
        valueRecord.Write( extensionElement, extensionSize, offset );
        free( extensionElement );
      }
    }
/*
Default save function.

*/
    Attribute *Attribute::Open( SmiRecord& valueRecord, 
                                   size_t& offset, const ListExpr typeInfo )
    {
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId = nl->IntValue( nl->First( typeInfo ) ),
          typeId = nl->IntValue( nl->Second( typeInfo ) );
      size_t size = (algMgr->SizeOfObj(algId, typeId))();

      Attribute*
        elem = (Attribute*)(algMgr->CreateObj(algId, typeId))( typeInfo ).addr;
      // Read the element
      valueRecord.Read( elem, size, offset );
      elem = (Attribute*)(algMgr->Cast(algId, typeId))( elem );
      elem->del.refs = 1;
      elem->del.isDelete = true;
      offset += size;

      // Open the FLOBs
      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        FLOB *tmpFLOB = elem->GetFLOB(i);
	unsigned int size = tmpFLOB->Size();
        char *flob = (char*)malloc( size );
        valueRecord.Read( flob, size, offset );
        unsigned int bytes = tmpFLOB->ReadFrom( flob );
	assert( size == bytes );
        offset += bytes;
      }

      return elem;
    }
/*
Default open function.

*/
    bool Attribute::DeleteIfAllowed()
    {
      assert( del.refs > 0 );
      del.refs--;
      if( del.refs == 0 )
      {
        Finalize();
        if( del.isDelete )
          delete this;
        else {
          for( int i = 0; i < NumOfFLOBs(); i++) {
            GetFLOB(i)->Clean();		  
          }		  
          free( this );
	}  
        return true;
      }
      return false;
    }
/*
Deletes an attribute if allowed, i.e. if ~refs~ = 0.

*/
    Attribute* Attribute::Copy()
    {
      if( del.refs == numeric_limits<unsigned char>::max() )
        return Clone();
      del.refs++;
      return this;
    }
/*
Copies this element if it is possible, otherwise
clones it.

*/

    string Attribute::AttrDelete2string()
    {
      std::string Result, str;
      std::stringstream ss;
      ss << ((int) del.refs);
      ss >> str;

      Result += " del.Refs=";
      Result += str;
      Result += ", del.IsDelete=";
      if (del.isDelete)
        Result += "true";
      else 
        Result +="false";
      return Result;
    }
/*
Print the delete reference info to a string (for debugging)

*/

