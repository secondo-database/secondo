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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the createbtree2 operator for the BTree2-Algebra

[TOC]

1 Defines and Includes

*/

#include "op_createbtree2.h"

#include "WinUnix.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"

#include "BTree2.h"
#include "BTree2Impl.h"

#include <limits>
#include "Symbols.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace BTree2Algebra {
namespace Operators {

/*
1.1 Default values and constants

*/
const double STD_FILL = 0.5;
/*
Default fill factor.

*/
const int STD_NODESIZE = 3072;
/*
Default record size.

*/
const int PAGE_CONST = 29;
/*
Used for determination of the maximum record size: PageSize - PAGECONST.

*/

/*
2.1 Type mapping function of operator

*/
ListExpr createbtree2::TypeMapping(ListExpr args)
{
  if ((nl->ListLength(args) < 2)) {
    return listutils::typeError("at least two arguments expected");
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(!listutils::isRelDescription(first) &&
     !listutils::isTupleStream(first)){
     return listutils::typeError("first arg is not a rel or a tuple stream");
  }

//  ListExpr tupleDescription = nl->Second(first);

  if( listutils::isSymbol(nl->First(first),Relation::BasicType()) ) {
    if(nl->AtomType(second)!=SymbolType){
      return listutils::typeError("second argument is not a valid attr name");
    }

    string name = nl->SymbolValue(second);

    int keyIndex;
    ListExpr keyType;
    keyIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
                                         name,keyType);
    if(keyIndex==0){
      return listutils::typeError("attr name " + name +
                                   " not found in attr list");
    }

    if(!listutils::isSymbol(keyType,CcString::BasicType()) &&
       !listutils::isSymbol(keyType,CcInt::BasicType()) &&
       !listutils::isSymbol(keyType,CcBool::BasicType()) &&
       !listutils::isSymbol(keyType,CcReal::BasicType()) &&
       !listutils::isKind(keyType,Kind::INDEXABLE())){
      return listutils::typeError("selected attribut not in kind INDEXABLE");
    }

    ListExpr appendArgs = nl->ThreeElemList(nl->RealAtom(STD_FILL),
                          nl->IntAtom(STD_NODESIZE), nl->IntAtom(keyIndex));

    if (nl->ListLength(args) >= 3) {
       ListExpr fillArg = nl->Third(args);
       if (!listutils::isSymbol(fillArg,CcReal::BasicType())) {
        return listutils::typeError
                ("Argument three expected to be real between 0 and 1");
      }
      appendArgs = nl->Rest(appendArgs);
    }

    if (nl->ListLength(args) >= 4) {
       ListExpr fillArg = nl->Fourth(args);
       if (!listutils::isSymbol(fillArg,CcInt::BasicType())) {
        return listutils::typeError("Argument four expected to be int");
      }
      appendArgs = nl->Rest(appendArgs);
    }
    if ((nl->ListLength(args) >= 5)) {
      return listutils::typeError("too many arguments");
    }

    return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                              appendArgs,
                              nl->FourElemList(
                                 nl->SymbolAtom(BTree2::BasicType()),
                                 keyType,
                                 nl->SymbolAtom(TupleIdentifier::BasicType()),
                                 nl->SymbolAtom("multiple")));
    } else { // nl->IsEqual(nl->First(first), Symbol::STREAM())
    // New createbtree2 operator: Distinguish by number of arguments
    if (nl->ListLength(args) == 6) {

      ListExpr fillArg = nl->Second(args);
      if (!listutils::isSymbol(fillArg,CcReal::BasicType())) {
        return listutils::typeError
                  ("Argument two expected to be real between 0 and 1");
      }

      ListExpr nodesizeArg = nl->Third(args);
      if (!listutils::isSymbol(nodesizeArg,CcInt::BasicType())) {
       return listutils::typeError("Argument three expected to be int");
      }

      string keyAttrName = nl->SymbolValue(nl->Fourth(args));
      string valueAttrName = nl->SymbolValue(nl->Fifth(args));

      int keyAttrIndex, valueAttrIndex;
      ListExpr keyAttrType, valueAttrType;

      keyAttrIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
           keyAttrName,keyAttrType);

      if(keyAttrIndex==0){
        return listutils::typeError("key attribute " + keyAttrName +
              "not found in attr list");
      }

      if (valueAttrName == "none") {
        valueAttrType = nl->SymbolAtom("none");
        valueAttrIndex=-1;
      } else {
        valueAttrIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
             valueAttrName,valueAttrType);
        if(valueAttrIndex==0){
          return listutils::typeError("value attribute " + valueAttrName +
                                       "not found in attr list");
        }
      }

