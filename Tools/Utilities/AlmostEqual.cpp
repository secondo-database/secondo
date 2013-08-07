/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
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

1 Implementation for ~AlmostEqual~

This file implements several different methods to test float values on whether they
are semantically equal.

 The header file is: ``../../inlcude/AlmostEqual.h''

*/


#include "AlmostEqual.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <math.h>
#include "WinUnix.h"

// The following  line selects the used standard implementation for
// AlmostEqual(double, double):
// If the #define ALMOSTEQUAL_USE_2S_COMPLEMENT is active, the
// AlmostEqual2sComplement(...) version is used, otherwise the
// AlmostEqualAbsolute(...) is utilized:

//#define ALMOSTEQUAL_USE_2S_COMPLEMENT


using namespace std;

const double FACTOR = 0.00000001;            // Precision factor. Maximun
                                             //   tolerated absolute difference
                                             //   between "equal" float numbers.
                                             //   used by AlmostEqualAbsolute()
const int64_t DELTA_NUMBERS =                // Precision factor, used within
  ((uint64_t)(0x1)) <<                       //   AlmostEqual2sComplement()
  ((numeric_limits<double>::digits-30)>0 ?   //   Number of tolerated
   (numeric_limits<double>::digits-30) : 8); //   representable floats between
                                             //   "equal" values. Must be >0.



const uint64_t NEGZERO =                    // Used in AlmostEqual2sComplement()
  ((uint64_t)(0x1)) << ((sizeof(double)*8)-1);// must be 2^(bitlength(double)-1)


/*

1.1 AlmostEqual

The standard function for bool AlmostEqual(double d1, double d2):

*/

bool AlmostEqual( const double &d1, const double &d2 ){
#ifdef ALMOSTEQUAL_USE_2S_COMPLEMENT
  return AlmostEqual2sComplement( d1, d2, DELTA_NUMBERS );
#else
  return AlmostEqualAbsolute( d1, d2, FACTOR );
#endif
};



/*
1.1 AlmostEqualAbsolute

~Auxilary function to compare Double values for equality~

*/

bool AlmostEqualAbsolute( const double &d1,
                                 const double &d2, const double &epsilon)
{
  double diff = fabs(d1-d2);
  return ( diff < epsilon );

//   double i1, i2;
//   double dd1 = modf( d1, &i1 ),
//          dd2 = modf( d2, &i2 );
//   long ii1 = (long)i1,
//        ii2 = (long)i2;
//
//   if( abs(ii1 - ii2) > 1 )
//     return false;

//   int d = abs(ii1) - abs(ii2);
//   return fabs(dd1 - dd2 - d) < epsilon;
}

/*

1.1 AlmostEqual2sComplement

Uses the integral interpretation of float values to decide AlmostEqual.
Taken from: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

To map negative numbers, we need to detremine the integral value ~aInt~ corresponding to negative zero:

----
IEEE754 Type  sizeOf(Type)  sign  exponent  mantisse  negzero
float          4 (32 bit)    1     8 bit     23 bit    2^31 = 0x80000000
double         8 (64 bit)    1     11 bit    52 bit    2^63 = 0x8000000000000000
----

*/
bool AlmostEqual2sComplement( const double &A,
                                     const double &B, const int64_t &maxUlps  )
{
    // maxUlps is the number of tolerated representable double numbers between
    // A and B.
    // Make sure maxUlps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    // maxUlps must be >0.
    const int64_t NEGZERO = 2^(numeric_limits<double>::digits - 1);

    assert( maxUlps > 0 );
    int64_t aInt = *(int64_t*)&A;
    // Make aInt lexicographically ordered as a twos-complement long int
    if (aInt < 0)
        aInt = NEGZERO - aInt;
    // Make bInt lexicographically ordered as a twos-complement long int
    int64_t bInt = *(int64_t*)&B;
    if (bInt < 0)
        bInt = NEGZERO - bInt;
#ifdef SECONDO_ANDROID
    int64_t intDiff = abs((long int)(aInt - bInt));
#else
    int64_t intDiff = abs(aInt - bInt);
#endif
    if (intDiff <= maxUlps)
        return true;
    return false;
}

bool AlmostEqual_CheckTypeSizes() {
  cout << endl<< endl << "Type sizes (bytes) used by this system:" << endl;
  cout << "\tsizeof(void*)       = " << sizeof(void*) << endl;
  cout << "\tsizeof(size_t)      = " << sizeof(size_t) << endl;
  cout << "\tsizeof(bool)        = " << sizeof(bool) << endl;
  cout << "\tsizeof(char)        = " << sizeof(char) << endl;
  cout << "\tsizeof(int)         = " << sizeof(int) << endl;
  cout << "\tsizeof(long)        = " << sizeof(long) << endl;
  cout << "\tsizeof(long long)   = " << sizeof(long long) << endl;
  cout << "\tsizeof(short)       = " << sizeof(short) << endl;
  cout << "\tsizeof(float)       = " << sizeof(float) << endl;
  cout << "\tsizeof(double)      = " << sizeof(double) << endl;
  cout << "\tsizeof(long double) = " << sizeof(long double) << endl;
  cout << "\tisLittleEndian      = "
       << (WinUnix::isLittleEndian() ? "yes" : "no") << endl << endl;
  bool ok  = (sizeof(double) == 8);
  if( !ok ){
      cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << ":" << __LINE__
           << "): Incompatible type sizes:" << endl;
      cerr << "AlmostEqual2sComplement(...) relies on 'double' being 64-bit "
           << "floats!" << endl;
  };
  return ok;
}
