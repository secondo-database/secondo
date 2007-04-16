/*
----
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title:  [{\Large \bf ]  [}]
//[->] [\ensuremath{\rightarrow}]

[1] Secondo GSLAlgebra


March 22, 2007 C. Duentgen created the algebra

\begin{center}
\footnotesize
\tableofcontents
\end{center}


1 Overview

This algebra provides functionality of the GSL (Gnu Scinetific Library) to the
Secondo user.



2 Includes

In addition to the normal CC includes we have to include header files
for QueryProcessor and Tuplemanager and the file which contains the
definitions of our four classes: ~CcInt~, ~CcReal~, ~CcBool~, ~CcString~.
and --- of cause --- the GSL.

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include <iostream>
#include <string>
#include <sstream>

#include <cstdlib>
#include <unistd.h>
#include <errno.h>

#include <NList.h>

#ifndef SECONDO_WIN32
#define HAVE_INLINE
#endif
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "GSLAlgebra.h"

/*
3. C++ Data Types 

3.1 Type ~randomgen~

Type ~randomgen~ is a random number generator. As an object, it also stores
the type of generation method to apply, and a random seed.

It could be expanded to become of kind SIMPLE or DATA later. The ~defined~ flag
was included to this goal.

*/

// implementations for class GslRandomgen
GslRandomgen::GslRandomgen()
{} // the simple constructor. Never use this one!

GslRandomgen::GslRandomgen( const bool def ) 
  : defined( def )
{ // the simple constructor
  if( !initialized )
  {
    InitializeTables();
  }
  myType = gsl_rng_defaultIndex;
  mySeed = gsl_rng_default_seed;
  if( def )
  {
    me = gsl_rng_alloc(GetRngType(myType));
    assert( me != NULL );
    gsl_rng_set(me, mySeed);
  }
  else
  {
    me = NULL;
  }
}

GslRandomgen::GslRandomgen( const int typeNo, const unsigned long seed ) 
  : defined( true )
{ // the fully parameterized constructor
  if( !initialized )
  {
    InitializeTables();
  }
  if(typeNo < 0 || typeNo >= gslalg_randomgeneratorTableSize)
  {
    defined = false;
    me = NULL;
  }
  else
  {
    myType = typeNo;
    const gsl_rng_type *T = GetRngType(myType);
    me = gsl_rng_alloc(T);
    assert( me != NULL );
    mySeed = seed;
    gsl_rng_set(me, mySeed);
  }
}

GslRandomgen::GslRandomgen( const GslRandomgen& other )
{ // the copy constructor
  defined = other.defined;
  myType = other.myType;
  mySeed = other.mySeed;
  if( other.me != NULL )
    me = gsl_rng_clone (other.me);
  else
    me = NULL;
}

GslRandomgen& GslRandomgen::operator= ( const GslRandomgen& other )
{ // the assignment operator
  defined = other.defined;
  myType = other.myType;
  mySeed = other.mySeed;
  if( me != NULL )
  {
    gsl_rng_free(me);
    me = NULL;
  }
  if( other.me != NULL )
  {
    me = gsl_rng_clone(other.me);
    assert( me != NULL );
  }
  else
    me = NULL;
  return *this;
}

GslRandomgen::~GslRandomgen()
{ // the destructor
  if( me != NULL )
  { 
    gsl_rng_free(me);
    me = NULL;
  }
}

inline void GslRandomgen::SetDefined( const bool def )
{ // set defined flag and ensure proper settings
  if( def && !defined && me == NULL )
  { // create a valid instance for me*
    myType = gsl_rng_defaultIndex;
    gsl_rng_free(me);
    me = gsl_rng_alloc(GetRngType(myType));
    assert( me != NULL );
    mySeed = gsl_rng_default_seed;
    gsl_rng_set(me, mySeed);
  }
  defined = def;
}

inline bool GslRandomgen::IsDefined() const
{ // return defined flag
  return defined;
}

inline size_t GslRandomgen::Sizeof() const
{ // return the size of used memory
  return sizeof( *this ) + (me == NULL ? 0 : gsl_rng_size(me));
}

inline void GslRandomgen::SetSeed( const unsigned long seed )
{
  assert( defined && me != NULL );
  mySeed = seed;
  gsl_rng_set (me, mySeed);
}

inline unsigned long GslRandomgen::GetSeed()
{
  assert( defined && me != NULL );
  return mySeed;
}

inline int GslRandomgen::GetType()
{
  assert( defined && me != NULL );
  return myType;
}

inline unsigned long int GslRandomgen::NextInt() 
{ // return next random int number from generator
  // uniformly distributed [MinRand, MaxRand]
  assert( defined && me != NULL );
  return gsl_rng_get(me);
}