      if(!listutils::isSymbol(keyAttrType,CcString::BasicType()) &&
         !listutils::isSymbol(keyAttrType,CcInt::BasicType()) &&
         !listutils::isSymbol(keyAttrType,CcBool::BasicType()) &&
         !listutils::isSymbol(keyAttrType,CcReal::BasicType()) &&
         !listutils::isKind(keyAttrType,Kind::INDEXABLE())){
        return listutils::typeError("selected key attribut not INDEXABLE");
      }

      ListExpr uniq = nl->Sixth(args);

      BTree2::multiplicityType u;

      if (listutils::isSymbol(uniq,"multiple")) {
        u = BTree2::multiple;
      } else if (listutils::isSymbol(uniq,"stable_multiple")) {
        u = BTree2::stable_multiple;
      } else if (listutils::isSymbol(uniq,"uniqueKeyMultiData")) {
        u = BTree2::uniqueKeyMultiData;
      } else if (listutils::isSymbol(uniq,"uniqueKey")) {
        u = BTree2::uniqueKey;
      } else {
         return listutils::typeError("ambiguity argument must be one of"
                  "'multiple','uniqueKeyMultiData','uniqueKey' or "
                  "'stable_multiple'");
      }
      return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                                nl->ThreeElemList( nl->IntAtom(keyAttrIndex),
                                     nl->IntAtom(valueAttrIndex),
                                     nl->IntAtom(u)),
                                nl->FourElemList(
                                  nl->SymbolAtom(BTree2::BasicType()),
                                  keyAttrType,
                                  valueAttrType,
                                  uniq));
    } else {
      // Find the attribute with type tid
      string name;

      int tidIndex = listutils::findType(nl->Second(nl->Second(first)),
                                  nl->SymbolAtom(TupleIdentifier::BasicType()),
                                      name);
      if(tidIndex ==0){
      return listutils::typeError("attr list does not contain a tid attribute");
      }
      string name2;
      if(listutils::findType(nl->Second(nl->Second(first)),
                             nl->SymbolAtom(TupleIdentifier::BasicType()),
                             name2,
                             tidIndex+1)>0){
        return listutils::typeError("multiple tid attributes found");
      }

      // Get key attribute

      if(nl->AtomType(second)!=SymbolType){
        return listutils::typeError("second argument is not a valid attr name");
      }

      string keyAttrName = nl->SymbolValue(second);

      int keyIndex;
      ListExpr keyType;
      keyIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
                                           keyAttrName,keyType);
      if(keyIndex==0){
        return listutils::typeError("key attr name " + name +
                                       "not found in attr list");
      }

      if(!listutils::isSymbol(keyType,CcString::BasicType()) &&
         !listutils::isSymbol(keyType,CcInt::BasicType()) &&
         !listutils::isSymbol(keyType,CcBool::BasicType()) &&
         !listutils::isSymbol(keyType,CcReal::BasicType()) &&
         !listutils::isKind(keyType,Kind::INDEXABLE())){
        return listutils::typeError("selected attribut not in kind INDEXABLE");
      }

      // Get optional arguments fill and node size

      ListExpr appendArgs = nl->FourElemList(nl->RealAtom(STD_FILL),
                                   nl->IntAtom(STD_NODESIZE),
                                   nl->IntAtom(keyIndex),nl->IntAtom(tidIndex));

      if (nl->ListLength(args) >= 3) {
         ListExpr fillArg = nl->Third(args);
         if (!listutils::isSymbol(fillArg,CcReal::BasicType())) {
          return listutils::typeError
                         ("Argument three expected to be real between 0 and 1");
        }
        appendArgs = nl->Rest(appendArgs);
      }
      if (nl->ListLength(args) >= 4) {
         ListExpr fillArg = nl->Fourth(args);
         if (!listutils::isSymbol(fillArg,CcInt::BasicType())) {
          return listutils::typeError("Argument four expected to be int");
        }
        appendArgs = nl->Rest(appendArgs);
      }
      if ((nl->ListLength(args) >= 5)) {
        return listutils::typeError("too many arguments");
      }

      ListExpr res =  nl->ThreeElemList(
          nl->SymbolAtom(Symbol::APPEND()),
          appendArgs,
          nl->FourElemList(
                                 nl->SymbolAtom(BTree2::BasicType()),
                                 keyType,
                                 nl->SymbolAtom(TupleIdentifier::BasicType()),
                                 nl->SymbolAtom("multiple")));

      return res;
    }
  }
}

