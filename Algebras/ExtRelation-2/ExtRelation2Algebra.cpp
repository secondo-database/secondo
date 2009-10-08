/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
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

May 2009. Sven Jungnickel. Optimized implementation of operators sort,
sortby, sortmergejoin and a new hashjoin implementation called
hybridhashjoin. Result of the work for my master's thesis.

[1] Implementation of the Module Extended Relation2 Algebra for
Persistent Storage

[TOC]

0 Overview

This file contains an optimized implementation of algorithms for
external sorting, sortmergejoin and a new hash-join algorithm called
hybrid hash-join. Hybrid hash-join is an advanced version of the
GRACE hash-join. These implementations are the results of my work
for my master's thesis


1 Includes and defines

*/
#include <vector>
#include <deque>
#include <sstream>
#include <stack>
#include <limits.h>
#include <set>

#include "LogMsg.h"
#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "SecondoInterface.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "Progress.h"
#include "RTuple.h"
#include "Symbols.h"
#include "Sort.h"

using namespace std;
using namespace symbols;

/*
2 External linking

*/

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

/*
3 Operators

All operators have been put into namespace ~extrel2~. In this
way it will later be easier to replace the existing operators
by the new implementations.

*/
namespace extrel2
{

/*
3.1 Operator ~sort2~

This operator sorts a stream of tuples lexicographically.

3.1.1 Type mapping function of Operator ~sort2~

Type mapping for ~sort2~ is

----  (stream (tuple ((x1 t1)...(xn tn))))  -> ((stream (tuple ((x1 t1)...(xn tn))))
              APPEND (n i1 true i2 true ... in true))
----

*/

ListExpr SortTypeMap( ListExpr args )
{
  NList type(args);

  // check list length
  if ( !type.hasLength(1) )
  {
    return NList::typeError(
        "Operator sort expects a list of length one.");
  }

  // check if first argument is a tuple stream
  NList attr;
  if ( !type.first().checkStreamTuple(attr) )
  {
    return NList::typeError(
        "Operator sort2: first argument is not a tuple stream!"
        "Operator received: " + type.first().convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attr.listExpr()) )
  {
    return NList::typeError(
        "Operator sort2: first argument does not contain a tuple description!"
        "Operator received: " + attr.convertToString() );
  }

  // create SortOrderSpecification as nested list
  // (n i1 asc i2 asc ... in asc)
  // n: number of tuple attributes
  // i1: index of first attribute (1-based)
  // i2: index of second attribute and so on (1-based)
  // ...
  // asc: boolean value true (ascending sort order = true)
  int n = attr.length();

  NList sortDesc;
  sortDesc.append(NList(n));

  for(int i = 1; i <= n; i++)
  {
    sortDesc.append(NList(i));
    sortDesc.append(NList(true,true));
  }

  return NList(NList(APPEND), sortDesc, type.first()).listExpr();
}

/*
3.1.2 Value mapping function of operator ~sort2~

The argument vector ~args~ contains in the first slot ~args[0]~ the
stream and in ~args[2]~ the number of sort attributes. ~args[3]~
contains the index of the first sort attribute, ~args[4]~ a boolean
indicating whether the stream is sorted in ascending order with regard
to the sort first attribute. ~args[5]~ and ~args[6]~ contain these
values for the second sort attribute  and so on.

*/

int SortValueMap( Word* args, Word& result,
                   int message, Word& local, Supplier s );

/*
3.1.3 Specification of operator ~sort2~

*/
const string SortSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                         ")))) -> (stream (tuple([a1:d1, ... ,an:dn])))"
                         "</text--->"
                         "<text>_ sort2</text--->"
                         "<text>Sorts input stream lexicographically."
                         "</text--->"
                         "<text>query cities feed sort2 consume</text--->"
                         ") )";

/*
3.1.4 Definition of operator ~sort2~

*/
Operator extrelsort (
         "sort2",                 // name
         SortSpec,                // specification
         SortValueMap<1, false>,  // value mapping - first argument
                                  // of sort order spec is 1
         Operator::SimpleSelect,  // trivial selection function
         SortTypeMap              // type mapping
);

/*
3.2 Operator ~sortby2~

This operator sorts a stream of tuples by a given list of attributes.
For each attribute it can be specified whether the list should be sorted
in ascending (asc) or descending (desc) order with regard to that attribute.

3.2.1 Type mapping function of operator ~sortby2~

Type mapping for ~sortby2~ is

---- ((stream (tuple ((x1 t1)...(xn tn)))) ((xi1 asc/desc) ... (xij asc/desc)))
             -> ((stream (tuple ((x1 t1)...(xn tn))))
                 APPEND (j i1 asc/desc i2 asc/desc ... ij asc/desc))
----

*/

ListExpr SortByTypeMap( ListExpr args )
{
  NList type(args);

  // check list length
  if ( !type.hasLength(2) )
  {
    return NList::typeError(
        "Operator sortby2 expects a list of "
        "length two.");
  }

  NList streamDesc = type.first();
  NList attrDesc = type.second();

  // check if first argument is a tuple stream
  NList attr;
  if ( !streamDesc.checkStreamTuple(attr) )
  {
    return NList::typeError(
        "Operator sortby2: first argument is not a tuple stream!"
        "Operator received: " + streamDesc.convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attr.listExpr()) )
  {
    return NList::typeError(
        "Operator sortby2: first argument does not "
        "contain a tuple description!"
        "Operator received: " + attr.convertToString() );
  }

  int numberOfSortAttrs = attrDesc.length();

  // check length of attribute specification
  if ( numberOfSortAttrs <= 0 )
  {
    return NList::typeError(
        "Operator sortby2: sort order specification "
        "list may not be empty!");
  }

  NList sortDesc;
  sortDesc.append(NList(numberOfSortAttrs));
  NList rest = attrDesc;

  // process attribute description
  while( !rest.isEmpty() )
  {
    // extract first element of attribute specification
    NList attrElem = rest.first();

    // cut off first element
    rest.rest();

    // attrElem may be an atom (no optional asc/desc specifier)
    // or a list of two (with asc/desc specifier)
    if ( !(attrElem.isAtom() || attrElem.length() == 2) )
    {
      return NList::typeError(
          "Operator sortby2 expects as second argument "
          "a list of (attrname [asc, desc])|attrname.");
    }

    string attrName;

    // handle the two different cases
    if ( attrElem.length() == 2 )
    {
      // check types of attrElem
      if ( !(attrElem.first().isAtom() &&
             attrElem.first().isSymbol() &&
             attrElem.second().isAtom() &&
             attrElem.second().isSymbol() ) )
      {
        return NList::typeError(
            "Operator sortby2 expects as second argument a list"
            " of (attrname [asc, desc])|attrname .\n"
            "Operator sortby2 gets a list '"
            + attrDesc.convertToString() + "'.");
      }
      attrName = attrElem.first().str();
    }
    else
    {
      // check type of atom in attrElem
      if ( !(attrElem.isSymbol()) )
      {
        return NList::typeError(
            "Operator sortby2 expects as second argument a list"
            " of (attrname [asc, desc])|attrname .\n"
            "Operator sortby2 gets a list '"
            + attrDesc.convertToString() + "'.");
      }
      attrName = attrElem.str();
    }

    // determine attribute index (1-based)
    ListExpr attrType;
    int j = FindAttribute(attr.listExpr(), attrName, attrType);

    // determine asc/desc specifier
    if (j > 0)
    {
      bool isAscending = true;

      if( attrElem.length() == 2 )
      {
         if ( !( attrElem.second().str() == "asc" ||
                 attrElem.second().str() == "desc" ) )
         {
           return NList::typeError(
               "Operator sortby2 sorting criteria must "
               "be asc or desc, not '"
               + attrElem.second().convertToString() + "'!");
         }
         isAscending = attrElem.second().str() == "asc" ? true : false;
      }

      sortDesc.append(NList(j));
      sortDesc.append(NList(isAscending, isAscending));
    }
    else
    {
      return NList::typeError(
          "Operator sortby2: attribute name '" + attrName +
          "' is not known.\nKnown Attribute(s): "
          + attr.convertToString());
    }
  }

  return NList(NList("APPEND"), sortDesc, streamDesc).listExpr();
}
/*

3.2.2 Specification of operator ~sortby2~

*/
const string SortBySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (tuple([a1:d1, ... ,an:dn])))"
                           " ((xi1 asc/desc) ... (xij [asc/desc]))) -> "
                           "(stream (tuple([a1:d1, ... ,an:dn])))</text--->"
                           "<text>_ sortby2 [list]</text--->"
                           "<text>Sorts input stream according to a list "
                           "of attributes ai1 ... aij. If no order is "
                           "specified, ascending is assumed.</text--->"
                           "<text>query employee feed sortby2[DeptNo asc] "
                           "consume</text--->"
                              ") )";

