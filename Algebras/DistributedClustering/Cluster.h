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
 
 
 This file contains the implementation of the class dbDacScanAlgebra
 
 2 Includes
 
*/

#ifndef CLUSTER_H_
#define CLUSTER_H_

#include <list>
#include <vector>
#include <utility>  // std::pair, std::make_pair
#include <iostream>
#include <cstdlib>
#include "Member.h"
#include <algorithm>
#include <limits>
#include <cmath>

using namespace std;
namespace distributedClustering{
  

enum Kind {NOISE,CLUSTERCAND, CLUSTER , LEFT, 
  RIGHT, BOTH,CLCANDCLUSTERS, UNDEF};

template <class MEMB_TYP_CLASS>
struct ccMelting {
  unsigned int clusterIndex;
  Kind clusterKind;
  bool meltWithRightCluster;
  MEMB_TYP_CLASS* clusterCandMember;
};

/*
class Cluster
this class represents a list of clusters.

*/


template <class MEMB_TYP_CLASS, class TYPE>
class Cluster
{
private:

/*
Members

*/
  enum MinMaxKind {LEFTMM, RIGHTMM, BOTHMM, GLOBAL, CLCANDCLMM};

  const double MIN_DOUBLE = -1 * numeric_limits<double>::max();
  const double MAX_DOUBLE = numeric_limits<double>::max();

  //clusterTypes
  static const int NOISE_CL_NO = -2 ;
  static const int CLUSTERCAND_CL_NO = -1 ;
  static const int CLUSTER_CL_NO = 1 ;
  static  const int LEFT_CL_NO = 2 ;
  static  const int RIGHT_CL_NO = 3 ;
  static  const int BOTH_CL_NO = 4 ;
  static const int CLCANDCL_CL_NO = 5;
  static const int UNDEF_CLUSTER_CL_NO = 6;

  MEMB_TYP_CLASS* firstElem;
  TYPE* leftOuterPoint ; // of List -> 
                         //corresponds to the right Outer Cluster Point
  TYPE* rightOuterPoint ; // of List -> 
                         //corresponds to the left Outer Cluster Point

  list<MEMB_TYP_CLASS*> noiseList; // final noise points
  list<MEMB_TYP_CLASS*> clusterCandList; 
  // stores points who are not in a cluster yet
  list<MEMB_TYP_CLASS*> emptyList; // emty help list
  vector<list<MEMB_TYP_CLASS*> > clusterArray; //final cluster
  //clusters on right side which are adjacent to the left boundary
  vector<list<MEMB_TYP_CLASS*> > leftPartCluster;
  //clusters on left side which are adjacent to the right boundary
  vector<list<MEMB_TYP_CLASS*> > rightPartCluster; 
  // cluster which are adjacent to both sides boundary
  vector<list<MEMB_TYP_CLASS*> > bothSideCluster;
  vector<list<MEMB_TYP_CLASS*> > undefinedCluster;
  vector<list<MEMB_TYP_CLASS*> > clusterCandClusters;
  vector<list<MEMB_TYP_CLASS*> > emptyVectorList;


  vector<pair <double,double> > clusterMinMaxY;
  vector<pair <double,double> > leftPCMinMaxY;
  vector<pair <double,double> > rightPCMinMaxY;
  vector<pair <double,double> > bothSCMinMaxY;
  vector<pair <double,double> > clusterCandClMinMax;
  vector<pair <double,double> > emptyMinMaxY;

  double eps;
  int minPts ;

  typedef struct ccMelting<MEMB_TYP_CLASS>clusterCandMelt;

public:

/*
constructor

*/
  Cluster( MEMB_TYP_CLASS* leftMember, double _eps, int _minPts);
  Cluster(vector <MEMB_TYP_CLASS*>& members, double _eps, int _minPts);


/*
meltClusters
melt this Cluster with the right cluster
medianPoint and rightMedPoint are the two outer
Points which represent the splitline

*/
  void meltClusters(Cluster * rightCluster,
                   TYPE* leftInnerPoint,
                    TYPE* rightInnerPoint);



/*
getVectorSize
returns the quantity of Clsuters

*/
  unsigned int getVectorSize(Kind kind){
    switch(kind){
      case CLUSTER:
        return clusterArray.size();
      case LEFT:
        return leftPartCluster.size();
      case RIGHT:
        return rightPartCluster.size();
      case BOTH:
        return bothSideCluster.size();
      case CLCANDCLUSTERS:
        return clusterCandClusters.size();
      default:
        return 0;
    }
    return 0;
  }

/*
getClusterVector

*/
  vector<list<MEMB_TYP_CLASS*> >& getClusterVector(Kind kind){

    switch(kind){
      case NOISE:
      case CLUSTERCAND:
        return undefinedCluster;
      case CLUSTER:
        return clusterArray;
      case LEFT:
        return leftPartCluster;
      case RIGHT:
        return rightPartCluster;
      case BOTH:
        return bothSideCluster;
      case CLCANDCLUSTERS:
        return clusterCandClusters;
      case UNDEF:
        return undefinedCluster;
    }
    return undefinedCluster;
  }

private: 

/*
getRightOuterPoint

*/
  TYPE* getRightOuterPoint()
  {
    return rightOuterPoint ;
  }

/*
getListLength

return the list length from position i

*/
  unsigned int getListLength( pair<unsigned int,Kind>& index){
    return getListLength(index.first,index.second);
  }

  unsigned int getListLength(int i,Kind kind){
    switch (kind){
      case NOISE:
        return noiseList.size();
      case CLUSTERCAND:
        return clusterCandList.size();
      case CLUSTER:
        return clusterArray.at(i).size();
      case LEFT:
        return leftPartCluster.at(i).size();
      case RIGHT:
        return rightPartCluster.at(i).size();
      case BOTH:
        return bothSideCluster.at(i).size();
      case CLCANDCLUSTERS:
        return clusterCandClusters.at(i).size();
      case UNDEF:
        return -1;
    }
    return -1;
  }

/*
updateMinMaxVal

compare new point with the value in the pair

*/
  void updateMinMaxVal(Kind kind, int listNo, MEMB_TYP_CLASS *member )
  {
    updateMinMaxVal(getMinMaxFromCluster(kind,listNo),member);
  }

  void updateMinMaxVal(Kind kind, int listNo, pair <double,double> &minMax )
  {
    updateMinMaxVal( kind,  listNo,  minMax.first) ;
    updateMinMaxVal( kind,  listNo,  minMax.second) ;
  }

  void updateMinMaxVal(Kind kind, int listNo, double extrema )
  {
    //    updateGlobalMinMax(GLOBAL,extrema);
    updateMinMaxVal(getMinMaxFromCluster(kind,listNo),extrema);
    //
  }

  void updateMinMaxVal(pair <double,double> &minMax, MEMB_TYP_CLASS *member)
  {
    double newExtrema = member->getYVal();
    updateMinMaxVal(minMax,newExtrema);
  }

  void updateMinMaxVal(pair <double,double> &newMinMax, 
                       pair <double,double> &oldMinMax )
  {
    updateMinMaxVal(newMinMax,oldMinMax.first);
    updateMinMaxVal(newMinMax,oldMinMax.second);
  }

  void updateMinMaxVal(pair <double,double> &minMax, double newExtrema )
  {
    if(newExtrema<MAX_DOUBLE && newExtrema > MIN_DOUBLE){
        if(newExtrema < (minMax.first)){
            minMax.first = newExtrema;
        }
        if(newExtrema > minMax.second){
            minMax.second = newExtrema;
        }
    }
  }



/*
  getNewMinMaxForClusterList
  return a new MinMax Pair  and set MinMax from destination to standardVals
  
*/
  pair <double,double> 
  getNewMinMaxForClusterList(pair <double,double>& srcMinMax,
                             pair <double,double>& destMinMax)
  {
    pair <double,double> retMM(MAX_DOUBLE,MIN_DOUBLE);
    updateMinMaxVal(retMM,destMinMax);
    updateMinMaxVal(retMM, srcMinMax);
    srcMinMax.first =MAX_DOUBLE;
    srcMinMax.second = MIN_DOUBLE;
    return retMM;
  }

  
/*
findNextMinList

returns the nex minimum element from a list

*/
  bool findNextMinList(int& leftIndex, Kind& leftKind,
                       double& leftMinima,double& leftMaxima,
                       bool calcLeftMin, int& leftCnt,
                       int& rightIndex, Kind& rightKind,
                       double& rightMinima,double& rightMaxima,
                       bool calcRightMin, int& rightCnt,
                       Cluster* rightCluster);
  
/*
findNextMinList

find the next List in Y direction from a given minimum point
if no min found return -1
  
*/
  void findNextMinList(int& retIndex, Kind& retKind,
                       double actualMinima, bool rightCluster);
  
/*
  getIndexOfFindNextMinList
  
*/
  int getIndexOfFindNextMinList(double actualMinima, Kind kind)
  {
    int retVal = -1;
    double min = MAX_DOUBLE;
    for (int i =0; i < getVectorSize(kind); i++)
      {
        if(getYMinfromCluster(kind,i) < min
            && getYMinfromCluster(kind,i) > actualMinima){
            min= getYMinfromCluster(kind,i);
            retVal=i;
        }
      }

    return retVal;
  }

/*
  findNextMinListOfClCand
   
   returns the next minimum clustercand
   
*/

  bool findNextMinListOfClCand(vector<pair <double,double> >&
                               minMaxlist,int& index,
                               double& actualMinima,
                               double& actualMaxima,
                               bool& clCandOutOfRangeLeftCl,
                               bool& clCandOutOfRangeRightCl,
                               double actMaxLeftList,
                               double actMaxRightLsit);


/*
getYMinfromCluster

return the min Y value from the appropriate cluster list

*/
  double getYMinfromCluster(pair<unsigned int,Kind>& list)
  {
    return getYMinfromCluster(list.second,list.first);
  }
  double getYMinfromCluster(Kind kind, int i)
  {
    switch (kind){
      case NOISE:
      case CLUSTERCAND:
      case UNDEF:
        return MIN_DOUBLE;
      case CLUSTER:
        return clusterMinMaxY.at(i).first;
      case LEFT:
        return leftPCMinMaxY.at(i).first;
      case RIGHT:
        return rightPCMinMaxY.at(i).first;
      case BOTH:
        return bothSCMinMaxY.at(i).first;
      case CLCANDCLUSTERS:
        return clusterCandClMinMax.at(i).first;
    }
    return MIN_DOUBLE;
  }

/*

getYMinfromCluster

return the min Y value from the appropriate cluster list

*/
  double getYMaxfromCluster(Kind kind, int i)
  {
    switch (kind){
      case NOISE:
      case CLUSTERCAND:
      case UNDEF:
        return MAX_DOUBLE;
      case CLUSTER:
        return clusterMinMaxY.at(i).second;
      case LEFT:
        return leftPCMinMaxY.at(i).second;
      case RIGHT:
        return rightPCMinMaxY.at(i).second;
      case BOTH:
        return bothSCMinMaxY.at(i).second;
      case CLCANDCLUSTERS:
        return clusterCandClMinMax.at(i).second;
    }
    return MAX_DOUBLE;
  }

/*
setNewMin

*/
  void setNewYMin(Kind kind, int i, double val)
  {
    switch (kind){
      case NOISE:
      case CLUSTERCAND:
        break;
      case CLUSTER:
        clusterMinMaxY.at(i).first = val;
        break;
      case LEFT:
        leftPCMinMaxY.at(i).first = val;
        break;
      case RIGHT:
        rightPCMinMaxY.at(i).first = val;
        break;
      case BOTH:
        bothSCMinMaxY.at(i).first = val;
        break;

      default:
        break;
    }
  }

/*
setNewMax

*/
  void setNewYMax(Kind kind, int i, double val)
  {
    switch (kind){
      case NOISE:
      case CLUSTERCAND:
        break;
      case CLUSTER:
        clusterMinMaxY.at(i).first=val;
        break;
      case LEFT:
        leftPCMinMaxY.at(i).second = val;
        break;
      case RIGHT:
        rightPCMinMaxY.at(i).second = val;
        break;
      case BOTH:
        bothSCMinMaxY.at(i).second = val;
        break;
      default:
        break;
    }
  }


/*
pushMemberToClusterList

pushes a Member to list from a cluster-Vector

*/
  pair <double,double>&
  getMinMaxFromCluster(int i,Kind kind)
  {
    return getMinMaxFromCluster(kind,  i);
  }