/*
2.2 Select function of operator

*/
int createbtree2::Select( ListExpr args )
{
  if (nl->IsEqual(nl->First(nl->First(args)), Relation::BasicType())) {
    return 0;
  }
  if (nl->IsEqual(nl->First(nl->First(args)), Symbol::STREAM())) {
    if (nl->ListLength(args) == 6) {  // Args! Übel!:
          // Distinguish by number of arguments (but not the appended ones)
      return 2;
    } else {
      return 1;
    }
  }
  return -1;
}

/*
2.3 Value mapping function of operator

The AppendTYPE methods here are used out of performance reasons.
Normally, one should not use the BTree2Impl class directly.

*/
void createbtree2::AppendDouble(BTree2* inBtree,
                              int attrIndex,
                              GenericRelationIterator* iter) {
  BTree2Impl<double,TupleId>* btree =
                           dynamic_cast<BTree2Impl<double,TupleId>*>(inBtree);
  assert(btree != 0);
  Tuple* tuple;
  while ((tuple = iter->GetNextTuple()) != 0) {
    CcReal* attr = (CcReal*) tuple->GetAttribute(attrIndex);
    double key;
    if (!attr->IsDefined()) {
      key = numeric_limits<double>::min();
    } else {
      key = (double) attr->GetRealval();
    }
    btree->Append(key, iter->GetTupleId());
    tuple->DeleteIfAllowed();
  }
  delete iter;
}

void createbtree2::AppendInt(BTree2* inBtree,
                              int attrIndex,
                              GenericRelationIterator* iter) {
  BTree2Impl<int,TupleId>* btree =
                            dynamic_cast<BTree2Impl<int,TupleId>*>(inBtree);
  assert(btree != 0);
  Tuple* tuple;
  while ((tuple = iter->GetNextTuple()) != 0) {
    CcInt* attr = (CcInt*) tuple->GetAttribute(attrIndex);
    int key;
    if (!attr->IsDefined()) {
      key = numeric_limits<int>::min();
    } else {
      key = (int) attr->GetIntval();
    }
    btree->Append(key, iter->GetTupleId());
    tuple->DeleteIfAllowed();
  }
  delete iter;
}

void createbtree2::AppendString(BTree2* inBtree,
                              int attrIndex,
                              GenericRelationIterator* iter) {
  BTree2Impl<string,TupleId>* btree =
                       dynamic_cast<BTree2Impl<string,TupleId>*>(inBtree);
  assert(btree != 0);
  Tuple* tuple;
  while ((tuple = iter->GetNextTuple()) != 0) {
    CcString* attr = (CcString*) tuple->GetAttribute(attrIndex);
    string key = (char*) (attr->GetStringval());
    btree->Append(key, iter->GetTupleId());
    tuple->DeleteIfAllowed();
  }
  delete iter;
}

