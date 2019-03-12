/*
1 Merger

This class merges two adjacent MergedArea instances into a new MergedArea.
The process is interrupted whenever the output TBlock is full and can be
resumed by calling merge() again.

To keep terminology clear,

  * A / B always refers to the two rectangle sets A / B to be joined;

  * 1 / 2 always refers to the two areas 1 / 2 to be merged
    (area 1 being the left neighbor of area 2);

  * left / right (or L / R) always refers to the left / right edge of a
    single rectangle;

  * S / T is used for two different JoinEdge vectors in reportPairsSub():

    * regarding sets, either (S=A and T=B) or (S=B and T=A);

    * regarding areas, either (S=1 and T=2) or (S=2 and T=1);

    * regarding edges, either (S=left and T=right) or (S=right and T=left);

*/

#pragma once

#include <memory>
#include <iostream>
#include <ostream>

#include "MergedArea.h"

namespace cdacspatialjoin {

// define a callback function for appending a result tuple to the
// output TBlock (this saves us from including the whole JoinState.h)
typedef std::function<bool(const JoinEdge&, const JoinEdge&)> AppendToOutput;

class Merger {
#ifdef CDAC_SPATIAL_JOIN_METRICS
   // statistical

   /* the number of times reportPairs() was called */
   static uint64_t reportPairsCount;

   /* the number of times reportPairsSub() was called (excluding special cases
    * with only 1 edge in one of the sets) */
   static uint64_t reportPairsSubCount;

   /* the number of times reportPairsSub() was called with only 1 edge in
    * either (but not both) of the sets */
   static uint64_t reportPairsSub1Count;

   /* the number of times reportPairsSub() was called with only 1 edge in
    * both sets */
   static uint64_t reportPairsSub11Count;

   /* size of loopStats array (see below). 32 allows for while loops up to
    * 2^32 - 1 = approx. 4 billion cycles */
   static constexpr unsigned LOOP_STATS_COUNT = 32;

   /* statistics on the while loops in Merger::reportPairsSub():
    * for each while loop with n cycles, the entry loopStats[lb n] is
    * incremented by 1. For instance, loopStats[5] holds the number of while
    * loops that had 16-31 (i.e. between 2^4 and 2^5 - 1) cycles */
   static uint64_t loopStats[LOOP_STATS_COUNT];
#endif

   /* a rough sequence of tasks to be performed by the Merger */
   enum TASK { initialize, report, postProcess, done };

   // -----------------------------------------------------
   // data passed to the constructor

   /* the first area to be merged (left neighbor of area2) */
   const MergedAreaPtr area1;

   /* the second area to be merged (right neighbor of area1) */
   const MergedAreaPtr area2;

   /* true if this is the last merge operation called by a JoinState instance;
    * the post-processing can then be omitted */
   const bool isLastMerge;

   /* the result area to be calculated by the Merger */
   MergedAreaPtr result;

   // -----------------------------------------------------
   // temporary vectors used for finding intersections and calculating
   // the result MergedArea

   /* the left edges of set A in area1 which span the complete x extent of
    * area2 (i.e. the corresponding right edge is outside area2 to the right) */
   JoinEdgeVec leftASpan;

   /* the right edges of set A in area2 which span the complete x extent of
    * area1 (i.e. the corresponding left edge is outside area1 to the left) */
   JoinEdgeVec rightASpan;

   /* the rectangles of set A which have a left edge in area1 and a right edge
    * in area2 */
   JoinEdgeVec leftRightA;


   /* the left edges of set B in area1 which span the complete x extent of
    * area2 (i.e. the corresponding right edge is outside area2 to the right) */
   JoinEdgeVec leftBSpan;

   /* the right edges of set B in area2 which span the complete x extent of
    * area1 (i.e. the corresponding left edge is outside area1 to the left) */
   JoinEdgeVec rightBSpan;

   /* the rectangles of set B which have a left edge in area1 and a right edge
    * in area2 */
   JoinEdgeVec leftRightB;

   // -----------------------------------------------------
   // control variables used to interrupt the process whenever the outTBlock
   // is full and to resume it when merge() is called

   /* the task currently performed */
   TASK currentTask;

   /* the type of reportPairs() call currently performed in the outer loop */
   unsigned reportType = 0;

   /* the subtype of reportPairs() call currently performed in the inner loop */
   unsigned reportSubType = 0;

