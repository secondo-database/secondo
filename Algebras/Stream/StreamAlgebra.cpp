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
//[ue] [\"u]
//[ae] [\"a]

//[TOC] [\tableofcontents]

[1] StreamAlgebra - Implementing Generalized (non-tuple) Streams of Objects

December 2006, Initial version implemented by Christian D[ue]ntgen,
Faculty for Mathematics and Informatics,
LG Database Systems for New Applications
Feruniversit[ae]t in Hagen.

----

State Operator/Signatures


OK    use:   (stream X)            (map X Y)            --> (stream Y)
OK           (stream X)            (map X (stream Y))   --> (stream Y)

OK    use2:  (stream X) Y          (map X Y Z)          --> (stream Z)
OK           (stream X) Y          (map X Y stream(Z))  --> (stream Z)
OK           X          (stream Y) (map X y Z)          --> (stream Z)
OK           X          (stream Y) (map X y (stream Z)) --> (stream Z)
OK           (stream X) (stream Y) (map X Y Z)          --> (stream Z)
OK           (stream X) (stream Y) (map X Y (stream Z)) --> (stream Z)
             for X,Y,Z of kind DATA

OK    feed:                           T --> (stream T)

OK    transformstream: stream(tuple((id T))) --> (stream T)
OK                                (stream T) --> stream(tuple((element T)))
OK    aggregateS:        (stream T) x (T x T --> T) x T  --> T
OK    count:                      (stream T) --> int
OK    filter:      ((stream T) (map T bool)) --> int
OK    printstream:                (stream T) --> (stream T)

COMMENTS:

(*):  These operators have been implemented for
      T in {bool, int, real, point}
(**): These operators have been implemented for
      T in {bool, int, real, point, string, region}

Key to STATE of implementation:

   OK : Operator has been implemented and fully tested
  (OK): Operator has been implemented and partially tested
  Test: Operator has been implemented, but tests have not been done
  Pre : Operator has not been functionally implemented, but
        stubs (dummy code) exist
  n/a : Neither functionally nor dummy code exists for this ones

    + : Equivalent exists for according mType
    - : Does nor exist for according mType
    ? : It is unclear, whether it exists or not

----

*/


/*
0. Bug-List

----

 (none known)

Key:
 (C): system crash
 (R): Wrong result

----

*/

/*

[TOC]

1 Overview

This file contains the implementation of the stream operators.

2 Defines, includes, and constants

*/


#include <cmath>
#include <stack>
#include <limits>
#include <sstream>
#include <vector>

#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

bool GSA_DEBUG = false; // Set to true to activate debugging code
//bool GSA_DEBUG = true; // Set to true to activate debugging code

/*
4 General Selection functions

*/

/*
5 Implementation of Algebra Operators

*/

/*
5.19 Operator ~feed~

The operator is used to cast a single value T to a (stream T)
having a single element of type T.

5.19.1 Type Mapping for ~feed~

*/
ListExpr
TypeMapStreamfeed( ListExpr args )
{
  ListExpr arg1;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if ( ( nl->ListLength(args) == 1 ) && ( nl->IsAtom(nl->First(args) ) ) )
    {
      arg1 = nl->First(args);
      if( am->CheckKind("DATA", arg1, errorInfo) )
        return nl->TwoElemList(nl->SymbolAtom("stream"), arg1);
    }
  ErrorReporter::ReportError("Operator feed  expects a list of length one, "
                             "containing a value of one type 'T' with T in "
                             "kind DATA.");
  return nl->SymbolAtom( "typeerror" );
}

/*
5.19.2 Value Mapping for ~feed~

*/
struct SFeedLocalInfo
{
  bool finished;
};

int MappingStreamFeed( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  SFeedLocalInfo *linfo;
  Word argValue;

  switch( message )
    {
    case OPEN:
      linfo = new SFeedLocalInfo;
      linfo->finished = false;
      local = SetWord(linfo);
      return 0;

    case REQUEST:
      if ( local.addr == 0 )
        return CANCEL;
      linfo = ( SFeedLocalInfo *)local.addr;
      if ( linfo->finished )
        return CANCEL;
      argValue = args[0];
      result = SetWord(((Attribute*) (argValue.addr))->Clone());
      linfo->finished = true;
      return YIELD;

    case CLOSE:
      if ( local.addr == 0 )
        {
          linfo = ( SFeedLocalInfo*) local.addr;
          delete linfo;
        }
      return 0;
    }
  return -1; // should not be reached
}

/*
5.19.3 Specification for operator ~feed~

*/
const string
StreamSpecfeed=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in kind DATA:\n"
"T -> (stream T)</text--->"
"<text>_ feed</text--->"
"<text>create a single-value stream from "
"a single value.</text--->"
"<text>query [const int value 5] feed count;</text---> ) )";

/*
5.19.4 Selection Function of operator ~feed~

*/

ValueMapping streamfeedmap[] = { MappingStreamFeed };

int StreamfeedSelect( ListExpr args )
{
  return 0;
}

/*
5.19.5  Definition of operator ~feed~

*/
Operator streamfeed( "feed",
                     StreamSpecfeed,
                     1,
                     streamfeedmap,
                     StreamfeedSelect,
                     TypeMapStreamfeed);


/*
5.20 Operator ~use~

The ~use~ class of operators implements a set of functors, that derive
stream-valued operators from operators taking scalar arguments and returning
scalar values or streams of values:

----

     use: (stream X)            (map X Y)            -> (stream Y)
           (stream X)            (map X (stream Y))   -> (stream Y)
           (stream X) Y          (map X Y Z)          -> (stream Z)
           (stream X) Y          (map X Y stream(Z))  -> (stream Z)
           X          (stream Y) (map X y Z)          -> (stream Z)
           X          (stream Y) (map X y (stream Z)) -> (stream Z)
           (stream X) (stream Y) (map X Y Z)          -> (stream Z)
           (stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)
           for X,Y,Z of kind DATA

----

5.20.1 Type Mapping for ~use~

*/

ListExpr
TypeMapUse( ListExpr args )
{
  string outstr1, outstr2;            // output strings
  ListExpr errorInfo;
  ListExpr sarg1, map;                // arguments to use
  ListExpr marg1, mres;               // argument to mapping
  ListExpr sarg1Type, sresType;       // 'flat' arg type

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if (GSA_DEBUG) cout << "\nTypeMapUse: 0" << endl;

  if ( (nl->ListLength( args ) != 2) )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 1" << endl;
      ErrorReporter::ReportError("Operator use expects a list of length two ");
      return nl->SymbolAtom( "typeerror" );
    }
  if (GSA_DEBUG) cout << "TypeMapUse: 2" << endl;

  // get use arguments
  sarg1 = nl->First( args );
  map = nl->Second( args );

  // check sarg1 for being a stream
  if(     nl->IsAtom( sarg1 )
          || ( nl->ListLength( sarg1 ) != 2)
          || !(TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 3" << endl;

      ErrorReporter::ReportError(
        "Operator use expects its first Argument to "
        "be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );
    }
  sarg1Type = nl->Second(sarg1);
  if (GSA_DEBUG) cout << "TypeMapUse: 4" << endl;

  // check sarg1 to be a (stream T) for T in kind DATA
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg1Type )
         && !am->CheckKind("DATA", nl->Second( sarg1Type ), errorInfo) )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 5" << endl;
      nl->WriteToString(outstr1, sarg1Type);
      ErrorReporter::ReportError("Operator use expects its 1st argument "
                                 "to be '(stream T)', T of kind DATA, but"
                                 "receives '" + outstr1 + "' as T.");
      return nl->SymbolAtom( "typeerror" );
    }
  if (GSA_DEBUG) cout << "TypeMapUse: 6" << endl;

  // This check can be removed when operators working on tuplestreams have
  // been implemented:
  if ( !nl->IsAtom( sarg1Type ) &&
       (nl->ListLength( sarg1Type ) == 2) &&
       nl->IsEqual( nl->First(sarg1Type), "tuple") )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 7" << endl;
      ErrorReporter::ReportError("Operator use still not implemented for "
                                 "arguments of type 'tuple(X)' or "
                                 "'(stream tuple(X))'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if (GSA_DEBUG) cout << "TypeMapUse: 8" << endl;
  if ( !nl->IsAtom( sarg1Type ) &&
       ( (nl->ListLength( sarg1Type ) != 2) ||
         !nl->IsEqual( nl->First(sarg1Type), "tuple") ||
         !IsTupleDescription(nl->Second(sarg1Type))
         )
       )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 9" << endl;
      nl->WriteToString(outstr1, sarg1);
      return nl->SymbolAtom( "typeerror" );
    }
  if (GSA_DEBUG) cout << "TypeMapUse: 10" << endl;

  // check for map
  if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map), "map") ) )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 11" << endl;
      nl->WriteToString(outstr1, map);
      ErrorReporter::ReportError("Operator use expects a map as "
                                 "2nd argument, but gets '" + outstr1 +
                                 "' instead.");
      return nl->SymbolAtom( "typeerror" );
    }
  if (GSA_DEBUG) cout << "TypeMapUse: 12" << endl;

  if ( nl->ListLength(map) != 3 )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 13" << endl;
      ErrorReporter::ReportError("Number of map arguments must be 1 "
                                 "for operator use.");
      return nl->SymbolAtom( "typeerror" );
    }

  if (GSA_DEBUG) cout << "TypeMapUse: 14" << endl;
  // get map arguments
  marg1 = nl->Second(map);
  mres  = nl->Third(map);

  // check marg1

  if ( !( nl->Equal(marg1, sarg1Type) ) )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 15" << endl;
      nl->WriteToString(outstr1, sarg1Type);
      nl->WriteToString(outstr2, marg1);
      ErrorReporter::ReportError("Operator use: 1st argument's stream"
                                 "type does not match the type of the "
                                 "mapping's 1st argument. If e.g. the first "
                                 "is 'stream X', then the latter must be 'X'."
                                 "The types passed are '" + outstr1 +
                                 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }
  if (GSA_DEBUG) cout << "TypeMapUse: 16" << endl;

  // get map result type 'sresType'
  if( !( nl->IsAtom( mres ) ) && ( nl->ListLength( mres ) == 2) )
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 17" << endl;

      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
        {
          if (GSA_DEBUG) cout << "TypeMapUse: 18" << endl;
          if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) )
            {
              if (GSA_DEBUG) cout << "TypeMapUse: 19" << endl;

              ErrorReporter::ReportError(
                "Operator use expects its 2nd Argument to "
                "return a '(stream T)', T of kind DATA'.");
              return nl->SymbolAtom( "typeerror" );
            }
          if (GSA_DEBUG) cout << "TypeMapUse: 20" << endl;

          sresType = mres; // map result type is already a stream
          nl->WriteToString(outstr1, sresType);
          if (GSA_DEBUG) cout << "\nTypeMapUse Resulttype (1): "
                              << outstr1 << "\n";
          return sresType;
        }
      if (GSA_DEBUG) cout << "TypeMapUse: 21" << endl;

    }
  else // map result type is not a stream, so encapsulate it
    {
      if (GSA_DEBUG) cout << "TypeMapUse: 22" << endl;

      if ( !am->CheckKind("DATA", mres, errorInfo) )
        {
          if (GSA_DEBUG) cout << "TypeMapUse: 23" << endl;

          ErrorReporter::ReportError(
            "Operator use expects its 2nd Argument to "
            "return a type of kind DATA.");
          return nl->SymbolAtom( "typeerror" );
        }
      if (GSA_DEBUG) cout << "TypeMapUse: 24" << endl;

      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);
      nl->WriteToString(outstr1, sresType);
      if (GSA_DEBUG) cout << "\nTypeMapUse Resulttype (2): "
                          << outstr1 << "\n";
      return sresType;
    }
  if (GSA_DEBUG) cout << "TypeMapUse: 25" << endl;

  // otherwise (some unmatched error)
  return nl->SymbolAtom( "typeerror" );
}


