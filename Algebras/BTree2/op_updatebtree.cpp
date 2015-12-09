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


[1] Implementation of the updatebtree and updatebtree2 Operator

[TOC]

0 Overview

1 Defines and Includes

*/

#include "op_updatebtree.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "Symbols.h"

#include "BTree2.h"

using namespace std;


extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operators ~updatebtree~ and ~updatebtree2~

Updates the entries given in the tuple stream. updatebtree2
inserts a new entry, if an entry with the argument key does not
exist. If the valuetype is none, updatebtree2 simply tries to
insert an entry with the argument key.

Signature is

----
    updatebtree: stream(tuple(X@[(a1\_old x1)...(ak\_old Tk)...(ak\_old xk )
                 (TID tid)])) x btree2 x ak -->stream(tuple(X@[(a1\_old x1)
                 ...(ak\_old Tk)...(ak\_old xk ) (TID tid)]))

    updatebtree2: stream(tuple(T)) x btree2 x ak x ad ->
                    stream(tuple(T))
----

2.1 TypeMappings

*/

ListExpr updatebtree::TypeMapping1(ListExpr args){
  if(nl->ListLength(args) != 3){
     return listutils::typeError("Operator updatebtree expects 3 arguments.");
   }

/*
Split argument in three parts

*/
  int keyIndex;
  ListExpr streamDescription = nl->First(args);
  ListExpr btreeDescription = nl->Second(args);
  ListExpr nameOfKeyAttribute = nl->Third(args);

  if(!listutils::isTupleStream(streamDescription)){
   return listutils::typeError("first arguments must be a tuple stream");
  }
  ListExpr attrList = nl->Second(nl->Second(streamDescription));
  if(nl->ListLength(attrList)<3){
    return listutils::typeError("stream must contains at least 3 attributes");
  }
/*
 Proceed to last attribute of stream-tuples

*/
  ListExpr rest = attrList;
  ListExpr next = nl->First(rest);
  while (!(nl->IsEmpty(rest)))
  {
    next = nl->First(rest);
    rest = nl->Rest(rest);
  }

  if(!listutils::isSymbol(nl->Second(next),TupleIdentifier::BasicType())){
   return listutils::typeError("last attribut must be of type tid");
  }
  int tidIndex = nl->ListLength(attrList);

  if(!listutils::isBTree2Description(btreeDescription)){
    return listutils::typeError("second argument is not a valid btree2");
  }

  ListExpr btreeKey = nl->Second(btreeDescription);
  ListExpr btreeValue = nl->Third(btreeDescription);

  if(!listutils::isSymbol(btreeValue, TupleIdentifier::BasicType())){
     return listutils::typeError("Value type of btree has to be tid");
  }
  if(!listutils::isSymbol(nl->Fourth(btreeDescription), "multiple")){
     return listutils::typeError("Keys have to be multiple");
  }


  if(!listutils::isSymbol(nameOfKeyAttribute)){
    return listutils::typeError("invalid attribute name for key");
  }
/*
check that attributelist has old and new attributenames

*/
  if ((nl->ListLength(attrList) % 2) != 1){
    return listutils::typeError("tuple stream must contain one attribute "
                             "of kind tid and the same amount of old and "
                             "new attributes.");
  }
  int split = (nl->ListLength(attrList) - 1) / 2;
  ListExpr oldAttrList = attrList;
  for (int i = 0; i < split; i++){
    oldAttrList = nl->Rest(oldAttrList);
  }
  rest = attrList;
  for (int i = 0; i < split; i++){
    string oldName;
    nl->WriteToString(oldName,
                        nl->First(nl->First(rest)));
    oldName += "_old";
    ListExpr oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),
                                     nl->Second(nl->First(rest)));
    if(!nl->Equal(oldAttribute, nl->First(oldAttrList))){
         return listutils::typeError("Second part of the "
                   "tupledescription of the stream without the last "
                   "attribute has to be the same as the tuple"
                   "description of the btree except that the "
                   "attributenames carry an additional '_old.'");
    }
    rest = nl->Rest(rest);
    oldAttrList = nl->Rest(oldAttrList);
    i++;
  }
/*
Test if attributename of the third argument exists as a name in the
attributelist of the streamtuples

*/
  string attrname = nl->SymbolValue(nameOfKeyAttribute);
  ListExpr attrtype;
  keyIndex = listutils::findAttribute(attrList,attrname,attrtype);
  if(keyIndex==0){
    return listutils::typeError("attribute name of key not found");
  }

  if(!nl->Equal(attrtype, btreeKey)){
    return listutils::typeError("key attribute type "
                                "different from indexed type");
  }
  ListExpr outList = nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->TwoElemList(nl->IntAtom(keyIndex),
                                nl->IntAtom(tidIndex)),streamDescription);
  return outList;
}


