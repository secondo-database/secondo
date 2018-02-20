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

[1] Implementation of the insertbtree and insertbtree2 Operators

[TOC]

0 Overview

1 Defines and Includes

*/

#include "op_insertbtree.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "Symbols.h"

#include "BTree2.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operators ~insertbtree~ and ~insertbtree2~

Inserts the entries from the stream into the btree2.

Signatures are

----
    insertbtree: stream(tuple(T)) x (btree2 Tk Td u) x ak x ad -->
                    stream(tuple(T))

    insertbtree2: stream(tuple(X@[TID tid])) x (btree2 Tk tid multiple)
                     x ak --> stream(tuple(X@[TID tid]))
----

2.1 TypeMapping

*/
ListExpr insertbtree::TypeMappingAll(ListExpr args, bool wrapper)
{
  if (wrapper){
   if(nl->ListLength(args) != 3){
     return listutils::typeError("Operator insertbtree expects 3 arguments.");
    }
  }
  else{
    if(nl->ListLength(args) != 4){
      return listutils::typeError("Operator insertbtree2 expects"
                                  " 4 arguments.");
    }
  }

  /* Split argument in four parts */
  ListExpr streamDescription = nl->First(args);
  ListExpr btreeDescription = nl->Second(args);
  ListExpr nameOfKeyAttribute = nl->Third(args);
  ListExpr attrType;


  if(!listutils::isTupleStream(streamDescription)){
   return listutils::typeError("first arguments must be a tuple stream");
  }

  ListExpr attrList = nl->Second(nl->Second(streamDescription)); // get attrlist

  if (wrapper){
    if(nl->ListLength(attrList)<=1){
       return listutils::typeError("stream must contain at least 2 attributes");
    }
    ListExpr rest = nl->Second(nl->Second(streamDescription));
    ListExpr next = nl->First(rest);
    while (!(nl->IsEmpty(rest)))
    {
      next = nl->First(rest);
      rest = nl->Rest(rest);
    }

    if(!listutils::isSymbol(nl->Second(next),TupleIdentifier::BasicType())){
      return listutils::typeError("last attribute must be of type tid");
    }
  }

  if(!listutils::isBTree2Description(btreeDescription)){
    return listutils::typeError("second argument is not a valid btree2");
  }

  ListExpr btreeValue = nl->Third(btreeDescription);

  if (wrapper){
    if(!listutils::isSymbol(btreeValue, TupleIdentifier::BasicType())){
       return listutils::typeError("Value type of btree has to b tid");
    }
    if(!listutils::isSymbol(nl->Fourth(btreeDescription), "multiple")){
       return listutils::typeError("Keys have to be multiple");
    }
  }

  if(!listutils::isSymbol(nameOfKeyAttribute)){
    return listutils::typeError("invalid name for key attribute");
  }



/*
Check key

*/

  string name;
  nl->WriteToString(name, nameOfKeyAttribute);
  int keyIndex = listutils::findAttribute(attrList, name, attrType);
  if(keyIndex <= 0){
      return listutils::typeError("Tuples do not contain an attribute named "
                                   + name + "."); 
   }
  ListExpr btreeKey = nl->Second(btreeDescription);
  if(!nl->Equal(attrType, btreeKey)){
     return listutils::typeError( "Key in tuple is " 
                                  "different from btree2 key.");
   }

/*
Check value-types

*/
  int valueIndex = 0;
  if (!wrapper){
    ListExpr nameOfDataAttribute = nl->Fourth(args);
    if(!listutils::isSymbol(nameOfDataAttribute)){
      return listutils::typeError("invalid name for data attribute");
    }
    nl->WriteToString(name, nameOfDataAttribute);

    if (name == "none"){
      if(!nl->Equal(nameOfDataAttribute, btreeValue)){
         return listutils::typeError("Argument value type is different"
                                     " from btree value type.");
      }
    }
    else {
      valueIndex = listutils:: findAttribute(attrList, name, attrType);
      if(valueIndex <= 0){
          return listutils::typeError("Tuples do not contain an "
                                      "attribute named " +
                                       name + ".");
      }
      if(!nl->Equal(attrType, btreeValue)){
         return listutils::typeError("Value type in tuple is "
                               "different from btree2 value type.");
       }
    }
  }
  if (wrapper){
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                      nl->TwoElemList(nl->IntAtom(keyIndex), nl->
                      IntAtom(nl->ListLength(attrList))), streamDescription);
  }
  else{
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                          nl->TwoElemList(nl->IntAtom(keyIndex),
                                nl->IntAtom(valueIndex)), streamDescription);
  }
}

/*
2.1.1 Type map for insertbtree

*/

ListExpr insertbtree::TypeMapping1(ListExpr args){
  return insertbtree::TypeMappingAll(args);
}