  pair <double,double>&
  getMinMaxFromCluster(pair<unsigned int,Kind>& index)
  {
    return getMinMaxFromCluster(index.first,index.second);
  }

  pair <double,double>&
  getMinMaxFromCluster(Kind kind, int i)
  {
    switch (kind)
    {
      case NOISE:
      case CLUSTERCAND:
      case UNDEF:
        return emptyMinMaxY.at(0);
      case CLUSTER:
        return clusterMinMaxY.at(i);
      case LEFT:
        return leftPCMinMaxY.at(i);
      case RIGHT:
        return rightPCMinMaxY.at(i);
      case BOTH:
        return bothSCMinMaxY.at(i);
      case CLCANDCLUSTERS:
        return clusterCandClMinMax.at(i);
    }
    return emptyMinMaxY.at(0);
  }

  unsigned int getMinMaxSize(Kind kind)
  {
    switch (kind)
    {
      case NOISE:
      case CLUSTERCAND:
      case UNDEF:
        return -1;
      case CLUSTER:
        return clusterMinMaxY.size();
      case LEFT:
        return leftPCMinMaxY.size();
      case RIGHT:
        return rightPCMinMaxY.size();
      case BOTH:
        return bothSCMinMaxY.size();
      case CLCANDCLUSTERS:
        return clusterCandClMinMax.size();
    }
    return -1;
  }
  
/*
getMinMaxFromCluster

*/
  vector<pair <double,double> >&
  getMinMaxVector(Kind kind)
  {
    switch (kind){
      case NOISE:
      case CLUSTERCAND:
        return emptyMinMaxY;
      case CLUSTER:
        return clusterMinMaxY;
      case LEFT:
        return leftPCMinMaxY;
      case RIGHT:
        return rightPCMinMaxY;
      case BOTH:
        return bothSCMinMaxY;
      case CLCANDCLUSTERS:
        return clusterCandClMinMax;
      default:
        return emptyMinMaxY;
    }
    return emptyMinMaxY;

  }


/*
  getList
  return the clusterList on position i

*/
  list<MEMB_TYP_CLASS*> &getList(pair<unsigned int,Kind>& list){
    return getList(list.first,list.second);
  }
  list<MEMB_TYP_CLASS*> &getList(int i, Kind kind){
    switch (kind){
      case NOISE:
        return noiseList;
      case CLUSTERCAND:
        return clusterCandList;
      case CLUSTER:
        return clusterArray.at(i);
      case LEFT:
        return leftPartCluster.at(i);
      case RIGHT:
        return rightPartCluster.at(i);
      case BOTH:
        return bothSideCluster.at(i);
      case CLCANDCLUSTERS:
        return clusterCandClusters.at(i);
      default:
        return emptyList;
    }
    return emptyList;
  }

/*
insertElement
insert a member at given position i

*/
  void insertElement(typename list<MEMB_TYP_CLASS*>::iterator it,
                     MEMB_TYP_CLASS* memb, int i, Kind kind)
  {
    switch (kind){
      case NOISE:
        noiseList.insert(it,memb);
        break;
      case CLUSTERCAND:
        clusterCandList.insert(it,memb);
        break;
      case CLUSTER:
        clusterArray.at(i).insert(it,memb);
        break;
      case LEFT:
        leftPartCluster.at(i).insert(it,memb);
        break;
      case RIGHT:
        rightPartCluster.at(i).insert(it,memb);
        break;
      case BOTH:
        bothSideCluster.at(i).insert(it,memb);
        break;
      case CLCANDCLUSTERS:
        clusterCandClusters.at(i).insert(it,memb);
        break;

      default:
        break;
    }
  }

  void insertList(typename vector<list<MEMB_TYP_CLASS*> >::iterator it,
                  list<MEMB_TYP_CLASS*>& list,Kind kind)
  {
    switch (kind){
      case NOISE:
      case CLUSTERCAND:
      case UNDEF:
        break;
      case CLUSTER:
        clusterArray.insert(it,list);
        break;
      case LEFT:
        leftPartCluster.insert(it,list);
        break;
      case RIGHT:
        rightPartCluster.insert(it,list);
        break;
      case BOTH:
        bothSideCluster.insert(it,list);
        break;
      case CLCANDCLUSTERS:
        clusterCandClusters.insert(it,list);
        break;
    }
  }

