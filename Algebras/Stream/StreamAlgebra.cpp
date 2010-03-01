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
//[_] [\_]
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
      projecttransformstream: stream(tuple((a1 t1) ..(an tn))) x ai -> stream(ti)

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
#include <deque>
#include <limits>
#include <sstream>
#include <vector>
#include <fstream>
#include <time.h>


#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "SecondoSystem.h"
#include "Symbols.h"
#include "NList.h"
#include "ListUtils.h"
#include "Progress.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

// #define GSA_DEBUG
#define STRALG_DEBUG false
#define DEBUGMESSAGE(MESSAGE) if(STRALG_DEBUG) cout << __PRETTY_FUNCTION__ \
         << " (" << __FILE__ << ":" << __LINE__ << ") " << MESSAGE << endl

using namespace symbols;

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

---- DATA -> stream(DATA)
----

*/
ListExpr
TypeMapStreamfeed( ListExpr args )
{
  ListExpr arg1;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if ( ( nl->ListLength(args) == 1 ) )
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
  bool sonIsObjectNode;
  bool progressinitialized;
  double* attrSize;
  double* attrSizeExt;

  SFeedLocalInfo(Attribute* arg, const bool isObject):
    finished( false ),
    sonIsObjectNode( isObject ),
    progressinitialized( false )
  {
    double coresize = arg->Sizeof();
    double flobsize = 0.0;
    for(int i=0; i < arg->NumOfFLOBs(); i++){
      flobsize += arg->GetFLOB(i)->getSize();
    }
    attrSize = new double[1];
    attrSize[0] = coresize + flobsize;
    attrSizeExt = new double[1];
    attrSizeExt[0] = coresize;
  }

  ~SFeedLocalInfo(){
    if(attrSize) {delete[] attrSize; attrSize = 0;}
    if(attrSizeExt) {delete[] attrSizeExt; attrSizeExt = 0;}
  }
};

int MappingStreamFeed( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  SFeedLocalInfo *linfo;
  Attribute* arg = (static_cast<Attribute*>(args[0].addr));

  switch( message ){
    case OPEN:{
      linfo = new SFeedLocalInfo(arg,qp->IsObjectNode(qp->GetSupplierSon(s,0)));
      local.setAddr(linfo);
      return 0;
    }
    case REQUEST:{
      if ( local.addr == 0 )
        return CANCEL;
      linfo = static_cast<SFeedLocalInfo*>(local.addr);
      if ( linfo->finished )
        return CANCEL;
      result.setAddr(arg->Clone());
      linfo->finished = true;
      return YIELD;
    }
    case CLOSE:{
      // localinfo is disposed by CLOSEPROGRESS
      return 0;
    }
    case CLOSEPROGRESS:{
      if ( local.addr )
        {
          linfo = static_cast<SFeedLocalInfo*>(local.addr);
          delete linfo;
          local.setAddr(0);
        }
      return 0;
    }
    case REQUESTPROGRESS:{
      if ( !local.addr ) {
        return CANCEL;
      }
      linfo = static_cast<SFeedLocalInfo*>(local.addr);
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;
      ProgressInfo p1;
      if( !linfo->sonIsObjectNode && qp->RequestProgress(args[0].addr, &p1) ) {
        // the son is a computed result node
        // just copy everything
        pRes->CopyBlocking(p1);
        pRes->Time = p1.Time;
      } else {
        // the son is a database object
        pRes->BTime = 0.00001; // no blocking time
        pRes->BProgress = 1.0; // non-blocking
        pRes->Time = 0.00001;  // (almost) zero runtime
      }
      if(!linfo->progressinitialized){
        pRes->Card = 1;    // cardinality
        pRes->Size = linfo->attrSize[0]; // total size
        pRes->SizeExt = linfo->attrSizeExt[0]; // size w/o FLOBS
        pRes->noAttrs = 1;    //no of attributes
        pRes->attrSize = linfo->attrSize;
        pRes->attrSizeExt = linfo->attrSizeExt;
        linfo->progressinitialized = true;
        pRes->sizesChanged = true;  //sizes have been recomputed
        linfo->progressinitialized = true;
      } else {
        pRes->sizesChanged = false;
      }
      if(linfo->finished){
        pRes->Progress = 1.0;
        pRes->Time = 0.00001;
      }
      return YIELD;
    }
  } // switch
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

  if ( (nl->ListLength( args ) != 2) )
    {
      ErrorReporter::ReportError("Operator use expects a list of length two ");
      return nl->SymbolAtom( "typeerror" );
    }

  // get use arguments
  sarg1 = nl->First( args );
  map = nl->Second( args );

  // check sarg1 for being a stream
  if(     nl->IsAtom( sarg1 )
          || ( nl->ListLength( sarg1 ) != 2)
          || !(TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    {
      ErrorReporter::ReportError(
        "Operator use expects its first Argument to "
        "be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );
    }
  sarg1Type = nl->Second(sarg1);

  // check sarg1 to be a (stream T) for T in kind DATA
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg1Type )
      && !am->CheckKind("DATA", nl->Second( sarg1Type ), errorInfo) )
    {
      nl->WriteToString(outstr1, sarg1Type);
      ErrorReporter::ReportError("Operator use expects its 1st argument "
                                 "to be '(stream T)', T of kind DATA, but"
                                 "receives '" + outstr1 + "' as T.");
      return nl->SymbolAtom( "typeerror" );
    }

  // This check can be removed when operators working on tuplestreams have
  // been implemented:
  if ( !nl->IsAtom( sarg1Type ) &&
       (nl->ListLength( sarg1Type ) == 2) &&
       nl->IsEqual( nl->First(sarg1Type), "tuple") )
    {
      ErrorReporter::ReportError("Operator use still not implemented for "
                                 "arguments of type 'tuple(X)' or "
                                 "'(stream tuple(X))'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if ( !nl->IsAtom( sarg1Type ) &&
       ( (nl->ListLength( sarg1Type ) != 2) ||
         !nl->IsEqual( nl->First(sarg1Type), "tuple") ||
         !IsTupleDescription(nl->Second(sarg1Type))
         )
       )
    {
      nl->WriteToString(outstr1, sarg1);
      return nl->SymbolAtom( "typeerror" );
    }

  // check for map
  if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map), "map") ) )
    {
      nl->WriteToString(outstr1, map);
      ErrorReporter::ReportError("Operator use expects a map as "
                                 "2nd argument, but gets '" + outstr1 +
                                 "' instead.");
      return nl->SymbolAtom( "typeerror" );
    }

  if ( nl->ListLength(map) != 3 )
    {
      ErrorReporter::ReportError("Number of map arguments must be 1 "
                                 "for operator use.");
      return nl->SymbolAtom( "typeerror" );
    }

  // get map arguments
  marg1 = nl->Second(map);
  mres  = nl->Third(map);

  // check marg1

  if ( !( nl->Equal(marg1, sarg1Type) ) )
    {
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

  // get map result type 'sresType'
  if( !( nl->IsAtom( mres ) ) && ( nl->ListLength( mres ) == 2) )
    {

      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
        {
          if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) )
            {
              ErrorReporter::ReportError(
                "Operator use expects its 2nd Argument to "
                "return a '(stream T)', T of kind DATA'.");
              return nl->SymbolAtom( "typeerror" );
            }
          sresType = mres; // map result type is already a stream
          nl->WriteToString(outstr1, sresType);
#ifdef GSA_DEBUG
          cout << "\nTypeMapUse Resulttype (1): "
               << outstr1 << "\n";
#endif
          return sresType;
        }
    }
  else // map result type is not a stream, so encapsulate it
    {
      if ( !am->CheckKind("DATA", mres, errorInfo) )
        {
          ErrorReporter::ReportError(
            "Operator use expects its 2nd Argument to "
            "return a type of kind DATA.");
          return nl->SymbolAtom( "typeerror" );
        }
      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);
      nl->WriteToString(outstr1, sresType);
#ifdef GSA_DEBUG
      cout << "\nTypeMapUse Resulttype (2): " << outstr1 << "\n";
#endif
      return sresType;
    }

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

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  // 0. Check number of arguments
  if ( (nl->ListLength( args ) != 3) )
    {
      ErrorReporter::ReportError("Operator use2 expects a list of "
                                 "length three ");
      return nl->SymbolAtom( "typeerror" );
    }

  // 1. get use arguments
  sarg1 = nl->First( args );
  sarg2 = nl->Second( args );
  map   = nl->Third( args );

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

  // 4. First and Second argument
  // check whether at least one stream argument is present
  if ( !sarg1isstream && !sarg2isstream )
    {
      ErrorReporter::ReportError(
        "Operator use2 expects at least one of its both first "
        "argument to be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );
    }

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

  // 6. Determine result type
  // get map result type 'sresType'
  if( !nl->IsAtom( mres )  && ( nl->ListLength( mres ) == 2) )
    {
      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
        {
          if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) &&
               !( !nl->IsAtom(nl->Second(mres)) &&
                  nl->ListLength(nl->Second(mres)) == 2 &&
                  TypeOfRelAlgSymbol(nl->First(nl->Second(mres)) == tuple) &&
                  IsTupleDescription(nl->Second(nl->Second(mres)))
                  )
               )
            {
              ErrorReporter::ReportError(
                "Operator use2 expects its 3rd Argument to "
                "return a '(stream T)', T of kind DATA or T = 'tuple(X)'.");
              return nl->SymbolAtom( "typeerror" );
            }
          resisstream = true;
          sresType = mres; // map result type is already a stream
        }
    }
  else // map result type is not a stream, so encapsulate it
    {
      if (    !( nl->IsAtom(mres) && am->CheckKind("DATA", mres, errorInfo))
              && !( !nl->IsAtom(mres) &&
                    nl->ListLength(mres) == 2 &&
                    !nl->IsAtom(nl->Second(mres)) &&
                    TypeOfRelAlgSymbol(nl->First(mres)  == tuple) &&
                    IsTupleDescription(nl->Second(mres))
                    )
              )
        {
          ErrorReporter::ReportError(
            "Operator use2 expects its 3rd Argument to "
            "return a type T of kind DATA or T = 'tuple(X)'.");
          return nl->SymbolAtom( "typeerror" );
        }
      resisstream = false;
      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);
    }

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

