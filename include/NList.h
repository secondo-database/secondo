/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and Computer Science,
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

January - March 2006, M. Spiekermann. New constructors and functions added. Documentation revised.

April 2006, M. Spiekermann. Helpful functions for type mappings added.

*/

#ifndef SECONDO_NLIST_H
#define SECONDO_NLIST_H

#include <assert.h>
#include <sstream>
#include <set>

#include "NestedList.h"
#include "LogMsg.h"


/*

1 Overview

The class ~NList~ can be used as replacement for the syntactical ~noisy~
interface defined in ~NestedList.h~. Currently, not all methods of the ~old~
interface are supported, but may be easily added when needed.

This interface simplifes the check of a list structure, e.g.  member function
~isSymbol()~ will return TRUE, if ~n~ is a symbol atom. I think this will make
type mappings better readable since most Operations are done by
~list.function()~ instead of ~nl->Function(list)~. For example retrieving the
second element of the first element can be done by

----    old: nl->Second(nl->First(list))
        new: nlist.first().second()
----

Conversion from/to type ~ListExpr~ which is used inside SECONDO whenever nested
lists are used is done by

---- #include "NList.h"

     ListExpr f(ListExpr list)
     {
       Nlist nlist(list);
       ...
       return nlist.listExpr();
     }
----

*/


/*
2 String Constants


Type ~Symbols~ defines some string constants for symbols reserved as keywords in
the query processor or often used types. This should be used to avoid writing
always redundant string constants into the code.

*/

using namespace std;

struct Symbols {

    Symbols() {}

    static const string& STRING() {
       static string s("string"); return s;
    }
#ifdef SECONDO_WIN32
#undef TEXT
#endif
    static const string& TEXT() {
       static string s("text"); return s;
    }
    static const string& INT() {
       static string s("int"); return s;
    }
    static const string& REAL() {
       static string s("real"); return s;
    }
    static const string& real() {
       static string s("real"); return s;
    }
    static const string& BOOL() {
       static string s("bool"); return s;
    }
    static const string& REL() {
       static string s("rel"); return s;
    }
    static const string& TUPLE() {
       static string s("tuple"); return s;
    }
    static const string& PTUPLE() {
       static string s("ptuple"); return s;
    }
    static const string& STREAM() {
       static string s("stream"); return s;
    }
    static const string& MAP() {
       static string s("map"); return s;
    }
    static const string& TYPEERROR() {
       static string s("typeerror"); return s;
    }
    static const string& APPEND() {
       static string s("APPEND"); return s;
    }
};


class NList;

ostream& operator<<(ostream& os, const NList& n);


// defined in Secondointerface.cpp
extern Symbols sym;

/*
3 Exception Class ~NListErr~

A class representing nested list exceptions.

*/

class NListErr : public SecondoException {

  public:
  NListErr(const string& Msg) : SecondoException(Msg) {}
  NListErr(const Cardinal n, const Cardinal len, const string& ext="") :
    SecondoException()
  {
    stringstream s;
    s << "Out of range exception in nested list! "
      << "Element " << n << " requested, but "
      << "list has length " << len << "." << ext;
    msgStr = s.str();
  }

  const string msg() { return msgStr; }

};