inline unsigned long int GslRandomgen::MinRand() const
{ // get minimum bounding value
  assert( defined && me != NULL );
  return gsl_rng_min(me);
}

inline unsigned long int GslRandomgen::MaxRand() const
{ // get maximum bounding value
  assert( defined && me != NULL );
  return gsl_rng_max(me);
}

inline double GslRandomgen::NextReal()
{ // returns a double precision floating point number 
      // uniformly distributed in the range [0,1)
  assert( defined && me != NULL );
  return gsl_rng_uniform(me);
}

inline double GslRandomgen::NextRealPos()
{ // returns a double precision floating point number 
      // uniformly distributed in the range (0,1)
  assert( defined && me != NULL );
  return gsl_rng_uniform_pos(me);
}

inline unsigned long int GslRandomgen::NextIntN(const unsigned long int n) 
{ // returns a random integer from 0 to n-1
  assert( defined && me != NULL );
  return gsl_rng_uniform_int (me, n);
}

inline gsl_rng* GslRandomgen::GetGenerator()
{
  return me;
}

void GslRandomgen::InitializeTables()
{ // initialize table of random number generators:
  if( !GslRandomgen::initialized )
  {
    gsl_rng_env_setup(); // get default values
    // get rng types
    GslRandomgen::gslalg_randomgeneratorTable = gsl_rng_types_setup(); 
    GslRandomgen::gslalg_randomgeneratorTableSize = 0;
    GslRandomgen::gsl_rng_defaultIndex = 0;
    while(GslRandomgen::gslalg_randomgeneratorTable[
          GslRandomgen::gslalg_randomgeneratorTableSize] != NULL)
    {
      if( GslRandomgen::gslalg_randomgeneratorTable[
          GslRandomgen::gslalg_randomgeneratorTableSize]->set 
          == gsl_rng_default->set )
      {
        GslRandomgen::gsl_rng_defaultIndex =
          GslRandomgen::gslalg_randomgeneratorTableSize;
      }
      GslRandomgen::gslalg_randomgeneratorTableSize++;
    }
    GslRandomgen::initialized = true;
  }
}

const gsl_rng_type* GslRandomgen::GetRngType(int index)
{
  assert( index >= 0 && index < gslalg_randomgeneratorTableSize );
  return gslalg_randomgeneratorTable[index];
}

long GslRandomgen::rngType_getNoOfGenerators()
{
  return GslRandomgen::gslalg_randomgeneratorTableSize;
}

unsigned long GslRandomgen::rngType_getMinRand(const int index)
{
  assert(index >= 0 && index < GslRandomgen::gslalg_randomgeneratorTableSize);
  return GslRandomgen::gslalg_randomgeneratorTable[index]->max;
}

unsigned long GslRandomgen::rngType_getMaxRand(const int index)
{
  assert(index >= 0 && index < GslRandomgen::gslalg_randomgeneratorTableSize);
  return GslRandomgen::gslalg_randomgeneratorTable[index]->min;
}

string GslRandomgen::rngType_getName(const int index)
{
  assert(index >= 0 && index < GslRandomgen::gslalg_randomgeneratorTableSize);
  string res(GslRandomgen::gslalg_randomgeneratorTable[index]->name);
  return res;
}


/*
Define static variables

*/

bool GslRandomgen::initialized = false;
int GslRandomgen::gslalg_randomgeneratorTableSize = 0;
int GslRandomgen::gsl_rng_defaultIndex = 0;
const gsl_rng_type** GslRandomgen::gslalg_randomgeneratorTable = NULL;

/*
4 Secondo Datatypes
*/

/*
5 Operators

Common Type Mapping functions for this algebra:

----
    gslalg_empty2int_TM:                 -> int
    gslalg_empty2real_TM:                -> real
    gslalg_int2bool_TM:              int -> bool
    gslalg_int2int_TM:               int -> int
    gslalg_intint2bool_TM:    int x  int -> bool
    gslalg_intreal2real_TM:   int x real -> real
    gslalg_intreal2int_TM:    int x real -> int
    gslalg_real2real_TM:            real -> real
    gslalg_real2int_TM:             real -> int
    gslalg_realreal2real_TM: real x real -> real

----

*/

enum gslCcType { ccint, ccreal, ccerror, ccbool, ccstring, ccconst, ccset };
gslCcType gslTypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == "int"    ) return (ccint);
    if ( s == "real"   ) return (ccreal);
    if ( s == "bool"   ) return (ccbool);
    if ( s == "string" ) return (ccstring);
    if ( s == "const"  ) return (ccconst);
    if ( s == "set"    ) return (ccset);
  }
  return (ccerror);
}

