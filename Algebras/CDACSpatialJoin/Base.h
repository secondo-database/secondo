/*
1 Typedefs and constants for the CDACSpatialJoin operator

*/

#pragma once

#include <memory>

namespace cdacspatialjoin {

#define CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
// #define CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE

enum SET : char {
   A = 0,
   B = 1
};
constexpr unsigned SET_COUNT = 2;
constexpr SET SETS[] { SET::A, SET::B };
const static std::string SET_NAMES[] { "A", "B" };

typedef uint32_t EdgeIndex_t;
typedef uint32_t RectInfoIndex_t;
typedef uint16_t BlockIndex_t; // since blocks contain at least 1 MB,
                               // 65535 blocks would require 65 GB main memory,
                               // so uint16_t should be sufficient
typedef uint32_t RowIndex_t;   // this value may well exceed 65535


}