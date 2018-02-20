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

[1] Implementation of the keyrange and keyrange2 Operators

[TOC]

0 Overview

1 Defines and Includes

*/



#include "ListUtils.h"
#include "QueryProcessor.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "Symbols.h"


#include "BTree2.h"
#include "BTree2Impl.h"
#include "op_keyrange.h"

#include <math.h>
#include <iostream>
#include <iomanip>


extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operators ~keyrange~ and ~keyrange2~

These operators estimate the ratio of entries with keys smaller than,
equal to and larger than the argument key. Estimation is done by
inspecting treeheight + 1 nodes and assuming an equal distribution
of entries amongst the nodes.

Signatures are

----
    keyrange: (btree2) x rel(tuple(T)) x Tk --> stream(tuple( (Less real)
                        Equal real) (Greater real) (NumOfKeys int)))

    keyrange2: (btree2) x Tk --> stream(tuple( (Less real)
                        Equal real) (Greater real) (NumOfKeys int)))
----

2.1 TypeMappings

*/
ListExpr keyrange::TypeMapping(ListExpr args)
{

  if(nl->ListLength(args)!=3){
    return listutils::typeError("wrong number of arguments");
  }

/*
Split argument in two parts

*/
  ListExpr btreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr keyDescription = nl->Third(args);


  if(!listutils::isSymbol(keyDescription)){
    return listutils::typeError("invalid key");
  }

  if(!listutils::isBTree2Description(btreeDescription)){
    return listutils::typeError("not a btree");
  }

  if(nl->SymbolValue(nl->Third(btreeDescription)) !=
         TupleIdentifier::BasicType()){
     return listutils::typeError("Data type has to be tid");
   }

  if(nl->SymbolValue(nl->Fourth(btreeDescription)) != "multiple"){
    return listutils::typeError("Keys have to be multiple.");
  }

  ListExpr btreeKeyType = nl->Second(btreeDescription);


  if(!nl->Equal(keyDescription, btreeKeyType)){
    return listutils::typeError("key and btree key are different");
  }

   if(!listutils::isRelDescription(relDescription)){
    return listutils::typeError("not a relation");
  }


  NList a1(NList("Less"), NList(CcReal::BasicType()));
  NList a2(NList("Equal"), NList(CcReal::BasicType()));
  NList a3(NList("Greater"), NList(CcReal::BasicType()));
  NList a4(NList("NumOfKeys"), NList(CcInt::BasicType()));

  NList result(NList(Symbol::STREAM()), NList(NList(Tuple::BasicType()),
                                              NList(a1,a2,a3,a4)));

  return result.listExpr();
}

ListExpr keyrange::TypeMapping2(ListExpr args)
{

  if(nl->ListLength(args)!=2){
    return listutils::typeError("wrong number of arguments");
  }

/*
Split argument in two parts

*/
  ListExpr btreeDescription = nl->First(args);

  ListExpr keyDescription = nl->Second(args);

  if(!listutils::isSymbol(keyDescription)){
    return listutils::typeError("invalid key");
  }

  if(!listutils::isBTree2Description(btreeDescription)){
    return listutils::typeError("not a btree");
  }

  ListExpr btreeKeyType = nl->Second(btreeDescription);

  if(!nl->Equal(keyDescription, btreeKeyType)){
    return listutils::typeError("key and btree key are different");
  }
  NList a1(NList("Less"), NList(CcReal::BasicType()));
  NList a2(NList("Equal"), NList(CcReal::BasicType()));
  NList a3(NList("Greater"), NList(CcReal::BasicType()));
  NList a4(NList("NumOfKeys"), NList(CcInt::BasicType()));

  NList result(NList(Symbol::STREAM()), NList(NList(Tuple::BasicType()),
                                              NList(a1,a2,a3,a4)));

  return result.listExpr();
}

/*
2.2 Select functions

*/
int keyrange::Select( ListExpr args )
{

  ListExpr keyDescription = nl->Third(args);
  string keyTypeString;
  nl->WriteToString(keyTypeString, keyDescription);

  if (keyTypeString == CcReal::BasicType())
    return 0;
  else if (keyTypeString == CcInt::BasicType())
     return 1;
  else if (keyTypeString == CcString::BasicType())
     return 2;
  else if (keyTypeString == CcBool::BasicType())
    return 3;
  else
    return 4;

  return -1;
}

