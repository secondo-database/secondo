/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science,
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

[1] Interface of the Vector-based Type Map Utilities

\tableofcontents

\noindent

1 Introduction

This file primarily declares the utility functions $vectorTypeMap()$ and
$vectorSelect()$ and the types $VectorTypeMaps$ and $NVectorTypeMaps$. This is
similar to the functions found in {\tt include/TypeMapUtils.h}, but the present
functions work on $std::vector$s instead of raw arrays. They provide two
benefits:

  1 Overloads can have different numbers of arguments.

  2 Operators can have stream-type return values.

The type $VectorTypeMaps$ is based on type names, whereas $NVectorTypeMaps$ is
based on ~NList~s. The latter additionally allows for nesting of lists, but its
interface is a bit more unclear due to the recurring appearance of ~NList(...)~.


1.1 Examples

Definition of type maps:

---- const mappings::VectorTypeMaps my_operator_maps = {
       {{CcInt::BasicType(), CcInt::BasicType()}, {CcInt::BasicType()}},
       {{CcReal::BasicType(), CcReal::BasicType()}, {CcReal::BasicType()}},
       {{CcReal::BasicType()}, {CcReal::BasicType()}},
       {{CcInt::BasicType()}, {Stream<CcInt>::BasicType(), CcInt::BasicType()}},
       {{Stream<CcInt>::BasicType(), CcInt::BasicType()}, {CcInt::BasicType()}}
     };
----

These type maps define the following overloads with the corresponding overload
indices in parentheses:

        $(0)\ op : int \times int \rightarrow int$\newline
        $(1)\ op : real \times real \rightarrow real$\newline
        $(2)\ op : real \rightarrow real$\newline
        $(3)\ op : int \rightarrow stream(int)$\newline
        $(4)\ op : stream(int) \rightarrow int$

Implementation of type mapping and selection function:

---- ListExpr MyOperatorTypeMap(ListExpr args)
     {
       return mappings::vectorTypeMap(my_operator_maps, args);
     }

     int MyOperatorSelect(ListExpr args)
     {
       return mappings::vectorSelect(my_operator_maps, args);
     }
----

The ~NList~-based type map

---- const mappings::VectorTypeMaps my_2nd_operator_maps = {
      {NList(
        NList(CcReal::BasicType()),
        NList(
          NList(Symbols::MAP()),
          NList(CcReal::BasicType()),
          NList(CcBool::BasicType())
        )
      ), NList(CcInt::BasicType())}
     };
----

defines the following signature:

        $op : real \times (real \rightarrow bool) \rightarrow int$


2 Includes, Constants, Forward and Using Declarations

*/

#ifndef __VECTOR_TYPE_MAP_UTILS_H__
#define __VECTOR_TYPE_MAP_UTILS_H__

#include "NList.h"


namespace mappings {

/*
3 Declaration of Types

A vector of type names.

*/
using TypeNameVector = std::vector<std::string>;

/*
A pair of type name vectors, where the first vector represents the argument
types and the second vector represents the result types.

*/
using VectorTypeMap = std::pair<TypeNameVector, TypeNameVector>;

/*
A vector of string-based type maps, where each element represents argument types
and the corresponding result types.

*/
using VectorTypeMaps = std::vector<VectorTypeMap>;

/*
A pair of ~NList~s, where the first ~NList~ represents the argument types and
the second ~NList~ represents the result types.

*/
using NVectorTypeMap = std::pair<NList, NList>;

/*
A vector of NList-based  type maps, where each element represents argument types
and the corresponding result types.

*/
using NVectorTypeMaps = std::vector<NVectorTypeMap>;


/*
4 Declaration of Utility Functions

Generic type mapping function.

*/
ListExpr vectorTypeMap(const VectorTypeMaps& maps, const ListExpr args);
ListExpr vectorTypeMap(const NVectorTypeMaps& maps, const ListExpr args);

/*
Generic select function.

*/
int vectorSelect(const VectorTypeMaps& maps, const ListExpr args);
int vectorSelect(const NVectorTypeMaps& maps, const ListExpr args);

} //-- namespace mappings

#endif  //-- "__VECTOR_TYPE_MAP_UTILS_H__"
