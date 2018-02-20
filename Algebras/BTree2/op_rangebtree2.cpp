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

[1] Implementation of the range operators for the BTree2-Algebra

[TOC]

1 Defines and Includes

*/

#include "op_rangebtree2.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
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
The following constant values of the operators are defined to calculate
the number of arguments by (value div 9) and the type of range operator
by calculating (value mod 3). Type 0 means range2, type 1 means rangeS
and type 2 means range2.
The type mapping function and value mapping function use these constant
value as a template parameter.

*/
const int EXACTMATCH2 = 18; // (div 9 = 2) (mod 3 = 0)
const int EXACTMATCHS = 19; // (div 9 = 2) (mod 3 = 1)
const int LEFTRANGE2 = 21;  // (div 9 = 2) (mod 3 = 0)
const int LEFTRANGES = 22;  // (div 9 = 2) (mod 3 = 1)
const int RIGHTRANGE2 = 24; // (div 9 = 2) (mod 3 = 0)
const int RIGHTRANGES = 25; // (div 9 = 2) (mod 3 = 1)
const int EXACTMATCH = 29;  // (div 9 = 3) (mod 3 = 2)
const int LEFTRANGE = 32;   // (div 9 = 3) (mod 3 = 2)
const int RIGHTRANGE = 35;  // (div 9 = 3) (mod 3 = 2)
const int RANGE2 = 27;      // (div 9 = 3) (mod 3 = 0)
const int RANGES = 28;      // (div 9 = 3) (mod 3 = 1)
const int RANGE = 38;       // (div 9 = 4) (mod 3 = 2)


/*
2 Operators ~exactmatch2~, ~exactmatchS~, ~exactmatch~

The operator ~exactmatch2~ uses the given btree2 to find all keys or
(key-value)-pairs of the btree2 with key Tk.
The operator ~exactmatchS~ uses the given btree2 to find all tuples
identifiers where the key matches argument value Tk.
The operator ~exactmatch~ uses the given btree2 to find all tuples
in the given relation where the key matches argument value Tk.

The signatures are

----
  exactmatch2: (btree2 Tk none u) x Tk --> stream(tuple((Key Tk)))
  exactmatch2: (btree2 Tk Td   u) x Tk --> stream(tuple((Key Tk) (Data Td)))

  exactmatchS: (btree2 Tk tid multiple) x Tk --> stream(tuple((id tid)))

  exactmatch:  (btree2 Tk tid multiple) x rel(tuple(T)) x Tk --> stream(tuple(T))

----


2.1 Type mapping function of operators ~exactmatch2~, ~exactmatchS~, ~exactmatch~

The template parameter operatorId specifies the range operator. The operator ~exactmatch2~
with value 18 needs (18 div 9 =) 2 parameters and is a range2-type with code (18 mod 3 =) 0,
the operator ~exactmatchS~ with value 19 needs (19 div 9 =) 2 parameters, too, and is a
rangeS-type with code (19 mod 3 =) 1 and finally the operator ~exactmatch~ with value 29
needs (29 div 9 =) 3 parameters and is a range-type with code (29 mod 3 =) 2.

*/

