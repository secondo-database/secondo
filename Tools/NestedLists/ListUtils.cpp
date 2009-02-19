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


*/

#include "ListUtils.h"
#include "NestedList.h"
#include "SecondoSystem.h"

#include <set>
#include <string>

extern NestedList* nl;

namespace listutils{

/*
 Returns a list containing a symbol "ERROR";

*/

  ListExpr emptyErrorInfo(){
    return nl->OneElemList(nl->SymbolAtom("ERROR"));
  }

/*
Checks for a Spatial type.

*/

  bool isSpatialType(ListExpr arg){
    AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
    ListExpr errorInfo = emptyErrorInfo();
    return algMgr->CheckKind("SPATIAL2D", arg, errorInfo) ||
            algMgr->CheckKind("SPATIAL3D", arg, errorInfo) ||
            algMgr->CheckKind("SPATIAL4D", arg, errorInfo) ||
            algMgr->CheckKind("SPATIAL8D", arg, errorInfo);      
  }

/*
Checks for a rectangle type 

*/

  bool isRectangle(ListExpr arg){
    return nl->IsEqual(arg, "rect") ||
           nl->IsEqual(arg, "rect3") ||
           nl->IsEqual(arg, "rect4") ||
           nl->IsEqual(arg, "rect8");
 
  }


/*
Creates a list "typeerror".

*/
  ListExpr typeError(){
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

   if( (rtreestr != "rtree") &&
       (rtreestr != "rtree3") &&
       (rtreestr != "rtree4") &&
       (rtreestr != "rtree8") ){
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
Checks for valid description of a tuple.

*/ 

  bool isTupleDescription(ListExpr tuple){
    if(nl->ListLength(tuple)!=2){
       return false;
    }
    if(!nl->IsEqual(nl->First(tuple),"tuple")){
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
      if(!am->CheckKind("DATA", attrType, errorInfo)){
         return false;
      }
    }
    return true;
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
     if(t=="rtree") return 2;
     if(t=="rtree3") return 3;
     if(t=="rtree4") return 4;
     if(t=="rtree8") return 8;
     assert(false);
     return -1; 
  }

/*
Checks for a valid relation description.

*/
  bool isRelDescription(ListExpr rel, const bool trel /*=false*/){
    if(nl->ListLength(rel)!=2){
       return false;
    }
    string relsymb = trel?"trel":"rel";
    if(!nl->IsEqual(nl->First(rel),relsymb)){
       return false;
    }
    return isTupleDescription(nl->Second(rel));   
  }

/*
Checks for a tuple stream

*/
  bool isTupleStream(ListExpr s){
    if(nl->ListLength(s)!=2){
       return false;
    }
    if(!nl->IsEqual(nl->First(s),"stream")){
       return false;
    }
    return isTupleDescription(nl->Second(s));
  }

/*
Checks for a stream of kind DATA

*/
  bool isDATAStream(ListExpr s){
    if(nl->ListLength(s)!=2){
       return false;
    }
    if(!nl->IsEqual(nl->First(s),"stream")){
       return false;
    }
    AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
    ListExpr errorInfo = emptyErrorInfo();
    return algMgr->CheckKind("DATA", nl->Second(s), errorInfo);
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
                string& name, int start/*=1*/){
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
} // end of namespace listutils
