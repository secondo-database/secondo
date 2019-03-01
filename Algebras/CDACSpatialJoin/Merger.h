/*
1 Merger

This class merges two adjacent MergedArea instances into a new MergedArea.
The process is interrupted whenever the output TBlock is full and can be
resumed by calling merge() again.

To keep terminology clear,

  * A / B always refers to the two rectangle sets A / B,

  * 1 / 2 always refers to the two areas 1 / 2 to be merged (of which area1
    is the left neighbor of area2)

  * left / right (or L / R) always refers to the left / right edge of a
    rectangle

  * in reportPairs()

*/

#pragma once

#include <memory>
#include "MergedArea.h"

namespace cdacspatialjoin {

// define a callback function for appending a result tuple to the
// output TBlock (this saves us from including the whole JoinState.h)
typedef std::function<bool(const JoinEdge&, const JoinEdge&)> AppendToOutput;

class Merger {
   /* the number of different reportPairs() calls in the outer loop
    * (reportType) */
   static constexpr unsigned REPORT_TYPE_COUNT = 4;

   /* the number of different reportPairs() calls in the inner loop
    * (reportSubType) */
   static constexpr unsigned REPORT_SUB_TYPE_COUNT = 2;

   /* a rough sequence of tasks to be performed by the Merger */
   enum TASK { initialize, report, postProcess, done };


   /* the first area to be merged (left neighbor of area2) */
   MergedAreaPtr area1;

   /* the second area to be merged (right neighbor of area1) */
   MergedAreaPtr area2;

   /* true if this is the last merge operation called by a JoinState instance;
    * the post-processing can then be omitted */
   bool isLastMerge;

   /* the result area to be calculated by the Merger */
   MergedAreaPtr result;

   // -----------------------------------------------------
   // temporary vectors used for finding intersections and calculating
   // the result MergedArea

   /* the left edges of set A in area1 which span the complete x extent of
    * area2 (i.e. the corresponding right edge is outside area2 to the right) */
   std::vector<JoinEdge> leftASpan;

   /* the right edges of set A in area2 which span the complete x extent of
    * area1 (i.e. the corresponding left edge is outside area1 to the left) */
   std::vector<JoinEdge> rightASpan;

   /* the rectangles of set A which have a left edge in area1 and a right edge
    * in area2 */
   std::vector<JoinEdge> leftRightA;


   /* the left edges of set B in area1 which span the complete x extent of
    * area2 (i.e. the corresponding right edge is outside area2 to the right) */
   std::vector<JoinEdge> leftBSpan;

   /* the right edges of set B in area2 which span the complete x extent of
    * area1 (i.e. the corresponding left edge is outside area1 to the left) */
   std::vector<JoinEdge> rightBSpan;

   /* the rectangles of set B which have a left edge in area1 and a right edge
    * in area2 */
   std::vector<JoinEdge> leftRightB;

   // -----------------------------------------------------
   // control variables used to interrupt the process whenever the outTBlock
   // is full and to resume it when merge() is called

   /* the task currently performed */
   TASK currentTask;

   /* the type of reportPairs() call currently performed in the outer loop */
   unsigned reportType = 0;

   /* the subtype of reportPairs() call currently performed in the inner loop */
   unsigned reportSubType = 0;

   /* the current start index of vector edgesR */
   size_t indexRBegin = 0;

   /* the current index of vector edgesR */
   size_t indexR = 0;

   /* the current start index of vector edgesS */
   size_t indexSBegin = 0;

   /* the current index of vector S (starting from indexSBegin, indexS is being
    * incremented until edgeS.yMin exceeds edgeR.yMax ) */
   size_t indexS = 0;

public:
   /* constructor expects two adjacent areas which shall be merged to a single
    * area. Note that the input areas must fulfil the invariants listed in the
    * MergedArea.h comment */
   Merger(MergedAreaPtr& area1_, MergedAreaPtr& area2_, bool isLastMerge_);

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
   void removeCompleteRectangles(const std::vector<JoinEdge>& left1,
           const std::vector<JoinEdge>& right2,
           std::vector<JoinEdge>& leftSpan,
           std::vector<JoinEdge>& rightSpan,
           std::vector<JoinEdge>& leftRight);

   /* resets the indices indexR, indexSBegin, and indexS used in
    * reportPairsSub() to 0. Must be called at the beginning of a new
    * report subtype. */
   void resetReportIndices();

   /* reports rectangle intersections between
    * a) an edge in the "span" vector (from one area and set), and
    * b) an edge in either the "left" or the "complete" vector (which must
    * be from the other area and set than the edges in "span").
    * Returns true if completed, or false if outTBlock is full */
   bool reportPairs(const std::vector<JoinEdge>& span,
                    const std::vector<JoinEdge>& left,
                    const std::vector<JoinEdge>& complete,
                    const AppendToOutput* output);

   /* reports rectangle intersections between
    * a) an edge in the "edgesR" vector (from one area and set), and
    * b) an edge in the "edgesS" vector (from the other area and set).
    * returns true if completed, or false if outTBlock is full */
   bool reportPairsSub(const std::vector<JoinEdge>& edgesR,
                       const std::vector<JoinEdge>& edgesS,
                       const AppendToOutput* output);

   /* merges the given source vectors "source1" and "source2" (starting from
    * the given indices) into the destination vector "dest", using the sort
    * order determined by the JoinEdge operator<. Source vectors must already
    * be sorted in this order. */
   static void merge(const std::vector<JoinEdge>& source1, size_t startIndex1,
           const std::vector<JoinEdge>& source2, size_t startIndex2,
           std::vector<JoinEdge>& dest);

   /* merges the given source vectors "source1/2/3" into the destination vector
    * "dest", using the sort order determined by the JoinEdge operator<.
    * Source vectors must already be sorted in this order. */
   static void merge(const std::vector<JoinEdge>& source1,
              const std::vector<JoinEdge>& source2,
              const std::vector<JoinEdge>& source3,
              std::vector<JoinEdge>& dest);
};

} // end of namespace cdacspatialjoin