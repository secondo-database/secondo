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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Class Member

August-February 2015, Daniel Fuchs 

[TOC]

1 Overview

This file contains the implementation of the generic class Member  
and their inheriting classes for each secondo type.

1.1 Includes

*/ 

#include <limits>
#include <list>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "Algebras/Spatial/Point.h"
#include "StandardTypes.h"
#include "StringUtils.h"
#include "Algebras/Picture/PictureAlgebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/GeneralTree/DistfunReg.h"
#include "Algebras/GeneralTree/PictureFuns.h"

#ifndef MEMBER_H_
#define MEMBER_H_


namespace distributedClustering{
  

/*
2 class ~Member~

Is a generic wrapper class for the member elements linke points etc.

*/

template <class MEMB_TYP_CLASS>
class Member{ 
  
  
public:
  Picture* picRef;
  //   int* point;
  std::list<MEMB_TYP_CLASS*> epsNeighborhood;
  
  bool innerPnt; //if epsNeighborhood > minPts
  
  bool densityReachable; 
  //true if one point in epsNeighborhood is a innerPnt or it is a innerPnt
  //   bool outerPnt;
  int clusterNo,clusterType,tuplePos;
  long int tupleId;
  
  //reference to Tuple
  Tuple* tuple;
  
  
/*
2.1 ~constructor~

*/
  Member():innerPnt(false),densityReachable(false),
  clusterNo(-1),clusterType(-1),tuplePos(0),tupleId(0){}

/*
2.2 ~addNeighbor~

Add a members addNeighbor to eps neighborhood. 

*/
  void addNeighbor(MEMB_TYP_CLASS* memb){
    epsNeighborhood.push_back(memb);
  }
  
/*
2.3 ~updateInnerPnt~

Check if this is a inner point wrt. eps and minPts.

*/
  bool updateInnerPnt(int minPts){
    if(!innerPnt){ 
    int size = epsNeighborhood.size() ;
    size >= minPts ? innerPnt =true : innerPnt= false;
    }
    densityReachable = innerPnt;
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
2.4  ~updateDensityReachable~

Check if this isdensity reachable wrt. eps and minPts.

*/
  bool updateDensityReachable(int minPts){
    bool retVal=false;
    if(innerPnt || densityReachable){
      retVal=true;
    }else{
      typename std::list<MEMB_TYP_CLASS*>::iterator 
              it = epsNeighborhood.begin();
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
2.5 ~existNeighbor~

Chekc if the given member is already in eps neighborhood.

*/
  bool existNeighbor(MEMB_TYP_CLASS* memb){
    typename std::list<MEMB_TYP_CLASS*>::iterator it = epsNeighborhood.begin();
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
2.6 ~getEpsNeighborhood~

Return an iterator for the epsNeighborhood list. The committed bool value
 determines if return the beginning or end of list.

*/
  typename std::list<MEMB_TYP_CLASS*>::iterator getEpsNeighborhood(bool begin){
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
  
  long int getTupleId(){
    return tupleId;
  }
  
  void setTupleId(long int id){
    tupleId = id;
  }
  
};


/*
3 class ~IntMember~

Is the wrapper member class for secondo type int. Inherhit form 
class Member.

*/

class IntMember : public Member<IntMember>{
private:
  
  CcInt* point;
  
public:
/*
3.1 ~constructor~

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
3.2  ~calcDistanz~

Calculate the distance between this and the committed point.

*/
  double calcDistanz (IntMember* memb){
    if(!point->IsDefined() && !(memb->getPoint())->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !(memb->getPoint())->IsDefined()){
      return std::numeric_limits<double>::max();
    }
    double retval= memb->getXVal() - getXVal();
    return retval<0 ? -retval:retval;
    
  }
  
  double calcDistanz(CcInt* pnt){
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return std::numeric_limits<double>::max();
    }
    
    double retval= pnt->GetValue() - getXVal();
    return retval<0 ? -retval:retval;
    
  }
  
/*
3.3 ~calcXDistanz~

Calculate the distance in the first 
dimension between this and the committed point.

*/
  double calcXDistanz(CcInt* pnt){
    return calcDistanz(pnt);
  }
  
  void printPoint(){
    cout << getXVal();
  }
  
/*
3.4 ~getOuterLeftValue~
  
get the mostly left item.
  
*/
  CcInt* getOuterLeftValue(CcInt* outerPoint, CcInt* innerPoint ){
    return 
    innerPoint->GetValue() 
    > outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
  
/*
3.5 ~getOuterRightValue~

get teh mostly right item.

*/
  CcInt* getOuterRightValue(CcInt* outerPoint,  
                            CcInt* outerPointRighCl, CcInt* innerPoint ){
    CcInt* retPoint = getOuterRightValue( outerPoint,  innerPoint ) ;
    return getOuterRightValue(outerPointRighCl,retPoint) ;
  }
  
  CcInt* getOuterRightValue(CcInt* outerPoint, CcInt* innerPoint ){
    return innerPoint->GetValue() 
    < outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
 
/*
3.6 ~setCoordinates~

Only used for picture type.

*/
  void setCoordinates(CcInt* xRef,CcInt* yRef){};
  void setCoordinates(CcInt* _xRef,double _yRef){};
  
};

/*
4 class ~RealMember~

Is the wrapper member class for secondo type real. Inherhit form 
class Member.

*/
class RealMember :public Member<RealMember>{
private:
  
  CcReal* point;
  
public:
/*
4.1 ~constructor~

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
4.2  ~calcDistanz~

Calculate the distance between this and the committed point.

*/
  double calcDistanz (RealMember* memb){
    if(!point->IsDefined() && !(memb->getPoint())->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !(memb->getPoint())->IsDefined()){
      return std::numeric_limits<double>::max();
    }
    double retval= memb->getXVal() - getXVal();
    return retval<0 ? -retval:retval;
    
  }
  
  double calcDistanz(CcReal* pnt){
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return std::numeric_limits<double>::max();
    }
    double retval= pnt->GetValue() - getXVal();
    return retval<0 ? -retval:retval;
  }
  
/*
4.3 ~calcXDistanz~

Calculate the distance in the first 
dimension between this and the committed point.

*/
  double calcXDistanz(CcReal* pnt){
    return calcDistanz( pnt);
  }
  
  int getCntDimensions(){
    return 1;
  }
  
  void printPoint(){
    cout << getXVal();
  }
  
/*
4.4 ~getOuterLeftValue~
  
get the mostly left item.
  
*/
  CcReal* getOuterLeftValue(CcReal* outerPoint, CcReal* innerPoint ){
    return innerPoint->GetValue() 
    > outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
  
/*
4.5 ~getOuterRightValue~

get teh mostly right item.

*/
  CcReal* getOuterRightValue(CcReal* outerPoint,  
                             CcReal* outerPointRighCl, CcReal* innerPoint ){
    CcReal* retPoint = getOuterRightValue( outerPoint,  innerPoint ) ;
    return getOuterRightValue(outerPointRighCl,retPoint) ;
  }
  
  CcReal* getOuterRightValue(CcReal* outerPoint, CcReal* innerPoint ){
    return innerPoint->GetValue() 
    < outerPoint->GetValue() ? innerPoint : outerPoint ;
  }
  
 
/*
4.6 ~setCoordinates~

Only used for picture type.

*/
  void setCoordinates(CcReal* xRef,CcReal* yRef){};
  void setCoordinates(CcReal* _xRef,double _yRef){};
  
};

/*
5 class ~PointMember~

Is the wrapper member class for secondo type point. Inherhit form 
class Member.

*/
class PointMember : public Member<PointMember>{
private:
  Point* point;
  
public:
/*
5.1 ~constructor~

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
5.2  ~calcDistanz~

Calculate the distance between this and the committed point.

*/
  double calcDistanz (PointMember* memb){
    if(!point->IsDefined() && !(memb->getPoint())->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !(memb->getPoint())->IsDefined()){
      return std::numeric_limits<double>::max();
    }
    
    return point->Distance(*(memb->getPoint()));
    
  }
  
  double calcDistanz(Point* pnt){
    
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return std::numeric_limits<double>::max();
    }
    return point->Distance(*pnt);
  }
  
/*
5.3 ~calcXDistanz~

Calculate the distance in the first 
dimension between this and the committed point.

*/
  double calcXDistanz(Point* pnt){
    
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return std::numeric_limits<double>::max();
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
  
/*
5.4 ~getOuterLeftValue~
  
get the mostly left item.
  
*/

  Point* getOuterLeftValue(Point* outerPoint, Point* innerPoint ){
    return innerPoint->GetX() 
    > outerPoint->GetX() ? innerPoint : outerPoint ;
  }
  
/*
5.5 ~getOuterRightValue~

get teh mostly right item.

*/
  Point* getOuterRightValue(Point* outerPoint,  
                            Point* outerPointRighCl, Point* innerPoint ){
    Point* retPoint = getOuterRightValue( outerPoint,  innerPoint ) ;
    return getOuterRightValue(outerPointRighCl,retPoint) ;
  }
  
  Point* getOuterRightValue(Point* outerPoint, Point* innerPoint ){
    return innerPoint->GetX() < outerPoint->GetX() ? innerPoint : outerPoint ;
  }
  
/*
5.6 ~setCoordinates~

Only used for picture type.

*/
  void setCoordinates(Point* xRef,Point* yRef){};
  void setCoordinates(Point* _xRef,double _yRef){};
};



/*
6 class ~PictureMember~

Is the wrapper member class for secondo type picture. Inherhit form 
class Member.

*/

class PictureMember : public Member<PictureMember>{
private:
  Picture* point, *xRef; //, *yRef;
  gta::DistfunInfo df;
  double xVal,yVal;
  
public:
/*
6.1 ~constructor~

*/
  PictureMember() : point(0),xVal(0),yVal(0){
    innerPnt=false;
    densityReachable = false;
    init();

  }
  
  PictureMember(Picture* memb):xVal(0),yVal(0){
    point= memb;
    innerPnt=false;
    densityReachable = false;
    init();

  }

/*
6.2 ~init~

initalize the picture distance function.

*/
  void init()
  {
    if(!gta::DistfunReg::isInitialized())
    {
      gta::DistfunReg::initialize();
    }
    gta::DistDataId id = gta::DistDataReg::getId(Picture::BasicType()
    ,gta::DistDataReg::defaultName(Picture::BasicType()));
    
//     df = gta::DistfunReg::getInfo(gta::DFUN_DEFAULT, id);
    df = gta::DistfunReg::getInfo(gta::DFUN_QUADRATIC, id);
  }
  
  

  Picture* getPoint(){
    return point;
  }
  
  double getXVal(){
    
    return xVal;
  }
  
  double getXValOfPic(Picture *pic){
    return calcDistanz( xRef,  pic);
  }
  
  double getYVal(){
    return xVal;
  }
  
/*
6.3  ~calcDistanz~

Calculate the distance between this and the committed point.

*/
  double calcDistanz (PictureMember* memb){
    if(!point->IsDefined() && !(memb->getPoint())->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !(memb->getPoint())->IsDefined()){
      return  std::numeric_limits<double>::max();
    }
    
    return calcDistanz(point,memb->getPoint());
    
  }
  
  double calcDistanz(Picture* pnt){
    
    if(!point->IsDefined() && !pnt->IsDefined()){
      return 0.0;
    }
    if(!point->IsDefined() || !pnt->IsDefined()){
      return  std::numeric_limits<double>::max();
    }
    return calcDistanz(point,pnt);
    
  }
  
  double calcDistanz(Picture* pnt1, Picture* pnt2)
  {
    double distance;
    gta::DistData* dd1 = df.getData(pnt1);
    gta::DistData* dd2 = df.getData(pnt2); 
    df.dist(dd1, dd2, distance);
    delete dd1;
    delete dd2;
    return distance;
  }
  
/*
6.4 ~calcXDistanz~

Calculate the distance in the first 
dimension between this and the committed point.

*/
  double calcXDistanz(Picture* pnt){
    return calcDistanz(pnt);
    
  }
  
  int getCntDimensions(){
    return 2;
  }
  
  void printPicture(){
  }
  
/*
6.5 ~getOuterLeftValue~
  
get the mostly left item.
  
*/
  Picture* getOuterLeftValue(Picture* outerPicture, Picture* innerPicture ){
    return getXValOfPic(innerPicture) 
    > getXValOfPic(outerPicture) ? innerPicture : outerPicture ;
  }
  

/*
6.6 ~getOuterRightValue~

get teh mostly right item.

*/
  Picture* getOuterRightValue(Picture* outerPicture,  
                            Picture* outerPictureRighCl, Picture* innerPicture )
  {
    Picture* retPicture = getOuterRightValue( outerPicture,  innerPicture ) ;
    return getOuterRightValue(outerPictureRighCl,retPicture) ;
  }
                            
  Picture* getOuterRightValue(Picture* outerPicture, Picture* innerPicture )
  {
    return getXValOfPic(innerPicture) 
    < getXValOfPic(outerPicture) ? 
    innerPicture : outerPicture ;
  }                          
     
/*
6.7 ~setCoordinates~

Set the setCoordinates with the pictrure distance function.
This is necessary for sorting pictures in first dimension.

*/
  void setCoordinates(Picture* _xRef,Picture* _yRef)
     {
       xRef = _xRef;
       xVal = calcDistanz(_xRef,point);
       yVal = calcDistanz(_yRef,point);
       
    };
    
    void setCoordinates(Picture* _xRef,double _yRef)
    {
      xRef = _xRef;
      xVal = calcDistanz(_xRef,point);
      yVal = _yRef;
      
    };
};


}

#endif
