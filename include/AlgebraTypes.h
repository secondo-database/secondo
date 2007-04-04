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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Algebra Types

May 2002 Ulrich Telle

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

1.1 Overview

This module defines several types which are important not only to algebra
modules but also throughout the whole "Secondo"[3] system.

1.1 Imports, Types and Defines

*/

#ifndef ALGEBRA_TYPES_H
#define ALGEBRA_TYPES_H

#include "NestedList.h"

#ifndef TYPE_ADDRESS_DEFINED
#define TYPE_ADDRESS_DEFINED
typedef void* Address;
#endif
/*
Is the type for generic references. To use such references one need to
apply an appropriate type cast.

*/

union Word
{
//  Word()                   : addr( 0 )       {};
//  Word( Address  newaddr ) : addr( newaddr ) {};
//  Word( ListExpr newlist ) : list( newlist ) {};
//  Word( int      newival ) : ival( newival ) {};
//  Word( float    newrval ) : rval( newrval ) {};
/*
Unfortunately C++ does not allow members with constructors in unions.
Therefore some inline initialization functions ("SetWord"[4]) are defined below.

*/
  Address  addr; // generic reference
  ListExpr list; // nested list expression
  int      ival; // integer value
  float    rval; // floating point value with single precision
};
/*
Specifies a generic variant type for a "Word"[4] of memory used for "Secondo"[3]
objects. To be independent of the underlying processor architecture no
assumptions about the size of a "Word"[4] should be made but all required
variants should be defined as a separate variant. For each variant a
constructor must be added to the list of constructors.

*/

static inline Word SetWord( Address  newaddr )
                     { Word w; w.addr = newaddr; return w; };
static inline Word SetWord( ListExpr newlist )
                     { Word w; w.list = newlist; return w; };
static inline Word SetWord( int      newival )
                     { Word w; w.ival = newival; return w; };
static inline Word SetWord( float    newrval )
                     { Word w; w.rval = newrval; return w; };
/*
Are several inline initialization functions for ~Word~ instances.

*/

const int MAXARG = 20;
const int FUNMSG = 10;

/*
Is the maximal number of arguments for one operator

*/

const int OPEN        = 1;
const int REQUEST     = 2;
const int CLOSE       = 3;
const int YIELD       = 4;
const int CANCEL      = 5;
const int CARDINALITY = 6;    //unused at the moment
const int PROGRESS    = 7;

const int FAILURE = 10;


/*
Are constants for stream processing.

Operators which have a parameter function with streams as arguments need to
recognize more different messages. For this purpose we will encode the argument
number in the second byte of the integer, in general:

(argNr * FUNMSG) + <message>

Hence if the first argument of a parameter function is a stream we
will use FUNMSG+OPEN (for OPEN), FUNMSG+RQUEST (for REQUEST) etc.

*/

enum PersistDirection { ReadFrom, WriteTo, DeleteFrom };
/*
Defines whether the methods managing the persistence of object values and models
are persisting an object ("WriteTo"[4]), restoring an object ("ReadFrom"[4]),
or deleting an object ("DeleteFrom"[4]).

*/
/*
1.5 Types

*/

typedef Word ArgVector[MAXARG];
typedef ArgVector* ArgVectorPointer;
/*
Are the types for generic argument vectors for algebra functions.

*/

typedef Address Supplier;
/*
Is the type for references to a supplier of information of the operator tree.

*/

typedef int (*ValueMapping)( ArgVector args, Word& result,
                             int msg, Word& local,
                             Supplier tree );
/*
Is the type of an evaluation function.

*/

typedef ListExpr (*TypeMapping)( ListExpr typeList );
/*
Is the type of a type mapping procedure.

*/

typedef int (*SelectFunction)( ListExpr typeList );
/*
Is the type of a selection function.

*/

typedef Word (*InObject)( const ListExpr typeInfo,
                          const ListExpr valueList,
                          const int errorPos,
                          ListExpr& errorInfo,
                          bool& correct );

typedef ListExpr (*OutObject)( const ListExpr numType,
                               const Word object );

typedef Word (*ObjectCreation)( const ListExpr typeInfo );

typedef void (*ObjectDeletion)( const ListExpr typeInfo,
                                Word& object );

typedef bool (*ObjectOpen)( SmiRecord& valueRecord,
                            size_t& offset,
                            const ListExpr typeExpr,
                            Word& value );

typedef bool (*ObjectSave)( SmiRecord& valueRecord,
                            size_t& offset,
                            const ListExpr typeExpr,
                            Word& value );

typedef void (*ObjectClose)( const ListExpr typeInfo,
                             Word& object );

typedef Word (*ObjectClone)( const ListExpr typeInfo,
                             const Word& object );

typedef void* (*ObjectCast)( void* );

typedef int (*ObjectSizeof)();

/*
Are the types used for creating, deleting and initializing the
algebra objects or components of the objects and for appending new
subobjects.

This shows also the types of the generic functions for the type constructors.
This is not yet satisfactory, will be revised.

*/

typedef bool (*TypeCheckFunction)( const ListExpr type,
                                   ListExpr& errorInfo );
/*
Is the type for type checking functions, one for each type constructor.

*/

typedef ListExpr (*TypeProperty)();
/*
Is the type of property functions, one for each type constructor.

*/



#endif