// int x int -> bool
ListExpr gslalg_intint2bool_TM ( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( gslTypeOfSymbol( arg1 ) == ccint && gslTypeOfSymbol( arg2 ) == ccint )
    {
      return (nl->SymbolAtom( "bool" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected int x int.");
  return (nl->SymbolAtom( "typeerror" ));
}

// () -> int
ListExpr gslalg_empty2int_TM ( ListExpr args )
{
  if ( !nl->IsEmpty( args ) ) 
  {
    ErrorReporter::ReportError("GSLAlgebra: Expected empty list.");
    return (nl->SymbolAtom( "typeerror" ));
  } 
  else 
  {
    return (nl->SymbolAtom( "int" ));
  }
}

// () -> real
ListExpr gslalg_empty2real_TM ( ListExpr args )
{
  if ( !nl->IsEmpty( args ) ) 
  {
    ErrorReporter::ReportError("GSLAlgebra: Expected empty list.");
    return (nl->SymbolAtom( "typeerror" ));
  } 
  else 
  {
    return (nl->SymbolAtom( "real" ));
  }
}

// int -> bool
ListExpr gslalg_int2bool_TM ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( gslTypeOfSymbol( arg1 ) == ccint )
    {
      return (nl->SymbolAtom( "bool" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected int.");
  return (nl->SymbolAtom( "typeerror" ));
}

// int -> int
ListExpr gslalg_int2int_TM ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( gslTypeOfSymbol( arg1 ) == ccint )
    {
      return (nl->SymbolAtom( "int" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected int.");
  return (nl->SymbolAtom( "typeerror" ));
}

// real -> real
ListExpr gslalg_real2real_TM ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( gslTypeOfSymbol( arg1 ) == ccreal )
    {
      return (nl->SymbolAtom( "real" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected real.");
  return (nl->SymbolAtom( "typeerror" ));
}

// real -> int
ListExpr gslalg_real2int_TM ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( gslTypeOfSymbol( arg1 ) == ccreal )
    {
      return (nl->SymbolAtom( "int" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected real.");
  return (nl->SymbolAtom( "typeerror" ));
}

// int -> string
ListExpr gslalg_int2string_TM ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( gslTypeOfSymbol( arg1 ) == ccint )
    {
      return (nl->SymbolAtom( "string" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected int.");
  return (nl->SymbolAtom( "typeerror" ));
}

// real x real -> real
ListExpr gslalg_realreal2real_TM ( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( gslTypeOfSymbol( arg1 ) 
         == ccreal && gslTypeOfSymbol( arg2 ) == ccreal )
    {
      return (nl->SymbolAtom( "real" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected real x real.");
  return (nl->SymbolAtom( "typeerror" ));
}

// int x real -> real
ListExpr gslalg_intreal2real_TM ( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( gslTypeOfSymbol( arg1 ) == ccint && gslTypeOfSymbol( arg2 ) == ccreal )
    {
      return (nl->SymbolAtom( "real" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected int x real.");
  return (nl->SymbolAtom( "typeerror" ));
}

// int x real -> real
ListExpr gslalg_intreal2int_TM ( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( gslTypeOfSymbol( arg1 ) == ccint && gslTypeOfSymbol( arg2 ) == ccreal )
    {
      return (nl->SymbolAtom( "int" ));
    }
  }
  ErrorReporter::ReportError("GSLAlgebra: Expected int x real.");
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.0 Operator ~rng\_init~

Create and initialize the random number generator.
Always returns `TRUE'.

----
    rng_init: int x int --> bool
    
----

*/

// define static variables
// the static generator instance - initialize with defaults
static GslRandomgen the_gsl_randomgenerator( true );

int gslalg_rng_init_VM ( Word* args, Word& result, int message, 
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = ((CcBool*)result.addr);
  CcInt *arg1 = ((CcInt*)args[0].addr);
  CcInt *arg2 = ((CcInt*)args[1].addr);
  if ( arg1->IsDefined() && arg2->IsDefined() )
  {
    // overflow save implementation
    long  type = arg1->GetIntval();
    unsigned long seed = arg2->GetIntval();
    if( type >= 0 && type < GslRandomgen::rngType_getNoOfGenerators() )
    {
      the_gsl_randomgenerator = GslRandomgen(type, seed);
      res->Set(true, true);
    }
    else
      res->Set(true, false);
  }
  else
    res->Set(true, false);
  return (0);
}

const string gslalg_rng_init_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>int x int -> bool</text--->"
    "<text>rng_init( type, seed )</text--->"
    "<text>Initialize the GSL random generator. Load generator type 'type' "
    "and use random seed 'seed'.</text--->"
    "<text>query rng_init(0, 0)</text--->"
    ") )";

Operator gslalg_rng_init( 
    "rng_init",
    gslalg_rng_init_Spec,
    gslalg_rng_init_VM,
    Operator::SimpleSelect,
    gslalg_intint2bool_TM) ;


/*
5.1 Operator ~rng\_int~

Get the next int random number

*/

int gslalg_rng_int_VM ( Word* args, Word& result, 
                        int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long resL = the_gsl_randomgenerator.NextInt(); // implicit conversion!
  res->Set(true, resL);
  return (0);
}

const string gslalg_rng_int_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>   -> int</text--->"
    "<text>rng_int()</text--->"
    "<text>Draw a uniformly distributed integer variate from the GSL random "
    "number generator. To get minimum and maximum boundary for "
    "random numbers, use rng_getMin() and rng_getMax().</text--->"
    "<text>query randint()</text--->"
    ") )";

Operator gslalg_rng_int( 
    "rng_int",
    gslalg_rng_int_Spec,
    gslalg_rng_int_VM,
    Operator::SimpleSelect,
    gslalg_empty2int_TM) ;


/*
5.2 Operator ~rng\_intN~

Get the next int random number

*/

int gslalg_rng_intN_VM ( Word* args, Word& result, 
                        int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  CcInt *cN = ((CcInt*) args[0].addr);
  long resL = 0;
  if( cN->IsDefined() )
  {
    long N = cN->GetIntval();
    resL = the_gsl_randomgenerator.NextIntN( N ); // implicit conversion!
    res->Set(true, resL);
  }
  else
    res->Set(false, resL);
  return (0);
}

const string gslalg_rng_intN_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int -> int</text--->"
    "<text>rng_intN( N )</text--->"
    "<text>Draw a uniformly distributed integer variate from [0,N-1] using "
    "the GSL random number generator.</text--->"
    "<text>query rng_intN(6)+1</text--->"
    ") )";

Operator gslalg_rng_intN( 
    "rng_intN",
    gslalg_rng_intN_Spec,
    gslalg_rng_intN_VM,
    Operator::SimpleSelect,
    gslalg_int2int_TM) ;


/*
5.3 Operator ~rng\_real~

Get the next real random number 

*/

int gslalg_rng_real_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal *res = (CcReal*) result.addr;
  double resD = the_gsl_randomgenerator.NextReal();
  res->Set(true, resD);
  return (0);
}

const string gslalg_rng_real_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>    -> real</text--->"
    "<text>rng_real( )</text--->"
    "<text>Draw a uniformly distributed real variate from [0,1) using "
    "the GSL random number generator.</text--->"
    "<text>query floor((rng_real()*6))+1</text--->"
    ") )";

Operator gslalg_rng_real( 
    "rng_real",
    gslalg_rng_real_Spec,
    gslalg_rng_real_VM,
    Operator::SimpleSelect,
    gslalg_empty2real_TM) ;

/*
5.4 Operator ~rng\_realpos~

Get the next positive real random number 

*/

int gslalg_rng_realpos_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal *res = (CcReal*) result.addr;
  double resD = the_gsl_randomgenerator.NextRealPos();
  res->Set(true, resD);
  return (0);
}

const string gslalg_rng_realpos_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>   -> real</text--->"
    "<text>rng_realpos( )</text--->"
    "<text>Draw a uniformly distributed real variate from (0,1) using "
    "the GSL random number generator.</text--->"
    "<text>query floor((rng_realpos()*6))+1</text--->"
    ") )";

Operator gslalg_rng_realpos( 
    "rng_realpos",
    gslalg_rng_realpos_Spec,
    gslalg_rng_realpos_VM,
    Operator::SimpleSelect,
    gslalg_empty2real_TM) ;

/*
5.5 Operator ~rng\_getMin~

Get the minimum random number

*/

int gslalg_rng_getMin_VM ( Word* args, Word& result, 
                        int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long resL = the_gsl_randomgenerator.MinRand(); // implicit conversion!
  res->Set(true, resL);
  return (0);
}

const string gslalg_rng_getMin_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>   -> int</text--->"
    "<text>rng_getMin()</text--->"
    "<text>Get the minimum integer, that is generated by the GSL random "
    "number generator.</text--->"
    "<text>query rng_getMin()</text--->"
    ") )";

Operator gslalg_rng_getMin( 
    "rng_getMin",
    gslalg_rng_getMin_Spec,
    gslalg_rng_getMin_VM,
    Operator::SimpleSelect,
    gslalg_empty2int_TM) ;

/*
5.6 Operator ~rng\_getMax~

Get the maximum random number

*/
int gslalg_rng_getMax_VM ( Word* args, Word& result, 
                           int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = (CcInt*) result.addr;
  long resL = the_gsl_randomgenerator.MaxRand(); // implicit conversion!
  res->Set(true, resL);
  return (0);
}

const string gslalg_rng_getMax_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>   -> int</text--->"
    "<text>rng_getMax()</text--->"
    "<text>Get the maximum integer, that is generated by the GSL random "
    "number generator.</text--->"
    "<text>query rng_getMax()</text--->"
    ") )";

Operator gslalg_rng_getMax( 
    "rng_getMax",
    gslalg_rng_getMax_Spec,
    gslalg_rng_getMax_VM,
    Operator::SimpleSelect,
    gslalg_empty2int_TM) ;

/*
5.7 Operator ~rng\_setSeed~

Set a new seed for the specified random number generator and reinitialize it.

*/

int gslalg_rng_setSeed_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = ((CcBool*) result.addr);
  CcInt *cN = ((CcInt*) args[0].addr);
  if( cN->IsDefined() )
  {
    long N = cN->GetIntval();
    the_gsl_randomgenerator.SetSeed( N ); // implicit conversion!
    res->Set(true, true);
  }
  else
    res->Set(true, false);
  return (0);
}

const string gslalg_rng_setSeed_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int -> bool</text--->"
    "<text>rng_setSeed( N )</text--->"
    "<text>Reinitialize the GSL random number generator with seed N.</text--->"
    "<text>query rng_setSeed(6)</text--->"
    ") )";

Operator gslalg_rng_setSeed( 
    "rng_setSeed",
    gslalg_rng_setSeed_Spec,
    gslalg_rng_setSeed_VM,
    Operator::SimpleSelect,
    gslalg_int2bool_TM) ;


/*
5.7 Operator ~rng\_getSeed~

get the seed value for the random number generator.

*/

int gslalg_rng_getSeed_VM ( Word* args, Word& result, 
                            int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long N = the_gsl_randomgenerator.GetSeed();
  res->Set(true, N);
  return (0);
}

const string gslalg_rng_getSeed_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>    -> int</text--->"
    "<text>rng_getSeed( N )</text--->"
    "<text>Get the seed used for the GSL random number generator.</text--->"
    "<text>query rng_getSeed()</text--->"
    ") )";


Operator gslalg_rng_getSeed( 
    "rng_getSeed",
    gslalg_rng_getSeed_Spec,
    gslalg_rng_getSeed_VM,
    Operator::SimpleSelect,
    gslalg_empty2int_TM) ;


/*
5.8 Operator ~rng\_flat~

Get a flat (uniform) distributed variate from randomgen

*/

int gslalg_rng_flat_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal *res = ((CcReal*) result.addr);
  double resD = 0;
  CcReal *cA = (CcReal*) args[0].addr;
  CcReal *cB = (CcReal*) args[1].addr;
  if( cA->IsDefined() && cB->IsDefined() )
  {
    double a = cA->GetRealval();
    double b = cB->GetRealval();
    if( a <= b )
    {
      resD = gsl_ran_flat(the_gsl_randomgenerator.GetGenerator(), a, b);
      res->Set(true, resD);
    }
    else
    {
      res->Set(false, 0);
    }
  }
  else
    res->Set(false, 0);
  return (0);
}

const string gslalg_rng_flat_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real x real -> real</text--->"
    "<text>rng_flat( Min, Max )</text--->"
    "<text>Get a flat (uniform) distributed variate from [Min, Max] using "
    "the GSL random number generator.</text--->"
    "<text>query rng_flat( -5.5, 14.7 )</text--->"
    ") )";

Operator gslalg_rng_flat( 
    "rng_flat",
    gslalg_rng_flat_Spec,
    gslalg_rng_flat_VM,
    Operator::SimpleSelect,
    gslalg_realreal2real_TM) ;


/*
5.9 Operator ~rng\_binomial~

Get a binomial distributed variate from randomgen

*/

//Operator gslalg_rng_binomial;

int gslalg_rng_binomial_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long resL = 0;
  CcInt  *cN = (CcInt*) args[0].addr;
  CcReal *cP = (CcReal*) args[1].addr;
  if( cN->IsDefined() && cP->IsDefined() )
  {
    double p = cP->GetRealval();
    long n = cN->GetIntval();
    if( n >= 0 && p >= 0 && p <= 1.0 )
    {
      resL = gsl_ran_binomial(the_gsl_randomgenerator.GetGenerator(), p, n);
      res->Set(true, resL);
    }
    else
    {
      res->Set(false, 0);
    }
  }
  else
    res->Set(false, 0);
  return (0);
}

const string gslalg_rng_binomial_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int x real -> int</text--->"
    "<text>rng_binomial( N, P )</text--->"
    "<text>This function returns a random integer from the binomial "
    "distribution, the number of successes in N independent trials "
    "with probability P, using the GSL random generator.</text--->"
    "<text>query rng_binomial( 10, 0.4 )</text--->"
    ") )";

Operator gslalg_rng_binomial( 
    "rng_binomial",
    gslalg_rng_binomial_Spec,
    gslalg_rng_binomial_VM,
    Operator::SimpleSelect,
    gslalg_intreal2int_TM) ;



/*
5.10 Operator ~rng\_gaussian~

Get a gaussian distributed variate from randomgen

*/

int gslalg_rng_gaussian_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal *res = ((CcReal*) result.addr);
  double resD = 0;
  CcReal *cSigma = (CcReal*) args[0].addr;
  if( cSigma->IsDefined() )
  {
    double sigma = cSigma->GetRealval();
    resD = gsl_ran_gaussian(the_gsl_randomgenerator.GetGenerator(), sigma);
    res->Set(true, resD);
  }
  else
    res->Set(false, 0);
  return (0);
}

const string gslalg_rng_gaussian_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real -> real</text--->"
    "<text>rng_gaussian( Sigma )</text--->"
    "<text>Get a Gaussian distributed real variate with mean zero "
    "and standard deviation Sigma using "
    "the GSL random number generator.</text--->"
    "<text>query rng_gaussian( 0.4 )</text--->"
    ") )";


Operator gslalg_rng_gaussian( 
    "rng_gaussian",
    gslalg_rng_gaussian_Spec,
    gslalg_rng_gaussian_VM,
    Operator::SimpleSelect,
    gslalg_real2real_TM) ;

/*
5.11 Operator ~rng\_exponential~

Get a gaussian distributed variate from randomgen

*/

int gslalg_rng_exponential_VM ( Word* args, Word& result, 
                             int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal *res = (CcReal*) result.addr;
  CcReal *cMu = (CcReal*) args[0].addr;
  if( cMu->IsDefined() )
  {
    double Mu = cMu->GetRealval();
    double resD = gsl_ran_exponential(the_gsl_randomgenerator.GetGenerator(), 
                                      Mu);
    res->Set(true, resD);
  }
  else
    res->Set(false, 0);
  return (0);
}

const string gslalg_rng_exponential_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real -> real</text--->"
    "<text>rng_exponential( Mu )</text--->"
    "<text>Get a exponentially distributed real variate with mean Mu "
    "using the GSL random number generator.</text--->"
    "<text>query rng_exponential( 0.4 )</text--->"
    ") )";


Operator gslalg_rng_exponential( 
    "rng_exponential",
    gslalg_rng_exponential_Spec,
    gslalg_rng_exponential_VM,
    Operator::SimpleSelect,
    gslalg_real2real_TM) ;


/*
5.12 Operator ~rng\_getType~

get the random generator' type index

*/

int gslalg_rng_getType_VM ( Word* args, Word& result, 
                            int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long N = the_gsl_randomgenerator.GetType();
  res->Set(true, N);
  return (0);
}

const string gslalg_rng_getType_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>    -> int</text--->"
    "<text>rng_getType( N )</text--->"
    "<text>Get the random generator's type index used for the GSL "
    "random number generator.</text--->"
    "<text>query rng_getType()</text--->"
    ") )";


Operator gslalg_rng_getType( 
    "rng_getType",
    gslalg_rng_getType_Spec,
    gslalg_rng_getType_VM,
    Operator::SimpleSelect,
    gslalg_empty2int_TM) ;


/*
5.13 Operator ~rng\_NoGenerators~

return the number of available GSL random number generator types.

*/

int gslalg_rng_NoGenerators_VM ( Word* args, Word& result, 
                            int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long N = GslRandomgen::rngType_getNoOfGenerators();
  res->Set(true, N);
  return (0);
}

const string gslalg_rng_NoGenerators_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> -> int</text--->"
    "<text>rng_NoGenerators( N )</text--->"
    "<text>Get the number of available GSL random number "
    "generator types.</text--->"
    "<text>query rng_NoGenerators()</text--->"
    ") )";

Operator gslalg_rng_NoGenerators( 
    "rng_NoGenerators",
    gslalg_rng_NoGenerators_Spec,
    gslalg_rng_NoGenerators_VM,
    Operator::SimpleSelect,
    gslalg_empty2int_TM) ;


/*
5.14 Operator ~rng\_GeneratorName~

return the number of available GSL random number generator types.

*/

int gslalg_rng_GeneratorName_VM ( Word* args, Word& result, 
                                 int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcString *res = ((CcString*) result.addr);
  CcInt *cIndex = (CcInt*) args[0].addr;
  if( cIndex->IsDefined() )
  {
    int index = cIndex->GetIntval();
    if( index >=0 && index < GslRandomgen::rngType_getNoOfGenerators() )
    {
      res->Set( true, (GslRandomgen::rngType_getName(index)) );
    }
    else
      res->SetDefined( false );
  }
  else
    res->SetDefined( false );
  return (0);
}

const string gslalg_rng_GeneratorName_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int -> string</text--->"
    "<text>rng_GeneratorName( N )</text--->"
    "<text>Get the name of the GSL random number "
    "generator type with index N.</text--->"
    "<text>query rng_GeneratorName( 1 )</text--->"
    ") )";

Operator gslalg_rng_GeneratorName( 
    "rng_GeneratorName",
    gslalg_rng_GeneratorName_Spec,
    gslalg_rng_GeneratorName_VM,
    Operator::SimpleSelect,
    gslalg_int2string_TM) ;


/*
5.15 Operator ~rng\_GeneratorMinRand~

Return the minimum number generated by the specified GSL random number 
generator type.

*/

int gslalg_rng_GeneratorMinRand_VM ( Word* args, Word& result, 
                                  int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  CcInt *cIndex = (CcInt*) args[0].addr;
  if( cIndex->IsDefined() )
  {
    int index = cIndex->GetIntval();
    if( index >=0 && index < GslRandomgen::rngType_getNoOfGenerators() )
    {
      res->Set( true, (GslRandomgen::rngType_getMinRand(index)) );
    }
    else
      res->SetDefined( false );
  }
  else
    res->SetDefined( false );
  return (0);
}

const string gslalg_rng_GeneratorMinRand_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int -> int</text--->"
    "<text>rng_GeneratorMinRand( N )</text--->"
    "<text>Get the minimum number generated by the specified GSL "
    "random number generator type N.</text--->"
    "<text>query rng_GeneratorMinRand( 1 )</text--->"
    ") )";

Operator gslalg_rng_GeneratorMinRand( 
    "rng_GeneratorMinRand",
    gslalg_rng_GeneratorMinRand_Spec,
    gslalg_rng_GeneratorMinRand_VM,
    Operator::SimpleSelect,
    gslalg_int2int_TM) ;

/*
5.16 Operator ~rng\_GeneratorMaxRand~

Return the maximum number generated by the specified GSL random number 
generator type.

*/

int gslalg_rng_GeneratorMaxRand_VM ( Word* args, Word& result, 
                                  int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  CcInt *cIndex = (CcInt*) args[0].addr;
  if( cIndex->IsDefined() )
  {
    int index = cIndex->GetIntval();
    if( index >=0 && index < GslRandomgen::rngType_getNoOfGenerators() )
    {
      res->Set( true, (GslRandomgen::rngType_getMaxRand(index)) );
    }
    else
      res->SetDefined( false );
  }
  else
    res->SetDefined( false );
  return (0);
}

const string gslalg_rng_GeneratorMaxRand_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int -> int</text--->"
    "<text>rng_GeneratorMaxRand( N )</text--->"
    "<text>Get the maximum number generated by the specified GSL "
    "random number generator type N.</text--->"
    "<text>query rng_GeneratorMaxRand( 1 )</text--->"
    ") )";

Operator gslalg_rng_GeneratorMaxRand( 
    "rng_GeneratorMaxRand",
    gslalg_rng_GeneratorMaxRand_Spec,
    gslalg_rng_GeneratorMaxRand_VM,
    Operator::SimpleSelect,
    gslalg_int2int_TM) ;

/*
5.17 Operator ~rng\_TypeDescriptors~

get a stream of tuples describing all available GSL random number generators.
tuple format is tuple((Index int)(MinInt int)(MaxInt int)(Name string)).

*/

int rng_TypeDescriptors_VM ( Word* args, Word& result, 
                            int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long N = the_gsl_randomgenerator.GetType();
  res->Set(true, N);
  return (0);
}

const string rng_TypeDescriptors_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>    -> stream(tuple((Index int)(MinInt int)(MaxInt int)"
    "(Name string)))</text--->"
    "<text>rng_TypeDescriptors( )</text--->"
    "<text>Get a tuple stream of all available GSL random number "
    "generators.</text--->"
    "<text>query rng_TypeDescriptors( ) consume</text--->"
    ") )";

Operator rng_TypeDescriptors( 
    "rng_TypeDescriptors",
    gslalg_rng_getType_Spec,
    gslalg_rng_getType_VM,
    Operator::SimpleSelect,
    gslalg_empty2int_TM) ;


/*
5.18 Operator ~rng\_poisson~

Get a poisson distributed int variate from randomgen

*/

int gslalg_rng_poisson_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long resD = 0;
  CcReal *cA = (CcReal*) args[0].addr;
  if( cA->IsDefined() )
  {
    double a = cA->GetRealval();
    resD = gsl_ran_poisson(the_gsl_randomgenerator.GetGenerator(), a);
    res->Set(true, resD);
  }
  else
    res->Set(false, 0);
  return (0);
}

const string gslalg_rng_poisson_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real -> int</text--->"
    "<text>rng_poisson( Mu )</text--->"
    "<text>Returns a random integer from the Poisson distribution with "
    "mean Mu using the GSL random number generator.</text--->"
    "<text>query rng_poisson( )</text--->"
    ") )";

Operator gslalg_rng_poisson( 
    "rng_poisson",
    gslalg_rng_poisson_Spec,
    gslalg_rng_poisson_VM,
    Operator::SimpleSelect,
    gslalg_real2int_TM) ;


/*
5.19 Operator ~rng\_geometric~

Get a geometric distributed int variate from randomgen

*/

int gslalg_rng_geometric_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = ((CcInt*) result.addr);
  long resD = 0;
  CcReal *cA = (CcReal*) args[0].addr;
  if( cA->IsDefined() )
  {
    double a = cA->GetRealval();
    if( 0.0 <= a && a <= 1.0 )
    {
      resD = gsl_ran_geometric(the_gsl_randomgenerator.GetGenerator(), a);
      res->Set(true, resD);
    }
    else
    {
      res->Set(false, 0);
    }
  }
  else
    res->Set(false, 0);
  return (0);
}

const string gslalg_rng_geometric_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real x real -> real</text--->"
    "<text>rng_geometric( p )</text--->"
    "<text>Returns a random integer from the geometric distribution, "
    "the number of independent trials with probability p until the first "
    "success. Uses the GSL random number generator.</text--->"
    "<text>query rng_geometric( 0.75 )</text--->"
    ") )";

Operator gslalg_rng_geometric( 
    "rng_geometric",
    gslalg_rng_geometric_Spec,
    gslalg_rng_geometric_VM,
    Operator::SimpleSelect,
    gslalg_real2int_TM) ;


/*
6 Class ~GSLAlgebra~

The last steps in adding an algebra to the Secondo system are

  * Associating value mapping functions with their operators

  * ``Bunching'' all
type constructors and operators in one instance of class ~Algebra~.

Therefore, a new subclass ~GSLAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~ccalgebra1~ is defined.

*/


class GSLAlgebra : public Algebra
{
 public:
  GSLAlgebra() : Algebra()
    {
    AddOperator( &gslalg_rng_init );
    AddOperator( &gslalg_rng_int );
    AddOperator( &gslalg_rng_intN );
    AddOperator( &gslalg_rng_getMin );
    AddOperator( &gslalg_rng_getMax );
    AddOperator( &gslalg_rng_real );
    AddOperator( &gslalg_rng_realpos );
    AddOperator( &gslalg_rng_setSeed );
    AddOperator( &gslalg_rng_getSeed );
    AddOperator( &gslalg_rng_getType ); 
    AddOperator( &gslalg_rng_flat );
    AddOperator( &gslalg_rng_gaussian );
    AddOperator( &gslalg_rng_binomial );
    AddOperator( &gslalg_rng_geometric );
    AddOperator( &gslalg_rng_exponential );
    AddOperator( &gslalg_rng_poisson );
    AddOperator( &gslalg_rng_NoGenerators );
    AddOperator( &gslalg_rng_GeneratorName );
    AddOperator( &gslalg_rng_GeneratorMinRand );
    AddOperator( &gslalg_rng_GeneratorMaxRand );
    }
  ~GSLAlgebra() {};

//   private:
//     bool initialized;
//     const gsl_rng_type **gslalg_randomgeneratorTable;
// // number of generator types available:
//     int gslalg_randomgeneratorTableSize;
//     int gsl_rng_defaultIndex;

    
};

GSLAlgebra gslAlgebra;

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
InitializeGSLAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&gslAlgebra);
}
