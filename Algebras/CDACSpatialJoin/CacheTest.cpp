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


1 CacheTest Operator

1.1 Imports

*/
#include <iostream>
#include <ostream>

#include "NestedList.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "ListUtils.h"

#include "CacheTest.h"
#include "CacheInfo.h"
#include "Utils.h"


using namespace cdacspatialjoin;
using namespace std;

/*
1.2 Class OperatorInfo

A subclass of class ~OperatorInfo~ is defined with information on the operator.

*/
class CacheTest::Info : public OperatorInfo {
public:
   Info() {
      name = "cacheTest";
      signature = "int -> bool";
      syntax = "cacheTest (_)";
      meaning = "Tests the speed and cooperation of the available caches and "
                "the main memory by performing both sequential and random read "
                "accesses to array scopes of different sizes. The intensity "
                "parameter should be 128 or higher for results to be "
                "significant. Test duration is a few seconds * intensity.";
      example = "query cacheTest(128);";
   }
};

std::shared_ptr<Operator> CacheTest::getOperator() {
   return std::make_shared<Operator>(
           Info(),
           &CacheTest::valueMapping,
           &CacheTest::typeMapping);
}

/*
1.3 Type Mapping

*/
ListExpr CacheTest::typeMapping(ListExpr args) {
   // check the number of arguments
   if (nl->ListLength(args) != 1)
      return listutils::typeError("One argument expected.");

   // check the type of argument 1
   const ListExpr intensity = nl->First(args);
   if (!CcInt::checkType(intensity))
      return listutils::typeError("argument 1: int expected");

   return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.4 Value Mapping

*/
int CacheTest::valueMapping(
        Word* args, Word& result, int message, Word& local, Supplier s) {

   auto intensity = static_cast<size_t>(
           static_cast<CcInt*>(args[0].addr)->GetValue());

   // report the available caches
   CacheInfos::report(cout);

   // test sequential and random access
   CacheTestLocalInfo::ACCESS_TYPE accessTypes[] {
           CacheTestLocalInfo::ACCESS_TYPE::sequentialAccess,
           CacheTestLocalInfo::ACCESS_TYPE::sequentialTwoLists,
           CacheTestLocalInfo::ACCESS_TYPE::randomAccess };
   for (CacheTestLocalInfo::ACCESS_TYPE accessType : accessTypes) {
      CacheTestLocalInfo li {cout, accessType, intensity};
      li.test(cout);
   }

   // print warning if intensity is small (small value may of course be used
   // for purposes of testing the operator)
   if (intensity < CacheTestLocalInfo::RECOMMENDED_INTENSITY) {
      cout << endl << "##### please use intensity = "
          << CacheTestLocalInfo::RECOMMENDED_INTENSITY << " or higher "
          << "for significant results #####" << endl;
   }
   cout << endl;

   qp->ResultStorage<CcBool>(result, s).Set(true, true);

   return 0;
}

// ========================================================
/*
1.5 CacheTestLocalInfo class

*/
CacheTestLocalInfo::CacheTestLocalInfo(ostream& out,
        const ACCESS_TYPE accessType_,
        const size_t intensity_ /* = 128 */) :
        accessType(accessType_),
        intensity(intensity_) {

   // print cache test title
   out << endl;
   switch (accessType) {
      case sequentialAccess:
         out << "Cache test with sequential read access:" << endl;
         break;
      case sequentialTwoLists:
         out << "Cache test with sequential read access on two lists:" << endl;
         break;
      case randomAccess:
         out << "Cache test with random read access:" << endl;
         break;
      default:
         assert (false); // unexpected type
         break;
   }

   // since random number generation takes a lot of time, each random array
   // entry is only accessed "intensity / randomDenominator" times (on average),
   // rather than "intensity" times:
   static_assert (1024 % randomDenom == 0,
                  "randomDenom must be a factor of 1024");

   // retrieve the size of the largest cache (e.g. 6 MB L3 cache); in case
   // getCacheInfo() does not work on this system, use 8 MB as a standard value
   size_t maxCacheSize = 8 * 1024 * 1024;
   maxCacheLevel = 0;
   CacheInfoPtr cacheInfo = nullptr;
   for (unsigned level = 5; level > 0; --level) {
      cacheInfo = CacheInfos::getCacheInfo(CacheType::Data, level);
      if (cacheInfo) {
         maxCacheSize = cacheInfo->sizeInBytes;
         maxCacheLevel = level;
         break;
      }
   }

   // create a "data" array of integer values that is at least 8 times larger
   // than the largest available cache
   size_t dataByteCount = 1024 * 1024; // at least 1 MiB
   while (dataByteCount < maxCacheSize)
      dataByteCount *= 2;
   dataByteCount *= 8;
   dataCount = dataByteCount / ENTRY_BYTE_COUNT;
   data = new entryType[dataCount];
   for (size_t i = 0; i < dataCount; ++i)
      data[i] = i % 1023; // the actual content does not really matter

   // create another array large enough to "clear" all caches when starting a
   // test (so cache content from a previous test does not influence the test)
   overwriteDataCount = maxCacheSize / ENTRY_BYTE_COUNT;
   overwriteData = new entryType[overwriteDataCount];
   for (size_t i = 0; i < overwriteDataCount; ++i)
      overwriteData[i] = i;

   // explain the tests to be performed
   out << "- using data array of " << formatInt(dataCount) << " entries"
       << " * " << formatInt(ENTRY_BYTE_COUNT) << " bytes = "
       << formatInt(ENTRY_BYTE_COUNT * dataCount / (1024 * 1024)) << " MiB"
       << endl;
   switch (accessType) {
      case sequentialAccess: {
         out << "- sequentially reading scopes of different sizes, "
             << "repeating each scope " << intensity << " times" << endl;
         out << "- e.g., array entries 0..1023 (scope size 1 KiB) are read "
             << intensity << " times, then entries 1024..2047 etc." << endl;
         break;
      }
      case sequentialTwoLists: {
         out << "- for different scope sizes, two scopes are randomly selected "
             << "and then scanned " << intensity << " times, alternately "
             << "reading entries from scope 1 and 2" << endl;
         out << "- e.g., a read sequence is entries 0, 8192, 1, 8193, 2, 8194, "
             << "..., 1023, 9215 for scope size 1 KiB." << endl;
         break;
      }
      case randomAccess: {
         out << "- reading random entries from scopes of different sizes; "
             << "on average, each entry is read "
             << intensity / (double)randomDenom << " times" << endl;
         out << "- e.g., " << intensity * 1024 / (double)randomDenom << " "
             << "random entries are read from array scope 0..1023 "
             << "(scope size 1 KiB), then random entries from scope 1024..2047 "
             << "etc." << endl;
         break;
      }
      default:
         assert (false); // unexpected type
         break;
   }
   if (TEST_COUNT > 1) {
      out << "- each test is performed " << TEST_COUNT << " times, "
          << "average results are reported" << endl;
   }
   out << "- horizontal separators show into which cache level the different "
       << "scope sizes fit" << endl;
   out << endl;

   // print the table header
   out << "      scope size |   read access duration |";
#ifdef TIMER_USES_PAPI
   out << " L1-I Misses | L1-Data Misses |     L2 Misses |     L3 Misses |";
#endif
   out << "  test avg - "
       << (accessType == sequentialAccess ? "loops only" : "loops/rand")
       << " = access only" << endl;
}

CacheTestLocalInfo::~CacheTestLocalInfo() {
   delete[] data;
   delete[] overwriteData;
}

void CacheTestLocalInfo::test(ostream& out) {
   // check sums are used to ensure the compiler does not "optimize away" the
   // array accesses (which are without effect otherwise)
   entryType sum1 = 0;
   uint64_t sum2 = 0;

   // start with a scope size of 1 KiB, then double it in each loop
   size_t scopeSizeKiB = 1;
   unsigned cacheLevel = 1; // will be increased if scopes exceed cache size
   bool printCacheLevel = true;
   size_t dataByteCount = dataCount * ENTRY_BYTE_COUNT;

   Timer timer { CACHE_TEST_TASK_COUNT };

   // loop over scope sizes
   while (dataByteCount % (scopeSizeKiB * 1024) == 0) {
      // omit last test for accessType sequentialTwoLists (as the scope size
      // is now the full list, but two different lists should be scanned)
      size_t scopeCount = dataByteCount / (scopeSizeKiB * 1024);
      if (accessType == sequentialTwoLists && scopeCount == 1)
         break;

      // perform test
      timer.reset();
      testScope(scopeSizeKiB, sum1, sum2, timer);

      // determine whether the size of the previous cache level has been
      // exceeded by the scope size in this test
      CacheInfoPtr cacheInfo =
              CacheInfos::getCacheInfo(CacheType::Data, cacheLevel);
      if (cacheInfo && (scopeSizeKiB > cacheInfo->getSizeInKiB())) {
         ++cacheLevel;
         printCacheLevel = true;
      }

      // report test results
      reportTest(out, scopeSizeKiB, cacheLevel, printCacheLevel, timer);
      printCacheLevel = false;

      // increase scope size for the next test
      scopeSizeKiB *= 2;
   }

   // report check sums. This ensures that the compiler does not
   // "optimize away" sum1 and sum2 and therefore should not be deleted
   out << "(check-sums: " << formatInt(sum1) << ", " << formatInt(sum2)
       << ")" << endl;
}

void CacheTestLocalInfo::testScope(const size_t scopeSizeKiB,
        size_t& sum1, size_t& sum2, Timer& timer) {
   const size_t entriesPerScope = scopeSizeKiB * 1024 / ENTRY_BYTE_COUNT;
   const size_t scopeCount = dataCount / entriesPerScope;
   const size_t rndsPerScope = (entriesPerScope * intensity) / randomDenom;
   assert (entriesPerScope > 0);
   assert (scopeCount * entriesPerScope == dataCount);

   // initialize the random number generator with a constant seed
   // to ensure reproducibility
   std::mt19937 rndGenerator(1);
   std::uniform_int_distribution<size_t> randomEntry(0, entriesPerScope - 1);
   std::uniform_int_distribution<size_t> randomScope(0, scopeCount - 1);

   // perform the test (TEST_COUNT) times
   for (unsigned test = 0; test < TEST_COUNT; ++test) {
      // clear all caches from data from the last test
      overwriteCaches(sum2);

      // perform the actual test, adding entries from data[] to sum1;
      // the use of locality depends on the scope size ("entriesPerScope")
      timer.start(CacheTestTask::fullTest);
      if (accessType == sequentialAccess) {
         // iterate over the scopes of the given size
         for (size_t scope = 0; scope < scopeCount; ++scope) {
            size_t offset = scope * entriesPerScope;
            // sequentially access the scope's entries (intensity) times
            for (size_t pass = 0; pass < intensity; ++pass) {
               for (size_t entry = 0; entry < entriesPerScope; ++entry) {
                  sum1 += data[offset + entry];
               }
            }
         }

      } else if (accessType == sequentialTwoLists) {
         assert (scopeCount % 2 == 0);
         // the outer loop uses only half the scopeCount as two scopes will be
         // accessed each time
         for (size_t scope = 0; scope < scopeCount / 2; ++scope) {
            // randomly select two different scopes
            size_t offset1 = randomScope(rndGenerator) * entriesPerScope;
            size_t offset2;
            do {
               offset2 = randomScope(rndGenerator) * entriesPerScope;
            } while (offset1 == offset2 && scopeCount > 1);
            // alternately access the entries of the scopes (intensity) times
            for (size_t pass = 0; pass < intensity; ++pass) {
               for (size_t entry = 0; entry < entriesPerScope; ++entry) {
                  sum1 += data[offset1 + entry];
                  sum1 += data[offset2 + entry];
               }
            }
         }

      } else if (accessType == randomAccess) {
         // iterate over the scopes of the given size
         for (size_t scope = 0; scope < scopeCount; ++scope) {
            size_t offset = scope * entriesPerScope;
            // access random entries within the scope (rndsPerScope) times
            for (size_t entry = 0; entry < rndsPerScope; ++entry) {
               sum1 += data[offset + randomEntry(rndGenerator)];
            }
         }

      } else {
         assert (false); // unexpected accessType
      }
      timer.stop();

      // measure the time used for loops and random number generation only
      // (without data access) to subtract it from the first duration
      rndGenerator.seed(1); // create the same random sequence again
      timer.start(CacheTestTask::loopTest);
      if (accessType == sequentialAccess) {
         // increment sum2 using the same loop ranges as above
         for (size_t scope = 0; scope < scopeCount; ++scope) {
            sum2 += scope * entriesPerScope;
            for (size_t pass = 0; pass < intensity; ++pass) {
               for (size_t entry = 0; entry < entriesPerScope; ++entry) {
                  ++sum2;
               }
            }
         }

      } else if (accessType == sequentialTwoLists) {
         // increment sum2 using the same loop ranges and random number
         // generations as above
         for (size_t i = 0; i < scopeCount / 2; ++i) {
            size_t offset1 = randomScope(rndGenerator) * entriesPerScope;
            size_t offset2;
            do {
               offset2 = randomScope(rndGenerator) * entriesPerScope;
            } while (offset1 == offset2 && scopeCount > 1);
            for (size_t pass = 0; pass < intensity; ++pass) {
               for (size_t entry = 0; entry < entriesPerScope; ++entry) {
                  ++sum1;
                  ++sum2;
               }
            }
         }

      } else if (accessType == randomAccess) {
         // create the same random values as above and add them to sum2
         for (size_t scope = 0; scope < scopeCount; ++scope) {
            sum2 += scope * entriesPerScope;
            for (size_t entry = 0; entry < rndsPerScope; ++entry) {
               sum2 += randomEntry(rndGenerator);
            }
         }

      } else {
         assert (false); // unexpected accessType
      }
      timer.stop();
   }
}

void CacheTestLocalInfo::overwriteCaches(size_t& sum) {
   // clear all caches by sequentially reading the overwriteData
   // which is large enough to fill the largest cache on this machine
   for (size_t i = 0; i < overwriteDataCount; ++i)
      sum += overwriteData[i];
   for (size_t i = 0; i < overwriteDataCount; ++i)
      sum -= overwriteData[i];
}

void CacheTestLocalInfo::reportTest(ostream& out, const size_t scopeSizeKiB,
        const unsigned cacheLevel, const bool printCacheLevel,
        Timer& timer) const {

   if (printCacheLevel) {
      // print horizontal separator
      out << "-----------------+------------------------+";
#ifdef TIMER_USES_PAPI
      out << "-------------+----------------+---------------+---------------+";
#endif
      out << "-------------------------------------" << endl;

      // report which cache level the current scope size fits into
      if (cacheLevel <= maxCacheLevel)
         out << "L" << cacheLevel << " "; // e.g., "L2 " for L2 data cache
      else
         out << "RAM";
   } else {
      out << "   ";
   }

   // determine the number of read access operations performed
   double accessCount = dataCount * intensity;
   if (accessType == randomAccess) {
      accessCount /= static_cast<double>(randomDenom);
   } // otherwise, keep accessCount

   // get average duration of the test (the timer keeps track of the number of
   // task calls (TEST_COUNT) and can therefore provide the average values)
   const Task* fullTest = timer.getTask(CacheTestTask::fullTest);
   const Task* loopTest = timer.getTask(CacheTestTask::loopTest);
   const clock_t fullTestTime = fullTest->getAvgTime();
   const clock_t loopTestTime = loopTest->getAvgTime();
   const clock_t arrayAccessTime = fullTestTime - loopTestTime;
   const auto arrayAccessTimePer1E9 = static_cast<clock_t>(
           arrayAccessTime * 1.0E9 / accessCount);

   // report test result (i.e. one line of the result table)
   // note that 1E09 = German "Milliarde" = English "billion" (used here)
   //       but 1E12 = German "Billion" = English "trillion" (not used here)
   out << setw(9) << formatInt(scopeSizeKiB) << " KiB |"
       << setw(11) << formatMillis(arrayAccessTimePer1E9) << " per billion |";
#ifdef TIMER_USES_PAPI
   out << setw(12) << formatInt(fullTest->getAvgL1InstrCacheMisses()) << " |"
       << setw(15) << formatInt(fullTest->getAvgL1DataCacheMisses()) << " |"
       << setw(14) << formatInt(fullTest->getAvgL2CacheMisses()) << " |"
       << setw(14) << formatInt(fullTest->getAvgL3CacheMisses()) << " |";
#endif
   out << setw(10) << formatMillis(fullTestTime) << " - "
       << setw(10) << formatMillis(loopTestTime) << " = "
       << setw(10) << formatMillis(arrayAccessTime) << endl;

}