template<int operatorId>
ListExpr RangeTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=(int)(operatorId/9)){
    return listutils::typeError("wrong number of arguments");
  }
  // Split arguments in the parts depending on the
  // operator-type indicated by operatorId
  ListExpr btree2Description = nl->First(args);
  ListExpr relDescription = nl->TheEmptyList();
  ListExpr keyDescription = nl->TheEmptyList();
  ListExpr secondKeyDescription = nl->TheEmptyList();

  switch ((int)(operatorId/9))
  {
    case 2:
      keyDescription = nl->Second(args);
      break;
    case 3:
      if ((operatorId%3)==2)
      {
        relDescription = nl->Second(args);
        keyDescription = nl->Third(args);
      }
      else
      {
        keyDescription = nl->Second(args);
        secondKeyDescription = nl->Third(args);
      }
      break;
    case 4:
      relDescription = nl->Second(args);
      keyDescription = nl->Third(args);
      secondKeyDescription = nl->Fourth(args);
      break;
  }

  // find out type of key
  if(!listutils::isSymbol(keyDescription)){
    return listutils::typeError("invalid key");
  }

  // check range-operators for a second key
  if ((nl->IsEmpty(secondKeyDescription)) &&
      ((operatorId==RANGE)||
       (operatorId==RANGES)||
       (operatorId==RANGE2)) )
  {
       return listutils::typeError("operator belongs second key type");
  }

  // find out type of second key (if any)
  if ((!nl->IsEmpty(secondKeyDescription)) &&
      (!nl->Equal(keyDescription, secondKeyDescription))) {
       return listutils::typeError("different key types");
  }

  // check btree2 part of argument
  if (!listutils::isBTree2Description(btree2Description)) {
    return listutils::typeError("first argument is not a valid btree2");
  }

  ListExpr btree2KeyType = nl->Second(btree2Description);

  // check that the type of given key is equal to the btree2 key type
  if(!nl->Equal(keyDescription, btree2KeyType)){
    return listutils::typeError("key and btree key are different");
  }

  ListExpr btree2ValType = nl->Third(btree2Description);
  ListExpr btree2Unique = nl->Fourth(btree2Description);

  ListExpr trel = nl->TheEmptyList();
  if (!nl->IsEmpty(relDescription))
  {
    if (!listutils::isRelDescription(relDescription))
    {
      return listutils::typeError("not a relation");
    }
    trel = nl->Second(relDescription);
  }

  // check loadtype and unique-type of overloaded btree-operators
  if ((operatorId%3)>0)
  {
    // check loadtype
    if (!nl->IsEqual(btree2ValType,TupleIdentifier::BasicType())){
      return listutils::typeError("datatype not a TID");
    }
    // check unique-type
    if ((!nl->IsEqual(btree2Unique,"multiple")) &&
            (!nl->IsEqual(btree2Unique,"stable_multiple"))) {
      return listutils::typeError("uniquetype not multiple");
    }
  }

  // generate different tuple-types as result of the operator
  // that is indicated by the operatorId
  ListExpr resultType;
  switch (operatorId%3)
  {
    case 0:
      if (nl->IsEqual(btree2ValType,"none"))
      {
        resultType = nl->TwoElemList(
                   nl->SymbolAtom(Symbol::STREAM()),
                   nl->TwoElemList(
                       nl->SymbolAtom(Tuple::BasicType()),
                       nl->OneElemList(
                           nl->TwoElemList(
                               nl->SymbolAtom("Key"),
                               keyDescription)
               )));
      }
      else
      {
        resultType = nl->TwoElemList(
                   nl->SymbolAtom(Symbol::STREAM()),
                   nl->TwoElemList(
                       nl->SymbolAtom(Tuple::BasicType()),
                       nl->TwoElemList(
                           nl->TwoElemList(
                               nl->SymbolAtom("Key"),
                               keyDescription),
                           nl->TwoElemList(
                               nl->SymbolAtom("Data"),
                               btree2ValType)
               )));
      }
      break;
    case 1:
      resultType = nl->TwoElemList(
                       nl->SymbolAtom(Symbol::STREAM()),
                       nl->TwoElemList(
                           nl->SymbolAtom(Tuple::BasicType()),
                           nl->OneElemList(
                               nl->TwoElemList(
                                   nl->SymbolAtom("Id"),
                                   btree2ValType)
                   )));
      break;
    case 2:
      resultType = nl->TwoElemList(
                       nl->SymbolAtom(Symbol::STREAM()),
                       trel);
      break;
  }

  return resultType;
}

/*
2.2 Value mapping function of operators ~exactmatch2~, ~exactmatchS~, ~exactmatch~

The template parameter operatorId specifies the range operator, a detailled
description is given with the type mapping function.

*/

struct RangeQueryLocalInfo
{
  RangeQueryLocalInfo(): iter(), iterStopMarker(), relation(0),
                         ttype(0), nextOK(false){}

  ~RangeQueryLocalInfo(){
    if(ttype){
       ttype->DeleteIfAllowed();
    }
    ttype=0;
    relation=0;
  }
  BTree2Iterator iter;
  BTree2Iterator iterStopMarker;
  Relation* relation;
  TupleType* ttype;
  bool nextOK;
};
/*
Structure to handle the information of the underlying btree2.

*/

