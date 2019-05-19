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

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "STR.h"

#include <string>
#include <iostream>
#include <vector>
#include "tchNode.h"


using namespace std;
using namespace mmrtreetouch;

namespace STR {

    vector<vector <Tuple *> > splitInSlices(
            vector<Tuple *> tuples,
            int numOfItemsInBucket,
            int64_t array_size)
    {
        int64_t numOfPartitions = ceil((double)array_size/numOfItemsInBucket);

        int64_t counter = 0;

        vector<Tuple *> temp;
        vector<vector <Tuple *> > container;
        temp.reserve(numOfItemsInBucket);
        container.reserve(numOfPartitions);

        for( int64_t i = 0; i < array_size; i++ ) {
            counter++;

            temp.push_back(tuples[i]);

            if (counter % numOfItemsInBucket == 0) {
                container.push_back(temp);
                temp.clear();
            }
        }

        if (!temp.empty()) {
            container.push_back(temp);
        }

        return container;
    }

    vector<vector <Tuple *> > sortSecondDimension(
            vector<vector <Tuple *> > container,
            int leftAttrIndex,
            int numOfItemsInBucket,
            int64_t array_size
    )
    {
        int64_t numOfPartitions = ceil((double)array_size/numOfItemsInBucket);

        vector<vector <Tuple *> > sortedSlicedList;
        sortedSlicedList.reserve(numOfPartitions);

        for (vector<Tuple *> currentSlice: container) {

            mergeSort(currentSlice, 0, (int64_t)(currentSlice.size()-1), 'y',
                    leftAttrIndex);

            sortedSlicedList.push_back(currentSlice);
        }

        return sortedSlicedList;
    }

    string bucketInfo(vector<tchNode* > bucketVector)
    {
        stringstream info;
        info << (int64_t)bucketVector.size() << "(";

        for (int64_t i=0; i < (int64_t)bucketVector.size(); i++) {
            tchNode * bucket = bucketVector.at(i);

            info << (i+1) << "[" << (int64_t)bucket->objects.size();

            info << "]";
        }

        info << ")";

        cout << info.str() << endl;

        return info.str();
    }

    vector<tchNode * > packInBuckets(
            vector<vector <Tuple *> > sortedSlicedList,
            int64_t sizeOfSortedList,
            int64_t initialListSize,
            int numOfItemsInBucket,
            int leftStreamWordIndex)
    {
        int64_t numOfPartitions =
                ceil((double)initialListSize/numOfItemsInBucket);

        vector<tchNode * > containerOfBuckets;
        containerOfBuckets.reserve(numOfPartitions);
        tchNode * bucketNode = NULL;
        int64_t counter = 0;

        for (vector<Tuple *> innerList : sortedSlicedList) {

            while((int64_t)innerList.size() > 0){
                counter++;

                if (bucketNode == NULL) {
                    bucketNode = new  tchNode(true);
                }

                bucketNode->level = 0;
                bucketNode->addObject(innerList.front(), leftStreamWordIndex);
                innerList.erase(innerList.begin());

                if(counter % numOfItemsInBucket == 0){

                    containerOfBuckets.push_back(bucketNode);

                    bucketNode = NULL;
                }
            }
            innerList.clear();

            if (bucketNode != NULL) {
                containerOfBuckets.push_back(bucketNode);
            }

        }

        return containerOfBuckets;
    }

    void mergeSort(
            vector<Tuple *> &tuples,
            int64_t l,
            int64_t r,
            char direction,
            int64_t leftAttrIndex)
    {
        if (l < r)
        {
            int64_t m = l+(r-l)/2;

            mergeSort(tuples, l, m, direction, leftAttrIndex);
            mergeSort(tuples, m+1, r, direction, leftAttrIndex);

            merge(tuples, l, m, r, direction, leftAttrIndex);
        }
    }

    void merge(
            vector<Tuple *> &tuples,
            int64_t l,
            int64_t m,
            int64_t r,
            char direction,
            int64_t leftAttrIndex)
    {
        Attribute * attr1, * attr2;
        double valueL, valueR;
        int64_t i, j, k;
        int64_t n1 = m - l + 1;
        int64_t n2 =  r - m;

        vector<Tuple*> L(n1), R(n2);

        for (i = 0; i < n1; i++)
            L[i] = tuples[l + i];
        for (j = 0; j < n2; j++)
            R[j] = tuples[m + 1+ j];

        i = 0;
        j = 0;
        k = l;
        while (i < n1 && j < n2)
        {
            if (direction == 'x') {

                attr1 = L[i]->GetAttribute(leftAttrIndex);
                attr2 = R[j]->GetAttribute(leftAttrIndex);

                valueL = (attr1->getMinX() + attr1->getMaxX()) / 2;
                valueR = (attr2->getMinX() + attr2->getMaxX()) / 2;

            } else if (direction == 'y') {

                attr1 = L[i]->GetAttribute(leftAttrIndex);
                attr2 = R[j]->GetAttribute(leftAttrIndex);

                valueL = (attr1->getMinY() + attr1->getMaxY()) / 2;
                valueR = (attr2->getMinY() + attr2->getMaxY()) / 2;
            }


            if (valueL <= valueR)
            {
                tuples[k] = L[i];
                i++;
            }
            else
            {
                tuples[k] = R[j];
                j++;
            }
            k++;
        }

        while (i < n1)
        {
            tuples[k] = L[i];
            i++;
            k++;
        }

        while (j < n2)
        {
            tuples[k] = R[j];
            j++;
            k++;
        }
    }

    /*
     * 1. create Array
     * 2. run mergeSort - sort by first dimension - x
     * 3. split in slices
     * 4. run mergeSort - sort by second dimension - y
     * 5. pack in buckets
     */
    vector<tchNode * > createBuckets(
            vector<Tuple*> tuples,
            int _firstStreamWordIndex,
            int _numOfItemsInBrucket
    ) {
        int64_t size = (int64_t) tuples.size();

        assert(size > 0);

        // # 1 create Array

        // # 2 run mergeSort - sort by first dimension - x
        STR::mergeSort(tuples, 0, size - 1, 'x', _firstStreamWordIndex);

        // # 3
        int numOfItemsInBrucket = _numOfItemsInBrucket;
        vector<vector <Tuple *> > container = STR::splitInSlices(
                tuples,
                numOfItemsInBrucket,
                size);

        // # 4
        vector<vector <Tuple *> > sortedSlicedList = STR::sortSecondDimension(
                container,
                _firstStreamWordIndex,
                numOfItemsInBrucket,
                size
        );


        // # 5
        return STR::packInBuckets(
                sortedSlicedList,
                (int64_t)sortedSlicedList.size(),
                size,
                numOfItemsInBrucket,
                _firstStreamWordIndex);
    }

}