/*
3.2.3 Definition of operator ~sortby2~

*/
Operator extrelsortby (
         "sortby2",               // name
         SortBySpec,              // specification
         SortValueMap<2, false>,  // value mapping - first argument
                                  // of sort order spec is 2
         Operator::SimpleSelect,  // trivial selection function
         SortByTypeMap            // type mapping
);


/*
3.3 Operator ~sortmergejoin2~

This operator sorts two input streams and computes their equijoin.

3.3.1 Type mapping function of operator ~sortmergejoin2~

Type mapping for ~sortmergejoin2~ is

----  ((stream (tuple ((x1 t1) ... (xn tn))))
       (stream (tuple ((y1 d1) ... (ym dm)))) xi yj)
         -> (stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm))))
            APPEND (i j)
----

Type mapping for ~hybridhashjoin~ is

----  ((stream (tuple ((x1 t1) ... (xn tn))))
       (stream (tuple ((y1 d1) ... (ym dm)))) xi yj int)
         -> (stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm))))
            APPEND (i j)
----

*/

template<int n>
ListExpr JoinTypeMap( ListExpr args )
{
  NList type(args);

  const char* op[] = { "hybridhashjoin",
                       "hybridhashjoinP",
                       "sortmergejoin2" };

  const char* ex[] = { "five", "eight", "four" };

  int expected;

  switch(n)
  {
    case 0: expected = 5; break;
    case 1: expected = 8; break;
    case 2: expected = 4; break;
    default: assert(0); break;
  }

  // check list length
  if ( !type.hasLength(expected) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) +
        ": expects a list of length " + string(ex[n]) );
  }

  NList streamA = type.first();
  NList streamB = type.second();
  NList joinA = type.third();
  NList joinB = type.fourth();

  // check if type of first argument is a tuple stream
  NList attrA;
  if ( !streamA.checkStreamTuple(attrA) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": first argument is not "
        "a tuple stream!"
        "Operator received: " + streamA.convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attrA.listExpr()) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": first argument does "
        "not contain a tuple description!"
        "Operator received: " + attrA.convertToString() );
  }

  // check if type of second argument is a tuple stream
  NList attrB;
  if ( !streamB.checkStreamTuple(attrB) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": second argument is not "
        "a tuple stream!"
        "Operator received: " + streamB.convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attrB.listExpr()) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": second argument does no "
        "contain a tuple description!"
        "Operator received: " + attrB.convertToString() );
  }

  // check if attribute names are disjoint
  if ( !AttributesAreDisjoint(attrA.listExpr(), attrB.listExpr()) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": Attribute names of first "
        "and second argument "
        "list must be disjoint.\n Attribute names of first list are: '" +
        attrA.convertToString() +
        "'.\n Attribute names of second list are: '" +
        attrB.convertToString() + "'." );
  }

  // Concatenate tuple descriptions and create tuple stream
  // for joined relations
  NList attr = NList(ConcatLists(attrA.listExpr(), attrB.listExpr()));
  NList stream = NList(NList(STREAM, NList(NList(TUPLE), attr)));

  // check types of join attributes
  if ( !(joinA.isAtom() && joinA.isSymbol() &&
         joinB.isAtom() && joinB.isSymbol() ) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": Join attributes must be of "
        "type SymbolType!\n");
  }

  // check if join attribute of first stream is tuple description
  ListExpr attrTypeA;
  int attrAIndex = FindAttribute(attrA.listExpr(), joinA.str(), attrTypeA);
  if ( !(attrAIndex > 0) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": First join attribute '" +
         joinA.str() + "' is not in first argument list '" +
         attrA.convertToString() +"'.\n");
  }


  // check if join attribute of second stream is tuple description
  ListExpr attrTypeB;
  int attrBIndex = FindAttribute(attrB.listExpr(), joinB.str(), attrTypeB);
  if ( !(attrBIndex > 0) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) + ": Second join attribute '" +
         joinB.str() + "' is not in second argument list '" +
         attrB.convertToString() +"'.\n");
  }

  // check if types of the join attributes match
  if ( NList(attrTypeA) != NList(attrTypeB) )
  {
    return NList::typeError(
          "Operator " + string(op[n]) +
          ": Type of first join attribute "
          "is different from type of second join argument.\n");

  }

  if( n == 0 || n == 1 )
  {
    if ( type.fifth().str() != "int" )
    {
      return NList::typeError(
            "Operator " + string(op[n]) +
            ": Parameter 'number of buckets' "
            "must be of type int.\n");
    }
  }

  if( n == 1 )
  {
    if ( type.elem(6).str() != "int" )
    {
      return NList::typeError(
            "Operator " + string(op[n]) +
            ": Parameter 'number of partitions' "
            "must be of type int.\n");
    }

    if ( type.elem(7).str() != "int" )
    {
      return NList::typeError(
            "Operator " + string(op[n]) +
            ": Parameter 'maximum memory size' "
            "must be of type int.\n");
    }

    if ( type.elem(8).str() != "int" )
    {
      return NList::typeError(
            "Operator " + string(op[n]) +
            ": Parameter 'I/O buffer size' "
            "must be of type int.\n");
    }
  }

  return NList( NList("APPEND"), NList( NList(attrAIndex),
      NList(attrBIndex)), stream).listExpr();
}