template<int operatorId>
int
RangeQuery(Word* args, Word& result, int message, Word& local, Supplier s)
{
  BTree2* btree2;
  Attribute* key;
  Attribute* secondKey;
  Tuple* tuple;
  TupleId id;
  RangeQueryLocalInfo* localInfo;
  bool nextOK;

  switch (message)
  {
    case OPEN :
      // local information structure for range operations
      localInfo = (RangeQueryLocalInfo*)local.addr;
      if(localInfo){
          delete localInfo;
      }
      localInfo = new RangeQueryLocalInfo;
      btree2 = (BTree2*)args[0].addr;

      localInfo->ttype = new TupleType(nl->Second(GetTupleResultType(s)));
      localInfo->nextOK = true;

      // Split arguments in the parts depending on the
      // operator-type indicated by operatorId
      switch ((int)(operatorId/9))
      {
        case 2:
          key = (Attribute*)args[1].addr;
          secondKey = key;
          break;
        case 3:
          if ((operatorId%3)==2)
          {
            localInfo->relation = (Relation*)args[1].addr;
            key = (Attribute*)args[2].addr;
            secondKey = key;
          }
          else
          {
            key = (Attribute*)args[1].addr;
            secondKey = (Attribute*)args[2].addr;
          }
          break;
        case 4:
          localInfo->relation = (Relation*)args[1].addr;
          key = (Attribute*)args[2].addr;
          secondKey = (Attribute*)args[3].addr;
          break;
      }

      assert(btree2 != 0);
      assert(key != 0);

      // get starting point and endmarker point of the range operation
      // depending on the operator-type indicated by operatorId
      switch(operatorId)
      {
        case EXACTMATCH:
        case EXACTMATCH2:
        case EXACTMATCHS:
        // exactmatch of key
          btree2->findExactmatchBoundary(key, localInfo->iter,
                                              localInfo->iterStopMarker);
          break;
        case RANGE:
        case RANGE2:
        case RANGES:
        // range between and including key and secondKey,
        // where key <= secondKey
          btree2->findRangeBoundary(key, secondKey, localInfo->iter,
                                              localInfo->iterStopMarker);
          break;
        case LEFTRANGE:
        case LEFTRANGE2:
        case LEFTRANGES:
        // leftrange is the range of the B-tree smaller and equal key
          localInfo->iterStopMarker = btree2->findRightBoundary(key);
          localInfo->iter = btree2->begin();
          break;
        case RIGHTRANGE:
        case RIGHTRANGE2:
        case RIGHTRANGES:
        // rightrange is the range of the B-tree greater and equal key
          localInfo->iterStopMarker = btree2->end();
          localInfo->iter = btree2->findLeftBoundary(key);
          break;
        default:
          assert(false);
      }

      // if iterator indicates the endmarker position, the range is empty
      localInfo->nextOK = (localInfo->iter != localInfo->iterStopMarker);

      if (localInfo->iter != btree2->end())
      {
        local = SetWord(localInfo);
        return 0;
      }
      else
      // if iterator indicates the endposition, there is nothing to do
      {
        delete localInfo;
        return -1;
      }
    case REQUEST :
      localInfo = (RangeQueryLocalInfo*)local.addr;

      if (localInfo == 0)
        return CANCEL;

      // nextOK indicates, if there is another result of the iterator
      nextOK = localInfo->nextOK;

      if(nextOK)
      {
        // get the attributes of the result tuple of the range operation
        // depending on the operator-type indicated by operatorId
        switch (operatorId%3)
        {
          case 0:
            tuple = new Tuple(localInfo->ttype);
            tuple->PutAttribute(0, ((localInfo->iter).key())->Copy());
            if (tuple->GetNoAttributes()>1)
              tuple->PutAttribute(1, (*(localInfo->iter))->Copy());
            break;
          case 1:
            tuple = new Tuple(localInfo->ttype);
            tuple->PutAttribute(0, (*(localInfo->iter))->Copy());
            break;
          case 2:
            id = static_cast<TupleIdentifier*>(*(localInfo->iter))->GetTid();
            if (id == 0)
              return CANCEL;
            tuple = localInfo->relation->GetTuple( id, false );
            if(tuple == 0)
            {
              cerr << "Could not find tuple for the given tuple id. "
                   << "Maybe the given btree2 and the given relation "
                   << "do not match." << endl;
              return CANCEL;
              assert(false);
            }
            break;
        }
        result = SetWord(tuple);
        // nextOK is true, if there is another result of the iterator and
        // the iterator has not reached the endmarking point
        localInfo->nextOK = (localInfo->iter).next();
        localInfo->nextOK &= (localInfo->iter != localInfo->iterStopMarker);

        return YIELD;
      }
      else
      // iterator has reached the endmarking point
      {
        return CANCEL;
      }
    case CLOSE :
      if(local.addr)
      {
        localInfo = (RangeQueryLocalInfo*)local.addr;
        delete localInfo;
        local = SetWord(Address(0));
      }
      return 0;
  }
  return 0;
}

