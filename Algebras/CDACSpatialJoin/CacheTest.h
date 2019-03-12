/*
----
This file is part of SECONDO.

Copyright (C) 2018,
Faculty of Mathematics and Computer Science,
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

\tableofcontents


1 CacheTest operator

This operator performs a cache test of the given intensity (use
intensity = 128 or higher for meaningful test results) by creating a large
array of integer values and repeatedly performing first sequential access,
then random access to various scopes of this array.

Different scope sizes are tested, starting from 1 KiB scope size (which easily
fits into the L1 Data Cache, making repeated access to such a scope very fast),
then doubling scope size until L1, L2, and L3 cache sizes are exceeded. The
final scope size is at least 8 times higher than the size of the largest cache
(e.g., for an 8 MiB L3 cache, the final scope size is 64 MiB, and access is
significantly slower than access with 1 KiB scope size - e.g. 2 times slower
for sequential access, and 40 times slower for random access).

The test results for each scope size are reported to the console. In between
tests, all caches are overwritten with other data. Test results are given in
milliseconds (ms) per 1 billion read access operations, depending on scope size
and access type (sequential / random). These results are obtained by

  1 performing a test with array access ("gross" duration),

  2 performing the same test without array access (but with the same loops
    and, if applicable, random number generation as used in the first test),
    and then

  3 calculating the "net" duration as the difference between 1 and 2. To
    reduce deviation, this process is performed 3 times and the average
    duration is used as a result.

The total execution time of the test is approx. "a few seconds
(e.g. 6 seconds) * intensity", obviously depending on the machine.

*/

#pragma once

#include <memory>
#include <iostream>
#include <ostream>

#include "Operator.h"
#include "QueryProcessor.h"
#include "CacheInfo.h"

namespace cdacspatialjoin {

/*
1.1 CacheTest operator class

*/
class CacheTest {
public:
   explicit CacheTest() = default;

   ~CacheTest() = default;

   std::shared_ptr<Operator> getOperator();

private:
   class Info;

   static ListExpr typeMapping(ListExpr args);

   static int valueMapping(Word* args, Word& result, int message,
                           Word& local, Supplier s);
};


/*
1.2 CacheTestLocalInfo class

*/
class CacheTestLocalInfo {
   // typedef and constants

public:
   /* the access type to be tested */
   enum ACCESS_TYPE {
      sequentialAccess,
      sequentialTwoLists,
      randomAccess
   };

   /* the recommended (minimum) intensity value */
   static constexpr size_t RECOMMENDED_INTENSITY = 128;

private:
   /* the type used in the data array */
   typedef uint64_t entryType;

   /* the size of an entry in the data array in bytes */
   static constexpr size_t ENTRY_BYTE_COUNT = sizeof(entryType);

   /* the number of times a test is performed consecutively to determine the
    * average execution time */
   static constexpr unsigned TEST_COUNT = 4;

   /* since random number generation takes a lot of time, each random array
    * entry is only accessed "intensity / randomDenominator" times (on average),
    * rather than "intensity" times */
   static constexpr size_t randomDenom = 16;

   // -----------------------------------------------------
   // variables passed to the constructor

   /* true for testing random access, false for sequential access */
   const ACCESS_TYPE accessType;

   /* the intensity of the test; test duration is a few seconds * intensity;
    * for meaningful results, intensity = 128 or higher should be used */
   const size_t intensity;

   // -----------------------------------------------------
   // variables determined in the constructor (and never changed afterwards)

   /* the level of the largest cache (e.g. 3 for an L3 cache) */
   unsigned maxCacheLevel;

   /* the number of entries in the data array */
   size_t dataCount;

   /* the data array which is accessed during the tests */
   entryType* data;

   /* the number of entries in the overwriteData array */
   size_t overwriteDataCount;

   /* the array which is used to clear all caches at the beginning of a test */
   entryType* overwriteData;

public:
   /* instantiates a test class for the given access type and intensity */
   CacheTestLocalInfo(std::ostream& out, ACCESS_TYPE accessType_,
           size_t intensity_ = RECOMMENDED_INTENSITY);

   /* destructor */
   ~CacheTestLocalInfo();

   /* performs the tests with the parameters given in the constructor */
   void test(std::ostream& out);

private:
   /* performs (TEST_COUNT) tests with the given scope size, returning
    * a) the "gross" duration sum (including array access), and
    * b) the duration of loops and random number generation (with no array
    * access). Use "a - b" to get the "net" duration of the array access */
   std::pair<clock_t, clock_t> testScope(size_t scopeSizeKiB, size_t& sum1,
           size_t& sum2);

   /* clears all caches by filling them with the overwriteData array entries
    * (which are not used in the actual tests) */
   void overwriteCaches(size_t& sum);

   /* reports the given test results to the given out stream */
   void reportTest(std::ostream& out, size_t scopeSizeKiB,
           unsigned int cacheLevel, bool printCacheLevel,
           clock_t duration1Sum, clock_t duration2Sum) const;
};

} // end namespace cdacspatialjoin
