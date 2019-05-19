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

#ifndef STR_H
#define STR_H

#include <iostream>
#include "tchNode.h"


namespace STR {

    void mergeSort(
            std::vector<Tuple *> &tuples,
            int64_t l,
            int64_t r,
            char direction,
            int64_t leftAttrIndex
            );

    void merge(
            std::vector<Tuple *> &tuples,
            int64_t l,
            int64_t m,
            int64_t r,
            char direction,
            int64_t leftAttrIndex
            );

    std::vector<std::vector <Tuple *> > splitInSlices(
            std::vector<Tuple *> tuples,
            int numOfPartitions,
            int64_t array_size);

    std::vector<std::vector <Tuple *> > sortSecondDimension(
            std::vector<std::vector <Tuple *> > container,
            int leftAttrIndex,
            int numOfPartitions,
            int64_t array_size
            );

    std::vector<mmrtreetouch::tchNode* > packInBuckets(
            std::vector<std::vector <Tuple *> > sortedSlicedList,
            int64_t sizeOfSortedList,
            int64_t arr_size,
            int numOfPartitions,
            int leftAttrIndex);

    std::string bucketInfo(std::vector<mmrtreetouch::tchNode* > bucketVector);

    std::vector<mmrtreetouch::tchNode *> createBuckets(
            std::vector<Tuple*> tuples,
            int firstStreamWordIndex,
            int _numOfPartitions
            );

} /* namespace STR */

#endif /* STR_H */