/*
2.3 Specification of operators ~exactmatch2~, ~exactmatchS~, ~exactmatch~

2.3.1 Specification of operator ~exactmatch2~

*/
const string ExactMatch2Spec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk none u) x Tk -> stream(tuple((Key Tk)))\n"
               "(btree2 Tk Td   u) x Tk -> stream(tuple((Key Tk)"
               "(Data Td)))";
  string spec = "_ exactmatch2 [ _ ]";
  string meaning = "Uses the given btree2 to find all keys or (key-value)-"
                   "pairs of the btree2 with key ti = argument value Tk";
  string example = "query Staedte_SName_btree2 exactmatch2"
                   "[\"Hagen\"] consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
2.3.2 Specification of operator ~exactmatchS~

*/
const string ExactMatchSSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x Tk -> stream(tuple((id tid)))";
  string spec = "_ exactmatchS [ _ ]";
  string meaning = "Uses the given btree2 to find all tuple identifiers"
                   " where the key matches argument value Tk";
  string example = "query Staedte_SName_btree2 exactmatchS[\"Bochum\"]"
                   " Staedte gettuples consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
2.3.3 Specification of operator ~exactmatch~

*/
const string ExactMatchSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x rel(tuple(T)) x Tk -> "
               "stream(tuple(T))";
  string spec = "_ _ exactmatch [ _ ]";
  string meaning = "Uses the given btree2 to find all tuples in the "
                   "given relation with key = argument value Tk";
  string example = "query Staedte_SName_btree2 Staedte exactmatch"
                   "[\"Dortmund\"] count";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
2.4 Definition of operators ~exactmatch2~, ~exactmatchS~, ~exactmatch~

2.4.1 Definition of operator ~exactmatch2~

*/
Operator rangebtree2::exactmatch2 (
         "exactmatch2",             // name
   ExactMatch2Spec(),               // specification
   RangeQuery<EXACTMATCH2>,         // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<EXACTMATCH2>        // type mapping
);

/*
2.4.2 Definition of operator ~exactmatchS~

*/
Operator rangebtree2::exactmatchS (
         "exactmatchS",             // name
   ExactMatchSSpec(),               // specification
   RangeQuery<EXACTMATCHS>,         // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<EXACTMATCHS>        // type mapping
);


/*
2.4.3 Definition of operator ~exactmatch~

*/
Operator rangebtree2::exactmatch (
         "exactmatch",              // name
   ExactMatchSpec(),                // specification
   RangeQuery<EXACTMATCH>,          // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<EXACTMATCH>         // type mapping
);


