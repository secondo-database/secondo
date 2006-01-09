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

#endif