ListExpr
TypeMapUse2( ListExpr args )
{
  string   outstr1, outstr2;               // output strings
  ListExpr errorInfo;
  ListExpr sarg1, sarg2, map;              // arguments to use
  ListExpr marg1, marg2, mres;             // argument to mapping
  ListExpr sarg1Type, sarg2Type, sresType; // 'flat' arg type
  ListExpr argConfDescriptor;
  bool
    sarg1isstream = false,
    sarg2isstream = false,
    resisstream   = false;
  int argConfCode = 0;

  if (GSA_DEBUG) cout << "\nTypeMapUse2: 0" << endl;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  // 0. Check number of arguments
  if ( (nl->ListLength( args ) != 3) )
    {
      ErrorReporter::ReportError("Operator use2 expects a list of "
                                 "length three ");
      return nl->SymbolAtom( "typeerror" );
    }

  if (GSA_DEBUG) cout << "TypeMapUse2: 1" << endl;

  // 1. get use arguments
  sarg1 = nl->First( args );
  sarg2 = nl->Second( args );
  map   = nl->Third( args );

  if (GSA_DEBUG) cout << "TypeMapUse2: 2" << endl;

  // 2. First argument
  // check sarg1 for being a stream
  if( nl->IsAtom( sarg1 )
      || !(TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    { // non-stream datatype
      sarg1Type = sarg1;
      sarg1isstream = false;
    }
  else if ( !nl->IsAtom( sarg1 )
            && ( nl->ListLength( sarg1 ) == 2)
            && (TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    { // (stream datatype)
      sarg1Type = nl->Second(sarg1);
      sarg1isstream = true;
    }
  else // wrong type for sarg1
    {
      ErrorReporter::ReportError(
                                 "Operator use2 expects its first Argument to "
        "be of type 'T' or '(stream T).");
      return nl->SymbolAtom( "typeerror" );
    }

  // check sarg1 to be a (stream T) for T in kind DATA
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg1Type )
      && !am->CheckKind("DATA", nl->Second( sarg1Type ), errorInfo) )
    { // T is not of kind DATA
      nl->WriteToString(outstr1, sarg1Type);
      ErrorReporter::ReportError("Operator use2 expects its 1st argument "
                                 "to be '(stream T)', T of kind DATA, but"
                                 "receives '" + outstr1 + "' as T.");

      return nl->SymbolAtom( "typeerror" );
    }
  else if ( !nl->IsAtom( sarg1Type ) &&
            ( (nl->ListLength( sarg1Type ) != 2) ||
              !nl->IsEqual( nl->First(sarg1Type), "tuple") ||
              !IsTupleDescription(nl->Second(sarg1Type))
              )
            )
    { // neither T is tuple(X)
      nl->WriteToString(outstr1, sarg1Type);
      ErrorReporter::ReportError("Operator use2 expects its 1st argument "
                                 "to be 'T' or '(stream T), T of kind DATA "
                                 "or of type 'tuple(X))', but"
                                 "receives '" + outstr1 + "' for T.");
      return nl->SymbolAtom( "typeerror" );
    }

  if (GSA_DEBUG) cout << "TypeMapUse2: 3" << endl;

  // 3. Second Argument
  // check sarg2 for being a stream
  if( nl->IsAtom( sarg2 )
      || !(TypeOfRelAlgSymbol(nl->First(sarg2) == stream )) )
    { // non-stream datatype
      sarg2Type = sarg2;
      sarg2isstream = false;
    }
  else if ( !nl->IsAtom( sarg2 )
            && ( nl->ListLength( sarg2 ) == 2)
            && (TypeOfRelAlgSymbol(nl->First(sarg2) == stream )) )
    { // (stream datatype)
      sarg2Type = nl->Second(sarg2);
      sarg2isstream = true;
    }
  else // wrong type for sarg2
    {
      ErrorReporter::ReportError(
        "Operator use2 expects its second Argument to "
        "be of type 'T' or '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );
    }

  // check sarg2 to be a (stream T) for T in kind DATA
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg2Type )
         && !am->CheckKind("DATA", nl->Second( sarg2Type ), errorInfo) )
    { // T is not of kind DATA
      nl->WriteToString(outstr2, sarg2Type);
      ErrorReporter::ReportError("Operator use2 expects its 2nd argument "
                                 "to be '(stream T)', T of kind DATA, but"
                                 "receives '" + outstr1 + "' as T.");

      return nl->SymbolAtom( "typeerror" );
    }
  else if ( !nl->IsAtom( sarg2Type ) &&
            ( (nl->ListLength( sarg2Type ) != 2) ||
              !nl->IsEqual( nl->First(sarg2Type), "tuple") ||
              !IsTupleDescription(nl->Second(sarg2Type))
              )
            )
    { // neither T is tuple(X)
      nl->WriteToString(outstr1, sarg2Type);
      ErrorReporter::ReportError("Operator use2 expects its 2nd argument "
                                 "to be 'T' or '(stream T), T of kind DATA "
                                 "or of type 'tuple(X))', but"
                                 "receives '" + outstr1 + "' for T.");
      return nl->SymbolAtom( "typeerror" );
    }

  if (GSA_DEBUG) cout << "TypeMapUse2: 4" << endl;

  // 4. First and Second argument
  // check whether at least one stream argument is present
  if ( !sarg1isstream && !sarg2isstream )
    {
      ErrorReporter::ReportError(
        "Operator use2 expects at least one of its both first "
        "argument to be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if (GSA_DEBUG) cout << "TypeMapUse2: 5" << endl;

  // 5. Third argument
  // check third for being a map
  if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map), "map") ) )
    {
      nl->WriteToString(outstr1, map);
      ErrorReporter::ReportError("Operator use2 expects a map as "
                                 "3rd argument, but gets '" + outstr1 +
                                 "' instead.");
      return nl->SymbolAtom( "typeerror" );
    }

  if ( nl->ListLength(map) != 4 )
    {
      ErrorReporter::ReportError("Number of map arguments must be 2 "
                                 "for operator use2.");
      return nl->SymbolAtom( "typeerror" );
    }

  // get map arguments
  marg1 = nl->Second(map);
  marg2 = nl->Third(map);
  mres  = nl->Fourth(map);

  // check marg1

  if ( !( nl->Equal(marg1, sarg1Type) ) )
    {
      nl->WriteToString(outstr1, sarg1Type);
      nl->WriteToString(outstr2, marg1);
      ErrorReporter::ReportError("Operator use2: 1st argument's stream"
                                 "type does not match the type of the "
                                 "mapping's 1st argument. If e.g. the first "
                                 "is 'stream X', then the latter must be 'X'."
                                 "The types passed are '" + outstr1 +
                                 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  // check marg2
  if ( !( nl->Equal(marg2, sarg2Type) ) )
    {
      nl->WriteToString(outstr1, sarg2Type);
      nl->WriteToString(outstr2, marg2);
      ErrorReporter::ReportError("Operator use2: 2nd argument's stream"
                                 "type does not match the type of the "
                                 "mapping's 2nd argument. If e.g. the second"
                                 " is 'stream X', then the latter must be 'X'."
                                 "The types passed are '" + outstr1 +
                                 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if (GSA_DEBUG) cout << "TypeMapUse2: 6" << endl;

  // 6. Determine result type
  // get map result type 'sresType'
  if( !nl->IsAtom( mres )  && ( nl->ListLength( mres ) == 2) )
    {
      if (GSA_DEBUG) cout << "TypeMapUse2: 6.1" << endl;

      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
        {
          if (GSA_DEBUG) cout << "TypeMapUse2: 6.2" << endl;

          if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) &&
               !( !nl->IsAtom(nl->Second(mres)) &&
                  nl->ListLength(nl->Second(mres)) == 2 &&
                  TypeOfRelAlgSymbol(nl->First(nl->Second(mres)) == tuple) &&
                  IsTupleDescription(nl->Second(nl->Second(mres)))
                  )
               )
            {
              if (GSA_DEBUG) cout << "TypeMapUse2: 6.3" << endl;

              ErrorReporter::ReportError(
                "Operator use2 expects its 3rd Argument to "
                "return a '(stream T)', T of kind DATA or T = 'tuple(X)'.");
              return nl->SymbolAtom( "typeerror" );
            }
          if (GSA_DEBUG) cout << "TypeMapUse2: 6.4" << endl;

          resisstream = true;
          sresType = mres; // map result type is already a stream
        }
    }
  else // map result type is not a stream, so encapsulate it
    {
      if (GSA_DEBUG) cout << "TypeMapUse2: 6.5" << endl;

      if (    !( nl->IsAtom(mres) && am->CheckKind("DATA", mres, errorInfo))
              && !( !nl->IsAtom(mres) &&
                    nl->ListLength(mres) == 2 &&
                    !nl->IsAtom(nl->Second(mres)) &&
                    TypeOfRelAlgSymbol(nl->First(mres)  == tuple) &&
                    IsTupleDescription(nl->Second(mres))
                    )
              )
        {
          if (GSA_DEBUG) cout << "TypeMapUse2: 6.6" << endl;

          ErrorReporter::ReportError(
            "Operator use2 expects its 3rd Argument to "
            "return a type T of kind DATA or T = 'tuple(X)'.");
          return nl->SymbolAtom( "typeerror" );
        }
      if (GSA_DEBUG) cout << "TypeMapUse2: 6.7" << endl;

      resisstream = false;
      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);
    }

  if(GSA_DEBUG) cout << "TypeMapUse2: 7" << endl;

  // 7. This check can be removed when operators working on tuplestreams have
  //    been implemented:
  if (   (!nl->IsAtom(sarg1Type) &&
          TypeOfRelAlgSymbol(nl->First(sarg1Type)) == tuple )
         || (!nl->IsAtom(sarg1Type) &&
             TypeOfRelAlgSymbol(nl->First(sarg1Type)) == tuple ) )
    {
      ErrorReporter::ReportError("Operator use2 still not implemented for "
                                 "arguments of type 'tuple(X)' or "
                                 "'(stream tuple(X))'.");
      return nl->SymbolAtom( "typeerror" );
    }


  if (GSA_DEBUG) cout << "TypeMapUse2: 8" << endl;

  // 8. Append flags describing argument configuration for value mapping:
  //     0: no stream
  //     1: sarg1 is a stream
  //     2: sarg2 is a stream
  //     4: map result is a stream
  //
  //    e.g. 7=4+2+1: both arguments are streams and the
  //                  map result is a stream

  if(sarg1isstream) argConfCode += 1;
  if(sarg2isstream) argConfCode += 2;
  if(resisstream)   argConfCode += 4;

  argConfDescriptor = nl->OneElemList(nl->IntAtom(argConfCode));
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                           argConfDescriptor, sresType);
}