  void insertList(unsigned int listNo,
                  list<MEMB_TYP_CLASS*>& list,Kind kind)
  {
    switch (kind){
      case NOISE:
      case CLUSTERCAND:
      case UNDEF:
        break;
      case CLUSTER:
        clusterArray.insert(clusterArray.begin() + listNo,list);
        break;
      case LEFT:
        leftPartCluster.insert(leftPartCluster.begin() + listNo,list);
        break;
      case RIGHT:
        rightPartCluster.insert(rightPartCluster.begin() + listNo,list);
        break;
      case BOTH:
        bothSideCluster.insert(bothSideCluster.begin() + listNo,list);
        break;
      case CLCANDCLUSTERS:
        clusterCandClusters.insert(clusterCandClusters.begin() + listNo,list);
        break;
    }
  }


/*
insertMinMax

*/
   void insertMinMax(typename vector<pair<double,double> >::iterator it,
                     pair<double,double>& list,Kind kind)
   {
     switch (kind){
       case NOISE:
       case CLUSTERCAND:
       case UNDEF:
         break;
       case CLUSTER:
         clusterMinMaxY.insert(it,list);
         break;
       case LEFT:
         leftPCMinMaxY.insert(it,list);
         break;
       case RIGHT:
         rightPCMinMaxY.insert(it,list);
         break;
       case BOTH:
         bothSCMinMaxY.insert(it,list);
         break;
       case CLCANDCLUSTERS:
         clusterCandClMinMax.insert(it,list);
         break;
     }
   }

/*
insertMinMax

*/
   void insertMinMax(int i,
                     pair<double,double>& list,Kind kind)
   {
     switch (kind){
       case NOISE:
       case CLUSTERCAND:
       case UNDEF:
         break;
       case CLUSTER:
         clusterMinMaxY.insert(clusterMinMaxY.begin() + i,list);
         break;
       case LEFT:
         leftPCMinMaxY.insert(leftPCMinMaxY.begin() + i,list);
         break;
       case RIGHT:
         rightPCMinMaxY.insert(rightPCMinMaxY.begin() + i,list);
         break;
       case BOTH:
         bothSCMinMaxY.insert(bothSCMinMaxY.begin() +  i,list);
         break;
       case CLCANDCLUSTERS:
         clusterCandClMinMax.insert(clusterCandClMinMax.begin() +  i,list);
         break;
     }
   }

/*
getIterator
returns an iterator from a clusterlist or 
noiselist either at the beginning or at end

*/
   typename list<MEMB_TYP_CLASS*>::iterator 
   getIterator(int list, bool begin, Kind kind)
   {
     switch (kind){

       case NOISE:
         if (begin)
           return noiseList.begin();
         else
           return noiseList.end();
         break;
       case CLUSTERCAND:
         if (begin)
           return clusterCandList.begin();
         else
           return clusterCandList.end();
         break;
       case CLUSTER:
         if (begin)
           return clusterArray.at(list).begin();
         else
           return clusterArray.at(list).end();
         break;
       case LEFT:
         if (begin)
           return leftPartCluster.at(list).begin();
         else
           return leftPartCluster.at(list).end();
         break;
       case RIGHT:
         if (begin)
           return rightPartCluster.at(list).begin();
         else
           return rightPartCluster.at(list).end();
         break;
       case BOTH:
         if (begin)
           return bothSideCluster.at(list).begin();
         else
           return bothSideCluster.at(list).end();
         break;
       case CLCANDCLUSTERS:
         if (begin)
           return clusterCandClusters.at(list).begin();
         else
           return clusterCandClusters.at(list).end();

       default:
         return emptyList.begin();
     }
     return emptyList.begin();
   }

/*
 eraseItem
 delete an member item either at the beginning or at end

*/
   typename list<MEMB_TYP_CLASS*>::
   iterator eraseItem(int list,
                      typename std::list<MEMB_TYP_CLASS*>::iterator it,
                      Kind kind)
   {
     switch (kind){

       case NOISE:
         return noiseList.erase(it);
       case CLUSTERCAND:
         return clusterCandList.erase(it);
       case CLUSTER:
         return clusterArray.at(list).erase(it);
       case LEFT:
         return leftPartCluster.at(list).erase(it);
       case RIGHT:
         return rightPartCluster.at(list).erase(it);
       case BOTH:
         return bothSideCluster.at(list).erase(it);
       case CLCANDCLUSTERS:
         return clusterCandClusters.at(list).erase(it);

       default:
         return emptyList.begin();
     }
     return emptyList.begin();
   }


/*
eraseList

*/
   void eraseList(Kind kind, unsigned int list)
   {
     switch (kind){
       case NOISE:
       case CLUSTERCAND:
         break;
       case CLUSTER:
         clusterArray.erase(clusterArray.begin()+list);
         break;
       case LEFT:
         leftPartCluster.erase(leftPartCluster.begin()+list);
         break;
       case RIGHT:
         rightPartCluster.erase(rightPartCluster.begin()+list);
         break;
       case BOTH:
         bothSideCluster.erase(bothSideCluster.begin()+list);
         break;
       case CLCANDCLUSTERS:
         clusterCandClusters.erase(clusterCandClusters.begin() + list);
         break;
       default:
         break;
     }
   }

/*
eraseMinMax

*/
   void eraseMinMax(Kind kind, unsigned int list)
   {
     switch (kind){
       case NOISE:
       case CLUSTERCAND:
       case CLUSTER:
         clusterMinMaxY.erase(clusterMinMaxY.begin()+list);
         break;
       case LEFT:
         leftPCMinMaxY.erase(leftPCMinMaxY.begin()+list);
         break;
       case RIGHT:
         rightPCMinMaxY.erase(rightPCMinMaxY.begin()+list);
         break;
       case BOTH:
         bothSCMinMaxY.erase(bothSCMinMaxY.begin()+list);
         break;
       case CLCANDCLUSTERS:
         clusterCandClMinMax.erase(clusterCandClMinMax.begin() + list);
         break;
       default:
         break;
     }
   }

/*
clearList

*/
   void clearList(Kind kind, unsigned int list)
   {
     switch (kind){
       case NOISE:
         noiseList.clear();
         break;
       case CLUSTERCAND:
         clusterCandList.clear();
         break;
       case CLUSTER:
         clusterArray.at(list).clear();
         break;
       case LEFT:
         leftPartCluster.at(list).clear();
         break;
       case RIGHT:
         rightPartCluster.at(list).clear();
         break;
       case BOTH:
         bothSideCluster.at(list).clear();
         break;
       case CLCANDCLUSTERS:
         clusterCandClusters.at(list).clear();
         break;
       default:
         break;
     }
   }

/*
pushMemberToClusterList

push a committed member at the end or beginning of a given clusterlist

*/
   void pushMemberToClusterList(bool front,MEMB_TYP_CLASS *member,
                                int list, Kind kind);

/*
pushListToCluster

*/
   void pushListToCluster(Kind kind,list<MEMB_TYP_CLASS *>& list)
   {
     switch (kind)
     {
       case NOISE:
         break;
       case CLUSTERCAND:
         break;
       case CLUSTER:
         clusterArray.push_back(list);
         break;
       case LEFT:
         leftPartCluster.push_back(list);
         break;
       case RIGHT:
         rightPartCluster.push_back(list);
         break;
       case BOTH:
         bothSideCluster.push_back(list);
         break;
       case CLCANDCLUSTERS:
         clusterCandClusters.push_back(list);
         break;
       default:
         break;
     }
   }

/*
pushListToCluster

*/
   void pushMinMaxToCluster(Kind kind,pair<double,double>& list)
   {
     switch (kind){
       case NOISE:
         break;
       case CLUSTERCAND:
         break;
       case CLUSTER:
         clusterMinMaxY.push_back(list);
         break;
       case LEFT:
         leftPCMinMaxY.push_back(list);
         break;
       case RIGHT:
         rightPCMinMaxY.push_back(list);
         break;
       case BOTH:
         bothSCMinMaxY.push_back(list);
         break;
       case CLCANDCLUSTERS:
         clusterCandClMinMax.push_back(list);
         break;
       default:
         break;
     }

   }

/*
moveItemUnsorted

moves item unsorted from srclist to destlist mostly 
used for noise and clustercand list
returns the Iterator from list where item was deleted

*/
   typename list<MEMB_TYP_CLASS*>::iterator
   moveItemUnsorted(
       typename list<MEMB_TYP_CLASS*>::iterator delIt,
       list<MEMB_TYP_CLASS*>& srcList,
       list<MEMB_TYP_CLASS*>& destList, int clusterNo, int clusterType)
   {
     destList.push_back((*delIt));
     (*delIt)->setClusterNo(clusterNo);
     (*delIt)->setClusterType(clusterType);
     return  srcList.erase(delIt);
   }

/*
moveItemSorted
moves item sorted from eraselist to pushlist list

*/
   typename list<MEMB_TYP_CLASS*>::iterator
   moveItemSorted(Cluster* updateCluster,
                  typename list<MEMB_TYP_CLASS*>::iterator delIt,
                  list<MEMB_TYP_CLASS*>& srcList,
                  list<MEMB_TYP_CLASS*>& destList,
                  bool searchAndDelete, int clusterNo,
                  int clusterType, bool updateMinMax,
                  bool& moveWorked)
   {
     typename list<MEMB_TYP_CLASS*>::iterator destListIt = destList.begin(),
         retIt;
     //search position to insert
     //first go back in list until listPoint<neighborPoint
     if(destListIt != destList.end()){ //else list is empty
         //then serch position to insert
         while(destListIt != destList.end() &&
             (*destListIt)->getXVal() >= (*delIt)->getXVal()){
             destListIt++;
         }
     }

     if(clusterType >= 0 && updateMinMax)
       {//update Y Borders
         updateCluster->
         updateMinMaxVal(getClusterKindFromType(clusterType),
                          getListNoOfClusterNo(clusterNo,
                                          getClusterKindFromType(clusterType)),
                          *delIt);
       }

     if(searchAndDelete){
         retIt = searchAndDeletItemFromList(delIt,srcList,moveWorked);
     }else{
         retIt = srcList.erase(delIt);
         moveWorked = true;
     }

     if(moveWorked){
         destList.insert(destListIt,*delIt);
         (*delIt)->setClusterNo(clusterNo);
         (*delIt)->setClusterType(clusterType);
     }

     return retIt;
   }

/*

moveNeighborsToDestList
verschiebt alle moeglichen Nachbarn eines Members in die Zielliste

*/
   void moveNeighborsToDestList(Cluster* rightClsuter,
       pair <double,double>& clusterMinMax,
       MEMB_TYP_CLASS *member,
       list<MEMB_TYP_CLASS*>& srcList,
       list<MEMB_TYP_CLASS*>& otherSrcList,
       list<MEMB_TYP_CLASS*>& destList,
       bool searchAndDelete, //used for moveItemSorted
       int clusterNo, int clusterType)
   {
     typename list<MEMB_TYP_CLASS*>::iterator
     membNeighborIt = member->getEpsNeighborhood(true);

     while(membNeighborIt != member->getEpsNeighborhood(false)){
         if(!(*membNeighborIt)->isClusterMember()){ //point is not in cluster

             bool moveItem = false;

             typename list<MEMB_TYP_CLASS*>::iterator testIt=
                 moveItemSorted(this,membNeighborIt,srcList,destList,
                                searchAndDelete,clusterNo,
                                clusterType,false,moveItem);
             if(!moveItem){
                 testIt = moveItemSorted(this,membNeighborIt,
                                         otherSrcList,destList,
                                         searchAndDelete,clusterNo,
                                         clusterType,false,moveItem);
             }

             if(!moveItem && rightClsuter != 0){ 
                 testIt = moveItemSorted(this,membNeighborIt,
                                         getList(0,NOISE),destList,
                                         searchAndDelete,clusterNo,
                                         clusterType,false,moveItem);
             }
             if(!moveItem && rightClsuter != 0){
                 testIt = moveItemSorted(this,membNeighborIt,
                                         rightClsuter->getList(0,NOISE),
                                         destList, searchAndDelete,clusterNo,
                                         clusterType,false,moveItem);
             }
             updateMinMaxVal(clusterMinMax,*membNeighborIt);
         }
         membNeighborIt++;
     }
   }


/*
moveClusterList
move a list from origIndex to destIndex
only usable for left lists

*/
   void moveClusterList(pair<unsigned int,Kind>& destIndex,
                        pair<unsigned int,Kind>& origIndex,
                        Cluster* origCluster)
   {

     bool pushBackToDesInd =
         destIndex.first == getVectorSize(destIndex.second);

     if(pushBackToDesInd)
       {
         list<MEMB_TYP_CLASS*> emptyList;
         //push original list back to dest List
         pushListToCluster(destIndex.second,
                           origCluster->getList(origIndex.first,
                                                origIndex.second));
         //remove list from original vector and insert empty list
         origCluster->eraseList(origIndex.second,
                                origIndex.first);
         origCluster->insertList(origIndex.first,
                                 emptyList,
                                 origIndex.second);
       }else { // swap element with pos i
           getList(destIndex.first,
                   destIndex.second).swap(
                       origCluster->getList(origIndex.first,
                                            origIndex.second));
       }
   }

