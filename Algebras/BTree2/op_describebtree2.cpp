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

[1] Implementation of some description operators for the BTree2-Algebra

[TOC]

1 Defines and Includes

*/

#include "op_describebtree2.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Symbols.h"

#include "BTree2.h"
#include "BTree2Iterator.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~getNodeInfo~

This operator allows introspection of a BTree2.
It creates a stream of a tuple, which describes the node of the BTree2 with the
specified record number. If the record number is not valid, the stream is empty.
The tuple contains the record number of the interesting node, the number of sons,
bool values which show if the node is a leaf node resp. a root node and finally
displays the minimum key value of the node.

Signature is

----
getNodeInfo: (btree2 Tk Td u) x int -> stream(tuple((NodeId int) (NoOfSons int)
                                             (IsLeafNode bool) (IsRootNode bool)
                                                                   (MinKey Tk)))

----

*/

/*
2.1 TypeMapping for Operator ~getNodeInfo~

*/
ListExpr getNodeInfoTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expected exactly 2 arguments.");
  };
  // test first argument for btree2
  ListExpr btree2Description = nl->First(args);
  if( !listutils::isBTree2Description(btree2Description ) ){
    return listutils::typeError("Expected BTree2 as 1st argument.");
  }

  // get key description
  ListExpr keyDescription = nl->Second(nl->First(args));

  // test second argument for integer
  ListExpr RecNumber = nl->Second(args);
  if ( !nl->IsAtom(RecNumber) || !listutils::isNumericType(RecNumber) )
  {
    return listutils::typeError("Expecting int as second argument.");
  }

  ListExpr reslist =
   nl->TwoElemList(
    nl->SymbolAtom(Symbol::STREAM()),
    nl->TwoElemList(
     nl->SymbolAtom(Tuple::BasicType()),
     nl->FiveElemList(
         nl->TwoElemList(nl->SymbolAtom("NodeId"),
                         nl->SymbolAtom(CcInt::BasicType())),
         nl->TwoElemList(nl->SymbolAtom("NoOfSons"),
                         nl->SymbolAtom(CcInt::BasicType())),
         nl->TwoElemList(nl->SymbolAtom("IsLeafNode"),
                         nl->SymbolAtom(CcBool::BasicType())),
         nl->TwoElemList(nl->SymbolAtom("IsRootNode"),
                         nl->SymbolAtom(CcBool::BasicType())),
         nl->TwoElemList(nl->SymbolAtom("MinKey"), keyDescription)
     )
    )
   );
  return reslist;
}

/*
2.2 Value Mapping for Operator ~getNodeInfo~

*/

struct GetNodeLocalInfo{
  BTree2* btree2;
  int nodeId;
  int index;
  int maxEntries;
  TupleType* ttype;
  bool once;

  GetNodeLocalInfo(BTree2* bt2, int nId, int max, TupleType* tty):
      btree2(bt2), nodeId(nId), maxEntries(max), ttype(tty)
  {
    index = 0;
    once = true;
  }
};
/*
Structure to handle the information of the interesting node.

*/

bool checkRecordId(BTree2* btree2, int nodeId)
{
  bool findRID = false;
  if (   (nodeId > 0)
      && (nodeId != static_cast<int>(btree2->GetHeaderId()))
      && (nodeId < btree2->GetNodeCount()+2))
      findRID = true;
  return findRID;
}

/*
Checks if the given nodeId is valid in btree2

*/

int getNodeInfoValueMap( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  GetNodeLocalInfo* gnli;

  switch( message )
  {
    case OPEN:
    {
      int nodeId = ((CcInt*)args[1].addr)->GetIntval();
      BTree2* btree2 = (BTree2*) args[0].addr;
      if ((btree2 != 0)&&(checkRecordId(btree2, nodeId)))
      {
        gnli = new GetNodeLocalInfo(btree2, nodeId,
                   btree2->GetNodeEntryCount(nodeId),
                   new TupleType(nl->Second(GetTupleResultType(s))));
        local.setAddr(gnli);
      }
      else
        local.setAddr(0);
      return 0;
    }

    case REQUEST:
    {
      if(local.addr == NULL)
      {
        return CANCEL;
      }

      gnli = (GetNodeLocalInfo*)local.addr;

      int noOfSons = 0;
      bool isInternal = gnli->btree2->ProbeIsInternal(gnli->nodeId);
      if (isInternal)
        noOfSons = gnli->maxEntries+1;
      bool isRoot =
                gnli->nodeId == static_cast<int>(gnli->btree2->GetRootNode());

      if (gnli->once) {
        Tuple *tuple = new Tuple( gnli->ttype );
        tuple->PutAttribute(0, new CcInt(true, gnli->nodeId));
        tuple->PutAttribute(1, new CcInt(true, noOfSons));
        tuple->PutAttribute(2, new CcBool(true, !isInternal));
        tuple->PutAttribute(3, new CcBool(true, isRoot));
        if (isInternal)
          tuple->PutAttribute(4,
                         gnli->btree2->GetEntryKeyInternal(gnli->nodeId, 0));
        else
          tuple->PutAttribute(4, gnli->btree2->GetEntryKey(gnli->nodeId, 0));
        result.setAddr(tuple);
        gnli->once = false;
        return YIELD;
      }
      else
        return CANCEL;
    }

    case CLOSE:
    {
      if(local.addr)
      {
        gnli = (GetNodeLocalInfo*)local.addr;
        gnli->ttype->DeleteIfAllowed();
        delete gnli;
        local.setAddr(Address(0));
      }
      return 0;
    }
  } // end switch
  cout << "BTree2GetNodeInfoVM(): Received UNKNOWN message!" << endl;
  return 0;
}

