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

January-April 2006, M. Spiekermann   

*/

#ifndef CONS_TEMPLATES_H
#define CONS_TEMPLATES_H

#include "AlgebraTypes.h"
#include "Attribute.h"

/* 
The templates below define some default functions which can be passed to the
constructor of class ~TypeConstructor~. Whenever these defaults are not
sufficient you must implement your own functions. 

*/

template<class T>
struct ConstructorFunctions
{

  OutObject out;
  InObject in;
  OutObject saveToList;
  InObject restoreFromList;
  ObjectCreation create;
  ObjectDeletion deletion;
  ObjectOpen open;
  ObjectSave save;
  ObjectClose close;
  ObjectClone clone;
  ObjectCast cast;
  ObjectSizeof sizeOf;
  TypeCheckFunction typeCheck;

  ConstructorFunctions() :
   out(Out),
   in(In),
   saveToList(0),
   restoreFromList(0),
   create(Create),
   deletion(Delete),
   open(Open),
   save(Save),
   close(Close),
   clone(Clone),
   cast(Cast),
   sizeOf(SizeOf),
   typeCheck(0)
  {}
  
  static void* Cast( void* addr ) 
  { 
    return (new (addr)T ); 
  }

  static Word Create( const ListExpr typeInfo )
  {
    return (SetWord( new T() ));
  }

  static void Delete( const ListExpr, Word& w )
  {
    delete (T *)w.addr;
    w.addr = 0;
  }

  static void Close( const ListExpr, Word& w )
  {
    delete (T *)w.addr;
    w.addr = 0;
  }

  static Word Clone( const ListExpr, const Word& w )
  {
    return SetWord( new T(*(T *)w.addr) );
  }

  static int SizeOf()
  {
    return sizeof(T);
  }

  static bool Open( SmiRecord& valueRecord,
             size_t& offset,  
             const ListExpr typeInfo,
             Word& value )
  {
    T* p = (T*)Attribute::Open( valueRecord, offset, typeInfo );
    value = SetWord( p );
    return true;
  }

  static bool Save( SmiRecord& valueRecord,
             size_t& offset,  
             const ListExpr typeInfo,
             Word& value )
  {
    T* p = (T *)value.addr;
    Attribute::Save( valueRecord, offset, typeInfo, (Attribute*)p );
    return true;
  }


  static ListExpr Out( ListExpr typeInfo, Word value )
  {
    assert(false);
    return 0;
  }

  static Word In( const ListExpr typeInfo, const ListExpr instance,
                  const int errorPos, ListExpr& errorInfo, bool& correct )
  {
    assert(false);
    return SetWord(new T);
  }

};



#endif
