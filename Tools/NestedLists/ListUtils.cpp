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

#include "ListUtils.h"
#include "NestedList.h"
#include "SecondoSystem.h"

#include "Symbols.h"            // Kind::*, Symbol::*
#include "StandardTypes.h"      // int, real, bool, string
#include "OldRelationAlgebra.h" // rel, tuple
#include "RelationAlgebra.h"    // rel, trel, tuple
#include "OrderedRelationAlgebra.h" // orel
#include "TupleIdentifier.h"    // tid
#include "RectangleAlgebra.h"   // rect, rect3, rect4, rect8
#include "RTreeAlgebra.h"       // rtree, rtree3, rtree4, rtee8
#include "BTreeAlgebra.h"       // btree
#include "BTree2.h"             // btree2
#include "HashAlgebra.h"        // hash


#include <set>
#include <string>
#include <stdarg.h>

extern NestedList* nl;

namespace listutils{

/*
 Returns a list containing a symbol "ERROR";

*/

  ListExpr emptyErrorInfo(){
    return nl->OneElemList(nl->SymbolAtom(Symbol::ERROR()));
  }

/*
Checks for a Spatial type.

*/

  bool isSpatialType(ListExpr arg){
    AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
    ListExpr errorInfo = emptyErrorInfo();
    return algMgr->CheckKind(Kind::SPATIAL2D(), arg, errorInfo) ||
           algMgr->CheckKind(Kind::SPATIAL3D(), arg, errorInfo) ||
           algMgr->CheckKind(Kind::SPATIAL4D(), arg, errorInfo) ||
           algMgr->CheckKind(Kind::SPATIAL8D(), arg, errorInfo);
  }

/*
Checks for a rectangle type

*/

  bool isRectangle(ListExpr arg){
    return nl->IsEqual(arg, Rectangle<2>::BasicType()) ||
           nl->IsEqual(arg, Rectangle<3>::BasicType()) ||
           nl->IsEqual(arg, Rectangle<4>::BasicType()) ||
           nl->IsEqual(arg, Rectangle<8>::BasicType());

  }



/*
Creates a list "typeerror".

*/
  ListExpr typeError(){
     return nl->TypeError();
  }

/*
Writes a message to the errorreporter and returns
"typeerror".

*/
  ListExpr typeError(string message){
    ErrorReporter::ReportError(message);
    return nl->TypeError();
  }


/*
Checks for a valid description of an rtree.


*/

  bool isRTreeDescription(ListExpr rtree){
   // (rtree tupledescription type bool)
   if(nl->ListLength(rtree)!=4){
     return false;
   }
   ListExpr rtreeSymbol = nl->First(rtree);
   if(nl->AtomType(rtreeSymbol)!=SymbolType){
     return false;
   }
   string rtreestr = nl->SymbolValue(rtreeSymbol);

   if( (rtreestr != RTree2TID::BasicType()) &&
       (rtreestr != RTree3TID::BasicType()) &&
       (rtreestr != RTree4TID::BasicType()) &&
       (rtreestr != RTree8TID::BasicType()) ){
      return false;
   }

   if(!isTupleDescription(nl->Second(rtree))){
     return false;
   }
   if(nl->AtomType(nl->Fourth(rtree))!= BoolType){
     return false;
   }
   // check for valid type as third element omitted
   return true;
  }