/*
Some explicit instantiations in order to use them also
outside of the implementation file.

*/

template ListExpr
JoinTypeMap<0>(ListExpr args);

template ListExpr
JoinTypeMap<1>(ListExpr args);

template ListExpr
JoinTypeMap<2>(ListExpr args);

/*
3.3.2 Value mapping function of operator ~sortmergejoin2~

The argument vector ~args~ contains in the first slot ~args[0]~ the
stream and in ~args[2]~ the number of sort attributes. ~args[3]~
contains the index of the first sort attribute, ~args[4]~ a boolean
indicating whether the stream is sorted in ascending order with regard
to the sort first attribute. ~args[5]~ and ~args[6]~ contain these
values for the second sort attribute  and so on.

*/

int SortMergeJoinValueMap( Word* args, Word& result,
                             int message, Word& local, Supplier s);

/*
3.3.3 Specification of operator ~sortmergejoin2~

*/
const string SortMergeJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                                   "\"Meaning\" \"Example\" ) "
                                   "( <text>((stream (tuple ((x1 t1) ... "
                                   "(xn tn)))) (stream (tuple ((y1 d1) ..."
                                   " (ym dm)))) xi yj) -> (stream (tuple "
                                   "((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)"
                                   ")))</text--->"
                                   "<text>_ _ sortmergejoin2 [ _ , _ ]"
                                   "</text--->"
                                   "<text>Computes the equijoin of two "
                                   "streams using the new sort2 operator "
                                   "implementation.</text--->"
                                   "<text>query duplicates feed ten feed "
                                   "sortmergejoin2[no, nr] consume</text--->"
                                   ") )";

/*
3.3.4 Definition of operator ~sortmergejoin2~

*/
Operator extrelsortmergejoin2 (
         "sortmergejoin2",        // name
         SortMergeJoinSpec,       // specification
         SortMergeJoinValueMap,   // value mapping - first argument
                                  // of sort order spec is 1
         Operator::SimpleSelect,  // trivial selection function
         JoinTypeMap<2>           // type mapping
);

/*
3.4 Operator ~hybridhashjoin~

This operator sorts two input streams and computes their equijoin.

3.4.2 Value mapping function of operator ~hybridhashjoin~

The argument vector ~args~ contains in the first slot ~args[0]~ the
first stream and in ~args[1]~ the second stream. ~args[3]~
contains the index of the join attribute for the first stream,
~args[4]~ contains the index of the join attribute of the second stream.
~args[5]~ holds the number of buckets.

*/
template<bool param>
int HybridHashJoinValueMap( Word* args, Word& result,
                              int message, Word& local, Supplier s );