int keyrange::Select2( ListExpr args )
{

  ListExpr valueDescription = nl->Third(nl->First(args));
  ListExpr keyDescription = nl->Second(args);
  string keyTypeString, valueTypeString;
  nl->WriteToString(valueTypeString, valueDescription);
  nl->WriteToString(keyTypeString, keyDescription);

  if (keyTypeString == CcReal::BasicType()) {
    if (valueTypeString == "none") {
    return 0;
    } else if (valueTypeString == CcString::BasicType()) {
    return 1;
    } else if (valueTypeString ==CcInt::BasicType()){
    return 2;
    } else if (valueTypeString =="double"){
    return 3;
    } else if (valueTypeString ==TupleIdentifier::BasicType()){
    return 4;
    } else {
    return 5;
    }
  } else if (keyTypeString == CcInt::BasicType()) {
    if (valueTypeString == "none") {
    return 6;
    } else if (valueTypeString == CcString::BasicType()) {
    return 7;
    } else if (valueTypeString ==CcInt::BasicType()){
    return 8;
    } else if (valueTypeString =="double"){
    return 9;
    } else if (valueTypeString ==TupleIdentifier::BasicType()){
    return 10;
    } else {
    return 11;
    }
  } else if (keyTypeString == CcString::BasicType()) {
    if (valueTypeString == "none") {
    return 12;
    } else if (valueTypeString == CcString::BasicType()) {
    return 13;
    } else if (valueTypeString ==CcInt::BasicType()){
    return 14;
    } else if (valueTypeString =="double"){
    return 15;
    } else if (valueTypeString ==TupleIdentifier::BasicType()){
    return 16;
    } else {
    return 17;
    }
  } else if (keyTypeString == CcBool::BasicType()) {
    if (valueTypeString == "none") {
    return 18;
    } else if (valueTypeString == CcString::BasicType()) {
    return 19;
    } else if (valueTypeString ==CcInt::BasicType()){
    return 20;
    } else if (valueTypeString =="double"){
    return 21;
    } else if (valueTypeString ==TupleIdentifier::BasicType()){
    return 22;
    } else {
    return 23;
    }
  }else {
    if (valueTypeString == "none") {
    return 24;
    } else if (valueTypeString == CcString::BasicType()) {
    return 25;
    } else if (valueTypeString ==CcInt::BasicType()){
    return 26;
    } else if (valueTypeString =="double"){
    return 27;
    } else if (valueTypeString ==TupleIdentifier::BasicType()){
    return 28;
    } else {
    return 29;
    }
  }

  return -1;
}



/*
2.3 Auxiliary functions for Value Mapping of keyrange.

These functions actually estimate the values for less, equal
and greater. The first function is used for btrees with unique
keys, whereas the second one is used for btree with multiple keys.

*/
template<typename keytype, typename valuetype>
void getKeyrangeValuesUnique (keytype key, double& less, double& equal,
         double& greater, BTree2Impl<keytype, valuetype>* btree)
{
  cout << "here in keyrange" << endl;
  int internalEntries = btree->GetInternalEntryCount();
  int internalNodes = btree->GetInternalNodeCount();
  double avgSons = 0;
  if (internalNodes > 0)
    avgSons = (double)internalEntries / internalNodes + 1.0;
  double e = 0.0, g = 0.0, l = 0.0;
  double avgLeafEntries = (double)btree->GetLeafEntryCount()
                             / btree->GetLeafNodeCount();
  BTreeNode<keytype, valuetype>* leafNode;
  InternalNodeClass<keytype>* internalNode;
  int treeheight = btree->GetTreeHeight();
  int index;
  vector <NodeId> path;
  btree->FindLeftmostLeafNodeId(key, path);
  for (unsigned j = 0; j < path.size(); j++){
    if (j == path.size() - 1){
      leafNode = btree->cache.fetchLeafNode(path[j]);
      for (int i = 0; i < leafNode->GetCount(); i++){
        if (leafNode->GetEntryPtr(i)->keyLessThan(key)){
          l += 1.0;
        }
        else if(leafNode->GetEntryPtr(i)->keyEquals(key)){
          e += 1.0;
        }
        else {
          g += 1.0;
        }
      }
      btree->cache.dispose(leafNode);
    }
    else{
      internalNode = btree->cache.fetchInternalNode(path[j]);
      index = internalNode->GetEntryIndexByValue(path[j+1]);
      int noGreater = internalNode->GetCount() - index;
      int noLess = index;
      g += noGreater * pow (avgSons, treeheight - (int)j - 1) * avgLeafEntries;
      l += noLess * pow (avgSons, treeheight - (int)j - 1) * avgLeafEntries;
      btree->cache.dispose(internalNode);
    }
  }
  if (e == 1){//adjust values to exact amount of keys
    double total = g + l;
    int entries = btree->GetLeafEntryCount();
    greater = g / total * (entries - 1) / entries;
    less = l / total * (entries - 1) / entries;
    equal = e / entries;
  }
  else{
    greater = g / (g + e + l);
    equal = e / (g + e + l);
    less = l / (g + e + l);
  }
}