  bool isRTreeDescription(ListExpr rtree, const string& basicType){
   // (rtree tupledescription type bool)
   if(nl->ListLength(rtree)!=4){
     return false;
   }
   ListExpr rtreeSymbol = nl->First(rtree);
   if(nl->AtomType(rtreeSymbol)!=SymbolType){
     return false;
   }
   string rtreestr = nl->SymbolValue(rtreeSymbol);

   if( rtreestr != basicType) {
      return false;
   }

   if(!isTupleDescription(nl->Second(rtree))){
     return false;
   }
   if(nl->AtomType(nl->Fourth(rtree))!= BoolType){
     return false;
   }
   // check for valid type as third element omitted
   return true;
  }


/*
Checks for a BTreeDescription

*/
bool isBTreeDescription(ListExpr btree){
  if((nl->ListLength(btree)<3) || (nl->ListLength(btree)>4)){
    return false;
  }
  return nl->IsEqual(nl->First(btree),BTree::BasicType()) &&
         isTupleDescription(nl->Second(btree)) &&
         isDATA(nl->Third(btree)) &&
         ((nl->ListLength(btree) == 3) ||
           isKeyDescription(nl->Second(btree), nl->Fourth(btree)));
}


/*
Checks wether the list given as argument is a type usuable as
an key for a Berkeley DB index.

*/
bool isBDBIndexableType(ListExpr key){
  ListExpr errorInfo = emptyErrorInfo();
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  bool indexable = algMgr->CheckKind(Kind::INDEXABLE(), key,errorInfo) ||
                   (nl->IsAtom(key) &&
                    nl->AtomType(key) == SymbolType &&
                    (nl->SymbolValue(key) == CcInt::BasicType() ||
                     nl->SymbolValue(key) == TupleIdentifier::BasicType() ||
                     nl->SymbolValue(key) == CcReal::BasicType() ||
                     nl->SymbolValue(key) == CcBool::BasicType() ||
                     nl->SymbolValue(key) == CcString::BasicType()));
  return indexable;
}


/*
Checks for a BTree2Description

*/
bool isBTree2Description(ListExpr btree2){
  if(nl->ListLength(btree2)!=4){
    return false;
  }
  ListExpr uniq = nl->Fourth(btree2);
  ListExpr key = nl->Second(btree2);
  bool indexable = isBDBIndexableType(key);
  return (nl->IsEqual(nl->First(btree2),BTree2Algebra::BTree2::BasicType()) &&
            indexable &&
            nl->IsAtom(nl->Third(btree2)) &&
            nl->AtomType(uniq) == SymbolType) &&
            (nl->SymbolValue(uniq) == "multiple" ||
            nl->SymbolValue(uniq) == "uniqueKey" ||
            nl->SymbolValue(uniq) == "uniqueKeyMultiData");
}

/*
 Checks for a valid hash table

*/

  bool isHashDescription(ListExpr hash){
    if(nl->ListLength(hash)!=3){
      return false;
    }
    return isSymbol(nl->First(hash),Hash::BasicType()) &&
         isTupleDescription(nl->Second(hash)) &&
         isDATA(nl->Third(hash));
  }


/*
Checks for valid description of a tuple.

*/

  bool isTupleDescription(ListExpr tuple, const bool ismtuple /*=false*/){
    if(nl->ListLength(tuple)!=2){
       return false;
    }
    string tuplesym = ismtuple?CcTuple::BasicType():Tuple::BasicType();
    if(!isSymbol(nl->First(tuple),tuplesym)){
       return false;
    }
    return isAttrList(nl->Second(tuple));
  }

/*
Checks for a valid atribute list

*/
  bool isAttrList(ListExpr attrList){
    ListExpr rest = attrList;
    ListExpr current;
    ListExpr errorInfo = emptyErrorInfo();

    if(nl->AtomType(attrList)!=NoAtom){
       return  false;
    }
    if(nl->IsEmpty(attrList)){
       return false;
    }
    set<string> attrnames;
    while(!nl->IsEmpty(rest)) {
      current = nl->First(rest);
      rest = nl->Rest(rest);
      if(nl->ListLength(current)!=2){
         return false;
      }
      ListExpr attrName = nl->First(current);
      ListExpr attrType = nl->Second(current);
      if(nl->AtomType(attrName)!=SymbolType){
        return false;
      }
      string name = nl->SymbolValue(attrName);
      if(attrnames.find(name)!=attrnames.end()){
         return false;
      }
      attrnames.insert(name);
      if(!am->CheckKind(Kind::DATA(), attrType, errorInfo)){
         return false;
      }
    }
    return true;
  }

/*
Checks an attribute list for naming conventions

*/
  bool checkAttrListForNamingConventions(ListExpr attrList){
     // assert(isAttrList(attrList));
     while(!nl->IsEmpty(attrList)){
        if(!checkAttrForNamingConventions(nl->First(nl->First(attrList)))){
          return false;
        }
        attrList = nl->Rest(attrList);
     }
     return true;
 
  }

/*
Checks an attribute name for naming conventions.

*/
  bool checkAttrForNamingConventions(const ListExpr attr){
     // assert(isSymbol(attr))
     string name = nl->SymbolValue(attr);
     if(name.length()<1){
       return false;
     }
     char f = name[0];
     if(f<'A' || f>'Z'){
        return false;
     } else {
        return true;
     }
  }