   void moveMinMaxPair(pair<unsigned int,Kind>& destIndex,
                       pair<unsigned int,Kind>& origIndex,
                       Cluster* origCluster)
   {
     bool pushBackToDesInd =
         destIndex.first == getMinMaxSize(destIndex.second);

     if(pushBackToDesInd)
       {
         pair<double,double> minMaxInit = make_pair(MAX_DOUBLE,MIN_DOUBLE);
         //push original list back to dest List
         pushMinMaxToCluster(destIndex.second,
                             origCluster->getMinMaxFromCluster(origIndex));
         //remove list from original vector and insert empty list
         origCluster->eraseMinMax(origIndex.second,
                                  origIndex.first);
         origCluster->insertMinMax(origIndex.first,
                                   minMaxInit,
                                   origIndex.second);

       }else { // swap element with pos i
           swap(getMinMaxFromCluster(destIndex),
                origCluster->getMinMaxFromCluster(origIndex));
       }
   }

/*
findClusterCands
with given clusterList - search more clusterCands
returns true if more clustercands are added
member werden von clusterCandList  zu foundMembList hinzugefuegt

*/
   bool findClusterCandsforClusterList(list<MEMB_TYP_CLASS*>& clusterlist,
                                       pair<double,double>& clusterMinMax,
                                       bool clusterIsRight,
                                       TYPE* borderPoint,
                                       //if richtCluster take leftInnerPoint
                                       TYPE* secBorderPoint,
                                       list<MEMB_TYP_CLASS*>& clusterCandList,
                                       list<MEMB_TYP_CLASS*>& foundMembList,
                                       pair<double,double>& foundMinMax ,
                                       int clusterNo)
   {
     bool elementsfound = false, endOfClusterList = false;

     typename list<MEMB_TYP_CLASS*>::iterator
     clusterListIt = clusterlist.begin(),
     clusterCandIt = clusterCandList.begin();

     if(clusterListIt != clusterlist.end() &&
         clusterCandIt != clusterCandList.end()){

         if(!clusterIsRight)
           {//begin with list end because points are sorted descending
             clusterListIt = --clusterlist.end();
           }

         while (!endOfClusterList &&
             (*clusterListIt)->calcXDistanz(borderPoint) <= eps)
           {
             while (clusterCandIt != clusterCandList.end())
               {
                 if((*clusterListIt)->calcDistanz(*clusterCandIt) <= eps)
                   {
                     elementsfound=true;
                     //update Neighbor
                     if(clusterIsRight){
                         updateNeighborLeftPointToRightList(
                             *clusterCandIt,
                             clusterlist,
                             clusterListIt,borderPoint,
                             secBorderPoint);
                     }else{
                         updateNeighborRightPointToLeftList(
                             *clusterCandIt,
                             clusterlist,
                             clusterListIt,borderPoint,
                             secBorderPoint);
                     }

                     moveItemSorted(clusterCandIt,clusterCandList,
                                    foundMembList,false,clusterNo,false);
                     updateMinMaxVal(foundMinMax,*clusterCandIt);
                     //start from beginning
                     clusterCandIt = clusterCandList.begin();
                   } else{
                       clusterCandIt++;
                   }
               }
             if(clusterIsRight){
                 clusterListIt++;
                 if(clusterListIt == clusterlist.end())
                   endOfClusterList = true;
             }else{
                 clusterListIt--;
                 if(clusterListIt == --clusterlist.begin())
                   endOfClusterList = true;
             }
           }
     }
     return elementsfound;
   }

/*
searchClusterListToMelt
suche zur linken lister rechte cluster die miteinander verschmolzen werden

*/
   bool  searchClusterListToMelt(list<MEMB_TYP_CLASS*>& leftList,
                                 pair<double,double>& leftMinMax,
                                 Cluster* rightCluster,int leftListNo,
                                 Kind rightKind,
                                 TYPE* leftInnerPoint,
                                 //if richtCluster take leftInnerPoint
                                 TYPE* rightInnerPoint,
                                 vector<unsigned int> *clusterToMeltRight,
                                 vector<unsigned int> *clusterToMeltLeft)
   {
     typename list<MEMB_TYP_CLASS*>::iterator
     itLeft = --leftList.end(),
     itRight;

     bool jAlreadyAppend = false,memberAdded = false,
         newClusterFoundInL = false;

     if(itLeft != --leftList.begin() &&
         (*itLeft)->calcXDistanz(rightInnerPoint) <=eps)
       { //check outer point distance - if bigger than eps - take next list
         while(itLeft != --leftList.begin() &&
             (*itLeft)->calcXDistanz(rightInnerPoint) <=eps)
           {
             //compare with each cluster on the right side
             for(unsigned int j=0;j<rightCluster->getVectorSize(rightKind) ;j++)
               {
                 if(
                     listIsInYBordersOfList(leftMinMax,
                                            rightCluster,j,rightKind))
                   {
                     bool jAlreadyAppend = false;
                     vector<unsigned int>::iterator it =
                         clusterToMeltRight[leftListNo].begin();
                     while(!jAlreadyAppend && it !=
                         clusterToMeltRight[leftListNo].end()){
                         if(j==*it){
                             jAlreadyAppend = true;
                         }
                         it++;
                     }
                     if(!jAlreadyAppend){
                         newClusterFoundInL = false;
                         // get this most left point from right Cluster
                         itRight=rightCluster->getIterator(j,true,rightKind);

                         if(!newClusterFoundInL &&
                             itRight !=
                             rightCluster->getIterator(j,false,rightKind) &&
                             (*itRight)->calcXDistanz(leftInnerPoint) <=eps)
                           {//check outer point distance -
                             //if bigger than eps->take next list
                             while(!newClusterFoundInL &&
                                 itRight != 
                                 rightCluster->getIterator(j,false,rightKind) &&
                                 (*itRight)->
                                 calcXDistanz(leftInnerPoint) <= eps)
                               {//get distance from left Point to right Point

                                 if((*itLeft)->calcDistanz(*itRight) <= eps){
                                     memberAdded=true;
                                     newClusterFoundInL = true;
                                     updateNeighborRightListToLeftList(
                                         leftList,
                                         rightCluster->getList(j,rightKind),
                                         true,true,
                                         leftInnerPoint,rightInnerPoint);
                                     if((*itLeft)->updateInnerPnt(minPts) &&
                                         (*itRight)->updateInnerPnt(minPts))
                                       {
                                         //remember cluster listNo
                                         clusterToMeltRight[
                                                    leftListNo].push_back(j);
                                         clusterToMeltLeft[j].
                                                        push_back(leftListNo);
                                       }
                                 }
                                 ++itRight;
                               }
                           } 
                        //else newClusterFound || itRight==end || distance > eps
                     }// else jAlreadyAppend
                   }//else list is not in Y Border of Cluster
               }
             --itLeft;
           }
       }
     return memberAdded;
   }

/*
meltClusterLists
melt lists of Cluster at one side Cluster. So all  list 
which are stored in the meltingSideArray Index where melted.

*/
   void meltClusterLists(Cluster *meltingCluster,
                         vector<pair<unsigned int,Kind> > *meltingSideArray,
                         vector<unsigned int> minToMaxIndexes,
                         vector<pair<unsigned int,Kind> > * indexArray,
                         vector<pair<unsigned int,Kind> >& newIndices, 
                         unsigned int bothDist)
   {
     //if kind is both then meltClusterCandListWithClusterList[bothDist + i]
     vector<unsigned int>::iterator leftIt = minToMaxIndexes.end();
     leftIt--;
     typename vector<pair<unsigned int,Kind> >::iterator destIt,srcIt;

     while(leftIt != --minToMaxIndexes.begin())
       {
         if(meltingSideArray[*leftIt].size()>1){

             destIt = meltingSideArray[*leftIt].begin();
             srcIt = --meltingSideArray[*leftIt].end();

             while(destIt != srcIt){

                 if((*destIt) != (*srcIt)){
                     meltListsAndIndexOfCluster(meltingCluster,indexArray,
                                                *destIt,*srcIt,
                                                newIndices,bothDist);
                 }

                 srcIt--;
             }
         }
         leftIt--;
       }
   }

/*

meltListsAndIndexOfCluster

*/
   void meltListsAndIndexOfCluster(Cluster* meltingCluster,
                                   vector<pair<unsigned int,Kind> >*
                                   indexArray,
                                   pair<unsigned int,Kind>& destIndex,
                                   pair<unsigned int,Kind>& srcIndex,
                                   vector<pair<unsigned int,Kind> >& 
                                   newIndices,
                                   unsigned int& bothDist) 
   {
     //find out correct index
     int destInd, srcInd;

     destInd = getCorrectListIndex(destIndex.first,
                                   bothDist,
                                   destIndex.second);

     srcInd = getCorrectListIndex(srcIndex.first,
                                  bothDist,
                                  srcIndex.second);

     //melt Clusters and update minMaxY
     pair<unsigned int,Kind> newIndex=
         meltingCluster->meltListsOfCluster(destIndex,srcIndex,newIndices);
     //set new index
     newIndices[destInd]= newIndex;
     newIndices[srcInd]= newIndex;
     //melt indexarrays only if destInd != srcInd
     meltIndexOfCluster(
         indexArray[destInd],
         indexArray[srcInd]);
   }

/*

initIndicies

initialice newIndicies Left and Right with respectivly the first entry of
clusterToMelt indicies

*/
void initIndicies( vector<pair<unsigned int,Kind> >& newIndices,
                     int indexSize,int bothDist, bool isRightCluster)
   {
     for(int i = 0; i<indexSize; i++){
         int ind; Kind kind;
         if(i < bothDist)
           {
             ind = i;
             if(isRightCluster){
                 kind = LEFT;
             }else{
                 kind = RIGHT;
             }
           } else{
               ind = i - bothDist;
               kind = BOTH;
           }
         newIndices.push_back(
             make_pair(ind,kind));

     }
   }

/*

meltIndexOfCluster
melt the given clusterindexes

*/
   void meltIndexOfCluster(vector<pair<unsigned int,Kind> > &destIndList ,
                           vector<pair<unsigned int,Kind> > &sourceIndList);

/*
meltListsOfCluster

melt two list of a cluster

*/
   pair<unsigned int,Kind>
   meltListsOfCluster(pair<unsigned int,Kind>& destinationList,
                      pair<unsigned int,Kind>& sourceList,
                      vector<pair<unsigned int,Kind> >& newIndicies
                     );

