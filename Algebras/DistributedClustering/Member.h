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
 
 
 This file contains the implementation of the class Member
 
 2 Includes
  
*/ 

#include <limits>
#include <list>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "Point.h"
#include "StandardTypes.h"
#include "StringUtils.h"
#include "PictureAlgebra.h"
#include "RelationAlgebra.h"

#ifndef MEMBER_H_
#define MEMBER_H_

using namespace std;

namespace distributedClustering{
  

/*
* class Member
* is a wrapper class for the member elements linke points etc

*/

template <class MEMB_TYP_CLASS>
class Member{ 
  
public:
  //   int* point;
  list<MEMB_TYP_CLASS*> epsNeighborhood;
  
  bool innerPnt; //if epsNeighborhood > minPts
  
  bool densityReachable; 
  //true if one point in epsNeighborhood is a innerPnt or it is a innerPnt
  //   bool outerPnt;
  int clusterNo,clusterType,tuplePos;
  long int tupleId;
  
  //reference to Tuple
  Tuple* tuple;
  
  
/*
 constructor

*/
  Member():innerPnt(false),densityReachable(false),
  clusterNo(-1),clusterType(-1),tuplePos(0),tupleId(0){}

/*
* addNeighbor
* add one or two neighbors to epsNeighborhood

*/
  void addNeighbor(MEMB_TYP_CLASS* memb){
    epsNeighborhood.push_back(memb);
  }
  
/*
* updateInnerPnt
* update the boolean innerPnt

*/
  bool updateInnerPnt(int minPts){
    int size = epsNeighborhood.size() ;
    size >= minPts ? innerPnt =true : innerPnt= false;
    return innerPnt;
  }
  
  bool isInnerPoint(){
    return innerPnt;
  }
  
  
  bool isDensityReachable(){
    if(innerPnt)
      densityReachable= true;
    return densityReachable;
  }
  
/*
* updateDensityReachable
* looks if a point is a inner Point or is density  reachable

*/
  bool updateDensityReachable(int minPts){
    bool retVal=false;
    if(innerPnt || densityReachable){
      retVal=true;
    }else{
      typename list<MEMB_TYP_CLASS*>::iterator it = epsNeighborhood.begin();
      while(it!=epsNeighborhood.end() && !retVal){
        if((*it)->updateInnerPnt(minPts)){
          retVal=true;
        }
        it++;
      }
    }
    densityReachable = retVal;
    
    return densityReachable;
  }
  
/*
 existNeighbor
 is used for updateNeighbor in class Cluster. 
 Check if the committed Member is allready in the epsNeighborhood

*/
  bool existNeighbor(MEMB_TYP_CLASS* memb){
    typename list<MEMB_TYP_CLASS*>::iterator it = epsNeighborhood.begin();
    bool exist = false;
    if(this == memb){
      exist=true;
    }
    while(it!=epsNeighborhood.end() && !exist){
      if((*it)==memb)
        exist = true;
      else
        exist = false;
      it++;
    }
    return exist;
  }
  
  
  int getCountNeighbors(){
    return epsNeighborhood.size();
  }
  
/*
 getEpsNeighborhood
 return an iterator for the epsNeighborhood list. The committed bool value
 determines if return the beginning or end of list

*/
  typename list<MEMB_TYP_CLASS*>::iterator getEpsNeighborhood(bool begin){
    if(begin)
      return epsNeighborhood.begin();
    else
      return epsNeighborhood.end();
  }
  
  void setClusterType(int type){
    clusterType = type;
  }
  int getClusterType(){
    return clusterType;
  }
  
  void setClusterNo(int no){
    clusterNo=no;
  }
  
  int getClusterNo(){
    return clusterNo;
  }
  
  bool isClusterMember(){
    if(getClusterNo() < 0){
      return false;
    }else{
      return true;
    }
  }
  
  void setTuple(Tuple* tup){
    tuple = tup;
  }
  
  Tuple* getTuple(){
    return tuple;
  }
  
//   int getTuplePos(){
//     return tuplePos;
//   }
//   
//   void setTuplePos(int pos){
//     tuplePos = pos;
//   }
  
  long int getTupleId(){
    return tupleId;
  }
  
  void setTupleId(long int id){
    tupleId = id;
  }
  

};




class IntMember : public Member<IntMember>{
private:
  
  CcInt* point;
  
public:
/*
 Constructor

*/
  IntMember(): point(0){
    innerPnt=false;
    densityReachable = false;
  }
  
  IntMember(CcInt* memb) {
    point= memb;
    innerPnt=false;
    densityReachable = false;
  }
  
/*
 return the point value

*/
  CcInt* getPoint(){
    return point;
  }
  
  double getXVal(){
    if(!point->IsDefined())
      return 0;
    return static_cast<double>(point->GetValue());
  }
  
  double getYVal(){
    return getXVal();
  }
  
/*
 calcDistanz
 calculate the distance between this and the committed point

*/
  double calcDistanz (IntMember* memb){
    if(!point->IsDefined() && !(memb->getPoint())->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !(memb->getPoint())->IsDefined()){
      return numeric_limits<double>::max();
    }
    double retval= memb->getXVal() - getXVal();
    return retval<0 ? -retval:retval;
    
  }
  