 bool isValidAttributeName(const ListExpr attr, string& error){
     if(!isSymbol(attr)){
        error = "attribute name is not a symbol";
        return false;
     }
     string attrstr = nl->SymbolValue(attr);
     if(!SecondoSystem::GetCatalog()->IsValidIdentifier(attrstr,
                                                        error)){
         error = "attribute name "+ error + ".";
         return false;
     }  
     if(!checkAttrForNamingConventions(attr)){
        error = attrstr + " does not fit Secondo's names conventions";
        return false;
     }
     return true;
 } 


/*
Checks for disjoint attribute lists.

Precondition isAttrList(l1) [&]  isAttrList(l2)

*/
  bool disjointAttrNames(ListExpr l1, ListExpr l2){
    assert(isAttrList(l1));
    assert(isAttrList(l2));
    set<string> names;
    ListExpr rest = l1;
    while(!nl->IsEmpty(rest)){
      names.insert(nl->SymbolValue(nl->First(nl->First(rest))));
      rest = nl->Rest(rest);
    }
    rest = l2;
    while(!nl->IsEmpty(rest)){
      string name = nl->SymbolValue(nl->First(nl->First(rest)));
      if(names.find(name)!=names.end()){
        return false;
      }
      rest = nl->Rest(rest);
    }
    return true;
  }

/*
Checks whether the list corresponds to a given symbol.

*/
 bool isSymbol(const ListExpr list, const string& v){
   if(nl->AtomType(list)!=SymbolType){
     return false;
   }
   return nl->SymbolValue(list) == v;
 }

 bool isSymbol(const ListExpr list){
   return nl->IsAtom(list) && (nl->AtomType(list)==SymbolType);
 }

 bool isASymbolIn(const ListExpr list, const set<string>& s){
   if(!isSymbol(list)){
     return false;
   }
   string v = nl->SymbolValue(list);
   return s.find(v)!=s.end();
 }

/*
Concatenates l1 and l2.

*/
 ListExpr concat(ListExpr l1, ListExpr l2){
   assert(nl->AtomType(l1) == NoAtom);
   assert(nl->AtomType(l2) == NoAtom);
   if(nl->IsEmpty(l1)){
     return l2;
   }
   ListExpr res = nl->OneElemList(nl->First(l1));
   l1 = nl->Rest(l1);
   ListExpr last = res;
   while(!nl->IsEmpty(l1)){
     last = nl->Append(last, nl->First(l1));
     l1 = nl->Rest(l1);
   }
   while(!nl->IsEmpty(l2)){
     last = nl->Append(last, nl->First(l2));
     l2 = nl->Rest(l2);
   }
   return res;
 }





/*
Returns the keytype fo an rtree description.

*/
  ListExpr getRTreeType(ListExpr rtree){
     assert(isRTreeDescription(rtree));
     return nl->Third(rtree);
  }

/*
Returns the dimension of an rtree.

*/
  int getRTreeDim(ListExpr rtree){
     assert(isRTreeDescription(rtree));
     string t = nl->SymbolValue(nl->First(rtree));
     if(t==RTree2TID::BasicType()) return 2;
     if(t==RTree3TID::BasicType()) return 3;
     if(t==RTree4TID::BasicType()) return 4;
     if(t==RTree8TID::BasicType()) return 8;
     assert(false);
     return -1;
  }

/*
Checks for a valid relation description.

*/
  bool isRelDescription(ListExpr rel, const bool trel /*=false*/){
    string relsymb = trel?TempRelation::BasicType():Relation::BasicType();
    return isRelDescription2(rel, relsymb);
  }


  bool isRelDescription2(ListExpr rel, const string& relsymb){
    if(nl->ListLength(rel)!=2){
       return false;
    }
    if(!isSymbol(nl->First(rel),relsymb)){
       return false;
    }
    bool mtuple = relsymb==CcRel::BasicType();
    return isTupleDescription(nl->Second(rel),mtuple);
  }

  bool isOrelDescription(ListExpr orel) {
    if(nl->ListLength(orel)!=3) {
      return false;
    }
    return isSymbol(nl->First(orel),OrderedRelation::BasicType()) &&
            isTupleDescription(nl->Second(orel),false) &&
            isKeyDescription(nl->Second(orel), nl->Third(orel));
  }