/*
5.20.2 Value Mapping for ~use~

*/

struct UseLocalInfo{
  bool Xfinished, Yfinished, funfinished; // whether we have finished
  Word X, Y, fun;                         // pointers to the argument nodes
  Word XVal, YVal, funVal;                // the last arg values
  int  argConfDescriptor;          // type of argument configuration
};

// (stream X) (map X Y) -> (stream Y)
int Use_SN( Word* args, Word& result, int message,
             Word& local, Supplier s )
{
  UseLocalInfo     *sli;
  Word              instream = args[0], fun = args[1];
  Word              funResult, argValue;
  ArgVectorPointer  funArgs;

  switch (message)
    {
    case OPEN :

      if(GSA_DEBUG) cout << "Use_SN received OPEN" << endl;
      sli = new UseLocalInfo;
      sli->Xfinished = true;
      qp->Open(instream.addr);
      sli->Xfinished = false;
      local = SetWord(sli);
      if(GSA_DEBUG) cout << "Use_SN finished OPEN" << endl;
      return 0;

    case REQUEST :

      // For each REQUEST, we get one value from the stream,
      // pass it to the parameter function and evalute the latter.
      // The result is simply passed on.

      if(GSA_DEBUG) cout << "Use_SN received REQUEST" << endl;
      if( local.addr == 0 )
        {
          if(GSA_DEBUG) cout << "Use_SN finished REQUEST: CANCEL (1)" << endl;
          return CANCEL;
        }
      sli = (UseLocalInfo*)local.addr;

      if (sli->Xfinished)
        {
          if(GSA_DEBUG) cout << "Use_SN finished REQUEST: CANCEL (2)" << endl;
          return CANCEL;
        }

      funResult.addr = 0;
      argValue.addr  = 0;
      qp->Request(instream.addr, argValue); // get one arg value from stream
      if(qp->Received(instream.addr))
        {
          funArgs = qp->Argument(fun.addr); // set argument for the
          (*funArgs)[0] = argValue;         //   parameter function
          qp->Request(fun.addr, funResult); // call parameter function
          // copy result:
          result = SetWord(((Attribute*) (funResult.addr))->Clone());
          ((Attribute*) (argValue.addr))->DeleteIfAllowed(); // delete argument
          if(GSA_DEBUG)
            cout << "        result.addr    =" << result.addr << endl;
          argValue.addr = 0;
          if(GSA_DEBUG)
            cout << "Use_SN finished REQUEST: YIELD" << endl;
          return YIELD;
        }
      else // (input stream consumed completely)
        {
          qp->Close(instream.addr);
          sli->Xfinished = true;
          result.addr = 0;
          if(GSA_DEBUG)
            cout << "Use_SN finished REQUEST: CANCEL (3)" << endl;
          return CANCEL;
        }

    case CLOSE :

      if(GSA_DEBUG) cout << "Use_SN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if ( !sli->Xfinished )
            qp->Close( instream.addr );
          delete sli;
        }
      if(GSA_DEBUG) cout << "Use_SN finished CLOSE" << endl;
      return 0;

    }  // end switch
  cout << "Use_SN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}


// (stream X) (map X (stream Y)) -> (stream Y)
int Use_SS( Word* args, Word& result, int message,
             Word& local, Supplier s )
{
  UseLocalInfo    *sli;
  Word             funResult;
  ArgVectorPointer funargs;

  switch (message)
    {
    case OPEN :

      sli = new UseLocalInfo;
      sli->X   = SetWord( args[0].addr );
      sli->fun = SetWord( args[1].addr );
      sli->Xfinished   = true;
      sli->funfinished = true;
      sli->XVal.addr   = 0;
      // open the ("outer") input stream and
      qp->Open( sli->X.addr );
      sli->Xfinished = false;
      // save the local information
      local = SetWord(sli);

      return 0;

    case REQUEST :

      // For each value from the 'outer' stream, an 'inner' stream
      // of values is generated by the parameter function.
      // For each REQUEST, we pass one value from the 'inner' stream
      // as the result value.
      // If the inner stream is consumed, we try to get a new value
      // from the 'outer' stream and re-open the inner stream

      if(GSA_DEBUG) cout << "\nUse_SS: Received REQUEST";
      //1. recover local information
      if( local.addr == 0 )
        return CANCEL;
      sli = (UseLocalInfo*)local.addr;

      // create the next result
      while( !sli->Xfinished )
        {
          if( sli->funfinished )
            {// end of map result stream reached -> get next X
              qp->Request( sli->X.addr, sli->XVal);
              if (!qp->Received( sli->X.addr ))
                { // Stream X is exhaused
                  qp->Close( sli->X.addr );
                  sli->Xfinished = true;
                  result.addr = 0;
                  return CANCEL;
                } // got an X-elem
              funargs = qp->Argument( sli->fun.addr );
              (*funargs)[0] = sli->XVal;
              qp->Open( sli->fun.addr );
              sli->funfinished = false;
            } // Now, we have an open map result stream
          qp->Request( sli->fun.addr, funResult );
          if(qp->Received( sli->fun.addr ))
            { // cloning and passing the result
              result = SetWord(((Attribute*) (funResult.addr))->Clone());
              ((Attribute*) (funResult.addr))->DeleteIfAllowed();
              if(GSA_DEBUG) cout << "     result.addr=" << result.addr << endl;
              return YIELD;
            }
          else
            { // end of map result stream reached
              qp->Close( sli->fun.addr );
              sli->funfinished = true;
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
            }
        } // end while

    case CLOSE :

      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if( !sli->funfinished )
            {
              qp->Close( sli->fun.addr );
              ((Attribute*)(sli->X.addr))->DeleteIfAllowed();
            }
          if ( !sli->Xfinished )
            qp->Close( sli->X.addr );
          delete sli;
        }
      return 0;
    }  // end switch
  cout << "\nUse_SS received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}


// (stream X) Y          (map X Y Z) -> (stream Z)
// X          (stream Y) (map X y Z) -> (stream Z)
int Use_SNN( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  UseLocalInfo     *sli;
  Word              xval, funresult;
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :

      if(GSA_DEBUG) cout << "\nUse_SNN received OPEN" << endl;
      sli = new UseLocalInfo ;
      sli->Xfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;
      sli->fun = SetWord(args[2].addr);
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
        {
          delete( sli );
          local.addr = 0;
          if(GSA_DEBUG)
            cout << "\nUse_SNN was called with stream result mapping!"
                 <<  endl;
          return 0;
        }
      if(sli->argConfDescriptor & 1)
        { // the first arg is the stream
          sli->X = SetWord(args[0].addr); // X is the stream
          sli->Y = SetWord(args[1].addr); // Y is the constant value
        }
      else
        { // the second arg is the stream
          sli->X = SetWord(args[1].addr); // X is the stream
          sli->Y = SetWord(args[0].addr); // Y is the constant value
        }

      qp->Open(sli->X.addr);              // open outer stream argument
      sli->Xfinished = false;

      local = SetWord(sli);
      if(GSA_DEBUG) cout << "Use_SNN finished OPEN" << endl;
      return 0;

    case REQUEST :

      // For each REQUEST, we get one value from the stream,
      // pass it (and the remaining constant argument) to the parameter
      // function and evalute the latter. The result is simply passed on.
      // sli->X is the stream, sli->Y the constant argument.

      if(GSA_DEBUG) cout << "Use_SNN received REQUEST" << endl;

      // 1. get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(GSA_DEBUG) cout << "Use_SNN finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (UseLocalInfo*) local.addr;
      if (sli->Xfinished)
        { // stream already exhausted earlier
          result.addr = 0;
          if(GSA_DEBUG) cout << "Use_SNN finished REQUEST: CLOSE (2)" << endl;
          return CANCEL;
        }

      // 2. request value from outer stream
      qp->Request( sli->X.addr, xval );
      if(!qp->Received( sli->X.addr ))
        { // stream exhausted now
          qp->Close( sli->X.addr );
          sli->Xfinished = true;
          if(GSA_DEBUG) cout << "Use_SNN finished REQUEST: CLOSE (3)" << endl;
          return CANCEL;
        }

      // 3. call parameter function, delete args and return result
      funargs = qp->Argument( sli->fun.addr );
      if (sli->argConfDescriptor & 1)
        {
          (*funargs)[0] = xval;
          (*funargs)[1] = sli->Y;
        }
      else
        {
          (*funargs)[0] = sli->Y;
          (*funargs)[1] = xval;
        }
      qp->Request( sli->fun.addr, funresult );
      result = SetWord(((Attribute*) (funresult.addr))->Clone());
      if(GSA_DEBUG) cout << "     result.addr=" << result.addr << endl;
      ((Attribute*) (xval.addr))->DeleteIfAllowed();
      if(GSA_DEBUG) cout << "Use_SNN finished REQUEST: YIELD" << endl;
      return YIELD;

    case CLOSE :

      if(GSA_DEBUG) cout << "Use_SNN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if (!sli->Xfinished)
            qp->Close( sli->X.addr ); // close input
          delete sli;
        }
      if(GSA_DEBUG) cout << "Use_SNN finished CLOSE" << endl;
      return 0;

    }  // end switch
  cout << "\nUse_SNN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}