void createbtree2::AppendBool(BTree2* inBtree,
                              int attrIndex,
                              GenericRelationIterator* iter) {
  BTree2Impl<bool,TupleId>* btree =
                       dynamic_cast<BTree2Impl<bool,TupleId>*>(inBtree);
  assert(btree != 0);
  Tuple* tuple;
  while ((tuple = iter->GetNextTuple()) != 0) {
    CcBool* attr = (CcBool*) tuple->GetAttribute(attrIndex);
    bool key = attr->GetBoolval();
    btree->Append(key, iter->GetTupleId());
    tuple->DeleteIfAllowed();
  }
  delete iter;
}

void createbtree2::AppendIndexableAttribute(BTree2* inBtree,
                              int attrIndex,
                              GenericRelationIterator* iter) {
  BTree2Impl<IndexableAttribute*,TupleId>* btree =
               dynamic_cast<BTree2Impl<IndexableAttribute*,TupleId>*>(inBtree);
  assert(btree != 0);
  Tuple* tuple;
  while ((tuple = iter->GetNextTuple()) != 0) {
    IndexableAttribute* attr =
              (IndexableAttribute*) tuple->GetAttribute(attrIndex);
    btree->Append(attr, iter->GetTupleId());
    tuple->DeleteIfAllowed();
  }
  delete iter;
}

