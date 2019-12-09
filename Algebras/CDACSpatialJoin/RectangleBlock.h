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


1 RectangleBlock class

Reserves a given amount of memory and fills it with the coordinates of 2- or
3-dimensional rectangles. Can be used to efficiently store the bounding boxes
of spatial attributes.

This class is used for the CDACSpatialJoinCount operator; to increase spatial
locality, the x and y coordinates are stored separately from the z coordinates
(and those are only stored for 3-dimensional rectangles, but obviously omitted
in the 2-dimensional case).

*/

#pragma once

#include "Base.h" // -> <memory>, <vector>
#include "Algebras/Rectangle/RectangleAlgebra.h"

namespace cdacspatialjoin {

/*
1.1 PlainRect2 struct

Represents the coordinates a 2-dimensional rectangle, but without the overhead
of a Rectangle<2> (i.e. without the IsDefined attribute).

*/
struct PlainRect2 {
   double xMin;
   double xMax;
   double yMin;
   double yMax;

   PlainRect2(double xMin_, double xMax_, double yMin_, double yMax_) {
      xMin = xMin_;
      xMax = xMax_;
      yMin = yMin_;
      yMax = yMax_;
   }

   ~PlainRect2() = default;
};

/*
1.2 PlainInterval struct

Represents an interval, especially of z coordinates [zMin .. zMax].

*/
struct PlainInterval {
   double zMin;
   double zMax;

   PlainInterval(double zMin_, double zMax_) {
      zMin = zMin_;
      zMax = zMax_;
   }

   ~PlainInterval() = default;
};

/*
1.3 RectangleBlock class

*/
class RectangleBlock {
   /* the dimension (2 or 3) of the rectangles stored in this RectangleBlock */
   const unsigned dim;

   /* the number of rectangles that can be stored in this instance,
    * depending on the "sizeInBytes" given in the constructor */
   const size_t capacity;

   /* the number of rectangles currently stored in this instance */
   size_t count;

   /* sequentially stores xMin, xMax, yMin, yMax for each rectangle; for
    * 3-dimensional rectangles, the z coordinates are stored separately in
    * coordsZ to increase locality for usage in CDACSpatialJoinCount */
   std::vector<PlainRect2> coordsXY;

   /* sequentially stores zMin, zMax for each rectangle if dim == 3;
    * otherwise, coordsZ is unused */
   std::vector<PlainInterval> coordsZ;

   /* the bounding box of all rectangles stored in this RectangleBlock */
   double bboxXMin, bboxXMax;
   double bboxYMin, bboxYMax;
   double bboxZMin, bboxZMax;

public:
   /* returns the memory (in bytes) required for the given number of rectangles
    * with the given dimension (2 or 3), but without the overhead of
    * sizeof(RectangleBlock) */
   static size_t getRequiredMemory(unsigned dim, size_t rectangleCount);

   /* constructor for a new RectangleBlock of the given size (in bytes) and
    * the given dimension (2 or 3) of rectangles */
   RectangleBlock(unsigned dim_, size_t sizeInBytes);

   /* destructor */
   ~RectangleBlock() = default;

   /* adds the given 2-dimensional rectangle to the block. Must only be used
    * if dim == 2 was provided in the constructor */
   void add(const Rectangle<2>& rectangle);

   /* adds the given 3-dimensional rectangle to the block. Must only be used
    * if dim == 3 was provided in the constructor */
   void add(const Rectangle<3>& rectangle);

   /* returns true if the reserved memory is full and no further rectangles
    * can be added */
   inline bool isFull() const { return (count >= capacity); };

   /* returns the number of rectangles currently stored in this instance */
   inline size_t getRectangleCount() const { return count; }

   /* returns the memory (in bytes) used by the rectangles which are currently
    * stored in this RectangleBlock (excluding the overhead of
    * sizeof(RectangleBlock) and reserved memory) */
   size_t getUsedMemory() { return getRequiredMemory(dim, count); }

   /* returns the vector of x and y coordinates which stores xMin, xMax, yMin,
    * yMax for each rectangle */
   const std::vector<PlainRect2>& getCoordsXY() const { return coordsXY; }

   /* returns the vector of z coordinates which stores zMin, zMax for each
    * rectangle */
   const std::vector<PlainInterval>& getCoordsZ() const { return coordsZ; }

   /* returns the Rectangle<2> stored at the given position. This method can
    * be used for occasional access but is not optimized for bulk access */
   Rectangle<2> getRectangle2D(RowIndex_t row) const;

   /* returns the Rectangle<3> stored at the given position. This method can
    * be used for occasional access but is not optimized for bulk access */
   Rectangle<3> getRectangle3D(RowIndex_t row) const;

   /* creates a new StandardSpatialAttribute instance from the Rectangle<2>
    * or Rectangle<3> stored at the given position. */
   Attribute* getRectangleAttr(RowIndex_t row) const;

   /* returns the 3-dimensional bounding box that contains all rectangles
    * stored in this RectangleBlock. If this instance was constructed for
    * 2-dimensional rectangles, the z interval of the bounding box is the
    * maximum possible double range */
   Rectangle<3> getBbox() const;

private:
   /* adds the given x and y coordinates */
   inline void add(double xMin, double xMax, double yMin, double yMax) {
      // add X and Y coordinates
      coordsXY.emplace_back(xMin, xMax, yMin, yMax);

      // update bounding box
      if (xMin < bboxXMin)
         bboxXMin = xMin;
      if (xMax > bboxXMax)
         bboxXMax = xMax;
      if (yMin < bboxYMin)
         bboxYMin = yMin;
      if (yMax > bboxYMax)
         bboxYMax = yMax;

      ++count;
   }
};

} // end of namespace cdacspatialjoin