  bool isKeyDescription(ListExpr tupleList, ListExpr keyList) {
    ListExpr attrType;
    int attrIndex;
    if(nl->IsAtom(keyList)) {
      if(nl->AtomType(keyList)!=SymbolType)
        return false;
      attrIndex = findAttribute(nl->Second(tupleList),
                                nl->SymbolValue(keyList), attrType);
      return attrIndex > 0 && isBDBIndexableType(attrType);
    }
    while(!nl->IsEmpty(keyList)) {
      ListExpr elem = nl->First(keyList);
      if(nl->AtomType(elem)!=SymbolType) {
        return false;
      }
      keyList = nl->Rest(keyList);
      ListExpr attrType;
      int attrIndex = findAttribute(nl->Second(tupleList),
                                    nl->SymbolValue(elem), attrType);
      if (attrIndex == 0 || !isBDBIndexableType(attrType)) {
        return false;
      }
    }
    return true;
  }



/*
Checks for a tuple stream

*/
  bool isTupleStream(ListExpr s){
    if(nl->ListLength(s)!=2){
       return false;
    }
    if(!nl->IsEqual(nl->First(s),Symbol::STREAM())){
       return false;
    }
    return isTupleDescription(nl->Second(s));
  }

/*
Checks for Kind DATA

*/
bool isDATA(ListExpr type){
 AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
 ListExpr errorInfo = emptyErrorInfo();
 return algMgr->CheckKind(Kind::DATA(), type, errorInfo);
}


/*
CHecks whether this list corresponds to a type in given kind

*/
  bool isKind(ListExpr type, const string& kind){
    AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
    ListExpr errorInfo = emptyErrorInfo();
    return algMgr->CheckKind(kind, type, errorInfo);
  }


/*
 Checks for a numeric type

*/
bool isNumeric(ListExpr num){
   return nl->AtomType(num) == IntType ||
          nl->AtomType(num) == RealType;
}

double getNumValue(ListExpr n){
  if(nl->AtomType(n)==IntType){
    return nl->IntValue(n);
  } if(nl->AtomType(n)==RealType){
    return nl->RealValue(n);
  } else {
    assert(false);
  }
  return 0.0;
}

bool isNumericType(ListExpr n){
  if(nl->AtomType(n)!=SymbolType){
     return false;
  }
  string v = nl->SymbolValue(n);
  return v==CcInt::BasicType() || v==CcReal::BasicType();
}



/*
Checks for a stream of kind DATA

*/
  bool isDATAStream(ListExpr s){
    if(nl->ListLength(s)!=2){
       return false;
    }
    if(!nl->IsEqual(nl->First(s),Symbol::STREAM())){
       return false;
    }
    AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
    ListExpr errorInfo = emptyErrorInfo();
    return algMgr->CheckKind(Kind::DATA(), nl->Second(s), errorInfo);
  }

/*
Checks whether the list represents a stream.

*/
  bool isStream(ListExpr s){
    if(nl->ListLength(s)!=2){
       return false;
    }
    if(!nl->IsEqual(nl->First(s),Symbol::STREAM())){
       return false;
    }
    return true;
  }






  int findAttribute(ListExpr attrList, const string& name, ListExpr& type){
     assert(isAttrList(attrList));
     int j = 0;
     ListExpr rest = attrList;
     while(!nl->IsEmpty(rest)){
       ListExpr current = nl->First(rest);
       j++;
       if(nl->IsEqual(nl->First(current),name)){
          type = nl->Second(current);
          return j;
       }
       rest = nl->Rest(rest);
     }
     return 0;
  }

  int findType(ListExpr attrList, const ListExpr type,
                string& name, const int start/*=1*/){
    assert(isAttrList(attrList));
    ListExpr rest = attrList;
    int j = 0;
    while(!nl->IsEmpty(rest)){
       ListExpr current = nl->First(rest);
       rest = nl->Rest(rest);
       j++;
       if(j>=start){
          if(nl->Equal(nl->Second(current),type)){
              name = nl->SymbolValue(nl->First(current));
              return j;
          }
       }
    }
    return 0;

  }