   void meltClusterCandListWithClusterList(pair<unsigned int,Kind>& 
                                           destinationList,
                                           list<MEMB_TYP_CLASS*> &sourceIndList,
                                           pair <double,double>& minMaxList);

/*
findClListToMeltWithClustCandList

*/
   void findClListToMeltWithClustCandList(Cluster * rightCluster,
                                      vector<clusterCandMelt>& clCaMeltInd,
                                      list<MEMB_TYP_CLASS*>& clusterCandList,
                                      pair <double,double>& clCandMinMax,
                                      int bothDistLeft,
                                      vector<pair<unsigned int,Kind> >&
                                      newLeftIndices,
                                      int bothDistRight,
                                      vector<pair<unsigned int,Kind> >&
                                      newRightIndices)
   {
     typename vector< clusterCandMelt>::iterator
     membIt = clCaMeltInd.begin(),
     helpIt = clCaMeltInd.begin();
     bool leftMelted = false;
     //Try to melt with list on left side -> thus saves you a copy operation
     while(!leftMelted &&
         helpIt != clCaMeltInd.end())
       {
         if(!(*helpIt).meltWithRightCluster)
           {//melt with left clusterlist
             meltClusterCandListWithClusterList((*helpIt),
                                                clusterCandList,clCandMinMax,
                                                bothDistLeft,
                                                newLeftIndices);
             leftMelted=true;
           }
         helpIt++;
       }

     if(!leftMelted)
       {
         if((*membIt).meltWithRightCluster)
           {
             //melt with first element
             rightCluster->
             meltClusterCandListWithClusterList((*membIt),
                                                clusterCandList,clCandMinMax,
                                                bothDistRight,
                                                newRightIndices);
           } else { //Normally not used
               meltClusterCandListWithClusterList((*membIt),
                                                  clusterCandList,clCandMinMax,
                                                  bothDistLeft,
                                                  newLeftIndices);
           }
       }
   }

/*
 meltClusterCandListWithClusterList
first find the index to melt the given 
clusterList which is stored in clCaMeltInd
melt the lists

*/
   void meltClusterCandListWithClusterList(clusterCandMelt& clCaMeltInd,
                                        list<MEMB_TYP_CLASS*>& clusterCandList,
                                        pair <double,double>& clCandMinMax,
                                        int bothDist,
                                        vector<pair<unsigned int,Kind> >&
                                        newIndices)
   {
     pair<unsigned int,Kind> clusterList =
         make_pair(clCaMeltInd.clusterIndex,
                   clCaMeltInd.clusterKind);
     // set clusterList with correct indices to melt
     int index =
         findListToMeltWithClusterCand(clCaMeltInd,
                                       clusterList,
                                       bothDist,newIndices);
     if(index > -1)
       {
         meltClusterCandListWithClusterList(clusterList,
                                            clusterCandList,
                                            clCandMinMax);
       }else{
//            cout << "in meltClusterCandListWithClusterList" << endl;
//            cout << "FAIL member not found" << endl;
       }
   }


/*

meltClsuterCandWithClusterList

melt foundet reachabel clusterCands with cluster lists

*/
   void meltClsuterCandWithClusterList(Cluster* rightCluster,
                                    vector< clusterCandMelt>* clusterCandIndex,
                                    unsigned int indexSize,
                                    vector<pair<unsigned int,Kind> > *
                                    clusterToMeltOnRightForLeftSide,
                                    vector<pair<unsigned int,Kind> >&
                                    newLeftIndices,
                                    int bothDistLeft,
                                    vector<pair<unsigned int,Kind> > *
                                    clusterToMeltOnLeftForRightSide,
                                    vector<pair<unsigned int,Kind> >&
                                    newRightIndices,
                                    int bothDistRight)
   {
     
     for(unsigned int i = 0; i< indexSize; i++){
         if(clusterCandIndex[i].size() > 0)
           {
             //add item to first clusterList
             list<MEMB_TYP_CLASS*> clusterCandList;
             clusterCandList.push_back(
               clusterCandIndex[i].at(0).clusterCandMember);
             double minMax = 
             clusterCandIndex[i].at(0).clusterCandMember->getYVal();
             pair <double,double> clCandMinMax = make_pair(minMax,minMax);
             //melt list with first entry -> search entry
             findClListToMeltWithClustCandList(rightCluster,
                                               clusterCandIndex[i],
                                               clusterCandList,
                                               clCandMinMax,
                                               bothDistLeft,newLeftIndices,
                                               bothDistRight,newRightIndices);

             bool membDeleted = false;
             if(clusterCandIndex[i].at(0).meltWithRightCluster)
               {
                 searchAndDeletItemFromList(
                   clusterCandIndex[i].at(0).clusterCandMember,
                                            getList(0,CLUSTERCAND),
                                            membDeleted);

               } else {
                 searchAndDeletItemFromList(
                   clusterCandIndex[i].at(0).clusterCandMember,
                                          rightCluster->getList(0,CLUSTERCAND),
                                            membDeleted);
               }

             clusterCandList.clear();
             if(clusterCandIndex[i].size() > 1)
               {
                 //update MeltingIndexes - 
                 //and melt cluster lists if it is necessary
                 updateMeltedCluster(rightCluster,
                                     clusterCandIndex[i],
                                     clusterToMeltOnRightForLeftSide,
                                     bothDistLeft,
                                     newLeftIndices,
                                     clusterToMeltOnLeftForRightSide,
                                     bothDistRight,
                                     newRightIndices);
               }
           }
     }
   }


/*
findListToMeltWithClusterCand

*/
   int
   findListToMeltWithClusterCand(clusterCandMelt& clCaMeltInd,
                                 pair<unsigned int,Kind>& clusterList,
                                 int bothDist,
                                 vector<pair<unsigned int,Kind> >& newIndices )
   {

     int foundIndex = -1;
     //test list length
     if(getListLength(clCaMeltInd.clusterIndex,
                      clCaMeltInd.clusterKind) > 0)
       {//listlenght is > 0 -> so this is the 
         //index where the other lists are melted
         //with clusterlist is nothing to do
         foundIndex = clCaMeltInd.clusterIndex;
       }else { //find entry in clusterToMeltIndex
           foundIndex =
               findLastIndex(clusterList,newIndices,bothDist);
       }
     return foundIndex;
   }

/*
getCorrectListIndex
 auxilary function to get corect Index regarding BothDist

*/
   int getCorrectListIndex(int listIndex, int bothDist, Kind kind){
     if(kind== BOTH)
       {
         return listIndex + bothDist;
       }else{
           return listIndex;
       }
   }

/*
findLastIndex
finds the last Index in newIndices
preconditions: clusterList must be initialized

*/
   int findLastIndex(pair<unsigned int,Kind>& clusterList,
                     vector<pair<unsigned int,Kind> >& newIndices,
                     int bothDist)
   {
     bool memberExist = false;
     int foundIndex = -1;
     unsigned int listIndex = clusterList.first;
     //find out correct Index regarding bothDist
     unsigned int helpInd =
         getCorrectListIndex( clusterList.first,
                              bothDist,
                              clusterList.second);
     //find melted list in which the given list is stored
     while(!memberExist)
       {
         if(listIndex != newIndices[helpInd].first)
           {
             listIndex=newIndices[helpInd].first;
             helpInd =
                 getCorrectListIndex(listIndex,
                                     bothDist,
                                     newIndices[helpInd].second);
           }else{
             memberExist = true; 
             if(helpInd < getVectorSize(newIndices[helpInd].second)){
               clusterList.first = newIndices[helpInd].first;
               clusterList.second = newIndices[helpInd].second;
               foundIndex = helpInd;
             }
           }
       }
     return foundIndex;
   }

/*
updateClusterToMelt
used for clustercands
melt all cluster list which are not melted yet

*/
   void updateMeltedCluster(Cluster * rightCluster,
                            vector<clusterCandMelt>& clCaMeltInd,
                            vector<pair<unsigned int,Kind> >*
                            clusterToMeltOnRightForLeftSide,
                            int bothDistLeft,
                            vector<pair<unsigned int,Kind> >& newLeftIndices,
                            vector<pair<unsigned int,Kind> >*
                            clusterToMeltOnLeftForRightSide,
                            int bothDistRight,
                            vector<pair<unsigned int,Kind> >& newRightIndices)
   {
     typename vector< clusterCandMelt>::iterator
     membIt = clCaMeltInd.begin();

     vector<clusterCandMelt> leftMembers;
     vector<clusterCandMelt> rightMembers;
     while(membIt != clCaMeltInd.end())
       {
         if((*membIt).meltWithRightCluster)
           {
             rightMembers.push_back((*membIt));
           }else{
               leftMembers.push_back((*membIt));
           }
         membIt++;
       }

     typename vector< clusterCandMelt>::iterator
     rightIt = rightMembers.begin(),
     leftIt = leftMembers.begin();
     if(rightMembers.size()>0)
       {
         meltClusterCandClusterWithList(rightCluster,
                                        rightMembers,
                                        newRightIndices,
                                        clusterToMeltOnLeftForRightSide,
                                        newRightIndices,
                                        bothDistRight);
       }
     if(leftMembers.size()>0)
       {
         meltClusterCandClusterWithList(this,
                                        leftMembers,
                                        newLeftIndices,
                                        clusterToMeltOnRightForLeftSide,
                                        newLeftIndices,
                                        bothDistLeft);
       }
   }

/*
memberHasSameLastIndex
compare a clusterCand member list if the indexes have the same
last index. If the result is true then the lists are melted

*/
   bool meltClusterCandClusterWithList(Cluster* meltingCluster,
                                  vector<clusterCandMelt>& members,
                                  vector<pair<unsigned int,Kind> >& srcIndices,
                                  vector<pair<unsigned int,Kind> > * indexArray,
                                  vector<pair<unsigned int,Kind> >& meltIndices,
                                  unsigned int bothDist
   )
   {

     typename vector< clusterCandMelt>::iterator
     membIt = members.begin(),
     oldMembIt = members.begin();

     pair<unsigned int,Kind> index, oldIndex;
     bool allMembersEqual = true;

     if(membIt != members.end()){
       
         oldIndex = 
         make_pair((*oldMembIt).clusterIndex, (*oldMembIt).clusterKind);
         findLastIndex(oldIndex,srcIndices,bothDist);
         //      lowestIndex = oldIndex;
         membIt++;
         while(membIt!=members.end())
           {
             index = make_pair((*membIt).clusterIndex, (*membIt).clusterKind);
             findLastIndex(index,srcIndices,bothDist);
             if(oldIndex != index ){
                 allMembersEqual = false;
                 //melt oldIndex with index
                 //update left with right index
                 double yOldIndex, yIndex;
                 //find lower Y coord
                 yIndex = 
                 meltingCluster->getYMinfromCluster(index.second,index.first);
                 yOldIndex = 
                 meltingCluster->getYMinfromCluster(oldIndex.second,
                                                    oldIndex.first);
                 pair<unsigned int,Kind> destIndex,srcIndex;

                 if(yIndex < yOldIndex)
                   {
                     destIndex = index;
                     srcIndex = oldIndex;
                   } else{
                       destIndex = oldIndex;
                       srcIndex = index;
                   }

                 if(destIndex != srcIndex){
                     meltListsAndIndexOfCluster(meltingCluster,
                                                indexArray,
                                                destIndex,
                                                srcIndex,
                                                meltIndices,
                                                bothDist);
                 }
                 //set control indices
                 oldIndex = destIndex;
                 oldMembIt = membIt;
             }
             membIt++;
           }
     }

     return allMembersEqual;
   }

/*
compareLeftWithRightList
compare two committed list if there are member who can merge together.
if isNewClusterCand is true the a new list is created 
  - mostly when search Cluster Candidates
  
*/
   bool compareLeftWithRightList(TYPE* leftInnerPoint,
                                 TYPE* rightInnerPoint,
                                 bool isNewClusterCand,
                                 list<MEMB_TYP_CLASS*>& leftList ,
                                 list<MEMB_TYP_CLASS*>& rightList,
                                 vector<list<MEMB_TYP_CLASS*> >& retClusterCand,
                                 vector<pair<double,double> >& 
                                 retClusterCandMinMax,
                                 bool leftListIsClusterCandList,
                                 bool rightListIsClusterCandList,
                                 Cluster* rightCluster = 0);

/*

testClusterCandListsOnClusters

*/
   void testClusterCandListsOnClusters(Cluster* rightCluster,
                                       TYPE* leftInnerPoint,
                                       TYPE* rightInnerPoint,
                                       list<MEMB_TYP_CLASS*>& leftList ,
                                       list<MEMB_TYP_CLASS*>& rightList,
                                       vector<list<MEMB_TYP_CLASS*> >& 
                                       retClusterCand,
                                       vector<pair<double,double> >& 
                                       retClusterCandMinMax)
   {

     list<MEMB_TYP_CLASS*> leftTmpList, rightTmpList;

     while(leftList.size()>1){
         leftTmpList.push_back(*leftList.begin());
         leftList.pop_front();
         compareLeftWithRightList(leftInnerPoint,rightInnerPoint,true,
                                  leftTmpList,leftList,retClusterCand,
                                  retClusterCandMinMax,true,true,rightCluster);
     }
     while(leftTmpList.size()){
         leftList.push_back(*leftTmpList.begin());
         leftTmpList.pop_front();
     }

     while(rightList.size()>1){
         rightTmpList.push_back(*rightList.begin());
         rightList.pop_front();
         compareLeftWithRightList(leftInnerPoint,rightInnerPoint,true,
                                  rightTmpList,rightList,retClusterCand,
                                  retClusterCandMinMax,true,true,rightCluster);
     }
     while(rightTmpList.size()){
         rightList.push_back(*rightTmpList.begin());
         rightTmpList.pop_front();
     }

   }



/*

compareClusterCandsWithoppositeList
vergleicht die clusterCand liste mit der gegenueberliegenden liste
speichert indexe in der uebergebenen indexvector

*/
   void compareClusterCandsWithOppositeList(TYPE* leftInnerPoint,
                                            TYPE* rightInnerPoint,
                                            list<MEMB_TYP_CLASS*>& 
                                            clusterCandList,
                                            bool listIsRight,
                                            list<MEMB_TYP_CLASS*>& compList,
                                            Kind compKind, int compIndex,
                                            int bothIndOffset,
                                            vector< clusterCandMelt>* 
                                            clCandMelt)
   {
     typename list<MEMB_TYP_CLASS*>::iterator
     clusterCandIt = clusterCandList.begin();
     int i=0;
     while(clusterCandIt != clusterCandList.end())
       {
         if(compareMemberWithList(leftInnerPoint,rightInnerPoint,*clusterCandIt,
                                  listIsRight,compList))
           {
             //save indexes
             clusterCandMelt newItem = {(unsigned int)compIndex,compKind,
                 listIsRight,*clusterCandIt};

             insertIndexToClusterCandToMelt(i,newItem,clCandMelt);
           }
         clusterCandIt++;
         i++;
       }


   }

/*
insertIndexToClusterCandToMelt
similar to insertIndexToClusterToMelt

*/
   void insertIndexToClusterCandToMelt (int minIndex,
                                        clusterCandMelt& newItem,
                                        vector< clusterCandMelt>* clCandMelt)
   {
     bool found = false;

     typename vector< clusterCandMelt>::iterator
            it = clCandMelt[minIndex].begin();

     while(!found && it != clCandMelt[minIndex].end()){
         if( it->clusterIndex == newItem.clusterIndex &&
             it->clusterKind == newItem.clusterKind &&
             it->meltWithRightCluster == newItem.meltWithRightCluster &&
             it->clusterCandMember == newItem.clusterCandMember)
           {
             found = true;
           }
         it++;
     }
     if (!found)
       {
         clCandMelt[minIndex].push_back(newItem);
       }
   }


/*

compareMemberWithList
similar to compareLeftWithRightList
compare points with lists and return true if neighbor is in epsNeighborhood

*/
   bool compareMemberWithList(TYPE* leftInnerPoint,
                              TYPE* rightInnerPoint,
                              MEMB_TYP_CLASS* member,
                              bool listIsRight,
                              list<MEMB_TYP_CLASS*>& compList)
   {
     bool endOfList= false, distGreaterEps = false;
     typename list<MEMB_TYP_CLASS*>::iterator itList=
         initIteratorOfList(leftInnerPoint, rightInnerPoint,
                            listIsRight,endOfList,
                            distGreaterEps,compList);

     while(!endOfList && !distGreaterEps) {

         if((*itList)->calcDistanz(member) <= eps){
             if(listIsRight){
                 updateNeighborLeftPointToRightList(
                     member,compList,itList,leftInnerPoint,rightInnerPoint);
             }else{
                 updateNeighborRightPointToLeftList(
                     member,compList,itList,leftInnerPoint,rightInnerPoint);
             }
             return true;
         }
         itList = updateIterator(leftInnerPoint,rightInnerPoint,listIsRight,
                                 endOfList,distGreaterEps,compList,itList);
     }

     return false;
   }

/*
initIteratorOfList
initializes an iterator for a given list

*/
   typename list<MEMB_TYP_CLASS*>::iterator
   initIteratorOfList(TYPE* leftInnerPoint,
                      TYPE* rightInnerPoint,
                      bool listIsRight,
                      bool& endOfList, bool& distGreaterEps,
                      list<MEMB_TYP_CLASS*>& compList)
   {
     typename list<MEMB_TYP_CLASS*>::iterator itList;
     if(listIsRight){
         itList=compList.begin();
         if(itList == compList.end()){
             endOfList = true;
         }
         if((*itList)->calcXDistanz(leftInnerPoint) > eps){
             distGreaterEps = true;
         }
     }else{ //list is on left side
         itList = compList.end();
         itList--;
         if(itList== --compList.begin()){
             endOfList = true;
         }
         if((*itList)->calcXDistanz(rightInnerPoint) > eps){
             distGreaterEps = true;
         }
     }
     return itList;
   }

/*
updateIterator
updates a given Iterator

*/
   typename list<MEMB_TYP_CLASS*>::iterator
   updateIterator(TYPE* leftInnerPoint,
                  TYPE* rightInnerPoint,
                  bool listIsRight,
                  bool& endOfList, bool& distGreaterEps,
                  list<MEMB_TYP_CLASS*>& compList,
                  typename list<MEMB_TYP_CLASS*>::iterator itList)
   {
     if(listIsRight){
         itList++;
         if(itList == compList.end()){
             endOfList = true;
         }
         if(!endOfList && (*itList)->calcXDistanz(leftInnerPoint) > eps){
             distGreaterEps = true;
         } else {
             distGreaterEps = false;
         }
     }else{ //list is on left side
         itList--;
         if(itList== --compList.begin()){
             endOfList = true;
         }
         if(!endOfList && (*itList)->calcXDistanz(rightInnerPoint) > eps){
             distGreaterEps = true;
         }else {
             distGreaterEps = false;
         }
     }
     return itList;
   }
   
/*
compareSrcListFromItWithRightList

durchsucht 2 listen - zielliste mit uebergebenen zieliterator und
eine quellliste
falls zwei punkte mit Abstand <= eps gefunden wird -> wird der Iterator
datauf zurueckgegeben
ansonsten wird der QuellIterator zurueckgegeben

*/
   typename list<MEMB_TYP_CLASS*>::iterator
   compareSrcListFromItWithRightList(TYPE* leftInnerPoint,
                                     TYPE* rightInnerPoint,
                                     list<MEMB_TYP_CLASS*>& destList ,
                                     typename list<MEMB_TYP_CLASS*>::iterator
                                     _destIt,
                                     list<MEMB_TYP_CLASS*>& srcList,
                                     int clusterNo, bool considerClusterNo,
                                     bool sortedSrcList,
                                     bool updateN)
   {
     typename list<MEMB_TYP_CLASS*>::iterator srcIt = srcList.begin();
     typename list<MEMB_TYP_CLASS*>::iterator destIt =
         //       --destList.end();
         _destIt;

     bool distEps = true;
     if(sortedSrcList)
       distEps = (*srcIt)->calcXDistanz(leftInnerPoint) <= eps;

     if(considerClusterNo)
       {
         if((*srcIt)->getClusterNo() == clusterNo)
           {
             srcIt++;
           }
       }

     while(srcIt != srcList.end() && distEps)
       {
         while((destIt != --destList.begin()) &&
             srcIt != srcList.end() && distEps  &&
             (*destIt)->calcXDistanz(rightInnerPoint) <= eps){
             if((*destIt)->calcDistanz(*srcIt) <= eps){
                 if(updateN)
                   {
                     list<MEMB_TYP_CLASS*> neighborListLeft,neighborListRight ;
                     updateNeighbor(*destIt,
                                    *srcIt);
                   }
                 return srcIt;

             }
             destIt--;
         }
         destIt = _destIt;
         srcIt++;
         if(sortedSrcList)
           distEps = (*srcIt)->calcXDistanz(leftInnerPoint) <= eps;
         if(considerClusterNo && srcIt != srcList.end())
           {
             if((*srcIt)->getClusterNo() == clusterNo)
               {
                 srcIt++;
               }
           }
       }
     return _destIt;
   }

