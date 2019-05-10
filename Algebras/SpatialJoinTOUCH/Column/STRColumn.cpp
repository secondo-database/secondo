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

    vector<vector <binaryTuple *> > splitInSlices(
            binaryTuple * arr[],
            int numOfPartitions,
            int array_size)
    {
        int numOfObjsInSlices = ceil((double)array_size/numOfPartitions)*2;

        int counter = 0;

        vector<binaryTuple *> temp;
        vector<vector <binaryTuple *> > container;
        temp.reserve(numOfObjsInSlices);
        container.reserve(numOfPartitions);

        for( int i = 0; i < array_size; i++ ) {
            counter++;

            temp.push_back(arr[i]);

            if (counter % numOfObjsInSlices == 0) {
                container.push_back(temp);
                temp.clear();
            }
        }

        if (!temp.empty()) {
            container.push_back(temp);
        }

        return container;
    }

    vector<vector <binaryTuple *> > sortSecondDimension(
            vector<vector <binaryTuple *> > container,
            int leftAttrIndex,
            int numOfPartitions
            )
    {
        vector<vector <binaryTuple *> > sortedSlicedList;
        sortedSlicedList.reserve(numOfPartitions);

        for (vector<binaryTuple *> currentSlice: container) {
            binaryTuple * arr[currentSlice.size()];
            copy(currentSlice.begin(), currentSlice.end(), arr);

            mergeSort(arr, 0, (int)(currentSlice.size()-1), 'y', leftAttrIndex);

            int arraySize = sizeof(arr)/ sizeof(arr[0]);

            vector<binaryTuple *> v(arr, arr + arraySize);

            sortedSlicedList.push_back(v);
        }

        return sortedSlicedList;
    }


    string bucketInfo(vector<nodeCol* > bucketVector)
    {
        stringstream info;
        info << (int)bucketVector.size() << "(";

        for (int i=0; i < (int)bucketVector.size(); i++) {
            nodeCol * bucket = bucketVector.at(i);

            info << (i+1) << "[" << (int)bucket->objects.size();

            info << "]";
        }

        info << ")";

        cout << info.str() << endl;

        return info.str();
    }

    vector<nodeCol * > packInBuckets(
            vector<vector <binaryTuple *> > sortedSlicedList,
            int sizeOfSortedList,
            int initialListSize,
            int numOfPartitions,
            int leftStreamWordIndex)
    {
        int numOfItemsInBucket = ceil((double)initialListSize/numOfPartitions);
        vector<nodeCol * > containerOfBuckets;
        containerOfBuckets.reserve(numOfPartitions);
        nodeCol * bucketNode = NULL;
        int counter = 0;

        for (vector<binaryTuple *> innerList : sortedSlicedList) {

            int sizeOfInnerList = (int)innerList.size();
            while(sizeOfInnerList > 0){
                counter++;

                if (bucketNode == NULL) {
                    bucketNode = new  nodeCol(true);
                }

                bucketNode->level = 0;
                bucketNode->addObject(innerList.front());
                innerList.erase(innerList.begin());

                sizeOfInnerList = (int)innerList.size();

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
            binaryTuple * arr[],
            int l,
            int r,
            char direction,
            int _firstStreamWordIndex
            )
    {
        if (l < r)
        {
            int m = l+(r-l)/2;

            mergeSort(arr, l, m, direction, _firstStreamWordIndex);
            mergeSort(arr, m+1, r, direction, _firstStreamWordIndex);

            merge(arr, l, m, r, direction, _firstStreamWordIndex);
        }
    }

    void merge(
            binaryTuple * arr[],
            int l,
            int m,
            int r,
            char direction,
            int _firstStreamWordIndex
            )
    {
        double valueL, valueR;
        binaryTuple * entryL,* entryR;
        int i, j, k;
        int n1 = m - l + 1;
        int n2 =  r - m;

        binaryTuple * L[n1], * R[n2];

        for (i = 0; i < n1; i++)
            L[i] = arr[l + i];
        for (j = 0; j < n2; j++)
            R[j] = arr[m + 1+ j];

        i = 0;
        j = 0;
        k = l;
        while (i < n1 && j < n2)
        {
            if (direction == 'x') {

                entryL = L[i];
                entryR = R[j];

                valueL = (entryL->xMin + entryL->xMax) / 2;
                valueR = (entryR->xMin + entryR->xMax) / 2;


            } else if (direction == 'y') {

                entryL = L[i];
                entryR = R[j];

                valueL = (entryL->yMin + entryL->yMax) / 2;
                valueR = (entryR->yMin + entryR->yMax) / 2;

            }

            if (valueL <= valueR)
            {
                arr[k] = L[i];
                i++;
            }
            else
            {
                arr[k] = R[j];
                j++;
            }
            k++;
        }

        while (i < n1)
        {
            arr[k] = L[i];
            i++;
            k++;
        }

        while (j < n2)
        {
            arr[k] = R[j];
            j++;
            k++;
        }
    }

    void createArrayFromTupleVector(
            binaryTuple * arr[], vector<binaryTuple *> tuples) {

        long i = 0;

        for (binaryTuple * bt: tuples) {
            arr[i] = bt;
            i++;
        }
        tuples.clear();
    }

    vector<nodeCol *> createBuckets(
            vector<binaryTuple *> tuples,
            int _firstStreamWordIndex,
            int _numOfPartitions
            ) {

        int size = (int) tuples.size();


        // # 1 create Array
        binaryTuple * arr[size] = {};
        STRColumn::createArrayFromTupleVector(arr, tuples);

        // # 2 run mergeSort - sort by first dimension - x
        STRColumn::mergeSort(arr, 0, size - 1, 'x', _firstStreamWordIndex);

        // # 3
        int numOfPartitions = _numOfPartitions;
        vector<vector <binaryTuple *> > container = STRColumn::splitInSlices(
                arr,
                numOfPartitions,
                size);

        // # 4
        vector<vector <binaryTuple *> > sortedSlicedList =
                STRColumn::sortSecondDimension(
                    container,
                    _firstStreamWordIndex,
                    numOfPartitions
        );

        // # 5
        return STRColumn::packInBuckets(
                sortedSlicedList,
                (int)sortedSlicedList.size(),
                size,
                numOfPartitions,
        _firstStreamWordIndex);


    }


}