/*
2.3 Specification for Operator ~getNodeInfo~

*/
const string getNodeInfoSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk Td u) x int -> "
               "stream(tuple((NodeId int)(NoOfSons int)(IsLeafNode bool)"
               "(IsRootNode bool)(MinKey Tk)))";
  string spec = "getNodeInfo( _, _ )";
  string meaning = "Displays an Info of the given Node of the BTree2 ";
  string example = "query getNodeInfo(Staedte_SName_btree2, 5) consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
2.4 Definition of Operator ~getNodeInfo~

*/
Operator describebtree2::getnodeinfo(
         "getNodeInfo",           // name
         getNodeInfoSpec(),       // specification
         getNodeInfoValueMap,     // value mapping
         Operator::SimpleSelect,  // selection function
         getNodeInfoTypeMap       // type mapping
        );


/*
3 Operator ~getNodeSons~

This operator allows introspection of a BTree2. It creates a stream
of tuples, each describe a son node of the node with the specified record no.
If the record number is not valid, the stream is empty.
Each tuple of the stream contains the record number of the interesting node,
the record number of the son node, the minimum key value and the maximum key
value of the son node.

Signature is

----
getNodeSons: (btree2 Tk Td u) x int -> stream(tuple((NodeId int) (SonId int)
                                                    (Lower Tk) (Upper Tk)))

----

*/

/*
3.1 TypeMapping for Operator ~getNodeSons~

*/
ListExpr getNodeSonsTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expected exactly 2 arguments.");
  };
  // test first argument for btree2
  ListExpr btree2Description = nl->First(args);
  if( !listutils::isBTree2Description(btree2Description ) ){
    return listutils::typeError("Expected BTree2 as 1st argument.");
  }

  // get key description
  ListExpr keyDescription = nl->Second(nl->First(args));

  // test second argument for integer
  ListExpr RecNumber = nl->Second(args);
  if ( !nl->IsAtom(RecNumber) || !listutils::isNumericType(RecNumber) )
  {
    return listutils::typeError("Expecting int as second argument.");
  }

  ListExpr reslist =
     nl->TwoElemList(
       nl->SymbolAtom(Symbol::STREAM()),
       nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
        nl->FourElemList(
          nl->TwoElemList(nl->SymbolAtom("NodeId"),
                          nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("SonId"),
                          nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("Lower"), keyDescription),
          nl->TwoElemList(nl->SymbolAtom("Upper"), keyDescription)
        )
       )
     );
  return reslist;
}