/*
3.1.3 Specification of operator ~hybridhashjoin~

*/
const string HybridHashJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                                   "\"Meaning\" \"Example\" ) "
                                   "( <text>((stream (tuple ((x1 t1) ... "
                                   "(xn tn)))) (stream (tuple ((y1 d1) ..."
                                   " (ym dm)))) xi yj b) -> (stream (tuple "
                                   "((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)"
                                   ")))</text--->"
                                   "<text>_ _ hybridhashjoin [ _ , _ , _]"
                                   "</text--->"
                                   "<text>Computes the equijoin of two "
                                   "streams using using the hybrid hash "
                                   "algorithm.</text--->"
                                   "<text>query duplicates feed ten feed "
                                   "hybridhashjoin[no, nr] consume</text--->"
                                   ") )";

/*
3.1.4 Definition of operator ~hybridhashjoin~

*/
Operator extrelhybridhashjoin(
         "hybridhashjoin",              // name
         HybridHashJoinSpec,            // specification
         HybridHashJoinValueMap<false>, // value mapping - first
                                        // argument of sort order spec is 1
         Operator::SimpleSelect,        // trivial selection function
         JoinTypeMap<0>                 // type mapping
);

/*
4 Test Operators

These operators were used to test the functionality of single classes
that are used within the operators of this algebra or for benchmarking.

4.1 Operator ~heapstl~

This operator tests the STL heap implementation.

4.1.1 Type mapping function of Operator ~heapstl~

Type mapping for operator ~heapstl~ is

----  (stream (tuple ((x1 t1)...(xn tn))))  -> int
----

This function is also the type mapping function for operators ~heapstd~,
~heapbup~, ~heapbup2~ and ~heapmdr~. The operator's name is passed as
template parameter ~op~.

*/
template<int n>
ListExpr HeapTypeMap(ListExpr args)
{
  NList type(args);

  const char* op[] = { "heapstl", "heapstd", "heapbup",
                       "heapbup2", "heapmdr", "tuplecomp" };

  // check list length
  if ( !type.hasLength(1) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) +
        " expects a list of length one.");
  }

  NList attr;
  if ( !type.first().checkStreamTuple(attr) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) +
        ": first argument is not a tuple stream!"
        "Operator received: " + type.first().convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attr.listExpr()) )
  {
    return NList::typeError(
        "Operator " + string(op[n]) +
        ": first argument does not contain a tuple description!"
        "Operator received: " + attr.convertToString() );
  }

  return nl->SymbolAtom("int");
}

/*
4.1.2 Value mapping function of operator ~heapstl~

*/

int HeapStlValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s);

/*
4.1.3 Specification of operator ~heapstl~

*/

const string HeapStlSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> int"
                            "</text--->"
                            "<text>_ heapstl</text--->"
                            "<text>Tests STL heap implementation. All tuples "
                            "from the stream are consumed and inserted into "
                            "the heap. The operator returns the total number "
                            "of tuple comparisons performed.</text--->"
                            "<text>query cities feed heapstl</text--->"
                            ") )";

/*
4.1.4 Definition of operator ~heapstl~

*/

Operator extrelheapstl (
         "heapstl",               // name
         HeapStlSpec,             // specification
         HeapStlValueMap,         // value mapping
         Operator::SimpleSelect,  // trivial selection function
         HeapTypeMap<1>   // type mapping
);

/*
4.2 Operator ~heapstd~

This operator tests the standard heap implementation.

4.2.1 Value mapping function of operator ~heapstd~

*/

int HeapStdValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s);

/*
4.2.2 Specification of operator ~heapstd~

*/
const string HeapStdSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> int"
                            "</text--->"
                            "<text>_ heapstd</text--->"
                            "<text>Tests alternative standard heap "
                            "implementation. All tuples from the stream "
                            "are consumed and inserted into the heap. "
                            "The operator returns the total number "
                            "of tuple comparisons performed.</text--->"
                            "<text>query cities feed heapstd</text--->"
                            ") )";

/*
4.2.3 Definition of operator ~heapstd~

*/
Operator extrelheapstd (
         "heapstd",               // name
         HeapStdSpec,             // specification
         HeapStdValueMap,         // value mapping
         Operator::SimpleSelect,  // trivial selection function
         HeapTypeMap<2>   // type mapping
);

/*
4.3 Operator ~heapbup~

This operator tests the bottom-up heap implementation.

4.3.1 Value mapping function of operator ~heapbup~

*/
int HeapBupValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s );

/*
4.3.2 Specification of operator ~heapbup~

*/
const string HeapBupSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> int"
                            "</text--->"
                            "<text>_ heapbup</text--->"
                            "<text>Tests alternative bottom-up heap "
                            "implementation. All tuples from the stream "
                            "are consumed and inserted into the heap. "
                            "The operator returns the total number "
                            "of tuple comparisons performed.</text--->"
                            "<text>query cities feed heapbup</text--->"
                            ") )";

/*
4.3.3 Definition of operator ~heapbup~

*/
Operator extrelheapbup (
         "heapbup",               // name
         HeapBupSpec,             // specification
         HeapBupValueMap,         // value mapping
         Operator::SimpleSelect,  // trivial selection function
         HeapTypeMap<3>   // type mapping
);

/*
4.4 Operator ~heapbup2~

This operator tests the bottom-up heap implementation.

4.4.1 Value mapping function of operator ~heapbup2~

*/
int HeapBup2ValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s );

