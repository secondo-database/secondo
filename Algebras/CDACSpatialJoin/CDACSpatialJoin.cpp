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


1 Cache-oriented Spatial Join with Divide and Conquer


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

uint64_t CDACSpatialJoin::DEFAULT_INPUT_BLOCK_SIZE_MIB = 10;
uint64_t CDACLocalInfo::DEFAULT_OUTPUT_TUPLE_VECTOR_MEM_SIZE_KIB = 256;

/*
1.2 Class OperatorInfo

A subclass of class ~OperatorInfo~ is defined with information on the operator.

*/
class CDACSpatialJoin::Info : public OperatorInfo {
public:
   Info() {
      name = "cdacspatialjoin";
      signature = "stream (tuple or tblock (a ((x1 t1) ... (xn tn)))) x \n"
         "stream (tuple or tblock (b ((y1 d1) ... (ym dm)))) x \n"
         "xi x yj x int \n"
         "-> \n"
         "stream (tuple or tblock (c ((x1 t1) ... (xn tn) "
         "(y1 d1) ... (ym dm))))";
      syntax = "_ _ cdacspatialjoin [ _ , _  , _ ]";
      meaning = "Cache-oriented spatial join operator using divide and "
                "conquer to perform a spatial join on two streams of tuples "
                "or tuple blocks, where xi and yj (both optional) are the "
                "names of the join attributes of the first and second stream, "
                "respectively, and int is the (optional) size of the output "
                "TBlocks in MiB. If both inputs are tuple streams and no "
                "fifth parameter is given, a stream of tuples is returned, "
                "otherwise a stream of TBlocks.";
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

The type mapping checks the number (2 to 5) and type of the arguments
passed to the operator: Parameter 1 and 2 must either be streams of tuples,
or streams of tuple blocks, respectively. Parameter 3 and 4 (optional) may be
the names of the join attributes of the first and the second stream,
respectively. Parameter 5 (optional, not expected for CDACSpatialJoinCount)
may be the desired output TBlock size in MiB. If both input streams are
tuple streams and no fifth parameter is given, a tuple stream is returned,
otherwise a TBlock stream.

*/
ListExpr CDACSpatialJoin::typeMapping(ListExpr args) {
   return typeMapping2(false, args);
}

ListExpr CDACSpatialJoin::typeMapping2(const bool countOnly, ListExpr args) {
   // check the number of arguments; a fifth argument is only expected for
   // the CDACSpatialJoin operator if the desired output type is a TBlock
   // stream  (not for CDACSpatialJoinCount or for an ouput tuple stream)
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

      // extract information about tuple blocks. We use tBlockInfo here, even
      // if the desired output may be a tuple stream
      tBlockColumns[i] = 0; // is returned by getTBlockTI()
      tBlockInfo[i] = isTBlockStream[i] ?
         CRelAlgebra::TBlockTI(nl->Second(stream[i]), false) :
         getTBlockTI(nl->Second(nl->Second(stream[i])),
                     DEFAULT_INPUT_BLOCK_SIZE_MIB, tBlockColumns[i]);

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

   // get explicit result block size if provided in the query
   long outTBlockSize = 0;
   if (argCount == 5) {
      ListExpr outTBlockSizeLE = nl->Fifth(args);
      if (!CcInt::checkType(nl->First(outTBlockSizeLE)))
         return listutils::typeError(
                 "Error in fifth argument: int expected.");
      outTBlockSize = nl->IntValue(nl->Second(outTBlockSizeLE));
   }

   // determine the output type: CDACSpatialJoin returns a tuple stream only
   // if both input streams are tuple streams and no outTBlockSize is specified
   OutputType outputType = countOnly ? outputCount :
           (isTBlockStream[0] || isTBlockStream[1] || argCount == 5 ?
           outputTBlockStream : outputTupleStream);

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
   appendEnd = nl->Append(appendEnd, nl->IntAtom((int)outputType));
   for (unsigned i = 0; i < STREAM_COUNT; ++i) {
      appendEnd = nl->Append(appendEnd, nl->IntAtom(attrIndex[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(attrCount[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(dim[i]));
      appendEnd = nl->Append(appendEnd, nl->BoolAtom(isTBlockStream[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(tBlockColumns[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(blockSize[i]));
   }

   if (outputType != outputCount) {
      // check for duplicate column names (again, we use tBlockInfo here even
      // if the desired output is a tuple stream)
      set<string> columnNames;
      for (const TBlockColInfo& colInfo : tBlockInfo[0].columnInfos)
         columnNames.insert(colInfo.name);
      for (const TBlockColInfo& colInfo : tBlockInfo[1].columnInfos) {
         if (!columnNames.insert(colInfo.name).second) {
            return listutils::typeError("Column name " + colInfo.name +
                                        " exists in both relations");
         }
      }
   }

   ListExpr resultType = 0;
   switch (outputType) {
      case outputCount: {
         // the result type is an integer value
         resultType = nl->SymbolAtom(LongInt::BasicType());
         break;
      }
      case outputTupleStream: {
         // the result  type is a tuple stream
         // concatenate the input tuple attributes to get the output attributes
         ListExpr attrList1 = nl->Second(streamType[0]);
         ListExpr attrList2 = nl->Second(streamType[1]);
         ListExpr outputAttrList = listutils::concat(attrList1, attrList2);

         resultType = nl->TwoElemList(
                 nl->SymbolAtom(Stream<Tuple>::BasicType()),
                 nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                 outputAttrList));
         break;
      }
      case outputTBlockStream: {
         // the result type is a TBlock stream.
         // Initialize its type information
         CRelAlgebra::TBlockTI resultTBlockInfo = CRelAlgebra::TBlockTI(false);

         // set the size of the result tuple block, using the larger input
         // block size or the explicitly provided result block size
         uint64_t desiredBlockSize = max(tBlockInfo[0].GetDesiredBlockSize(),
                                         tBlockInfo[1].GetDesiredBlockSize());
         if (outTBlockSize > 0)
            desiredBlockSize = static_cast<uint64_t>(outTBlockSize);
         resultTBlockInfo.SetDesiredBlockSize(desiredBlockSize);

         // add columns
         for (const TBlockColInfo& colInfo : tBlockInfo[0].columnInfos)
            resultTBlockInfo.columnInfos.push_back(colInfo);
         for (const TBlockColInfo& colInfo : tBlockInfo[1].columnInfos)
            resultTBlockInfo.columnInfos.push_back(colInfo);

         resultType = resultTBlockInfo.GetTypeExpr(true);
         break;
      }
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
int CDACSpatialJoin::valueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s) {

   auto localInfo = static_cast<CDACLocalInfo*>(local.addr);

   // get the desired output type. Note that for message == CLOSE, this value
   // is NOT reliable (e.g. it may be outputCount in a CDACSpatialJoin query)!
   // example: let partOfMensa = [const (rel (tuple ((Bbox rect)) )) value
   //                            (((7.49577 7.49578 51.3767 51.3768)))];
   //         query cbuildings feed partOfMensa feed cdacspatialjoin[] consume;
   // Now, when valueMapping is called with message == CLOSE, outputType
   // is set to outputCount (= 0), although this is a outputTBlockStream query.
   const auto outputType = static_cast<OutputType>(
           (static_cast<CcInt*>(args[MAX_ARG_COUNT].addr))->GetValue());

   // OPEN: create LocalInfo instance
   if (message == OPEN) {
      delete localInfo;
      ListExpr tupleTypeLE = (outputType == outputTupleStream) ?
                             nl->Second(GetTupleResultType(s)) : 0;
      localInfo = new CDACLocalInfo(outputType,
              tupleTypeLE,
              createInputStream(outputType, args, 0),
              createInputStream(outputType, args, 1), s);
      local.addr = localInfo;
#ifdef CDAC_SPATIAL_JOIN_METRICS
      MergerStats::onlyInstance->reset();
#endif
   }

   // REQUEST: depending on the desired outputType.
   if (message == CLOSE) {
      // with message == CLOSE, the value of outputType is not reliable,
      // so we must not depend on it. Therefore this block is skipped.
   } else if (outputType == outputCount) { // already for message == OPEN
      // call getNext() simply to count intersections
      localInfo->getNext();
      size_t joinCount = localInfo->getIntersectionCount();
      qp->ResultStorage<LongInt>(result, s).Set(true, joinCount);

   } else if (message == REQUEST) {
      if (outputType == outputTupleStream) {
         // get next result tuple
         result.addr = localInfo ? localInfo->getNextTuple() : nullptr;
      } else { // outputType == outputTBlockStream
         // get next result TBlock
         result.addr = localInfo ? localInfo->getNextTBlock() : nullptr;
      }
      return result.addr ? YIELD : CANCEL;
   }

   // CLOSE
   if (message == CLOSE) {
#ifdef CDAC_SPATIAL_JOIN_METRICS
      MergerStats::onlyInstance->report(cout);
      MergerStats::onlyInstance->reset();
#endif
      delete localInfo;
      local.addr = nullptr;
   }

   return 0;
}

InputStream* CDACSpatialJoin::createInputStream(const OutputType outputType,
        Word* args, const unsigned streamIndex) {
   // extract information about the first or second stream, creating an
   // InputStream instance for either a tuple block stream or a mere
   // tuple stream
   unsigned argIndex = MAX_ARG_COUNT + 1 + 6 * streamIndex;
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
      return new InputTBlockStream(args[streamIndex], outputType, attrIndex,
                                   attrCount, dim, blockSize);
   } else if (outputType == outputTupleStream) {
      // both this input and the output is a stream of tuples
      return new InputTupleStream(args[streamIndex], outputType, attrIndex,
              attrCount, dim, nullptr, blockSize);
   } else {
      // input is stream of tuples which will be inserted into tuple
      // blocks by this operator using the DEFAULT_INPUT_BLOCK_SIZE
      ListExpr tBlockType = nl->TwoElemList(nl->SymbolAtom("tblock"),
              nl->TwoElemList(nl->IntAtom(DEFAULT_INPUT_BLOCK_SIZE_MIB),
                              tBlockColumns));
      // construct TBlockTI; information in tBlock type is not numeric
      CRelAlgebra::TBlockTI tBlockTI(tBlockType, false);
      return new InputTupleStream(args[streamIndex], outputType,
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
           &CDACSpatialJoin::valueMapping,
           &CDACSpatialJoinCount::typeMapping);
}


ListExpr CDACSpatialJoinCount::typeMapping(ListExpr args) {
   return CDACSpatialJoin::typeMapping2(true, args);
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
   maxMemOutputDataAddSize = std::max(maxMemOutputDataAddSize,
                              joinStateInfo.outputDataAddSizeMax);
   maxMemOutputDataMemSize = std::max(maxMemOutputDataMemSize,
                                      joinStateInfo.outputDataMemSizeMax);
   maxMemTotal = std::max(maxMemTotal,
                              joinStateInfo.getTotalUsedMemoryMax());

   sumMemInputData += joinStateInfo.usedInputDataMemory;
   sumMemSortEdges += joinStateInfo.usedSortEdgeMemory;
   sumMemRectInfos += joinStateInfo.usedRectInfoMemory;
   sumMemJoinEdges += joinStateInfo.usedJoinEdgeMemory;
   sumMemMergedAreas += joinStateInfo.usedMergedAreaMemoryMax;
   sumMemOutputDataAddSizeMax += joinStateInfo.outputDataAddSizeMax;
   sumMemOutputDataMemSizeMax += joinStateInfo.outputDataMemSizeMax;
   sumMemTotal += joinStateInfo.getTotalUsedMemoryMax();

   totalOutputTupleCount += joinStateInfo.outputTupleCount;
   totalOutputDataAddSize += joinStateInfo.outputDataAddSize;
   totalOutputDataMemSize += joinStateInfo.outputDataMemSize;

   maxJoinEdgeQuota = std::max(maxJoinEdgeQuota,
                               joinStateInfo.getUsedJoinEdgeQuotaMax());
}

void MemoryInfo::setInputSize(
     const size_t totalInputATupleCount_, const size_t totalInputADataSize_,
     const size_t totalInputBTupleCount_, const size_t totalInputBDataSize_) {
   totalInputATupleCount = totalInputATupleCount_;
   totalInputADataSize = totalInputADataSize_;
   totalInputBTupleCount = totalInputBTupleCount_;
   totalInputBDataSize = totalInputBDataSize_;
}

void MemoryInfo::print(ostream& out, OutputType outputType) {
   CacheInfoPtr cacheInfo = CacheInfos::getCacheInfo(CacheType::Data, 1);
   const unsigned lineSize = cacheInfo ? cacheInfo->coherencyLineSize : 64U;
   cout << endl;
   if (joinStateCount > 1) {
      cout << "Data type      |   total memory |  JoinSt. max |  JoinSt. avg |"
           << " cache lines | note" << endl;
      cout << "---------------+----------------+--------------+--------------+";
   } else {
      cout << "Data type      |         memory |"
           << " cache lines | note" << endl;
      cout << "---------------+----------------+";
   }
   cout << "-------------+" << string(39, '-') << endl;
   printLineMem(cout, "Input data", sumMemInputData, maxMemInputData,
             "same lifetime as respective JoinState", lineSize);
   printLineMem(cout, "RectangleInfos", sumMemRectInfos, maxMemRectInfos,
             "same lifetime as JoinState constructor", lineSize);
   printLineMem(cout, "SortEdges", sumMemSortEdges, maxMemSortEdges,
             "same lifetime as JoinState constructor", lineSize);
   printLineMem(cout, "JoinEdges", sumMemJoinEdges, maxMemJoinEdges,
             "same lifetime as respective JoinState", lineSize);
   // for MergedAreas, memory was reserved for the worst case, i.e. for
   // maxJoinEdgeQuota = 2.0 (which means that rectangles are very wide and
   // JoinEdges are not moved to the "complete" set prior to the last merge)
   // In any case, 1.0 <= maxJoinEdgeQuota <= 2.0.
   stringstream stMaxQuota;
   stMaxQuota << "reserved space used by max. "
              << maxJoinEdgeQuota / 2.0 * 100.0 << "%";
   printLineMem(cout, "MergedAreas", sumMemMergedAreas, maxMemMergedAreas,
             stMaxQuota.str(), lineSize);
   printLineMem(cout, "Output data", sumMemOutputDataAddSizeMax,
           maxMemOutputDataAddSize, "refers to the largest output block",
           lineSize);

   cout << endl << "Maximum memory used at any given time: "
        << setw(11) << formatInt(maxMemTotal) << " bytes "
        << "(" << formatInt(maxMemTotal >> 20U) << " MiB)" << endl << endl;

   printLineInOut(cout, "Total input from stream A: ",
           totalInputADataSize, totalInputATupleCount, "");
   printLineInOut(cout, "Total input from stream B: ",
           totalInputBDataSize, totalInputBTupleCount, "");
   if (outputType == outputTupleStream) {
      printLineInOut(cout, "Additional output data   : ",
         totalOutputDataAddSize, totalOutputTupleCount,
         "(excluding Attribute instances shared between input and output)");
      printLineInOut(cout, "Total output data        : ",
         totalOutputDataMemSize, totalOutputTupleCount,
         "(including Attribute instances shared between input and output)");
   } else {
      printLineInOut(cout, "Total output data        : ",
                     totalOutputDataMemSize, totalOutputTupleCount, "");
   }
   cout << endl;
}

void MemoryInfo::printLineMem(ostream& out, const string& text,
        const size_t sumValue, const size_t maxValue, const string& note,
        const unsigned cacheLineSize) {
   out << text;
   if (text.length() < 14) {
      out << string(14 - text.length(), ' ');
   }
   out << " |" << setw(15) << formatInt(sumValue);
   size_t avgValue = (joinStateCount > 0) ? sumValue / joinStateCount : 0;
   if (joinStateCount > 1) {
      out << " |" << setw(13) << formatInt(maxValue);
      out << " |" << setw(13) << formatInt(avgValue);
   }
   out << " |" << setw(12) << formatInt(avgValue / cacheLineSize);
   out << " | " << note << endl;
}

void MemoryInfo::printLineInOut(std::ostream& out, const std::string& text,
        uint64_t bytes, uint64_t tupleCount, const std::string& note) {
   out << text;
   out << setw(15) << formatInt(bytes) << " bytes"
        << " (" << formatInt(bytes >> 20U) << " MiB)";
   if (tupleCount > 0) {
      out << " = " << (bytes / (double)tupleCount) << " bytes "
           << "* " << formatInt(tupleCount) << " tuples";
   }
   out << " " << note << endl;
}

// ========================================================
/*
1.6 LocalInfo class

1.6.1 constructor

*/
unsigned CDACLocalInfo::activeInstanceCount = 0;

CDACLocalInfo::CDACLocalInfo(const OutputType outputType_,
                             ListExpr outputTupleTypeLE,
        InputStream* const inputA_, InputStream* const inputB_, Supplier s_) :
        outputType(outputType_),
        outputTupleType(outputTupleTypeLE == 0 ?
            nullptr : new TupleType(outputTupleTypeLE)),
        inputA(inputA_),
        inputB(inputB_),
        s(s_),
        isFirstRequest(true),
        memLimit(qp->GetMemorySize(s) * 1024 * 1024),

        // Extract information about result tuple block type and size
        outTypeInfo(outputType == outputTBlockStream ?
            CRelAlgebra::TBlockTI(qp->GetType(s), false) :
            CRelAlgebra::TBlockTI(true)),
        outTBlockInfo(outputType == outputTBlockStream ?
            outTypeInfo.GetBlockInfo() : nullptr),
        outBufferSize(outputType == outputTBlockStream ?
            outTypeInfo.GetDesiredBlockSize()
                * CRelAlgebra::TBlockTI::blockSizeFactor :
            DEFAULT_OUTPUT_TUPLE_VECTOR_MEM_SIZE_KIB * 1024),
        outTupleAddSize(0),
        outBufferTupleCountMax(0),
        instanceNum(++activeInstanceCount),
        joinState(nullptr),
        joinStateCount(0),
        intersectionCount(0),
        outTBlock(nullptr),
        outTuples(outputType == outputTupleStream ?
            new vector<Tuple*>() : nullptr),
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
         "createSortEdges",
         "sortSortEdges",
         "createJoinEdges",
         "merge",
         "saveToTempFile",
         "clearMemory"
   } };
   timer = make_shared<Timer>(taskNames);

   if (outputTupleType) {
      // since the input tuples and their attributes already exist in memory
      // when an output tuple is concatenated, the output tuple will usually
      // only get pointers to these attributes. Therefore it is sufficient to
      // measure the size of an empty tuple to get the extra memory size
      // required for output tuples
      auto emptyTuple = new Tuple(outputTupleType);
      outTupleAddSize = emptyTuple->GetMemSize();
      emptyTuple->DeleteIfAllowed();
      outBufferTupleCountMax = outBufferSize / outTupleAddSize;
      // we keep outTuplesSizeMax within the interval [1; 65534]:
      if (outBufferTupleCountMax < 1) {
         outBufferTupleCountMax = 1;
      } else if (outBufferTupleCountMax >
         std::numeric_limits<uint16_t>::max() - 1) {
         // see Attribute::Copy() in Algebras/Relation-C++/Attribute.cpp:
         // Once 65535 references to an attribute exist, the attribute will
         // be cloned each time another reference is required as the reference
         // counter AttrDelete::refs is of type uint16_t. In case of a
         // large input rectangle (which would typically be part of many
         // intersections!), every single attribute including its FLOBs will
         // then be cloned (x - 65535) times. This can be very costly, e.g. for
         // the tuple of the river Rhine in the Waterways relation of the NRW
         // database, which has > 1 million intersections with Buildings.
         // To avoid all this, we limit outTuplesSizeMax to ensure that no
         // output tuple's attributes will be referenced more than 65535 times
         // before the output tuples are flushed to the stream and consumed.
         outBufferTupleCountMax = std::numeric_limits<uint16_t>::max() - 1;
      }
      // recalculate outBufferSize to match outBufferTupleCountMax
      outBufferSize = outBufferTupleCountMax * outTupleAddSize;
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
      cout << "Output buffer: max. "
           << formatInt(outBufferTupleCountMax) << " tuples = "
           << formatInt(outBufferSize) << " bytes / "
           << formatInt(outTupleAddSize) << " bytes per tuple." << endl;
#endif
   }
}

/*
1.6.2  destructor

*/
CDACLocalInfo::~CDACLocalInfo() {
   timer->start(JoinTask::clearMemory);

   // remember activeInstanceCount before "delete inputA/B" may decrease it
   // (in case inputA and/or inputB used another CDACSpatialJoin operator)
   unsigned int activeInstanceCountCopy = activeInstanceCount;

   if (outTBlock) {
      outTBlock->DecRef();
      outTBlock = nullptr;
   }

   if (outTuples) {
      // normally, outTuples should be empty, but just in case ...
      if (!outTuples->empty()) {
         for (Tuple* tuple : *outTuples)
            tuple->DeleteIfAllowed();
         outTuples->clear();
      }
      delete outTuples;
   }

   delete joinState;
   delete inputA;
   delete inputB;

   // outputTupleType may still be used in the output tuples which were
   // written to the stream, so rather than simply deleting outTupleType, we
   // must call DeleteIfAllowed() which decreases the reference counter. This
   // should happen only after treating inputA, inputB, and outTuples above.
   if (outputTupleType) {
      outputTupleType->DeleteIfAllowed();
      outputTupleType = nullptr;
   }

   timer->stop();

   stringstream opInfo;
   if (activeInstanceCountCopy > 1) {
      opInfo << "operator " << instanceNum << " (" << getOperatorName() << ")";
   } else {
      opInfo << getOperatorName();
   }
   opInfo << " with "<< joinStateCount << " ";
   opInfo << ((joinStateCount == 1) ? "JoinState" : "JoinStates");

#ifdef CDAC_SPATIAL_JOIN_METRICS
   // print memoryInfo (only after inputA and inputB were deleted: if inputA
   // and/or inputB used another CDACSpatialJoin operator, their memoryInfo
   // (and timer) gets reported first which is more intuitive)
   cout << endl << "Memory (in bytes and cache lines) used for "
        << opInfo.str() << ":" << endl;
   memoryInfo.print(cout, outputType);
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
   uint64_t blockSize1 = inputA->blockSizeInBytes;
   uint64_t blockSize2 = inputB->blockSizeInBytes;
   while (true) {
      uint64_t usedMemory = getRequiredMemory();
      bool deny1 = inputA->isDone() ||
                  (!inputA->empty() && usedMemory + blockSize1 >= memLimit);
      bool deny2 = inputB->isDone() ||
                  (!inputB->empty() && usedMemory + blockSize2 >= memLimit);

      if (deny1 && deny2)
         break;
      else if (deny1)
         inputB->request();
      else if (deny2)
         inputA->request();
      else {
         // both streams may be requested; choose the stream from which fewer
         // tuples have been read so far
         if (inputA->getCurrentTupleCount() > inputB->getCurrentTupleCount()
             || inputA->isAverageTupleCountExceeded()) {
            inputB->request();
         } else {
            inputA->request();
         }
      }
   }
}

size_t CDACLocalInfo::getRequiredMemory() const {
   const size_t tupleSum = inputA->getCurrentTupleCount() +
           inputB->getCurrentTupleCount();

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

   // outBufferSize contains the number bytes reserved for the output TBlock
   // or the output tuple vector.
   size_t outputData = (outputType == outputCount) ? 0 : outBufferSize;

   // since JoinState construction and execution take place sequentially,
   // the maximum (rather than the sum) can be used:
   return inputA->getUsedMem() + inputB->getUsedMem() +
         std::max(joinStateConstruction, joinStateExecution) + outputData;
}

string CDACLocalInfo::getOperatorName() const {
   return (outputType == outputCount) ? "CDACSpatialJoinCount"
       : "CDACSpatialJoin";
}

/*
1.6.4 getNext() function

*/
CRelAlgebra::TBlock* CDACLocalInfo::getNextTBlock() {
   outTBlock = new CRelAlgebra::TBlock(outTBlockInfo, 0, 0);
   if (getNext()) {
      return outTBlock;
   }
   if (outTBlock) {
      assert (outTBlock->GetRowCount() == 0);
      outTBlock->DecRef();
      outTBlock = nullptr;
   }
   return nullptr;
}


Tuple* CDACLocalInfo::getNextTuple() {
   if (!outTuples->empty() || getNext()) {
      Tuple* tuple = outTuples->back();
      outTuples->pop_back();
      // do NOT perform tuple->DeleteIfAllowed() here
      return tuple;
   }
   // getNext() should have returned true if any outTuples were created
   assert (outTuples->empty());
   return nullptr;
}

bool CDACLocalInfo::getNext() {
   while (true) {
      // if a JoinState has been created, ...
      if (joinState) {
         if (outputType == outputCount) {
            // ... either count this JoinState's join results ...
            joinState->nextTBlock(nullptr, nullptr);
         } else {
            // ... or calculate the next block of join results
            if (joinState->nextTBlock(outTBlock, outTuples)) {
               return true;
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
         if (!inputA->request() || !inputB->request()) {
            return false;
         }

         // read as much as possible from both input streams
         requestInput();

         // continue creating a JoinState below

      } else if (inputA->isDone() && inputB->isDone()) {
         // all input was read, join is complete
#ifdef CDAC_SPATIAL_JOIN_METRICS
         memoryInfo.setInputSize(
                 inputA->getTotalTupleCount(), inputA->getTotalByteCount(),
                 inputB->getTotalTupleCount(), inputB->getTotalByteCount());
#endif
#ifndef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
         cout << "\r" << string(100, ' ') << "\r" << flush;
#endif
         return false;

      // in all remaining cases: join is incomplete, i.e. the tuple data did
      // not fit completely into the main memory; the tuples read so far were
      // processed, now read and process more data
      } else if (inputA->isFullyLoaded()) {
         // continue reading from inputB
         timer->start(JoinTask::clearMemory);
         inputB->clearMem();

         timer->start(JoinTask::requestData);
         uint64_t blockSize2 = inputB->blockSizeInBytes;
         do {
            inputB->request();
         } while (!inputB->isDone() &&
                  getRequiredMemory() + blockSize2 < memLimit);
         // continue creating a JoinState below

      } else {
         // if neither stream fits into the main memory, then the data of the
         // inner stream must be temporarily saved, if this is the first pass
         if (!inputB->isFullyLoaded() && inputB->getChunkCount() == 1) {
            timer->start(JoinTask::saveToTempFile);
            if (inputA->saveToTempFile()) {
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
               clock_t saveToTempFileTime = timer->stop();
               cout << "- saved stream A, chunk " << inputA->getChunkCount()
                    << " to temporary file ";
               cout << "in " << formatMillis(saveToTempFileTime) << endl;
#endif
            }
         }
         // read more data
         if (!inputA->isDone()) {
            // continue reading from inputA (the inner stream)
            timer->start(JoinTask::clearMemory);
            inputA->clearMem();

            uint64_t blockSize1 = inputA->blockSizeInBytes;
            timer->start(JoinTask::requestData);
            do {
               inputA->request();
            } while (!inputA->isDone() &&
                     getRequiredMemory() + blockSize1 < memLimit);
         } else {
            // inputA is done, but inputB is not
            timer->start(JoinTask::clearMemory);
            inputA->clearMem();
            inputB->clearMem();

            // read next bit from inputB, restarting inputA from the beginning
            // (i.e. inputB is the "outer loop", inputA the "inner loop")
            timer->start(JoinTask::requestData);
            if (inputB->request()) {
               inputA->restart();
               requestInput();
            }
         }
         // continue creating a JoinState below
      }

      // create a JoinState from the data that was read to the main memory
      if (!inputA->empty() && !inputB->empty()) {
         assert (!joinState);

#ifndef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
         // only report the progress of the last instance (if several
         // CDACSpatialJoin[Count] operators are used within one query)
         if (instanceNum == activeInstanceCount) {
            cout << "\r" << "running " << (outputType == outputCount ?
                "CDACSpatialJoinCount" : "CDACSpatialJoin");
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

         // timer->start(...) see JoinState constructor
         ++joinStateCount;
         joinState = new JoinState(outputType, outputTupleType,
                 inputA, inputB, outBufferSize, outTupleAddSize,
                 outBufferTupleCountMax, instanceNum, joinStateCount, timer);
         timer->start(JoinTask::merge);
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