#ifdef GSA_DEBUG
      cout << "Use_SN received OPEN" << endl;
#endif
      sli = new UseLocalInfo;
      sli->Xfinished = true;
      qp->Open(instream.addr);
      sli->Xfinished = false;
      local.setAddr(sli);
#ifdef GSA_DEBUG
      cout << "Use_SN finished OPEN" << endl;
#endif
      return 0;

    case REQUEST :

      // For each REQUEST, we get one value from the stream,
      // pass it to the parameter function and evalute the latter.
      // The result is simply passed on.

#ifdef GSA_DEBUG
      cout << "Use_SN received REQUEST" << endl;
#endif
      if( local.addr == 0 )
        {
#ifdef GSA_DEBUG
          cout << "Use_SN finished REQUEST: CANCEL (1)" << endl;
#endif
          return CANCEL;
        }
      sli = (UseLocalInfo*)local.addr;

      if (sli->Xfinished)
        {
#ifdef GSA_DEBUG
          cout << "Use_SN finished REQUEST: CANCEL (2)" << endl;
#endif
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
          result.setAddr(((Attribute*) (funResult.addr))->Clone());
          ((Attribute*) (argValue.addr))->DeleteIfAllowed(); // delete argument
#ifdef GSA_DEBUG
          cout << "        result.addr    =" << result.addr << endl;
#endif
          argValue.addr = 0;
#ifdef GSA_DEBUG
          cout << "Use_SN finished REQUEST: YIELD" << endl;
#endif
          return YIELD;
        }
      else // (input stream consumed completely)
        {
          qp->Close(instream.addr);
          sli->Xfinished = true;
          result.addr = 0;
#ifdef GSA_DEBUG
          cout << "Use_SN finished REQUEST: CANCEL (3)" << endl;
#endif
          return CANCEL;
        }

    case CLOSE :

#ifdef GSA_DEBUG
      cout << "Use_SN received CLOSE" << endl;
#endif
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if ( !sli->Xfinished )
            qp->Close( instream.addr );
          delete sli;
          local.setAddr(0);
        }
#ifdef GSA_DEBUG
      cout << "Use_SN finished CLOSE" << endl;
#endif
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
      sli->X.setAddr( args[0].addr );
      sli->fun.setAddr( args[1].addr );
      sli->Xfinished   = true;
      sli->funfinished = true;
      sli->XVal.addr   = 0;
      // open the ("outer") input stream and
      qp->Open( sli->X.addr );
      sli->Xfinished = false;
      // save the local information
      local.setAddr(sli);

      return 0;

    case REQUEST :

      // For each value from the 'outer' stream, an 'inner' stream
      // of values is generated by the parameter function.
      // For each REQUEST, we pass one value from the 'inner' stream
      // as the result value.
      // If the inner stream is consumed, we try to get a new value
      // from the 'outer' stream and re-open the inner stream

#ifdef GSA_DEBUG
      cout << "\nUse_SS: Received REQUEST";
#endif
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
              result.setAddr(((Attribute*) (funResult.addr))->Clone());
              ((Attribute*) (funResult.addr))->DeleteIfAllowed();
#ifdef GSA_DEBUG
              cout << "     result.addr=" << result.addr << endl;
#endif
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
          local.setAddr(0);
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

#ifdef GSA_DEBUG
      cout << "\nUse_SNN received OPEN" << endl;
#endif
      sli = new UseLocalInfo ;
      sli->Xfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;
      sli->fun.setAddr(args[2].addr);
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
        {
          delete( sli );
          local.addr = 0;
#ifdef GSA_DEBUG
          cout << "\nUse_SNN was called with stream result mapping!" <<  endl;
#endif
          return 0;
        }
      if(sli->argConfDescriptor & 1)
        { // the first arg is the stream
          sli->X.setAddr(args[0].addr); // X is the stream
          sli->Y.setAddr(args[1].addr); // Y is the constant value
        }
      else
        { // the second arg is the stream
          sli->X.setAddr(args[1].addr); // X is the stream
          sli->Y.setAddr(args[0].addr); // Y is the constant value
        }

      qp->Open(sli->X.addr);              // open outer stream argument
      sli->Xfinished = false;

      local.setAddr(sli);
#ifdef GSA_DEBUG
      cout << "Use_SNN finished OPEN" << endl;
#endif
      return 0;

    case REQUEST :

      // For each REQUEST, we get one value from the stream,
      // pass it (and the remaining constant argument) to the parameter
      // function and evalute the latter. The result is simply passed on.
      // sli->X is the stream, sli->Y the constant argument.

#ifdef GSA_DEBUG
      cout << "Use_SNN received REQUEST" << endl;
#endif

      // 1. get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
#ifdef GSA_DEBUG
          cout << "Use_SNN finished REQUEST: CANCEL (1)" << endl;
#endif
          return CANCEL;
        }
      sli = (UseLocalInfo*) local.addr;
      if (sli->Xfinished)
        { // stream already exhausted earlier
          result.addr = 0;
#ifdef GSA_DEBUG
          cout << "Use_SNN finished REQUEST: CANCEL (2)" << endl;
#endif
          return CANCEL;
        }

      // 2. request value from outer stream
      qp->Request( sli->X.addr, xval );
      if(!qp->Received( sli->X.addr ))
        { // stream exhausted now
          qp->Close( sli->X.addr );
          sli->Xfinished = true;
#ifdef GSA_DEBUG
          cout << "Use_SNN finished REQUEST: CANCEL (3)" << endl;
#endif
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
      result.setAddr(((Attribute*) (funresult.addr))->Clone());
#ifdef GSA_DEBUG
      cout << "     result.addr=" << result.addr << endl;
#endif
      ((Attribute*) (xval.addr))->DeleteIfAllowed();
#ifdef GSA_DEBUG
      cout << "Use_SNN finished REQUEST: YIELD" << endl;
#endif
      return YIELD;

    case CLOSE :

#ifdef GSA_DEBUG
      cout << "Use_SNN received CLOSE" << endl;
#endif
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if (!sli->Xfinished)
            qp->Close( sli->X.addr ); // close input
          delete sli;
          local.setAddr(0);
        }
#ifdef GSA_DEBUG
      cout << "Use_SNN finished CLOSE" << endl;
#endif
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

#ifdef GSA_DEBUG
      cout << "\nUse_SNS received OPEN" << endl;
#endif
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
          sli->X.setAddr(args[0].addr); // X is the stream
          sli->Y.setAddr(args[1].addr); // Y is the constant value
        }
      else
        { // the second arg is the stream
          sli->X.setAddr(args[1].addr); // X is the stream
          sli->Y.setAddr(args[0].addr); // Y is the constant value
        }
      sli->YVal = sli->Y; // save value of constant argument
      qp->Open(sli->X.addr);               // open the ("outer") input stream
      sli->Xfinished = false;
      sli->fun.setAddr(args[2].addr);
      local.setAddr(sli);
#ifdef GSA_DEBUG
      cout << "Use_SNN finished OPEN" << endl;
#endif
      return 0;

    case REQUEST :

      // First, we check whether an inner stream is finished
      // (sli->funfinished). If so, we try to get a value from
      // the outer stream and try to re-open the inner stream.
      // sli->X is a pointer to the OUTER stream,
      // sli->Y is a pointer to the constant argument.

#ifdef GSA_DEBUG
      cout << "Use_SNN received REQUEST" << endl;
#endif

      // 1. get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
#ifdef GSA_DEBUG
          cout << "Use_SNN finished REQUEST: CANCEL (1)" << endl;
#endif
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
                  qp->Close(sli->X.addr);
#ifdef GSA_DEBUG
                  cout << "Use_SNN finished REQUEST: CANCEL (3)" << endl;
#endif
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
              result.setAddr(((Attribute*) (funresult.addr))->Clone());
              ((Attribute*) (funresult.addr))->DeleteIfAllowed();
#ifdef GSA_DEBUG
              cout << "     result.addr=" << result.addr << endl;
              cout << "Use_SNN finished REQUEST: YIELD" << endl;
#endif
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
#ifdef GSA_DEBUG
      cout << "Use_SNN finished REQUEST: CANCEL (4)" << endl;
#endif
      return CANCEL;

    case CLOSE :

#ifdef GSA_DEBUG
      cout << "Use_SNN received CLOSE" << endl;
#endif
      if( local.addr != 0 )
        {
          sli = (UseLocalInfo*)local.addr;
          if (!sli->funfinished)
            qp->Close( sli->fun.addr ); // close map result stream
          if (!sli->Xfinished)
            qp->Close( sli->X.addr );   // close outer stream
          delete sli;
          local.setAddr(Address(0));
        }
#ifdef GSA_DEBUG
      cout << "Use_SNN finished CLOSE" << endl;
#endif
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

#ifdef GSA_DEBUG
      cout << "\nUse_SSN received OPEN" << endl;
#endif
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
      sli->X.setAddr(args[0].addr);   // X is the stream
      sli->Y.setAddr(args[1].addr);   // Y is the constant value
      sli->fun.setAddr(args[2].addr); // fun is the mapping function

      qp->Open(sli->X.addr);            // open outer stream argument
      sli->Xfinished = false;
      local.setAddr(sli);
