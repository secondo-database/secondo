/*
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

*/

#ifndef STRColumn_H
#define STRColumn_H

#include <iostream>
#include "nodeCol.h"
#include "BinaryTuple.h"


namespace CRelAlgebra {
    class TBlockEntry;
    class TBlock;
}

namespace STRColumn {

    void mergeSort(
            std::vector<mmrtreetouch::binaryTuple> &tuples,
            int64_t l,
            int64_t r,
            char direction
            );

    void merge(
            std::vector<mmrtreetouch::binaryTuple> &tuples,
            int64_t l,
            int64_t m,
            int64_t r,
            char direction
            );

    std::vector<std::vector <mmrtreetouch::binaryTuple> > splitInSlices(
            std::vector<mmrtreetouch::binaryTuple> tuples,
            int numOfItemsInBucket,
            int64_t vectorSize
            );

    std::vector<std::vector <mmrtreetouch::binaryTuple> > sortSecondDimension(
            std::vector<std::vector <mmrtreetouch::binaryTuple> > container,
            int numOfItemsInBucket,
            int64_t vectorSize
            );

    std::vector<mmrtreetouch::nodeCol* > packInBuckets(
            std::vector<std::vector <mmrtreetouch::binaryTuple> >
                    sortedSlicedList,
            int64_t sizeOfSortedList,
            int64_t vectorSize,
            int numOfItemsInBucket
            );

    std::string bucketInfo(std::vector<mmrtreetouch::nodeCol* > bucketVector);

    std::vector<mmrtreetouch::nodeCol *> createBuckets(
            std::vector<mmrtreetouch::binaryTuple> tuples,
            int numOfItemsInBucket,
            int64_t &remainingMem
    );

} /* namespace STRColumn */

#endif /* STRColumn_H */
