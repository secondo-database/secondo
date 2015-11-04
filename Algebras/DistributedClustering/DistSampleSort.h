/*
---- 
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
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the Spatial Algebra

Jun 2015, Daniel Fuchs 

[TOC]

1 Overview


This file contains the implementation of the class Distsamp

2 Includes


*/


#include "AlgebraTypes.h"
#include "RelationAlgebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "Stream.h"
#include <utility>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream> //TODO for log
#include "FTextAlgebra.h"
#include "Member.h"
#include <limits>  

#ifndef SISTRIBUTEDSORT_H
#define SISTRIBUTEDSORT_H

using namespace std;

namespace distributedClustering{
  
/*
class Distsamp

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  class Distsamp{
  public:
   
       
/*
constructor

*/
    Distsamp(Word& _inSampStream, Word& _sampStream, ListExpr& _tupleType,
             int _attrPos, int _cntWorkers, size_t _maxMem):
             cntWorkers(_cntWorkers),resIt(0),tt(0),buffer(0),sampBuff(0)
             ,attrPos(_attrPos)
    {
      tt = new TupleType(_tupleType);
      init(_maxMem,_inSampStream, _sampStream);
      mergeSort(sampleArray,0,sampleArray.size());
      initOutput();
    }
    
/*
destructor

*/
    ~Distsamp(){
      if(buffer)
        delete buffer;
      if(sampBuff)
        delete sampBuff;
      if(resIt) 
        delete resIt;
      if(tt) 
        tt->DeleteIfAllowed();
    }
    
/*
next

*/    
    Tuple* next(){
      if(resIt){
        Tuple* tuple = resIt->GetNextTuple();
        if(!tuple){
          return 0;
        }
        TupleId id = resIt->GetTupleId();
        Tuple* resTuple = new Tuple(tt);
        int noAttr = tuple->GetNoAttributes();
        for(int i = 0; i<noAttr; i++){
          resTuple->CopyAttribute(i,tuple,i);
        }
        tuple->DeleteIfAllowed();
        TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
        MEMB_TYP_CLASS dummy(obj);
        resTuple->PutAttribute(noAttr, 
                               new CcInt(true,
                                      getWorkerID((double)dummy.getXVal())));
        return resTuple;
      } else {
        return 0;
      }
  
    }
    
    
    
  private:

/*
members

*/
    int  cntWorkers,pos,sizeOfBuffer,attrPos;
    TupleBuffer* buffer, *sampBuff;
    GenericRelationIterator* resIt;  // iterator 
    TupleType* tt;   // the result tuple type
    vector <MEMB_TYP_CLASS*> sampleArray;
    vector <double> border;

/*
initialize

*/
    void init(size_t maxMem, Word& _inStream, Word& _sampStream){
      Tuple* tuple;
      buffer = new TupleBuffer(maxMem);
      Stream<Tuple> inStream(_inStream);
      inStream.open();
      while((tuple = inStream.request())){
        buffer->AppendTuple(tuple);
        tuple->DeleteIfAllowed();
      }
      inStream.close();
      sampBuff = new TupleBuffer();
      Stream<Tuple> sampStream(_sampStream);
      sampStream.open();
      while((tuple = sampStream.request())){
        sampBuff->AppendTuple(tuple);
        TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
        if(obj->IsDefined()){
          MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
          sampleArray.push_back(member);
        }
        tuple->DeleteIfAllowed();
      }
      sampStream.close();
    }
    
/*
getWorkerID

returns the WorkerId which is putted to the result relation

*/
    int getWorkerID(double val){ //TODO make a binary search
      bool workerFound = false;
      int worker = 0;
      for (int i=0; i< cntWorkers && !workerFound; i++){
        if(val < border.at(i) && val >= border.at(i+1)){
          workerFound = true;
          worker = i;
        }
      }
      return worker;
    }
    
/*
init Output()
Starts the begin of returning tuples.

*/
    void initOutput(){
      if(resIt) delete resIt;
      resIt = buffer->MakeScan(); 
      
      //make bordervektor
      border.clear();
      border.push_back(numeric_limits<double>::max());
      for(int i = 1; i < cntWorkers; i++){
        border.push_back(sampleArray.at(b(i))->getXVal());
      }
      border.push_back(-1 * numeric_limits<double>::max());
    }

/*
mergeSort
sort an array in ascending order

*/
     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       MEMB_TYP_CLASS ** auxiliaryArray = new MEMB_TYP_CLASS*[right-left+1];
       if(auxiliaryArray!= 0){
         mergeSort(array,left,right,auxiliaryArray);
         delete [] auxiliaryArray;
       }
     }
     
/*
mergeSort
sort an array in ascending order

*/
     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, 
                    int right,MEMB_TYP_CLASS** auxiliaryArray){
       if(right == left+1)
         return ; //mergeSort finisch
         else{
           int i = 0;
           int length = right - left;
           int median = (right - left)/2;
           int l = left; //position to the left subarray
           int r = left + median; //position to the right subarray
           
           //divide array
           mergeSort(array, left, r, auxiliaryArray);
           mergeSort(array, r, right, auxiliaryArray);
           
           //merge array
           /* Check to see if any elements remain in the left array; if so,
            * we check if there are any elements left in the right array; if
            * so, we compare them.  Otherwise, we know that the merge must
            * use take the element from the left array */
           for(i = 0; i < length; i++){
             if(l < left+median && (r==right || leftIsMax(array, l, r))){
               auxiliaryArray[i]=array[l];
               l++;
             }
             else{
               auxiliaryArray[i]= array[r];
               r++;
             }
           }
           //Copy the sorted subarray back to the input array
           for(i=left; i < right; i++){
             array[i]=auxiliaryArray[i-left];
           }
         }
     }
     
/*
leftIsMax()
auxiliary fuction to compare the maximum Object with the left object

*/
     bool leftIsMax(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       bool retVal = false;
       
       double leftXVal = array[left]->getXVal();
       double rightXVal = array[right]->getXVal();
       
       leftXVal > rightXVal ? retVal = true : retVal = false;
       return retVal;
     }
     
/*
b(int i)
return the position of Borderpoint i


*/
    int b(int i){
      
      int retVal = i* (int) 
      floor((double)sampleArray.size() / (double) cntWorkers);
      return retVal;
    }
    
  };
}

#endif
