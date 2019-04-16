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


1 Cache-conscious Spatial Join with Divide and Conquer


1.1 Imports

*/
#include <iostream>
#include <ostream>
#include <iomanip>

#include "CDACSpatialJoin.h" // -> ... -> SortEdge, JoinEdge
#include "Utils.h"

#include "AlgebraManager.h"
#include "Symbols.h"
#include "Algebras/CRel/Operators/OperatorUtils.h" // -> ListUtils.h
#include "Algebras/Standard-C++/LongInt.h" // -> StandardTypes.h
#include "CacheInfo.h"

typedef CRelAlgebra::TBlockTI::ColumnInfo TBlockColInfo;

using namespace cdacspatialjoin;
using namespace std;

uint64_t CDACSpatialJoin::DEFAULT_INPUT_BLOCK_SIZE = 10;

/*
1.2 Class OperatorInfo

A subclass of class ~OperatorInfo~ is defined with information on the operator.

*/
class CDACSpatialJoin::Info : public OperatorInfo {
public:
   Info() {
      name = "cdacspatialjoin";
      signature = "stream (tblock (a ((x1 t1) ... (xn tn)))) x \n"
         "stream (tblock (b ((y1 d1) ... (ym dm)))) x \n"
         "xi x yj x int \n"
         "-> \n"
         "stream (tblock (c ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm))))";
      syntax = "_ _ cdacspatialjoin [ _ , _  , _ ]";
      meaning = "Cache-conscious spatial join operator using divide and "
                "conquer to perform a spatial join on two streams of tuples "
                "or tuple blocks, where xi and yj (both optional) are the "
                "names of the join attributes of the first and second stream, "
                "respectively, and int is the (optional) size of the output "
                "TBlocks in MiB.";
      example = "query Roads feed toblocks[10] {a} "
                "Water feed toblocks[10] {b} "
                "cdacspatialjoin[GeoData_a, GeoData_b, 10] count";
   }
};

shared_ptr<Operator> CDACSpatialJoin::getOperator() {
   return make_shared<Operator>(
           Info(),
           &CDACSpatialJoin::valueMapping,
           &CDACSpatialJoin::typeMapping);
}

// ========================================================
/*
1.3 Type Mapping

The type mapping checks if exactly four arguments are passed to the operator.
The first two arguments must be streams of tuple blocks. The second two
arguments must be the names of the join attributes of the first and the
second stream, respectively.

*/
ListExpr CDACSpatialJoin::typeMapping(ListExpr args) {
   return typeMapping2(false, args);
}