/*
4.4.2 Specification of operator ~heapbup2~

*/
const string HeapBup2Spec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> int"
                            "</text--->"
                            "<text>_ heapbup2</text--->"
                            "<text>Tests improved bottom-up heap "
                            "implementation. All tuples from the stream "
                            "are consumed and inserted into the heap. "
                            "The operator returns the total number "
                            "of tuple comparisons performed.</text--->"
                            "<text>query cities feed heapbup2</text--->"
                            ") )";

/*
4.4.3 Definition of operator ~heapbup2~

*/
Operator extrelheapbup2 (
         "heapbup2",              // name
         HeapBup2Spec,            // specification
         HeapBup2ValueMap,        // value mapping
         Operator::SimpleSelect,  // trivial selection function
         HeapTypeMap<4>  // type mapping
);

/*
4.5 Operator ~heapmdr~

This operator tests the bottom-up heap implementation.

4.5.1 Value mapping function of operator ~heapmdr~

*/
int HeapMdrValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s );

/*
4.5.2 Specification of operator ~heapmdr~

*/
const string HeapMdrSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> int"
                            "</text--->"
                            "<text>_ heapmdr</text--->"
                            "<text>Tests alternative MDR heap heap "
                            "implementation. All tuples from the stream "
                            "are consumed and inserted into the heap. "
                            "The operator returns the total number "
                            "of tuple comparisons performed.</text--->"
                            "<text>query cities feed heapmdr</text--->"
                            ") )";

/*
4.5.3 Definition of operator ~heapmdr~

*/
Operator extrelheapmdr (
         "heapmdr",               // name
         HeapMdrSpec,             // specification
         HeapMdrValueMap,         // value mapping
         Operator::SimpleSelect,  // trivial selection function
         HeapTypeMap<5>   // type mapping
);

/*
4.5 Operator ~tuplecomp~

This operator performs tuple comparisons between the first tuple
of a stream and all following tuples of the stream including itself.

4.5.1 Value mapping function of operator ~tuplecomp~

*/
int TupleCompValueMap( Word* args, Word& result,
                         int message, Word& local, Supplier s );

/*
4.5.2 Specification of operator ~tuplecomp~

*/
const string TupleCompSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> int"
                            "</text--->"
                            "<text>_ tuplecomp</text--->"
                            "<text>Performs a number of tuple comparisons "
                            "equal to the number of tuples in the stream. "
                            "The first tuple of the stream is compared with"
                            "itself and all following tuples in the stream. "
                            "The total time for this operation may be used "
                            "to calculate the necessary CPU time for "
                            "one tupe conmparison.</text--->"
                            "<text>query cities feed tuplecomp</text--->"
                            ") )";

/*
4.5.3 Definition of operator ~tuplecomp~

*/
Operator extreltuplecomp (
         "tuplecomp",             // name
         TupleCompSpec,           // specification
         TupleCompValueMap,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         HeapTypeMap<6> // type mapping
);

/*
4.5 Operator ~tuplefile~


This operator stores a tuple stream into a temporary tuple file and
reads the tuples from this file again when requested from the its
successor.

4.5.1 Type mapping function of Operator ~tuplefile~

Type mapping for ~tuplefile~ is

----  (stream (tuple ((x1 t1)...(xn tn))))  ->
      (stream (tuple ((x1 t1)...(xn tn))))
----

*/
ListExpr TupleFileTypeMap(ListExpr args)
{
  NList type(args);

  // check list length
  if ( !type.hasLength(1) )
  {
    return NList::typeError(
        "Operator tuplefile expects a list of length one.");
  }

  NList attr;
  if ( !type.first().checkStreamTuple(attr) )
  {
    return NList::typeError(
        "Operator tuplefile: first argument is not a tuple stream!"
        "Operator received: " + type.first().convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attr.listExpr()) )
  {
    return NList::typeError(
        "Operator tuplefile: first argument does "
        "not contain a tuple description!"
        "Operator received: " + attr.convertToString() );
  }

  return type.first().listExpr();
}

/*
4.5.1 Value mapping function of operator ~tuplefile~

*/
int TupleFileValueMap( Word* args, Word& result,
                        int message, Word& local, Supplier s );

/*
4.5.2 Specification of operator ~tuplefile~

*/
const string TupleFileSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> ((stream (tuple([a1:d1, ... ,an:dn]))))"
                            "</text--->"
                            "<text>_ tuplefile</text--->"
                            "<text>Stores a stream temporarily in a tuple "
                            "file and restores the tuples from file when "
                            "they are requested by the next operator."
                            "</text---><text>query cities feed "
                            "tuplefile consume</text--->) )";

/*
4.5.3 Definition of operator ~tuplefile~

*/
Operator tuplefiletest (
         "tuplefile",             // name
         TupleFileSpec,           // specification
         TupleFileValueMap,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         TupleFileTypeMap         // type mapping
);

/*
4.6 Operator ~tuplebuffer~

This operator stores a tuple stream into a tuple buffer and
reads the tuples from the buffer again when requested from the its
successor.

4.6.1 Type mapping function of Operator ~tuplebuffer~

Type mapping for ~tuplebuffer~ is

----  ((stream (tuple ((x1 t1)...(xn tn)))) int)  ->
       (stream (tuple ((x1 t1)...(xn tn))))
----

*/
ListExpr TupleBufferTypeMap(ListExpr args)
{
  NList type(args);

  // check list length
  if ( !type.hasLength(2) )
  {
    return NList::typeError(
        "Operator tuplebuffer expects a list of length two.");
  }

  NList attr;
  if ( !type.first().checkStreamTuple(attr) )
  {
    return NList::typeError(
        "Operator tuplebuffer: first argument is not a tuple stream!"
        "Operator received: " + type.first().convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attr.listExpr()) )
  {
    return NList::typeError(
        "Operator tuplebuffer: first argument does not "
        "contain a tuple description!"
        "Operator received: " + attr.convertToString() );
  }

  if ( type.second() != INT )
  {
    return NList::typeError(
        "Operator tuplebuffer: second argument must be an integer!"
        "Operator received: " + type.second().convertToString() );
  }

  return type.first().listExpr();
}

