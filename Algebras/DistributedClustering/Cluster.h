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

[1] Implementation of Class Cluster

August-February 2015, Daniel Fuchs 

[TOC]

1 Overview

This file contains the implementation of the class Cluster.

1.1 Includes

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

namespace distributedClustering{
  
/*
2 ~enum Kind~

Specify the clustertype.

*/
enum Kind {NOISE,CLUSTERCAND, CLUSTER , LEFT, 
  RIGHT, BOTH,CLCANDCLUSTERS, UNDEF};


/*
3 ~ccMelting~

Is a struct to define the index type for storing cluster lists 
which are to melt.

*/
template <class MEMB_TYP_CLASS>
struct ccMelting {
  unsigned int clusterIndex;
  Kind clusterKind;
  bool meltWithRightCluster;
  MEMB_TYP_CLASS* clusterCandMember;
};

/*
4 ~Class Cluster~

This class represents a set of lists and vector lists to 
store cluster with noise. Also it provides methods to 
melt such objects of type Cluster.

*/


template <class MEMB_TYP_CLASS, class TYPE>
class Cluster
{
private:

/*
4.1 ~members~

*/
  enum MinMaxKind {LEFTMM, RIGHTMM, BOTHMM, GLOBAL, CLCANDCLMM};

  const double MIN_DOUBLE = -1 * std::numeric_limits<double>::max();
  const double MAX_DOUBLE = std::numeric_limits<double>::max();

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

  std::list<MEMB_TYP_CLASS*> noiseList; // final noise points
  std::list<MEMB_TYP_CLASS*> clusterCandList; 
  // stores points who are not in a cluster yet
  std::list<MEMB_TYP_CLASS*> emptyList; // emty help list
  std::vector<std::list<MEMB_TYP_CLASS*> > clusterArray; //final cluster
  //clusters on right side which are adjacent to the left boundary
  std::vector<std::list<MEMB_TYP_CLASS*> > leftPartCluster;
  //clusters on left side which are adjacent to the right boundary
  std::vector<std::list<MEMB_TYP_CLASS*> > rightPartCluster; 
  // cluster which are adjacent to both sides boundary
  std::vector<std::list<MEMB_TYP_CLASS*> > bothSideCluster;
  std::vector<std::list<MEMB_TYP_CLASS*> > undefinedCluster;
  std::vector<std::list<MEMB_TYP_CLASS*> > clusterCandClusters;
  std::vector<std::list<MEMB_TYP_CLASS*> > emptyVectorList;


  std::vector<std::pair <double,double> > clusterMinMaxY;
  std::vector<std::pair <double,double> > leftPCMinMaxY;
  std::vector<std::pair <double,double> > rightPCMinMaxY;
  std::vector<std::pair <double,double> > bothSCMinMaxY;
  std::vector<std::pair <double,double> > clusterCandClMinMax;
  std::vector<std::pair <double,double> > emptyMinMaxY;

  double eps;
  int minPts ;

