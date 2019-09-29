/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{2}
\tableofcontents


1 CacheTest Operator

1.1 Imports

*/
#include <iostream>
#include <ostream>
#include <random>

#include "CacheTest.h"
#include "Utils.h"

#include "QueryProcessor.h" // -> AlgebraManager.h -> NestedList.h
#include "Symbols.h"
#include "StandardTypes.h"
#include "ListUtils.h"


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

#ifdef TIMER_USES_PAPI
   // Timer::testPAPIOverhead(cout);
#endif

   // report the available caches
   CacheInfos::report(cout);

   // test sequential and random access
   CacheTestLocalInfo::ACCESS_TYPE accessTypes[] {
           CacheTestLocalInfo::ACCESS_TYPE::sequentialAccess,
           CacheTestLocalInfo::ACCESS_TYPE::reverseSequentialAccess,
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
        testCount(intensity_ == 0 ? 1 : NORMAL_TEST_COUNT),
        intensity(intensity_ == 0 ? 1 : intensity_) {

   // print cache test title
   out << setfill(' ') << endl;
   switch (accessType) {
      case sequentialAccess:
         out << "Cache test with sequential read access:" << endl;
         break;
      case reverseSequentialAccess:
         out << "Cache test with reverse sequential read access:" << endl;
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
         out << "- e.g., array entries 0..1023 (scope size 8 KiB) are read "
             << intensity << " times, then entries 1024..2047 etc." << endl;
         break;
      }
      case reverseSequentialAccess: {
         out << "- reading scopes of different sizes in reverse sequential "
             << "order, repeating each scope " << intensity << " times" << endl;
         out << "- e.g., array entries 0..1023 (scope size 8 KiB) are read "
             << intensity << " times, then entries 1024..2047 etc." << endl;
         break;
      }
      case sequentialTwoLists: {
         out << "- for different scope sizes, two scopes are randomly selected "
             << "and then scanned " << intensity << " times, alternately "
             << "reading entries from scope 1 and 2" << endl;
         out << "- e.g., a read sequence is entries 0, 8192, 1, 8193, 2, 8194, "
             << "..., 1023, 9215 for scope size 8 KiB." << endl;
         break;
      }
      case randomAccess: {
         out << "- reading random entries from scopes of different sizes; "
             << "on average, each entry is read "
             << intensity / (double)randomDenom << " times" << endl;
         out << "- e.g., " << intensity * 1024 / (double)randomDenom << " "
             << "random entries are read from array scope 0..1023 "
             << "(scope size 8 KiB), then random entries from scope 1024..2047 "
             << "etc." << endl;
         break;
      }
      default:
         assert (false); // unexpected type
         break;
   }
   if (testCount > 1) {
      out << "- each test is performed " << testCount << " times, "
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
   out << "  test avg - loops only = access only" << endl;
}

CacheTestLocalInfo::~CacheTestLocalInfo() {
   delete[] data;
   delete[] overwriteData;
}

void CacheTestLocalInfo::test(ostream& out) {
   // initialize the random number generators with a constant seed to ensure
   // reproducibility of the test
   unsigned long RND_SEED = 1;

   // check sums are used to ensure the compiler does not "optimize away" the
   // array accesses (which are without effect otherwise)
   size_t sum1 = 0;
   size_t sum2 = 0;

   // start with a scope size of 1 KiB, then double it in each loop
   size_t scopeSizeKiB = 1;
   unsigned cacheLevel = 1; // will be increased if scopes exceed cache size
   bool printCacheLevel = true;
   size_t dataByteCount = dataCount * ENTRY_BYTE_COUNT;

   Timer timer { TASK_COUNT };

   // loop over scope sizes
   while (dataByteCount % (scopeSizeKiB * 1024) == 0) {
      // omit last test for accessType sequentialTwoLists (as the scope size
      // is now the full list, but two different lists should be scanned)
      size_t scopeCount = dataByteCount / (scopeSizeKiB * 1024);
      if (accessType == sequentialTwoLists && scopeCount == 1)
         break;

      // perform test
      timer.reset();
      testScope(scopeSizeKiB, RND_SEED, sum1, sum2, timer);

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
        const unsigned long rndSeed,
        size_t& sum1, size_t& sum2, Timer& timer) {

   const size_t entriesPerScope = scopeSizeKiB * 1024 / ENTRY_BYTE_COUNT;
   const size_t scopeCount = dataCount / entriesPerScope;
   const size_t scopeCountHalf = scopeCount / 2;
   const size_t iterationsPerScope = (accessType == randomAccess) ?
           entriesPerScope * intensity / randomDenom :
           entriesPerScope * intensity;

   assert (entriesPerScope > 0);
   assert (scopeCount * entriesPerScope == dataCount);
   if (accessType == sequentialTwoLists)
      assert (scopeCount % 2 == 0);

   // create a random sequence of scopes
   std::mt19937 rndGenerator(rndSeed);
   std::uniform_int_distribution<size_t> randomScope(0, scopeCount - 1);
   auto randomScopeStart = new size_t[scopeCount];
   if (accessType == reverseSequentialAccess) {
      // start at the last entry of a scope
      for (size_t i = 0; i < scopeCount; ++i)
         randomScopeStart[i] = (i + 1) * entriesPerScope - 1;
   } else {
      // start at the first entry of a scope
      for (size_t i = 0; i < scopeCount; ++i)
         randomScopeStart[i] = i * entriesPerScope;
   }
   for (size_t i = 0; i < scopeCount; ++i){
      size_t j = randomScope(rndGenerator);
      std::swap(randomScopeStart[i], randomScopeStart[j]);
   }

   // prepare data[] by writing into each entry the index of the next entry
   // that must be visited within the scope
   if (accessType == sequentialAccess || accessType == sequentialTwoLists) {
      createSequentialCycles(scopeCount, entriesPerScope);

   } else if (accessType == reverseSequentialAccess) {
      createReverseSequentialCycles(scopeCount, entriesPerScope);

   } else if (accessType == randomAccess) {
      createRandomCycles(scopeCount, entriesPerScope, rndSeed);

   } else {
      assert (false); // unexpected accessType
   }

   // perform the test (testCount) times
   for (unsigned test = 0; test < testCount; ++test) {
      // clear all caches from data from the last test
      overwriteCaches(sum2);

      // perform the actual test; the use of locality depends on the scope
      // size ("entriesPerScope")
      timer.start(CacheTestTask::fullTest);
      if (accessType == sequentialAccess ||
          accessType == reverseSequentialAccess ||
          accessType == randomAccess) {
         // iterate over the scopes of the given size
         for (size_t scope = 0; scope < scopeCount; ++scope) {
            // the start index is the first index of this scope
            size_t index = randomScopeStart[scope]; // scope * entriesPerScope;
            // access the scope's entries (intensity) times
            for (size_t entry = 0; entry < iterationsPerScope; ++entry) {
               index = data[index];
            }
            // ensure that the loop is not "optimized away"
            sum1 += index;
         }

      } else if (accessType == sequentialTwoLists) {
         // the outer loop uses only half the scopeCount as two scopes will be
         // accessed each time
         for (size_t scope = 0; scope < scopeCountHalf; ++scope) {
            // randomly select two different scopes
            size_t index1 = randomScopeStart[scope];
            size_t index2 = randomScopeStart[scopeCountHalf + scope];
            // alternately access the entries of the scopes (intensity) times
            for (size_t entry = 0; entry < iterationsPerScope; ++entry) {
               index1 = data[index1];
               index2 = data[index2];
            }
            // ensure that the loop is not "optimized away"
            sum1 += index1 + index2;
         }

      } else {
         assert (false); // unexpected accessType
      }
      timer.stop();

      // measure the time used for loops and random number generation only
      // (without data access) to subtract it from the first duration
      timer.start(CacheTestTask::loopTest);
      if (accessType == sequentialAccess ||
          accessType == reverseSequentialAccess ||
          accessType == randomAccess) {
         // increment sum2 using the same loop ranges as above
         for (size_t scope = 0; scope < scopeCount; ++scope) {
            size_t index = randomScopeStart[scope];
            for (size_t entry = 0; entry < iterationsPerScope; ++entry) {
               ++index;
            }
            // ensure that the loop is not "optimized away"
            sum2 += index;
         }

      } else if (accessType == sequentialTwoLists) {
         // increment sum2 using the same loop ranges and scopes as above
         for (size_t scope = 0; scope < scopeCountHalf; ++scope) {
            size_t index1 = randomScopeStart[scope];
            size_t index2 = randomScopeStart[scopeCountHalf + scope];
            for (size_t entry = 0; entry < iterationsPerScope; ++entry) {
               ++index1;
               ++index2;
            }
            // ensure that the loop is not "optimized away"
            sum2 += index1 + index2;
         }

      } else {
         assert (false); // unexpected accessType
      }
      timer.stop();
   }

   delete[] randomScopeStart;
}

void CacheTestLocalInfo::createSequentialCycles(const size_t scopeCount,
        const size_t entriesPerScope) const {
   // iterate over the scopes of the given size
   for (size_t scope = 0; scope < scopeCount; ++scope) {
      size_t offset = scope * entriesPerScope;
      // set each data entry to the index of the next entry
      size_t loopEnd = offset + entriesPerScope;
      for (size_t entry = offset; entry < loopEnd; ++entry)
         data[entry] = entry + 1;
      // set last entry in this scope to the index of the first in scope
      data[loopEnd - 1] = offset;
   }
}

void CacheTestLocalInfo::createReverseSequentialCycles(const size_t scopeCount,
        const size_t entriesPerScope) const {
   // iterate over the scopes of the given size
   for (size_t scope = 0; scope < scopeCount; ++scope) {
      size_t offset = scope * entriesPerScope;
      size_t loopEnd = offset + entriesPerScope;
      // set first entry in this scope to the index of the last in scope
      data[offset] = loopEnd - 1;
      // set each data entry to the index of the next entry
      for (size_t entry = offset + 1; entry < loopEnd; ++entry)
         data[entry] = entry - 1;
   }
}

void CacheTestLocalInfo::createRandomCycles(size_t scopeCount,
        size_t entriesPerScope, const unsigned long rndSeed) const {
   // initialize the random number generator
   std::mt19937 rndGenerator(rndSeed);

   // use auxiliary array
   auto aux = new entryType[entriesPerScope];
   // iterate over the scopes of the given size
   for (size_t scope = 0; scope < scopeCount; ++scope) {
      const size_t start = scope * entriesPerScope;
      // fill aux array with entry indices of this scope,
      // omitting first entry (e.g., aux = { 1025, 1026, ... 2047 })
      size_t auxSize = entriesPerScope - 1;
      for (size_t i = 0; i < auxSize; ++i)
         aux[i] = start + i + 1;
      // fill data with a random sequence of indices in this scope that
      // form a single cycle (so, by following this cycle, all entries in
      // this scope are being visited)
      size_t entry = start;
      while (auxSize > 0) {
         // get random aux index
         std::uniform_int_distribution<size_t> randomAux(0, auxSize - 1);
         const size_t auxIndex = randomAux(rndGenerator);
         const size_t nextEntry = aux[auxIndex];
         data[entry] = nextEntry;
         entry = nextEntry;
         // remove entry from aux (replacing it with the last aux entry)
         aux[auxIndex] = aux[--auxSize];
      }
      // set last entry in this sequence to the index of the first in scope
      data[entry] = start;
   }

   /*
   // test the sequence
   for (size_t entry = 0; entry < entriesPerScope; ++entry)
      aux[entry] = 0;
   size_t index = 0;
   for (size_t entry = 0; entry < entriesPerScope; ++entry) {
      ++aux[index];
      index = data[index];
   }
   for (size_t entry = 0; entry < entriesPerScope; ++entry)
      assert (aux[entry] == 1);
   */

   delete[] aux;
}

void CacheTestLocalInfo::overwriteCaches(size_t& sum) {
   // clear all caches by sequentially reading the overwriteData
   // which is large enough to fill the largest cache on this machine
   size_t count = overwriteDataCount;
   for (size_t i = 0; i < count; ++i)
      sum += overwriteData[i];
   for (size_t i = 0; i < count; ++i)
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
   // task calls (testCount) and can therefore provide the average values)
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
