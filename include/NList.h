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

August 2005, M. Spiekermann. Initial version.

*/

#ifndef SECONDO_NLIST_H
#define SECONDO_NLIST_H

#include <assert.h>
#include "NestedList.h"


/*

The class ~NList~ can be used as replacement for the syntactical noisy interface
defined in NestedList.h. 

Currently, not all methods of the ~old~ interface are supported.  It simplifes
the check of a list structure, e.g.  n.isSymbol() will return true, if n is a
symbol.

*/

extern NestedList* nl;
class NList {

  ListExpr l;
  Cardinal len;
  inline bool isNodeType(const int n, const NodeType t) const
  {
    if ( !n ) 
      return ( nl->IsAtom(l) && (nl->AtomType(l) == t) );

    ListExpr list = nl->Nth(n,l);
    return ( nl->IsAtom(list) && (nl->AtomType(list) == t) ); 
  }

 public:
  NList(const ListExpr list) : l(list), len(0) { /*cout << "Nlist: nl = " << (void*)nl << endl;*/ }
  ~NList() {}
  inline bool isEmpty() const { return nl->IsEmpty(l); }

  inline bool isEqual(const string& s) const { return nl->IsEqual(l, s); }

  inline bool isAtom() const   { return nl->IsAtom(l); }
  inline bool isSymbol(const int n = 0) const { return isNodeType(n, SymbolType); }
  inline bool isString(const int n = 0) const { return isNodeType(n, StringType); }
  inline bool isText(const int n = 0) const   { return isNodeType(n, TextType); }
  inline bool isNoAtom(const int n = 0) const { return isNodeType(n, NoAtom); }
  inline bool isInt(const int n = 0) const    { return isNodeType(n, IntType); }
  inline bool isReal(const int n = 0) const   { return isNodeType(n, RealType); }
  inline bool isBool(const int n = 0) const   { return isNodeType(n, BoolType); }
  
  inline bool hasLength(const Cardinal c)  { return (length() == c);  }

  inline NList first() const  { return NList(nl->First(l)); }
  inline NList second() const { return NList(nl->Second(l)); }
  inline NList third() const  { return NList(nl->Third(l)); }
  inline NList fourth() const { return NList(nl->Fourth(l)); }
  inline NList fifth() const  { return NList(nl->Fifth(l)); }
  inline NList sixth() const  { return NList(nl->Sixth(l)); }

  inline NList elem(const int n) const { return NList(nl->Nth(n,l)); }
  
  inline NList typeError() const { return NList(nl->TypeError()); }

  // for interchange with the old NestedList interface
  inline ListExpr listExpr() const { return l; }
 
  inline bool hasStringValue() const { return (isString() || isText() || isSymbol()); }
  inline string str() const // retrieve a string value 
  {
    assert( hasStringValue() );

    static string result = "";
    if ( isSymbol() )
      return nl->SymbolValue(l);

    if ( isString() )
      return nl->StringValue(l);
    
    if ( isText() )
      return nl->Text2String(l);

    return result;
  } 
 
  inline Cardinal length()
  {
    if ( len ) {
      return len;
    } else {
      len = nl->ListLength(l);
      return len;
    } 
  }

}; 

#endif
