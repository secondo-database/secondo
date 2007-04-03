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

*/

#ifndef GSL_ALGEBRA_H
#define GSL_ALGEBRA_H

using namespace std;

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


extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

/*
3. C++ Data Types 

3.1 Type ~randomgen~

Type ~randomgen~ is a random number generator. As an object, it also stores
the type of generation method to apply, and a random seed.

It could be expanded to become of kind SIMPLE or DATA later. The ~defined~ flag
was included to this goal.

*/

class GslRandomgen //: public Attribute
{
  public:
    GslRandomgen();
    GslRandomgen( const bool def );
    GslRandomgen( const int typeNo, const unsigned long seed );
    GslRandomgen( const GslRandomgen& other );
    GslRandomgen& operator= ( const GslRandomgen& other );
    ~GslRandomgen();
    inline void SetDefined( const bool def );
    inline bool IsDefined() const;
    inline size_t Sizeof() const;
    inline void SetSeed( const unsigned long seed );
    inline unsigned long GetSeed();
    inline int GetType();
    inline unsigned long int NextInt();
    inline unsigned long int MinRand() const;
    inline unsigned long int MaxRand() const;
    inline double NextReal();
    inline double NextRealPos();
    inline unsigned long int NextIntN(const unsigned long int n);
    inline gsl_rng* GetGenerator();
    static long rngType_getNoOfGenerators();
    static unsigned long rngType_getMinRand(const int index);
    static unsigned long rngType_getMaxRand(const int index);
    static string rngType_getName(const int index);

  private:
    bool defined;
    unsigned long mySeed;
    int myType;
    gsl_rng *me;
    
    static void InitializeTables();
    static int gslalg_randomgeneratorTableSize;
    static const gsl_rng_type* GetRngType(int index);
    static bool initialized;
    static int gsl_rng_defaultIndex;
    static const gsl_rng_type **gslalg_randomgeneratorTable;

};

#endif

