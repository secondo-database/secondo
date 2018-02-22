/*

----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]



1.1 Definition of Outerjoin for Module Extended Relation Algebra

Using Storage Manager Berkeley DB

January 2009, B. Poneleit


This file contains the definition of ~Operator~ instances for outerjoin
operators. To include an additional outerjoin operator you have to add
the following to ExtRelationAlgebra.cpp:

Declaration of Operator instance
extern Operator $Operatorname$;

and a call to AddOperator in the constructor of the algebra.

Implementation of the operators is in Outerjoin.cpp

1.1.1 Includes and defines

*/

#include <vector>
#include <list>
#include <set>
#include <queue>

#include "LogMsg.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"
#include "SecondoInterface.h"
#include "StopWatch.h"
#include "Counter.h"
#include "Progress.h"
#include "Symbols.h"
#include "RTuple.h"
#include "Tupleorder.h"
#include "ListUtils.h"

template<bool expectSorted> int
smouterjoin_vm( Word* args, Word& result,
                  int message, Word& local, Supplier s );

template<int dummy> int
symmouterjoin_vm( Word* args, Word& result,
                  int message, Word& local, Supplier s );

/*
2.16.1 Operator ~smouterjoin~

This operator sorts two input streams and computes their full outerjoin.

2.16.1.1 Specification of operator ~smouterjoin~

*/
const std::string SortMergeOuterJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                                  "\"Meaning\" \"Example\" ) "
                             "( <text>((stream (tuple ((x1 t1) ... "
                             "(xn tn)))) (stream (tuple ((y1 d1) ..."
                             " (ym dm)))) xi yj) -> (stream (tuple "
                             "((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)"
                             ")))</text--->"
                             "<text>_ _ smouterjoin [ _ , _ ]"
                             "</text--->"
                             "<text>Computes the full outerjoin of two streams."
                             "</text--->"
                             "<text>query duplicates feed ten feed "
                             "smouterjoin[no, nr] consume</text--->"
                             ") )";

template<int dummy>
ListExpr OuterjoinTypeMap (ListExpr args)
{
  int expLength = 4;
  std::string err = "stream(tuple[y1 : d1, ..., yn : dn]) x "
               "stream(tuple[z1 : e1, ..., zn : en]) x di x e1 ";

  err += " expected";
  if(nl->ListLength(args)!=expLength){
    return listutils::typeError(err + "(wrong number of args)");
  }

  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr attr1 = nl->Third(args);
  ListExpr attr2 = nl->Fourth(args);
  if(!listutils::isTupleStream(stream1) ||
     !listutils::isTupleStream(stream2) ||
     nl->AtomType(attr1)!=SymbolType ||
     nl->AtomType(attr2)!=SymbolType){
    return listutils::typeError(err);
  }

  ListExpr list1 = nl->Second(nl->Second(stream1));
  ListExpr list2 = nl->Second(nl->Second(stream2));
  if(!listutils::disjointAttrNames(list1,list2)){
    return listutils::typeError("Attribute lists are not disjoint");
  }

  ListExpr list = ConcatLists(list1, list2);
  ListExpr outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
      nl->TwoElemList(nl->SymbolAtom("tuple"), list));

  std::string attrAName = nl->SymbolValue(attr1);
  std::string attrBName = nl->SymbolValue(attr2);

  ListExpr attrTypeA, attrTypeB;

  int attrAIndex = listutils::findAttribute(list1, attrAName, attrTypeA);
  if(attrAIndex<1){
    return listutils::typeError("Attributename " + attrAName +
                                " not found in the first argument");
  }

  int attrBIndex = listutils::findAttribute(list2, attrBName, attrTypeB);
  if(attrBIndex<1){
    return listutils::typeError("Attributename " + attrBName +
                                " not found in the second argument");
  }

  if(!nl->Equal(attrTypeA, attrTypeB)){
    return listutils::typeError("different types selected for operation");
  }


  ListExpr joinAttrDescription =
    nl->TwoElemList(nl->IntAtom(attrAIndex), nl->IntAtom(attrBIndex));
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
              joinAttrDescription, outlist);
}

template ListExpr
OuterjoinTypeMap<1>(ListExpr args);

/*
2.16.2.1 Definition of operator ~smouterjoin~

*/
Operator extrelsmouterjoin(
         "smouterjoin",            // name
         SortMergeOuterJoinSpec,          // specification
         smouterjoin_vm<false>,        // value mapping
         Operator::SimpleSelect,     // trivial selection function
         OuterjoinTypeMap<1>       // type mapping
);

ListExpr SymmOuterJoinTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }
  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr map = nl->Third(args);

  std::string err = "stream(tuple1) x stream(tuple2) x "
               "( tuple1 x tuple2 -> bool) expected";
  if(!listutils::isTupleStream(stream1) ||
     !listutils::isTupleStream(stream2) ||
     !listutils::isMap<2>(map)){
    return listutils::typeError(err);
  }

  if(!nl->Equal(nl->Second(stream1), nl->Second(map)) ||
     !nl->Equal(nl->Second(stream2), nl->Third(map)) ||
     !listutils::isSymbol(nl->Fourth(map),CcBool::BasicType())){
    return listutils::typeError(err +"(wrong mapping)");
  }

  ListExpr a1List = nl->Second(nl->Second(stream1));
  ListExpr a2List = nl->Second(nl->Second(stream2));

  if(!listutils::disjointAttrNames(a1List,a2List)){
    return listutils::typeError(err + "(name conflict in tuples");
  }
  ListExpr list = ConcatLists(a1List, a2List);

  return nl->TwoElemList(nl->SymbolAtom("stream"),
           nl->TwoElemList(nl->SymbolAtom("tuple"),
             list));
}

/*

5.10.3.1 Specification of operator ~SymmOuterJoin~

*/
const std::string SymmOuterJoinSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream (tuple (x1 ... xn))) (stream "
  "(tuple (y1 ... ym)))) (map (tuple (x1 ... xn)) "
  "(tuple (y1 ... ym)) -> bool) -> (stream (tuple (x1 "
  "... xn y1 ... ym)))</text--->"
  "<text>_ _ symmouterjoin[ fun ]</text--->"
  "<text>Computes a Cartesian product stream from "
  "its two argument streams filtering by the third "
  "argument.</text--->"
  "<text>query ten feed {a} twenty feed {b} "
  "symmouterjoin[.no_a = .no_b] count</text--->"
  " ) )";

/*

5.10.4.1 Definition of operator ~SymmOuterJoin~

*/
Operator extrelsymmouterjoin (
         "symmouterjoin",            // name
         SymmOuterJoinSpec,          // specification
         symmouterjoin_vm<1>,              // value mapping
         Operator::SimpleSelect,         // trivial selection function
         SymmOuterJoinTypeMap        // type mapping
//         true                   // needs large amounts of memory
);