   /* the current start index of vector edgesS */
   size_t indexSBegin = 0;

   /* the current index of vector edgesS */
   size_t indexS = 0;

   /* the current start index of vector edgesT */
   size_t indexTBegin = 0;

   /* the current index of vector T (starting from indexTBegin, indexT is being
    * incremented until edgeT.yMin exceeds edgeS.yMax ) */
   size_t indexT = 0;

public:
   /* constructor expects two adjacent areas which shall be merged to a single
    * area. Note that the input areas must fulfil the invariants listed in the
    * MergedArea.h comment */
   Merger(MergedAreaPtr& area1_, MergedAreaPtr& area2_,
           bool isLastMerge_);

   /* destructor */
   ~Merger() = default;

   /* starts or continues merging the areas given in the constructor;
    * returns false if the outTBlock is full and merge needs to be resumed
    * later (by calling this function again with a new outTBlock),
    * or true if merge was completed and the result can be obtained by
    * calling getResult() */
   bool merge(const AppendToOutput* output);

   /* returns the resulting MergedArea. Must only be called after merge() was
    * completed (i.e. merge() returned true) */
   MergedAreaPtr getResult() const;

private:
   /* for all edges in the input vectors left1 and right2,
    * a) the newly completed rectangles are determined (i.e. rectangles with
    * their left edge in area1 and their right edge in area2) and stored in
    * the output vector leftRight; all other edges are either stored in
    * b) leftSpan (left edges from area1 which span the complete x range of
    * area2), or
    * c) in rightSpan (right edges from area2 which span the complete x range
    * of area1). */
   void removeCompleteRectangles(const JoinEdgeVec& left1,
                                 const JoinEdgeVec& right2,
                                 JoinEdgeVec& leftSpan,
                                 JoinEdgeVec& rightSpan,
                                 JoinEdgeVec& leftRight);

   /* reports rectangle intersections between
    * a) an edge in the "span" vector (from one area and set), and
    * b) an edge in either the "left" or the "complete" vector (both from the
    * other area and set).
    * Returns true if completed, or false if the output TBlock is full */
   bool reportPairs(const JoinEdgeVec& span,
                    const JoinEdgeVec& left,
                    const JoinEdgeVec& complete,
                    const AppendToOutput* output);

   /* reports rectangle intersections between
    * a) an edge in the "edgesS" vector (from one area and set), and
    * b) an edge in the "edgesT" vector (from the other area and set).
    * returns true if completed, or false if the output TBlock is full */
   bool reportPairsSub(const JoinEdgeVec& edgesS,
                       const JoinEdgeVec& edgesT,
                       const AppendToOutput* output);

   /* specialized version of reportPairsSub() for edgesS containing only 1
    * edge, but edgesT containing multiple edges */
   bool reportPairsSub1(const JoinEdge& edgeS,
                       const JoinEdgeVec& edgesT,
                       const AppendToOutput* output);

   /* specialized version of reportPairsSub() for both edgesS and edgesT
    * containing only 1 edge */
   bool reportPairsSub11(const JoinEdge& edgeS,
                         const JoinEdge& edgeT,
                         const AppendToOutput* output);

   /* merges the given source vectors "source1" and "source2" (starting from
    * the given indices) into the destination vector "dest", using the sort
    * order determined by the JoinEdge operator<. Source vectors must already
    * be sorted in this order. */
   static void merge(const JoinEdgeVec& source1, size_t startIndex1,
                     const JoinEdgeVec& source2, size_t startIndex2,
                     JoinEdgeVec& dest);

   /* merges the given source vectors "source1/2/3" into the destination vector
    * "dest", using the sort order determined by the JoinEdge operator<.
    * Source vectors must already be sorted in this order. */
   static void merge(const JoinEdgeVec& source1,
                     const JoinEdgeVec& source2,
                     const JoinEdgeVec& source3,
                     JoinEdgeVec& dest);

#ifdef CDAC_SPATIAL_JOIN_METRICS
   /* updates the loop statistics, adding a loop with the given count of
    * cycles */
   static void addToLoopStats(size_t cycleCount);

public:
   /* resets the loop statistics to zero */
   static void resetLoopStats();

   /* reports the loop statistics (while loops in Merger::reportPairsSub())
    * to the given out stream */
   static void reportLoopStats(std::ostream& out);
#endif
};

} // end of namespace cdacspatialjoin