int
createbtree2::ValueMapping_Rel(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);

  Relation* relation = (Relation*)args[0].addr;
  int nodesize = ((CcInt*)args[3].addr)->GetIntval();
  double fill = ((CcReal*)args[2].addr)->GetRealval();
  int attrIndex = ((CcInt*)args[4].addr)->GetIntval() - 1;
  assert(relation != 0);

  GenericRelationIterator *iter = relation->MakeScan();

  BTree2* btree = (BTree2*)result.addr;
  assert(btree != 0);
  if (!(fill >= 0.0) || !(fill <= 1.0)) {
    cout << "Invalid fill factor, correcting to " << STD_FILL << endl;
    fill = STD_FILL;
  }

  int minNodeSize = btree->GetMinNodeSize();
  int maxNodeSize = WinUnix::getPageSize() - PAGE_CONST;
  if (nodesize < minNodeSize) {
    nodesize = minNodeSize;
    cout << "Nodesize too small, correcting to " << minNodeSize << endl;
  }
  if (nodesize > maxNodeSize) {
    nodesize = maxNodeSize;
    cout << "Nodesize too large, correcting to " << minNodeSize << endl;
  }

  if (!btree->CreateNewBTree(fill,nodesize,BTree2::multiple,false)) {
    return 0;
  }

  if (btree->GetKeyType() == CcReal::BasicType()) {
    AppendDouble(btree, attrIndex, iter);
  } else if (btree->GetKeyType() == CcInt::BasicType()) {
    AppendInt(btree, attrIndex, iter);
  } else if (btree->GetKeyType() == CcString::BasicType()) {
    AppendString(btree, attrIndex, iter);
  } else if (btree->GetKeyType() == CcBool::BasicType()) {
    AppendBool(btree, attrIndex, iter);
  } else {
    AppendIndexableAttribute(btree, attrIndex, iter);
  }

  btree->printNodeInfos();

  return 0;
}

 int
 createbtree2::ValueMapping_Stream_Tid(Word* args, Word& result, int message,
                                Word& local, Supplier s)
 {
   result = qp->ResultStorage(s);
   BTree2* btree = (BTree2*)result.addr;
   int nodesize = ((CcInt*)args[3].addr)->GetIntval();
   double fill = ((CcReal*)args[2].addr)->GetRealval();
   int attrIndex = ((CcInt*)args[4].addr)->GetIntval() - 1;
   int tidIndex = ((CcInt*)args[5].addr)->GetIntval() - 1;
   Word wTuple;

   assert(btree != 0);

   if (!(fill >= 0.0) || !(fill <= 1.0)) {
     fill = STD_FILL;
     cout << "Invalid fill factor, correcting to " << STD_FILL << endl;
   }

   int minNodeSize = btree->GetMinNodeSize();
   int maxNodeSize = WinUnix::getPageSize() - PAGE_CONST;
   if (nodesize < minNodeSize) {
     nodesize = minNodeSize;
     cout << "Nodesize too small, correcting to " << minNodeSize << endl;
   }
   if (nodesize > maxNodeSize) {
     nodesize = maxNodeSize;
     cout << "Nodesize too large, correcting to " << nodesize << endl;
   }

   if (!btree->CreateNewBTree(fill,nodesize,BTree2::multiple,false)) {
     return CANCEL;
   }

   qp->Open(args[0].addr);
   qp->Request(args[0].addr, wTuple);

   if (btree->GetKeyType() == CcInt::BasicType()) {
     // Could be faster, but maybe not... needs to be checked again.
     BTree2Impl<int,TupleId>* btreeX =
                              dynamic_cast<BTree2Impl<int,TupleId>*>(btree);
     while (qp->Received(args[0].addr))
     {
       Tuple* tuple = (Tuple*) wTuple.addr;
       if ((Attribute *)tuple->GetAttribute(attrIndex)->IsDefined() &&
           (Attribute *)tuple->GetAttribute(tidIndex)->IsDefined() )
       {
         CcInt* attr = (CcInt*) tuple->GetAttribute(attrIndex);
         TupleIdentifier* value = (TupleIdentifier*)
                                     tuple->GetAttribute(tidIndex);
         int key = (int) attr->GetIntval();
         btreeX->Append(key, value->GetTid());
       }
       tuple->DeleteIfAllowed();

       qp->Request(args[0].addr, wTuple);
     }
    //Append_Stream_Int(args[0].addr, btree, attrIndex,tidIndex);
   } else {
     while (qp->Received(args[0].addr))
     {
       Tuple* tuple = (Tuple*)wTuple.addr;
       if( (Attribute *)tuple->GetAttribute(attrIndex)->IsDefined() &&
           (Attribute *)tuple->GetAttribute(tidIndex)->IsDefined() )
       {
         Attribute* a = tuple->GetAttribute(attrIndex);
         Attribute* b = tuple->GetAttribute(tidIndex);
         btree->AppendGeneric(a,b);
       }
       tuple->DeleteIfAllowed();

       qp->Request(args[0].addr, wTuple);
     }
   }
   qp->Close(args[0].addr);

   return 0;
 }

 int
 createbtree2::ValueMapping_Stream_Attrib(Word* args, Word& result, int message,
                                Word& local, Supplier s)
 {
   result = qp->ResultStorage(s);
   BTree2* btree = (BTree2*)result.addr;
   int nodesize = ((CcInt*)args[2].addr)->GetIntval();
   double fill = ((CcReal*)args[1].addr)->GetRealval();
   int keyAttrIndex = ((CcInt*)args[6].addr)->GetIntval() - 1;
   int valueAttrIndex = ((CcInt*)args[7].addr)->GetIntval() - 1;
   BTree2::multiplicityType u =
           (BTree2::multiplicityType) ((CcInt*) args[8].addr)->GetIntval();
   Word wTuple;

   assert(btree != 0);

   if (!(fill >= 0.0) || !(fill <= 1.0)) {
     fill = STD_FILL;
     cout << "Invalid fill factor, correcting to " << STD_FILL << endl;
   }

   int minNodeSize = btree->GetMinNodeSize();
   int maxNodeSize = WinUnix::getPageSize() - PAGE_CONST;
   if (nodesize < minNodeSize) {
     nodesize = minNodeSize;
     cout << "Nodesize too small, correcting to " << minNodeSize << endl;
   }
   if (nodesize > maxNodeSize) {
     nodesize = maxNodeSize;
     cout << "Nodesize too large, correcting to " << nodesize << endl;
   }

   if (!btree->CreateNewBTree(fill,nodesize,u,false)) {
     return CANCEL;
   }

   qp->Open(args[0].addr);
   qp->Request(args[0].addr, wTuple);
   while (qp->Received(args[0].addr)) {
     Tuple* tuple = (Tuple*)wTuple.addr;
     if ((Attribute *)tuple->GetAttribute(keyAttrIndex)->IsDefined()) {
       Attribute* keyattr = tuple->GetAttribute(keyAttrIndex);
       if (valueAttrIndex<0) {
         btree->AppendGeneric(keyattr, 0);
       } else {
         if ((Attribute *)tuple->GetAttribute(valueAttrIndex)->IsDefined()) {
           Attribute* valueattr = tuple->GetAttribute(valueAttrIndex);
           btree->AppendGeneric(keyattr, valueattr);
         }
       }
     }
     tuple->DeleteIfAllowed();

     qp->Request(args[0].addr, wTuple);
   }
   qp->Close(args[0].addr);

   return 0;
 }