  int removeAttributes(ListExpr list, const set<string>& names,
                       ListExpr& head, ListExpr& last){
     assert(isAttrList(list));
     bool firstCall = true;
     int count = 0;
     while(!nl->IsEmpty(list)){
       ListExpr pair = nl->First(list);
       list = nl->Rest(list);
       string name = nl->SymbolValue(nl->First(pair));
       if(names.find(name)==names.end()){
         if(firstCall){
           firstCall=false;
           head = nl->OneElemList(pair);
           last = head;
         } else {
           last = nl->Append(last, pair);
         }
       } else {
         count++;
       }

     }
     if(firstCall){
      head = nl->TheEmptyList();
      last = head;
     }
     return count;
  }



  bool replaceAttributes( ListExpr attrList,
                          map<string, string>& renameMap,
                          ListExpr& resAttrList, string& errmsg){

    bool firstCall = true;
    ListExpr last = nl->TheEmptyList();
    map<string, string>::iterator it;
    while(!nl->IsEmpty(attrList)){
        ListExpr attr = nl->First(attrList);
        attrList = nl->Rest(attrList);
        string name = nl->SymbolValue(nl->First(attr));
        it =  renameMap.find(name);
        ListExpr newAttr;
        if(it==renameMap.end()){
           newAttr = attr;
        } else {
           newAttr = nl->TwoElemList( nl->SymbolAtom(it->second),
                                      nl->Second(attr));
           renameMap.erase(it);
        }
        if(firstCall){
          resAttrList = nl->OneElemList(newAttr);
          last = resAttrList;
          firstCall = false;
        } else {
          last = nl->Append(last, newAttr);
        }
    }
    if(firstCall){
      errmsg = "empty attribute list cannot be renamed";
      resAttrList = nl->TheEmptyList();
      return false;
    }
    if(!renameMap.empty()){
      it = renameMap.begin();
      errmsg = "attribute " + it->first + " not found in attrlist";
      resAttrList = nl->TheEmptyList();
      return false;
    }


    if(!isAttrList(resAttrList)){
      errmsg = "renamed names are not unique";
      resAttrList = nl->TheEmptyList();
      return  false;
    }

    return true;

  }

  bool isSymbolUndefined( const string s ) {
    return (s == Symbol::UNDEFINED()) ||
           (s == "undef")     || (s == "UNDEF") ||
           (s == "undefined") || (s == "UNDEFINED") ||
           (s == "null")      || (s == "NULL") ;
  }

  bool isSymbolUndefined( ListExpr le ){
    if(isSymbol(le)){
      string s = nl->SymbolValue(le);
      return isSymbolUndefined(s);
    } else {
      return false;
    }
  }

  ListExpr getUndefined(){
     return nl->SymbolAtom("undefined");
  }

  string getUndefinedString(){
     return "undefined";
  }

  ListExpr getPtrList(const void* ptr){
    // ensure that two int atoms can pick up a pointer
    assert(sizeof(void*) <= 8);
    uint32_t v1 = 0;
    uint32_t v2 = 0;
    uint64_t v = (uint64_t) ptr;
    v1 = v;         // lower bits
    v2 = (v >> 32); // higher bits
    ListExpr res = nl->TwoElemList(nl->IntAtom(v2), nl->IntAtom(v1));
    return res; 

  }

  bool isPtrList(const ListExpr list){

    return nl->HasLength(list,2) &&
           nl->AtomType(nl->First(list)) == IntType &&
           nl->AtomType(nl->Second(list)) == IntType;
  }

 void* getPtr( const ListExpr ptrList){
    uint64_t v2 = (uint32_t) nl->IntValue(nl->First(ptrList));
    uint64_t v1 = (uint32_t) nl->IntValue(nl->Second(ptrList));

    uint64_t v = (v2 << 32) | v1;
    return (void*) v;
 }
 

 string stringValue(ListExpr src){
   if(nl->AtomType(src)==StringType){
      return nl->StringValue(src);
   }
   if(nl->AtomType(src)==TextType){
      return nl->Text2String(src); 
   }
   return ""; 

 }

 ListExpr xElemList(int count, ...){
    if(count <= 0){
        return nl->TheEmptyList();
    }
    va_list ap;
    va_start(ap,count);
    
    ListExpr first = nl->OneElemList(va_arg(ap,ListExpr));
    ListExpr last = first;
    for(int i=1;i<count;i++){
       last = nl->Append(last, va_arg(ap,ListExpr));
    }

    va_end(ap);
    return first;
 }




} // end of namespace listutils