/*
3 Operators ~range2~, ~rangeS~, ~range~

The operator ~range2~ uses the given btree2 to find all stored keys or
(key-value)-pairs of the btree2 with key $ >= $ Tk argument value 1 and
key $ <= $ Tk argument value 2.
The operator ~rangeS~ uses the given btree2 to find all stored tuple
identifiers referencing tuples with key $ >= $ Tk argument value 1 and
key $ <= $ Tk argument value 2.
The operator ~range~ uses the given btree2 to find all tuples in the
given relation with key $ >= $ Tk argument value 1 and key $ <= $ Tk argument
value 2.
The stream is empty, if (Tk argument value 2) $ < $ (Tk argument value 1)
or the given range includes no key.

The signatures are

----
range2: (btree2 Tk none u) x Tk x Tk --> stream(tuple((Key Tk)))
range2: (btree2 Tk Td   u) x Tk x Tk --> stream(tuple((Key Tk) (Data Td)))

rangeS: (btree2 Tk tid multiple) x Tk x Tk --> stream(tuple((id tid)))

range:  (btree2 Tk tid multiple) x rel(tuple(T)) x Tk x Tk --> stream(tuple(T))

----


3.1 Type mapping function of operators ~range2~, ~rangeS~, ~range~

The same as for ~exactmatch2~, ~exactmatchS~ and ~exactmatch~.
The template parameter operatorId specifies the range operator. The operator ~range2~
with value 27 needs (27 div 9 =) 3 parameters and is a range2-type with code (27 mod 3 =) 0,
the operator ~rangeS~ with value 28 needs (28 div 9 =) 3 parameters, too, and is a
rangeS-type with code (28 mod 3 =) 1 and finally the operator ~range~ with value 38
needs (38 div 9 =) 4 parameters and is a range-type with code (38 mod 3 =) 2.


3.2 Value mapping function of operators ~range2~, ~rangeS~, ~range~

The same as for ~exactmtach2~, ~exactmatchS~ and ~exactmatch~. The template parameter
operatorId specifies the range operator, a detailled description is given with the
type mapping function of the operators ~range2~, ~rangeS~ and ~range~.


3.3 Specification  of operators ~range2~, ~rangeS~, ~range~

3.3.1 Specification of operator ~range2~

*/
const string Range2Spec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk none u) x Tk x Tk -> stream(tuple((Key Tk)))\n"
               "(btree2 Tk Td   u) x Tk x Tk -> stream(tuple((Key Tk) "
               "(Data Td)))";
  string spec = "_ range2 [ _ , _ ]";
  string meaning = "Uses the given btree2 to find all stored keys or "
                   "(key-value)-pairs of the btree2 "
                   "with key >= Tk argument value 1"
                   " and key <= Tk argument value 2";
  string example = "query Staedte_Bev_btree2 range2[105000, 200000] consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
3.3.2 Specification of operator ~rangeS~

*/
const string RangeSSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x Tk x Tk -> "
               "stream(tuple((id tid)))";
  string spec = "_ rangeS [ _ , _ ]";
  string meaning = "Uses the given btree2 to find all stored "
                   "tuple identifiers referencing tuples "
                   "with key >= Tk argument value 1"
                   " and key <= Tk argument value 2";
  string example = "query Staedte_Bev_btree2 rangeS[100000, 500000] "
                   "Staedte gettuples consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
3.3.3 Specification of operator ~range~

*/
const string RangeSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x rel(tuple(T)) x Tk x Tk"
               " -> stream(tuple(T))";
  string spec = "_ _ range [ _ , _ ]";
  string meaning = "Uses the given btree2 to find all tuples in the "
                   "given relation with key >= Tk argument value 1"
                   " and key <= Tk argument value 2";
  string example = "query Staedte_Bev_btree2 Staedte range"
                   "[250000, 400000] consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
3.4 Definition  of operators ~range2~, ~rangeS~, ~range~

3.4.1 Definition of operator ~range2~

*/
Operator rangebtree2::range2 (
         "range2",                  // name
   Range2Spec(),                    // specification
   RangeQuery<RANGE2>,              // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<RANGE2>             // type mapping
);

/*
3.4.2 Definition of operator ~rangeS~

*/
Operator rangebtree2::rangeS (
         "rangeS",                  // name
   RangeSSpec(),                    // specification
   RangeQuery<RANGES>,              // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<RANGES>             // type mapping
);

/*
3.4.3 Definition of operator ~range~

*/
Operator rangebtree2::range (
         "range",                   // name
   RangeSpec(),                     // specification
   RangeQuery<RANGE>,               // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<RANGE>              // type mapping
);

