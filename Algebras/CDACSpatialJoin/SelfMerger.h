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


1 includes

*/

#pragma once

#include <ostream>

#include "SelfMergedArea.h" // -> ... -> <memory>
#include "MergerStats.h"
#include "IOData.h"

namespace cdacspatialjoin {

/*
2 SelfMerger class

This class is used within a self join to merge two adjacent SelfMergedArea
instances into a new SelfMergedArea. The process is interrupted whenever the
output TBlock is full and can be resumed by calling merge() again.

To keep terminology clear,

  * 1 / 2 always refers to the two areas 1 / 2 to be merged
    (area 1 being the left neighbor of area 2);

  * left / right (or L / R) always refers to the left / right edge of a
    single rectangle;

  * S / T is used for two different JoinEdge vectors in reportPairsSub():

    * regarding areas, either (S=1 and T=2) or (S=2 and T=1);

    * regarding edges, either (S=left and T=right) or (S=right and T=left);

*/
class SelfMerger {
public:
#ifdef CDAC_SPATIAL_JOIN_METRICS
   /* statistics on the usage of SelfMerger functions and the
    * iteration frequency of the while loops in SelfMerger::reportPairsSub().
    * Using a static instance is not a "pretty" solution but saves us from
    * injecting a MergerStats instance into millions of SelfMerger instances */
   static std::unique_ptr<MergerStats> stats;
#endif

private:
   /* a rough sequence of tasks to be performed by the SelfMerger */
   enum TASK { initialize, report, postProcess, done };

   // -----------------------------------------------------
   // data passed to the constructor

   /* the first area to be merged (left neighbor of area2) */
   const SelfMergedAreaPtr area1;

   /* the second area to be merged (right neighbor of area1) */
   const SelfMergedAreaPtr area2;

   /* true if this is the last merge operation called by a JoinState instance;
    * the post-processing can then be omitted */
   const bool isLastMerge;

   /* ioData holds all current input data and provides a function to add a
    * result tuple to the output tuple block */
   IOData* const ioData;

   /* the result area to be calculated by the SelfMerger */
   SelfMergedAreaPtr result;

   // -----------------------------------------------------
   // temporary vectors used for finding intersections and calculating
   // the result SelfMergedArea

   /* the left edges in area1 which span the complete x extent of
    * area2 (i.e. the corresponding right edge is outside area2 to the right) */
   JoinEdgeVec leftSpan;

   /* the right edges in area2 which span the complete x extent of
    * area1 (i.e. the corresponding left edge is outside area1 to the left) */
   JoinEdgeVec rightSpan;

   /* the rectangles which have a left edge in area1 and a right edge
    * in area2 */
   JoinEdgeVec leftRight;


   // -----------------------------------------------------
   // control variables used to interrupt the process whenever the outTBlock
   // is full and to resume it when merge() is called

   /* the task currently performed */
   TASK currentTask;

   /* the type of reportPairs() call currently performed in the outer loop */
   unsigned reportType;

   /* the subtype of reportPairs() call currently performed in the inner
    * loop */
   unsigned reportSubType;

   /* the current start index of vector edgesS */
   size_t indexSBegin;

   /* the current index of vector edgesS */
   size_t indexS;

   /* the current start index of vector edgesT */
   size_t indexTBegin;

   /* the current index of vector T (starting from indexTBegin, indexT is being
    * incremented until edgeT.yMin exceeds edgeS.yMax ) */
   size_t indexT;

public:
#ifdef CDAC_SPATIAL_JOIN_METRICS
   static void resetLoopStats();
#endif

   /* constructor expects two adjacent areas which shall be merged to a single
    * area. Note that the input areas must fulfil the invariants listed in the
    * SelfMergedArea.h comment */
   SelfMerger(SelfMergedAreaPtr area1_, SelfMergedAreaPtr area2_,
          bool isLastMerge_, IOData* ioData_);

   /* destructor */
   ~SelfMerger();

   /* starts or continues merging the areas given in the constructor;
    * returns false if the outTBlock is full and merge needs to be resumed
    * later (by calling this function again with a new outTBlock),
    * or true if merge was completed and the result can be obtained by
    * calling getResult() */
   bool merge();

   /* returns the resulting SelfMergedArea. Must only be called after merge()
    * was completed (i.e. merge() returned true) */
   inline SelfMergedAreaPtr getResult() const {
      // assert (currentTask == TASK::done);
      return result;
   }

private:
   /* reports rectangle intersections between
    * a) an edge in the "span" vector (from one area), and
    * b) an edge in either the "left" or the "complete" vector (from the
    * other area).
    * Returns true if completed, or false if the output TBlock is full */
   bool reportPairs(const JoinEdgeVec& span,
                    const JoinEdgeVec& left,
                    const JoinEdgeVec& complete);

   /* reports rectangle intersections between
    * a) an edge in the "edgesS" vector (from one area), and
    * b) an edge in the "edgesT" vector (from the other area).
    * returns true if completed, or false if the output TBlock is full */
   bool reportPairsSub(const JoinEdgeVec& edgesS,
                       const JoinEdgeVec& edgesT);

   /* specialized version of reportPairsSub() for edgesS containing only 1
    * edge, but edgesT containing multiple edges */
   bool reportPairsSub1(const JoinEdge& edgeS,
                        const JoinEdgeVec& edgesT, size_t sizeT);

   /* specialized version of reportPairsSub() for both edgesS and edgesT
    * containing only 1 edge */
   inline bool reportPairsSub11(const JoinEdge& edgeS,
                                const JoinEdge& edgeT);
   // this function is only called from one point in reportPairsSub(),
   // requires just 82 bytes assembly code, but is used very often, so it
   // deserves to be "inline"

public:
   /* returns the total number of JoinEdge instances stored in this SelfMerger,
    * optionally including the input SelfMergedAreas and the result
    * SelfMergedArea */
   size_t getJoinEdgeCount(bool includeAreas) const;

   /* returns the number of bytes currently used by this SelfMerger, including
    * the input SelfMergedAreas and the result SelfMergedArea */
   size_t getUsedMemory() const;

};

} // end of namespace cdacspatialjoin