   typename list<MEMB_TYP_CLASS*>::iterator
   compareSrcListFromItWithLEFTList(TYPE* leftInnerPoint,
                                    TYPE* rightInnerPoint,
                                    list<MEMB_TYP_CLASS*>& destList ,
                                    typename list<MEMB_TYP_CLASS*>::iterator
                                    _destIt,
                                    list<MEMB_TYP_CLASS*>& srcList,
                                    int clusterNo, bool considerClusterNo,
                                    bool sortedSrcList,
                                    bool updateNeigb)
   {
     typename list<MEMB_TYP_CLASS*>::iterator srcIt = --srcList.end();
     typename list<MEMB_TYP_CLASS*>::iterator destIt =
         //       destList.begin();
         _destIt;
     destIt++;

     bool distEps = true;
     if(sortedSrcList)
       distEps = (*srcIt)->calcXDistanz(rightInnerPoint) <= eps;

     if(considerClusterNo )
       {
         if((*srcIt)->getClusterNo() == clusterNo)
           {
             srcIt--;
           }
       }

     while(srcIt != --srcList.begin() && distEps)
       {

         while(destIt != destList.end() &&
             srcIt != --srcList.begin() && distEps &&
             (*destIt)->calcXDistanz(leftInnerPoint) <= eps){
             if((*destIt)->calcDistanz(*srcIt) <= eps){
                 if(updateNeigb)
                   {
                     updateNeighbor(*destIt,
                                    *srcIt);
                   }

                 return srcIt;
             }
             destIt++;
         }
         destIt = _destIt;
         destIt++;
         srcIt--;
         if(sortedSrcList)
           distEps = (*srcIt)->calcXDistanz(rightInnerPoint) <= eps;

         if(considerClusterNo && srcIt != --srcList.begin())
           {
             if((*srcIt)->getClusterNo() == clusterNo)
               {
                 srcIt--;
               }
           }

       }
     return _destIt;
   }


/*
concatPointsfromList
concat the two points from iterators 
- rummage eps neighborhood for appropriate members

*/
   void concatClusterCand(Cluster* rightCluster,
                          TYPE* leftInnerPoint,
                          TYPE* rightInnerPoint,
                          list<MEMB_TYP_CLASS*>& destList,
                          pair<double,double>& clusterPair,
                          MEMB_TYP_CLASS* leftMemb,
                          MEMB_TYP_CLASS* rightMemb,
                          list<MEMB_TYP_CLASS*>& leftList,
                          list<MEMB_TYP_CLASS*>& rightList,
                          int clusterNo);



/*
addListToCorrectClusterType
finds out correct cluster type an push it to  correct cluster vector

*/
   void addListToCorrectClusterType(
       list<MEMB_TYP_CLASS*>& clusterList,
       pair<double,double>& clusterPair,
       pair<unsigned int,Kind>& index
   );

/*

defineDestIndexPair
define the new index for two concated clusterLists
if retKind is different from leftKind then return false
  
*/
   bool defineDestIndexPair(list<MEMB_TYP_CLASS*>& leftList,
                  list<MEMB_TYP_CLASS*>& rightList,//This could be a empty list
                  pair<unsigned int,Kind>& leftIndex,
                  pair<unsigned int,Kind>& retIndex)
   {

     typename list<MEMB_TYP_CLASS*>::iterator  itLeftPoint,itRightPoint;

     int leftListSize = leftList.size();
     int rightListSize = rightList.size();

     if(leftListSize && rightListSize)
       {
         itLeftPoint = leftList.begin();
         itRightPoint = --rightList.end();
       }else{
           if(leftListSize)
             {
               itLeftPoint = leftList.begin() ; 
               //left Point of cluster is at the end of List
               itRightPoint = --leftList.end();
             }
       }

     double rightDist = (*itRightPoint)->calcXDistanz(rightOuterPoint);
     double leftDist = (*itLeftPoint)->calcXDistanz(leftOuterPoint);

     int clusterType =1;
     if(leftDist <= eps){
         //cluster is from left side reachable
         clusterType++;
     }
     if(rightDist <= eps){
         //cluster is from right side reachable
         if(clusterType == 1)
           clusterType = 3; //list is not reachable from left side
         else
           clusterType = 4; //list is reachable from left and right side
     }
     int listNo = getVectorSize(getClusterKindFromType(clusterType));

     retIndex = make_pair(listNo,getClusterKindFromType(clusterType));
     bool indexIsEqual =
         retIndex.second == leftIndex.second;
     if(!indexIsEqual){
         findNextEmptyList(retIndex);
     }
     return indexIsEqual;
   }

/*

findNextEmptyList
find the next empty list in cluster vector
if no list is empty retIndex==VectorSize
  
*/
   void findNextEmptyList(pair<unsigned int,Kind>& retIndex )
   {
     //find out list no in which to save
     bool indexFound = false;
     //find first empty List
     for(unsigned int i =0; i< getVectorSize(retIndex.second) &&
     !indexFound ; i++)
       {
         if(getListLength(i,retIndex.second) == 0)
           {// empty list
             retIndex.first=i;
             indexFound= true;
           }
       }
     if(!indexFound ){
         retIndex.first=getVectorSize(retIndex.second);
     }

   }


/*

searchAndDeletItemFromList
search an item in given list and delete it

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator
   searchAndDeletItemFromList(
       typename list<MEMB_TYP_CLASS*>::iterator itemIt,
       list<MEMB_TYP_CLASS*>& list, bool& elemDeleted)
   {
     return
         searchAndDeletItemFromList(*itemIt,itemIt,list,elemDeleted);
   }


   typename std::list<MEMB_TYP_CLASS*>::iterator
   searchAndDeletItemFromList(MEMB_TYP_CLASS* item,
                              list<MEMB_TYP_CLASS*>& list,  bool& elemDeleted)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator searchIt = list.begin();
     bool deleted = false;
     while(!deleted && searchIt != list.end()){
         if(item == *searchIt){
             deleted = true;
             searchIt = list.erase(searchIt);
         }else{
             searchIt++;
         }
     }
     elemDeleted = deleted;
     return searchIt;
   }


   typename std::list<MEMB_TYP_CLASS*>::iterator
   searchAndDeletItemFromList(MEMB_TYP_CLASS* item,
                              typename list<MEMB_TYP_CLASS*>::iterator itemIt,
                              list<MEMB_TYP_CLASS*>& list,  bool& elemDeleted)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator searchIt = //list.begin();
         searchAndDeletItemFromList( item, list, elemDeleted);
     if(!elemDeleted){
         return itemIt;
     }else{
         return searchIt;
     }
   }


/*

updateNeighbor

*/
   void updateNeighborLeftPointToRightList(MEMB_TYP_CLASS* point, 
                                           list<MEMB_TYP_CLASS*>& destList,
                                           typename list<MEMB_TYP_CLASS*>::
                                           iterator listIt,
                                           TYPE* leftInnerPoint, 
                                           TYPE* rightInnerPoint)
   {
     list<MEMB_TYP_CLASS*> neighborList ;
     while (listIt != destList.end() &&
         listIt != --destList.begin() &&
         (*listIt)->calcXDistanz(leftInnerPoint) <= eps){

         if((*listIt)->calcDistanz(point) <= eps){

             updateNeighbor(point,*listIt);
         }
         listIt++;
     }
   }