#ifdef GSA_DEBUG
      cout << "Use_SSN finished OPEN" << endl;
#endif
      return 0;

    case REQUEST :

      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we evaluate the
      // parameter function (sli->fun), which return a single result.
      // A clone of the result is passed as the result.
      // We also need to delete each element, when it is not required
      // anymore.

#ifdef GSA_DEBUG
      cout << "Use_SSN received REQUEST" << endl;
#endif

      // get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
#ifdef GSA_DEBUG
          cout << "Use_SSN finished REQUEST: CANCEL (1)" << endl;
#endif
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
#ifdef GSA_DEBUG
                  cout << "Use_SSN finished REQUEST: CANCEL (2)" << endl;
#endif
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
              result.setAddr(((Attribute*) (funresult.addr))->Clone());
              ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
#ifdef GSA_DEBUG
              cout << "Use_SSN finished REQUEST: YIELD" << endl;
#endif
              return YIELD;
            }
        } // end while
#ifdef GSA_DEBUG
      cout << "Use_SSN finished REQUEST: CANCEL (3)" << endl;
#endif
      return CANCEL;

    case CLOSE :

#ifdef GSA_DEBUG
      cout << "Use_SSN received CLOSE" << endl;
#endif
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
          local.setAddr(0);
        }
      result.addr = 0;
#ifdef GSA_DEBUG
      cout << "Use_SSN finished CLOSE" << endl;
#endif
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

#ifdef GSA_DEBUG
      cout << "\nUse_SSS received OPEN" << endl;
#endif
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
      local.setAddr(sli);
#ifdef GSA_DEBUG
      cout << "Use_SSS finished OPEN" << endl;
#endif
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

#ifdef GSA_DEBUG
      cout << "Use_SSS received REQUEST" << endl;
#endif

      // get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
#ifdef GSA_DEBUG
          cout << "Use_SSS finished REQUEST: CANCEL (1)" << endl;
#endif
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
#ifdef GSA_DEBUG
                  cout << "Use_SSS finished REQUEST: CANCEL (2)" << endl;
#endif
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
                  result.setAddr(((Attribute*)(funresult.addr))->Clone());
                  ((Attribute*) (funresult.addr))->DeleteIfAllowed();
#ifdef GSA_DEBUG
                  cout << "Use_SSS finished REQUEST: YIELD" << endl;
#endif
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
#ifdef GSA_DEBUG
      cout << "Use_SSS finished REQUEST: CANCEL (3)" << endl;
#endif
      return CANCEL;

    case CLOSE :

#ifdef GSA_DEBUG
      cout << "Use_SSS received CLOSE" << endl;
#endif
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
          local.setAddr(0);
        }
#ifdef GSA_DEBUG
      cout << "Use_SSS finished CLOSE" << endl;