ListExpr updatebtree::TypeMapping2(ListExpr args){

  if(nl->ListLength(args) != 4){
    return listutils::typeError("Operator updatebtree2 expects 4 arguments.");
  }
/*
Split argument in four parts

*/
  ListExpr streamDescription = nl->First(args);
  ListExpr btreeDescription = nl->Second(args);
  ListExpr nameOfKeyAttribute = nl->Third(args);
  ListExpr nameOfDataAttribute = nl->Fourth(args);
  ListExpr attrType;


  if(!listutils::isTupleStream(streamDescription)){
   return listutils::typeError("first arguments must be a tuple stream");
  }

  ListExpr attrList = nl->Second(nl->Second(streamDescription));

  if(!listutils::isBTree2Description(btreeDescription)){
    return listutils::typeError("second argument is not a valid btree2");
  }

  ListExpr btreeValue = nl->Third(btreeDescription);

  if(!listutils::isSymbol(nameOfKeyAttribute)){
    return listutils::typeError("invalid name for key attribute");
  }

  if(!listutils::isSymbol(nameOfDataAttribute)){
    return listutils::typeError("invalid name for data attribute");
  }
/*
Check key

*/
  string name;
  nl->WriteToString(name, nameOfKeyAttribute);
  int keyIndex = listutils::findAttribute(attrList, name, attrType);
  if(keyIndex <=0 ){
    return listutils::typeError("Tuples do not contain an attribute named " +
                                 name + ".");
  }
  ListExpr btreeKey = nl->Second(btreeDescription);
  if(!nl->Equal(attrType, btreeKey)){
     return listutils::typeError("Key in tuple is different from btree2 key.");
  }
/*
Check value-types

*/
  int valueIndex = 0;
  nl->WriteToString(name, nameOfDataAttribute);

  if (name == "none"){
    if(!nl->Equal(nameOfDataAttribute, btreeValue)){
       return listutils::typeError("Argument value type is "
                                   "different from btree value type.");
    }
  }
  else {
    valueIndex = listutils:: findAttribute(attrList, name, attrType);
    if(valueIndex <=  0){
         return listutils::typeError("Tuples do not contain"
                                     " an attribute named " +
                                     name + ".");
    }
    if(!nl->Equal(attrType, btreeValue)){
       return listutils::typeError("Value type in tuple is "
                         "different from btree2 value type.");
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                          nl->TwoElemList(nl->IntAtom(keyIndex),
                                nl->IntAtom(valueIndex)), streamDescription);
}

/*
2.2 Value mappings

*/
struct vmInfo{
   BTree2* btree;
   int keyIndex;
   int valueIndex;

   vmInfo(BTree2* b, int k, int v) : btree(b), keyIndex(k), valueIndex(v) {}
};

int
updatebtree::ValueMapping1(Word* args, Word& result, int message,
                                Word& local, Supplier s)
{
  vmInfo* info;
  Tuple* tuple;
  Word elem;
  Attribute* key, *keyOld;
  Attribute* value;
  bool res = false;

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
        keyOld = tuple->GetAttribute((tuple->GetNoAttributes() - 1) / 2 +
                                                   info->keyIndex);
        value = tuple->GetAttribute(info->valueIndex);
        if ( (key->Compare(keyOld) != 0) )
        {
          if (info->btree->DeleteGeneric(keyOld,value))
            res = info->btree->AppendGeneric(key,value);
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

int
updatebtree::ValueMapping2(Word* args, Word& result, int message,
                                Word& local, Supplier s)
{
  vmInfo* info;
  Tuple* tuple;
  Word elem;
  Attribute* key;
  Attribute* value;
  bool res = false;

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
          res = info->btree->AppendGeneric(key, 0);
        }
        else{
          value = tuple->GetAttribute(info->valueIndex);
          res = info->btree->UpdateGeneric(key, value);
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
2.3 Operator specifications

*/

struct getUpdateBTree2Info : OperatorInfo {

  getUpdateBTree2Info() : OperatorInfo()
  {
    name =      "updatebtree2";
    signature = "stream(tuple(T)) x (btree2 Tk Td u) x ak x ad ->"
                " stream(tuple(T))";
    syntax =    "_ _ updatebtree2 [ _, _ ]";
    meaning =   "Updates the first found entry with the argument key, "
                "if existant,"
                " or inserts the pairs of values/keys specified by the "
                "arguments into the BTree2";
    example =   "query Staedte feed extend [Bev_new: .Bev + 1] "
                "staedte_btree2 updatebtree2 [SName, Bev_new] consume";
  }
};

struct getUpdateBTreeInfo : OperatorInfo {

  getUpdateBTreeInfo() : OperatorInfo()
  {
    name =      "updatebtree";
    signature = "stream(tuple(X@[(a1 x1)...(ak Tk)...(an xn)(TID tid)]))"
                " x (btree2 Tk tid multiple) x ak ->"
                " stream(tuple(X@[(a1 x1)...(ak Tk)...(an xn)(TID tid)]))";
    syntax =    "_ _ updatebtree [ _ ]";
    meaning =    "Updates the key of the old entry";
    example =   " query Staedte feed Staedte updatedirect[Bev: .Bev + 1] "
                "staedte_btree2_tid updatebtree [SName] count";
  }
};


Operator updatebtree::updatebtree1 (getUpdateBTreeInfo(), updatebtree::
                                    ValueMapping1, updatebtree::TypeMapping1);

Operator updatebtree::updatebtree2 (getUpdateBTree2Info(), updatebtree::
                                    ValueMapping2, updatebtree::TypeMapping2);

}
}