/*
4 Operators ~leftrange2~, ~leftrangeS~, ~leftrange~

The operator ~leftrange2~ uses the given btree2 to find all stored keys or
(key-value)-pairs of the btree2 with key $ <= $ argument value Tk.
The operator ~leftrangeS~ uses the given btree2 to find all stored tuple
identifiers referencing tuples with key $ <= $ argument value Tk.
The operator ~leftrange~ uses the given btree2 to find all tuples in the
given relation with key $ <= $ argument value Tk.

The signatures are

----
  leftrange2: (btree2 Tk none u) x Tk --> stream(tuple((Key Tk)))
  leftrange2: (btree2 Tk Td   u) x Tk --> stream(tuple((Key Tk) (Data Td)))

  leftrangeS: (btree2 Tk tid multiple) x Tk --> stream(tuple((id tid)))

  leftrange:  (btree2 Tk tid multiple) x rel(tuple(T)) x Tk --> stream(tuple(T))

----


4.1 Type mapping function of operators ~leftrange2~, ~leftrangeS~, ~leftrange~

The same as for ~exactmtach2~, ~exactmatchS~ and ~exactmatch~.
The template parameter operatorId specifies the range operator. The operator ~leftrange2~
with value 21 needs (21 div 9 =) 2 parameters and is a range2-type with code (21 mod 3 =) 0,
the operator ~leftrangeS~ with value 22 needs (22 div 9 =) 2 parameters, too, and is a
rangeS-type with code (22 mod 3 =) 1 and finally the operator ~leftrange~ with value 32
needs (32 div 9 =) 3 parameters and is a range-type with code (32 mod 3 =) 2.


4.2 Value mapping function of operators ~leftrange2~, ~leftrangeS~, ~leftrange~

The same as for ~exactmatch2~, ~exactmatchS~ and ~exactmatch~. The template parameter
operatorId specifies the range operator, a detailled description is given with the
type mapping function of the operators ~leftrange2~, ~leftrangeS~ and ~leftrange~.


4.3 Specification of operators ~leftrange2~, ~leftrangeS~, ~leftrange~

4.3.1 Specification of operator ~leftrange2~

*/
const string LeftRange2Spec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk none u) x Tk -> stream(tuple((Key Tk)))\n"
               "(btree2 Tk Td   u) x Tk -> stream(tuple((Key Tk) "
               "(Data Td)))";
  string spec = "_ leftrange2 [ _ ]";
  string meaning = "Uses the given btree2 to find all stored keys or "
                   "(key-value)-pairs of the btree "
                   "with key <= argument value Tk";
  string example = "query Staedte_Bev_btree2 leftrange2[150000] consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
4.3.2 Specification of operator ~leftrangeS~

*/
const string LeftRangeSSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x Tk -> stream(tuple((id tid)))";
  string spec = "_ leftrangeS [ _ ]";
  string meaning = "Uses the given btree2 to find all stored "
                   "tuple identifiers referencing tuples "
                   "with key <= argument value Tk";
  string example = "query Staedte_Bev_btree2 leftrangeS[250000] Staedte "
                   "gettuples consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
4.3.3 Specification of operator ~leftrange~

*/
const string LeftRangeSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x rel(tuple(T)) x Tk -> "
               "stream(tuple(T))";
  string spec = "_ _ leftrange [ _ ]";
  string meaning = "Uses the given btree2 to find all tuples in the "
                   "given relation with key <= argument value Tk";
  string example = "query Staedte_Bev_btree2 Staedte leftrange"
                   "[500000] consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
4.4 Definition of operators ~leftrange2~, ~leftrangeS~, ~leftrange~

4.4.1 Definition of operator ~leftrange2~

*/
Operator rangebtree2::leftrange2 (
         "leftrange2",              // name
   LeftRange2Spec(),                // specification
   RangeQuery<LEFTRANGE2>,          // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<LEFTRANGE2>         // type mapping
);

/*
4.4.2 Definition of operator ~leftrangeS~

*/
Operator rangebtree2::leftrangeS (
         "leftrangeS",              // name
   LeftRangeSSpec(),                // specification
   RangeQuery<LEFTRANGES>,          // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<LEFTRANGES>         // type mapping
);

/*
4.4.3 Definition of operator ~leftrange~

*/
Operator rangebtree2::leftrange (
         "leftrange",               // name
   LeftRangeSpec(),                 // specification
   RangeQuery<LEFTRANGE>,           // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<LEFTRANGE>          // type mapping
);