  double calcDistanz(CcInt* pnt){
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return numeric_limits<double>::max();
    }
    
    double retval= pnt->GetValue() - getXVal();
    return retval<0 ? -retval:retval;
    
  }
  
  double calcXDistanz(CcInt* pnt){
    return calcDistanz(pnt);
  }
  
  void printPoint(){
    cout << getXVal();
  }
  
  
  CcInt* getOuterLeftValue(CcInt* outerPoint, CcInt* innerPoint ){
    return 
    innerPoint->GetValue() 
    > outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
  
  CcInt* getOuterRightValue(CcInt* outerPoint,  
                            CcInt* outerPointRighCl, CcInt* innerPoint ){
    CcInt* retPoint = getOuterRightValue( outerPoint,  innerPoint ) ;
    return getOuterRightValue(outerPointRighCl,retPoint) ;
  }
  
  CcInt* getOuterRightValue(CcInt* outerPoint, CcInt* innerPoint ){
    return innerPoint->GetValue() 
    < outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
  
};

class RealMember :public Member<RealMember>{
private:
  
  CcReal* point;
  
public:
/*
 Constructor

*/
  RealMember() : point(0){
    innerPnt=false;
    densityReachable = false;
  }
  
  RealMember(CcReal* memb){
    point = memb;
    innerPnt=false;
    densityReachable = false;
  }
/*
 return the point value

*/
  CcReal* getPoint(){
    return point;
  }
  
  double getXVal(){
    if(!point->IsDefined())
      return 0;
    return point->GetValue();
  }
  
  double getYVal(){
    return getXVal();
  }
  
/*
 calcDistanz
 calculate the distance between this and the committed point

*/
  double calcDistanz (RealMember* memb){
    if(!point->IsDefined() && !(memb->getPoint())->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !(memb->getPoint())->IsDefined()){
      return numeric_limits<double>::max();
    }
    double retval= memb->getXVal() - getXVal();
    return retval<0 ? -retval:retval;
    
  }
  
  double calcDistanz(CcReal* pnt){
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return numeric_limits<double>::max();
    }
    double retval= pnt->GetValue() - getXVal();
    return retval<0 ? -retval:retval;
  }
  
  double calcXDistanz(CcReal* pnt){
    return calcDistanz( pnt);
  }
  
  int getCntDimensions(){
    return 1;
  }
  
  void printPoint(){
    cout << getXVal();
  }
  
  CcReal* getOuterLeftValue(CcReal* outerPoint, CcReal* innerPoint ){
    return innerPoint->GetValue() 
    > outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
  
  CcReal* getOuterRightValue(CcReal* outerPoint,  
                             CcReal* outerPointRighCl, CcReal* innerPoint ){
    CcReal* retPoint = getOuterRightValue( outerPoint,  innerPoint ) ;
    return getOuterRightValue(outerPointRighCl,retPoint) ;
  }
  
  CcReal* getOuterRightValue(CcReal* outerPoint, CcReal* innerPoint ){
    return innerPoint->GetValue() 
    < outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
  
};


//class PointMember : Member;
class PointMember : public Member<PointMember>{
private:
  Point* point;
  
public:
/*
 Constructor

*/
  PointMember() : point(0){
    innerPnt=false;
    densityReachable = false;
  }
  
  PointMember(Point* memb){
    point= memb;
    innerPnt=false;
    densityReachable = false;
  }
/*
 return the point value

*/
  Point* getPoint(){
    return point;
  }
  
  double getXVal(){
    if(!point->IsDefined())
      return 0;
    return point->GetX();
  }
  
  double getYVal(){
    if(!point->IsDefined())
      return 0;
    return point->GetY();
  }

/*
calcDistanz

calculate the distance between this and the committed point

*/
  double calcDistanz (PointMember* memb){
    if(!point->IsDefined() && !(memb->getPoint())->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !(memb->getPoint())->IsDefined()){
      return numeric_limits<double>::max();
    }
    
    return point->Distance(*(memb->getPoint()));
    
  }
  
  double calcDistanz(Point* pnt){
    
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return numeric_limits<double>::max();
    }
    return point->Distance(*pnt);
  }
  
  double calcXDistanz(Point* pnt){
    
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return numeric_limits<double>::max();
    }
    
    double retval= pnt->GetX() - getXVal();
    return retval<0 ? -retval:retval;
    
  }
  
  int getCntDimensions(){
    return 2;
  }
  
  void printPoint(){
    cout << "("<< point->GetX()<<  ", " << point->GetY() <<  ") ";
  }
  
  Point* getOuterLeftValue(Point* outerPoint, Point* innerPoint ){
    return innerPoint->GetX() 
    > outerPoint->GetX() ? innerPoint : outerPoint ;
  }
  
  Point* getOuterRightValue(Point* outerPoint,  
                            Point* outerPointRighCl, Point* innerPoint ){
    Point* retPoint = getOuterRightValue( outerPoint,  innerPoint ) ;
    return getOuterRightValue(outerPointRighCl,retPoint) ;
  }
  
  Point* getOuterRightValue(Point* outerPoint, Point* innerPoint ){
    return innerPoint->GetX() < outerPoint->GetX() ? innerPoint : outerPoint ;
  }
  
};

}

#endif
