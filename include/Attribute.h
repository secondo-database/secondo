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
#include "TupleElement.h"

extern NestedList *nl;

class Attribute : public TupleElement
{
public:
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
  virtual bool IsDefined() const = 0;
  virtual void SetDefined(bool defined) = 0;
  virtual int Compare( const Attribute *attrib ) const = 0;

/*
This function should define an order on the attribute values. 
Return values are 0: for equal, -1: this < attrib, and 1: this > attrib. 
The implementaion must also consider that values may be undefined. 

*/
  virtual bool Adjacent( const Attribute *attrib ) const = 0;
/*

However, TupleElement is the base class for the hierachy

----
TupleElement -> Attribute -> StandardAttribute -> IndexableStandardAttribute
----

In a future redesign this should be structured more simple. Moreover
Polymorphism is known to be a performance brake since the derived classes need
to store a pointer to a table of functions. Moreover, this pointer is also
saved in our standard mechanism of making attributes persistent on disk. For
small data types like int this (and the defined flag) blows up the size
dramatically (12 bytes instead of 4).  

Below theres a proposal for a simple redesign concerning the defined status of 
an attribute:

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