#define CHECK(n) {if (n>0 && length()<n) { \
        throw NListErr("Element "#n" not in list!"); } }

/*
4 Class ~NList~

This is a ~wrapper~ class for values of type ~ListExpr~.

*/

class NList {


 public:

/*
2.2 Construction and Destruction

  * ~NList()~: Empty List.

  * NList(const NList\& rhs): Create a new reference to ~rhs~.

  * ~NList(const ListExpr list)~: Initialize with ~list~.

  * ~NList(const NList\& a, const NList\& b)~: Create a two elem list ~(a b)~.

  * ~NList(const NList\& a, const NList\& b, const NList\& c)~: Three elems ~(a b
c)~.

  * ~NList(const string\& s, const bool isStr = false)~: Create a symbol or string atom.

*/

  NList(NestedList* ptr = nlGlobal) :
    nl(ptr),
    l(nl->Empty()),
    e(nl->Empty()),
    len(0)
  {}

  NList(const NList& rhs) :
    nl(rhs.nl),
    l(rhs.l),
    e(rhs.e),
    len(rhs.len)
  {}

  NList(const ListExpr list, NestedList* ptr = nlGlobal) :
    nl(ptr),
    l(list),
    e(nl->Empty())
  { findEnd();
  }


  NList(const ListExpr list, int knownLen, NestedList* ptr = nlGlobal) :
    nl(ptr),
    l(list),
    e(nl->Empty()),
    len(knownLen)
  {
    findEnd();
  }

  NList(const NList& a, const NList& b) :
    nl(nlGlobal),
    l( nl->TwoElemList(a.l, b.l) ),
    e( b.l ),
    len(2)
  { findEnd(); }

  NList(const NList& a, const NList& b, const NList& c) :
    nl(nlGlobal),
    l( nl->ThreeElemList(a.l, b.l, c.l) ),
    e( c.l ),
    len(3)
  { findEnd(); }

  NList(const NList& a, const NList& b, const NList& c, const NList& d) :
    nl(nlGlobal),
    l( nl->FourElemList(a.l, b.l, c.l, d.l) ),
    e( d.l ),
    len(4)
  { findEnd(); }


  NList(const NList& a, const NList& b, const NList& c, const NList& d,
       const NList& _e) :
    nl(nlGlobal),
    l( nl->FiveElemList(a.l, b.l, c.l, d.l, _e.l) ),
    e( _e.l ),
    len(5)
  { findEnd(); }

  NList(const NList& a, const NList& b, const NList& c, const NList& d,
       const NList& _e,const NList& f) :
    nl(nlGlobal),
    l( nl->SixElemList(a.l, b.l, c.l, d.l, _e.l, f.l) ),
    e( f.l ),
    len(6)
  { findEnd();}


  // INSTANTIATION PATTERNS:
  // (value), (value, false), (value, false, false) : symbolAtom  = DEFAULT1
  // (value, true),           (value, true,  false) : stringAtom  = DEFAULT2
  // (value, true,  true )                          : textAtom

  NList(const string& s, const bool isStr = false, const bool isText = false):
    nl(nlGlobal),
    e( nl->Empty() ),
    len(0)
  {
    if (isStr && !isText)
      l = nl->StringAtom(s);
    else if (isStr && isText)
      l = nl->TextAtom(s);
    else
      l = nl->SymbolAtom(s);
  }

  NList(const string& s1, const string& s2) :
    nl(nlGlobal),
    len(2)
  {
    e = nl->SymbolAtom(s2);
    l = nl->TwoElemList(nl->SymbolAtom(s1), e);
  }

  NList(const string& s1, const string& s2, const string& s3) :
    nl(nlGlobal),
    len(3)
  {
    e = nl->SymbolAtom(s3);
    l = nl->ThreeElemList(nl->SymbolAtom(s1), nl->SymbolAtom(s2), e);
  }


   NList(const bool b, const bool dummy) :
       nl(nlGlobal),
       l( nl->BoolAtom(b) ),
       e( nl->Empty() ),
       len(0)
   {}

  NList(const int n) :
    nl(nlGlobal),
    l( nl->IntAtom(n) ),
    e( nl->Empty() ),
    len(0)
  {}

  NList(const double d) :
    nl(nlGlobal),
    l( nl->RealAtom(d) ),
    e( nl->Empty() ),
    len(0)
  {}

 inline NList& boolAtom(const bool val)
  {
    nl = nlGlobal;
    len = 0;
    e = nl->Empty();
    l = nl->BoolAtom(val);
    return *this;
  }

  inline NList& intAtom(const int val)
  {
    nl = nlGlobal;
    len = 0;
    e = nl->Empty();
    l = nl->IntAtom(val);
    return *this;
  }

  inline NList& realAtom(const double val)
  {
    nl = nlGlobal;
    len = 0;
    e = nl->Empty();
    l = nl->RealAtom(val);
    return *this;
  }

  inline NList& stringAtom(const string& val)
  {
    nl = nlGlobal;
    len = 0;
    e = nl->Empty();
    l = nl->StringAtom(val);
    return *this;
  }

  inline NList& textAtom(const string& val)
  {
    nl = nlGlobal;
    len = 0;
    e = nl->Empty();
    l = nl->TextAtom(val);
    return *this;
  }

  inline NList& symbolAtom(const string& val)
  {
    nl = nlGlobal;
    len = 0;
    e = nl->Empty();
    l = nl->SymbolAtom(val);
    return *this;
  }

  inline NList& enclose()
  {
    nl = nlGlobal;
    len = 1;
    l = nl->OneElemList(l);
    e = l;
    return *this;
  }

  ~NList() {}

/*
2.3 Structure Tests and Value Retrieval

The following functions can be used to get information about the
list stucture and to extract subexpressions or atom values.

*/

  inline bool isEmpty() const { return nl->IsEmpty(l); }

  inline bool isEqual(const string& s) const {
    return nl->IsEqual(l, s);
  }
  inline bool isAtom() const { return nl->IsAtom(l); }

  inline bool isSymbol(const Cardinal n = 0) const {
    CHECK(n) return isNodeType(n, SymbolType);
  }
  inline bool isSymbol(const string& s) const {
    return isNodeType(0, SymbolType) && (s == nl->SymbolValue(l));
  }
  inline bool isString(const Cardinal n = 0) const {
    CHECK(n) return isNodeType(n, StringType);
  }
  inline bool isText(const Cardinal n = 0) const {
    CHECK(n) return isNodeType(n, TextType);
  }
  inline bool isNoAtom(const Cardinal n = 0) const {
    CHECK(n) return isNodeType(n, NoAtom);
  }
  inline bool isList(const Cardinal n = 0) const {
    CHECK(n) return isNodeType(n, NoAtom);
  }
  inline bool isInt(const Cardinal n  = 0) const {
    CHECK(n) return isNodeType(n, IntType);
  }
  inline bool isReal(const Cardinal n = 0) const {
    CHECK(n) return isNodeType(n, RealType);
  }
  inline bool isBool(const Cardinal n = 0) const {
    CHECK(n) return isNodeType(n, BoolType);
  }

  inline bool hasLength(const Cardinal c) const { return (length() == c);  }

  inline NList first()  const { return elem(1); }
  inline NList second() const { return elem(2); }
  inline NList third()  const { return elem(3); }
  inline NList fourth() const { return elem(4); }
  inline NList fifth()  const { return elem(5); }
  inline NList sixth()  const { return elem(6); }
  inline NList seventh() const { return elem(7); }
  inline NList eigth() const { return elem(8); }
  inline NList nineth() const { return elem(9); }
  inline NList tenth() const { return elem(10); }
  inline NList eleventh() const {return elem(11); }
  inline NList twelfth() const {return elem(12); }

  inline Cardinal length() const
  {
    return len;
  }

  inline NList elem(Cardinal n) const
  {
    if ( (length() < n)   ) {
      throw NListErr(n,len);
    }

    if ( (n >= 1) && isAtom() ) {
      throw NListErr("Element 1 requested but list is an Atom!");
    }

    return NList(nl->Nth(n,l));
  }

  // for usage in type mappings
  inline static ListExpr typeError(const string& msg)
  {
    ErrorReporter::ReportError(msg);
    return nlGlobal->TypeError();
  }

  inline static ListExpr typeError( int argNumber,
		                    const string& expectedListStruct )
  {
    stringstream msg;
    msg << "Expecting list structure " << expectedListStruct
	<< " for argument number "<< argNumber << ".";

    ErrorReporter::ReportError( msg.str() );
    return nlGlobal->TypeError();
  }


  // interchange with type ~ListExpr~
  inline ListExpr listExpr() const { return l; }

/*
Functions applicable for ~symbol/string/text~ atoms.

*/

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

  inline int intval() const
  {
    return nl->IntValue(l);
  }

  inline double realval() const
  {
    return nl->RealValue(l);
  }

  inline int boolval() const
  {
    return nl->BoolValue(l);
  }


/*
Conversion to C++ strings or a string Atom. The latter conversion can be
used to convert a symbol or text atom into a string atom.

*/
  inline string convertToString() const { return nl->ToString(l); }

  inline NList toStringAtom() {
    const string& s = str();
    return NList(s, true);
  }

/*
Comparison between ~NList/NList~ and ~NList/string~ instances

*/
  inline bool operator==(const NList& rhs) const
  {
    return nl->Equal(l,rhs.l);
  }

  inline bool operator!=(const NList& rhs) const
  {
    return !(*this == rhs);
  }

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

 inline bool operator<(const NList& rhs) const
  {
    return (len < rhs.len);
  }



/*
Write the text or binary representation of the nested list
to a given ~ostream~ reference.

*/

  inline bool writeAsStringTo( ostream& os ) const
  {
    return nl->WriteStringTo(l,os);
  }
  inline bool writeAsBinaryTo( ostream& os ) const
  {
    return nl->WriteBinaryTo(l,os);
  }

/*
Construction of bigger lists

*/
  inline void makeHead(const NList& head)
  {
    l = nl->OneElemList( head.l );
    e = l;
    len = 1;
  }

  inline void append(const NList& tail)
  {
     if(this->isEmpty()){
       *this = tail;
       makeHead(*this);
       return;
     }

     if (len == 0) { // an atom
       makeHead((*this));
     } 
     e = nl->Append(e, tail.l);
     len++;
  }

  inline void concat(const NList& tail){
     if(this->isEmpty()){
       (*this)  = tail;
       return;
     }
     if(tail.isEmpty()){
       return;
     }
     if(tail.isAtom()){
       append(tail);
       return;
     }
     if(isAtom()){
       makeHead(*this);
     }
     NList rest(tail);
     while(!rest.isEmpty()){
        append(rest.first());
					rest.rest();
     }
  }




/*
Iteration over lists

The function ~rest~ removes the first element of a list.

*/

  inline void rest() { 
       l = nl->Rest(l); 
       if(nl->IsEmpty(l)){
         e = nl->TheEmptyList();
       }
  }


/*
3 Support for implementing type mappings

The functions below may be useful for implementing type mapping
functions.

*/

  inline bool checkAtomList(NList al)
  {
    while( !al.isEmpty() )
    {
       if ( ! al.first().isAtom() ) {
         return false;
       }
       al.rest();
    }
    return true;
  }

 inline bool checkSymbolList(NList sl)
  {
    while( !sl.isEmpty() )
    {
       if ( ! sl.first().isSymbol() ) {
         return false;
       }
       sl.rest();
    }
    return true;
  }



  inline bool checkUniqueMembers(NList m)
  {
    set<string> tree;
    while( !m.isEmpty() )
    {
       if ( tree.find(m.first().convertToString()) != tree.end() ) {
         return false;
       }
       tree.insert( m.first().convertToString() );
       m.rest();
    }
    return true;
  }

  inline bool checkStream(NList& type)
  {
    return ( hasLength(2) && first().isSymbol(sym.STREAM())
		          && second() == type );
  }

  inline NList& streamOf(NList& type)
  {
    l = nl->OneElemList( nl->SymbolAtom(sym.STREAM()) );
    e = l;
    len = 1;
    append(type);
    return *this;
  }

  inline NList& tupleOf(NList& type)
  {
    l = nl->OneElemList( nl->SymbolAtom(sym.TUPLE()) );
    e = l;
    len = 1;
    append(type);
    return *this;
  }

  inline NList& tupleStreamOf(NList& type)
  {
    l = nl->OneElemList( nl->SymbolAtom(sym.STREAM()) );
    e = l;
    len = 1;
    append( NList().tupleOf(type) );
    return *this;
  }


  inline bool checkStreamTuple(NList& attrs)
  {
    return checkDepth3(sym.STREAM(), sym.TUPLE(), attrs);
  }

  inline bool checkRel(NList& attrs)
  {
    return checkDepth3(sym.REL(), sym.TUPLE(), attrs);
  }

  inline bool checkLength(const int len, string& err)
  {
    if ( !hasLength(len) )
    {
      stringstream s;
      s << "List length unequal " << len <<  ", " << err;
      err = s.str();
      return false;
    }
    return true;
  }


/*
The function below resets the global nested list pointer. This
is necessary to switch from Kernel-Instance to the Application-Instance.

Use with care! Normally you will not need this.

*/
 static void setNLRef(NestedList* ptr) { nlGlobal = ptr; }
 void showNLRefs() {
    cerr << "nlGlobal:" << nlGlobal << endl;
    cerr << "nl:" << nl << endl;
 }


  ostream& print(ostream& o){
    o << "[NList :  l =" << nl->ToString(l) << ",  e = " <<  nl->ToString(e) 
      << ", len = " << len << "]";
    return o; 
  }


 private:
  static NestedList* nlGlobal;
  NestedList* nl;
  ListExpr l;
  ListExpr e; // points to the last element of a list
  Cardinal len;
 
   inline bool isNodeType(const int n, const NodeType t) const
  {
    if ( !n )
      return ( /*nl->IsAtom(l) &&*/ (nl->AtomType(l) == t) );

    ListExpr list = nl->Nth(n,l);
    return ( /*nl->IsAtom(list) &&*/ (nl->AtomType(list) == t) );
  }

  inline bool checkDepth3(const string& s1, const string& s2, NList& attrs)
  {

  if ( !hasLength(2) )
    return false;

  if ( !first().isSymbol(s1) )
    return false;

  NList s = second();
  if ( !s.hasLength(2) )
    return false;

  if ( !s.first().isSymbol(s2) )
    return false;

  if ( !s.second().isList() )
    return false;

  attrs = s.second();
  return true;
  }

/*
~findEnd~ sets the internal pointer to the end of the list to the correct value.
If the list is just an atom, the end will be an empty list.

*/

  void findEnd(){
     if(nl->IsAtom(l)){
       e = nl->TheEmptyList();
       len = 0;
     } else if(nl->IsEmpty(l)){
       e = l;
       len = 0;
     } else {
       e = l;
       len = 1;
       while(!nl->IsEmpty(nl->Rest(e))){
          e = nl->Rest(e);
          len++;
       }
     }
  }


};


#endif
