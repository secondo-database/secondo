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

//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[11]	title:		[{\large \bf ] [}]
//paragraph	[12]	title:		[{\normalsize \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Tuple Element

May 1998 Stefan Dieker

April 2002 Ulrich Telle Adjustments for the new Secondo version

October 2003 Victor Almeida added the functions ~Open~ and ~Save~

November 2004 M. Spiekermann. Some uninitialized variables were set to 
avoid warnings when compiler flag -O2 is used.

1.1 Overview

The ~Tuple Manager~ is an important support component for the relational
algebra. Relations consist of tuples, tuples consist of tuple elements.
Classes implementing attribute data types have to be subtypes of class
attribute. Whatever the shape of such derived attribute classes might be,
their instances can be aggregated and made persistent via instances of class
~Tuple~, while the user is (almost) not aware of the additional management
actions arising from persistency.

1.1 Types

*/

#ifndef TUPLE_ELEMENT_H
#define TUPLE_ELEMENT_H

#ifndef TYPE_ADDRESS_DEFINED
#define TYPE_ADDRESS_DEFINED
typedef void* Address;
#endif

#include "NestedList.h"
#include "SecondoSystem.h"
#include "FLOB.h"

#include <iostream>

/*
Are type definitions for a generic address pointer and for a ~fake large object~.

*/

/*
1.1 Class "TupleElement"[1]

This class defines several virtual methods which are essential for the
~Tuple Manager~.

*/
class TupleElement // renamed, previous name: TupleElem
{
  public:
    virtual ~TupleElement()
      {}

    virtual int NumOfFLOBs()
      { return 0; }

    virtual FLOB* GetFLOB( const int )
      { assert( false ); }

    virtual void Initialize()   {}

    virtual ostream& Print( ostream& os )
      { assert( false ); }

    static void Save( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo, TupleElement *elem )
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
        if( tmpFLOB->IsLob() )
          tmpFLOB->SaveToLob( *SecondoSystem::GetFlobFile() );
        else
          extensionSize += tmpFLOB->Size();
      }

      // Move FLOB data to extension tuple
      char *extensionElement = 0;
      if( extensionSize > 0 )
      {
        extensionElement = (char *)malloc( extensionSize );
        char *extensionPtr = extensionElement;
        for( int i = 0; i < elem->NumOfFLOBs(); i++ )
        {
          FLOB *tmpFLOB = elem->GetFLOB(i);
          if( !tmpFLOB->IsLob() )
          {
            tmpFLOB->SaveToExtensionTuple( extensionPtr );
            extensionPtr += tmpFLOB->Size();
          }
        }
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

    static TupleElement *Open( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo )
    {
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId = nl->IntValue( nl->First( typeInfo ) ),
          typeId = nl->IntValue( nl->Second( typeInfo ) ),
          size = (algMgr->SizeOfObj(algId, typeId))();

      TupleElement *elem = (TupleElement*)(algMgr->CreateObj(algId, typeId))( typeInfo ).addr;
      // Read the element
      valueRecord.Read( elem, size, offset );
      elem = (TupleElement*)(algMgr->Cast(algId, typeId))( elem );
      offset += size;

      // Calculate the extension size
      int extensionSize = 0;
      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        FLOB *tmpFLOB = elem->GetFLOB(i);
        if( tmpFLOB->IsLob() )
          tmpFLOB->SetLobFile( SecondoSystem::GetFlobFile() );
        else
          extensionSize += tmpFLOB->Size();
      }

      // Read the extension size
      char *extensionElement = 0;
      if( extensionSize > 0 )
      {
        extensionElement = (char*)malloc( extensionSize );
        valueRecord.Read( extensionElement, extensionSize, offset );
      }

      // Restore the FLOB data
      char *extensionPtr = extensionElement;
      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        FLOB* tmpFLOB = elem->GetFLOB(i);
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->Restore( extensionPtr );
          extensionPtr = extensionPtr + tmpFLOB->Size();
        }
      }
      return elem;
    }

};

ostream& operator<< (ostream &os, TupleElement &attrib);

#endif

