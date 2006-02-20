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
#include <sstream>

#include "NestedList.h"
#include "LogMsg.h"
#include "NList.h"


/*

The class ~NList~ can be used as replacement for the syntactical noisy
interface defined in NestedList.h. 

Currently, not all methods of the ~old~ interface are supported.  It simplifes
the check of a list structure, e.g.  n.isSymbol() will return true, if n is a
symbol.

*/

extern NestedList* nl;

/*
Some string constants for symbols reserved as keaywords in the
queryprocessor or often used types.
   
*/   

struct Symbols {

    const string rel;
    const string map;
    const string tuple;
    const string ptuple;
    const string stream;
    const string typeerror;
    const string integer;
   
    Symbols() :
      rel("rel"),
      map("map"),
      tuple("tuple"),
      ptuple("ptuple"),
      stream("stream"),
      typeerror("typeerror"),
      integer("int")
    {} 
};  


class NListErr {

  public:
  NListErr(const string& Msg) : msg(Msg) {}
  
  string msg;
};


#define CHECK(n) {if (n>0 && length()<n) { \
        throw NListErr("Element "#n" not in list!"); } }

class NList {

  ListExpr l;
  Cardinal len;
  inline bool isNodeType(const int n, const NodeType t) const
  {
    if ( !n ) 
      return ( /*nl->IsAtom(l) &&*/ (nl->AtomType(l) == t) );

    ListExpr list = nl->Nth(n,l);
    return ( /*nl->IsAtom(list) &&*/ (nl->AtomType(list) == t) ); 
  }

 public:
  NList(const ListExpr list) : l(list), len(nl->ListLength(list)) { 
    /*cout << "Nlist: nl = " << (void*)nl << endl;*/ 
  }
  NList() : l(nl->TheEmptyList()), len(0) { 
    /*cout << "Nlist: nl = " << (void*)nl << endl;*/ 
  }
  NList(const NList& rhs) : l(rhs.l), len(rhs.len) { 
    /*cout << "Nlist(const NList& rhs)  << endl;*/ 
  }
  NList(const NList& e1, const NList& e2) : 
    l( nl->TwoElemList(e1.l, e2.l) ), 
    len(2)
  {}
  NList(const string& s) : l(nl->SymbolAtom(s)), len(1)
  {} 
  
  ~NList() {}
  inline bool isEmpty() const { return nl->IsEmpty(l); }

  inline bool isEqual(const string& s) const { return nl->IsEqual(l, s); }

  inline bool isAtom()                       const { return nl->IsAtom(l); }
  inline bool isSymbol(const Cardinal n = 0) const { 
    CHECK(n) return isNodeType(n, SymbolType); 
  }
  inline bool isSymbol(const string& s)      const { 
    return isNodeType(0, SymbolType) && (s == nl->SymbolValue(l)); 
  }
  inline bool isString(const Cardinal n = 0) const { 
    CHECK(n) return isNodeType(n, StringType); 
  }
  inline bool isText(const Cardinal n = 0)   const { 
    CHECK(n) return isNodeType(n, TextType); 
  }
  inline bool isNoAtom(const Cardinal n = 0) const { 
    CHECK(n) return isNodeType(n, NoAtom); 
  }
  inline bool isList(const Cardinal n = 0)   const { 
    CHECK(n) return isNodeType(n, NoAtom); 
  }
  inline bool isInt(const Cardinal n  = 0)   const { 
    CHECK(n) return isNodeType(n, IntType); 
  }
  inline bool isReal(const Cardinal n = 0)   const { 
    CHECK(n) return isNodeType(n, RealType); 
  }
  inline bool isBool(const Cardinal n = 0)   const { 
    CHECK(n) return isNodeType(n, BoolType); 
  }
  
  inline bool hasLength(const Cardinal c) const { return (length() == c);  }

  inline NList first()  const { return elem(1); }
  inline NList second() const { return elem(2); }
  inline NList third()  const { return elem(3); }
  inline NList fourth() const { return elem(4); }
  inline NList fifth()  const { return elem(5); }
  inline NList sixth()  const { return elem(6); }

  inline NList elem(Cardinal n) const
  {
    if ( (length() < n)   ) {
      stringstream s; 
      s << "Out of range exception in nested list. "
        << "Element " << n << " requested, but "
        << "list has length " << len << ".";
      throw NListErr(s.str());
    } 
    
    if ( (n == 1) && isAtom() ) {
      stringstream s; 
      s << "Out of range exception in nested list. "
        << "Element " << n << " requested, but "
        << "list is an atom.";
      throw NListErr(s.str());
    }

    return NList(nl->Nth(n,l)); 
  }
  
  inline static ListExpr typeError(const string& msg)
  { 
    ErrorReporter::ReportError(msg); 
    return nl->TypeError(); 
  }

  // for interchange with the old NestedList interface
  inline ListExpr listExpr() const { return l; }
 
  inline bool hasStringValue() const 
  { 
    return (isString() || isText() || isSymbol()); 
  }

  inline string str() const // retrieve a string value 
  {
    static string result = "";

    if( !hasStringValue() )
      throw NListErr("List has no string value exception!");

    if ( isSymbol() )
      return nl->SymbolValue(l);

    if ( isString() )
      return nl->StringValue(l);
    
    if ( isText() )
      return nl->Text2String(l);

    return result;
  } 

  inline string convertToString() const { return nl->ToString(l); } 
  
  inline bool operator==(const NList& rhs) const { return nl->Equal(l,rhs.l); }
  
  inline bool operator==(const string& rhs) const
  {
    if( !hasStringValue() ) 
    {
      return false;
    }
    else
    {
      return ( str() == rhs );
    }
  }
 
  inline Cardinal length() const
  {
    return len;
  }

}; 

ostream& operator<<(ostream& os, const NList& n);

#endif
