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


1 Cache-conscious Spatial Join with Divide and Conquer


1.1 Imports

*/
#include <iostream>
#include <math.h>

#include "NestedList.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "ListUtils.h"

#include "CDACSpatialJoin.h"
#include "CacheInfo.h"
#include "SortEdge.h"
#include "JoinEdge.h"

#include "Algebras/CRel/Operators/OperatorUtils.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"


typedef CRelAlgebra::TBlockTI::ColumnInfo TBlockColInfo;

using namespace cdacspatialjoin;


// TODO: guten Wert finden, ggf. abhängig von qp->GetMemorySize(s)
uint64_t CDACSpatialJoin::DEFAULT_BLOCK_SIZE = 10;

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
         "xi x yj \n"
         "-> \n"
         "stream (tblock (c ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm))))";
      syntax = "_ _ cdacspatialjoin [ _ , _ ]";
      meaning = "Cache-conscious spatial join operator performing "
                "a partitioned spatial join on two streams of tuples or tuple "
                "blocks, where xi and yj (both optional) are the names of the "
                "join attributes of the first and second stream, respectively.";
      example = "query Roads feed toblocks[1000] {a}"
                "Roads feed toblocks[1000] {b}"
                "cdacspatialjoin[GeoData_a, GeoData_b] count";
   }
};