// (stream X) Y          (map X Y (stream Z)) -> (stream Z)
// X          (stream Y) (map X y (stream Z)) -> (stream Z)
int Use_SNS( Word* args, Word& result, int message,
              Word& local, Supplier s )
{

  UseLocalInfo     *sli;
  Word              funresult;
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :

      if(GSA_DEBUG) cout << "\nUse_SNS received OPEN" << endl;
      sli = new UseLocalInfo ;
      sli->Xfinished   = true;
      sli->funfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;
      sli->fun.addr = 0;
      sli->XVal.addr = 0;
      sli->YVal.addr = 0;
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(! (sli->argConfDescriptor & 4))
        {
          delete( sli );
          local.addr = 0;
          cout << "\nUse_SNS was called with non-stream result mapping!"
               <<  endl;
          return 0;
        }
      if(sli->argConfDescriptor & 1)
        { // the first arg is the stream
          sli->X = SetWord(args[0].addr); // X is the stream
          sli->Y = SetWord(args[1].addr); // Y is the constant value
        }
      else
        { // the second arg is the stream
          sli->X = SetWord(args[1].addr); // X is the stream
          sli->Y = SetWord(args[0].addr); // Y is the constant value
        }
      sli->YVal = sli->Y; // save value of constant argument
      qp->Open(sli->X.addr);               // open the ("outer") input stream
      sli->Xfinished = false;
      sli->fun = SetWord(args[2].addr);
      local = SetWord(sli);
      if(GSA_DEBUG) cout << "Use_SNN finished OPEN" << endl;
      return 0;

    case REQUEST :

      // First, we check whether an inner stream is finished
      // (sli->funfinished). If so, we try to get a value from
      // the outer stream and try to re-open the inner stream.
      // sli->X is a pointer to the OUTER stream,
      // sli->Y is a pointer to the constant argument.

      if(GSA_DEBUG) cout << "Use_SNN received REQUEST" << endl;

      // 1. get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(GSA_DEBUG) cout << "Use_SNN finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (UseLocalInfo*) local.addr;
      // 2. request values from inner stream
      while (!sli->Xfinished)
        {
          while (sli->funfinished)
            { // the inner stream is closed, try to (re-)open it
              // try to get the next X-value from outer stream
              qp->Request(sli->X.addr, sli->XVal);
              if (!qp->Received(sli->X.addr))
                { // stream X exhaused. CANCEL
                  sli->Xfinished = true;
                  if(GSA_DEBUG)
                    cout << "Use_SNN finished REQUEST: CLOSE (3)" << endl;
                  return CANCEL;
                }
              funargs = qp->Argument( sli->fun.addr );
              if (sli->argConfDescriptor & 1)
                {
                  (*funargs)[0] = sli->XVal;
                  (*funargs)[1] = sli->YVal;
                }
              else
                {
                  (*funargs)[0] = sli->YVal;
                  (*funargs)[1] = sli->XVal;
                }
              qp->Open( sli->fun.addr );
              sli->funfinished = false;
            } // end while - Now, the inner stream is open again
          qp->Request(sli->fun.addr, funresult);
          if (qp->Received(sli->fun.addr))
            { // inner stream returned a result
              result = SetWord(((Attribute*) (funresult.addr))->Clone());
              ((Attribute*) (funresult.addr))->DeleteIfAllowed();
              if(GSA_DEBUG)
                {
                  cout << "     result.addr=" << result.addr << endl;
                  cout << "Use_SNN finished REQUEST: YIELD" << endl;
                }
              return YIELD;
            }
          else{ // inner stream exhausted
            qp->Close(sli->fun.addr);
            sli->funfinished = true;
            ((Attribute*)(sli->XVal.addr))->DeleteIfAllowed();
            sli->XVal.addr = 0;
          }
        } // end while
      result.addr = 0;
      if(GSA_DEBUG) cout << "Use_SNN finished REQUEST: CLOSE (4)" << endl;
      return CANCEL;

    case CLOSE :

      if(GSA_DEBUG) cout << "Use_SNN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if (!sli->funfinished)
            qp->Close( sli->fun.addr ); // close map result stream
          if (!sli->Xfinished)
            qp->Close( sli->X.addr );   // close outer stream
          delete sli;
        }
      if(GSA_DEBUG) cout << "Use_SNN finished CLOSE" << endl;
      return 0;

    }  // end switch
  cout << "Use_SNN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached

}

// (stream X) (stream Y) (map X Y Z) -> (stream Z)
int Use_SSN( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  UseLocalInfo     *sli;
  Word              funresult;
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :

      if(GSA_DEBUG) cout << "\nUse_SSN received OPEN" << endl;
      sli = new UseLocalInfo ;
      sli->Xfinished = true;
      sli->Yfinished = true;
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
        {
          delete( sli );
          local.addr = 0;
          cout << "\nUse_SSN was called with stream result mapping!"
               <<  endl;
          return 0;
        }
      if(!(sli->argConfDescriptor & 3))
        {
          delete( sli );
          local.addr = 0;
          cout << "\nUse_SSN was called with non-stream arguments!"
               <<  endl;
          return 0;
        }
      sli->X = SetWord(args[0].addr);   // X is the stream
      sli->Y = SetWord(args[1].addr);   // Y is the constant value
      sli->fun = SetWord(args[2].addr); // fun is the mapping function

      qp->Open(sli->X.addr);            // open outer stream argument
      sli->Xfinished = false;
      local = SetWord(sli);
      if(GSA_DEBUG) cout << "Use_SSN finished OPEN" << endl;
      return 0;

    case REQUEST :

      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we evaluate the
      // parameter function (sli->fun), which return a single result.
      // A clone of the result is passed as the result.
      // We also need to delete each element, when it is not required
      // anymore.

      if(GSA_DEBUG) cout << "Use_SSN received REQUEST" << endl;

      // get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(GSA_DEBUG) cout << "Use_SSN finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (UseLocalInfo*) local.addr;

      while(!sli->Xfinished)
        {
          if (sli->Yfinished)
            { // try to (re-) start outer instream
              qp->Request(sli->X.addr, sli->XVal);
              if (!qp->Received(sli->X.addr))
                { // outer instream exhaused
                  qp->Close(sli->X.addr);
                  sli->Xfinished = true;
                  result.addr = 0;
                  if(GSA_DEBUG)
                    cout << "Use_SSN finished REQUEST: CANCEL (2)" << endl;
                  return CANCEL;
                }
              // Got next X-elem. (Re-)Start inner instream:
              qp->Open(sli->Y.addr);
              sli->Yfinished = false;
            }
          // Now, we have open inner and outer streams
          qp->Request(sli->Y.addr, sli->YVal);
          if (!qp->Received(sli->Y.addr))
            { // inner stream is exhausted
              qp->Close(sli->Y.addr);
              // Delete current X-elem:
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
              sli->Yfinished = true;
            }
          // got next Y-elem
          if (!sli->Xfinished && !sli->Yfinished)
            { // pass parameters and call mapping, clone result
              funargs = qp->Argument( sli->fun.addr );
              (*funargs)[0] = sli->XVal;
              (*funargs)[1] = sli->YVal;
              qp->Request( sli->fun.addr, funresult );
              result = SetWord(((Attribute*) (funresult.addr))->Clone());
              ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
              if(GSA_DEBUG) cout << "Use_SSN finished REQUEST: YIELD" << endl;
              return YIELD;
            }
        } // end while
      if(GSA_DEBUG) cout << "Use_SSN finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;

    case CLOSE :

      if(GSA_DEBUG) cout << "Use_SSN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if (!sli->Yfinished)
            {
              qp->Close( sli->Y.addr ); // close inner instream
              // Delete current X-elem:
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
            }
          if (!sli->Xfinished)
            qp->Close( sli->X.addr ); // close outer instream
          delete sli;
        }
      result.addr = 0;
      if(GSA_DEBUG) cout << "Use_SSN finished CLOSE" << endl;
      return 0;

    }  // end switch
  result.addr = 0;
  cout << "\nUse_SSN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}