/*
4.6.2 Value mapping function of operator ~tuplebuffer~

*/
int TupleBufferValueMap( Word* args, Word& result,
                           int message, Word& local, Supplier s);

/*
4.6.3 Specification of operator ~tuplebuffer~

*/
const string TupleBufferSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            "))) int) -> (stream (tuple([a1:d1, ... ,an:dn])))"
                            "</text--->"
                            "<text>_ tuplebuffer[_]</text--->"
                            "<text>Stores a stream temporarily in a tuple "
                            "buffer and restores the tuples when they are "
                            "requested by the next operator. The in-memory "
                            "buffer size is specified in KBytes as an "
                            "additional argument."
                            "</text---><text>query cities feed "
                            "tuplebuffer[256] consume</text--->) )";

/*
4.6.4 Definition of operator ~tuplebuffer~

*/
Operator tuplebuffertest (
         "tuplebuffer",           // name
         TupleBufferSpec,         // specification
         TupleBufferValueMap,     // value mapping
         Operator::SimpleSelect,  // trivial selection function
         TupleBufferTypeMap       // type mapping
);

/*
4.7 Operator ~sort2With~

This operator is used to simplify the testing of the new
~sort2~ operator implementation. The operator takes three additional
parameters, the second argument specifies the used operators main
memory in bytes, the third argument the maximum fan-in of a merge
phase respectively the maximum number of temporary open tuple
files and the fourth argument specifies the size of the
I/O buffer for read/write operations on disc.

4.7.1 Type mapping function of Operator ~sort2With~

Type mapping for operator ~sort2With~ is

----  ((stream (tuple ((x1 t1)...(xn tn)))) int int int)  ->
      (stream (tuple ((x1 t1)...(xn tn))) int int int
      APPEND (n i1 true i2 true ... in true))
----

*/
ListExpr Sort2WithTypeMap(ListExpr args)
{
  NList type(args);

  // check list length
  if ( !type.hasLength(4) )
  {
    return NList::typeError(
        "Operator sort2With expects a list of length four.");
  }

  // check if first argument is a tuple stream
  NList attr;
  if ( !type.first().checkStreamTuple(attr) )
  {
    return NList::typeError(
        "Operator sort2With: first argument is not a tuple stream!"
        "Operator received: " + type.first().convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attr.listExpr()) )
  {
    return NList::typeError(
        "Operator sort2with: first argument does not "
        "contain a tuple description!"
        "Operator received: " + attr.convertToString() );
  }

  // create SortOrderSpecification as nested list
  // (n i1 asc i2 asc ... in asc)
  // n: number of tuple attributes
  // i1: index of first attribute (1-based)
  // i2: index of second attribute and so on (1-based)
  // ...
  // asc: boolean value true (ascending sort order = true)
  NList tupleDesc = type.first().second().second();
  int n = tupleDesc.length();

  NList sortDesc;
  sortDesc.append(NList(n));

  for(int i = 1; i <= n; i++)
  {
    sortDesc.append(NList(i));
    sortDesc.append(NList(true,true));
  }

  // check if second argument is an integer (operator memory)
  if ( type.second() != INT )
  {
    return NList::typeError(
        "Operator sort2With: second argument (operator memory) "
        "must be an integer!"
        "Operator received: " + type.second().convertToString() );
  }

  // check if third argument is an integer (maximum fan-in merge-phase)
  if ( type.third() != INT )
  {
    return NList::typeError(
        "Operator sort2With: third argument (maximum fan-in merge-phase)"
        " must be an integer!"
        "Operator received: " + type.third().convertToString() );
  }

  // check if fourth argument is an integer (I/O buffer size)
  if ( type.fourth() != INT )
  {
    return NList::typeError(
        "Operator sort2With: fourth argument ('I/O buffer size') "
        "must be an integer!"
        "Operator received: " + type.fourth().convertToString() );
  }

  return NList(NList(APPEND), sortDesc, type.first()).listExpr();
}

/*
4.7.2 Value mapping function of operator ~sort2With~

*/

int Sort2WithValueMap( Word* args, Word& result,
                        int message, Word& local, Supplier s );

/*
4.7.3 Specification of operator ~sort2With~

*/

const string Sort2WithSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            "))) int int int) -> "
                            "(stream (tuple([a1:d1, ... ,an:dn])))"
                            "</text--->"
                            "<text>_ sort2With[_, _, _]</text--->"
                            "<text>This operator is used to simplify the "
                            "testing of "
                            "the new sort2 operator implementation. The "
                            "operator takes two additional parameters, "
                            "the first one specifies the usable main "
                            "memory in KBytes and the second one the maximum "
                            "fan-in of a merge phase respectively the maximum "
                            "number of temporary open tuple files. Usable "
                            "memory size must be between 1-65536 KByte. "
                            "The maximum fan-in is limited by 2-1000. If "
                            "these limits are exceeded default values"
                            "will be used instead.</text--->"
                            "<text>query plz feed sort2With[16386, 50]"
                            " consume</text--->) )";

/*
4.7.4 Definition of operator ~sort2With~

*/
Operator extrelsort2with(
         "sort2with",             // name
         Sort2WithSpec,           // specification
         SortValueMap<4, true>,   // value mapping
         Operator::SimpleSelect,  // trivial selection function
         Sort2WithTypeMap         // type mapping
);