string createbtree2::Specification1() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "stream (tuple (T)) x real x int x ak x ad x u"
               " -> (btree2 Tk Td u)\n"
               "rel (tuple ((x1 t1)...(xn tn))) x ak [x f x n]"
               " -> (btree2 Tk tid multiple)\n"
               "stream (tuple ((x1 t1)...(xn tn) (id tid))) x ak [x f x n]"
               " -> (btree2 Tk tid multiple)";
  string spec = "_ createbtree [ _ ]";
  string meaning = "Creates a btree2. The key type ak must "
    "be either string, int, real, bool or to implement the "
    "kind INDEXABLE. u must be 'multiple', 'stable_multiple',"
    "'uniqueKeyMultiData' or 'uniqueKey'.";

  string example = "let mybtree = ten createbtree [nr];\n"
    "let mybtree = ten feed extend[id: tupleid(.)] "
    "sortby[no asc] createbtree[no]";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

string createbtree2::Specification2() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "stream (tuple (T)) x real x int x ak x ad x u"
               " -> (btree2 Tk Td u)\n"
               "rel (tuple ((x1 t1)...(xn tn))) x ak [x f x n]"
               " -> (btree2 Tk tid multiple)\n"
               "stream (tuple ((x1 t1)...(xn tn) (id tid))) x ak [x f x n]"
               " -> (btree2 Tk tid multiple)";
  string spec = "_ createbtree2 [ _ ]";
  string meaning = "Creates a btree2. The key type ak must "
    "be either string, int, real, bool or to implement the "
    "kind INDEXABLE. u must be 'multiple', 'stable_multiple',"
    "'uniqueKeyMultiData' or 'uniqueKey'.";

  string example = "let mybtree = ten createbtree2 [nr];\n"
    "let mybtree = ten feed extend[id: tupleid(.)] "
    "sortby[no asc] createbtree2[no]";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

int createbtree2::numberOfValueMappings = 3;
ValueMapping createbtree2::valueMappings[] = {
                  createbtree2::ValueMapping_Rel,
                  createbtree2::ValueMapping_Stream_Tid,
                  createbtree2::ValueMapping_Stream_Attrib
                };

Operator createbtree2::def1 (
          "createbtree",                     // name
          createbtree2::Specification1(),      // specification
          createbtree2::numberOfValueMappings,// number of overloaded functions
          createbtree2::valueMappings,        // value mapping
          createbtree2::Select,               // trivial selection function
          createbtree2::TypeMapping           // type mapping
);

Operator createbtree2::def2 (
          "createbtree2",                     // name
          createbtree2::Specification2(),      // specification
          createbtree2::numberOfValueMappings,// number of overloaded functions
          createbtree2::valueMappings,        // value mapping
          createbtree2::Select,               // trivial selection function
          createbtree2::TypeMapping           // type mapping
);

} // end namespace operators
} // end namespace btree2algebra