// (stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)
int Use_SSS( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  UseLocalInfo     *sli;
  Word              funresult;
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :

      if(GSA_DEBUG) cout << "\nUse_SSS received OPEN" << endl;
      sli = new UseLocalInfo ;
      sli->Xfinished   = true;
      sli->Yfinished   = true;
      sli->funfinished = true;
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(!(sli->argConfDescriptor & 4) )
        {
          delete( sli );
          local.addr = 0;
          cout << "\nUse_SSS was called with non-stream result mapping!"
               <<  endl;
          return 0;
        }
      if(!(sli->argConfDescriptor & 3))
        {
          delete( sli );
          local.addr = 0;
          cout << "\nUse_SSS was called with non-stream arguments!"
               <<  endl;
          return 0;
        }
      sli->X   = args[0]; // X is the stream
      sli->Y   = args[1]; // Y is the constant value
      sli->fun = args[2]; // fun is the mapping function
      qp->Open(sli->X.addr);            // open X stream argument
      sli->Xfinished = false;
      local = SetWord(sli);
      if(GSA_DEBUG) cout << "Use_SSS finished OPEN" << endl;
      return 0;

    case REQUEST :

      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we open the
      // parameter function (sli->fun), which returns a stream result.
      // We consume this map result stream one-by-one.
      // When it is finally consumed, we try to restart it with the next
      // X/Y value pair.
      // A clone of the result is passed as the result.
      // We also need to delete each X/Y element, when it is not required
      // any more.

      if(GSA_DEBUG) cout << "Use_SSS received REQUEST" << endl;

      // get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(GSA_DEBUG) cout << "Use_SSS finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (UseLocalInfo*) local.addr;

      while(!sli->Xfinished)
        {
          if (sli->Yfinished)
            { // get next X-value from outer instream
              // and restart inner (Y-) instream
              qp->Request(sli->X.addr, sli->XVal);
              if (!qp->Received(sli->X.addr))
                { // X-instream exhaused
                  qp->Close(sli->X.addr);
                  sli->Xfinished = true;
                  if(GSA_DEBUG)
                    cout << "Use_SSS finished REQUEST: CANCEL (2)" << endl;
                  result.addr = 0;
                  return CANCEL;
                }
              // Got next X-elem. (Re-)Start inner Y-instream:
              qp->Open(sli->Y.addr);
              sli->Yfinished = false;
            } // Now, we have open X- and Y- streams
          if (sli->funfinished)
            { // get next Y-value from inner instream
              // and open new map result stream
              qp->Request(sli->Y.addr, sli->YVal);
              if (!qp->Received(sli->Y.addr))
                {
                  qp->Close(sli->Y.addr);
                  ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
                  sli->Yfinished = true;
                }
              else
                {
                  funargs = qp->Argument( sli->fun.addr );
                  (*funargs)[0] = sli->XVal;
                  (*funargs)[1] = sli->YVal;
                  qp->Open( sli->fun.addr );
                  sli->funfinished = false;
                }
            }
          // Now, we have an open map result streams
          if (!sli->Xfinished && !sli->Yfinished && !sli->funfinished)
            { // pass parameters and call mapping, clone result
              funargs = qp->Argument( sli->fun.addr );
              (*funargs)[0] = sli->XVal;
              (*funargs)[1] = sli->YVal;
              qp->Request( sli->fun.addr, funresult );
              if ( qp->Received(sli->fun.addr) )
                { // got a value from map result stream
                  result=SetWord(((Attribute*)(funresult.addr))->Clone());
                  ((Attribute*) (funresult.addr))->DeleteIfAllowed();
                  if(GSA_DEBUG)
                    cout << "Use_SSS finished REQUEST: YIELD" << endl;
                  return YIELD;
                }
              else
                { // map result stream exhausted
                  qp->Close( sli->fun.addr) ;
                  ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
                  sli->funfinished = true;
                } // try to restart with new X/Y pairing
            }
        } // end while
      result.addr = 0;
      if(GSA_DEBUG) cout << "Use_SSS finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;

    case CLOSE :

      if(GSA_DEBUG) cout << "Use_SSS received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if (!sli->funfinished)
            {
              qp->Close( sli->fun.addr ); // close map result stream
              // Delete current Y-elem:
              ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
            }
          if (!sli->Yfinished)
            {
              qp->Close( sli->Y.addr ); // close inner instream
              // Delete current X-elem:
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
            }
          if (!sli->Xfinished)
            qp->Close( sli->X.addr ); // close outer instream
          delete sli;
        }
      if(GSA_DEBUG) cout << "Use_SSS finished CLOSE" << endl;
      return 0;

    }  // end switch
  cout << "\nUse_SSS received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}



/*
5.20.3 Specification for operator ~use~

*/
const string
StreamSpecUse=
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For X in kind DATA or X = tuple(Z)*, Y in kind DATA:\n"
  "(*: not yet implemented)\n"
  "((stream X) (map X Y)         ) -> (stream Y) \n"
  "((stream X) (map X (stream Y))) -> (stream Y)</text--->"
  "<text>_ use [ _ ]</text--->"
  "<text>The use class of operators implements "
  "a set of functors, that derive stream-valued "
  "operators from operators taking scalar "
  "arguments and returning scalar values or "
  "streams of values.</text--->"
  "<text>query intstream(1,5) use[ fun(i:int) i*i ] printstream count;\n"
  "query intstream(1,5) use[ fun(i:int) intstream(i,5) ] printstream count;"
  "</text---> ) )";

const string
StreamSpecUse2=
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For X in kind DATA or X = tuple(W)*, Y,Z in kind DATA:\n"
  "(*: not yet implemented)\n"
  "((stream X) Y          (map X Y Z)         ) -> (stream Z) \n"
  "((stream X) Y          (map X Y stream(Z)) ) -> (stream Z) \n"
  "(X          (stream Y) (map X Y Z)         ) -> (stream Z) \n"
  "(X          (stream Y) (map X Y (stream Z))) -> (stream Z) \n"
  "((stream X) (stream Y) (map X Y Z)         ) -> (stream Z) \n"
  "((stream X) (stream Y) (map X Y (stream Z))) -> (stream Z)</text--->"
  "<text>_ _ use2 [ _ ]</text--->"
  "<text>The use2 class of operators implements "
  "a set of functors, that derive stream-valued "
  "operators from operators taking scalar "
  "arguments and returning scalar values or "
  "streams of values. use2 performs a product "
  "between the two first of its arguments, passing each "
  "combination to the mapped function once.</text--->"
  "<text>query intstream(1,5) [const int value 5] use2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;\n"
  "query [const int value 3] intstream(1,5) use2[ fun(i:int, j:int) i+j ] "
  "printstream count;\n"
  "query intstream(1,5) [const int value 3] use2[ fun(i:int, j:int) i+j ] "
  "printstream count;\n"
  "query [const int value 2] intstream(1,5) use2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;\n"
  "query [const int value 3] intstream(1,5) use2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;\n"
  "query intstream(1,2) intstream(1,3) use2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;</text---> ) )";

/*
5.20.4 Selection Function of operator ~use~

*/

ValueMapping streamusemap[] =
  { Use_SN,
    Use_SS
    //    ,
    //    Use_TsN,
    //    Use_TsS
  };

int
streamUseSelect( ListExpr args )
{
  ListExpr sarg1     = nl->First( args );
  ListExpr mapresult = nl->Third(nl->Second(args));
  bool     isStream  = false;
  bool     isTuple   = false;

  // check type of sarg1
  if( TypeOfRelAlgSymbol(nl->First(sarg1)) == stream &&
      (!nl->IsAtom(nl->Second(sarg1))) &&
      (nl->ListLength(nl->Second(sarg1)) == 2) &&
      TypeOfRelAlgSymbol(nl->First(nl->Second(sarg1))) == tuple &&
      IsTupleDescription(nl->Second((nl->Second(sarg1)))) )
    isTuple = true;
  else
    isTuple = false;

  // check type of map result (stream or non-stream type)
  if(   !( nl->IsAtom(mapresult) )
        && ( nl->ListLength( mapresult ) == 2)
        && TypeOfRelAlgSymbol(nl->First(sarg1) == stream ) )
    isStream = true;
  else
    isStream = false;

  // compute index without offset
  if      (!isTuple && !isStream) return 0;
  else if (!isTuple &&  isStream) return 1;
  else if ( isTuple && !isStream) return 2;
  else if ( isTuple &&  isStream) return 3;
  else
    {
      cout << "\nstreamUseSelect: Something's wrong!\n";
    }
  return -1;
}


ValueMapping streamuse2map[] =
  { Use_SNN,
    Use_SNS,
    Use_SSN,
    Use_SSS
    //     ,
    //    Use_TsNN,
    //    Use_TsNS,
    //    Use_TsTsN,
    //    Use_TsTsS
  };

int
streamUse2Select( ListExpr args )
{
  ListExpr
    X = nl->First(args),
    Y = nl->Second(args),
    M = nl->Third(args);
  bool
    xIsStream = false,
    yIsStream = false,
    resIsStream = false;
  bool
    xIsTuple = false,
    yIsTuple = false,
    resIsTuple = false;
  int index = 0;

  // examine first arg
  // check type of sarg1
  if( nl->IsAtom(X) )
    { xIsTuple = false; xIsStream = false;}
  if( !nl->IsAtom(X) &&
      TypeOfRelAlgSymbol(nl->First(X)) == tuple )
    { xIsTuple = true; xIsStream = false; }
  if( !nl->IsAtom(X) &&
      TypeOfRelAlgSymbol(nl->First(X)) == stream )
    {
      xIsStream = true;
      if(!nl->IsAtom(nl->Second(X)) &&
         (nl->ListLength(nl->Second(X)) == 2) &&
         TypeOfRelAlgSymbol(nl->First(X)) == tuple )
        xIsTuple = true;
      else
        xIsTuple = false;
    }

  // examine second argument
  if( nl->IsAtom(Y) )
    { yIsTuple = false; yIsStream = false;}
  if( !nl->IsAtom(Y) &&
      TypeOfRelAlgSymbol(nl->First(Y)) == tuple )
    { yIsTuple = true; yIsStream = false; }
  if( !nl->IsAtom(Y) &&
      TypeOfRelAlgSymbol(nl->First(Y)) == stream )
    {
      yIsStream = true;
      if(!nl->IsAtom(nl->Second(Y)) &&
         (nl->ListLength(nl->Second(Y)) == 2) &&
         TypeOfRelAlgSymbol(nl->First(Y)) == tuple )
        yIsTuple = true;
      else
        yIsTuple = false;
    }

  // examine mapping result type
  if( nl->IsAtom(nl->Fourth(M)) )
    { resIsTuple = false; resIsStream = false;}
  if( !nl->IsAtom(nl->Fourth(M)) &&
      TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == tuple )
    { resIsTuple = true; resIsStream = false; }
  if( !nl->IsAtom(nl->Fourth(M)) &&
      TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == stream )
    {
      resIsStream = true;
      if(!nl->IsAtom(nl->Second(nl->Fourth(M))) &&
         (nl->ListLength(nl->Second(nl->Fourth(M))) == 2) &&
         TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == tuple )
        resIsTuple = true;
      else
        resIsTuple = false;
    }

  // calculate appropriate index value

  // tuple variants offest    : +4
  // both args streams        : +2
  // mapping result is stream : +1
  index = 0;
  if ( xIsTuple || yIsTuple )   index += 4;
  if ( xIsStream && yIsStream ) index += 2;
  if ( resIsStream )            index += 1;

  if (index > 3)
    cout << "\nWARNING: index =" << index
         << ">3 in streamUse2Select!" << endl;

  return index;
}

/*
5.20.5  Definition of operator ~use~

*/


Operator streamuse( "use",
                           StreamSpecUse,
                           2,
                           streamusemap,
                           streamUseSelect,
                           TypeMapUse);


Operator streamuse2( "use2",
                            StreamSpecUse2,
                            4,
                            streamuse2map,
                            streamUse2Select,
                            TypeMapUse2);


