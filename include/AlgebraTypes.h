/*
//[ue] [\"{u}]

1 Header File: AlgebraTypes

May 2002 Ulrich Telle

1.1 Overview

This module defines several types which are important not only to algebra
modules but also throughout the whole ~Secondo~ system.

*/

#ifndef ALGEBRA_TYPES_H
#define ALGEBRA_TYPES_H

#include "NestedList.h"

#ifndef TYPE_ADDRESS_DEFINED
#define TYPE_ADDRESS_DEFINED
typedef void* Address;
#endif
/*
is the type for generic references. To use such references one need to
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
Therefore some inline initialization functions (SetWord) are defined below.

*/
  Address  addr; // generic reference
  ListExpr list; // nested list expression
  int      ival; // integer value
  float    rval; // floating point value with single precision
};
/*
specifies a generic variant type for a ~Word~ of memory used for ~Secondo~
objects. To be independent of the underlying processor architecture no
assumptions about the size of a ~Word~ should be made but all required
variants should be defined as a separate variant. For each variant a
constructor must be added to the list of constructors.

*/

static inline Word SetWord( Address  newaddr ) { Word w; w.addr = newaddr; return w; };
static inline Word SetWord( ListExpr newlist ) { Word w; w.list = newlist; return w; };
static inline Word SetWord( int      newival ) { Word w; w.ival = newival; return w; };
static inline Word SetWord( float    newrval ) { Word w; w.rval = newrval; return w; };
/*
are several inline initialization functions for ~Word~ instances.

*/

enum AlgebraLevel { UndefinedLevel   = 0,
                    DescriptiveLevel = 1,
                    ExecutableLevel  = 2,
                    HybridLevel      = 3 };
/*
is an enumeration of the algebra levels.

*/

#endif