  typedef struct ccMelting<MEMB_TYP_CLASS>clusterCandMelt;

public:

/*
4.2 ~constructor~

constructor for creating a Cluster object and for 
melting two objects of type Cluster.

*/
  Cluster( MEMB_TYP_CLASS* leftMember, double _eps, int _minPts);
  Cluster(std::vector <MEMB_TYP_CLASS*>& members, double _eps, int _minPts);


/*
4.2 ~meltClusters~

Melt this Cluster with the right cluster
medianPoint and rightMedPoint are the two outer
Points which represent the splitline.

*/
  void meltClusters(Cluster * rightCluster,
                   TYPE* leftInnerPoint,
                    TYPE* rightInnerPoint);



/*
4.3 ~getVectorSize~

Returns the quantity of Clsuters.

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
4.4 ~getClusterVector~

*/
  std::vector<std::list<MEMB_TYP_CLASS*> >& getClusterVector(Kind kind){

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
4.5 ~getRightOuterPoint~

*/
  TYPE* getRightOuterPoint()
  {
    return rightOuterPoint ;
  }

/*
4.6 ~getListLength~

Return the list length from position i.

*/
  unsigned int getListLength( std::pair<unsigned int,Kind>& index){
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
4.7 ~updateMinMaxVal~

Compare given point with the value in the pair and update
the min max vector for y direction.

*/
  void updateMinMaxVal(Kind kind, int listNo, MEMB_TYP_CLASS *member )
  {
    updateMinMaxVal(getMinMaxFromCluster(kind,listNo),member);
  }

  void updateMinMaxVal(Kind kind, int listNo, 
                       std::pair <double,double> &minMax )
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

  void updateMinMaxVal(std::pair <double,double> &minMax, 
                       MEMB_TYP_CLASS *member)
  {
    double newExtrema = member->getYVal();
    updateMinMaxVal(minMax,newExtrema);
  }

  void updateMinMaxVal(std::pair <double,double> &newMinMax, 
                       std::pair <double,double> &oldMinMax )
  {
    updateMinMaxVal(newMinMax,oldMinMax.first);
    updateMinMaxVal(newMinMax,oldMinMax.second);
  }

  void updateMinMaxVal(std::pair <double,double> &minMax, double newExtrema )
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
4.8  ~getNewMinMaxForClusterList~
  
Return a new MinMax Pair  
and set MinMax from destination to standard values.
  
*/
  std::pair <double,double> 
  getNewMinMaxForClusterList(std::pair <double,double>& srcMinMax,
                             std::pair <double,double>& destMinMax)
  {
    std::pair <double,double> retMM(MAX_DOUBLE,MIN_DOUBLE);
    updateMinMaxVal(retMM,destMinMax);
    updateMinMaxVal(retMM, srcMinMax);
    srcMinMax.first =MAX_DOUBLE;
    srcMinMax.second = MIN_DOUBLE;
    return retMM;
  }

  
/*
4.9 ~findNextMinList~

Find the next minimum point in y direction in the clusterlists.

*/
  bool findNextMinList(int& leftIndex, Kind& leftKind,
                       double& leftMinima,double& leftMaxima,
                       bool calcLeftMin, int& leftCnt,
                       int& rightIndex, Kind& rightKind,
                       double& rightMinima,double& rightMaxima,
                       bool calcRightMin, int& rightCnt,
                       Cluster* rightCluster);
  

  void findNextMinList(int& retIndex, Kind& retKind,
                       double actualMinima, bool rightCluster);
  
/*
4.10  ~getIndexOfFindNextMinList~

Return the index value for the next minimum in y direction.
  
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
4.11 findNextMinListOfClCand

Returns the next minimum clustercand in y direction.

*/

  bool findNextMinListOfClCand(std::vector<std::pair <double,double> >&
                               minMaxlist,int& index,
                               double& actualMinima,
                               double& actualMaxima,
                               bool& clCandOutOfRangeLeftCl,
                               bool& clCandOutOfRangeRightCl,
                               double actMaxLeftList,
                               double actMaxRightLsit);


/*
4.12 ~getYMinfromCluster~

return the min Y value from the appropriate cluster list

*/
  double getYMinfromCluster(std::pair<unsigned int,Kind>& list)
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
4.13 ~getYMinfromCluster~

Return the min Y value from the appropriate cluster list.

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
4.14 ~setNewMin~

Set new min value for list with given index.

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
4.15 ~setNewMax~

Set new max value for list with given index.

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
4.16 ~pushMemberToClusterList~

Get minimum maximum value for given index.

*/
  std::pair <double,double>&
  getMinMaxFromCluster(int i,Kind kind)
  {
    return getMinMaxFromCluster(kind,  i);
  }

  std::pair <double,double>&
  getMinMaxFromCluster(std::pair<unsigned int,Kind>& index)
  {
    return getMinMaxFromCluster(index.first,index.second);
  }

  std::pair <double,double>&
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

/*
4.17 ~getMinMaxSize~

Get vector size from min max values for given cluster type.

*/
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
4.18 ~getMinMaxVector~

*/
  std::vector<std::pair <double,double> >&
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
 4.18 ~getList~
 
Return the clusterList on position i.

*/
  std::list<MEMB_TYP_CLASS*> &getList(std::pair<unsigned int,Kind>& list){
    return getList(list.first,list.second);
  }
  std::list<MEMB_TYP_CLASS*> &getList(int i, Kind kind){
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
4.19 ~insertElement~

Insert a member at given position i.

*/
  void insertElement(typename std::list<MEMB_TYP_CLASS*>::iterator it,
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

/*
4.19 ~insertList~

Insert a list at given cluster type.

*/
  void insertList(
        typename std::vector<std::list<MEMB_TYP_CLASS*> >::iterator it,
        std::list<MEMB_TYP_CLASS*>& list,Kind kind)
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
                  std::list<MEMB_TYP_CLASS*>& list,Kind kind)
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
4.20 ~insertMinMax~

Insert min and max value pair to vector.

*/
   void insertMinMax(
            typename std::vector<std::pair<double,double> >::iterator it,
            std::pair<double,double>& list,Kind kind)
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


   void insertMinMax(int i,
                     std::pair<double,double>& list,Kind kind)
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
4.21 ~getIterator~

Returns an iterator from a clusterlist or 
noiselist either at the beginning or at end.

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator 
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
4.22 ~eraseItem~

Delete an member item either at the beginning or at end.

*/
   typename std::list<MEMB_TYP_CLASS*>::
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
4.23 ~eraseList~

Delete a list on given position from given cluster type.

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
4.24 ~eraseMinMax~

Erase list from vector.

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
4.25 ~clearList~

Empty list from given index.

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
4.26 ~pushMemberToClusterList~

Push a committed member at the end or beginning of a given clusterlist.

*/
   void pushMemberToClusterList(bool front,MEMB_TYP_CLASS *member,
                                int list, Kind kind);

/*
4.27 ~pushListToCluster~

Insert given list to cluster vector of given cluster type.

*/
   void pushListToCluster(Kind kind,std::list<MEMB_TYP_CLASS *>& list)
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
 4.28 ~pushMinMaxToCluster~

 Insert given min max values to  vector of given cluster type.
 
*/
   void pushMinMaxToCluster(Kind kind,std::pair<double,double>& list)
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
4.29 ~moveItemUnsorted~

Moves item unsorted from srclist to destlist mostly 
used for noise and clustercand list
returns the Iterator from list where item was deleted.

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator
   moveItemUnsorted(
       typename std::list<MEMB_TYP_CLASS*>::iterator delIt,
       std::list<MEMB_TYP_CLASS*>& srcList,
       std::list<MEMB_TYP_CLASS*>& destList, int clusterNo, int clusterType)
   {
     destList.push_back((*delIt));
     (*delIt)->setClusterNo(clusterNo);
     (*delIt)->setClusterType(clusterType);
     return  srcList.erase(delIt);
   }

/*
4.30 ~moveItemSorted~

moves item sorted from eraselist to pushlist list

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator
   moveItemSorted(Cluster* updateCluster,
                  typename std::list<MEMB_TYP_CLASS*>::iterator delIt,
                  std::list<MEMB_TYP_CLASS*>& srcList,
                  std::list<MEMB_TYP_CLASS*>& destList,
                  bool searchAndDelete, int clusterNo,
                  int clusterType, bool updateMinMax,
                  bool& moveWorked)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator destListIt 
                                                  = destList.begin(),
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

4.31 ~moveNeighborsToDestList~

Moves all neigbors from member to destination list of given index.

*/
   void moveNeighborsToDestList(Cluster* rightClsuter,
       std::pair <double,double>& clusterMinMax,
       MEMB_TYP_CLASS *member,
       std::list<MEMB_TYP_CLASS*>& srcList,
       std::list<MEMB_TYP_CLASS*>& otherSrcList,
       std::list<MEMB_TYP_CLASS*>& destList,
       bool searchAndDelete, //used for moveItemSorted
       int clusterNo, int clusterType)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator
     membNeighborIt = member->getEpsNeighborhood(true);

     while(membNeighborIt != member->getEpsNeighborhood(false)){
         if(!(*membNeighborIt)->isClusterMember()){ //point is not in cluster

             bool moveItem = false;

             typename std::list<MEMB_TYP_CLASS*>::iterator testIt=
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
4.32 ~moveClusterList~

move a list from origIndex to destIndex
only usable for left lists

*/
   void moveClusterList(std::pair<unsigned int,Kind>& destIndex,
                        std::pair<unsigned int,Kind>& origIndex,
                        Cluster* origCluster)
   {

     bool pushBackToDesInd =
         destIndex.first == getVectorSize(destIndex.second);

     if(pushBackToDesInd)
       {
         std::list<MEMB_TYP_CLASS*> emptyList;
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

   void moveMinMaxPair(std::pair<unsigned int,Kind>& destIndex,
                       std::pair<unsigned int,Kind>& origIndex,
                       Cluster* origCluster)
   {
     bool pushBackToDesInd =
         destIndex.first == getMinMaxSize(destIndex.second);

     if(pushBackToDesInd)
       {
         std::pair<double,double> minMaxInit 
                           = std::make_pair(MAX_DOUBLE,MIN_DOUBLE);
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
4.33 ~findClusterCands~

with given clusterList - search more clusterCands
returns true if more clustercands are added
member werden von clusterCandList  zu foundMembList hinzugefuegt

*/
   bool findClusterCandsforClusterList(std::list<MEMB_TYP_CLASS*>& clusterlist,
                                 std::pair<double,double>& clusterMinMax,
                                 bool clusterIsRight,
                                 TYPE* borderPoint,
                                 //if richtCluster take leftInnerPoint
                                 TYPE* secBorderPoint,
                                 std::list<MEMB_TYP_CLASS*>& clusterCandList,
                                 std::list<MEMB_TYP_CLASS*>& foundMembList,
                                 std::pair<double,double>& foundMinMax ,
                                 int clusterNo)
   {
     bool elementsfound = false, endOfClusterList = false;

     typename std::list<MEMB_TYP_CLASS*>::iterator
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
4.34 ~meltClusterLists~

Melt lists of Cluster at one side Cluster. So all  list 
which are stored in the meltingSideArray Index where melted.

*/
   void meltClusterLists(Cluster *meltingCluster,
                   std::vector<std::pair<unsigned int,Kind> > *meltingSideArray,
                   std::vector<unsigned int> minToMaxIndexes,
                   std::vector<std::pair<unsigned int,Kind> > * indexArray,
                   std::vector<std::pair<unsigned int,Kind> >& newIndices, 
                   unsigned int bothDist)
   {
     //if kind is both then meltClusterCandListWithClusterList[bothDist + i]
     std::vector<unsigned int>::iterator leftIt = minToMaxIndexes.end();
     leftIt--;
     typename std::vector<std::pair<unsigned int,Kind> >::iterator destIt,srcIt;

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
4.35 ~meltListsAndIndexOfCluster~

Melt given cluster list and they respective indexes.

*/
   void meltListsAndIndexOfCluster(Cluster* meltingCluster,
                                   std::vector<std::pair<unsigned int,Kind> >*
                                   indexArray,
                                   std::pair<unsigned int,Kind>& destIndex,
                                   std::pair<unsigned int,Kind>& srcIndex,
                                   std::vector<std::pair<unsigned int,Kind> >& 
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
     std::pair<unsigned int,Kind> newIndex=
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
4.36 ~initIndicies~

initialice newIndicies Left and Right with respectivly the first entry of
clusterToMelt indicies

*/
void initIndicies( std::vector<std::pair<unsigned int,Kind> >& newIndices,
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
             std::make_pair(ind,kind));

     }
   }

/*
4.37 ~meltIndexOfCluster~

Melt the given clusterindexes.

*/
   void meltIndexOfCluster(
          std::vector<std::pair<unsigned int,Kind> > &destIndList ,
          std::vector<std::pair<unsigned int,Kind> > &sourceIndList);

/*
4.38 ~meltListsOfCluster~

melt two list of a cluster

*/
   std::pair<unsigned int,Kind>
   meltListsOfCluster(std::pair<unsigned int,Kind>& destinationList,
                      std::pair<unsigned int,Kind>& sourceList,
                      std::vector<std::pair<unsigned int,Kind> >& newIndicies
                     );

   void meltClusterCandListWithClusterList(std::pair<unsigned int,Kind>& 
                                  destinationList,
                                  std::list<MEMB_TYP_CLASS*> &sourceIndList,
                                  std::pair <double,double>& minMaxList);

/*
4.39 ~findClListToMeltWithClustCandList~

Find cluster lists which can melt with cluster candidates list.

*/
   void findClListToMeltWithClustCandList(Cluster * rightCluster,
                              std::vector<clusterCandMelt>& clCaMeltInd,
                              std::list<MEMB_TYP_CLASS*>& clusterCandList,
                              std::pair <double,double>& clCandMinMax,
                              int bothDistLeft,
                              std::vector<std::pair<unsigned int,Kind> >&
                              newLeftIndices,
                              int bothDistRight,
                              std::vector<std::pair<unsigned int,Kind> >&
                              newRightIndices)
   {
     typename std::vector< clusterCandMelt>::iterator
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
4.40 ~meltClusterCandListWithClusterList~

first find the index to melt the given 
clusterList which is stored in clCaMeltInd then
melt the lists

*/
   void meltClusterCandListWithClusterList(clusterCandMelt& clCaMeltInd,
                                std::list<MEMB_TYP_CLASS*>& clusterCandList,
                                std::pair <double,double>& clCandMinMax,
                                int bothDist,
                                std::vector<std::pair<unsigned int,Kind> >&
                                newIndices)
   {
     std::pair<unsigned int,Kind> clusterList =
         std::make_pair(clCaMeltInd.clusterIndex,
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
4.41 ~meltClsuterCandWithClusterList~

melt foundet reachabel clusterCands with cluster lists

*/
   void meltClsuterCandWithClusterList(Cluster* rightCluster,
                            std::vector< clusterCandMelt>* clusterCandIndex,
                            unsigned int indexSize,
                            std::vector<std::pair<unsigned int,Kind> > *
                            clusterToMeltOnRightForLeftSide,
                            std::vector<std::pair<unsigned int,Kind> >&
                            newLeftIndices,
                            int bothDistLeft,
                            std::vector<std::pair<unsigned int,Kind> > *
                            clusterToMeltOnLeftForRightSide,
                            std::vector<std::pair<unsigned int,Kind> >&
                            newRightIndices,
                            int bothDistRight)
   {
     
     for(unsigned int i = 0; i< indexSize; i++){
         if(clusterCandIndex[i].size() > 0)
           {
             //add item to first clusterList
             std::list<MEMB_TYP_CLASS*> clusterCandList;
             clusterCandList.push_back(
               clusterCandIndex[i].at(0).clusterCandMember);
             double minMax = 
             clusterCandIndex[i].at(0).clusterCandMember->getYVal();
             std::pair <double,double> clCandMinMax 
                                      = std::make_pair(minMax,minMax);
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
4.42 ~findListToMeltWithClusterCand~

Retunrns the index from found cluster list
to melt with clustercand.

*/
   int
   findListToMeltWithClusterCand(clusterCandMelt& clCaMeltInd,
                  std::pair<unsigned int,Kind>& clusterList,
                  int bothDist,
                  std::vector<std::pair<unsigned int,Kind> >& newIndices )
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
4.43 ~getCorrectListIndex~

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
4.44 ~findLastIndex~

Finds the last Index in newIndices.
preconditions: clusterList must be initialized

*/
   int findLastIndex(std::pair<unsigned int,Kind>& clusterList,
                     std::vector<std::pair<unsigned int,Kind> >& newIndices,
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
4.45 ~updateClusterToMelt~

Used for clustercands.
Melt all cluster list which are not melted yet.

*/
   void updateMeltedCluster(Cluster * rightCluster,
                  std::vector<clusterCandMelt>& clCaMeltInd,
                  std::vector<std::pair<unsigned int,Kind> >*
                  clusterToMeltOnRightForLeftSide,
                  int bothDistLeft,
                  std::vector<std::pair<unsigned int,Kind> >& newLeftIndices,
                  std::vector<std::pair<unsigned int,Kind> >*
                  clusterToMeltOnLeftForRightSide,
                  int bothDistRight,
                  std::vector<std::pair<unsigned int,Kind> >& newRightIndices)
   {
     typename std::vector< clusterCandMelt>::iterator
     membIt = clCaMeltInd.begin();

     std::vector<clusterCandMelt> leftMembers;
     std::vector<clusterCandMelt> rightMembers;
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

     typename std::vector< clusterCandMelt>::iterator
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
4.46 ~memberHasSameLastIndex~

Compare a clusterCand member list if the indexes have the same
last index. If the result is true then the lists are melted

*/
   bool meltClusterCandClusterWithList(Cluster* meltingCluster,
                       std::vector<clusterCandMelt>& members,
                       std::vector<std::pair<unsigned int,Kind> >& srcIndices,
                       std::vector<std::pair<unsigned int,Kind> > * indexArray,
                       std::vector<std::pair<unsigned int,Kind> >& meltIndices,
                       unsigned int bothDist
   )
   {

     typename std::vector< clusterCandMelt>::iterator
     membIt = members.begin(),
     oldMembIt = members.begin();

     std::pair<unsigned int,Kind> index, oldIndex;
     bool allMembersEqual = true;

     if(membIt != members.end()){
       
         oldIndex = 
         std::make_pair((*oldMembIt).clusterIndex, (*oldMembIt).clusterKind);
         findLastIndex(oldIndex,srcIndices,bothDist);
         //      lowestIndex = oldIndex;
         membIt++;
         while(membIt!=members.end())
           {
             index = std::make_pair((*membIt).clusterIndex,
                                    (*membIt).clusterKind);
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
                 std::pair<unsigned int,Kind> destIndex,srcIndex;

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
4.47 ~compareLeftWithRightList~

Compare two committed list if there are member who can merge together.
If isNewClusterCand is true the a new list is created 
  - mostly when search Cluster Candidates
  
*/
   bool compareLeftWithRightList(TYPE* leftInnerPoint,
                  TYPE* rightInnerPoint,
                  bool isNewClusterCand,
                  std::list<MEMB_TYP_CLASS*>& leftList ,
                  std::list<MEMB_TYP_CLASS*>& rightList,
                  std::vector<std::list<MEMB_TYP_CLASS*> >& retClusterCand,
                  std::vector<std::pair<double,double> >& 
                  retClusterCandMinMax,
                  bool leftListIsClusterCandList,
                  bool rightListIsClusterCandList,
                  Cluster* rightCluster = 0);

/*

4.48 ~testClusterCandListsOnClusters~

Compare clustercand list with clusterlist and store clustercands
in correct clusterlist.

*/
   void testClusterCandListsOnClusters(Cluster* rightCluster,
                       TYPE* leftInnerPoint,
                       TYPE* rightInnerPoint,
                       std::list<MEMB_TYP_CLASS*>& leftList ,
                       std::list<MEMB_TYP_CLASS*>& rightList,
                       std::vector<std::list<MEMB_TYP_CLASS*> >& 
                       retClusterCand,
                       std::vector<std::pair<double,double> >& 
                       retClusterCandMinMax)
   {

     std::list<MEMB_TYP_CLASS*> leftTmpList, rightTmpList;

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
4.49 ~compareClusterCandsWithoppositeList~

Compare clustercand list with given clusterlist and save 
indexes if the lists could be melt.

*/
   void compareClusterCandsWithOppositeList(TYPE* leftInnerPoint,
                               TYPE* rightInnerPoint,
                               std::list<MEMB_TYP_CLASS*>& 
                               clusterCandList,
                               bool listIsRight,
                               std::list<MEMB_TYP_CLASS*>& compList,
                               Kind compKind, int compIndex,
                               int bothIndOffset,
                               std::vector< clusterCandMelt>* 
                               clCandMelt)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator
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
4.50 ~insertIndexToClusterCandToMelt~

similar to insertIndexToClusterToMelt

*/
   void insertIndexToClusterCandToMelt (int minIndex,
                          clusterCandMelt& newItem,
                          std::vector< clusterCandMelt>* clCandMelt)
   {
     bool found = false;

     typename std::vector< clusterCandMelt>::iterator
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
4.51 ~compareMemberWithList~

Similar to compareLeftWithRightList.
Compare points with lists and return true if neighbor is in epsNeighborhood

*/
   bool compareMemberWithList(TYPE* leftInnerPoint,
                              TYPE* rightInnerPoint,
                              MEMB_TYP_CLASS* member,
                              bool listIsRight,
                              std::list<MEMB_TYP_CLASS*>& compList)
   {
     bool endOfList= false, distGreaterEps = false;
     typename std::list<MEMB_TYP_CLASS*>::iterator itList=
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
4.52 ~initIteratorOfList~

initializes an iterator for a given list

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator
   initIteratorOfList(TYPE* leftInnerPoint,
                      TYPE* rightInnerPoint,
                      bool listIsRight,
                      bool& endOfList, bool& distGreaterEps,
                      std::list<MEMB_TYP_CLASS*>& compList)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator itList;
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
4.53 ~updateIterator~

updates a given Iterator

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator
   updateIterator(TYPE* leftInnerPoint,
                  TYPE* rightInnerPoint,
                  bool listIsRight,
                  bool& endOfList, bool& distGreaterEps,
                  std::list<MEMB_TYP_CLASS*>& compList,
                  typename std::list<MEMB_TYP_CLASS*>::iterator itList)
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
4.54 ~compareSrcListFromItWithRightList~

Compare two list together. If distance of two points is 
less then EPS then returns iterator of destination  point.

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator
   compareSrcListFromItWithRightList(TYPE* leftInnerPoint,
                       TYPE* rightInnerPoint,
                       std::list<MEMB_TYP_CLASS*>& destList ,
                       typename std::list<MEMB_TYP_CLASS*>::iterator
                       _destIt,
                       std::list<MEMB_TYP_CLASS*>& srcList,
                       int clusterNo, bool considerClusterNo,
                       bool sortedSrcList,
                       bool updateN)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator srcIt = srcList.begin();
     typename std::list<MEMB_TYP_CLASS*>::iterator destIt =
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
                     std::list<MEMB_TYP_CLASS*> neighborListLeft,
                                                neighborListRight ;
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

/*
4.54 ~compareSrcListFromItWithRightList~

Compare two list together. If distance of two points is 
less then EPS then returns iterator of destination  point.

*/ 
   typename std::list<MEMB_TYP_CLASS*>::iterator
   compareSrcListFromItWithLEFTList(TYPE* leftInnerPoint,
                     TYPE* rightInnerPoint,
                     std::list<MEMB_TYP_CLASS*>& destList ,
                     typename std::list<MEMB_TYP_CLASS*>::iterator
                     _destIt,
                     std::list<MEMB_TYP_CLASS*>& srcList,
                     int clusterNo, bool considerClusterNo,
                     bool sortedSrcList,
                     bool updateNeigb)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator srcIt = --srcList.end();
     typename std::list<MEMB_TYP_CLASS*>::iterator destIt =
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
4.55 ~concatPointsfromList~

Concat the two points from iterators 
- rummage eps neighborhood for appropriate members

*/
   void concatClusterCand(Cluster* rightCluster,
                          TYPE* leftInnerPoint,
                          TYPE* rightInnerPoint,
                          std::list<MEMB_TYP_CLASS*>& destList,
                          std::pair<double,double>& clusterPair,
                          MEMB_TYP_CLASS* leftMemb,
                          MEMB_TYP_CLASS* rightMemb,
                          std::list<MEMB_TYP_CLASS*>& leftList,
                          std::list<MEMB_TYP_CLASS*>& rightList,
                          int clusterNo);



/*
4.56 ~addListToCorrectClusterType~

finds out correct cluster type an push it to  correct cluster vector

*/
   void addListToCorrectClusterType(
                                    std::list<MEMB_TYP_CLASS*>& clusterList,
                                    std::pair<double,double>& clusterPair,
                                    std::pair<unsigned int,Kind>& index,
                                    bool checkReachability);

/*
4.57 ~defineDestIndexPair~

Define the new index for two concated clusterLists.
if retKind is different from leftKind then return false
  
*/
   bool defineDestIndexPair(std::list<MEMB_TYP_CLASS*>& leftList,
          std::list<MEMB_TYP_CLASS*>& rightList,//This could be a empty list
          std::pair<unsigned int,Kind>& leftIndex,
          std::pair<unsigned int,Kind>& retIndex)
   {

     typename std::list<MEMB_TYP_CLASS*>::iterator  itLeftPoint,itRightPoint;

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

     retIndex = std::make_pair(listNo,getClusterKindFromType(clusterType));
     bool indexIsEqual =
         retIndex.second == leftIndex.second;
     if(!indexIsEqual){
         findNextEmptyList(retIndex);
     }
     return indexIsEqual;
   }

/*
4.58 ~findNextEmptyList~

Find the next empty list in cluster vector.
if no list is empty retIndex==VectorSize
  
*/
   void findNextEmptyList(std::pair<unsigned int,Kind>& retIndex )
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
4.59 ~searchAndDeletItemFromList~

Search an item in given list and delete it.

*/
   typename std::list<MEMB_TYP_CLASS*>::iterator
   searchAndDeletItemFromList(
       typename std::list<MEMB_TYP_CLASS*>::iterator itemIt,
       std::list<MEMB_TYP_CLASS*>& list, bool& elemDeleted)
   {
     return
         searchAndDeletItemFromList(*itemIt,itemIt,list,elemDeleted);
   }


   typename std::list<MEMB_TYP_CLASS*>::iterator
   searchAndDeletItemFromList(MEMB_TYP_CLASS* item,
                    std::list<MEMB_TYP_CLASS*>& list,  bool& elemDeleted)
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
                 typename std::list<MEMB_TYP_CLASS*>::iterator itemIt,
                 std::list<MEMB_TYP_CLASS*>& list,  bool& elemDeleted)
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
4.60 ~updateNeighbor~

update neighborhood of given point and list.

*/
   void updateNeighborLeftPointToRightList(MEMB_TYP_CLASS* point, 
                                           std::list<MEMB_TYP_CLASS*>& destList,
                                           typename std::list<MEMB_TYP_CLASS*>::
                                           iterator listIt,
                                           TYPE* leftInnerPoint, 
                                           TYPE* rightInnerPoint)
   {
     std::list<MEMB_TYP_CLASS*> neighborList ;
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
                                           std::list<MEMB_TYP_CLASS*>& destList,
                                           typename std::list<MEMB_TYP_CLASS*>::
                                           iterator listIt,
                                           TYPE* leftInnerPoint, 
                                           TYPE* rightInnerPoint)
   {
     std::list<MEMB_TYP_CLASS*> neighborList ;
     while (listIt != --destList.begin() &&
         listIt != destList.end() &&
         (*listIt)->calcXDistanz(rightInnerPoint) <= eps){
         if((*listIt)->calcDistanz(point) <= eps){
             updateNeighbor(point,*listIt);
         }
         listIt--;
     }
   }

/*
4.61 ~updateNeighbor~

update neighborhood of  point in given lists.

*/
   void updateNeighborRightListToLeftList(std::list<MEMB_TYP_CLASS*>& leftList,
                                          std::list<MEMB_TYP_CLASS*>& rightList,
                                          bool leftListSorted,
                                          bool rightListSorted,
                                          TYPE* leftInnerPoint, 
                                          TYPE* rightInnerPoint)
   {

     std::list<MEMB_TYP_CLASS*> neighborList ;
     typename std::list<MEMB_TYP_CLASS*>::iterator
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

   bool getGreaterEps(std::list<MEMB_TYP_CLASS*>& list,
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
/*
4.62 ~updateNeighbor~

update neighborhood of given points.

*/
   void updateNeighbor(MEMB_TYP_CLASS * leftMemb, MEMB_TYP_CLASS *rightMemb,
                       std::list<MEMB_TYP_CLASS*>& neighborList);

   void updateNeighbor(MEMB_TYP_CLASS * leftMemb, MEMB_TYP_CLASS *rightMemb)
   {
     std::list<MEMB_TYP_CLASS*> neighborList;
     updateNeighbor(leftMemb,rightMemb,neighborList);
   }

/*
4.63 ~isMembInList~

When point is in list -> return true.

*/
   bool isMembInList(MEMB_TYP_CLASS *memb, std::list<MEMB_TYP_CLASS*>& list);


/*
4.64 ~testReachabilityAndSetClusterNoAtEachPoint~

Test each point if it is density reachable,
update clusterNo and Type.

*/
   bool testReachabilityAndSetClusterNoAtEachPoint(TYPE* leftOuterPoint,
                                    TYPE* rightOuterPoint,
                                    std::list<MEMB_TYP_CLASS*>& list,
                                    std::pair<unsigned int,Kind>& clusterPair,
                                    bool isAlreadyClusterCand,
                                    bool checkReachability = true
                                                  )
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator
     it = list.begin();
     bool allDensReachable = true;
     int listNo = clusterPair.first;
     int type = getClusterType(clusterPair.second);
     Kind kind = clusterPair.second;
       while(it!=list.end()){
         if(checkReachability && !(*it)->updateInnerPnt(minPts)){
           if(checkReachability && !(*it)->updateDensityReachable(minPts) && 
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

/*
4.65 ~moveItemToClusterCandOrNoise~

Find out if item is noise or clustercand. Then move item
to correct list.

*/

   bool moveItemToClusterCandOrNoise(TYPE* leftOuterPoint, 
                                     TYPE* rightOuterPoint,
                                     typename std::list<MEMB_TYP_CLASS*>::
                                     iterator delIt,
                                     std::list<MEMB_TYP_CLASS*>& srcList,
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
           std::list<MEMB_TYP_CLASS*> innerNeighbors;
           std::list<MEMB_TYP_CLASS*> densReachNeighbors;
           typename std::list<MEMB_TYP_CLASS*>::iterator
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
4.66 ~inserElementSorted~

Sort the elements from two lists in a new return list and
delete elements from sourceList.

*/
   void sortElemtsFromListsInNewList(std::list<MEMB_TYP_CLASS*>& sourceList,
                                     std::list<MEMB_TYP_CLASS*>& destList,
                                     std::list<MEMB_TYP_CLASS*>& retList,
                                     std::pair<unsigned int,Kind>& retIndex)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator
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
4.67 ~sortRightListToLeftList~

Sorts a given source list in a given destination list
after this method the destinationList is empty.

*/
   void sortRightListToLeftList(Cluster * origCluster,
                                std::list<MEMB_TYP_CLASS*>& sourceList,
                                std::list<MEMB_TYP_CLASS*>& destList,
                                std::pair<double,double>& sourceMinMax,
                                std::pair<double,double>& destMinMax,
                                std::pair<unsigned int,Kind>& origIndex,
                                std::pair<unsigned int,Kind>& destIndex,
                                bool kindChanged,bool changeMinMax)
   {

     typename std::list<MEMB_TYP_CLASS*>::iterator
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
4.68 ~updateClusterNoAndTypeInList~

Update for each item in destList ClusterNo and ClusterType.

*/
   void updateClusterNoAndTypeInList(std::list<MEMB_TYP_CLASS*>& destList,
                                     std::pair<unsigned int,Kind>& destIndex)
   {
     updateClusterNoAndTypeInList(destList,destIndex.first, destIndex.second);
   }

   void updateClusterNoAndTypeInList(std::list<MEMB_TYP_CLASS*>& destList,
                                     int listNo ,Kind kind)
   {
     typename std::list<MEMB_TYP_CLASS*>::iterator
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
4.69 ~copyRightClusterListToLeftCluster~

copy the untouched right Cluster lists to left Cluster

*/
   void copyRightClusterListToLeftCluster(Cluster *rightCluster,
                                          Kind kind)
   {

     for(int i=0; i<rightCluster->getVectorSize(kind); i++)
       {
         if(rightCluster->getListLength(i,kind) > 0)
           {//list is not empty
             std::pair<unsigned int,Kind> index = std::make_pair(i,kind);
             addListToCorrectClusterType(
                 rightCluster->getList(i,kind),
                 rightCluster->getMinMaxFromCluster(i,kind),
                 index,false);
           }
       }
   }

/*
4.70 ~insertIndexToClusterToMelt~

If cluster to melt found, this mehthod insert the correct index
to clusterCandToMelt indexes.
 
*/
   void insertIndexToClusterToMelt(int minIndex,
                                   int insertInd,
                                   Kind kind,
                                   std::vector<std::pair<unsigned int,Kind> >* 
                                   clusterCandToMelt,
                                   std::vector<unsigned int >& minToMaxIndexes)
   {
     std::pair<unsigned int,Kind> newItem= std::make_pair(insertInd,kind);

     if ( find(clusterCandToMelt[minIndex].begin(),
               clusterCandToMelt[minIndex].end(), newItem)
         == clusterCandToMelt[minIndex].end() )
       {
         clusterCandToMelt[minIndex].push_back(newItem);
         insertInMinToMaxIndex(minToMaxIndexes,minIndex);
       }
   }


/*
4.71 ~insertInMinToMaxIndex~

insert val in index vector -> look up for doubles
 
*/
   void insertInMinToMaxIndex(std::vector<unsigned int >& minToMaxIndexes,
                              unsigned int val )
   {
     if ( find(minToMaxIndexes.begin(),
               minToMaxIndexes.end(), val)
         == minToMaxIndexes.end() )
       minToMaxIndexes.push_back(val);
   }


/*
4.72 ~deleteEmptyLists~

find empty lists and delete them
 
*/
   void deleteEmptyLists(Kind kind)
   {


     int i=0;
     int updateItems = getVectorSize(kind);
     while( i < getVectorSize(kind))
       {
         if(getListLength(i,kind) < minPts){
           if(getListLength(i,kind)){
             //verschiebe punkte zu clusterCand Or Noise
             typename std::list<MEMB_TYP_CLASS*>::iterator it =
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
               std::pair<unsigned int,Kind> index = std::make_pair(i,kind),
                                                            destIndex;
               std::list<MEMB_TYP_CLASS*> emptyL;
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
4.73 ~pointIsInOuterBorders~

check if a point is within right or left outer boreder

*/
   bool pointIsInOuterBorders(TYPE* leftOuterPoint, TYPE* rightOuterPoint,
                              typename std::list<MEMB_TYP_CLASS*>::iterator it)
   {
     return (*it)->calcXDistanz(leftOuterPoint) <= eps ||
         (*it)->calcXDistanz(rightOuterPoint) <= eps;
   }

/*
4.74 ~listIsInYBordersOfList~

checks if a list is in y borders of other lsit
   
*/
   bool listIsInYBordersOfList(Cluster* srcCluster,
                               int srcListNo, Kind srcKind,
                               Cluster* destCluster,
                               int destListNo, Kind destKind)
   {
     std::pair<double,double> border =
     srcCluster->getMinMaxFromCluster(srcListNo,srcKind);
     return listIsInYBordersOfList(border,destCluster,destListNo,destKind);
   }

   bool listIsInYBordersOfList(std::pair<double,double>& border,
                               Cluster* destCluster,
                               int destListNo, Kind destKind)
   {

     return(valIsInYBordersOfList(
         border.first,destCluster,destListNo,destKind)
         || valIsInYBordersOfList(
             border.second,destCluster,destListNo,destKind));
   }

/*
 4.75 ~memberIsInYBordersOfList~

checks if a member is in y borders of other lsit
   
*/
   bool memberIsInYBordersOfList(MEMB_TYP_CLASS* memb, Cluster* cluster,
                                 int listNo, Kind kind)
   {
     return valIsInYBordersOfList(memb->getYVal(),cluster,listNo,kind);
   }


   bool valIsInYBordersOfList(double val, Cluster* cluster,
                              int listNo, Kind kind)
   {
     //3 possibilities
     std::pair<double,double> border 
                      =cluster->getMinMaxFromCluster(listNo,kind);
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

/*
 4.76 ~deleteVector~

delete vectror of  index
   
*/   
   void deleteVector(std::vector<std::pair<unsigned int,
                     Kind> >* vecAr, unsigned int size )
   {
     for (unsigned int i = 0; i< size; i++){
         vecAr[i].clear();
     }
     //     delete[] vecAr;
   }
   void deleteVector(std::vector< clusterCandMelt>* vecAr, unsigned int size )
   {
     for (unsigned int i = 0; i< size; i++){
         vecAr[i].clear();
     }
     //         delete[] vecAr;
   }

/*
4.77 ~getClusterType~

return numerical value of cluster type

*/
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

/*
4.78 ~getClusterType~

return cluster type from numerical value

*/
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
7.79 ~calcClusterNo~

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

   unsigned int getClusterNo(std::pair<unsigned int,Kind>& list)
   {
     return getClusterNo(list.first,list.second);
   }

   int getListNoOfClusterNo(unsigned int clusterNo, Kind kind)
   {
     switch (kind){
       case NOISE:
         return NOISE_CL_NO;
       case CLUSTERCAND:
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
       default:
         return -1;
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