/*
5.24 Operator ~aggregateS~

Stream aggregation operator

This operator applies an aggregation function (which must be binary,
associative and commutative) to a stream of data using a given neutral (initial)
value (which is also returned if the stream is empty). If the stream contains
only one single element, this element is returned as the result.
The result a single value of the same kind.

----
       For T in kind DATA:
       aggregateS: ((stream T) x (T x T --> T) x T) --> T

----

The first argument is the input stream.
The second argument is the function used in the aggregation.
The third value is used to initialize the mapping (for the first elem)
and will also be return if the input stream is empty.

5.24.1 Type mapping function for ~aggregateS~

*/
ListExpr StreamaggregateTypeMap( ListExpr args )
{
  string outstr1, outstr2;
  ListExpr TypeT;

  // check for correct length
  if (nl->ListLength(args) != 3)
    {
      ErrorReporter::ReportError("Operator aggregateS expects a list of length "
                                 "three.");
      return nl->SymbolAtom( "typeerror" );
    }

  // get single arguments
  ListExpr instream   = nl->First(args),
    map        = nl->Second(args),
    zerovalue  = nl->Third(args),
    errorInfo  = nl->OneElemList(nl->SymbolAtom("ERROR"));

  // check for first arg to be atomic and of kind DATA
  if ( nl->IsAtom(instream) ||
       ( nl->ListLength( instream ) != 2) ||
       !(TypeOfRelAlgSymbol(nl->First(instream) == stream )) ||
       !am->CheckKind("DATA", nl->Second(instream), errorInfo) )
    {
      ErrorReporter::ReportError("Operator aggregateS expects a list of length "
                                 "two as first argument, having structure "
                                 "'(stream T)', for T in kind DATA.");
      return nl->SymbolAtom( "typeerror" );
    }
  else
    TypeT = nl->Second(instream);

  // check for second to be of length 4, (map T T T)
  // T of same type as first
  if ( nl->IsAtom(map) ||
       !(nl->ListLength(map) == 4) ||
       !( nl->IsEqual(nl->First(map), "map") ) ||
       !( nl->Equal(nl->Fourth(map), nl->Second(map)) ) ||
       !( nl->Equal(nl->Third(map), nl->Second(map)) ) ||
       !( nl->Equal(nl->Third(map), TypeT) ) )
    {
      ErrorReporter::ReportError("Operator aggregateS expects a list of length "
                                 "four as second argument, having structure "
                                 "'(map T T T)', where T has the base type of "
                                 "the first argument.");
      return nl->SymbolAtom( "typeerror" );
    }

  // check for third to be atomic and of the same type T
  if ( !nl->IsAtom(zerovalue) ||
       !nl->Equal(TypeT, zerovalue) )
    {
      ErrorReporter::ReportError("Operator aggregateS expects a list of length"
                                 "one as third argument (neutral elem), having "
                                 "structure 'T', where T is also the type of "
                                 "the mapping's arguments and result. Also, "
                                 "T must be of kind DATA.");
      return nl->SymbolAtom( "typeerror" );
    }

  // return T as the result type.
  return TypeT;
}

/*
5.24.2 Value mapping for operator ~aggregate~

*/

struct AggregStruct
{
  inline AggregStruct( long level, Word value ):
    level( level ), value( value )
  {}

  inline AggregStruct( const AggregStruct& a ):
    level( a.level ), value( a.value )
  {}

  inline AggregStruct& operator=( const AggregStruct& a )
  { level = a.level; value = a.value; return *this; }

  long level;
  Word value;
  // if the level is 0 then value contains an element pointer,
  // otherwise it contains a previous result of the aggregate operator
};

int Streamaggregate( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  // The argument vector contains the following values:
  Word
    stream  = args[0], // stream of elements T
    aggrmap = args[1], // mapping function T x T --> T
    nval    = args[2]; // zero value/neutral element T

  Word t1, t2, iterWord, resultWord;
  ArgVectorPointer vector = qp->Argument(aggrmap.addr);

  qp->Open(stream.addr);
  result = qp->ResultStorage(s);

  // read the first tuple
  qp->Request( stream.addr, t1 );
  if( !qp->Received( stream.addr ) )
    { // Case 1: empty stream
      result.addr = ((Attribute*) nval.addr)->Copy();
    }
  else
    {
      stack<AggregStruct> aggrStack;

      // read the second tuple
      qp->Request( stream.addr, t2 );
      if( !qp->Received( stream.addr ) )
        { // Case 2: only one single elem in stream
          result.addr = ((Attribute*)t1.addr)->Copy();
        }
      else
        { // there are at least two stream elements
          // match both elements and put the result into the stack
          (*vector)[0] = SetWord(t1.addr);
          (*vector)[1] = SetWord(t2.addr);
          qp->Request( aggrmap.addr, resultWord );
          aggrStack.push( AggregStruct( 1, resultWord ) );
          // level 1 because we matched a level 0 elem
          qp->ReInitResultStorage( aggrmap.addr );
          ((Attribute*)t1.addr)->DeleteIfAllowed();
          ((Attribute*)t2.addr)->DeleteIfAllowed();

          // process the rest of the stream
          qp->Request( stream.addr, t1 );
          while( qp->Received( stream.addr ) )
            {
              long level = 0;
              iterWord = SetWord( ((Attribute*)t1.addr)->Copy() );
              while( !aggrStack.empty() && aggrStack.top().level == level )
                {
                  (*vector)[0] = aggrStack.top().value;
                  (*vector)[1] = iterWord;
                  qp->Request(aggrmap.addr, resultWord);
                  ((Attribute*)iterWord.addr)->DeleteIfAllowed();
                  iterWord = resultWord;
                  qp->ReInitResultStorage( aggrmap.addr );
                  if( aggrStack.top().level == 0 )
                    ((Attribute*)aggrStack.top().value.addr)->DeleteIfAllowed();
                  else
                    delete (Attribute*)aggrStack.top().value.addr;
                  aggrStack.pop();
                  level++;
                }
              if( level == 0 )
                aggrStack.push( AggregStruct( level, t1 ) );
              else
                {
                  aggrStack.push( AggregStruct( level, iterWord ) );
                  ((Attribute*)t1.addr)->DeleteIfAllowed();
                }
              qp->Request( stream.addr, t1 );
            }

          // if the stack contains only one entry, then we are done
          if( aggrStack.size() == 1 )
            {
              result.addr = ((Attribute*)aggrStack.top().value.addr)->Copy();
              ((Attribute*)aggrStack.top().value.addr)->DeleteIfAllowed();
            }
          else
            // the stack must contain more elements and we call the
            // aggregate function for them
            {
              iterWord = aggrStack.top().value;
              int level = aggrStack.top().level;
              aggrStack.pop();
              while( !aggrStack.empty() )
                {
                  (*vector)[0] = level == 0 ?
                    SetWord( ((Attribute*)iterWord.addr) ) :
                    iterWord;
                  (*vector)[1] = aggrStack.top().value;
                  qp->Request( aggrmap.addr, resultWord );
                  ((Attribute*)iterWord.addr)->DeleteIfAllowed();
                  ((Attribute*)aggrStack.top().value.addr)->DeleteIfAllowed();
                  iterWord = resultWord;
                  qp->ReInitResultStorage( aggrmap.addr );
                  level++;
                  aggrStack.pop();
                }
              result.addr = ((Attribute*)iterWord.addr)->Copy();
              ((Attribute*)iterWord.addr)->DeleteIfAllowed();
            }
        }
    }

  qp->Close(stream.addr);

  return 0;
}


/*
5.24.3 Specification for operator ~aggregate~

*/

const string StreamaggregateSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>For T in kind DATA:\n"
  "((stream T) ((T T) -> T) T ) -> T\n</text--->"
  "<text>_ aggregateS [ fun ; _ ]</text--->"
  "<text>Aggregates the values from the stream (1st arg) "
  "using a binary associative and commutative "
  "aggregation function (2nd arg), "
  "and a 'neutral value' (3rd arg, also passed as the "
  "result if the stream is empty). If the stream contains"
  "only one single element, that element will be returned"
  "as the result.</text--->"
  "<text>query intstream(1,5) aggregateS[ "
  "fun(i1:int, i2:int) i1+i2 ; 0]\n"
  "query intstream(1,5) aggregateS[ "
  "fun(i1:STREAMELEM, i2:STREAMELEM) ifthenelse(i1>i2,i1,i2) ; 0]</text--->"
  ") )";

/*
5.24.4 Selection Function of operator ~aggregate~

*/

ValueMapping streamaggregatemap[] =
  {
    Streamaggregate
  };


int streamaggregateSelect( ListExpr args )
{
  return 0;
}

/*
5.24.5 Definition of operator ~aggregate~

*/

Operator streamaggregateS( "aggregateS",
                                 StreamaggregateSpec,
                                 1,
                                 streamaggregatemap,
                                 streamaggregateSelect,
                                 StreamaggregateTypeMap);


/*
5.27 Operator ~transformstream~

----
  transformstream: (stream T) -> stream(tuple((element T)))
                   stream(tuple((id T))) -> (stream T)

  for T in kind DATA, id some arbitrary identifier

----

Operator ~transformstream~ transforms a (stream DATA) into a
(stream(tuple((element DATA)))) and vice versa. ~element~ is the name for the
attribute created.

The result of the first variant can e.g. be consumed to form a relation
or be processed using ordinary tuplestream operators.

*/

/*
5.27.1 Type mapping function for ~transformstream~

*/

ListExpr StreamTransformstreamTypeMap(ListExpr args)
{
  ListExpr first ;
  string argstr;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  ListExpr TupleDescr, T;

  if (nl->ListLength(args) != 1)
    {
      ErrorReporter::ReportError("Operator transformstream expects a list of "
                                 "length one.");
      return nl->SymbolAtom("typeerror");
    }

  first = nl->First(args);
  nl->WriteToString(argstr, first);

  // check for variant 1: (stream T)
  if ( !nl->IsAtom(first) &&
       (nl->ListLength(first) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       nl->IsAtom(nl->Second(first)) &&
       am->CheckKind("DATA", nl->Second(first), errorInfo) )
    {
      T = nl->Second(first);
      return nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
            nl->SymbolAtom("tuple"),
            nl->OneElemList(
                nl->TwoElemList(
                    nl->SymbolAtom("elem"),
                    T))));
    }
  // check for variant 2: stream(tuple((id T)))
  if ( !nl->IsAtom(first) &&
       (nl->ListLength(first) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       !nl->IsAtom(nl->Second(first)) &&
       (nl->ListLength(nl->Second(first)) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) )
    {
      TupleDescr = nl->Second(nl->Second(first));
      nl->WriteToString(argstr, TupleDescr);
      if(GSA_DEBUG)
        cout << "\n In tupledescr = " << argstr << endl;
      if ( !nl->IsAtom(TupleDescr) &&
           (nl->ListLength(TupleDescr) == 1) &&
           !nl->IsAtom(nl->First(TupleDescr)) &&
           (nl->ListLength(nl->First(TupleDescr)) == 2) &&
           (nl->IsAtom(nl->First(nl->First(TupleDescr)))) &&
           (nl->IsAtom(nl->Second(nl->First(TupleDescr)))) &&
           am->CheckKind("DATA", nl->Second(nl->First(TupleDescr)), errorInfo))
        {
          T = nl->Second(nl->First(TupleDescr));
          return nl->TwoElemList(
                                 nl->SymbolAtom("stream"),
                                 T);
        }
    }

  // Wrong argument format!
  ErrorReporter::ReportError(
    "Operator transformstream expects exactly one argument. either "
    "of type '(stream T)',or 'stream(tuple((id T))))', where T is of "
    "kind DATA.\n"
    "The passed argument has type '"+ argstr +"'.");
  return nl->SymbolAtom("typeerror");
}