/*
4.8 Operator ~sortby2With~

This operator is used to simplify the testing of the new
~sortby2~ operator implementation. The operator takes three additional
parameters, the first one specifies the useable main memory
memory in Bytes, the second one the maximum fan-in of a merge
phase respectively the maximum number of temporary open tuple
files and the third one specifies the I/O buffer size in bytes
for read/write operations on disc.

4.8.1 Type mapping function of Operator ~sortby2with~

Type mapping for ~sortby2with~ is

----  ((stream (tuple ((x1 t1)...(xn tn))))
                      ((xi1 asc/desc) ... (xij asc/desc)) int int int)
              -> ((stream (tuple ((x1 t1)...(xn tn)))) int int int
                  APPEND (j i1 asc/desc i2 asc/desc ... ij asc/desc))
----

*/

ListExpr SortBy2WithTypeMap( ListExpr args )
{
  NList type(args);

  // check list length
  if ( !type.hasLength(5) )
  {
    return NList::typeError(
        "Operator sortby2with expects a list of "
        "length five.");
  }

  NList streamDesc = type.first();
  NList attrDesc = type.second();

  // check if first argument is a tuple stream
  NList attr;
  if ( !streamDesc.checkStreamTuple(attr) )
  {
    return NList::typeError(
        "Operator sortby2with: first argument is not a tuple stream!"
        "Operator received: " + streamDesc.convertToString() );
  }

  // check if there is a valid tuple description
  if ( !IsTupleDescription(attr.listExpr()) )
  {
    return NList::typeError(
        "Operator sortby2with: first argument does not "
        "contain a tuple description!"
        "Operator received: " + attr.convertToString() );
  }

  int numberOfSortAttrs = attrDesc.length();

  // check length of attribute specification
  if ( numberOfSortAttrs <= 0 )
  {
    return NList::typeError(
        "Operator sortby2with: sort order specification "
        "list may not be empty!");
  }

  NList sortDesc;
  sortDesc.append(NList(numberOfSortAttrs));
  NList rest = attrDesc;

  // process attribute description
  while( !rest.isEmpty() )
  {
    // extract first element of attribute specification
    NList attrElem = rest.first();

    // cut off first element
    rest.rest();

    // attrElem may be an atom (no optional asc/desc specifier)
    // or a list of two (with asc/desc specifier)
    if ( !(attrElem.isAtom() || attrElem.length() == 2) )
    {
      return NList::typeError(
          "Operator sortby2with expects as second argument "
          "a list of (attrname [asc, desc])|attrname.");
    }

    string attrName;

    // handle the two different cases
    if ( attrElem.length() == 2 )
    {
      // check types of attrElem
      if ( !(attrElem.first().isAtom() &&
             attrElem.first().isSymbol() &&
             attrElem.second().isAtom() &&
             attrElem.second().isSymbol() ) )
      {
        return NList::typeError(
            "Operator sortby2with expects as second argument a list"
            " of (attrname [asc, desc])|attrname .\n"
            "Operator sortby2with gets a list '"
            + attrDesc.convertToString() + "'.");
      }
      attrName = attrElem.first().str();
    }
    else
    {
      // check type of atom in attrElem
      if ( !(attrElem.isSymbol()) )
      {
        return NList::typeError(
            "Operator sortby2with expects as second argument a list"
            " of (attrname [asc, desc])|attrname .\n"
            "Operator sortby2with gets a list '"
            + attrDesc.convertToString() + "'.");
      }
      attrName = attrElem.str();
    }

    // determine attribute index (1-based)
    ListExpr attrType;
    int j = FindAttribute(streamDesc.second().second().listExpr(),
                          attrName, attrType);

    // determine asc/desc specifier
    if (j > 0)
    {
      bool isAscending = true;

      if( attrElem.length() == 2 )
      {
         if ( !( attrElem.second().str() == "asc" ||
                 attrElem.second().str() == "desc" ) )
         {
           return NList::typeError(
               "Operator sortby2with sorting criteria must "
               "be asc or desc, not '"
               + attrElem.second().str() + "'!");
         }
         isAscending = attrElem.second().str() == "asc" ? true : false;
      }

      sortDesc.append(NList(j));
      sortDesc.append(NList(isAscending, isAscending));
    }
    else
    {
      return NList::typeError(
          "Operator sortby2with: attribute name '" + attrName +
          "' is not known.\nKnown Attribute(s): "
          + streamDesc.second().second().convertToString());
    }
  }

  // check if third argument is an integer (operator memory)
  if ( type.third() != INT )
  {
    return NList::typeError(
        "Operator sortby2with: third argument (operator memory) "
        "must be an integer!"
        "Operator received: " + type.third().convertToString() );
  }

  // check if fourth argument is an integer (maximum fan-in merge-phase)
  if ( type.fourth() != INT )
  {
    return NList::typeError(
        "Operator sortby2with: fourth argument (maximum fan-in merge-phase) "
        "must be an integer!"
        "Operator received: " + type.fourth().convertToString() );
  }

  // check if fifth argument is an integer (I/O buffer size)
  if ( type.fifth() != INT )
  {
    return NList::typeError(
        "Operator sortby2with: fifth argument ('I/O buffer size') "
        "must be an integer!"
        "Operator received: " + type.fifth().convertToString() );
  }

  return NList(NList("APPEND"), sortDesc, type.first()).listExpr();
}

/*
4.8.2 Value mapping function of operator ~sortby2with~

*/