/*
3.2 Value Mapping for Operator ~getNodeSons~

*/
int getNodeSonsValueMap( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  GetNodeLocalInfo* gnli;

  switch( message )
  {
    case OPEN:
    {
      int nodeId = ((CcInt*)args[1].addr)->GetIntval();
      BTree2* btree2 = (BTree2*) args[0].addr;
      if ((btree2 != 0)&&(checkRecordId(btree2, nodeId)))
      {
        gnli = new GetNodeLocalInfo(btree2, nodeId,
                   btree2->GetNodeEntryCount(nodeId),
                   new TupleType(nl->Second(GetTupleResultType(s))));
        local.setAddr(gnli);
      }
      else
        local.setAddr(0);
      return 0;
    }

    case REQUEST:
    {
      if(local.addr == NULL)
      {
        return CANCEL;
      }
      gnli = (GetNodeLocalInfo*)local.addr;

      if (gnli->index == gnli->maxEntries+1)
      {
        return CANCEL;
      }

      Tuple* tuple = new Tuple( gnli->ttype );
      tuple->PutAttribute(0, new CcInt(true, gnli->nodeId));

      bool isInternal = gnli->btree2->ProbeIsInternal(gnli->nodeId);
      if (isInternal)
      {
        int sonId =
          gnli->btree2->GetEntryValueInternal(gnli->nodeId, gnli->index);
        tuple->PutAttribute(1, new CcInt(true, sonId));

        bool isSonInternal = gnli->btree2->ProbeIsInternal(sonId);
        int maxCount= gnli->btree2->GetNodeEntryCount(sonId);
        if (isSonInternal)
        {
          tuple->PutAttribute(2,
                       gnli->btree2->GetEntryKeyInternal(sonId, 0));
          tuple->PutAttribute(3,
                       gnli->btree2->GetEntryKeyInternal(sonId, maxCount-1));
        }
        else
        {
          tuple->PutAttribute(2, gnli->btree2->GetEntryKey(sonId, 0));
          tuple->PutAttribute(3, gnli->btree2->GetEntryKey(sonId, maxCount-1));
        }
        gnli->index++;
      }
      else
      {
        tuple->PutAttribute(1, new CcInt(false, 0));

        Attribute* attr = gnli->btree2->GetEntryKey(gnli->nodeId, 0);
        attr->SetDefined(false);
        tuple->PutAttribute(2, attr->Copy());
        tuple->PutAttribute(3, attr->Copy());
        attr->DeleteIfAllowed();
        gnli->index = gnli->maxEntries+1;
      }
      result.setAddr(tuple);
      return YIELD;
    }

    case CLOSE:
    {
      if(local.addr)
      {
        gnli = (GetNodeLocalInfo*)local.addr;
        gnli->ttype->DeleteIfAllowed();
        delete gnli;
        local.setAddr(Address(0));
      }
      return 0;
    }
  } // end switch
  cout << "BTree2GetNodeSonsVM(): Received UNKNOWN message!" << endl;
  return 0;
}

/*
3.3 Specification for Operator ~getNodeSons~

*/
const string getNodeSonsSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk Td u) x int -> "
               "stream(tuple((NodeId int)(SonId int)"
               "(Lower Tk)(Upper Tk)))";
  string spec = "getNodeSons( _, _ )";
  string meaning = "Displays all Sons of the given Node of the BTree2 ";
  string example = "query getNodeSons(Staedte_SName_btree2, 5) consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
3.4 Definition of Operator ~getNodeSons~

*/
Operator describebtree2::getnodesons(
         "getNodeSons",             // name
         getNodeSonsSpec(),         // specification
         getNodeSonsValueMap,       // value mapping
         Operator::SimpleSelect,    // selection function
         getNodeSonsTypeMap         // type mapping
        );


/*
4 Operator ~internal\_node\_capacity~

This operator allows introspection of a BTree2.
It gives the number of the internal node capacity. The value depends on
the nodesize of the btree2.

Signature is

----
     internal_node_capacity: (btree2 Tk Td u) ->  int

----

*/

/*
4.1 TypeMapping for Operator ~internal\_node\_capacity~

*/
ListExpr internalNodeCapacityTypeMap( ListExpr args)
{
    if(nl->ListLength(args) != 1){
      return listutils::typeError("Operator expects exactly one argument");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 object"
                                  " as argument.");
    }
  return (nl->SymbolAtom(CcInt::BasicType()));
}


/*
4.2 Value Mapping for Operator ~internal\_node\_capacity~

*/
int internalNodeCapacityValueMap(Word* args, Word& result, int message,
                                 Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree2 = (BTree2*)args[0].addr;
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree2->GetMaxEntries(true) );
  return 0;
}

/*

4.3 Specification of operator ~internal\_node\_capacity~

*/
const string internalNodeCapacitySpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk Td u) -> int";
  string spec = "internal_node_capacity( _ )";
  string meaning = "Displays the number of the internal node capacity "
                   "of the given btree2";
  string example = "query internal_node_capacity(Staedte_SName_btree2)";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
4.4 Definition of operator ~internal\_node\_capacity~

*/
Operator describebtree2::internalnodecapacity (
         "internal_node_capacity",              // name
         internalNodeCapacitySpec(),            // specification
         internalNodeCapacityValueMap,          // value mapping
         Operator::SimpleSelect,                // selection function
         internalNodeCapacityTypeMap            // type mapping
);


/*
5 Operator ~leaf\_node\_capacity~

This operator allows introspection of a BTree2.
It gives the number of the leaf node capacity. The value depends on
the nodesize of the btree2.

Signature is

----
     leaf_node_capacity: (btree2 Tk Td u) ->  int

----

*/