template<typename keytype, typename valuetype>
void getKeyrangeValuesMultiple (keytype key, double& less, double& equal,
         double& greater, BTree2Impl<keytype, valuetype>* btree)
{

  int internalEntries = btree->GetInternalEntryCount();
  int internalNodes = btree->GetInternalNodeCount();
  double avgSons = 0;
  if (internalNodes > 0)
    avgSons = (double)internalEntries / internalNodes + 1.0;
  double e = 0.0, g = 0.0, l = 0.0;
  double avgLeafEntries = (double)btree->GetLeafEntryCount()
                             / btree->GetLeafNodeCount();
  BTreeNode<keytype, valuetype>* leafNode;
  InternalNodeClass<keytype>* internalNode;
  int treeheight = btree->GetTreeHeight();
  int index;
  vector <NodeId> path;
  btree->FindLeftmostLeafNodeId(key, path);
  for (unsigned j = 0; j < path.size(); j++){
    if (j == path.size() - 1){
      leafNode = btree->cache.fetchLeafNode(path[j]);
      for (int i = 0; i < leafNode->GetCount(); i++){
        if (leafNode->GetEntryPtr(i)->keyLessThan(key)){
          l += 1.0;
        }
        else if(leafNode->GetEntryPtr(i)->keyEquals(key)){
          e += 1.0;
        }
        else {
          g += 1.0;
        }
      }
      btree->cache.dispose(leafNode);
    }
    else{
      internalNode = btree->cache.fetchInternalNode(path[j]);
      index = internalNode->GetEntryIndexByValue(path[j+1]);
      int higherIndex = index + 1;
      while (higherIndex < internalNode->GetCount() &&
         internalNode->GetEntryPtr(higherIndex)->keyEquals(key)){
        higherIndex++;
      }
      int noGreater = internalNode->GetCount() + 1 - higherIndex;
      int noEqual = higherIndex - 1 - index;
      int noLess = index;

      g += noGreater * pow (avgSons, treeheight - (int)j - 1) * avgLeafEntries;
      e += noEqual * pow (avgSons, treeheight - (int)j - 1) * avgLeafEntries;
      l += noLess * pow (avgSons, treeheight - (int)j - 1) * avgLeafEntries;
      btree->cache.dispose(internalNode);
    }
  }
  greater = g / (g + e + l);
  equal = e / (g + e + l);
  less = l / (g + e + l);
}



template<typename keytype, typename valuetype>
void keyrange::getKeyrangeValues (keytype key, double& less, double& equal,
         double& greater, BTree2Impl<keytype, valuetype>* btree)
{
  if (btree->GetMultiplicity() == btree->uniqueKey)
    getKeyrangeValuesUnique (key, less, equal, greater, btree);
  else
    getKeyrangeValuesMultiple (key, less, equal, greater, btree);
}


