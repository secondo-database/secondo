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

April 2006, M. Spiekermann. The file Algebra.h need to be divided into Operators.h. TypeConstructors.h, 
AlgebraClassDef.h and AlgebraInit.h   

*/


#ifndef ALGEBRA_INIT_H
#define ALGEBRA_INIT_H

#include <string>
using namespace std;

class QueryProcessor;
class Algebra;
class AlgebraManager;
class DynamicLibrary;
class NestedList;

/*
Are ~forward declarations~ of used data structures and classes.

*/

typedef Algebra*
          (*AlgebraInitFunction)( NestedList* nlRef,
                                  QueryProcessor* qpRef,
                                  AlgebraManager* am );
/*
Is the prototype of the algebra initialization functions. For each
algebra the algebra manager calls an initialization  function to get
a reference to the algebra and to provide the algebra with references
to the global nested list container and the query processor.

*/


struct AlgebraListEntry
{
  AlgebraListEntry()
    : algebraId( 0 ), algebraName( "" ),
      algebraInit( 0 ), dynlib( 0 ), useAlgebra( false ) {}
  AlgebraListEntry( const int algId, const string& algName,
                    const AlgebraInitFunction algInit,
                    DynamicLibrary* const dynlibInit,
                    const bool algUse )
    : algebraId( algId ), algebraName( algName ),
      algebraInit( algInit ), dynlib( dynlibInit ), 
      useAlgebra( algUse ) {}
  int                  algebraId;
  string               algebraName;
  AlgebraInitFunction  algebraInit;
  DynamicLibrary*      dynlib;
  bool                 useAlgebra;
};
/*
Is the type for entries in the list of algebras. Each algebra has a
unique identification number ~algebraId~ and a name ~algebraName~.

Additionally the address of the initialization function of the algebra is
registered. For algebras to be used this forces the linker to include the
algebra module from an appropriate link library. For an algebra to be
loaded dynamically the address of the initialization function is not set.
Instead the method ~LoadAlgebras~ uses the algebra name to identify and
load the shared library and to identify and call the initialization
function of the algebra module. A reference to the shared library is
stored in the member variable ~dynlib~ to be able to unload the library
on termination.

Finally there is a flag whether this algebra should be included in the
initialization process or not.

*/

typedef AlgebraListEntry& (*GetAlgebraEntryFunction)( const int j );

#define ALGEBRA_LIST_START \
static AlgebraListEntry algebraList[] = {

#define ALGEBRA_LIST_END \
  AlgebraListEntry( -1, "", 0, 0, false ) };

#define ALGEBRA_LIST_INCLUDE(ALGNO,ALGNAME) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   &Initialize##ALGNAME, 0, true ),

#define ALGEBRA_LIST_EXCLUDE(ALGNO,ALGNAME) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   0, 0, false ),

#define ALGEBRA_LIST_DYNAMIC(ALGNO,ALGNAME) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   0, 0, true ),

#define ALGEBRA_PROTO_INCLUDE(ALGNO,ALGNAME) \
extern "C" Algebra* \
Initialize##ALGNAME( NestedList* nlRef,\
                     QueryProcessor* qpRef, \
                     AlgebraManager* amRef );

#define ALGEBRA_PROTO_EXCLUDE(ALGNO,ALGNAME)

#define ALGEBRA_PROTO_DYNAMIC(ALGNO,ALGNAME)

/*
These preprocessor macros allow to easily define all available algebras.
To start the list the macro "ALGEBRA\_LIST\_START"[4] is used exactly once,
and the macro "ALGEBRA\_LIST\_END"[4] is used exactly once to terminate the list.

The macros "ALGEBRA\_PROTO\_INCLUDE"[4], "ALGEBRA\_PROTO\_EXCLUDE"[4] and
"ALGEBRA\_\-PROTO\_DYNAMIC"[4] are used to create prototypes for the algebra
initialization functions, the list entries for the algebra modules to be
included or excluclass QueryProcessor;

In the algebra list definition file "AlgebraList.i"[4] the developer uses
special versions of these macros, namely "ALGEBRA\_INCLUDE"[4],
\ "ALGEBRA\_EXCLUDE"[4] and "ALGEBRA\_\-DYNAMIC"[4]. They are expanded to the
appropriate prototype and list entry macros as required. These macros have
two parameters:

  1 ~the unique identification number~ which must be a positive integer,
it is recommended but not absolutely necessary to order the entries of the
list in ascending order. No identification number may occur more than once
in the list.

  2 ~the algebra name~ which is used to build the name of the initialization
function: the algebra name is appended to the string "Initialize".

As mentioned above the list of algebras is specified in the source file
"AlgebraList.i"[4] which is included by the algebra manager source file.

Example:

----  ALGEBRA_INCLUDE(1,StandardAlgebra)
      ALGEBRA_DYNAMIC(2,FunctionAlgebra)
      ALGEBRA_EXCLUDE(3,RelationAlgebra)
----

This means:

  * the ~StandardAlgebra~ will be *included* and must be available
at link time. It has the id number "1"[2].

  * the ~FunctionAlgebra~ will be *included*, but will be loaded dynamically
at runtime. It has the id number "2"[2].

  * the ~RelationAlgebra~ will be *excluded*. It has the id number "3"[2].

*/


#endif
