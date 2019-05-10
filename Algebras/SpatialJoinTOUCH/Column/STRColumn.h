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
            mmrtreetouch::binaryTuple * arr[],
            int l,
            int r,
            char direction,
            int leftAttrIndex);

    void merge(
            mmrtreetouch::binaryTuple * arr[],
            int l,
            int m,
            int r,
            char direction,
            int leftAttrIndex
            );

    std::vector<std::vector <mmrtreetouch::binaryTuple *> > splitInSlices(
            mmrtreetouch::binaryTuple * arr[],
            int numOfPartitions,
            int array_size);

    std::vector<std::vector <mmrtreetouch::binaryTuple *> > sortSecondDimension(
            std::vector<std::vector <mmrtreetouch::binaryTuple *> > container,
            int leftAttrIndex,
            int numOfPartitions
            );

    std::vector<mmrtreetouch::nodeCol* > packInBuckets(
            std::vector<std::vector <mmrtreetouch::binaryTuple *> >
                    sortedSlicedList,
            int sizeOfSortedList,
            int arr_size,
            int numOfPartitions,
            int leftAttrIndex);

    std::string bucketInfo(std::vector<mmrtreetouch::nodeCol* > bucketVector);

    void createArrayFromTupleVector(
            mmrtreetouch::binaryTuple * arr[],
            std::vector<mmrtreetouch::binaryTuple *> tuples);

    std::vector<mmrtreetouch::nodeCol *> createBuckets(
            std::vector<mmrtreetouch::binaryTuple *> tuples,
            int _firstStreamWordIndex,
            int _numOfPartitions
    );

} /* namespace STRColumn */

#endif /* STRColumn_H */