   void updateNeighborRightPointToLeftList(MEMB_TYP_CLASS* point, 
                                           list<MEMB_TYP_CLASS*>& destList,
                                           typename list<MEMB_TYP_CLASS*>::
                                           iterator listIt,
                                           TYPE* leftInnerPoint, 
                                           TYPE* rightInnerPoint)
   {
     list<MEMB_TYP_CLASS*> neighborList ;
     while (listIt != --destList.begin() &&
         listIt != destList.end() &&
         (*listIt)->calcXDistanz(rightInnerPoint) <= eps){
         if((*listIt)->calcDistanz(point) <= eps){
             updateNeighbor(point,*listIt);
         }
         listIt--;
     }
   }

   void updateNeighborRightListToLeftList(list<MEMB_TYP_CLASS*>& leftList,
                                          list<MEMB_TYP_CLASS*>& rightList,
                                          bool leftListSorted,
                                          bool rightListSorted,
                                          TYPE* leftInnerPoint, 
                                          TYPE* rightInnerPoint)
   {

     list<MEMB_TYP_CLASS*> neighborList ;
     typename list<MEMB_TYP_CLASS*>::iterator
     leftListIt = --leftList.end(),
     rightListIt = rightList.begin();
     bool epsLefListGreater = false, epsRightListGreater = false;

     if(leftListSorted)
       {
         epsLefListGreater = getGreaterEps(leftList,
                                           rightInnerPoint,leftListIt);
       }
     if (rightListSorted)
       {
         epsRightListGreater = getGreaterEps(rightList,
                                             leftInnerPoint,rightListIt);
       }

     while (leftListIt != --leftList.begin() &&
         leftListIt != leftList.end() && !epsLefListGreater){
         while (rightListIt != rightList.end() && !epsRightListGreater){


             if((*leftListIt)->calcDistanz(*rightListIt) <= eps){

//                  updateNeighbor(*leftListIt,*rightListIt);
                 //                              ,neighborList);
               if(!(*leftListIt)->existNeighbor((*rightListIt))){
                 (*leftListIt)->addNeighbor((*rightListIt));
                 }
                 if(!(*rightListIt)->existNeighbor((*leftListIt))){
                   (*rightListIt)->addNeighbor((*leftListIt));
                 }

             }
             rightListIt++;
             if (rightListSorted)
               {
                 epsRightListGreater = getGreaterEps(rightList,
                                                     rightInnerPoint,
                                                     rightListIt);
               }
         }
         rightListIt = rightList.begin();
         epsRightListGreater = false;
         leftListIt--;
         if(leftListSorted)
           {
             epsLefListGreater = getGreaterEps(leftList,
                                               leftInnerPoint,leftListIt);
           }
     }
   }

   bool getGreaterEps(list<MEMB_TYP_CLASS*>& list,
                      TYPE* borderPoint,
                      typename std::list<MEMB_TYP_CLASS*>::iterator it)
   {
     bool retVal = false;
     if((!retVal &&
         it != --list.begin() &&
         it != list.end()))
       {
         retVal = (*it)->calcXDistanz(borderPoint) > eps;

       }
     return retVal;
   }

   void updateNeighbor(MEMB_TYP_CLASS * leftMemb, MEMB_TYP_CLASS *rightMemb,
                       list<MEMB_TYP_CLASS*>& neighborList);

   void updateNeighbor(MEMB_TYP_CLASS * leftMemb, MEMB_TYP_CLASS *rightMemb)
   {
     list<MEMB_TYP_CLASS*> neighborList;
     updateNeighbor(leftMemb,rightMemb,neighborList);
   }

/*
isMembInList
when point is in list -> return true

*/
   bool isMembInList(MEMB_TYP_CLASS *memb, list<MEMB_TYP_CLASS*>& list);


/*
testReachabilityAndSetClusterNoAtEachPoint
test each point if it is density reachable
update clusterNo and Type

*/
   bool testReachabilityAndSetClusterNoAtEachPoint(TYPE* leftOuterPoint,
                                          TYPE* rightOuterPoint,
                                          list<MEMB_TYP_CLASS*>& list,
                                          pair<unsigned int,Kind>& clusterPair,
                                          bool isAlreadyClusterCand)
   {
     return testReachabilityAndSetClusterNoAtEachPoint( leftOuterPoint,
                                            rightOuterPoint,
                                            list,
                                            clusterPair.first,
                                            getClusterType(clusterPair.second),
                                            isAlreadyClusterCand);
   }


   bool testReachabilityAndSetClusterNoAtEachPoint(TYPE* leftOuterPoint,
                                                   TYPE* rightOuterPoint,
                                                   list<MEMB_TYP_CLASS*>& list,
                                                   int listNo,
                                                   int type,
                                                   bool isAlreadyClusterCand)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator
     it = list.begin();
     bool allDensReachable = true;

     Kind kind = getClusterKindFromType(type);

     while(it!=list.end()){
         if(!(*it)->updateInnerPnt(minPts)){
           if(!(*it)->updateDensityReachable(minPts) && 
             (kind == CLUSTER || kind == CLUSTERCAND) ){
                 allDensReachable = false;
                 if(moveItemToClusterCandOrNoise(leftOuterPoint,
                                                 rightOuterPoint,
                                                 it,list,
                                                 isAlreadyClusterCand))
                   {
                     it = list.begin();
                   }else{
                       it++;
                   }
             } else{
                 (*it)->setClusterNo(getClusterNo(listNo,kind));
                 (*it)->setClusterType(type);
                 it++;
             }
         }else{
             (*it)->setClusterNo(getClusterNo(listNo,kind));
             (*it)->setClusterType(type);
             it++;
         }

     }
     return allDensReachable;
   }


   bool moveItemToClusterCandOrNoise(TYPE* leftOuterPoint, 
                                     TYPE* rightOuterPoint,
                                     typename list<MEMB_TYP_CLASS*>::
                                     iterator delIt,
                                     list<MEMB_TYP_CLASS*>& srcList,
                                     bool isAlreadyClusterCand)
   {
     
// the examined Point is not denisty reachable
// look whether its distance to the outer borderpoints is > eps
// if it is consider its neighbors if they are Border or Inner Points
// if one ore more of its neighbors are border points which
// are within the outer borders - examine if the Point could
// transform to a density reachable point if his foundet Neighbors transform
// to inner points - when this could be move point to clusterCandlist else to
// NOISE
     


     //check distance to outer borders
     bool isClusterCand = false,itemWasMoved = false;
     if(pointIsInOuterBorders(leftOuterPoint,rightOuterPoint,delIt))
       {
         //add to clusterCand
         isClusterCand = true;

       } else {
           //check Neighbors if there are inner or outer points
           list<MEMB_TYP_CLASS*> innerNeighbors;
           list<MEMB_TYP_CLASS*> densReachNeighbors;
           typename list<MEMB_TYP_CLASS*>::iterator
           neighborIt = (*delIt)->getEpsNeighborhood(true);
           while (neighborIt != (*delIt)->getEpsNeighborhood(false) &&
               !isClusterCand)
             {
               if((*neighborIt)->updateInnerPnt(minPts))
                 {
                   innerNeighbors.push_back((*neighborIt));
                 }else {
                     if(pointIsInOuterBorders(leftOuterPoint,
                                              rightOuterPoint,neighborIt) &&
                         (*neighborIt)->updateDensityReachable(minPts))
                       {
                      //add borderPoints which are reachable in next clustering
                         densReachNeighbors.push_back((*neighborIt));
                       }
                 }
               //if now sizes from Neighbors >= minPts
          //->it is possible that this point is density reachable in next round
               isClusterCand = (innerNeighbors.size() +
                   densReachNeighbors.size()) >= minPts;
               neighborIt++;
             }
       }
     if(isClusterCand)
       {
         if(!isAlreadyClusterCand){
             moveItemUnsorted(
                 delIt, srcList, clusterCandList,
                 CLUSTERCAND_CL_NO,CLUSTERCAND_CL_NO);
             itemWasMoved = true;
         }
       } else {
           itemWasMoved = true;
           moveItemUnsorted(
               delIt, srcList, noiseList,
               NOISE_CL_NO,NOISE_CL_NO);
       }

     return itemWasMoved;
   }

/*
inserElementSorted
sort the elements from two lists in a new return list
delete elements from sourceList

*/
   void sortElemtsFromListsInNewList(list<MEMB_TYP_CLASS*>& sourceList,
                                     list<MEMB_TYP_CLASS*>& destList,
                                     list<MEMB_TYP_CLASS*>& retList,
                                     pair<unsigned int,Kind>& retIndex)
   {
     typename list<MEMB_TYP_CLASS*>::iterator
     sourceListIt  = sourceList.begin(),
     destListIt  = destList.begin();

     int retClNo = getClusterNo(retIndex);
     int clType = getClusterType(retIndex.second);

     while(destListIt != destList.end()
         && sourceListIt  != sourceList.end())
       {
         if((*destListIt)->getXVal() >= (*sourceListIt)->getXVal())
           {
             retList.push_back((*destListIt));
             (*destListIt)->setClusterNo(retClNo);
             (*destListIt)->setClusterType(clType);
             destListIt++;
           }
         else
           {
             retList.push_back((*sourceListIt));
             (*sourceListIt)->setClusterNo(retClNo);
             (*sourceListIt)->setClusterType(clType);
             sourceListIt++;
           }
       }

     while(destListIt != destList.end()){
         retList.push_back((*destListIt));
         (*destListIt)->setClusterNo(retClNo);
         (*destListIt)->setClusterType(clType);
         destListIt++;
     }
     while(sourceListIt != sourceList.end()){
         retList.push_back((*sourceListIt));
         (*sourceListIt)->setClusterNo(retClNo);
         (*sourceListIt)->setClusterType(clType);
         sourceListIt++;
     }

     sourceList.clear();

   }

/*
sortRightListToLeftList

sorts a given source list in a given destination list
after this method the destinationList is empty.

*/
   void sortRightListToLeftList(Cluster * origCluster,
                                list<MEMB_TYP_CLASS*>& sourceList,
                                list<MEMB_TYP_CLASS*>& destList,
                                pair<double,double>& sourceMinMax,
                                pair<double,double>& destMinMax,
                                pair<unsigned int,Kind>& origIndex,
                                pair<unsigned int,Kind>& destIndex,
                                bool kindChanged,bool changeMinMax)
   {

     typename list<MEMB_TYP_CLASS*>::iterator
     sourceListIt,destListIt ;
     int destClNo = getClusterNo(destIndex);
     int clType = getClusterType(destIndex.second);
     int destListLength = destList.size();

     if(kindChanged)
       {//change each clusterNo and Type in destList
         updateClusterNoAndTypeInList(destList,destIndex);
       }

     if(sourceList.size() > 0 ){
         sourceListIt  = sourceList.begin();
         destListIt  = --destList.end();

         while(sourceListIt != sourceList.end()){
             //first go back in list until destPoint > srcPoint
             if(destListLength)
               {
                 if(destListIt == destList.end()){
                     destListIt--;
                 }
                 while(destListIt != destList.begin() &&
                     destListIt != destList.end() &&
                     (*destListIt)->getXVal() < (*sourceListIt)->getXVal()){
                     destListIt--;
                 }
                 //then serch position to insert
                 while(destListIt != destList.end() &&
                     (*destListIt)->getXVal() >= (*sourceListIt)->getXVal()){
                     destListIt++;
                 }

                 destList.insert(destListIt,*sourceListIt);

               }else{
                   destList.push_back(*sourceListIt);
               }
             (*sourceListIt)->setClusterNo(destClNo);
             (*sourceListIt)->setClusterType(clType);

             sourceListIt++;
         }

         sourceList.clear();
     }

     bool minMaxAvailable = getMinMaxSize(origIndex.second) >= origIndex.first;

     //update MinMaxY
     if(changeMinMax){
         updateMinMaxVal(destMinMax,sourceMinMax);
     }
     if(kindChanged && minMaxAvailable){
         moveMinMaxPair(destIndex,origIndex,origCluster);
     }
     if(kindChanged)
       {// move list from sourceIndex to destIndex
         moveClusterList(destIndex,origIndex,origCluster);
       }
   }

/*
updateClusterNoAndTypeInList
update for each item in destList ClusterNo and ClusterType

*/
   void updateClusterNoAndTypeInList(list<MEMB_TYP_CLASS*>& destList,
                                     pair<unsigned int,Kind>& destIndex)
   {
     updateClusterNoAndTypeInList(destList,destIndex.first, destIndex.second);
   }

