
/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and Computer Science,
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

//[&] [\&]

*/

#ifndef LISTUTILS_H
#define LISTUTILS_H


#include "NestedList.h"
#include <set>


extern NestedList* nl;

namespace listutils{

/*
~emptyErrorInfo~

Returns a one elem lsit containing the symbol "ERROR".

*/
  ListExpr emptyErrorInfo();

/*

Permforms a kind check for SPATIAL?D

*/
  bool isSpatialType(ListExpr arg);

/*
CHecks whether this list corresponds to a type in given kind

*/
  bool isKind(ListExpr t, const string& kind);



/*
Check for rect, rect3, rect4, rect8

*/
  bool isRectangle(ListExpr args);


/*
Check for map

*/
  template<int noArgs>
  bool isMap(ListExpr map){

    int a = noArgs+2;


    return nl->ListLength(map)==a &&
           nl->AtomType(nl->First(map))==SymbolType &&
           nl->SymbolValue(nl->First(map)) == "map";

  }


/*
Returns the symbol typeerror.

*/
  ListExpr typeError();

/*
Writes the message to the errorReporter and returns the
symbol typeerror.

*/
  ListExpr typeError(string message);


/*
Checks for a valid rtree description.

*/
  bool isRTreeDescription(ListExpr rtree);

/*
Checks for a special rtree description

*/
 bool isRTreeDescription(ListExpr rtree, const string& basicType);


/*
Checks for a valid btree description.

*/
  bool isBTreeDescription(ListExpr btree);

/*
Checks for a valid btree2 description.

*/
  bool isBTree2Description(ListExpr btree2);

/*

 Checks for a valid hash table

*/
  bool isHashDescription(ListExpr hash);

/*
Checks for a valid tuple description

*/
  bool isTupleDescription(ListExpr tuple, const bool isMtuple = false);


/*
Checks for kind DATA

*/
  bool isDATA(ListExpr type);


/*
Checks for a numeric value

*/
 bool isNumeric(const ListExpr num);


 bool isSymbol(const ListExpr list);

 bool isSymbol(const ListExpr list, const string& v);

 bool isASymbolIn(const ListExpr list, const set<string>& s);


/*
transformes a pointer into a listexpr evaluable by the query processor

*/

 ListExpr getPtrList(const void* ptr);


/*
Checks whether this list represents a pointer

*/
 bool isPtrList(const ListExpr list);

/*
Converts a ptr list into a pointer.

*/
 void* getPtr(const ListExpr ptrList);




/*
Returns the numeric value.

*/
double getNumValue(const ListExpr n);

/*
Checks for a numeric type

*/
bool isNumericType(const ListExpr n);


/*
 Checks for a valid attribute list

*/
  bool isAttrList(ListExpr attrList);


/*
Checks an attribute list for naming conventions

*/
  bool checkAttrListForNamingConventions(ListExpr attrList);

/*
Checks an attribute name for naming conventions.

*/
 bool checkAttrForNamingConventions(const ListExpr attr);

/*
Checks whether a list represents a valid attribute name.

*/

 bool isValidAttributeName(const ListExpr attr, string& error);



/*
Checks for disjoint attribute lists.

Precondition isAttrList(l1) [&]  isAttrList(l2)

*/
  bool disjointAttrNames(ListExpr l1, ListExpr l2);



  ListExpr getRTreeType(ListExpr rtree);

  int getRTreeDim(ListExpr rtree);

/*
Checks for a valid description of a relation

*/
  bool isRelDescription(ListExpr rel, const bool trel =false);

  bool isRelDescription2(ListExpr rel, const string& reltype);

  bool isOrelDescription(ListExpr orel);

  bool isKeyDescription(ListExpr tupleList, ListExpr keyList);


/*
~isBDBIndexableType~

This function checks whether a type can be used as a key within a
Berkeley-DB structure.

*/
  bool isBDBIndexableType(ListExpr key);

/*
Checks for a tuple stream

*/
  bool isTupleStream(ListExpr s);


/*
Checks for a stream of kind DATA

*/
  bool isDATAStream(ListExpr s);


/*
 Checks wether the list represnets a stream.

*/
  bool isStream(ListExpr s);



/*

searches for an attribute name within the list.
The attrlist has to be a valid attribute list. If the attribute was found,
its position is returned and type is set to the corresponding attribute
type. If not is found, type remains unchanged and the result is 0.

*/

  int findAttribute(ListExpr attrList, const string& name, ListExpr& type);

/*
Searches a given type within an attribute list starting at the given position.
If the type was found, its positon is returned and the name is changed to the
name of the attribute. Otherwise, 0 is returned.

*/
  int findType(ListExpr attrList, const ListExpr type,
               string& name, const int start=1);


/*
Removes all attributes with names in __names__ from the list. It returns the
number of removed entries as well as the begin and the end of the resulting
list.

Precondition: isAttrList(list)

*/
  int removeAttributes(ListExpr list, const set<string>& names,
                       ListExpr& head, ListExpr& last);



  ListExpr concat(ListExpr l1, ListExpr l2);


/*
Replaces the attributenames in attrlist according to the map and stores the
result in resAttrlist. The returnvalue indicated the success. If the function
fails, e.g. if the attribute names are not uniwue in the result, the errmsg
parameter is set. The strings stored in the map must be valid symbols.

*/

  bool replaceAttributes( ListExpr attrList, map<string, string>& renameMap,
                          ListExpr& resAttrList, string& errmsg);

/*
Checks wheter a symbol or string is one of "undef", "UNDEF", "undefined",
"UNDEFINED", "null" or "NULL", which should all be handled equivalent to
const string Symbol::UNDEFINED().

These methods are intended to establish backward compatibility in IN-functions.
To represent undefined values in a nested list when implementing OUT-functions,
alway use const string Symbol::UNDEFINED()!

*/
  bool isSymbolUndefined( const string s );
  bool isSymbolUndefined( ListExpr le );


  ListExpr getUndefined(); 


/*
~basicSymbol~

Returns the basictype of a class as a symbol

*/
template<class C>
ListExpr basicSymbol(){
   return nl->SymbolAtom(C::BasicType());
}

} // end of namespace
#endif