/*
5.27.2 Value mapping for operator ~transformstream~

*/

struct TransformstreamLocalInfo
{
  bool     finished;
  TupleType *resultTupleType;
};

// The first variant creates a tuplestream from a stream:
int Transformstream_S_TS(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  TransformstreamLocalInfo *sli;
  Word      value;
  ListExpr  resultType;
  Tuple     *newTuple;


  switch ( message )
    {
    case OPEN:

      qp->Open( args[0].addr );
      sli = new TransformstreamLocalInfo;

      resultType = GetTupleResultType( s );
      sli->resultTupleType = new TupleType( nl->Second( resultType ) );
      sli->finished = false;
      local = SetWord(sli);
      return 0;

    case REQUEST:

      if (local.addr == 0)
        return CANCEL;

      sli = (TransformstreamLocalInfo*) (local.addr);
      if (sli->finished)
        return CANCEL;

      result = SetWord((Attribute*)((qp->ResultStorage(s)).addr));

      qp->Request( args[0].addr, value );
      if (!qp->Received( args[0].addr ))
        { // input stream consumed
          qp->Close( args[0].addr );
          sli->finished = true;
          result.addr = 0;
          return CANCEL;
        }
      // create tuple, copy and pass result, delete value
      newTuple = new Tuple( sli->resultTupleType );
      newTuple->PutAttribute( 0, ((Attribute*)value.addr)->Clone() );
      ((Attribute*)(value.addr))->DeleteIfAllowed();
      result = SetWord(newTuple);
      return YIELD;

    case CLOSE:

      if (local.addr != 0)
        {
          sli = (TransformstreamLocalInfo*) (local.addr);
          if (!sli->finished)
            qp->Close( args[0].addr );
          sli->resultTupleType->DeleteIfAllowed();
          delete sli;
        }
      return 0;
    }
  cout << "Transformstream_S_TS: UNKNOWN MESSAGE!" << endl;
  return 0;
}

// The second variant creates a stream from a tuplestream:
int Transformstream_TS_S(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  TransformstreamLocalInfo *sli;
  Word   tuple;
  Tuple* tupleptr;

  switch ( message )
    {
    case OPEN:
      if (GSA_DEBUG) cout << "Transformstream_TS_S: OPEN called" << endl;
      qp->Open( args[0].addr );
      sli = new TransformstreamLocalInfo;
      sli->finished = false;
      local = SetWord(sli);
      if (GSA_DEBUG) cout << "Transformstream_TS_S: OPEN finished" << endl;
      return 0;

    case REQUEST:
      if (GSA_DEBUG) cout << "Transformstream_TS_S: REQUEST called" << endl;
      if (local.addr == 0)
        {
          if (GSA_DEBUG) cout
            << "Transformstream_TS_S: REQUEST return CANCEL (1)" << endl;
          return CANCEL;
        }

      sli = (TransformstreamLocalInfo*) (local.addr);
      if (sli->finished)
        {
          if (GSA_DEBUG) cout
            << "Transformstream_TS_S: REQUEST return CANCEL (2)" << endl;
          return CANCEL;
        }

      qp->Request( args[0].addr, tuple );
      if (!qp->Received( args[0].addr ))
        { // input stream consumed
          qp->Close( args[0].addr );
          sli->finished = true;
          result.addr = 0;
          if (GSA_DEBUG) cout
            << "Transformstream_TS_S: REQUEST return CANCEL (3)" << endl;
          return CANCEL;
        }
      // extract, copy and pass value, delete tuple
      tupleptr = (Tuple*)tuple.addr;
      result.addr = tupleptr->GetAttribute(0)->Clone();
      tupleptr->DeleteIfAllowed();
      if (GSA_DEBUG) cout
        << "Transformstream_TS_S: REQUEST return YIELD" << endl;
      return YIELD;

    case CLOSE:

      if (GSA_DEBUG) cout << "Transformstream_TS_S: CLOSE called" << endl;
      if (local.addr != 0)
        {
          sli = (TransformstreamLocalInfo*) (local.addr);
          if (!sli->finished)
            qp->Close( args[0].addr );
          delete sli;
        }
      if (GSA_DEBUG) cout << "Transformstream_TS_S: CLOSE finished" << endl;
      return 0;

    }
  cout << "Transformstream_TS_S: UNKNOWN MESSAGE!" << endl;
  return 0;
}

/*
5.27.3 Specification for operator ~transformstream~

*/
const string StreamTransformstreamSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>For T in kind DATA:\n"
  "(stream T) -> stream(tuple((elem T)))\n"
  "stream(tuple(attrname T)) -> (stream T)</text--->"
  "<text>_ transformstream</text--->"
  "<text>Transforms a 'stream T' into a tuplestream "
  "with a single attribute 'elem' containing the "
  "values coming from the input stream and vice "
  "versa. The identifier 'elem' is fixed, the "
  "attribute name 'attrname' may be arbitrary "
  "chosen, but the tuplestream's tupletype may "
  "have only a single attribute.</text--->"
  "<text>query intstream(1,5) transformstream consume\n "
  "query ten feed transformstream printstream count</text--->"
  ") )";

/*
5.27.4 Selection Function of operator ~transformstream~

*/

ValueMapping streamtransformstreammap[] =
  {
    Transformstream_S_TS,
    Transformstream_TS_S
  };

int streamTransformstreamSelect( ListExpr args )
{
  ListExpr first = nl->First( args );
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if ( !nl->IsAtom(first) &&
       (nl->ListLength(first) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       nl->IsAtom(nl->Second(first)) &&
       am->CheckKind("DATA", nl->Second(first), errorInfo) )
    return 0;
  if ( !nl->IsAtom(first) &&
       (nl->ListLength(first) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       !nl->IsAtom(nl->Second(first)) &&
       (nl->ListLength(nl->Second(first)) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple))
    return 1;
  cout << "\nstreamTransformstreamSelect: Wrong type!" << endl;
  return -1;
}


/*
5.27.5 Definition of operator ~transformstream~

*/

Operator streamtransformstream( "transformstream",
                      StreamTransformstreamSpec,
                      2,
                      streamtransformstreammap,
                      streamTransformstreamSelect,
                      StreamTransformstreamTypeMap);


/*
5.28 Operator ~count~

Signature:

----
     For T in kind DATA:
     (stream T) -> int

----

The operator counts the number of stream elements.

*/

/*
5.28.1 Type mapping function for ~count~

*/

ListExpr
streamCountType( ListExpr args )
{
  ListExpr arg1;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string outstr;

  if ( nl->ListLength(args) == 1 )
    {
      arg1 = nl->First(args);

    if ( !nl->IsAtom(arg1) && nl->ListLength(arg1) == 2 )
      {
        if ( nl->IsEqual(nl->First(arg1), "stream")
             && ( nl->IsAtom(nl->Second(arg1) ) )
             && am->CheckKind("DATA", nl->Second(arg1), errorInfo) )
          return nl->SymbolAtom("int");
        else
          {
            nl->WriteToString(outstr, arg1);
            ErrorReporter::ReportError("Operator count expects a (stream T), "
                                       "T in kind DATA. The argument profided "
                                       "has type '" + outstr + "' instead.");
          }
      }
    }
  nl->WriteToString(outstr, nl->First(args));
  ErrorReporter::ReportError("Operator count expects only a single "
                             "argument of type (stream T), T "
                             "in kind DATA. The argument provided "
                             "has type '" + outstr + "' instead.");
  return nl->SymbolAtom("typeerror");
}

/*
5.28.2 Value mapping for operator ~count~

*/

int
streamCountFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
  Count the number of elements in a stream. An example for consuming a stream.

*/
{
  Word elem;
  int count = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);

  while ( qp->Received(args[0].addr) )
    {
      count++;
      ((Attribute*) elem.addr)->DeleteIfAllowed();// consume the stream objects
    qp->Request(args[0].addr, elem);
    }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);

  qp->Close(args[0].addr);

  return 0;
}

/*
5.28.3 Specification for operator ~count~

*/
const string streamCountSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in kind DATA:\n"
  "((stream T)) -> int</text--->"
  "<text>_ count</text--->"
  "<text>Counts the number of elements of a stream.</text--->"
  "<text>query intstream (1,10) count</text--->"
  ") )";

/*
5.28.4 Selection Function of operator ~count~

*/
int
streamCountSelect (ListExpr args ) { return 0; }

/*
5.28.5 Definition of operator ~count~

*/
Operator streamcount (
  "count",           //name
  streamCountSpec,   //specification
  streamCountFun,    //value mapping
  streamCountSelect, //trivial selection function
  streamCountType    //type mapping
);


/*
5.29 Operator ~printstream~

----
    For T in kind DATA:
    (stream T) -> (stream T)

----

For every stream element, the operator calls the ~print~ function
and passes on the element.

*/

/*
5.29.1 Type mapping function for ~printstream~

*/
ListExpr
streamPrintstreamType( ListExpr args )
{
  ListExpr stream, errorInfo;
  string out;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  stream = nl->First(args);

  if ( nl->ListLength(args) != 1 )
    {
      ErrorReporter::ReportError("Operator printstream expects only a single "
                                 "argument.");
      return nl->SymbolAtom("typeerror");
    }

  // test first argument for stream(T), T in kind DATA
  if (     nl->IsAtom(stream)
           || !(nl->ListLength(stream) == 2)
           || !nl->IsEqual(nl->First(stream), "stream")
           || !am->CheckKind("DATA", nl->Second(stream), errorInfo) )
    {
      nl->WriteToString(out, stream);
      ErrorReporter::ReportError("Operator printstream expects a (stream T), "
                                 "T in kind DATA, as its first argument. "
                                 "The argument provided "
                                 "has type '" + out + "' instead.");
      return nl->SymbolAtom("typeerror");
    }

  // return the input type as result
  return stream;
}