std::shared_ptr<Operator> CDACSpatialJoin::getOperator() {
   return std::make_shared<Operator>(
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
   // check the number of arguments
   // last two arguments are included for testing purposes

   constexpr unsigned STREAM_COUNT = 2;

   auto argCount = static_cast<unsigned>(nl->ListLength(args));
   if (argCount < STREAM_COUNT || argCount > STREAM_COUNT + STREAM_COUNT)
      return listutils::typeError("Two, three or four arguments expected.");

   // prepare values to hold information on the two input streams
   ListExpr stream[STREAM_COUNT];
   ListExpr streamType[STREAM_COUNT];
   bool isTBlockStream[STREAM_COUNT];
   ListExpr tBlockColumns[STREAM_COUNT];
   CRelAlgebra::TBlockTI tBlockInfo[STREAM_COUNT] =
           { CRelAlgebra::TBlockTI(false), CRelAlgebra::TBlockTI(false) };
   std::string attrName[STREAM_COUNT];
   uint64_t attrIndex[STREAM_COUNT];
   unsigned dim[STREAM_COUNT];

   // get information on the two input streams
   for (unsigned i = 0; i < STREAM_COUNT; ++i) {
      // first and second arguments must be a streams
      std::string argPos1or2 = (i == 0) ? "first" : "second";
      stream[i] = (i == 0) ? nl->First(args) : nl->Second(args);
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
         getTBlockTI(nl->Second(nl->Second(stream[i])), DEFAULT_BLOCK_SIZE,
                     tBlockColumns[i]);

      // depending on whether a third / fourth argument is given, ...
      if (STREAM_COUNT + i < argCount) {
         // extract join attribute names and indices
         // third and fourth argument must be an attribute name
         std::string argPos3or4 = (i == 0) ? "third" : "fourth";
         ListExpr attrNameLE = (i == 0) ? nl->Third(args) : nl->Fourth(args);
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
         TBlockColInfo& col = tBlockInfo[i].columnInfos[attrIndex[i]];
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

   // Initialize the type and size of result tuple block
   // and check for duplicates column names
   CRelAlgebra::TBlockTI resultTBlockInfo = CRelAlgebra::TBlockTI(false);
   if (tBlockInfo[0].GetDesiredBlockSize()
         > tBlockInfo[1].GetDesiredBlockSize()) {
      resultTBlockInfo.SetDesiredBlockSize(tBlockInfo[0].GetDesiredBlockSize());
   } else {
      resultTBlockInfo.SetDesiredBlockSize(tBlockInfo[1].GetDesiredBlockSize());
   }
   std::set<std::string> columnNames; // helps to identify duplicates
   for (const TBlockColInfo& colInfo : tBlockInfo[0].columnInfos) {
      columnNames.insert(colInfo.name);
      resultTBlockInfo.columnInfos.push_back(colInfo);
   }
   for (const TBlockColInfo& colInfo : tBlockInfo[1].columnInfos) {
      if (!columnNames.insert(colInfo.name).second) {
         return listutils::typeError(
                 "Column name " + colInfo.name + " exists in both relations");
      }
      resultTBlockInfo.columnInfos.push_back(colInfo);
   }

   // compile information required by Value Mapping
   ListExpr appendInfo = nl->OneElemList(nl->Empty()); // will be omitted below
   ListExpr appendEnd = appendInfo;
   // ensure that the Value Mapping args will start at args[4]
   // even if parameters three and four were omitted by the caller
   for (unsigned i = argCount; i < STREAM_COUNT + STREAM_COUNT; ++i) {
      appendEnd = nl->Append(appendEnd, nl->IntAtom(0));
   }
   // append the actual information on the input streams
   for (unsigned i = 0; i < STREAM_COUNT; ++i) {
      appendEnd = nl->Append(appendEnd, nl->IntAtom(attrIndex[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(dim[i]));
      appendEnd = nl->Append(appendEnd, nl->BoolAtom(isTBlockStream[i]));
      appendEnd = nl->Append(appendEnd, nl->IntAtom(tBlockColumns[i]));
   }

   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            nl->Rest(appendInfo),
                            resultTBlockInfo.GetTypeExpr(true));
}

CRelAlgebra::TBlockTI CDACSpatialJoin::getTBlockTI(
        const ListExpr attributeList, uint64_t desiredBlockSize,
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

   auto localInfo = static_cast<LocalInfo*>(local.addr);

   switch(message) {
      case OPEN: {
         delete localInfo;
         // extract information about the first and second stream, creating
         // InputStream instances for either a tuple block stream or a mere
         // tuple stream, respectively
         InputStream* input[2];
         unsigned argIndex = 4;
         for (int i = 0; i < 2; ++i) {
            auto attrIndex = static_cast<unsigned>(
                    (static_cast<CcInt*>(args[argIndex++].addr))->GetValue());
            auto dim = static_cast<unsigned>(
                    (static_cast<CcInt*>(args[argIndex++].addr))->GetValue());
            auto isTBlockStream =
                    (static_cast<CcBool*>(args[argIndex++].addr))->GetValue();
            auto tBlockColumns = static_cast<ListExpr>(
                    (static_cast<CcInt*>(args[argIndex++].addr))->GetValue());
            if (isTBlockStream) {
               // input is stream of tuple blocks
               input[i] = new InputTBlockStream(args[i], attrIndex, dim);
            } else {
               // input is stream of tuples which will be inserted into tuple
               // blocks by this operator using the DEFAULT_BLOCK_SIZE
               ListExpr tBlockType = nl->TwoElemList(nl->SymbolAtom("tblock"),
                       nl->TwoElemList(nl->IntAtom(DEFAULT_BLOCK_SIZE),
                               tBlockColumns));
               // construct TBlockTI; information in tBlock type is not numeric
               CRelAlgebra::TBlockTI tBlockTI(tBlockType, false);
               input[i] = new InputTupleStream(args[i], attrIndex, dim,
                     tBlockTI.GetBlockInfo(), tBlockTI.GetDesiredBlockSize());
            }
         }
         local.addr = new LocalInfo(input[0], input[1], s);
         return 0;
      }

      case REQUEST: {
         result.addr = localInfo ? localInfo->getNext() : nullptr;
         return result.addr ? YIELD : CANCEL;
      }

      case CLOSE: {
         if (localInfo) {
            delete localInfo;
            local.addr = nullptr;
         }
         return 0;
      }

      default: {
         assert (false);
      }
   } // end switch

   return 0;
}

// ========================================================
/*
1.5 LocalInfo class

1.5.1 constructor

*/
LocalInfo::LocalInfo(InputStream* input1_, InputStream* input2_, Supplier s_) :
        input1(input1_),
        input2(input2_),
        s(s_),
        isFirstRequest(true),
        memLimit(qp->GetMemorySize(s) * 1024 * 1024),

        // Extract information about result tuple block type and size
        outTypeInfo(CRelAlgebra::TBlockTI(qp->GetType(s), false)),
        outTBlockInfo(outTypeInfo.GetBlockInfo()),
        outTBlockSize(outTypeInfo.GetDesiredBlockSize()
                    * CRelAlgebra::TBlockTI::blockSizeFactor),
        joinState(nullptr),
        joinStateCount(0) {
#ifdef CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
   cout << "sizeof(SortEdge) = " << sizeof(SortEdge) << endl;
   cout << "sizeof(JoinEdge) = " << sizeof(JoinEdge) << endl;
   cout << endl;
   CacheInfos::report(cout);
#endif
}

/*
1.5.2  destructor

*/
LocalInfo::~LocalInfo() {
   delete joinState;
   delete input1;
   delete input2;
}

/*
1.5.3 support functions

*/
void LocalInfo::requestInput() {
   while ((getUsedMem() < memLimit)
         && (!input1->isDone() || !input2->isDone())) {
      if (input1->isDone())
         input2->request();
      else if (input2->isDone())
         input1->request();
      else {
         // both streams are not done; choose the stream from which fewer
         // tuples have been read so far
         if (input1->getTupleCount() > input2->getTupleCount())
            input2->request();
         else
            input1->request();
      }
   }
}

size_t LocalInfo::getUsedMem() {
   size_t tupleSum = input1->getTupleCount() + input2->getTupleCount();

   // first, we estimate the memory required by the JoinState constructor
   // (of which the SortEdge and RectangleInfo part will be released on
   // completion of the constructor):
   size_t joinStateConstruction = tupleSum * (2 * sizeof(SortEdge) +
           sizeof(RectangleInfo) + 2 * sizeof(JoinEdge));

   // during JoinState execution, we must consider both JoinState::joinEdges
   // (2 * sizeof(JoinEdge)) and JoinState::mergedAreas (1 * sizeof(...)):
   // mergedAreas duplicate JoinEdges, but they are only constructed over time
   // (not all at once), and the number of JoinEdges stored here is being
   // reduced with every merge step, since MergedArea::complete only
   // stores *one* edge (rather than two edges) per rectangle. Short of
   // extreme cases (where all rectangles are completed only at the last
   // merge step), it seems adequate to assume 1 * sizeof(JoinEdge) for all
   // mergedAreas at a given time:
   size_t joinStateExecution = tupleSum * ((2 + 1) * sizeof(JoinEdge));

   // since JoinState construction and execution take place sequentially,
   // the maximum (rather than the sum) can be used:
   return  input1->getUsedMem() + input2->getUsedMem() +
         std::max(joinStateConstruction, joinStateExecution);
}

/*
1.5.4 getNext() function

*/
CRelAlgebra::TBlock* LocalInfo::getNext() {
   auto outTBlock = new CRelAlgebra::TBlock(outTBlockInfo, 0, 0);

   while (true) {
      // if a JoinState has been created, ...
      if (joinState) {
         // ... return the next block of join results
         if (joinState->nextTBlock(outTBlock))
            return outTBlock;

         delete joinState;
         joinState = nullptr;
      }

      // read (more) data
      if (isFirstRequest) {
         // first attempt to read from the streams
         isFirstRequest = false; // prevent this block from being entered twice

         // test if any of the streams is empty - then nothing to do
         if (!input1->request() || !input2->request()) {
            outTBlock->DecRef();
            return nullptr;
         }

         // read as much as possible from both input streams
         requestInput();

         // continue creating a JoinState below

      } else if (input1->isDone() && input2->isDone()) {
         // all input was read, join is complete
         assert (outTBlock->GetRowCount() == 0);
         outTBlock->DecRef();
         return nullptr;

      } else {
         // neither first request nor join complete,
         // i.e. the tuple data did not fit completely into the main memory;
         // the tuples read so far were treated, now read and treat more data

         if (input1->isFullyLoaded()) {
            // continue reading from input2
            input2->clearMem();
            do {
               input2->request();
            } while (!input2->isDone() && getUsedMem() < memLimit);
         } else if (!input1->isDone()) {
            // continue reading from input1
            input1->clearMem();
            do {
               input1->request();
            } while (!input1->isDone() && getUsedMem() < memLimit);
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

      // create a JoinState from the TBlocks that were read to the main memory
      if (input1->hasTBlocks() && input2->hasTBlocks()) {
         assert (!joinState);
         ++joinStateCount;
         joinState = new JoinState(input1, input2, outTBlockSize,
                 joinStateCount);
      }

   } // end of while loop
} // end of getNext() function
