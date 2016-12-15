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

[1] Implementation of Class Distsamp

August-February 2015, Daniel Fuchs 

[TOC]

1 Overview


This is a implentation for distributed sort. This Class sort 
data into groups.

1.1 Includes

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
#include <fstream> 
#include "FTextAlgebra.h"
#include "Member.h"
#include <limits>
#include <limits.h>
#include "MergeSort.h"

#ifndef SISTRIBUTEDSORT_H
#define SISTRIBUTEDSORT_H


namespace distributedClustering{
  
/*
2.1 class ~Distsamp~

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  class Distsamp{
  public:
   
       
/*
2.2 ~constructor~

Constructor sorts the sample file and init the output.

*/
    Distsamp(Word& _inSampStream, Word& _sampStream, ListExpr& _tupleType,
             int _attrPos, int _xPicRefPos, bool _appendPictureRefs,
             int _cntWorkers, size_t _maxMem):
             cntWorkers(_cntWorkers)
             ,attrPos(_attrPos), xPicRefPos(_xPicRefPos)
             ,appendPictureRefs(_appendPictureRefs)
             ,buffer(0),sampBuff(0)
             ,resIt(0),tt(0)
             ,xRefPic(0),yRefPic(0),maxDist(0)
    {
      tt = new TupleType(_tupleType);
      init(_maxMem,_inSampStream, _sampStream);
      mergeSort<TYPE,MEMB_TYP_CLASS>(sampleArray,0,sampleArray.size());
      initOutput();
    }
    
/*
2.3 ~destructor~

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
2.4 ~next~

Returns the next output tuple which are expandet with
Worker id.
Requires the call of initOutput before.

*/    
    Tuple* next(){
      if(resIt){
        Tuple* tuple = resIt->GetNextTuple();
        if(!tuple){
          return 0;
        }
//         TupleId id = resIt->GetTupleId();
        Tuple* resTuple = new Tuple(tt);
        int noAttr = tuple->GetNoAttributes();
        for(int i = 0; i<noAttr; i++){
          resTuple->CopyAttribute(i,tuple,i);
        }
        tuple->DeleteIfAllowed();
        TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
        MEMB_TYP_CLASS dummy(obj);
        if(appendPictureRefs){ 
          dummy.setCoordinates(xRefPic,yRefPic);
        }
        resTuple->PutAttribute(noAttr, 
                               new CcInt(true,
                                      getWorkerID((double)dummy.getXVal())));
        if(appendPictureRefs){ 
          resTuple->PutAttribute(noAttr + 1, xRefPic->Clone());
          resTuple->PutAttribute(noAttr + 2, 
                                 new CcReal(true,dummy.getYVal()));
        }
        return resTuple;
      } else {
        return 0;
      }
  
    }
    
    
    
  private:

/*
2.5 ~members~

*/
    int  cntWorkers,attrPos,xPicRefPos;
    bool appendPictureRefs;
    TupleBuffer* buffer, *sampBuff;
    GenericRelationIterator* resIt;  // iterator 
    TupleType* tt;   // the result tuple type
    std::vector <MEMB_TYP_CLASS*> sampleArray;
    std::vector <double> border;
    //for Picture
    TYPE *xRefPic, *yRefPic;
    double maxDist;

/*
2.6 ~initialize~
 
Read in the tuple streams and store them in 
a vector.

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
      
      bool firstRun = true;
      bool secondRun = true;
      bool findPictureCoordRefs = false;
      bool pictureRefsExist = false;
      if(TYPE::BasicType() == Picture::BasicType()
        && appendPictureRefs)
      {
        findPictureCoordRefs = true;
        pictureRefsExist = false;
      }else if(TYPE::BasicType() == Picture::BasicType()
        && !appendPictureRefs
        && xPicRefPos >=0 )
      {
        findPictureCoordRefs = false;
        pictureRefsExist = true;
      }
      
      while((tuple = sampStream.request())){
        sampBuff->AppendTuple(tuple);
        TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
        if(obj->IsDefined()){
          MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
          sampleArray.push_back(member);
          
          if(pictureRefsExist){
            xRefPic = (TYPE*) tuple->GetAttribute(xPicRefPos);
            double yRefPicVal = ((CcReal*) 
                      tuple->GetAttribute(xPicRefPos+1))->GetValue();
            member->setCoordinates(xRefPic,yRefPicVal);
          }
          if(findPictureCoordRefs)
          {
            if(firstRun)
            {
              xRefPic = obj; 
              firstRun = false;
            }
            else //  if(!firstRun && findPictureCoordRefs)
            {
              if(secondRun){
                yRefPic = obj;
                maxDist = member->calcDistanz(xRefPic);
              } else 
              if(member->calcDistanz(xRefPic) > maxDist)
              {
                yRefPic = obj;
                maxDist = member->calcDistanz(xRefPic);
              }
            }
          }
        }
        tuple->DeleteIfAllowed();
      }
      
      if(findPictureCoordRefs)
      {
        //search maxDist
        for (size_t i = 1; i < sampleArray.size()-1;i++)
        {
          if( sampleArray.at(i)->calcDistanz(yRefPic) > maxDist)
          {
            xRefPic = sampleArray.at(i)->getPoint();
            maxDist =sampleArray.at(i)->calcDistanz(yRefPic);
          }
        }
        //set coordinates to each member
        for (size_t i = 0; i < sampleArray.size();i++)
        {
          sampleArray.at(i)->setCoordinates(xRefPic,yRefPic);
        }
      }
      sampStream.close();
    }
    
/*
2.7 ~getWorkerID~

Returns the WorkerId which is putted to the result relation.

*/
    int getWorkerID(double val){ 
      int mid = 0,left = 0,right = border.size()-1;
      while(left <= right)
      {
        mid = (int) floor(left + ((right - left) / 2 ));
        if(val < border.at(mid) && val >= border.at(mid+1))
        {
          return mid;
        }else
          if(val >= border.at(mid))
          {
          right = mid -1;
          }else
            if(val < border.at(mid+1))
            {
              left = mid +1;
            }
        }
        
      return -1;
    }
    
/*
2.8 ~initOutput~
Starts the begin of returning tuples.

*/
    void initOutput(){
      if(resIt) delete resIt;
      resIt = buffer->MakeScan(); 
      
      //make bordervektor
      border.clear();
      border.push_back(std::numeric_limits<double>::max());
      for(int i = 1; i < cntWorkers; i++){
        border.push_back(sampleArray.at(b(i))->getXVal());
      }
      border.push_back(-1 * std::numeric_limits<double>::max());
    }

/*
2.11 ~b~

Return the position of Borderpoint i with
$b_i = i \cdot \lfloor \frac{s}{t} \rfloor$

*/
    int b(int i){
      
      int retVal = i* (int) 
      floor((double)sampleArray.size() / (double) cntWorkers);
      return retVal;
    }
    
  };
}

#endif
