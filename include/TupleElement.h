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

July 2005 M. Spiekermann. Missing return statements in ~Print()~ and ~GetFlob()~ 
added.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. 

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
3.5 Struct ~AttrDelete~

This structure defines the way attributes are deleted. First, ~refs~ 
maintain a reference counter of that attribute. Many tuples can
point to the same attribute to avoid unnecessary copying. Every
tuple that points to an attribute increases this value and every
time it tries to delete the attribute, it decreses this value.
The attribute will only be delete when ~refs~ = 0. 

~type~ indicates which function must be called to delete the 
attribute. If it is ~FreeAttr~, then the attribute was created 
with ~malloc~ and must be deleted with ~free~. Otherwise, if it 
is ~DeleteAttr~, the attribute was created with ~new~ and must 
be deleted with ~delete~. The default is ~DeleteAttr~.

*/
enum AttrDeleteType { FreeAttr, DeleteAttr };

struct AttrDelete
{
  AttrDelete():
  refs( 1 ), type( DeleteAttr )
  {}

  int refs;
  AttrDeleteType type;
};

/*
1.1 Class "TupleElement"[1]

This class defines several virtual methods which are essential for 
attributes insite tuples.

*/
class TupleElement 
{
  public:

    inline TupleElement()
    {}
/*
The simple constructor.

*/
    inline virtual ~TupleElement()
    {}
/*
The virtual destructor.

*/
    inline virtual int NumOfFLOBs()
    { 
      return 0; 
    }
/*
Returns the number of FLOBs the attribute contains. The default
value of this funcion is 0, which means that if an attribute
does not contain FLOBs, it is not necessary to implement this
function.

*/
    inline virtual FLOB* GetFLOB( const int i )
    { 
      assert( false );
      return 0; 
    }
/*
Returns a reference to a FLOB given an index ~i~. If the attribute
does not contain any FLOBs (the default), this function should not
be called.

*/

    inline virtual void Initialize() {}
    inline virtual void Finalize()   {}
/*
These two functions are used to initialize and finalize values of
attributes. In some cases, the constructor and destructor are
not called, e.g. when deleting an attribute with delete type 
~FreeAttr~. In these cases, these two functions are called
anyway. An example of their usage is in the JNI algebras, where
some Java initialization and destructions are done in these 
functions. 

*/
    inline virtual ostream& Print( ostream& os )
    { 
      return os; 
    }
/*
Prints the attribute. Used for debugging purposes.

*/
    inline static void Save( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo, TupleElement *elem )
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
        tmpFLOB->SaveToExtensionTuple( extensionPtr );
        extensionPtr += tmpFLOB->Size();
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

    inline static TupleElement *Open( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo )
    {
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId = nl->IntValue( nl->First( typeInfo ) ),
          typeId = nl->IntValue( nl->Second( typeInfo ) );
      size_t size = (algMgr->SizeOfObj(algId, typeId))();

      TupleElement *elem = (TupleElement*)(algMgr->CreateObj(algId, typeId))( typeInfo ).addr;
      // Read the element
      valueRecord.Read( elem, size, offset );
      elem = (TupleElement*)(algMgr->Cast(algId, typeId))( elem );
      offset += size;

      // Save the FLOBs
      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        FLOB *tmpFLOB = elem->GetFLOB(i);
        tmpFLOB->ReadFromExtensionTuple( valueRecord, offset );
        offset += tmpFLOB->Size();
      }

      return elem;
    }
/*
Default open function.

*/

    inline void DeleteIfAllowed()
    {
      del.refs--;
      if( del.refs == 0 )
      {
        Finalize();
        if( del.type == DeleteAttr )
          delete this;
        else // del.type == FreeAttr
        {
          for( int j = 0; j < NumOfFLOBs(); j++)
          {
            FLOB *flob = GetFLOB(j);
            if( flob->GetType() != Destroyed )
              flob->Clean();
          }
          free( this );
        }
      }
    }
/*
Deletes an attribute if allowed, i.e. if ~refs~ = 0. 

*/

    inline void IncReference()
    {
      del.refs++;
    }
/*
Increments the reference counter.

*/

    inline void SetDeleteType( AttrDeleteType type )
    {
      del.type = type;
    }
/*
Sets the delete type.

*/

  private:

    AttrDelete del;
/*
Stores the way this attribute is deleted.

*/
};

ostream& operator<< (ostream &os, TupleElement &attrib);

#endif

