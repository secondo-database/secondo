/*
1 Typedefs and constants for the CDACSpatialJoin operator

*/

#pragma once

#include <memory>

namespace cdacspatialjoin {

#define CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
// #define CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
// #define CDAC_SPATIAL_JOIN_METRICS

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

}