/*
2.4 Valuemappings

There are several different value mapping functions differentiating between
different keytypes.

2.4.1 Valuemapping for keytype int

*/
template<typename valuetype, int argNo>
int keyrange::ValueMappingInt(Word* args, Word& result, int message,
                                  Word& local, Supplier s)
{
  switch (message)
  {
    case OPEN : {
      int* k = new int;
      *k = 1;
      local.addr = k;
      return 0;

    }
    case REQUEST : {

      int* k;
      k = (int*)local.addr;
      if(*k == 1)
      {
        double less, equal, greater;
        BTree2Impl<int, valuetype>* btree = (BTree2Impl<int,
                                       valuetype>*)args[0].addr;
        int keyValue = ((CcInt*)args[argNo].addr)->GetIntval();
        getKeyrangeValues(keyValue, less, equal, greater, btree);

        int n = btree->GetLeafEntryCount();

        CcReal* lt = new CcReal(less);
        CcReal* eq = new CcReal(equal);
        CcReal* gt = new CcReal(greater);
        CcInt* nk = new CcInt(n);

        Tuple* t = new Tuple( nl->Second(GetTupleResultType(s)) );
        t->PutAttribute(0, lt);
        t->PutAttribute(1, eq);
        t->PutAttribute(2, gt);
        t->PutAttribute(3, nk);

        result.addr = t;
        *k = 0;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE : {
      delete (int*) local.addr;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/*
2.4.2 Valuemapping for keytype double

*/
template<typename valuetype, int argNo>
int keyrange::ValueMappingReal(Word* args, Word& result, int message,
                                  Word& local, Supplier s)
{

  switch (message)
  {
    case OPEN : {
      int* k = new int;
      *k = 1;
      local.addr = k;
      return 0;
    }
    case REQUEST : {

      int* k;
      k = (int*)local.addr;
      if(*k == 1)
      {
        double less, equal, greater;
        BTree2Impl<double, valuetype>* btree = (BTree2Impl<double,
                                            valuetype>*)args[0].addr;
        double keyValue = ((CcReal*)args[argNo].addr)->GetRealval();
        getKeyrangeValues(keyValue, less, equal, greater, btree);

        int n = btree->GetLeafEntryCount();

        CcReal* lt = new CcReal(less);
        CcReal* eq = new CcReal(equal);
        CcReal* gt = new CcReal(greater);
        CcInt* nk = new CcInt(n);

        Tuple* t = new Tuple( nl->Second(GetTupleResultType(s)) );
        t->PutAttribute(0, lt);
        t->PutAttribute(1, eq);
        t->PutAttribute(2, gt);
        t->PutAttribute(3, nk);

        result.addr = t;
        *k = 0;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE : {
      delete (int*) local.addr;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/*
2.4.3 Valuemapping for keytype string

*/
template<typename valuetype, int argNo>
int keyrange::ValueMappingString(Word* args, Word& result, int message,
                                  Word& local, Supplier s)
{

  switch (message)
  {
    case OPEN : {
      int* k = new int;
      *k = 1;
      local.addr = k;
      return 0;

    }
    case REQUEST : {

      int* k;
      k = (int*)local.addr;
      if(*k == 1)
      {
        double less, equal, greater;
        BTree2Impl<string, valuetype>* btree = (BTree2Impl<string,
                                          valuetype>*)args[0].addr;

        string keyValue = ((CcString*)args[argNo].addr)->GetValue();
        getKeyrangeValues(keyValue, less, equal, greater, btree);

        int n = btree->GetLeafEntryCount();

        CcReal* lt = new CcReal(less);
        CcReal* eq = new CcReal(equal);
        CcReal* gt = new CcReal(greater);
        CcInt* nk = new CcInt(n);

        Tuple* t = new Tuple( nl->Second(GetTupleResultType(s)) );
        t->PutAttribute(0, lt);
        t->PutAttribute(1, eq);
        t->PutAttribute(2, gt);
        t->PutAttribute(3, nk);

        result.addr = t;
        *k = 0;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE : {
      delete (int*) local.addr;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/*
2.4.4 Valuemapping for keytype IndexableAttribute

*/
template<typename valuetype, int argNo>
int keyrange::ValueMappingAttr(Word* args, Word& result, int message,
                                  Word& local, Supplier s)
{

  switch (message)
  {
    case OPEN : {
      int* k = new int;
      *k = 1;
      local.addr = k;
      return 0;
    }
    case REQUEST : {

      int* k;
      k = (int*)local.addr;
      if(*k == 1)
      {
        double less, equal, greater;
        BTree2Impl<IndexableAttribute*, valuetype>* btree =
                (BTree2Impl<IndexableAttribute*, valuetype>*)args[0].addr;

        IndexableAttribute* keyValue = (IndexableAttribute*)args[argNo].addr;
        getKeyrangeValues(keyValue, less, equal, greater, btree);

        int n = btree->GetLeafEntryCount();

        CcReal* lt = new CcReal(less);
        CcReal* eq = new CcReal(equal);
        CcReal* gt = new CcReal(greater);
        CcInt* nk = new CcInt(n);

        Tuple* t = new Tuple( nl->Second(GetTupleResultType(s)) );
        t->PutAttribute(0, lt);
        t->PutAttribute(1, eq);
        t->PutAttribute(2, gt);
        t->PutAttribute(3, nk);

        result.addr = t;
        *k = 0;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE : {
      delete (int*) local.addr;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/*
2.4.5 Valuemapping for keytype bool

*/
template<typename valuetype, int argNo>
int keyrange::ValueMappingBool(Word* args, Word& result, int message,
                                  Word& local, Supplier s)
{
  switch (message)
  {
    case OPEN : {
      int* k = new int;
      *k = 1;
      local.addr = k;
      return 0;

    }
    case REQUEST : {

      int* k;
      k = (int*)local.addr;
      if(*k == 1)
      {
        double less, equal, greater;
        BTree2Impl<bool, valuetype>* btree = (BTree2Impl<bool,
                                       valuetype>*)args[0].addr;
        bool keyValue = ((CcInt*)args[argNo].addr)->GetIntval();
        getKeyrangeValues(keyValue, less, equal, greater, btree);

        int n = btree->GetLeafEntryCount();

        CcReal* lt = new CcReal(less);
        CcReal* eq = new CcReal(equal);
        CcReal* gt = new CcReal(greater);
        CcInt* nk = new CcInt(n);

        Tuple* t = new Tuple( nl->Second(GetTupleResultType(s)) );
        t->PutAttribute(0, lt);
        t->PutAttribute(1, eq);
        t->PutAttribute(2, gt);
        t->PutAttribute(3, nk);

        result.addr = t;
        *k = 0;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE : {
      delete (int*) local.addr;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}


/*
2.5 Operator Specifications

*/

struct getKeyrangeInfo : OperatorInfo {

  getKeyrangeInfo() : OperatorInfo()
  {
    name =      "keyrange";
    signature = "(btree2 Tk tid multiple) x rel(tuple(T)) ->"
                " stream(tuple((Less real)"
                "(Equal real)(Greater real)(NumOfKeys int)))";
    syntax =    "_ keyrange [ _ ]";
    meaning =   "Returns an estimate of entries with keys larger, smaller or "
                "equal to argument key.";
    example =   "query staedte_btree2_tid Staedte keyrange [\"D\"] consume";
  }
};

struct getKeyrange2Info : OperatorInfo {

  getKeyrange2Info() : OperatorInfo()
  {
    name =      "keyrange2";
    signature = "(btree2 Tk Td u) x Tk -> stream(tuple((Less real)"
                "(Equal real)(Greater real)(NumOfKeys int)))";
    syntax =    "_ _ keyrange2 [ _ ]";
    meaning =   "Returns an estimate of entries with keys larger, smaller or "
                "equal to argument key.";
    example =   "query staedte_btree2 keyrange2 [\"D\"] consume";
  }
};

ValueMapping keyrange::valueMappings [] =
{

    keyrange::ValueMappingReal<TupleId, 2>,   //0
    keyrange::ValueMappingInt<TupleId, 2>,    //1
    keyrange::ValueMappingString<TupleId, 2>, //2
    keyrange::ValueMappingBool<TupleId, 2>,   //3
    keyrange::ValueMappingAttr<TupleId, 2>    //4
  };


ValueMapping keyrange::valueMappings2 [] =
{

    keyrange::ValueMappingReal<NoneType, 1>,    //0
    keyrange::ValueMappingReal<string, 1>,      //1
    keyrange::ValueMappingReal<int, 1>,         //2
    keyrange::ValueMappingReal<double, 1>,      //3
    keyrange::ValueMappingReal<TupleId, 1>,     //4
    keyrange::ValueMappingReal<Attribute*, 1>,  //5

    keyrange::ValueMappingInt<NoneType, 1>,     //6
    keyrange::ValueMappingInt<string, 1>,       //7
    keyrange::ValueMappingInt<int, 1>,          //8
    keyrange::ValueMappingInt<double, 1>,       //9
    keyrange::ValueMappingInt<TupleId, 1>,      //10
    keyrange::ValueMappingInt<Attribute*, 1>,   //11

    keyrange::ValueMappingString<NoneType, 1>,  //12
    keyrange::ValueMappingString<string, 1>,    //13
    keyrange::ValueMappingString<int, 1>,       //14
    keyrange::ValueMappingString<double, 1>,    //15
    keyrange::ValueMappingString<TupleId, 1>,   //16
    keyrange::ValueMappingString<Attribute*, 1>,//17

    keyrange::ValueMappingBool<NoneType, 1>,    //18
    keyrange::ValueMappingBool<string, 1>,      //19
    keyrange::ValueMappingBool<int, 1>,         //20
    keyrange::ValueMappingBool<double, 1>,      //21
    keyrange::ValueMappingBool<TupleId, 1>,     //22
    keyrange::ValueMappingBool<Attribute*, 1>,  //23

    keyrange::ValueMappingAttr<NoneType, 1>,    //24
    keyrange::ValueMappingAttr<string, 1>,      //25
    keyrange::ValueMappingAttr<int, 1>,         //26
    keyrange::ValueMappingAttr<double, 1>,      //27
    keyrange::ValueMappingAttr<TupleId, 1>,     //28
    keyrange::ValueMappingAttr<Attribute*, 1>   //29

  };

Operator keyrange::keyrange1 (getKeyrangeInfo(), keyrange::valueMappings,
                          keyrange::Select, keyrange::TypeMapping);
Operator keyrange::keyrange2 (getKeyrange2Info(), keyrange::valueMappings2,
                          keyrange::Select2, keyrange::TypeMapping2);
}
}