ListExpr CDACSpatialJoin::typeMapping2(const bool countOnly, ListExpr args) {
   // check the number of arguments; a fifth argument is only expected for
   // the CDACSpatialJoin operator (not for CDACSpatialJoinCount)
   const auto argCount = static_cast<unsigned>(nl->ListLength(args));
   unsigned maxArgCount = countOnly ? MAX_ARG_COUNT - 1 : MAX_ARG_COUNT;
   if (argCount < STREAM_COUNT || argCount > maxArgCount) {
      return listutils::typeError(countOnly ? "2-4 arguments expected." :
                                              "2-5 arguments expected.");
   }

   // prepare values to hold information on the two input streams
   ListExpr stream[STREAM_COUNT];
   ListExpr streamType[STREAM_COUNT];
   bool isTBlockStream[STREAM_COUNT];
   ListExpr tBlockColumns[STREAM_COUNT];
   CRelAlgebra::TBlockTI tBlockInfo[STREAM_COUNT] =
           { CRelAlgebra::TBlockTI(false), CRelAlgebra::TBlockTI(false) };
   string attrName[STREAM_COUNT];
   uint64_t attrIndex[STREAM_COUNT];
   uint64_t attrCount[STREAM_COUNT];
   unsigned blockSize[STREAM_COUNT];
   unsigned dim[STREAM_COUNT];

   // get information on the two input streams
   for (unsigned i = 0; i < STREAM_COUNT; ++i) {
      // first and second arguments must be a streams
      const string argPos1or2 = (i == 0) ? "first" : "second";
      stream[i] = nl->First((i == 0) ? nl->First(args) : nl->Second(args));
      if (!listutils::isStream(stream[i])) {
         return listutils::typeError("Error in " + argPos1or2 + " argument: "
              "Stream expected.");
      }

      // stream must be of tuples or tuple blocks
      streamType[i] = nl->Second(stream[i]);
      isTBlockStream[i] = CRelAlgebra::TBlockTI::Check(streamType[i]);
      if (!isTBlockStream[i] && !Tuple::checkType(streamType[i])) {
         return listutils::typeError("Error in " + argPos1or2 + " argument: " +
             "Stream of tuples or tuple blocks expected.");
      }

      // extract information about tuple blocks
      tBlockColumns[i] = 0; // is returned by getTBlockTI()
      tBlockInfo[i] = isTBlockStream[i] ?
         CRelAlgebra::TBlockTI(nl->Second(stream[i]), false) :
         getTBlockTI(nl->Second(nl->Second(stream[i])),
                 DEFAULT_INPUT_BLOCK_SIZE, tBlockColumns[i]);

      attrCount[i] = tBlockInfo[i].columnInfos.size();
      blockSize[i] = tBlockInfo[i].GetDesiredBlockSize();

      // depending on whether a third / fourth argument is given, ...
      if (STREAM_COUNT + i < argCount) {
         // extract join attribute names and indices
         // third and fourth argument must be an attribute name
         const string argPos3or4 = (i == 0) ? "third" : "fourth";
         const ListExpr attrNameLE = nl->First(
                 (i == 0) ? nl->Third(args) : nl->Fourth(args));
         if (!listutils::isSymbol(attrNameLE)) {
            return listutils::typeError("Error in " + argPos3or4 +
               " argument: Attribute name expected.");
         }
         attrName[i] = nl->SymbolValue(attrNameLE);
         if (!GetIndexOfColumn(tBlockInfo[i], attrName[i], attrIndex[i])) {
            return listutils::typeError("Error in " + argPos3or4 +
               " argument: Invalid column name");
         }

         // join attributes must be a kind of SPATIALATTRARRAY2D / ...3D
         const TBlockColInfo& col = tBlockInfo[i].columnInfos[attrIndex[i]];
         if (listutils::isKind(col.type, Kind::SPATIALATTRARRAY2D())) {
            dim[i] = 2;
         } else if (listutils::isKind(col.type, Kind::SPATIALATTRARRAY3D())) {
            dim[i] = 3;
         } else {
            return listutils::typeError("Attribute " + col.name +
               " is not of kind SPATIALATTRARRAY2D or SPATIALATTRARRAY3D");
         }
      } else {
         // if no attribute name is given, find the first attribute of
         // kind SPATIALATTRARRAY2D or SPATIALATTRARRAY3D
         attrIndex[i] = 0;
         dim[i] = 0;
         for (const TBlockColInfo& col : tBlockInfo[i].columnInfos) {
            if (listutils::isKind(col.type, Kind::SPATIALATTRARRAY2D())) {
               dim[i] = 2;
               break;
            }
            if (listutils::isKind(col.type, Kind::SPATIALATTRARRAY3D())) {
               dim[i] = 3;
               break;
            }
            ++attrIndex[i];
         }
         if (dim[i] == 0) {
            return listutils::typeError("Error in " + argPos1or2 + " stream" +
               ": No attribute of kind "
               "SPATIALATTRARRAY2D or SPATIALATTRARRAY3D found");
         }
      } // end of if (argCount >= 3 + i)"
   }

   // compile information required by Value Mapping
   // the nl->Empty() element will be omitted below:
   const ListExpr appendInfo = nl->OneElemList(nl->Empty());
   ListExpr appendEnd = appendInfo;
   // ensure that the Value Mapping args will start at args[MAX_ARG_COUNT]
   // even if parameters three and four were omitted by the caller
   for (unsigned i = argCount; i < MAX_ARG_COUNT; ++i) {
      appendEnd = nl->Append(appendEnd, nl->IntAtom(0));
   }
   // append the actual information on the input streams
   for (unsigned i = 0; i < STREAM_COUNT; ++i) {
      appendEnd = nl->Append(appendEnd, nl->IntAtom(attrIndex[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(attrCount[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(dim[i]));
      appendEnd = nl->Append(appendEnd, nl->BoolAtom(isTBlockStream[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(tBlockColumns[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(blockSize[i]));
   }

   ListExpr resultType;
   if (countOnly) {
      resultType = nl->SymbolAtom(LongInt::BasicType());

   } else {
      // Initialize the type of result tuple block
      CRelAlgebra::TBlockTI resultTBlockInfo = CRelAlgebra::TBlockTI(false);

      // set the size of the result tuple block, using the larger input block
      // size
      uint64_t desiredBlockSize = max(tBlockInfo[0].GetDesiredBlockSize(),
                                      tBlockInfo[1].GetDesiredBlockSize());
      if (argCount == 5) {
         // ... unless an explicit result block size is provided in the query
         ListExpr outTBlockSizeLE = nl->Fifth(args);
         if (!CcInt::checkType(nl->First(outTBlockSizeLE)))
            return listutils::typeError(
                    "Error in fifth argument: int expected.");
         const long blockSize = nl->IntValue(nl->Second(outTBlockSizeLE));
         if (blockSize > 0)
            desiredBlockSize = static_cast<uint64_t>(blockSize);
      }
      resultTBlockInfo.SetDesiredBlockSize(desiredBlockSize);

      // check for duplicate column names
      set<string> columnNames;
      for (const TBlockColInfo& colInfo : tBlockInfo[0].columnInfos) {
         columnNames.insert(colInfo.name);
         resultTBlockInfo.columnInfos.push_back(colInfo);
      }
      for (const TBlockColInfo& colInfo : tBlockInfo[1].columnInfos) {
         if (!columnNames.insert(colInfo.name).second) {
            return listutils::typeError(
                    "Column name " + colInfo.name +
                    " exists in both relations");
         }
         resultTBlockInfo.columnInfos.push_back(colInfo);
      }
      resultType = resultTBlockInfo.GetTypeExpr(true);
   }

   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            nl->Rest(appendInfo),
                            resultType);
}

CRelAlgebra::TBlockTI CDACSpatialJoin::getTBlockTI(
        const ListExpr attributeList, const uint64_t desiredBlockSize,
        ListExpr& tBlockColumns) {
   // cp. ToBlocks::TypeMapping(ListExpr args) in ToBlocks.cpp
   // (which is private, however, so we need to copy it)
   CRelAlgebra::TBlockTI typeInfo(false);

   // Create column types of kind ATTRARRAY from attribute types of kind DATA
   ListExpr columns = nl->OneElemList(nl->Empty());
   ListExpr columnsEnd = columns;
   ListExpr attrRest = attributeList;

   while (!nl->IsEmpty(attrRest)) {
      const ListExpr current = nl->First(attrRest);
      const ListExpr columnName = nl->First(current);
      const ListExpr columnType = CRelAlgebra::AttrArrayTypeConstructor::
              GetDefaultAttrArrayType(nl->Second(current), false);

      attrRest = nl->Rest(attrRest);
      columnsEnd = nl->Append(columnsEnd, nl->TwoElemList(
              columnName, columnType));
   }
   tBlockColumns = nl->Rest(columns); // first element is ()
   typeInfo.AppendColumnInfos(tBlockColumns);
   typeInfo.SetDesiredBlockSize(desiredBlockSize);
   return typeInfo;
}

// ========================================================
/*
1.4 Value Mapping

*/
int CDACSpatialJoin::valueMapping(
        Word* args, Word& result, int message, Word& local, Supplier s) {
   return valueMapping2(false, args, result, message, local, s);
}

int CDACSpatialJoin::valueMapping2(const bool countOnly,
        Word* args, Word& result, int message, Word& local, Supplier s) {

   auto localInfo = static_cast<CDACLocalInfo*>(local.addr);

   // OPEN: create LocalInfo instance
   if (countOnly || message == OPEN) {
      delete localInfo;
      localInfo = new CDACLocalInfo(countOnly,
              createInputStream(countOnly, args, 0),
              createInputStream(countOnly, args, 1), s);
      local.addr = localInfo;
#ifdef CDAC_SPATIAL_JOIN_METRICS
      Merger::resetLoopStats();
#endif
   }

   // REQUEST: get next result tuple block (or call getNext() simply to count
   // intersections)
   if (countOnly) {
      localInfo->getNext();
   } else if (message == REQUEST) {
      result.addr = localInfo ? localInfo->getNext() : nullptr;
      return result.addr ? YIELD : CANCEL;
   }

   // CLOSE:
   if (countOnly || message == CLOSE) {
      if (countOnly) {
         size_t joinCount = localInfo->getIntersectionCount();
         qp->ResultStorage<LongInt>(result, s).Set(true, joinCount);
      }
#ifdef CDAC_SPATIAL_JOIN_METRICS
      Merger::stats->report(cout);
      Merger::resetLoopStats();
#endif
      delete localInfo;
      local.addr = nullptr;
   }

   return 0;
}

InputStream* CDACSpatialJoin::createInputStream(const bool countOnly,
        Word* args, const unsigned streamIndex) {
   // extract information about the first or second stream, creating an
   // InputStream instance for either a tuple block stream or a mere
   // tuple stream
   unsigned argIndex = MAX_ARG_COUNT + 6 * streamIndex;
   const auto attrIndex = static_cast<unsigned>(
           (static_cast<CcInt*>(args[argIndex++].addr))->GetValue());
   const auto attrCount = static_cast<unsigned>(
           (static_cast<CcInt*>(args[argIndex++].addr))->GetValue());
   const auto dim = static_cast<unsigned>(
           (static_cast<CcInt*>(args[argIndex++].addr))->GetValue());
   const auto isTBlockStream =
           (static_cast<CcBool*>(args[argIndex++].addr))->GetValue();
   const auto tBlockColumns = static_cast<ListExpr>(
           (static_cast<CcInt*>(args[argIndex++].addr))->GetValue());
   const auto blockSize = static_cast<uint64_t>(
           (static_cast<CcInt*>(args[argIndex].addr))->GetValue());

   if (isTBlockStream) {
      // input is stream of tuple blocks
      return new InputTBlockStream(args[streamIndex], countOnly, attrIndex,
                                       attrCount, dim, blockSize);
   } else {
      // input is stream of tuples which will be inserted into tuple
      // blocks by this operator using the DEFAULT_INPUT_BLOCK_SIZE
      ListExpr tBlockType = nl->TwoElemList(nl->SymbolAtom("tblock"),
              nl->TwoElemList(nl->IntAtom(DEFAULT_INPUT_BLOCK_SIZE),
                              tBlockColumns));
      // construct TBlockTI; information in tBlock type is not numeric
      CRelAlgebra::TBlockTI tBlockTI(tBlockType, false);
      return new InputTupleStream(args[streamIndex], countOnly,
              attrIndex, attrCount, dim, tBlockTI.GetBlockInfo(),
              tBlockTI.GetDesiredBlockSize());
   }
}

class CDACSpatialJoinCount::Info : public OperatorInfo {
public:
   Info() {
      name = "cdacspatialjoincount";
      signature = "stream (tblock (a ((x1 t1) ... (xn tn)))) x \n"
                  "stream (tblock (b ((y1 d1) ... (ym dm)))) x \n"
                  "xi x yj -> int";
      syntax = "_ _ cdacspatialjoincount [ _ , _  ]";
      meaning = "Operator counting the number of result tuples which a spatial "
                "join on the two input streams would produce. Uses the "
                "algorithm of cdacspatialjoin. xi and yj (both optional) are "
                "the names of the join attributes of the first and second "
                "stream, respectively. Returns the number of result tuples "
                "(but not the tuples themselves).";
      example = "query Roads feed {a} Water feed {b} "
                "cdacspatialjoincount[GeoData_a, GeoData_b]";
   }
};

shared_ptr<Operator> CDACSpatialJoinCount::getOperator() {
   return make_shared<Operator>(
           Info(),
           &CDACSpatialJoinCount::valueMapping,
           &CDACSpatialJoinCount::typeMapping);
}


ListExpr CDACSpatialJoinCount::typeMapping(ListExpr args) {
   return CDACSpatialJoin::typeMapping2(true, args);
}

int CDACSpatialJoinCount::valueMapping(Word* args, Word& result, int message,
                           Word& local, Supplier s) {
   return CDACSpatialJoin::valueMapping2(true, args, result, message, local, s);
}


// ========================================================
/*
1.5 MemoryInfo struct

*/
void MemoryInfo::add(const JoinStateMemoryInfo& joinStateInfo) {
   ++joinStateCount;

   maxMemInputData = std::max(maxMemInputData,
                              joinStateInfo.usedInputDataMemory);
   maxMemSortEdges = std::max(maxMemSortEdges,
                              joinStateInfo.usedSortEdgeMemory);
   maxMemRectInfos = std::max(maxMemRectInfos,
                              joinStateInfo.usedRectInfoMemory);
   maxMemJoinEdges = std::max(maxMemJoinEdges,
                              joinStateInfo.usedJoinEdgeMemory);
   maxMemMergedAreas = std::max(maxMemMergedAreas,
                                joinStateInfo.usedMergedAreaMemoryMax);
   maxMemTotal = std::max(maxMemTotal,
                          joinStateInfo.getTotalUsedMemoryMax());

   sumMemInputData += joinStateInfo.usedInputDataMemory;
   sumMemSortEdges += joinStateInfo.usedSortEdgeMemory;
   sumMemRectInfos += joinStateInfo.usedRectInfoMemory;
   sumMemJoinEdges += joinStateInfo.usedJoinEdgeMemory;
   sumMemMergedAreas += joinStateInfo.usedMergedAreaMemoryMax;
   sumMemTotal += joinStateInfo.getTotalUsedMemoryMax();

   maxJoinEdgeQuota = std::max(maxJoinEdgeQuota,
                               joinStateInfo.getUsedJoinEdgeQuotaMax());
}

void MemoryInfo::print(ostream& out) {
   CacheInfoPtr cacheInfo = CacheInfos::getCacheInfo(CacheType::Data, 1);
   const unsigned lineSize = cacheInfo ? cacheInfo->coherencyLineSize : 64U;
   cout << endl;
   if (joinStateCount > 1) {
      cout << "Data type      |   total memory | JoinSt. max | JoinSt. avg |"
           << " cache lines | note" << endl;
      cout << "---------------+----------------+-------------+-------------+";
   } else {
      cout << "Data type      |         memory |"
           << " cache lines | note" << endl;
      cout << "---------------+----------------+";
   }
   cout << "-------------+" << string(39, '-') << endl;
   printLine(cout, "Input data", sumMemInputData, maxMemInputData,
             "same lifetime as respective JoinState", lineSize);
   printLine(cout, "RectangleInfos", sumMemRectInfos, maxMemRectInfos,
             "same lifetime as JoinState constructor", lineSize);
   printLine(cout, "SortEdges", sumMemSortEdges, maxMemSortEdges,
             "same lifetime as JoinState constructor", lineSize);
   printLine(cout, "JoinEdges", sumMemJoinEdges, maxMemJoinEdges,
             "same lifetime as respective JoinState", lineSize);
   // for MergedAreas, memory was reserved for the worst case, i.e. for
   // maxJoinEdgeQuota = 2.0 (which means that rectangles are very wide and
   // JoinEdges are not moved to the "complete" set prior to the last merge)
   // In any case, 1.0 <= maxJoinEdgeQuota <= 2.0.
   stringstream stMaxQuota;
   stMaxQuota << "reserved space used by max. "
              << maxJoinEdgeQuota / 2.0 * 100.0 << "%";
   printLine(cout, "MergedAreas", sumMemMergedAreas, maxMemMergedAreas,
             stMaxQuota.str(), lineSize);

   cout << endl << "Maximum memory used at any given time: "
        << setw(11) << formatInt(maxMemTotal) << " bytes "
        << "(" << formatInt(maxMemTotal >> 20U) << " MiB)" << endl;
}

void MemoryInfo::printLine(ostream& out, const string& text,
        const size_t sumValue, const size_t maxValue, const string& note,
        const unsigned cacheLineSize) {
   out << text;
   if (text.length() < 14) {
      out << string(14 - text.length(), ' ');
   }
   out << " |" << setw(15) << formatInt(sumValue);
   size_t avgValue = (joinStateCount > 0) ? sumValue / joinStateCount : 0;
   if (joinStateCount > 1) {
      out << " |" << setw(12) << formatInt(maxValue);
      out << " |" << setw(12) << formatInt(avgValue);
   }
   out << " |" << setw(12) << formatInt(avgValue / cacheLineSize);
   out << " | " << note << endl;
}

// ========================================================
/*
1.6 LocalInfo class

1.6.1 constructor

*/
unsigned CDACLocalInfo::activeInstanceCount = 0;

CDACLocalInfo::CDACLocalInfo(const bool countOnly_, InputStream* const input1_,
                             InputStream* const input2_, Supplier s_) :
        countOnly(countOnly_),
        input1(input1_),
        input2(input2_),
        s(s_),
        isFirstRequest(true),
        memLimit(qp->GetMemorySize(s) * 1024 * 1024),

        // Extract information about result tuple block type and size
        outTypeInfo(countOnly ? CRelAlgebra::TBlockTI(true) :
                                CRelAlgebra::TBlockTI(qp->GetType(s), false)),
        outTBlockInfo(countOnly ? nullptr : outTypeInfo.GetBlockInfo()),
        outTBlockSize(countOnly ? 0 : outTypeInfo.GetDesiredBlockSize()
                    * CRelAlgebra::TBlockTI::blockSizeFactor),
        instanceNum(++activeInstanceCount),
        joinState(nullptr),
        joinStateCount(0),
        intersectionCount(0),
        timer(nullptr) {

#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   cout << "sizeof(SortEdge) = " << sizeof(SortEdge) << endl;
   cout << "sizeof(JoinEdge) = " << sizeof(JoinEdge) << endl;
   cout << "sizeof(RectangleInfo) = " << sizeof(RectangleInfo) << endl;
   cout << endl;
#endif

   /* a vector of task names that correspond to the elements of the JoinTask
    * enumeration */
   const vector<string> taskNames { {
         "requestData",
         "createJoinState",
         "createSortEdges",
         "sortSortEdges",
         "createJoinEdges",
         "merge"
   } };
   timer = make_shared<Timer>(taskNames);
}

/*
1.6.2  destructor

*/
CDACLocalInfo::~CDACLocalInfo() {
   timer->stop();

   // get operator info *before* "delete input1/2" may decrease
   // activeInstanceCount (in case input1 and/or input2 used another
   // CDACSpatialJoin operator)
   stringstream opInfo;
   if (activeInstanceCount > 1) {
      opInfo << "operator " << instanceNum << " (" << getOperatorName() << ")";
   } else {
      opInfo << getOperatorName();
   }
   opInfo << " with "<< joinStateCount << " ";
   opInfo << ((joinStateCount == 1) ? "JoinState" : "JoinStates");

   // delete members
   delete joinState;
   delete input1;
   delete input2;

#ifdef CDAC_SPATIAL_JOIN_METRICS
   // print memoryInfo (only after input1 and input2 were deleted: if input1
   // and/or input2 used another CDACSpatialJoin operator, their memoryInfo
   // (and timer) gets reported first which is more intuitive)
   cout << endl << "Memory (in bytes and cache lines) used for "
        << opInfo.str() << ":" << endl;
   memoryInfo.print(cout);
#endif

// #ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   cout << endl << "Time used for " << opInfo.str() << ":" << endl;
   timer->reportTable(cout, true, true, true, false, false);
// #endif

   --activeInstanceCount;
}

/*
1.6.3 support functions

*/
void CDACLocalInfo::requestInput() {
   uint64_t blockSize1 = input1->blockSizeInBytes;
   uint64_t blockSize2 = input2->blockSizeInBytes;
   while (true) {
      uint64_t usedMemory = getRequiredMemory();
      bool deny1 = input1->isDone() || usedMemory + blockSize1 >= memLimit;
      bool deny2 = input2->isDone() || usedMemory + blockSize2 >= memLimit;

      if (deny1 && deny2)
         break;
      else if (deny1)
         input2->request();
      else if (deny2)
         input1->request();
      else {
         // both streams may be requested; choose the stream from which fewer
         // tuples have been read so far
         if (input1->getCurrentTupleCount() > input2->getCurrentTupleCount()
             || input1->isAverageTupleCountExceeded()) {
            input2->request();
         } else {
            input1->request();
         }
      }
   }
}

size_t CDACLocalInfo::getRequiredMemory() const {
   const size_t tupleSum = input1->getCurrentTupleCount() +
           input2->getCurrentTupleCount();

   // first, we estimate the memory required by the JoinState constructor
   // (of which the SortEdge and RectangleInfo part will be released on
   // completion of the constructor):
   const size_t joinStateConstruction = tupleSum * (2 * sizeof(SortEdge) +
           sizeof(RectangleInfo) + 2 * sizeof(JoinEdge));

   // during JoinState execution, we must consider both JoinState::joinEdges
   // (2 * ...) and JoinState::mergedAreas (another 2 * ...): mergedAreas
   // duplicate JoinEdges. They are constructed over time (not all at once),
   // and the number of JoinEdges stored here is being reduced with every merge
   // step, since MergedArea::complete only stores *one* edge (rather than two
   // edges) per rectangle. In extreme cases, however (where all rectangles are
   // completed only with the last merge step), 2.0 * ... are required for all
   // mergedAreas. If rectangles are narrower, a value in [1.0, 2.0] is
   // possible. However, joinStateConstruction tends to be the dominant value
   const size_t joinStateExecution = tupleSum * ((2 + 2) * sizeof(JoinEdge));

   // since JoinState construction and execution take place sequentially,
   // the maximum (rather than the sum) can be used:
   return input1->getUsedMem() + input2->getUsedMem() +
         std::max(joinStateConstruction, joinStateExecution);
}

string CDACLocalInfo::getOperatorName() const {
   return countOnly ? "CDACSpatialJoinCount" : "CDACSpatialJoin";
}

/*
1.6.4 getNext() function

*/
CRelAlgebra::TBlock* CDACLocalInfo::getNext() {
   CRelAlgebra::TBlock* outTBlock = nullptr;
   if (!countOnly) {
      outTBlock = new CRelAlgebra::TBlock(outTBlockInfo, 0, 0);
   }

   while (true) {
      // if a JoinState has been created, ...
      if (joinState) {
         // ... return the next block of join results
         if (countOnly) {
            joinState->nextTBlock(nullptr);
         } else {
            if (joinState->nextTBlock(outTBlock)) {
               timer->stop();
               return outTBlock;
            }
         }
         intersectionCount += joinState->getOutTupleCount();

#ifdef CDAC_SPATIAL_JOIN_METRICS
         memoryInfo.add(joinState->getMemoryInfo());
#endif
         delete joinState;
         joinState = nullptr;
      }

      // read (more) data
      if (isFirstRequest) {
         // first attempt to read from the streams
         timer->start(JoinTask::requestData);
         isFirstRequest = false; // prevent this block from being entered twice

         // test if any of the streams is empty - then nothing to do
         if (!input1->request() || !input2->request()) {
            if (outTBlock) {
               outTBlock->DecRef();
            }
            timer->stop();
            return nullptr;
         }

         // read as much as possible from both input streams
         requestInput();

         // continue creating a JoinState below

      } else if (input1->isDone() && input2->isDone()) {
         // all input was read, join is complete
         if (outTBlock) {
            assert (outTBlock->GetRowCount() == 0);
            outTBlock->DecRef();
         }
         timer->stop();
#ifndef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
         cout << "\r" << string(100, ' ') << "\r" << flush;
#endif
         return nullptr;

      } else {
         // neither first request nor join complete,
         // i.e. the tuple data did not fit completely into the main memory;
         // the tuples read so far were treated, now read and treat more data

         timer->start(JoinTask::requestData);
         if (input1->isFullyLoaded()) {
            // continue reading from input2
            uint64_t blockSize2 = input2->blockSizeInBytes;
            input2->clearMem();
            do {
               input2->request();
            } while (!input2->isDone() &&
                  getRequiredMemory() + blockSize2 < memLimit);
         } else if (!input1->isDone()) {
            // continue reading from input1
            uint64_t blockSize1 = input1->blockSizeInBytes;
            input1->clearMem();
            do {
               input1->request();
            } while (!input1->isDone() &&
                  getRequiredMemory() + blockSize1 < memLimit);
         } else {
            // input1 is done, but input2 is not
            input1->clearMem();
            input2->clearMem();
            // read next bit from input2, restarting input1 from the beginning
            // (i.e. input2 is the "outer loop", input1 the "inner loop")
            if (input2->request()) {
               input1->restart();
               requestInput();
            }
         }
         // continue creating a JoinState below
      }

      // create a JoinState from the data that was read to the main memory
      if (!input1->empty() && !input2->empty()) {
         assert (!joinState);

#ifndef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
         // only report the progress of the last instance (if several
         // CDACSpatialJoin[Count] operators are used within one query)
         if (instanceNum == activeInstanceCount) {
            cout << "\r" << "running "
                 << (countOnly ? "CDACSpatialJoinCount" : "CDACSpatialJoin");
            if (instanceNum > 1)
               cout << " operator " << instanceNum;
            if (joinStateCount > 0) {
               cout << ": " << setw(11) << formatInt(intersectionCount)
                    << " intersections found in " << joinStateCount
                    << (joinStateCount == 1 ? " JoinState " : " JoinStates");
            }
            cout << "..." << flush;
         }
#endif

         timer->start(JoinTask::createJoinState);
         ++joinStateCount;
         joinState = new JoinState(countOnly, input1, input2, outTBlockSize,
                 instanceNum, joinStateCount, timer);
      } else {
         // a "requestData" task was started above but both input streams were
         // actually done; do not count this as a "requestData" instance
         // to avoid distortion of the average requestData time
         Task* curTask = timer->getCurrentTask();
         if (curTask)
            curTask->decreaseCount();
      }

   } // end of while loop
} // end of getNext() function