/*
5 Operators ~rightrange2~, ~rightrangeS~, ~rightrange~

The operator ~rightrange2~ uses the given btree2 to find all stored keys or
(key-value)-pairs of the btree2 with key $ >= $ argument value Tk.
The operator ~rightrangeS~ uses the given btree2 to find all stored tuple
identifiers referencing tuples with key $ >= $ argument value Tk.
The operator ~rightrange~ uses the given btree2 to find all tuples in the
given relation with key $ >= $ argument value Tk.

The signatures are

----
  rightrange2: (btree2 Tk none u) x Tk --> stream(tuple((Key Tk)))
  rightrange2: (btree2 Tk Td   u) x Tk --> stream(tuple((Key Tk) (Data Td)))

  rightrangeS: (btree2 Tk tid multiple) x Tk --> stream(tuple((id tid)))

  rightrange:  (btree2 Tk tid multiple) x rel(tuple(T)) x Tk --> stream(tuple(T))

----


5.1 Type mapping function of operators ~rightrange2~, ~rightrangeS~, ~rightrange~

The same as for ~exactmatch2~, ~exactmatchS~ and ~exactmatch~.
The template parameter operatorId specifies the range operator. The operator ~rightrange2~
with value 24 needs (24 div 9 =) 2 parameters and is a range2-type with code (24 mod 3 =) 0,
the operator ~rightrangeS~ with value 25 needs (25 div 9 =) 2 parameters, too, and is a
rangeS-type with code (25 mod 3 =) 1 and finally the operator ~rightrange~ with value 35
needs (35 div 9 =) 3 parameters and is a range-type with code (35 mod 3 =) 2.


5.2 Value mapping function of operators ~rightrange2~, ~rightrangeS~, ~rightrange~

The same as for ~exactmtach2~, ~exactmatchS~ and ~exactmatch~. The template parameter
operatorId specifies the range operator, a detailled description is given with the
type mapping function of ~rightrange2~, ~rightrangeS~ and ~rightrange~.


5.3 Specification of operators ~rightrange2~, ~rightrangeS~, ~rightrange~

5.3.1 Specification of operator ~rightrange2~

*/
const string RightRange2Spec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk none u) x Tk -> stream(tuple((Key Tk)))\n"
               "(btree2 Tk Td   u) x Tk -> stream(tuple((Key Tk) "
               "(Data Td)))";
  string spec = "_ rightrange2 [ _ ]";
  string meaning = "Uses the given btree2 to find all stored keys or "
                   "(key-value)-pairs of the btree "
                   "with key >= argument value Tk";
  string example = "query Staedte_Bev_btree2 rightrange2[500000] consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
5.3.2 Specification of operator ~rightrangeS~

*/
const string RightRangeSSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x Tk -> stream(tuple((id tid)))";
  string spec = "_ rightrangeS [ _ ]";
  string meaning = "Uses the given btree2 to find all stored "
                   "tuple identifiers referencing tuples "
                   "with key >= argument value Tk";
  string example = "query Staedte_Bev_btree2 rightrangeS[1500000] Staedte "
                   "gettuples consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
5.3.3 Specification of operator ~rightrange~

*/
const string RightRangeSpec() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk tid multiple) x rel(tuple(T)) x Tk -> "
               "stream(tuple(T))";
  string spec = "_ _ rightrange [ _ ]";
  string meaning = "Uses the given btree2 to find all tuples in the "
                   "given relation with key >= argument value Tk";
  string example = "query Staedte_Bev_btree2 Staedte rightrange"
                   "[1000000] consume";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

/*
5.4 Definition of operators ~rightrange2~, ~rightrangeS~, ~rightrange~

5.4.1 Definition of operator ~rightrange2~

*/
Operator rangebtree2::rightrange2 (
         "rightrange2",             // name
   RightRange2Spec(),               // specification
   RangeQuery<RIGHTRANGE2>,         // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<RIGHTRANGE2>        // type mapping
);

/*
5.4.2 Definition of operator ~rightrangeS~

*/
Operator rangebtree2::rightrangeS (
         "rightrangeS",             // name
   RightRangeSSpec(),               // specification
   RangeQuery<RIGHTRANGES>,         // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<RIGHTRANGES>        // type mapping
);

/*
5.4.3 Definition of operator ~rightrange~

*/
Operator rangebtree2::rightrange (
         "rightrange",              // name
   RightRangeSpec(),                // specification
   RangeQuery<RIGHTRANGE>,          // value mapping
   Operator::SimpleSelect,          // selection function
   RangeTypeMap<RIGHTRANGE>         // type mapping
);


} // end namespace operators
} // end namespace btree2algebra