#endif
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
  if( !nl->IsAtom(X) &&
      TypeOfRelAlgSymbol(nl->First(X)) == tuple )
    { xIsTuple = true; xIsStream = false; }
  else if( !nl->IsAtom(X) &&
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
  else
    { xIsTuple = false; xIsStream = false;}

  // examine second argument
  if( !nl->IsAtom(Y) &&
      TypeOfRelAlgSymbol(nl->First(Y)) == tuple )
    { yIsTuple = true; yIsStream = false; }
  else if( !nl->IsAtom(Y) &&
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
  else
    { yIsTuple = false; yIsStream = false;}

  // examine mapping result type
  if( !nl->IsAtom(nl->Fourth(M)) &&
      TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == tuple )
    { resIsTuple = true; resIsStream = false; }
  else if( !nl->IsAtom(nl->Fourth(M)) &&
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
  else
    { resIsTuple = false; resIsStream = false;}

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
5.24.2 Value mapping function of operator ~aggregateS~

The ~aggregateS~ operator uses a stack to compute the aggregation
balanced. This will have advantages in geometric aggregation.
It may also help to reduce numeric errors in aggregation using
double values.


5.24.2.1 ~StackEntry~

A stack entry consist of the level within the (simulated)
balanced tree and the corresponding value.
Note:
  The attributes at level 0 come directly from the input stream.
  We have to free them using the deleteIfAllowed function.
  On all other levels, the attribute is computes using the
  parameter function. Because this is outside of stream and
  tuples, no reference counting is available and we have to delete
  them using the usual delete function.

*/
struct AggrStackEntry
{
  inline AggrStackEntry(): level(-1),value(0)
  { }

  inline AggrStackEntry( long level, Attribute* value):
  level( level )
  { this->value = value;}

  inline AggrStackEntry( const AggrStackEntry& a ):
  level( a.level )
  { this->value = a.value;}

  inline AggrStackEntry& operator=( const AggrStackEntry& a )
  { level = a.level; value = a.value; return *this; }

  inline ~AggrStackEntry(){ } // use destroy !!

  inline void destroy(){
     if(level<0){
        return;
     }
     if(level==0){ // original from tuple
        value->DeleteIfAllowed();
     } else {
        delete value;
     }
     value = 0;
     level = -1;
  }

  long level;
  Attribute* value;
};

int Streamaggregate(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  // The argument vector contains the following values:
  // args[0] = stream of tuples
  // args[1] = mapping function
  // args[2] = zero value

  Word t1,resultWord;
  ArgVectorPointer vector = qp->Argument(args[1].addr);

  qp->Open(args[0].addr);
  result = qp->ResultStorage(s);
  // read the first tuple
  qp->Request( args[0].addr, t1 );

  if( !qp->Received( args[0].addr ) ){
    // special case: stream is empty
    // use the third argument as result
    ((Attribute*)result.addr)->
      CopyFrom( (const Attribute*)args[2].addr );
  } else {
    // nonempty stream, consume it
    stack<AggrStackEntry> theStack;
    while( qp->Received(args[0].addr)){
       // get the attribute
       Attribute* attr = (Attribute*)t1.addr;
       // put the attribute on the stack merging with existing entries
       // while possible
       int level = 0;
       while(!theStack.empty() && level==theStack.top().level){
         // merging is possible
         AggrStackEntry top = theStack.top();
         theStack.pop();
         // call the parameter function
         ((*vector)[0]).setAddr(top.value);
         ((*vector)[1]).setAddr(attr);
         qp->Request(args[1].addr, resultWord);
         qp->ReInitResultStorage(args[1].addr);
         top.destroy();  // remove stack content
         if(level==0){ // delete attr;
            attr->DeleteIfAllowed();
         } else {
            delete attr;
         }
         attr = (Attribute*) resultWord.addr;
         level++;
       }
       AggrStackEntry entry(level,attr);
       theStack.push(entry);
       qp->Request(args[0].addr,t1);
   }
   // stream ends, merge stack elements regardless of their level
   assert(!theStack.empty()); // at least one element must be exist
   AggrStackEntry tmpResult = theStack.top();
   theStack.pop();
   while(!theStack.empty()){
     AggrStackEntry top = theStack.top();
     theStack.pop();
     ((*vector)[0]).setAddr(top.value);
     ((*vector)[1]).setAddr(tmpResult.value);
     qp->Request(args[1].addr, resultWord);
     qp->ReInitResultStorage(args[1].addr);
     tmpResult.destroy(); // destroy temporarly result
     tmpResult.level = 1; // mark as computed
     tmpResult.value = (Attribute*) resultWord.addr;
     top.destroy();
   }
   ((Attribute*)result.addr)->
                          CopyFrom((Attribute*)tmpResult.value);
   tmpResult.destroy();
  }
  // close input stream
  qp->Close(args[0].addr);
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

----
    stream(DATA)  --> stream(tuple((elem DATA)))
    stream(tuple((attrname DATA))) --> stream(DATA)
----

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
#ifdef GSA_DEBUG
      cout << "\n In tupledescr = " << argstr << endl;
#endif
      if ( !nl->IsAtom(TupleDescr) &&
           (nl->ListLength(TupleDescr) == 1) &&
           !nl->IsAtom(nl->First(TupleDescr)) &&
           (nl->ListLength(nl->First(TupleDescr)) == 2) &&
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
5.27.3 Type Mapping for the ~namedtransformstream~ operator

This operator works as the transformstream operator. It takes a stream
of elements with kind data and produces a tuple stream, from it.
So, the value mapping is Transformstream[_]S[_]TS which is alos used by the
transformstream operator. The only difference is, additional to the
stream argument, this operator receives also a name for the attribute instead
using the defaul name 'elem'.

---- stream(DATA) x ident --> stream(tuple((ident DATA)))
----

*/
ListExpr NamedtransformstreamTypemap(ListExpr args){

  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments required");
    return nl->TypeError();
  }
  ListExpr stream = nl->First(args);
  if(nl->ListLength(stream)!=2){
    ErrorReporter::ReportError("first argument should be stream(t)");
    return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(stream),"stream")){
     ErrorReporter::ReportError("First argument must be a stream");
     return nl->TypeError();
  }
  ListExpr streamType = nl->Second(stream);
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if(!am->CheckKind("DATA", streamType, errorInfo)){
     ErrorReporter::ReportError("stream type not of kind DATA");
     return nl->TypeError();
  }
  // stream is korrekt, check attributename
  if(nl->AtomType(nl->Second(args))!=SymbolType){
     ErrorReporter::ReportError("wrong syntax for an attribute name");
     return nl->TypeError();
  }
  string name = nl->SymbolValue(nl->Second(args));
  if(SecondoSystem::GetCatalog()->IsTypeName(name)){
      ErrorReporter::ReportError(""+name+" is a type and can't be "
                                 "used as an attribute name ");
      return nl->TypeError();
  }
  if(SecondoSystem::GetCatalog()->IsOperatorName(name)){
      ErrorReporter::ReportError(""+name+" is an operator and can't be "
                                 "used as an attribute name ");
      return nl->TypeError();
  }

  return nl->TwoElemList(nl->SymbolAtom("stream"),
                         nl->TwoElemList(
                            nl->SymbolAtom("tuple"),
                            nl->OneElemList(
                              nl->TwoElemList(
                                nl->SymbolAtom(name),
                                streamType
                              )
                            )));
}

/*
5.27.2 Value mapping for operator ~transformstream~

*/

struct TransformstreamLocalInfo
{
  bool       finished;
  TupleType* resultTupleType;
  bool       progressinitialized;
};

// The first variant creates a tuplestream from a stream:
int Transformstream_S_TS(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  TransformstreamLocalInfo *sli;
  Word      value;
  ListExpr  resultType;
  Tuple     *newTuple;


  switch ( message ) {
    case OPEN:{
      qp->Open( args[0].addr );
      sli = new TransformstreamLocalInfo;

      resultType = GetTupleResultType( s );
      sli->resultTupleType = new TupleType( nl->Second( resultType ) );
      sli->finished = false;
      sli->progressinitialized = false;
      local.setAddr(sli);
      return 0;
    }
    case REQUEST:{
      if (local.addr == 0)
        return CANCEL;

      sli = (TransformstreamLocalInfo*) (local.addr);
      if (sli->finished)
        return CANCEL;

      result.setAddr((Attribute*)((qp->ResultStorage(s)).addr));

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
      newTuple->PutAttribute( 0, ((Attribute*)value.addr)->Copy() );
      ((Attribute*)(value.addr))->DeleteIfAllowed();
      result.setAddr(newTuple);
      return YIELD;
    }
    case CLOSE:{
      if (local.addr != 0) {
        sli = (TransformstreamLocalInfo*) (local.addr);
        if (!sli->finished){
          qp->Close( args[0].addr );
          sli->finished = true;
        }
      }
      return 0;
    }
    case CLOSEPROGRESS:{
      if (local.addr != 0)
        {
          sli = (TransformstreamLocalInfo*) (local.addr);
          sli->resultTupleType->DeleteIfAllowed();
          delete sli;
          local.setAddr(0);
        }
      return 0;
    }
    case REQUESTPROGRESS:{
      sli = (TransformstreamLocalInfo*) (local.addr);
      if( !sli ){
        return CANCEL;
      }
      ProgressInfo p1;
      ProgressInfo* pRes = (ProgressInfo*) result.addr;
      if( !qp->RequestProgress(args[0].addr, &p1) ){
        return CANCEL;
      };
      const double uProject = 0.00073; //millisecs per tuple
      const double vProject = 0.0004;  //millisecs per tuple and attribute
      pRes->Copy(p1);
      pRes->Time = p1.Time + pRes->Card *  (uProject + vProject);
      pRes->Progress = p1.Progress;  //a number between 0 and 1
      if( !sli->progressinitialized || p1.sizesChanged ) {
        pRes->sizesChanged = true;
        sli->progressinitialized = true;
      } else {
        pRes->sizesChanged = false;
      }
      return YIELD;
    }
  } // switch
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

  switch ( message ){
    case OPEN:{
      qp->Open( args[0].addr );
      sli = new TransformstreamLocalInfo;
      sli->finished = false;
      sli->progressinitialized = false;
      local.setAddr(sli);
      return 0;
    }
    case REQUEST:{
      if (local.addr == 0)
        {
          return CANCEL;
        }

      sli = (TransformstreamLocalInfo*) (local.addr);
      if (sli->finished)
        {
          return CANCEL;
        }

      qp->Request( args[0].addr, tuple );
      if (!qp->Received( args[0].addr ))
        { // input stream consumed
          qp->Close( args[0].addr );
          sli->finished = true;
          result.addr = 0;
          return CANCEL;
        }
      // extract, copy and pass value, delete tuple
      tupleptr = (Tuple*)tuple.addr;
      result.addr = tupleptr->GetAttribute(0)->Copy();
      tupleptr->DeleteIfAllowed();
      return YIELD;
    }
    case CLOSE:{
      if (local.addr != 0){
          sli = (TransformstreamLocalInfo*) (local.addr);
          if (!sli->finished){
            qp->Close( args[0].addr );
            sli->finished = true;
            // disposal of localinfo done in CLOSEPROGRESS
          }
      }
      return 0;
    }
    case CLOSEPROGRESS:{
      if (local.addr != 0)
        {
          sli = (TransformstreamLocalInfo*) (local.addr);
          if (!sli->finished)
            qp->Close( args[0].addr );
          delete sli;
          local.setAddr(0);
        }
      return 0;
    }
    case REQUESTPROGRESS:{
      sli = (TransformstreamLocalInfo*) (local.addr);
      if( !sli ){
        return CANCEL;
      }
      ProgressInfo p1;
      ProgressInfo* pRes = (ProgressInfo*) result.addr;
      if( !qp->RequestProgress(args[0].addr, &p1) ){
        return CANCEL;
      };
      const double uProject = 0.00073; //millisecs per tuple
      const double vProject = 0.0004;  //millisecs per tuple and attribute
      pRes->Copy(p1);
      pRes->Time = p1.Time + pRes->Card *  (uProject + vProject);
      pRes->Progress = p1.Progress;  //a number between 0 and 1
      if( !sli->progressinitialized || p1.sizesChanged ) {
        pRes->sizesChanged = true;
        sli->progressinitialized = true;
      } else {
        pRes->sizesChanged = false;
      }
      return YIELD;
    }
  }
  cout << __PRETTY_FUNCTION__ <<": UNKNOWN MESSAGE!" << endl;
  return -1;
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

const string NamedtransformstreamSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>stream(t) x name  -> stream (tuple(t name)))</text--->"
  "<text> _ namedtransformstream [ _ ] </text--->"
  "<text> Converts a stream to a tuple stream with"
  " given attribute name </text--->"
  "<text>query intsteam(0,100)"
       " namedtransformstream [Number] consume</text--->"
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
5.28 Operator ~projecttransformstream~

5.28.1 Type Mapping

---- stream(tuple((a1 t1) (a2 t2)...(an tn))) x ai --> stream(ti)
----

*/
ListExpr ProjecttransformstreamTM(ListExpr args){
   if(nl->ListLength(args)!=2){
       ErrorReporter::ReportError("2 arguments expected");
       return nl->SymbolAtom("typeerror");
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(nl->AtomType(arg2)!=SymbolType){
      ErrorReporter::ReportError("the second argument has"
                                 " to be an attributename");
      return nl->SymbolAtom("typeerror");
   }
   string aname = nl->SymbolValue(arg2);

   if( (nl->ListLength(arg1)!=2) || !nl->IsEqual(nl->First(arg1),"stream")){
      ErrorReporter::ReportError("stream expected");
      return nl->SymbolAtom("typeerror");
   }
   ListExpr streamtype = nl->Second(arg1);
   if( (nl->ListLength(streamtype)!=2) ||
        !nl->IsEqual(nl->First(streamtype),"tuple")){
      ErrorReporter::ReportError("stream(tuple) expected");
      return nl->SymbolAtom("typeerror");
   }
   ListExpr attributes = nl->Second(streamtype);
   int pos = 0;
   while(!nl->IsEmpty(attributes)){
      ListExpr attribute = nl->First(attributes);
      if(nl->ListLength(attribute)!=2){
          ErrorReporter::ReportError("invalid tuple representation");
          return nl->SymbolAtom("typeerror");
      }
      if(nl->IsEqual(nl->First(attribute),aname)){
        // name found -> create result list
        return nl->ThreeElemList( nl->SymbolAtom("APPEND"),
                                  nl->OneElemList(nl->IntAtom(pos)),
                                  nl->TwoElemList(
                                      nl->SymbolAtom("stream"),
                                      nl->Second(attribute)));
      }
      attributes = nl->Rest(attributes);
      pos++;
   }
   ErrorReporter::ReportError("attribute not found in tuple");
   return nl->SymbolAtom("typeerror");
}


/*
5.28.2 Value Mapping

*/

int Projecttransformstream(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  Word   tuple;
  Tuple* tupleptr;
  int pos;

  switch ( message )
    {
    case OPEN:
      qp->Open( args[0].addr );
      return 0;

    case REQUEST:
      qp->Request( args[0].addr, tuple );
      if (!qp->Received( args[0].addr ))
        { // input stream consumed
          result.addr = 0;
          return CANCEL;
        }
        // extract, copy and pass value, delete tuple
        tupleptr = (Tuple*)tuple.addr;
        pos = ((CcInt*)args[2].addr)->GetIntval();
        result.addr = tupleptr->GetAttribute(pos)->Copy();
        tupleptr->DeleteIfAllowed();
        return YIELD;

    case CLOSE:
      qp->Close(args[0].addr);
      return 0;
    }
  cerr << "Projecttransformstream: UNKNOWN MESSAGE!" << endl;
  return -1;
}

/*
5.28.3 Specification

*/
const string ProjecttransformstreamSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>stream(tuple((a1 t1)...(an tn))) x an  -> (stream tn)</text--->"
  "<text>_ project transformstream [ _ ] </text--->"
  "<text> extracts an attribute from a tuple stream </text--->"
  "<text>query Staedte feed projecttransformstream"
       " [PLZ] printintstream count</text--->"
  ") )";

/*
5.28.4 Definition of the operator instance

*/
Operator projecttransformstream (
  "projecttransformstream",     //name
  ProjecttransformstreamSpec,   //specification
  Projecttransformstream,    //value mapping
  Operator::SimpleSelect, //trivial selection function
  ProjecttransformstreamTM    //type mapping
);

Operator namedtransformstream (
  "namedtransformstream",     //name
  NamedtransformstreamSpec,   //specification
  Transformstream_S_TS,    //value mapping
  Operator::SimpleSelect, //trivial selection function
  NamedtransformstreamTypemap    //type mapping
);



/*
5.29 The ~echo~ operator

*/
ListExpr EchoTypeMap(ListExpr args){
  int len = nl->ListLength(args);
  if(len!=2 && len!=3){
     ErrorReporter::ReportError("Wrong number of parameters");
     return nl->TypeError();
  }
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if(len==2){  // T x S -> T , T # stream(...)
     // check for kind DATA
     ListExpr typeToPrint = nl->Second(args);
     if(! SecondoSystem::GetAlgebraManager()
           ->CheckKind("DATA",typeToPrint,errorInfo)){
        ErrorReporter::ReportError("last arg has to be in kind DATA");
        return nl->TypeError();
     }
     // check for T# stream
     if(nl->ListLength(nl->First(args))==2 &&
        nl->IsEqual(nl->First(nl->First(args)),"stream")){
        ErrorReporter::ReportError("If the first argument is a stream, two "
                                   "further parameters are required");
        return nl->TypeError();
     }
     return nl->First(args);
  } else { // len==3
     // first argument has to be a stream
     if(nl->ListLength(nl->First(args))!=2 ||
        !nl->IsEqual(nl->First(nl->First(args)),"stream")){
        ErrorReporter::ReportError("If the first argument is a stream, two "
                                   "further parameters are required");
        return nl->TypeError();
     }
     if(!nl->IsEqual(nl->Second(args),"bool")){
       ErrorReporter::ReportError("bool expected as second argument.");
       return nl->TypeError();
     }
     ListExpr typeToPrint = nl->Third(args);
     if(! SecondoSystem::GetAlgebraManager()
           ->CheckKind("DATA",typeToPrint,errorInfo)){
        ErrorReporter::ReportError("last arg has to be in kind DATA");
        return nl->TypeError();
     }
     return nl->First(args);
  }
}

/*
5.28.2 Value Mapping for the echo operator

*/

int Echo_Stream(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
   bool each = ((CcBool*) args[1].addr)->GetBoolval();
   Attribute* s1 = (Attribute*) args[2].addr;
   Word elem;
   switch(message){
     case OPEN:
            cout << "OPEN: ";
            s1->Print(cout) << endl;
            qp->Open(args[0].addr);
            return 0;
     case REQUEST:
            if(each){
               cout << "REQUEST: ";
               s1->Print(cout) << endl;
            }
            qp->Request(args[0].addr,elem);
            if(qp->Received(args[0].addr)){
                result.setAddr(elem.addr);
                return YIELD;
            } else{
                return CANCEL;
            }
     case CLOSE:
           cout << "CLOSE: ";
           s1->Print(cout)  << endl;
           qp->Close(args[0].addr);
           return 0;
   }
   return 0;
}



int Echo_Other(Word* args, Word& result, int message,
               Word& local, Supplier s)
{
   Attribute* s1 = (Attribute*) args[1].addr;
   result.setAddr(args[0].addr);
   s1->Print(cout) << endl;
   return 0;
}

/*
5.29.3 Selection function and VM array

*/
ValueMapping echovm[] = {Echo_Stream, Echo_Other};

int EchoSelect(ListExpr args){
  if(nl->ListLength(args)==2){
     return 1;
  } else {
     return 0;
  }
}

/*
5.29.4 Specification

*/

const string EchoSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>stream(T) x bool x string  -> stream(T) \n"
  " T x string -> T , T # stream</text--->"
  "<text>_ echo [ _ ] </text--->"
  "<text> prints the given string if operator mapping is called </text--->"
  "<text>query Staedte feed echo[TRUE, \"called\"] count</text--->"
  ") )";

/*
5.29.5 Creatinmg the operator instance

*/

Operator echo( "echo",
               EchoSpec,
               2,
               echovm,
               EchoSelect,
               EchoTypeMap);


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
  bool* initializedprogress;

  switch(message){
    case OPEN:
    case CLOSE:
    case REQUEST: {
      initializedprogress = new bool(false);
      local.addr = initializedprogress;
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, elem);
      while ( qp->Received(args[0].addr) ){
        count++;
        Attribute* attr = static_cast<Attribute*>( elem.addr );
        attr->DeleteIfAllowed(); // consume the stream object
        qp->Request(args[0].addr, elem);
      }
      result = qp->ResultStorage(s);
      static_cast<CcInt*>(result.addr)->Set(true, count);
      qp->Close(args[0].addr);
      return 0;
    }
    case REQUESTPROGRESS:{
      if(!local.addr){
        return CANCEL;
      }
      ProgressInfo* pRes;
      pRes = (ProgressInfo*) result.addr;
      initializedprogress = static_cast<bool*>(local.addr);
      if(!*initializedprogress){
        pRes->Card = 1 ;                //expected cardinality
        pRes->Size = sizeof(CcInt);     //expected total size
        pRes->SizeExt = sizeof(CcInt);  //expected root+ext size (no FLOBs)
        pRes->noAttrs = 1;              //no of attributes
        pRes->attrSize = new double[1]; // the complete size
        pRes->attrSize[0] = sizeof(CcInt);
        pRes->attrSizeExt = new double[1];  //the root and extension size
        pRes->attrSizeExt[0] = sizeof(CcInt);
        pRes->sizesChanged = true;  //sizes have been recomputed
        *initializedprogress = true;
      }
      ProgressInfo p1;
      if ( qp->RequestProgress(args[0].addr, &p1) ){
        pRes->BTime = p1.Time;        // this is a blocking operator!
        pRes->BProgress = p1.Progress; // this is a blocking operator!
        pRes->Progress = p1.Progress;
        pRes->Time = p1.Time;
        return YIELD;
      } else {
        return CANCEL;
      }
      break;
    }
    case CLOSEPROGRESS:{
        if(local.addr){
          initializedprogress = static_cast<bool*>(local.addr);
          delete initializedprogress;
          local.addr = 0;
        }
        return 0;
    }
    default: {
      return -1;
    }
  }
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
           || ( ( !am->CheckKind("DATA", nl->Second(stream), errorInfo) ) &&
                ( !(nl->ListLength(nl->Second(stream)) == 2 &&
                    TypeOfRelAlgSymbol(nl->First(nl->Second(stream)))
                    == tuple)) ) )
    {
      nl->WriteToString(out, stream);
      ErrorReporter::ReportError("Operator printstream expects a (stream T), "
                                 "T in kind DATA, or (stream (tuple X) ) as "
                                 "its first argument. The argument provided "
                                 "has type '" + out + "' instead.");
      return nl->SymbolAtom("typeerror");
    }

  // append list of attribute names for tuplestream signature
    if ( nl->ListLength(nl->Second(stream)) == 2 &&
         TypeOfRelAlgSymbol(nl->First(nl->Second(stream))) == tuple
       )
  {
    ListExpr tupleType = nl->Second(nl->Second(stream));
    bool firstcall = true;
    int noAttrs = 0;
    ListExpr attrList, lastofAttrList, currAttrList;

    while( !nl->IsEmpty(tupleType) )
    {
      currAttrList = nl->First(tupleType);
      tupleType = nl->Rest(tupleType);
      if ( nl->AtomType(nl->First(currAttrList)) != SymbolType )
      { // ERROR!
        nl->WriteToString(out, stream);
        ErrorReporter::ReportError("Operator printstream expects a "
            "(stream (tuple T)), where T is a list (AttrName AttrType)+ as "
            "its first argument. The argument provided "
            "has type '" + out + "' instead.");
        return nl->SymbolAtom("typeerror");
      }
      if (firstcall)
      {
        firstcall = false;
        attrList = nl->OneElemList(nl->StringAtom(
            nl->SymbolValue(nl->First(currAttrList))));
        lastofAttrList = attrList;
      }
      else
      {
        lastofAttrList = nl->Append(lastofAttrList, nl->StringAtom(
            nl->SymbolValue(nl->First(currAttrList))));
      }
      noAttrs++;
    }
    // return stream@(noAttrs,attrList)
    ListExpr reslist =
        nl->ThreeElemList(
          nl->SymbolAtom("APPEND"),
          nl->TwoElemList(
            nl->IntAtom(noAttrs),
            attrList),
          stream);
    return reslist;
  }
  else
  { // return stream
    return stream;
  }
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

int
    streamPrintTupleStreamFun (Word* args, Word& result,
                          int message, Word& local, Supplier s)

/*
Print the elements of a Tuple-type stream.

*/
{
  Word tupleWord, elem;
  Supplier son;
  string attrName;
  switch(message)
  {
    case OPEN:
      qp->Open(args[0].addr);
      return 0;

    case REQUEST:
      qp->Request(args[0].addr, tupleWord);
      if(qp->Received(args[0].addr))
      {
        cout << "Tuple: (" << endl;
        Tuple* tuple = (Tuple*) (tupleWord.addr);
        for(int i = 0; i < tuple->GetNoAttributes(); i++)
        {
          son = qp->GetSupplier(args[2].addr, i);
          qp->Request(son, elem);
          attrName = (string)(char*)((CcString*)elem.addr)->GetStringval();
          cout << attrName << ": ";
          ((Attribute*) (tuple->GetAttribute(i)))->Print(cout);
          cout << endl;
        }
        cout << "       )" << endl;
        result = tupleWord;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE:
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}

/*
5.29.3 Specification for operator ~printstream~

*/
const string streamPrintstreamSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in kind DATA:\n"
  "((stream T)) -> (stream T)\n"
  "((stream tuple(X))) -> (stream tuple(X))</text--->"
  "<text>_ printstream</text--->"
  "<text>Prints the elements of an arbitrary stream or tuplestream.</text--->"
  "<text>query intstream (1,10) printstream count</text--->"
  ") )";


/*
5.29.4 Selection Function of operator ~printstream~

Uses the same function as for ~count~.

*/
int
    streamPrintstreamSelect (ListExpr args )
{
  ListExpr streamType = nl->Second(nl->First(args));

  if( (nl->ListLength(streamType) == 2) &&
      (nl->IsEqual(nl->First(streamType),"tuple")))
    return 0;
  else
    return 1;
}

ValueMapping streamprintstreammap[] = {
  streamPrintTupleStreamFun,
  streamPrintstreamFun
};

/*
5.29.5 Definition of operator ~printstream~

*/
Operator streamprintstream (
  "printstream",           //name
  streamPrintstreamSpec,   //specification
  2,
  streamprintstreammap,    //value mapping
  streamPrintstreamSelect, //own selection function
  streamPrintstreamType    //type mapping
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
5.41 Operator ~realstream~

----
     real x real x real -> stream(real)

----

The ~realstream~ operator takes three arguments of type ~real~.
It produces a stream of real values in range provided by the first
two arguments with a stepwide taken from the third argument.

*/

/*
5.41.1 Type mapping function for ~realstream~

*/

ListExpr realstreamTypeMap( ListExpr args ){
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength(args) == 3 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    arg3 = nl->Third(args);
    if ( nl->IsEqual(arg1, "real") && nl->IsEqual(arg2, "real") &&
         nl->IsEqual(arg3, "real") ){
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("real"));
    }
  }
  ErrorReporter::ReportError("real x real x real expected");
  return nl->TypeError();
}

/*
5.41.2 Value mapping for operator ~realstream~

*/

int
realstreamFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  struct RangeAndDiff {
    double first, last, diff;
    long iter;
    long card;
    bool initializedprogress;
    double* attrSize;
    double* attrSizeExt;

    RangeAndDiff(Word* args) {
      initializedprogress = false;

      CcReal* r1 = ((CcReal*)args[0].addr);
      CcReal* r2 = ((CcReal*)args[1].addr);
      CcReal* r3 = ((CcReal*)args[2].addr);

      iter = 0;
      bool defined = r1->IsDefined() && r2->IsDefined() && r3->IsDefined();

      if (defined) {
        first = r1->GetRealval();
        last =  r2->GetRealval();
        diff = r3->GetRealval();
      }
      else {
        first = 0;
        last = -1;
        diff = 1;
      }
      if(diff > 0.0) {
        card = ceil(fabs( (last - first) / diff ) + 1.0);
        cout << __PRETTY_FUNCTION__ << "card = " << card << endl;
      } else {
        card = 1;
      }
      attrSize = new double[1];
      attrSize[0] = sizeof(CcReal);
      attrSizeExt = new double[1];
      attrSizeExt[0] = sizeof(CcReal);
    }

    ~RangeAndDiff() {
      if(attrSize) { delete[] attrSize; attrSize = 0;}
      if(attrSizeExt) {delete[] attrSizeExt; attrSizeExt = 0;}
    }
  };

  RangeAndDiff* range_d = 0;
  double current = 0;
  double cd = 0;
  CcReal* elem = 0;

  switch( message )
  {
    case OPEN: {
        range_d = new RangeAndDiff(args);
        local.addr = range_d;
        return 0;
      }
    case REQUEST: {
        range_d = ((RangeAndDiff*) local.addr);
        cd = (double) range_d->iter * range_d->diff;
        current = range_d->first + cd;
        if(range_d->diff == 0.0){ // don't allow endless loops
          return CANCEL;
        } else if(range_d->diff < 0.0){
          if(current < range_d->last){
              return CANCEL;
          } else {
              elem = new CcReal(true,current);
              result.addr = elem;
              range_d->iter++;
              return YIELD;
          }
        } else { // diff > 0.0
          if(current > range_d->last){
              return CANCEL;
          } else {
              elem = new CcReal(true,current);
              result.addr = elem;
              range_d->iter++;
              return YIELD;
          }
        }
      }
    case CLOSE: {
      // localinfo is destroyed in CLOSEPROGRESS
        return 0;
      }
    case CLOSEPROGRESS: {
        range_d = ((RangeAndDiff*) local.addr);
        if(range_d){
          delete range_d;
          local.setAddr(0);
        }
        range_d = 0;
          return 0;
        }
    case REQUESTPROGRESS: {
          ProgressInfo* pRes = (ProgressInfo*) result.addr;
          range_d = ((RangeAndDiff*) local.addr);
          if( range_d ){
            if(!range_d->initializedprogress){
              pRes->Size = sizeof(CcReal);    //total tuple size
                                              //  (including FLOBs)
              pRes->SizeExt = sizeof(CcReal); //tuple root and extension part
              pRes->noAttrs = 1;              //no of attributes
              pRes->attrSize = range_d->attrSize; // complete size per attr
              pRes->attrSizeExt = range_d->attrSizeExt; // root +extension
                                                        // size per attr
              pRes->sizesChanged = true;      //sizes were recomputed
              range_d->initializedprogress = true;
            } else {
              pRes->sizesChanged = false;     //sizes were not recomputed
            }
            const double feedccreal = 0.001;  //milliseconds per CcReal
            pRes->Card = range_d->card;       //expected cardinality
            pRes->Time = (range_d->card) * feedccreal; //expected time, [ms]
            pRes->Progress = (double) range_d-> iter / (double) range_d->card;
            pRes->BTime = 0.00001;            // blocking time must not be 0
            pRes->BProgress = 1.0;            // blocking progress [0,1]
            return YIELD;
          } else {
            return CANCEL;
          }
        }
    default: {
      return -1; /* should not happen */
      }
  } // switch
  return -1; /* should not happen */
}

/*
5.41.3 Specification for operator ~~

*/

struct realstreamInfo : OperatorInfo
{
  realstreamInfo() : OperatorInfo()
  {
    name      = REALSTREAM;
    signature = REAL + " x " + REAL + " -> stream(real)";
    syntax    = REALSTREAM + "(_ , _, _)";
    meaning   = "Creates a stream of reals containing the numbers "
                "between the first and the second argument. The third "
                "argument defines the step width.";
    example =   "realstream(-100.0, 100.0, 0.5) count";
    supportsProgress = true;
  }
};





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
6.3 Operator ~ensure~

6.3.1 The Specification

*/

struct ensure_Info : OperatorInfo {

  ensure_Info(const string& opName) : OperatorInfo()
  {
    name =      opName;
    signature = "stream(T) x int -> bool";
    syntax =    "ensure[n]";
    meaning =   "Returns true if at least n tuples are received"
                ", otherwise false.";
  }

};


/*
6.3.2 Type mapping of operator ~ensure~

   stream(DATA) x int -> bool
   stream(tuple(...) x int -> bool

*/

ListExpr ensure_tm( ListExpr args )
{
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(!nl->IsEqual(second,"int")){
    ErrorReporter::ReportError("second argument must be of type int");
    return nl->TypeError();
  }
  if(!listutils::isDATAStream(first) &&
     !listutils::isTupleStream(first)){
    ErrorReporter::ReportError("first argument must be of type"
                               " stream(tuple(...)) or stream(DATA)");
    return nl->TypeError();
  }
  return nl->SymbolAtom(BOOL);
}

int
ensure_sf( ListExpr args )
{
  NList list(args);
  list = list.first();

  int num = 0;
  NList attrs;
  if ( list.checkStreamTuple(attrs) ) {
    num = 0;
  } else {
    num = 1;
  }
  return num;
}

/*
6.3.3 Value mapping function of operator ~ensure~

*/

template<class T>
int ensure_vm(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem(Address(0) );
  int num = StdTypes::GetInt(args[1]);

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while (num && qp->Received(args[0].addr))
  {
    static_cast<T*>( elem.addr )->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
    num--;
  }
  qp->Close(args[0].addr);

  bool ensure = (num == 0);
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>( result.addr );
  res->Set( true, ensure );
  return 0;
}


ValueMapping ensure_vms[] =
{
  ensure_vm<Tuple>,
  ensure_vm<Attribute>,
  0
};


/*
6.4 Operator ~tail~

*/

/*
6.4.1 Type Mapping for Operator ~tail~:

---
      (stream (tuple X)) x int        ---> (append TRUE (stream (tuple X)))
      (stream (tuple X)) x int x bool ---> (stream (tuple X))
      (stream T)         x int        ---> (append TRUE (stream T))
      (stream (T))       x int x bool ---> (stream T)
---

*/
ListExpr streamTypeMapTail( ListExpr args )
{
  NList type(args);
  bool doAppend = false;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  if(type.length() < 2 || type.length() > 3){
    return NList::typeError( "Expected 2 or 3 arguments.");
  }
  if(type.hasLength(3)){
    if(type.third() != NList(BOOL)){
      return NList::typeError( "Optional 3rd argument must be "
          "'bool', if specified!");
    }
  }else{ // appending the default for unspecified optional 3rd argument required
    doAppend = true;
  }
  if(type.second() != NList(INT)){
    return NList::typeError( "Expected 'int' for 2nd argument!");
  }
  // 1st argument must be a stream...
  if(!(   type.first().hasLength(2)
       && type.first().first().isSymbol(sym.STREAM()))){
    return NList::typeError( "Expected a stream as 1st argument!");
  }
  NList streamtype = type.first().second();
  // stream elements must be in kind DATA or (tuple X)
  if(   !(   streamtype.hasLength(2)
          && streamtype.first().isSymbol(sym.TUPLE())
          && IsTupleDescription(streamtype.second().listExpr())
         )
     && !(am->CheckKind("DATA",streamtype.listExpr(),errorInfo))){
    return NList::typeError( "Expected a stream of DATA or TUPLE.");
  }
  if(doAppend){
    NList resType1 =NList( NList(symbols::APPEND),
                           NList(true, false).enclose(),
                          type.first()
                         );
    return resType1.listExpr();
  }else{
    DEBUGMESSAGE("Resulttype = " << type.first().convertToString());
    return type.first().listExpr();
  }
}

// localinfo used within
// value mapping for stream(tuple(X)) x int [ x bool ]--> stream(tuple(X))
class TailLocalInfo
{
  public:
    TailLocalInfo(const int mN,
                  const bool mKeepOrder)
      : n              ( mN ),
        keepOrder      ( mKeepOrder ),
        finished       ( true ),
        bufferSize     ( 0 ),
        returnedResults( 0 ),
        buffer         ( qp->MemoryAvailableForOperator() )
      {
        // member translationTable initialized automatically
      };

      ~TailLocalInfo()
      {
    // destructor for members buffer, it and
    // translationTable will be called automatically
      };

      // Store 'tuple' within the local buffer and delete it, if allowed.
      void AppendTuple(Tuple *tuple)
      {
        buffer.AppendTuple( tuple ); // append current stream elem
        if(bufferSize == 0){
//           DEBUGMESSAGE(" Inserting the first tuple...");
          finished = false;
        }
        bufferSize++;                   // increase element counter
//         DEBUGMESSAGE(" Inserting tuple " << bufferSize << "/" << n);
        if(bufferSize > n){
//           DEBUGMESSAGE(" Queue full. Pop front.");
          translationTable.pop_front(); // remove head of buffer
        }
        // The tuplebuffer should use subsequent tupleids starting with 0 and
        // proceeding up to bufferSize.
        translationTable.push_back((TupleId)(bufferSize-1)); // append tupleId
        tuple->DeleteIfAllowed(); // delete appended element from memory
        return;
      };

      // Get the next tuple from the local buffer
      // set member ~finished~ when done
      // return 0, iff no further result exists
      Tuple* GetNextTuple()
      {
        TupleId Id;
        // Since TupeId is defined as a long. and the tuplebuffer uses
        // subsequent long values as TupleIds (starting with 0), we can simply
        // enumerate all used TupleIds starting with the first one needed.
        if(finished || returnedResults >= n){
//           DEBUGMESSAGE(" Finished " << returnedResults << "/" << n);
          finished = true;
          return ((Tuple*) 0);
        }
        if(keepOrder){
          // get first elem first
//           DEBUGMESSAGE(" Getting from front");
          Id = translationTable.front();
          translationTable.pop_front();
        }else{
          // get last elem first
//           DEBUGMESSAGE(" Getting from back");
          Id = translationTable.back();
          translationTable.pop_back();
        }
        returnedResults++;
//         DEBUGMESSAGE(" Getting tuple " << returnedResults << "/" << n);
        finished = translationTable.empty();
        return buffer.GetTuple( Id );
      };

      int n;
      bool keepOrder;
      bool finished;

  protected:
    long bufferSize;
    long returnedResults;
    TupleBuffer buffer;
    deque<TupleId> translationTable;
};

// value mapping for stream(tuple(X)) x int [ x bool ]--> stream(tuple(X))
int StreamTailTupleTreamVM(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  TailLocalInfo* li;
  Word    InputStream = args[0];
  CcInt*  CcN         = static_cast<CcInt*>(args[1].addr);
  CcBool* CcKeepOrder = static_cast<CcBool*>(args[2].addr);

  Word elem;

  switch( message )
  {
    case OPEN:{
      DEBUGMESSAGE("Start OPEN");
      if(     !CcN->IsDefined()
           || !CcKeepOrder->IsDefined()
           || !InputStream.addr
           || CcN->GetIntval() <= 0
        )
      {
        local.setAddr(0);
        DEBUGMESSAGE("End OPEN 1");
        return 0;
      } // else: consume the InputStream
      li = new TailLocalInfo( CcN->GetIntval(),
                              CcKeepOrder->GetBoolval()
                            );
      local.setAddr(li);

      // open and consume the input stream
      qp->Open(InputStream.addr);
      qp->Request(InputStream.addr, elem); // get first stream elem
      while (qp->Received(args[0].addr))
      {
        Tuple *tuple = static_cast<Tuple*>(elem.addr);
        li->AppendTuple(tuple);
        qp->Request(InputStream.addr, elem); // get next stream elem
      }
      // InputStream will be closed when calling CLOSE
      DEBUGMESSAGE("End OPEN 2");
      return 0;
    }

    case REQUEST:{
      DEBUGMESSAGE("Start REQUEST");
      if(!local.addr){ DEBUGMESSAGE("End REQUEST: CANCEL1 "); return CANCEL; }
      li = static_cast<TailLocalInfo*>(local.addr);
      if(li->finished){ DEBUGMESSAGE("End REQUEST: CANCEL2 "); return CANCEL; }
      Tuple *restuple = li->GetNextTuple();
      if(!restuple){
        DEBUGMESSAGE("End REQUEST: CANCEL3 "); return CANCEL;
      }
//       else {
//          restuple->IncReference();  // reference for the stream
//       }

      result.setAddr( restuple );
      DEBUGMESSAGE("End REQUEST: YIELD");
      return YIELD;
    }

    case CLOSE:{
      DEBUGMESSAGE("Start CLOSE");
      qp->Close(InputStream.addr);
      if(local.addr){
        li = static_cast<TailLocalInfo*>(local.addr);
        delete li;
        local.setAddr(0);
      }
      DEBUGMESSAGE("End CLOSE");
      return 0;
    }
  }
  return 0;
}

// localinfo used within value mapping for
// stream T x int [ x bool ]--> stream T, T in DATA
// This is a specialization of TailLocalInfo.
class DataTailLocalInfo: public TailLocalInfo
{
  public:

    DataTailLocalInfo(const int mN,
                      const bool mKeepOrder,
                      const ListExpr elemType)

    : TailLocalInfo( mN, mKeepOrder )
    {
      ListExpr numericElemType =
          SecondoSystem::GetCatalog()->NumericType( elemType );
      ListExpr attrExpr =
          nl->TwoElemList(nl->SymbolAtom("elem"),numericElemType);
      ListExpr tupleExpr =
          nl->TwoElemList(nl->SymbolAtom("tuple"),nl->OneElemList(attrExpr));
      bufferTupleType = new TupleType(tupleExpr);
    };

    ~DataTailLocalInfo()
    {
      if(bufferTupleType)
        bufferTupleType->DeleteIfAllowed();
    };

    // move elem into internal tuplebuffer
    void AppendElem(Attribute *elem)
    {
      Tuple *tuple = new Tuple(bufferTupleType);
      tuple->PutAttribute( 0, elem );
      AppendTuple(tuple);
      // AppendTuple(...) already calls tuple->DeleteIfAllowed()!
    };

    // return the next element from the local tuplebuffer
    Attribute* GetNextElem(){
      Tuple *tuple = GetNextTuple();
      if(tuple){
        Attribute *elem = (tuple->GetAttribute(0))->Copy();
        tuple->DeleteIfAllowed();
        return elem;
      } // else: No elem left!
      return static_cast<Attribute*>(0);
    };

  private:
    TupleType *bufferTupleType;
};

// value mapping for stream T x int [ x bool ]--> stream T, T in DATA
int StreamTailDataStreamVM(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  DataTailLocalInfo* li;
  Word    InputStream = args[0];
  CcInt*  CcN         = static_cast<CcInt*>(args[1].addr);
  CcBool* CcKeepOrder = static_cast<CcBool*>(args[2].addr);

  Word elem;

  switch( message )
  {
    case OPEN:{
      DEBUGMESSAGE("Start OPEN");
      if(     !CcN->IsDefined()
               || !CcKeepOrder->IsDefined()
               || !InputStream.addr
               || CcN->GetIntval() <= 0
        )
      {
        local.setAddr(0);
        DEBUGMESSAGE("End OPEN 1");
        return 0;
      } // else: consume the InputStream
      ListExpr elemTypeNL = nl->Second(qp->GetType( s ));
      li = new DataTailLocalInfo( CcN->GetIntval(),
                                  CcKeepOrder->GetBoolval(),
                                  elemTypeNL
                                );
      local.setAddr(li);

      // open and consume the input stream
      qp->Open(InputStream.addr);
      qp->Request(InputStream.addr, elem); // get first stream elem
      while (qp->Received(args[0].addr))
      {
        Attribute *myObj = static_cast<Attribute*>(elem.addr);
        li->AppendElem(myObj);           // store the tuple in a tuplebuffer
        qp->Request(InputStream.addr, elem); // get next stream elem
      }
      // InputStream will be closed when calling CLOSE
      DEBUGMESSAGE("End OPEN 2");
      return 0;
    }

    case REQUEST:{
      DEBUGMESSAGE("Start REQUEST");
      if(!local.addr){ DEBUGMESSAGE("End REQUEST: CANCEL1 "); return CANCEL; }
      li = static_cast<DataTailLocalInfo*>(local.addr);
      if(li->finished){ DEBUGMESSAGE("End REQUEST: CANCEL2 "); return CANCEL; }
      result.setAddr(li->GetNextElem());    // extract the object
      if(!result.addr){
        DEBUGMESSAGE("End REQUEST: CANCEL3 ");
        return CANCEL;
      }
      DEBUGMESSAGE("End REQUEST: YIELD");
      return YIELD;
    }

    case CLOSE:{
      DEBUGMESSAGE("Start CLOSE");
      qp->Close(InputStream.addr);
      if(local.addr){
        li = static_cast<DataTailLocalInfo*>(local.addr);
        delete li;
        local.setAddr(0);
      }
      DEBUGMESSAGE("End CLOSE");
      return 0;
    }
  }
  return 0;
}

ValueMapping streamtailmap[] =
{ StreamTailTupleTreamVM,
  StreamTailDataStreamVM
};

int streamTailSelect( ListExpr args )
{
  NList type(args);
  if(   type.first().second().hasLength(2)
     && type.first().second().first() == "tuple")
    return 0;
  return 1;
}

const string StreamSpecTail  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(tuple(X)) x int [ x bool] -> stream(tuple(X))\n"
    "stream(T)) x int [ x bool] -> stream(T), T in DATA</text--->"
    "<text>_ tail[ n ], _ tail[ n, keepOrder ]</text--->"
    "<text>Delivers only the last 'n' stream elements. Optional parameter "
    "'keepOrder' controls the ordering of the result. If set to TRUE (default) "
    "the original ordering is maintained. Otherwise, the tuples are returned "
    "in reverse order.</text--->"
    "<text>query ten feed head[6] tail[2] tconsume</text--->"
    ") )";

Operator streamtail( "tail",
                     StreamSpecTail,
                     2,
                     streamtailmap,
                     streamTailSelect,
                     streamTypeMapTail);




/*
6.6 Operator ~kinds~

6.6.1 Type Mapping

*/
ListExpr KindsTypeMap(const ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("Wrong number of arguments ");
    return nl->TypeError();
  } else {
    if(nl->IsEqual(nl->First(args),symbols::STRING)){
      return nl->TwoElemList(nl->SymbolAtom(symbols::STREAM),
                             nl->SymbolAtom(symbols::STRING));
    } else {
      ErrorReporter::ReportError("Wrong number of arguments ");
      return nl->TypeError();
    }
  }
}

/*
6.6.2 Value Mapping

*/

class KindsLocalInfo{
  public:
    KindsLocalInfo(CcString* name):pos(0),kinds(){
       if(!name) {
         return;
       }
       if(!name->IsDefined()){
         return;
       }
       string type = name->GetValue();
       if(!SecondoSystem::GetCatalog()->IsTypeName(type)){
         return;
       }
       int algId=0;
       int typeId = 0;
       SecondoSystem::GetCatalog()->GetTypeId(name->GetValue(),algId,typeId);
       TypeConstructor* tc = am->GetTC(algId,typeId);
       kinds = tc->GetKinds();

    }

    CcString*  nextKind(){
      if(pos>=kinds.size()){
        return 0;
      } else {
        CcString* res = new CcString(true,kinds[pos]);
        pos++;
        return res;
      }
    }

  private:
     unsigned int pos;
     vector<string> kinds;
};


int KindsVM(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{

  switch(message){
    case OPEN: {
              local.setAddr( new KindsLocalInfo(
                                   static_cast<CcString*>(args[0].addr)));
              return 0;
             }
    case REQUEST: {
              KindsLocalInfo* li = static_cast<KindsLocalInfo*>(local.addr);
              if(!li){
                 return CANCEL;
              }else{
                 result.setAddr(li->nextKind());
                 return result.addr ? YIELD : CANCEL;
              }
         }
    case CLOSE: {
              KindsLocalInfo* li = static_cast<KindsLocalInfo*>(local.addr);
              if(li){
                 delete li;
                 local.setAddr(0);
              }
              return 0;
         }
    default: return 0;
  }

}

/*
6.6.3 Specification

*/
const string KindsSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>string -> stream(string)</text--->"
      "<text>_ kinds </text--->"
      "<text>Produces a stream of strings for a given type</text--->"
      "<text>query string kinds transformstream consume</text---> ))";

Operator kinds (
      "kinds",
      KindsSpec,
      KindsVM,
      Operator::SimpleSelect,
      KindsTypeMap );

/*
6.7 Operator ~timeout~

This operator will terminate stream procesing, when it its result is requested
a specified time after opening it. Until then, is just returns the result of its
stream predecessor.

*/

/*
Type Mapping Fubnction:

----  stream(X) x real --> stream(X)
----

*/
ListExpr TimeoutTypeMap(const ListExpr args){
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("Wrong number of arguments ");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  if(nl->ListLength(first)!=2){
    return listutils::typeError("Expected stream as 1st argument.");
  }
  if(!nl->IsEqual(nl->First(first),"stream")) {
    return listutils::typeError("Expected stream as 1st argument.");
  }
  ListExpr second = nl->Second(args);
  if( !listutils::isSymbol(second, "real") ) {
    return listutils::typeError("Expected real as 2nd argument.");
  }
  return nl->First(args);
}


struct TimeoutLocalInfo {
  TimeoutLocalInfo(const double _seconds):
    useProgress( false ),
    seconds( _seconds),
    elemcounter( 0 ),
    finished( false ),
    streamisopen( false )
  {
    initial = time( 0 ); // get current time
    if(seconds < 0.0){
      seconds = 0.0;
      finished = true;
    }
  };
  bool useProgress;   // check during RequestProgress only
  time_t initial;     // the time, when the stopwatch started
  double seconds;     // the time difference for the timeout (in seconds)
  long   elemcounter; // number of already returned stream elements
  bool   finished;    // true iff finished
  bool   streamisopen;
};

int TimeoutVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  switch(message){
    case OPEN: {
              // set termination conditions
              TimeoutLocalInfo* li = new TimeoutLocalInfo(
                  (static_cast<CcReal*>(args[1].addr))->GetRealval());
              local.setAddr(li);
              qp->Open(args[0].addr);
              li->streamisopen = true;
              return 0;
             }
    case REQUEST: {
              TimeoutLocalInfo* li = static_cast<TimeoutLocalInfo*>(local.addr);
              if(!li){
                 return CANCEL;
              } else if(    li->finished
                         || (    !li->useProgress
                              && (difftime(time(0),li->initial) >= li->seconds)
                            ) ) {
                li->finished = true;
                return CANCEL;
              } else {
                qp->Request(args[0].addr,result);
                if(result.addr == 0){
                  li->finished = true;
                  return CANCEL;
                } else {
                  li->elemcounter++;
                  return YIELD;
                }
              }
            }
    case CLOSE: {
              TimeoutLocalInfo* li = static_cast<TimeoutLocalInfo*>(local.addr);
              if(li && li->streamisopen){
                 qp->Close(args[0].addr);
              }
              return 0;
         }
    case CLOSEPROGRESS: {
          TimeoutLocalInfo* li = (TimeoutLocalInfo*) local.addr;
          if ( li ) {
            delete li;
            local.setAddr(0);
          }
          return 0;
        }
    case REQUESTPROGRESS: {
          TimeoutLocalInfo* li = (TimeoutLocalInfo*) local.addr;
          if( !li ) {
            return CANCEL;
          }
          ProgressInfo *pRes;
          pRes = (ProgressInfo*) result.addr;
          ProgressInfo p1;
          double runtime = difftime(time(0),li->initial);
          li->finished = ( runtime >= li->seconds );
          if ( qp->RequestProgress(args[0].addr, &p1) ) {
            pRes->Copy(p1);
            double myprogress = runtime / li->seconds;
            if(myprogress <= 0.0){
              myprogress = 0.0000001; // avoid div/0
            }
            double mycard = li->elemcounter / myprogress;
            if(mycard <= 1){
              mycard = 1; // avoid div/0
            }
            pRes->Progress = min(max(p1.Progress, myprogress), pRes->BProgress);
            pRes->Time = min( p1.Time, li->seconds*1000 );
            if( p1.BTime > pRes->Time){
              pRes->Time = max(pRes->Time, p1.BTime);
            }
            pRes->Card = min( p1.Card, mycard); //a number between 0 and 1
          } else {
            return CANCEL;
          }
          if( !li->useProgress || p1.sizesChanged ){
            li->useProgress = true;
            pRes->sizesChanged = true;
          }
          if(li->finished){
            pRes->Progress = 1.0;
          }
          return YIELD;
        }
    default: {
        return 0;
      }
  }
}


const string TimeoutSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>stream(T) x real -> stream(T)</text--->"
      "<text>_ timeout [ Seconds ]</text--->"
      "<text>Stops stream processing after the specified time has passed. "
      "Negative arguments are interpreted as 0.</text--->"
      "<text>query intstream(1,9999999999) timeout[5.0] count</text---> ))";

Operator timeout (
      "timeout",
      TimeoutSpec,
      TimeoutVM,
      Operator::SimpleSelect,
      TimeoutTypeMap );


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
    AddOperator( &projecttransformstream );
    AddOperator( &namedtransformstream );
    AddOperator( &streamfeed );
    AddOperator( &streamuse );
    AddOperator( &streamuse2 );
    AddOperator( &streamaggregateS );
    AddOperator( &streamfilter );
    AddOperator( ensure_Info("ensure"), ensure_vms, ensure_sf, ensure_tm );
    AddOperator( &echo );
    AddOperator( realstreamInfo(), realstreamFun, realstreamTypeMap );
    AddOperator( &STREAMELEM );
    AddOperator( &STREAMELEM2 );
    AddOperator( &streamtail );
    AddOperator( &kinds);
    AddOperator( &timeout);

#ifdef USE_PROGRESS
    streamcount.EnableProgress();
    streamtransformstream.EnableProgress();
    namedtransformstream.EnableProgress();
    streamfeed.EnableProgress();
    timeout.EnableProgress();
#endif
  }

  ~StreamAlgebra() {};
};

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
  return (new StreamAlgebra());
}