/*
5.1 TypeMapping for Operator ~leaf\_node\_capacity~

*/
ListExpr leafNodeCapacityTypeMap( ListExpr args)
{
    if(nl->ListLength(args) != 1){
      return listutils::typeError("Operator expects exactly one argument");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 "
                                  "object as argument.");
    }
  return (nl->SymbolAtom(CcInt::BasicType()));
}


/*
5.2 Value Mapping for Operator ~leaf\_node\_capacity~

*/
int leafNodeCapacityValueMap(Word* args, Word& result, int message,
                                 Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree2 = (BTree2*)args[0].addr;
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree2->GetMaxEntries(false) );
  return 0;
}

/*
5.3 Specification of operator ~leaf\_node\_capacity~

*/
const string leafNodeCapacitySpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk Td u) -> int";
  string spec = "leaf_node_capacity( _ )";
  string meaning = "Displays the number of the leaf note capacity "
                   "of the given btree2";
  string example = "query leaf_node_capacity(Staedte_SName_btree2)";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
5.4 Definition of operator ~leaf\_node\_capacity~

*/
Operator describebtree2::leafnodecapacity (
         "leaf_node_capacity",              // name
         leafNodeCapacitySpec(),            // specification
         leafNodeCapacityValueMap,          // value mapping
         Operator::SimpleSelect,                // selection function
         leafNodeCapacityTypeMap            // type mapping
);


/*
6 Operator ~getMinFillDegree~

This operator allows introspection of a BTree2.
It gives the value of the specified minimum fill degree.

Signature is

----
     getMinFillDegree: (btree2 Tk Td u) ->  real

----

*/

/*
6.1 TypeMapping for Operator ~getMinFillDegree~

*/
ListExpr getminfilldegreeTypeMap( ListExpr args){
    if(nl->ListLength(args) != 1){
      return listutils::typeError("Operator expects exactly one argument");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 "
                                  "object as argument.");
    }
  return (nl->SymbolAtom(CcReal::BasicType()));
}


/*
6.2 Value Mapping for Operator ~getMinFillDegree~

*/
int getminfilldegreeValueMap(Word* args, Word& result, int message,
                                 Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree2 = (BTree2*)args[0].addr;
  CcReal *res = (CcReal*) result.addr;
  res->Set( true, btree2->GetFill() );
  return 0;
}

/*
6.3 Specification of operator ~getMinFillDegree~

*/
const string getminfilldegreeSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk Td u) -> real";
  string spec = "getMinFillDegree( _ )";
  string meaning = "Displays the value of the specified minimum fill "
                   "degree of the given btree2";
  string example = "query getMinFillDegree(Staedte_SName_btree2)";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
6.4 Definition of operator ~getMinFillDegree~

*/
Operator describebtree2::getminfilldegree (
         "getMinFillDegree",                // name
         getminfilldegreeSpec(),            // specification
         getminfilldegreeValueMap,          // value mapping
         Operator::SimpleSelect,            // selection function
         getminfilldegreeTypeMap            // type mapping
);


/*
7 Operator ~getNodeSize~

This operator allows introspection of a BTree2.
It gives the value of the specified record size of the node.

Signature is

----
     getNodeSize: (btree2 Tk Td u) ->  int

----

*/

/*
7.1 TypeMapping for Operator ~getNodeSize~

*/
ListExpr getnodesizeTypeMap( ListExpr args){
    if(nl->ListLength(args) != 1){
       return listutils::typeError("Operator expects exactly one argument");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 object"
                                  " as argument.");
    }
  return (nl->SymbolAtom(CcInt::BasicType()));
}


/*
7.2 Value Mapping for Operator ~getNodeSize~

*/
int getnodesizeValueMap(Word* args, Word& result, int message,
                                 Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree2 = (BTree2*)args[0].addr;
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree2->GetRecordSize() );
  return 0;
}

/*
7.3 Specification of operator ~getNodeSize~

*/
const string getnodesizeSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk Td u) -> int";
  string spec = "getNodeSize( _ )";
  string meaning = "Displays the value of the specified record size "
                   "of the nodes of the given btree2";
  string example = "query getNodeSize(Staedte_SName_btree2)";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
7.4 Definition of operator ~getNodeSize~

*/
Operator describebtree2::getnodesize (
         "getNodeSize",                // name
         getnodesizeSpec(),            // specification
         getnodesizeValueMap,          // value mapping
         Operator::SimpleSelect,            // selection function
         getnodesizeTypeMap            // type mapping
);


} // end namespace operators
} // end namespace btree2algebra

