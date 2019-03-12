/*
1 JoinEdge struct

*/

#pragma once

#include <memory>
#include <vector>

#include "Base.h"

namespace cdacspatialjoin {

struct JoinEdge {
private:
   static constexpr EdgeIndex_t IS_LEFT_MASK = 0x8000000;
   static constexpr EdgeIndex_t COUNTER_PART_MASK = IS_LEFT_MASK - 1;

public:
   double yMin;
   double yMax;

   uint32_t flagsAndCounterPart;
   SetRowBlock_t address;

   JoinEdge(const double yMin_, const double yMax_,
            const EdgeIndex_t counterPartEdgeIndex_, const bool isLeft_,
            const SetRowBlock_t address_) :
           yMin(yMin_),
           yMax(yMax_),
           flagsAndCounterPart(
                   (isLeft_ ? IS_LEFT_MASK : 0) | counterPartEdgeIndex_),
           address(address_) {

   }

   ~JoinEdge() = default;

   inline bool operator<(const JoinEdge& other) const {
      return (yMin < other.yMin);
   }

   inline bool getIsLeft() const {
      return (flagsAndCounterPart & IS_LEFT_MASK) == IS_LEFT_MASK;
   }

   inline EdgeIndex_t getCounterPartEdgeIndex() const {
      return flagsAndCounterPart & COUNTER_PART_MASK;
   }
};

typedef std::vector<JoinEdge> JoinEdgeVec;

} // end of namespace cdacspatialjoin