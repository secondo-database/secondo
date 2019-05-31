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
#include "STRColumn.h"
#include "Algebras/CRel/SpatialAttrArray.h"

#include <string>
#include <iostream>
#include <vector>
#include "nodeCol.h"

#include "Algebras/CRel/TBlock.h"
#include "BinaryTuple.h"

namespace CRelAlgebra {
    //class TBlockEntry;
    class TBlock;
    class TBlockIterator;

}


using namespace std;
using namespace mmrtreetouch;
using namespace CRelAlgebra;

namespace STRColumn {

    vector<vector <binaryTuple> > splitInSlices(
            vector<binaryTuple> tuples,
            int numOfItemsInBucket,
            int64_t vectorSize)
    {
        int64_t numOfPartitions = ceil((double)vectorSize/numOfItemsInBucket);

        int64_t counter = 0;

        vector<binaryTuple> temp;
        vector<vector <binaryTuple> > container;
        temp.reserve(numOfItemsInBucket);
        container.reserve(numOfPartitions);

        for( int64_t i = 0; i < vectorSize; i++ ) {
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

    vector<vector <binaryTuple> > sortSecondDimension(
            vector<vector <binaryTuple> > container,
            int numOfItemsInBucket,
            int64_t vectorSize
            )
    {
        int64_t numOfPartitions = ceil((double)vectorSize/numOfItemsInBucket);

        vector<vector <binaryTuple> > sortedSlicedList;
        sortedSlicedList.reserve(numOfPartitions);

        for (vector<binaryTuple> currentSlice: container) {

            mergeSort(currentSlice, 0, (int64_t)(currentSlice.size()-1), 'y');

            sortedSlicedList.push_back(currentSlice);
        }

        return sortedSlicedList;
    }


    string bucketInfo(vector<nodeCol* > bucketVector)
    {
        stringstream info;
        info << (int64_t)bucketVector.size() << "(";

        for (int64_t i=0; i < (int64_t)bucketVector.size(); i++) {
            nodeCol * bucket = bucketVector.at(i);

            info << (i+1) << "[" << (int64_t)bucket->objects.size();

            info << "]";
        }

        info << ")";

        cout << info.str() << endl;

        return info.str();
    }

    vector<nodeCol * > packInBuckets(
            vector<vector <binaryTuple> > sortedSlicedList,
            int64_t sizeOfSortedList,
            int64_t initialListSize,
            int numOfItemsInBucket,
            int64_t &remainingMem
            )
    {
        int64_t numOfPartitions =
                ceil((double)initialListSize/numOfItemsInBucket);

        vector<nodeCol * > containerOfBuckets;
        containerOfBuckets.reserve(numOfPartitions);
        nodeCol * bucketNode = NULL;
        int64_t counter = 0;

        for (vector<binaryTuple> innerList : sortedSlicedList) {

            int64_t sizeOfInnerList = (int64_t)innerList.size();
            while(sizeOfInnerList > 0){
                counter++;

                if (bucketNode == NULL &&
                    (remainingMem-sizeof(nodeCol) <= 0)) {
                        cout << "Memory is not enough 10" << endl;
                        remainingMem -= sizeof(nodeCol);
                        return containerOfBuckets;
                }

                if (bucketNode == NULL) {
                    bucketNode = new  nodeCol(true);
                    remainingMem -= sizeof(nodeCol);
                }

                bucketNode->level = 0;
                bucketNode->addObject(innerList.front());
                innerList.erase(innerList.begin());

                sizeOfInnerList = (int64_t)innerList.size();

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
            vector<binaryTuple> &tuples,
            int64_t l,
            int64_t r,
            char direction
            )
    {
        if (l < r)
        {
            int64_t m = l+(r-l)/2;

            mergeSort(tuples, l, m, direction);
            mergeSort(tuples, m+1, r, direction);

            merge(tuples, l, m, r, direction);
        }
    }

    void merge(
            vector<binaryTuple> &tuples,
            int64_t l,
            int64_t m,
            int64_t r,
            char direction
            )
    {
        double valueL, valueR;
        binaryTuple entryL, entryR;
        int64_t i, j, k;
        int64_t n1 = m - l + 1;
        int64_t n2 =  r - m;

        vector<binaryTuple> L(n1), R(n2);

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

                entryL = L[i];
                entryR = R[j];

                valueL = (entryL.xMin + entryL.xMax) / 2;
                valueR = (entryR.xMin + entryR.xMax) / 2;


            } else if (direction == 'y') {

                entryL = L[i];
                entryR = R[j];

                valueL = (entryL.yMin + entryL.yMax) / 2;
                valueR = (entryR.yMin + entryR.yMax) / 2;

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

    vector<nodeCol *> createBuckets(
            vector<binaryTuple> tuples,
            int _numOfItemsInBrucket,
            int64_t &remainingMem
            ) {

        int64_t size = (int64_t) tuples.size();

        // # 2 run mergeSort - sort by first dimension - x
        STRColumn::mergeSort(tuples, 0, size - 1, 'x');

        // # 3
        int64_t numOfItemsInBrucket = _numOfItemsInBrucket;
        vector<vector <binaryTuple> > container = STRColumn::splitInSlices(
                tuples,
                numOfItemsInBrucket,
                size);

        // # 4
        vector<vector <binaryTuple> > sortedSlicedList =
                STRColumn::sortSecondDimension(
                    container,
                    numOfItemsInBrucket,
                    size
        );

        // # 5
        return STRColumn::packInBuckets(
                sortedSlicedList,
                (int64_t)sortedSlicedList.size(),
                size,
                numOfItemsInBrucket,
                remainingMem
        );


    }
}