/*
5.29.2 Value mapping for operator ~printstream~

*/
int
streamPrintstreamFun (Word* args, Word& result,
                      int message, Word& local, Supplier s)
/*
Print the elements of an Attribute-type stream.
An example for a pure stream operator (input and output are streams).

*/
{
  Word elem;

  switch( message )
    {
    case OPEN:

      qp->Open(args[0].addr);
      return 0;

    case REQUEST:

      qp->Request(args[0].addr, elem);
      if ( qp->Received(args[0].addr) )
        {
          ((Attribute*) elem.addr)->Print(cout); cout << endl;
          result = elem;
          return YIELD;
        }
      else return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
    }
  /* should not happen */
  return -1;
}

/*
5.29.3 Specification for operator ~printstream~

*/
const string streamPrintstreamSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in kind DATA:\n"
  "((stream T)) -> (stream T)</text--->"
  "<text>_ printstream</text--->"
  "<text>Prints the elements of an arbitrary stream.</text--->"
  "<text>query intstream (1,10) printstream count</text--->"
  ") )";


/*
5.29.4 Selection Function of operator ~printstream~

Uses the same function as for ~count~.

*/


/*
5.29.5 Definition of operator ~printstream~

*/
Operator streamprintstream (
  "printstream",         //name
  streamPrintstreamSpec, //specification
  streamPrintstreamFun,  //value mapping
  streamCountSelect,     //trivial selection function
  streamPrintstreamType  //type mapping
);


/*
5.30 Operator ~filter~

----
    For T in kind DATA:
    ((stream T) (map T bool)) -> (stream T)

----

The operator filters the elements of an arbitrary stream by a predicate.

*/

/*
5.30.1 Type mapping function for ~filter~

*/
ListExpr
streamFilterType( ListExpr args )
{
  ListExpr stream, map, errorInfo;
  string out, out2;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if ( nl->ListLength(args) == 2 )
    {
      stream = nl->First(args);
      map = nl->Second(args);

      // test first argument for stream(T), T in kind DATA
      if ( nl->IsAtom(stream)
           || !(nl->ListLength(stream) == 2)
           || !nl->IsEqual(nl->First(stream), "stream")
           || !am->CheckKind("DATA", nl->Second(stream), errorInfo) )
        {
          nl->WriteToString(out, stream);
          ErrorReporter::ReportError("Operator filter expects a (stream T), "
                                     "T in kind DATA as its first argument. "
                                     "The argument provided "
                                     "has type '" + out + "' instead.");
          return nl->SymbolAtom("typeerror");
        }

      // test second argument for map T' bool. T = T'
      if ( nl->IsAtom(map)
           || !nl->ListLength(map) == 3
           || !nl->IsEqual(nl->First(map), "map")
           || !nl->IsEqual(nl->Third(map), "bool") )
        {
          nl->WriteToString(out, map);
          ErrorReporter::ReportError("Operator filter expects a "
                                     "(map T bool), T in kind DATA, "
                                     "as its second argument. "
                                     "The second argument provided "
                                     "has type '" + out + "' instead.");
          return nl->SymbolAtom("typeerror");
        }

    if ( !( nl->Equal( nl->Second(stream), nl->Second(map) ) ) )
      {
        nl->WriteToString(out, nl->Second(stream));
        nl->WriteToString(out2, nl->Second(map));
        ErrorReporter::ReportError("Operator filter: the stream base type "
                                   "T must match the map's argument type, "
                                   "e.g. 1st: (stream T), 2nd: (map T bool). "
                                   "The actual types are 1st: '" + out +
                                   "', 2nd: '" + out2 + "'.");
        return nl->SymbolAtom("typeerror");
      }
    }
  else
    { // wrong number of arguments
      ErrorReporter::ReportError("Operator filter expects two arguments.");
      return nl->SymbolAtom("typeerror");
    }
  return stream; // return type of first argument
}

/*
5.30.2 Value mapping for operator ~filter~

*/
int
streamFilterFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Filter the elements of a stream by a predicate. An example for a stream
operator and also for one calling a parameter function.

*/
{
  Word elem, funresult;
  ArgVectorPointer funargs;

  switch( message )
    {
    case OPEN:

      qp->Open(args[0].addr);
      return 0;

    case REQUEST:

      funargs = qp->Argument(args[1].addr);  //Get the argument vector for
      //the parameter function.
      qp->Request(args[0].addr, elem);
      while ( qp->Received(args[0].addr) )
        {
          (*funargs)[0] = elem;
          //Supply the argument for the
          //parameter function.
          qp->Request(args[1].addr, funresult);
          //Ask the parameter function
          //to be evaluated.
          if ( ((CcBool*) funresult.addr)->GetBoolval() )
            {
              result = elem;
              return YIELD;
            }
          //consume the stream object:
        ((Attribute*) elem.addr)->DeleteIfAllowed();
        qp->Request(args[0].addr, elem); // get next element
        }
      return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
    }
  /* should not happen */
  return -1;
}

/*
5.30.3 Specification for operator ~filter~

*/
const string streamFilterSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in kind DATA:\n"
  "((stream T) (map T bool)) -> (stream T)</text--->"
  "<text>_ filter [ fun ]</text--->"
  "<text>Filters the elements of a stream by a predicate.</text--->"
  "<text>query intstream (1,10) filter[. > 7] printintstream count</text--->"
  ") )";

/*
5.30.4 Selection Function of operator ~filter~

Uses the same function as for ~count~.

*/

/*
5.30.5 Definition of operator ~filter~

*/
Operator streamfilter (
  "filter",            //name
  streamFilterSpec,   //specification
  streamFilterFun,    //value mapping
  streamCountSelect,  //trivial selection function
  streamFilterType    //type mapping
);

/*
5.41 Operator ~~

----
     (insert signature here)

----

*/

/*
5.41.1 Type mapping function for ~~

*/

/*
5.41.2 Value mapping for operator ~~

*/

/*
5.41.3 Specification for operator ~~

*/

/*
5.41.4 Selection Function of operator ~~

*/

/*
5.41.5 Definition of operator ~~

*/

/*
5.41 Operator ~~

----
     (insert signature here)

----

*/

/*
5.42.1 Type mapping function for ~~

*/

/*
5.42.2 Value mapping for operator ~~

*/

/*
5.42.3 Specification for operator ~~

*/

/*
5.42.4 Selection Function of operator ~~

*/

/*
5.42.5 Definition of operator ~~

*/

/*
5.41 Operator ~~

----
     (insert signature here)

----

*/

/*
5.43.1 Type mapping function for ~~

*/

/*
5.43.2 Value mapping for operator ~~

*/

/*
5.43.3 Specification for operator ~~

*/

/*
5.43.4 Selection Function of operator ~~

*/

/*
5.43.5 Definition of operator ~~

*/

/*
5.41 Operator ~~

----
     (insert signature here)

----

*/

/*
5.44.1 Type mapping function for ~~

*/

/*
5.44.2 Value mapping for operator ~~

*/

/*
5.44.3 Specification for operator ~~

*/

/*
5.44.4 Selection Function of operator ~~

*/

/*
5.44.5 Definition of operator ~~

*/


/*
6 Type operators

Type operators are used only for inferring argument types of parameter functions. They have a type mapping but no evaluation function.

*/

/*
6.1 Type Operator ~STREAMELEM~

This type operator extracts the type of the elements from a stream type given as the first argument and otherwise just forwards its type.

----
     ((stream T1) ...) -> T1
              (T1 ...) -> T1
----

*/
ListExpr
STREAMELEMTypeMap( ListExpr args )
{
  if(nl->ListLength(args) >= 1)
  {
    ListExpr first = nl->First(args);
    if (nl->ListLength(first) == 2)
    {
      if (nl->IsEqual(nl->First(first), "stream")) {
        return nl->Second(first);
      }
      else {
        return first;
      }
    }
    else {
      return first;
    }
  }
  return nl->SymbolAtom("typeerror");
}

const string STREAMELEMSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>((stream T1) ... ) -> T1\n"
      "(T1 ... ) -> T1</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the stream elements if the first "
      "argument is a stream and forwards the first argument's type "
      "otherwise.</text--->"
      "<text>Not for use with sos-syntax</text---> ))";

Operator STREAMELEM (
      "STREAMELEM",
      STREAMELEMSpec,
      0,
      Operator::SimpleSelect,
      STREAMELEMTypeMap );

/*
6.2 Type Operator ~STREAMELEM2~

This type operator extracts the type of the elements from the stream type within the second element within a list of argument types. Otherwise, the first arguments type is simplyforwarded.

----
     (T1 (stream T2) ...) -> T2
              (T1 T2 ...) -> T2
----

*/
ListExpr
STREAMELEM2TypeMap( ListExpr args )
{
  if(nl->ListLength(args) >= 2)
  {
    ListExpr second = nl->Second(args);
    if (nl->ListLength(second) == 2)
    {
      if (nl->IsEqual(nl->First(second), "stream")) {
        return nl->Second(second);
      }
      else {
        return second;
      }
    }
    else {
      return second;
    }
  }
  return nl->SymbolAtom("typeerror");
}

const string STREAMELEM2Spec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>(T1 (stream T2) ... ) -> T2\n"
      "( T1 T2 ... ) -> T2</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the elements from a stream given "
      "as the second argument if it is a stream. Otherwise, it forwards "
      "the original type.</text--->"
      "<text>Not for use with sos-syntax.</text---> ))";

Operator STREAMELEM2 (
      "STREAMELEM2",
      STREAMELEM2Spec,
      0,
      Operator::SimpleSelect,
      STREAMELEM2TypeMap );


/*
7 Creating the Algebra

*/

class StreamAlgebra : public Algebra
{
public:
  StreamAlgebra() : Algebra()
  {
    AddOperator( &streamcount );
    AddOperator( &streamprintstream );
    AddOperator( &streamtransformstream );
    AddOperator( &streamfeed );
    AddOperator( &streamuse );
    AddOperator( &streamuse2 );
    AddOperator( &streamaggregateS );
    AddOperator( &streamfilter );
    AddOperator( &STREAMELEM );
    AddOperator( &STREAMELEM2 );
  }
  ~StreamAlgebra() {};
};

StreamAlgebra streamAlgebra;

/*
7 Initialization

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
InitializeStreamAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&streamAlgebra);
}