/*
2.1.2 Type map for insertbtree2

*/
ListExpr insertbtree::TypeMapping2(ListExpr args){
  return insertbtree::TypeMappingAll(args, false);
}

/*
2.2 Value mapping for insertbtree

*/
struct vmInfo{
   BTree2* btree;
   int keyIndex;
   int valueIndex;

   vmInfo(BTree2* b, int k, int v) : btree(b), keyIndex(k), valueIndex(v) {}
};

int
insertbtree::ValueMapping1(Word* args, Word& result, int message,
                                Word& local, Supplier s)
{
  vmInfo* info;
  Tuple* tuple;
  Word elem;
  Attribute* key;
  Attribute* value;

  switch (message)
  {
    case OPEN :
    {
      BTree2* btree = (BTree2*)args[1].addr;
      int k = ((CcInt*)args[3].addr)->GetIntval();
      int v = ((CcInt*)args[4].addr)->GetIntval();
      info = new vmInfo(btree, k - 1, v - 1);
      local.addr = info;
      qp->Open(args[0].addr);
      return 0;
    }

    case REQUEST :
    {
      qp->Request(args[0].addr, elem);
      if (qp->Received(args[0].addr))
      {
        info = (vmInfo*)local.addr;
        tuple = (Tuple*)elem.addr;
        key = tuple->GetAttribute(info->keyIndex);
        value = tuple->GetAttribute(info->valueIndex);
        if (((TupleIdentifier*)value)->GetTid() == 0){
          tuple->DeleteIfAllowed();
          return CANCEL;
        }
        info->btree->AppendGeneric(key, value);
        result.setAddr(tuple);
        return YIELD;
      }
      return CANCEL;
    }

    case CLOSE :
    {
      if (local.addr){
        info = (vmInfo*)local.addr;
        delete info;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

/*
2.3 Valuemapping for insertbtree2

*/
int
insertbtree::ValueMapping2(Word* args, Word& result, int message,
                                Word& local, Supplier s)
{
  vmInfo* info;
  Tuple* tuple;
  Word elem;
  Attribute* key;
  Attribute* value;

  switch (message)
  {
    case OPEN :
    {
      BTree2* btree = (BTree2*)args[1].addr;
      int k = ((CcInt*)args[4].addr)->GetIntval();
      int v = ((CcInt*)args[5].addr)->GetIntval();
      info = new vmInfo(btree, k - 1, v - 1);
      local.addr = info;
      qp->Open(args[0].addr);
      return 0;
    }

    case REQUEST :
    {
      qp->Request(args[0].addr, elem);
      if (qp->Received(args[0].addr))
      {
        info = (vmInfo*)local.addr;
        tuple = (Tuple*)elem.addr;
        key = tuple->GetAttribute(info->keyIndex);
        if (info->valueIndex == -1){

          info->btree->AppendGeneric(key, 0);
        }
        else{
          value = tuple->GetAttribute(info->valueIndex);
          info->btree->AppendGeneric(key, value);
        }
        result.setAddr(tuple);
        return YIELD;
      }
      return CANCEL;
    }

    case CLOSE :
    {
      if (local.addr){
        info = (vmInfo*)local.addr;
        delete info;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

/*
2.4 Operator specifications

*/
struct getInsertBTree2Info : OperatorInfo {

  getInsertBTree2Info() : OperatorInfo()
  {
    name =      "insertbtree2";
    signature = "stream(tuple(T)) x (btree2 Tk Td u) x ak x ad ->"
                " stream(tuple(T))";
    syntax =    "_ _ insertbtree2 [ _, _ ]";
    meaning =   "Inserts the pairs of values/keys specified by the "
                "arguments into the BTree2";
    example =   "query Staedte feed staedte_btree2 insertbtree2 "
                "[SName, Bev] count;";
  }
};

struct getInsertBTreeInfo : OperatorInfo {

  getInsertBTreeInfo() : OperatorInfo()
  {
    name =      "insertbtree";
    signature = "stream(tuple(X@[TID tid])) x (btree2 Tk tid multiple) x ak"
                " -> stream(tuple(X@[TID tid]))";
    syntax =    "_ _ insertbtree [ _ ]";
    meaning =    "Inserts the pairs of values/keys specified by the "
                "arguments into the BTree2";
    example =   "query Staedte feed addid staedte_btree2_tid "
                "insertbtree [SName] count";
  }
};


Operator insertbtree::insertbtree1 (getInsertBTreeInfo(), insertbtree::
                                    ValueMapping1, insertbtree::TypeMapping1);

Operator insertbtree::insertbtree2 (getInsertBTree2Info(), insertbtree::
                                    ValueMapping2, insertbtree::TypeMapping2);

} // end namespace operators
} // end namespace btree2algebra