int SortBy2WithValueMap( Word* args, Word& result,
                           int message, Word& local, Supplier s );

/*
4.8.3 Specification of operator ~sortby2with~

*/

const string SortBy2WithSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            "))) int int) -> "
                            "(stream (tuple([a1:d1, ... ,an:dn])))"
                            "</text--->"
                            "<text>_ sort2With[list; _, _, _]</text--->"
                            "<text>This operator is used to simplify the "
                            "testing of the new sortby2 operator "
                            "implementation. The operator takes three "
                            "additional parameters. The first one specifies "
                            "the usable main memory in bytes, the second "
                            "one the maximum fan-in of a merge phase "
                            "respectively the maximum number of temporary "
                            "open tuple files and the third one the size of"
                            "the I/O buffer in bytes. Usable memory size "
                            "must be between 1-65536 KByte. The maximum "
                            "fan-in is limited by 2-1000. The size of the"
                            "I/O buffer is limited by 0-16384 bytes."
                            "If these limits are exceeded "
                            "default values will be used instead.</text--->"
                            "<text>query plz feed sortby2With"
                            "[Ort asc, PLZ asc; 16386, 50, 4096] "
                            "consume</text--->) )";

/*
4.8.4 Definition of operator ~sortby2with~

*/

Operator extrelsortby2with(
         "sortby2with",           // name
         SortBy2WithSpec,         // specification
         SortValueMap<5, true>,   // value mapping
         Operator::SimpleSelect,  // trivial selection function
         SortBy2WithTypeMap       // type mapping
);

/*
4.9 Operator ~hybridhashjoinP~

This operator computes the equijoin of two stream. This is
a full parameter-driven version of the ~hybridhashjoin~
operator. In addition to the number of buckets the user
may specify the number of partitions, the usable main memory
and the I/O buffer size.

4.9.1 Specification of operator ~hybridhashjoinP~

*/
const string HybridHashJoinPSpec  = "( ( \"Signature\" \"Syntax\" "
                                   "\"Meaning\" \"Example\" ) "
                                   "( <text>((stream (tuple ((x1 t1) ... "
                                   "(xn tn)))) (stream (tuple ((y1 d1) ..."
                                   " (ym dm)))) xi yj b p mem io) -> "
                                   "(stream (tuple "
                                   "((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)"
                                   ")))</text--->"
                                   "<text>_ _ hybridhashjoinP "
                                   "[ _ , _ , _, _, _, _]"
                                   "</text--->"
                                   "<text>Computes the equijoin of two "
                                   "streams using the hybrid hash algorithm. "
                                   "This is the fully parameter-driven version "
                                   "of the hybrid hash-join operator. This "
                                   "operator provides three additional "
                                   "attributes p, mem, io. p specifies the "
                                   "number of partitions to use "
                                   "(with 2 <= p <= b/2). Paramter mem is "
                                   "used to specify the amount of usable main "
                                   "memory for the operator in bytes. "
                                   "Parameter io specifies the I/O buffer "
                                   "size in bytes used for read/write "
                                   "operations to/from disk. This operator is "
                                   "used to test the operator under different "
                                   "conditions.</text--->"
                                   "<text>query duplicates feed ten feed "
                                   "hybridhashjoin[no, nr, 1000, 50, "
                                   "16*1024*1024, 4096] consume</text--->"
                                   ") )";

/*
4.9.2 Definition of operator ~hybridhashjoinP~

*/
Operator extrelhybridhashjoinP(
         "hybridhashjoinP",             // name
         HybridHashJoinPSpec,           // specification
         HybridHashJoinValueMap<true>,  // value mapping
         Operator::SimpleSelect,        // trivial selection function
         JoinTypeMap<1>                 // type mapping
);

} // end of namespace extrel2

/*

5 Class ~ExtRelation2Algebra~

A new subclass ~ExtRelation2Algebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the
actual algebra.

After declaring the new class, its only instance ~extendedRelation2Algebra~
is defined.

*/


class ExtRelation2Algebra : public Algebra
{
 public:
   ExtRelation2Algebra() : Algebra()
  {
    AddOperator(&extrel2::extrelsortby);
    AddOperator(&extrel2::extrelsort);
    AddOperator(&extrel2::extrelsortmergejoin2);
    AddOperator(&extrel2::extrelhybridhashjoin);

    AddOperator(&extrel2::extrelhybridhashjoinP);
    AddOperator(&extrel2::extrelsort2with);
    AddOperator(&extrel2::extrelsortby2with);
    AddOperator(&extrel2::tuplefiletest);
    AddOperator(&extrel2::tuplebuffertest);
    AddOperator(&extrel2::extrelheapstl);
    AddOperator(&extrel2::extrelheapstd);
    AddOperator(&extrel2::extrelheapbup);
    AddOperator(&extrel2::extrelheapbup2);
    AddOperator(&extrel2::extrelheapmdr);
    AddOperator(&extrel2::extreltuplecomp);

#ifdef USE_PROGRESS
// support for progress queries
   extrel2::extrelsortby.EnableProgress();
   extrel2::extrelsort.EnableProgress();
   extrel2::extrelsort2with.EnableProgress();
   extrel2::extrelsortby2with.EnableProgress();
   extrel2::extrelhybridhashjoin.EnableProgress();
   extrel2::extrelhybridhashjoinP.EnableProgress();
   extrel2::extrelsortmergejoin2.EnableProgress();
#endif
  }

  ~ExtRelation2Algebra() {};
};

/*

6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeExtRelation2Algebra( NestedList* nlRef,
                               QueryProcessor* qpRef,
                               AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new ExtRelation2Algebra());
}

