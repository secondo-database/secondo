/*
1 RectangleBlock class

*/

#include <assert.h>

#include "RectangleBlock.h"

using namespace cdacspatialjoin;
using namespace std;

size_t RectangleBlock::getRequiredMemory(unsigned dim, size_t rectangleCount) {
   const size_t entrySize = sizeof(PlainRect2) +
                            (dim - 2) * sizeof(PlainInterval);
   return rectangleCount * entrySize;
}

RectangleBlock::RectangleBlock(const unsigned dim_, const size_t sizeInBytes) :
      dim(dim_),
      capacity((sizeInBytes - sizeof(RectangleBlock)) /
               (2 * dim_ * sizeof(double))),
      count(0) {

   assert(dim == 2 || dim == 3);
   assert(capacity > 0);

   // reserve space in the coords vectors to prevent resizing of vectors
   coordsXY.reserve(4 * capacity); // xMin, xMax, yMin, yMax for each rectangle
   if (dim > 2)
      coordsZ.reserve(2 * (dim - 2) * capacity); // zMin, zMax (if applicable)

   // initialize the bounding box
   const double maxDouble = numeric_limits<double>::max();
   bboxXMin = bboxYMin = bboxZMin = maxDouble;
   bboxXMax = bboxYMax = bboxZMax = -maxDouble;

   // if only 2-dimensional rectangles are considered here, use the maximum
   // possible interval for bboxZMin..ZMax (so, in case the other InputStream
   // contains 3-dimensional data, no rectangles will be excluded due to their
   // z coordinates)
   if (dim == 2) {
      bboxZMin = -maxDouble;
      bboxZMax = maxDouble;
   }
}

void RectangleBlock::add(const Rectangle<2>& rectangle) {
   assert (dim == 2);

   // add X and Y coordinates
   add(rectangle.MinD(0), rectangle.MaxD(0),
       rectangle.MinD(1), rectangle.MaxD(1));
   // bboxZMin..bboxZMax were set in the constructor to be the maximum
   // possible interval
}

void RectangleBlock::add(const Rectangle<3>& rectangle) {
   assert (dim == 3);

   // add X and Y coordinates
   add(rectangle.MinD(0), rectangle.MaxD(0),
       rectangle.MinD(1), rectangle.MaxD(1));

   // add Z coordinates
   const double zMin = rectangle.MinD(2);
   const double zMax = rectangle.MaxD(2);
   coordsZ.emplace_back(zMin, zMax);

   // update bounding box
   if (zMin < bboxZMin)
      bboxZMin = zMin;
   if (zMax > bboxZMax)
      bboxZMax = zMax;
}

Rectangle<2> RectangleBlock::getRectangle2D(const RowIndex_t row) const {
   if (row >= count)
      return Rectangle<2>(false);

   // read x and y coordinates from the coordsXY vector
   const PlainRect2& rect2 = coordsXY[row];

   // construct the result Rectangle<2>
   const double min[] { rect2.xMin, rect2.yMin };
   const double max[] { rect2.xMax, rect2.yMax };
   return Rectangle<2>(true, min, max);
}

Rectangle<3> RectangleBlock::getRectangle3D(const RowIndex_t row) const {
   assert (dim == 3);

   if (row >= count)
      return Rectangle<3>(false);

   // read x and y coordinates from the coordsXY vector
   const PlainRect2& rect2 = coordsXY[row];

   // read z coordinates from the coordsZ vector
   const PlainInterval& intervalZ = coordsZ[row];

   // construct the result Rectangle<3>
   const double min[] { rect2.xMin, rect2.yMin, intervalZ.zMin };
   const double max[] { rect2.xMax, rect2.yMax, intervalZ.zMax };
   return Rectangle<3>(true, min, max);
}

Rectangle<3> RectangleBlock::getBbox() const {
   const bool defined = (count > 0);
   const double min[] { bboxXMin, bboxYMin, bboxZMin };
   const double max[] { bboxXMax, bboxYMax, bboxZMax };
   return Rectangle<3>(defined, min, max);
}
