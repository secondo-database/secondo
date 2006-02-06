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
//characters    [3]    capital:    [\textsc{]    [}]
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Attribute

May 1998 Stefan Dieker

April 2002 Ulrich Telle Adjustments for the new Secondo version

Oct 2004 M. Spiekermann. Adding some more detailed documentation and some 
thoughts about redesign and performance. 

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
#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "SecondoSystem.h"
#include "NestedList.h"
#include "AlgebraManager.h"
#include "FLOB.h"

extern NestedList *nl;

/*
3.5 Struct ~AttrDelete~

This structure defines the way attributes are deleted. First, ~refs~
maintain a reference counter of that attribute. Many tuples can
point to the same attribute to avoid unnecessary copying. Every
tuple that points to an attribute increases this value and every
time it tries to delete the attribute, it decreses this value.
The attribute will only be delete when ~refs~ = 0.

~isDelete~ indicates which function must be called to delete the
attribute. If it is ~false~, then the attribute was created
with ~malloc~ and must be deleted with ~free~. Otherwise, if it
is ~true~, the attribute was created with ~new~ and must
be deleted with ~delete~. The default is ~DeleteAttr~.

*/
struct AttrDelete
{
  AttrDelete():
  refs( 1 ), isDelete( true )
  {}

  unsigned char refs;
  bool isDelete;
};

/*
4 Class ~Attribute~

*/
class Attribute
{
  public:
    inline Attribute()
    {}
/*
The simple constructor.

*/
    inline virtual ~Attribute()
    {}
/*
The virtual destructor.

*/
    virtual bool IsDefined() const = 0;
/*
Returns whether the attribute is defined.

*/
    virtual void SetDefined(bool defined) = 0;
/*
Sets the ~defined~ flag of the attribute.

*/

    virtual int Compare( const Attribute *attrib ) const = 0;
/*
This function should define an order on the attribute values. 
Return values are 0: for equal, -1: this < attrib, and 1: this > attrib. 
The implementaion must also consider that values may be undefined. 

*/
    virtual bool Adjacent( const Attribute *attrib ) const = 0;
/*
This function checks if two attributes are adjacent. As an example,
1 and 2 are adjacent for integer attributes and "Victor" and "Victos"
are adjacent for string attributes.

*/
    virtual Attribute* Clone() const = 0;
/*
Warning: The simple implementation

----
X::X(const X&) { // do a deep copy here }
X::Clone() { return new X(*this); }
----

does only work correctly if the copy constructor is implemented otherwise
the compiler will not complain and replaces the call by a default implementation
returning just the this pointer, hence no new object will be created.

*/
    inline virtual int NumOfFLOBs() const
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
    inline virtual ostream& Print( ostream& os ) const
    {
      return os;
    }
/*
Prints the attribute. Used for debugging purposes.

*/
    inline static void Save( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo, Attribute *elem )
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
    inline static Attribute *Open( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo )
    {
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId = nl->IntValue( nl->First( typeInfo ) ),
          typeId = nl->IntValue( nl->Second( typeInfo ) );
      size_t size = (algMgr->SizeOfObj(algId, typeId))();

      Attribute *elem = (Attribute*)(algMgr->CreateObj(algId, typeId))( typeInfo ).addr;
      // Read the element
      valueRecord.Read( elem, size, offset );
      elem = (Attribute*)(algMgr->Cast(algId, typeId))( elem );
      offset += size;

      // Open the FLOBs
      for( int i = 0; i < elem->NumOfFLOBs(); i++ )
      {
        FLOB *tmpFLOB = elem->GetFLOB(i);
        char *flob = (char*)malloc( tmpFLOB->Size() );
        valueRecord.Read( flob, tmpFLOB->Size(), offset );
        tmpFLOB->ReadFromExtensionTuple( flob );
        offset += tmpFLOB->Size();
      }

      return elem;
    }
/*
Default open function.

*/
    inline void DeleteIfAllowed()
    {
      assert( del.refs > 0 );
      del.refs--;
      if( del.refs == 0 )
      {
        Finalize();
        if( del.isDelete )
          delete this;
        else
          free( this );
      }
    }
/*
Deletes an attribute if allowed, i.e. if ~refs~ = 0.

*/
    inline Attribute* Copy()
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
    inline void SetFreeAttr()
    {
      del.isDelete = false;
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

/* 
The next class defines some default functions which can be passed to the
constructor of class ~TypeConstructor~. Whenever these defaults are not
sufficient you must inherhit from this base class and overwrite (hide) the
default implementation.

*/

template<class T>
  void* Cast( void* addr ) 
  { 
    return (new (addr)T ); 
  }

template<class T>
  Word Create( const ListExpr typeInfo )
  {
    return (SetWord( new T() ));
  }

template<class T>
  void Delete( Word& w )
  {
    delete (T *)w.addr;
    w.addr = 0;
  }

template<class T>
  void Close( Word& w )
  {
    delete (T *)w.addr;
    w.addr = 0;
  }

template<class T>
  Word Clone( const Word& w )
  {
    return SetWord( new T(*(T *)w.addr) );
  }

template<class T>
  int SizeOf()
  {
    return sizeof(T);
  }

template<class T>
  bool Open( SmiRecord& valueRecord,
             const ListExpr typeInfo,
             Word& value )
  {
    T *p = new T;
    p->Open( valueRecord, typeInfo );
    value = SetWord( p );
    return true;
  }

template<class T>
  bool Save( SmiRecord& valueRecord,
             const ListExpr typeInfo,
             Word& value )
  {
    T *p = (T *)value.addr;
    p->Save( valueRecord, typeInfo );
    return true;
  }


template<class T>
bool
SimpleCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, T::Symbol() ));
}



#endif

