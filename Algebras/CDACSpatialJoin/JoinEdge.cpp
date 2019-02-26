/*
1 JoinEdge struct

*/

#include <string>
#include <iostream>
#include <sstream>

#include "JoinEdge.h"
#include "Base.h"

using namespace cdacspatialjoin;
using namespace std;

JoinEdge::JoinEdge(const double yMin_, const double yMax_,
                   const EdgeIndex_t counterPartEdgeIndex_,
                   const bool isLeft_, const SET set_,
                   const BlockIndex_t block_, const RowIndex_t row_) :
                   yMin(yMin_),
                   yMax(yMax_),
                   counterPartEdgeIndex(counterPartEdgeIndex_),
                   isLeft(isLeft_),
                   set(set_),
                   block(block_),
                   row(row_) {
}

std::string JoinEdge::toString() const {
   stringstream st;
   st << "y = [" << yMin << "; " << yMax << "]; ";
   st << (isLeft ? "left" : "right") << " edge ";
   st << "from set " << SET_NAMES[set] << ", ";
   st << "block " << block << ", row " << row;
   return st.str();
}
