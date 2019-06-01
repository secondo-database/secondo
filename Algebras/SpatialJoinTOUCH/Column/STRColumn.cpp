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
#include "tupleBlockStr.h"

namespace CRelAlgebra {
    //class TBlockEntry;
    class TBlock;
    class TBlockIterator;

}


using namespace std;
using namespace mmrtreetouch;
using namespace CRelAlgebra;

namespace STRColumn {

    vector<vector <tupleBlockStr> > splitInSlices(
            vector<tupleBlockStr> tuples,
            int numOfItemsInBucket,
            int64_t vectorSize)
    {
        int64_t numOfPartitions = ceil((double)vectorSize/numOfItemsInBucket);

        int64_t counter = 0;

        vector<tupleBlockStr> temp;
        vector<vector <tupleBlockStr> > container;
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

    vector<vector <tupleBlockStr> > sortSecondDimension(
            vector<vector <tupleBlockStr> > container,
            int numOfItemsInBucket,
            int64_t vectorSize
            )
    {
        int64_t numOfPartitions = ceil((double)vectorSize/numOfItemsInBucket);

        vector<vector <tupleBlockStr> > sortedSlicedList;
        sortedSlicedList.reserve(numOfPartitions);

        for (vector<tupleBlockStr> currentSlice: container) {

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
            vector<vector <tupleBlockStr> > sortedSlicedList,
            int64_t sizeOfSortedList,
            int64_t initialListSize,
            int numOfItemsInBucket
            )
    {
        int64_t numOfPartitions =
                ceil((double)initialListSize/numOfItemsInBucket);

        vector<nodeCol * > containerOfBuckets;
        containerOfBuckets.reserve(numOfPartitions);
        nodeCol * bucketNode = NULL;
        int64_t counter = 0;

        for (vector<tupleBlockStr> innerList : sortedSlicedList) {

            int64_t sizeOfInnerList = (int64_t)innerList.size();
            while(sizeOfInnerList > 0){
                counter++;

                if (bucketNode == NULL) {
                    bucketNode = new  nodeCol(true);
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
            vector<tupleBlockStr> &tuples,
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
            vector<tupleBlockStr> &tuples,
            int64_t l,
            int64_t m,
            int64_t r,
            char direction
            )
    {
        double valueL, valueR;
        tupleBlockStr entryL, entryR;
        int64_t i, j, k;
        int64_t n1 = m - l + 1;
        int64_t n2 =  r - m;

        vector<tupleBlockStr> L(n1), R(n2);

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
            vector<tupleBlockStr> tuples,
            int _numOfItemsInBrucket
            ) {

        int64_t size = (int64_t) tuples.size();

        // # 2 run mergeSort - sort by first dimension - x
        STRColumn::mergeSort(tuples, 0, size - 1, 'x');

        // # 3
        int64_t numOfItemsInBrucket = _numOfItemsInBrucket;
        vector<vector <tupleBlockStr> > container = STRColumn::splitInSlices(
                tuples,
                numOfItemsInBrucket,
                size);

        // # 4
        vector<vector <tupleBlockStr> > sortedSlicedList =
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
                numOfItemsInBrucket
        );


    }
}