   void updateClusterNoAndTypeInList(list<MEMB_TYP_CLASS*>& destList,
                                     int listNo ,Kind kind)
   {
     typename list<MEMB_TYP_CLASS*>::iterator
     destListIt ;
     int destClNo = getClusterNo(listNo,kind);
     int clType = getClusterType(kind);

     destListIt  = destList.begin();
     while(destListIt != destList.end()){
         (*destListIt)->setClusterNo(destClNo);
         (*destListIt)->setClusterType(clType);
         destListIt++;
     }
   }

/*
copyRightClusterListToLeftCluster

copy the untouched right Cluster lists to left Cluster

*/
   void copyRightClusterListToLeftCluster(Cluster *rightCluster,
                                          Kind kind)
   {

     for(int i=0; i<rightCluster->getVectorSize(kind); i++)
       {
         if(rightCluster->getListLength(i,kind) > 0)
           {//list is not empty
             pair<unsigned int,Kind> index = make_pair(i,kind);
             addListToCorrectClusterType(
                 rightCluster->getList(i,kind),
                 rightCluster->getMinMaxFromCluster(i,kind),
                 index);
           }
       }
   }

/*
 insertIndexToClusterToMelt
 
*/
   void insertIndexToClusterToMelt(int minIndex,
                                   int insertInd,
                                   Kind kind,
                                   vector<pair<unsigned int,Kind> >* 
                                   clusterCandToMelt,
                                   vector<unsigned int >& minToMaxIndexes)
   {
     pair<unsigned int,Kind> newItem= make_pair(insertInd,kind);

     if ( find(clusterCandToMelt[minIndex].begin(),
               clusterCandToMelt[minIndex].end(), newItem)
         == clusterCandToMelt[minIndex].end() )
       {
         clusterCandToMelt[minIndex].push_back(newItem);
         insertInMinToMaxIndex(minToMaxIndexes,minIndex);
       }
   }


/*
 insert val in index vector -> look up for doubles
 
*/
   void insertInMinToMaxIndex(vector<unsigned int >& minToMaxIndexes,
                              unsigned int val )
   {
     if ( find(minToMaxIndexes.begin(),
               minToMaxIndexes.end(), val)
         == minToMaxIndexes.end() )
       minToMaxIndexes.push_back(val);
   }




   void deleteEmptyLists(Kind kind)
   {


     int i=0;
     int updateItems = getVectorSize(kind);
     while( i < getVectorSize(kind))
       {
         if(getListLength(i,kind) < minPts){
           if(getListLength(i,kind)){
             //verschiebe punkte zu clusterCand Or Noise
             typename list<MEMB_TYP_CLASS*>::iterator it =
             getIterator(i,true,kind);
             while(it != getIterator(i,false,kind)){
               it = moveItemUnsorted(it,getList(i,kind),
                                     clusterCandList,
                                     CLUSTERCAND_CL_NO,
                                     CLUSTERCAND_CL_NO);
             }
           }
           //         }
           //         if(getListLength(i,kind) == 0)
           //           {
             eraseList(kind,i);
             eraseMinMax(kind,i);

             if(i < updateItems)
               {
                 updateItems=i;
               }
           }else{
          //Check kind and also update Lists push list back to existing vector
               pair<unsigned int,Kind> index = make_pair(i,kind), destIndex;
               list<MEMB_TYP_CLASS*> emptyL;
               if(!defineDestIndexPair(getList(index),emptyL,
                                       index,destIndex))
                 {
                   moveClusterList(destIndex,index,this);
                   moveMinMaxPair(destIndex,index,this);
                   eraseList(index.second,index.first);
                   eraseMinMax(index.second,index.first);

                   if(i < updateItems)
                     {
                       updateItems=i;
                     }
                 }else{
                     i++;
                 }
           }
       }
     for(int i =updateItems; i< getVectorSize(kind); i++)
       {
         updateClusterNoAndTypeInList(getList(i,kind),i,kind);
       }
   }



/*
pointIsInOuterBorders
check if a point is within right or left outer boreder

*/
   bool pointIsInOuterBorders(TYPE* leftOuterPoint, TYPE* rightOuterPoint,
                              typename list<MEMB_TYP_CLASS*>::iterator it)
   {
     return (*it)->calcXDistanz(leftOuterPoint) <= eps ||
         (*it)->calcXDistanz(rightOuterPoint) <= eps;
   }

   bool listIsInYBordersOfList(Cluster* srcCluster,
                               int srcListNo, Kind srcKind,
                               Cluster* destCluster,
                               int destListNo, Kind destKind)
   {
     pair<double,double> border =
     srcCluster->getMinMaxFromCluster(srcListNo,srcKind);
     return listIsInYBordersOfList(border,destCluster,destListNo,destKind);
   }

   bool listIsInYBordersOfList(pair<double,double>& border,
                               Cluster* destCluster,
                               int destListNo, Kind destKind)
   {

     return(valIsInYBordersOfList(
         border.first,destCluster,destListNo,destKind)
         || valIsInYBordersOfList(
             border.second,destCluster,destListNo,destKind));
   }

   bool memberIsInYBordersOfList(MEMB_TYP_CLASS* memb, Cluster* cluster,
                                 int listNo, Kind kind)
   {
     return valIsInYBordersOfList(memb->getYVal(),cluster,listNo,kind);
   }



   bool valIsInYBordersOfList(double val, Cluster* cluster,
                              int listNo, Kind kind)
   {
     //3 possibilities
     pair<double,double> border =cluster->getMinMaxFromCluster(listNo,kind);
     // 1. point is in middle of borders
     if(val >= border.first && val <= border.second)
       return true;
     // 2. point is above border
     if( abs(val - border.second) <= eps ||
         // 3. point is under borders
         abs(border.first - val) <= eps)
       return true;
     return false;
   }

   void deleteVector(vector<pair<unsigned int,
                     Kind> >* vecAr, unsigned int size )
   {
     for (unsigned int i = 0; i< size; i++){
         vecAr[i].clear();
     }
     //     delete[] vecAr;
   }
   void deleteVector(vector< clusterCandMelt>* vecAr, unsigned int size )
   {
     for (unsigned int i = 0; i< size; i++){
         vecAr[i].clear();
     }
     //         delete[] vecAr;
   }

   int getClusterType(Kind kind){
     switch (kind){
       case NOISE:
         return NOISE_CL_NO;
       case CLUSTERCAND:
         return CLUSTERCAND_CL_NO;
       case CLUSTER:
         return CLUSTER_CL_NO;
       case LEFT:
         return LEFT_CL_NO;
       case RIGHT:
         return RIGHT_CL_NO;
       case BOTH:
         return BOTH_CL_NO;
       case CLCANDCLUSTERS:
         return CLCANDCL_CL_NO;
       default:
         return -3;
     }
     return -3;
   }

   Kind getClusterKindFromType(int clusterType){
     switch (clusterType){
       case NOISE_CL_NO:
         return NOISE;
       case CLUSTERCAND_CL_NO:
         return CLUSTERCAND;
       case CLUSTER_CL_NO:
         return CLUSTER;
       case LEFT_CL_NO:
         return LEFT;
       case RIGHT_CL_NO:
         return RIGHT;
       case BOTH_CL_NO:
         return BOTH;
       case CLCANDCL_CL_NO :
         return CLCANDCLUSTERS;
       case UNDEF_CLUSTER_CL_NO:
         return UNDEF;
     }
     return UNDEF;
   }
   
/*

calcClusterNo
calculates a unique identifier based on the cantor pairing function

*/

   int getClusterNo(unsigned int listNo, Kind kind)
   {
     switch (kind){
       case NOISE:
         //        return getClusterNo(listNo,NOISE_CL_NO);
         return NOISE_CL_NO;
       case CLUSTERCAND:
         //        return getClusterNo(listNo,CLUSTERCAND_CL_NO);
         return CLUSTERCAND_CL_NO;
       case CLUSTER:
         return getClusterNo(listNo,CLUSTER_CL_NO);
       case LEFT:
         return getClusterNo(listNo,LEFT_CL_NO);
       case RIGHT:
         return getClusterNo(listNo,RIGHT_CL_NO);
       case BOTH:
         return getClusterNo(listNo,BOTH_CL_NO);
       case CLCANDCLUSTERS:
         return getClusterNo(listNo,CLCANDCL_CL_NO);
       case UNDEF:
         return -3;
     }
     return -1;
   }

   unsigned int getClusterNo(unsigned int listNo, int clustertype)
   {
     unsigned  int x = listNo;
     unsigned  int y =(unsigned int) clustertype;
     return (x+y)*(x+y+1)/2 + y;
   }

   unsigned int getClusterNo(pair<unsigned int,Kind>& list)
   {
     return getClusterNo(list.first,list.second);
   }

   int getListNoOfClusterNo(unsigned int clusterNo, Kind kind)
   {
     switch (kind){
       case NOISE:
         //        return getListNoOfClusterNo(clusterNo,NOISE_CL_NO);
         return NOISE_CL_NO;
       case CLUSTERCAND:
         //        return getListNoOfClusterNo(clusterNo,CLUSTERCAND_CL_NO);
         return CLUSTERCAND_CL_NO;
       case CLUSTER:
         return getListNoOfClusterNo(clusterNo,CLUSTER_CL_NO);
       case LEFT:
         return getListNoOfClusterNo(clusterNo,LEFT_CL_NO);
       case RIGHT:
         return getListNoOfClusterNo(clusterNo,RIGHT_CL_NO);
       case BOTH:
         return getListNoOfClusterNo(clusterNo,BOTH_CL_NO);
       case CLCANDCLUSTERS:
         return getListNoOfClusterNo(clusterNo,CLCANDCL_CL_NO);
     }
     return -1;
   }

   unsigned int getListNoOfClusterNo(unsigned int clusterNo, int clustertype)
   {
     return wHelp(clusterNo) - (unsigned int)clustertype;
   }


   int getClusterTypeOfClusterNo(unsigned int clusterNo,unsigned int listNo)
   {
     unsigned int w = wHelp(clusterNo);
     unsigned int t = (w*w +w) / 2 ;
     return (int)(listNo -t);
   }

   unsigned int wHelp(unsigned int z)
   {
     int retVal = floor( (sqrt((8 * z + 1)) -1) / 2);

     return (unsigned int) retVal;
   }



}; // class end


}
#endif

