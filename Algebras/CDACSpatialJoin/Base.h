/*
1 Typedefs and constants for the CDACSpatialJoin(Count) operators

To keep includes as limited as possible, this header contains common typedefs
and constants used in the context of the CDACSpatialJoin(Count) operators.

*/

#pragma once

#define CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
// #define CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
// #define CDAC_SPATIAL_JOIN_METRICS

#include <memory>
#include <vector>

namespace cdacspatialjoin {

/* an enumeration for the rectangle sets to be joined */
enum SET : char {
   A = 0,
   B = 1
};

/* the number of rectangle sets to be joined (always 2) */
static constexpr unsigned SET_COUNT = 2;

/* an array containing SET::A and SET::B for range loops */
static constexpr SET SETS[] { SET::A, SET::B };

/* the names of the two rectangle sets used in console output */
static const std::string SET_NAMES[] { "A", "B" };

/* the integer type used for indices in vectors of SortEdges or JoinEdges */
typedef uint32_t EdgeIndex_t;

/* the integer type used for indices in the RectangleInfo vector in the
 * JoinState constructor */
typedef uint32_t RectInfoIndex_t;

/* the integer type used for indices of the TBlocks of one of the input
 * streams. Since blocks contain at least 1 MB, 65535 blocks would require
 * 65 GB main memory, so uint16_t should be sufficient */
typedef uint16_t BlockIndex_t;

/* the integer type used for indices of rows inside a TBlock */
typedef uint32_t RowIndex_t;  // this value may well exceed 65535

/* the integer type used to store the full "address" of a rectangle: the
 * set (= input stream A / B), the index of the TBlock and the row inside
 * the TBlock where it originates from */
typedef uint32_t SetRowBlock_t;

/* short for "CDACSpatialJoinTask". Lists the different tasks to be performed
 * (each multiple times) during a CDACSpatialJoin(Count) operation */
enum CDSjTask : unsigned {
   /* the task of requesting data from the InputStreams */
   requestData,
   /* the task of creating a JoinState instance */
   createJoinState,
   /* the task of creating a vector of SortEdge instances */
   createSortEdges,
   /* the task of sorting the vector of SortEdge instances */
   sortSortEdges,
   /* the task of creating a vector of JoinEdge instances */
   createJoinEdges,
   /* the task of merging the JoinEdges and reporting (or counting) the
    * intersections */
   merge
};

/* a vector of task names that correspond to the elements of the CDSjTask
 * enumeration */
static const std::vector<std::string> CDSJ_TASK_NAMES {{
        "requestData",
        "createJoinState",
        "createSortEdges",
        "sortSortEdges",
        "createJoinEdges",
        "merge"
} };

/* lists the different tasks to be performed (each multiple times) during a
 * Cache Test operation */
enum CacheTestTask : unsigned {
   fullTest,
   loopTest
};

/* the number of items in the CacheTestTask enumeration */
static constexpr unsigned CACHE_TEST_TASK_COUNT = 2;

}