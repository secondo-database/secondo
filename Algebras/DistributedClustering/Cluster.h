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
#include <iostream>
#include <cstdlib>
#include "Member.h"
#include <algorithm>


using namespace std;

/*
class Cluster
this class represents a list of clusters.

*/

namespace distributedClustering{
  
  template <class MEMB_TYP_CLASS, class TYPE>
  class Cluster{
  private:
    enum Kind {NOISE, CLUSTER};
    
    list<MEMB_TYP_CLASS*> noiseList,emptyList;
    vector<list<MEMB_TYP_CLASS*> > clusterArray; 
    //each ClusterID has his own vector
    
    double eps;
    int minPts;
    bool allPntsVsitted;
    
    
  public:
    
/*    

constructor

*/   
    Cluster( MEMB_TYP_CLASS* leftMember, 
             MEMB_TYP_CLASS* rightMember,double _eps, int _minPts);
    
/*

meltClusters
melt this Cluster with the right cluster
medianPoint and rightMedPoint are the two outer 
Points which represent the splitline

*/
    void meltClusters(Cluster* rightCluster, 
                      TYPE* medianPoint,TYPE* rightMedPoint);
    
/*
addMember
add the committed member to the cluster

*/
    void addMember(MEMB_TYP_CLASS* memb);
    
/*
 printAll
 print out the cluster information at the console

*/
    void printAll();
    
/*
 getCntMembers();

*/
    int getCntMembers(){
      int retVal =0;
      for(int i=0;i< clusterArray.size();i++){
        retVal+=clusterArray.at(i).size();
      }
      retVal+=noiseList.size();
      return retVal;
    }
    
    //find equal elements 
    bool findEqualElements(vector<MEMB_TYP_CLASS*>& membArray){
      bool retVal=false;
      for( int i=0;i<membArray.size();i++){
        MEMB_TYP_CLASS* ptr = membArray.at(i);
        int cntMemebs=0;
        //search cluster
        for(int j =0;j<clusterArray.size();j++){
          typename list<MEMB_TYP_CLASS*>::iterator it = 
          clusterArray.at(j).begin();
          
          while(it!=clusterArray.at(j).end()){
            if(ptr== (*it)){
              cntMemebs++;
            }
            it++;
          }
        }
        //serch noise
        typename list<MEMB_TYP_CLASS*>::iterator nit = noiseList.begin();
        while(nit!=noiseList.end()){
          if(ptr == (*nit)){
            cntMemebs++;
          }
          nit++;
        }
        if(cntMemebs != 1){
          retVal=true;
          cout << "anzahl von Memb:" 
          << ptr->getXVal() << " ist:" << cntMemebs << endl;
        }
      }
      return retVal;
    }
    
/*
 getClusterArraySize
 returns the quantity of Clsuters

*/
    int getClusterArraySize(){
      return clusterArray.size();
    }
    
  private:
/*
updateNeighbor
update the eps neighborhood from left and right member

*/
    void updateNeighbor(MEMB_TYP_CLASS * leftMemb, MEMB_TYP_CLASS *rightMemb);
    
    
    
/*
getListLength
return the list length from position i

*/
    int getListLength(int i,Kind kind){
      switch (kind){
        case NOISE:
          return noiseList.size();
          break;
          
        case CLUSTER:
          return clusterArray.at(i).size();
          break;
      }
      return -1;
    }
    
/*
pushMember
add a member to a choosen cluster

*/
    void pushMember(bool front,MEMB_TYP_CLASS *member,int list){
      if(front){
        clusterArray.at(list).push_front(member);
      }else{
        clusterArray.at(list).push_back(member);
      }
    }

/*
 meltListsOfCluster

*/  
    void meltListsOfCluster(int destInd, int sourceInd);
    
/*
 meltIndexOfCluster

*/
    void meltIndexOfCluster( vector<unsigned int> &destIndList ,  
                             vector<unsigned int> &sourceIndList);
    
    
/*
 printList
 print out the a choosen cluster on position pos

*/
    void printList(int pos);
    void printList(list<MEMB_TYP_CLASS*>& list);
/*
 updateClusters
 this method is used at the end of dbDacScan
 it updates each cluster relating to minPts. 
 If the cluster points are density reachable they stay in the cluster
 otherwise they are moved to noise list.

*/
    void updateClusters();
    
/*
 getList
 return the clusterList on position i

*/
    list<MEMB_TYP_CLASS*>& getList(int i, Kind kind){
      switch (kind){
        case NOISE:
          return noiseList;
          break;
          
        case CLUSTER:
          return clusterArray.at(i);
          break;
      }
      return emptyList;
    }
    
    void insertElement(typename list<MEMB_TYP_CLASS*>::iterator it,
                       MEMB_TYP_CLASS* point, int pos)
    {
      clusterArray.at(pos).insert(it,point);
    }
    
/*
 getIterator
 returns an iterator from a clusterlist or noiselist either at the beginning or at end

*/
    typename list<MEMB_TYP_CLASS*>::iterator getIterator(int list, bool begin,
                                                         Kind kind)
    {
      switch (kind){
        
        case NOISE:
          if (begin)
            return noiseList.begin();
          else
            return noiseList.end();
          break;
          
        case CLUSTER:
          if (begin)
            return clusterArray.at(list).begin();
          else
            return clusterArray.at(list).end();
          break;
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
          break;
          
        case CLUSTER:
          return clusterArray.at(list).erase(it);
          break;
      }
      return emptyList.begin();
    }
    
/*
 eraseList

*/
    void eraseList(unsigned int list)
    {
      clusterArray.erase(clusterArray.begin()+list);
    }
    
    void clearList(unsigned int list)
    {
      clusterArray.at(list).clear();
    }
/*
 pushNoise

*/
    int pushNoise(MEMB_TYP_CLASS* noisePoint, list<MEMB_TYP_CLASS*>& destList, 
                  list<MEMB_TYP_CLASS*>& noiseList,int clusterNo);
    
/*
 deleteNeighborFromNoiseList

*/
    bool deleteNeighborFromNoiseList(typename list<MEMB_TYP_CLASS*>::iterator
    neighborIt,list<MEMB_TYP_CLASS*>& noiseList);
    
    
/*
compareLeftOuterClusterWithRightInnerCluster

*/
    void compareLeftOuterClusterWithRightInnerCluster(Cluster* srcCluster,
                                                      TYPE* destBorderPoint,
                                                      TYPE* sourceBorderPoint);
    
/*
 compareLeftNoiseWithRightCluster
 
*/
    void compareLeftNoiseWithRightCluster(Cluster* destCluster,  
                                          TYPE* destBorderPoint,
                                          TYPE* sourceBorderPoint);
    
/*
meltRightClusterWithLeftNoise

*/
    int meltRightClusterWithLeftNoise(int startPosList,  Cluster* destCluster,
                                      typename list<MEMB_TYP_CLASS*>::iterator
                                      sourceListIt,
                                      TYPE* destBorderPoint);
    
    
/*
 compareLeftClusterWithRightNoise

*/
    void compareLeftClusterWithRightNoise(Cluster* destCluster,
                                          TYPE* destBorderPoint,
                                          TYPE* sourceBorderPoint);
    
/*
 meltLeftClusterWithRightNois

*/
    int meltLeftClusterWithRightNoise(int startPos, Cluster* destCluster,
                                      typename list<MEMB_TYP_CLASS*>::iterator
                                      sourceListIt,
                                      TYPE* destBorderPoint);
    
/*
 updateBorderNeighbor

*/
    void updateBorderNeighbor(typename list<MEMB_TYP_CLASS*>::iterator borderIt,
                              list<MEMB_TYP_CLASS*>& destList,
                              TYPE* leftOuterPoint, TYPE* rightOuterPoint);
    
/*
 memberAllreadyExist

*/
    bool memberAllreadyExist(typename list<MEMB_TYP_CLASS*>::iterator helpIt,
                           typename list<MEMB_TYP_CLASS*>::iterator neighborIt,
                             list<MEMB_TYP_CLASS*>& destList);
    
/*
concatNoise

*/
    void concatNoise(list<MEMB_TYP_CLASS*>& destList,
                     MEMB_TYP_CLASS* leftNoisePoint,
                     MEMB_TYP_CLASS* rightNoisePoint, 
                     Cluster* cluster, int clusterNo);
    
    
/*
* initCluster
* initialize the first cluster. This method is called from a Constructor

*/
    void initCluster(bool threeMembers);
    
    
    
  };
  
/*
Constructor

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  Cluster<MEMB_TYP_CLASS,TYPE>::
  Cluster( MEMB_TYP_CLASS* leftMember, 
           MEMB_TYP_CLASS* rightMember,double _eps, int _minPts)
  {
    eps=_eps; minPts=_minPts;
    
    list<MEMB_TYP_CLASS*> clusterList;
    bool hasCluster = true;
    
    if(leftMember->calcDistanz(rightMember) <= eps){
      rightMember->addNeighbor( leftMember);
      leftMember->addNeighbor(rightMember);
      clusterList.push_back(leftMember);
      clusterList.push_back(rightMember);
    }else{ // members are noie
      hasCluster=false;
      noiseList.push_back(leftMember);
      noiseList.push_back(rightMember);
    }
    
    
    if(hasCluster){
      clusterArray.push_back(clusterList);
      updateClusters();
    }
    
  }
/*
* meltClusters
* melt this Cluster with the right cluster
* medianPoint and rightMedPoint are the two outer Points which 
* represent the splitline

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::
  meltClusters(Cluster * rightCluster, 
               TYPE* leftOuterPoint, TYPE* rightOuterPoint){
    
    typename list<MEMB_TYP_CLASS*>::iterator 
    itLeft,itRight,itList,itNoiseLeft, itNoiseRight, 
    itHelpNoiseLeft, itHelpNoiseRight;
    
    bool  endOfLeftNoiseList=false, endOfRightNoiseList=false;
    
    //check left noise with right cluster
    if(rightCluster->getClusterArraySize() && noiseList.size()){
      compareLeftNoiseWithRightCluster(
        rightCluster,rightOuterPoint,leftOuterPoint);
    }
    
    //check right noise with left cluster
    if(getClusterArraySize() && 
      rightCluster->getListLength(-1,NOISE)){
      compareLeftClusterWithRightNoise(
        rightCluster,leftOuterPoint,rightOuterPoint);
    }
    
    
    // check left noise with right noise
    endOfLeftNoiseList=false;
    endOfRightNoiseList=false;
    itNoiseLeft = noiseList.begin();
    while(!endOfRightNoiseList && itNoiseLeft!=noiseList.end()){
      if((*itNoiseLeft)->calcXDistanz(leftOuterPoint) <=eps){
        itNoiseRight = rightCluster->getIterator(-1,true,NOISE);
        while(!endOfLeftNoiseList && itNoiseRight!=
          rightCluster->getIterator(-1,false,NOISE) )
        {
          
          if((*itNoiseLeft)->calcDistanz(*itNoiseRight) <= eps){
            
            list<MEMB_TYP_CLASS*> clusterList;
            concatNoise(clusterList,*itNoiseLeft,*itNoiseRight,
                        rightCluster,clusterArray.size());
            updateNeighbor(*itNoiseRight,*itNoiseLeft);
            
            itNoiseLeft = noiseList.erase(itNoiseLeft);
            if(itNoiseLeft==noiseList.end()){
              endOfLeftNoiseList=true;
            }
            itNoiseRight = rightCluster->eraseItem(-1,itNoiseRight,NOISE);
            if(itNoiseRight==getIterator(-1,false,NOISE) ){
              endOfRightNoiseList=true;
            }
            clusterArray.push_back(clusterList);
          }else
            itNoiseRight++;
        }
      }
      itNoiseLeft++;
    }
    
    //copy rest right noise list to left noise list
    itNoiseRight = rightCluster->getIterator(-1,true,NOISE);
    while(rightCluster->getListLength(-1,NOISE)){
      noiseList.push_back((*itNoiseRight));
      itNoiseRight= rightCluster->eraseItem(-1,itNoiseRight,NOISE);
    }
    
    
    //compare left cluster with right cluster
    compareLeftOuterClusterWithRightInnerCluster(
      rightCluster,leftOuterPoint,rightOuterPoint);
    
    //append the rest of the right clusters and clustercandidates
    int i =0;
    for ( i=0;i<rightCluster->getClusterArraySize();i++){
      if(rightCluster->getListLength(i,CLUSTER)){
        clusterArray.push_back(rightCluster->getList(i,CLUSTER));
      }
    }
    
    delete rightCluster;
    updateClusters();
  }
  
/*
* addMember
* add the committed member to the cluster

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::addMember(MEMB_TYP_CLASS* member){
    //  MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(memb);
    int memberList = 0;
    bool isClusterMember = false, memberAdded=false;
    typename list<MEMB_TYP_CLASS*>::iterator itLeft,itNoiseLeft;
    
    for(unsigned int i=0;i<clusterArray.size() && !isClusterMember ;i++){
      memberList=i;
      itLeft =clusterArray.at(i).end();
      --itLeft;
      while(itLeft != --clusterArray.at(i).begin()&& !isClusterMember){
        int dist =(*itLeft)->calcDistanz(member);
        if(dist<=eps){
          isClusterMember = true;
          clusterArray.at(i).push_back(member);
          updateNeighbor(*itLeft,member);
        }
        --itLeft;
      }
    }
    
    //check noiseList
    itNoiseLeft = noiseList.begin();
    while(itNoiseLeft!=noiseList.end() && !memberAdded) {
      if((*itNoiseLeft)->calcDistanz(member) <=eps){
        if(isClusterMember){
          pushNoise(*itNoiseLeft,this->getList(memberList,CLUSTER),
                    this->getList(-1,NOISE),memberList);
          updateNeighbor(*itNoiseLeft,member);
        }else{
          list<MEMB_TYP_CLASS*> clusterList;
          clusterList.push_back(member);
          pushNoise(*itNoiseLeft,clusterList,
                    this->getList(-1,NOISE),clusterArray.size());
          updateNeighbor(*itNoiseLeft,member);
          clusterArray.push_back(clusterList);
          memberList= clusterArray.size()-1;
          isClusterMember=true;
        }
        itNoiseLeft=noiseList.erase(itNoiseLeft);
        memberAdded=true;
      }else{
        ++itNoiseLeft;
      }
    }
    
    if(!isClusterMember)
      noiseList.push_back(member);
    
    updateClusters();
  }
  
/*
* updateNeighbor
* update the eps neighborhood from left and right member

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::
  updateNeighbor(MEMB_TYP_CLASS * leftMemb, MEMB_TYP_CLASS *rightMemb){
    //     add left and right as neighbor
    if(leftMemb->calcDistanz(rightMemb)<=eps){
      //control if neighbor already existing
      if(!leftMemb->existNeighbor(rightMemb)){
        leftMemb->addNeighbor(rightMemb);
      }
      if(!rightMemb->existNeighbor(leftMemb)){
        rightMemb->addNeighbor(leftMemb);
      }
      
      typename list<MEMB_TYP_CLASS*>::iterator leftIt,rightIt;
      
      rightIt=rightMemb->getEpsNeighborhood(true);
      while(rightIt != rightMemb->getEpsNeighborhood(false)){
        if(leftMemb->calcDistanz(*rightIt)<=eps){
          if(!leftMemb->existNeighbor(*rightIt)){
            leftMemb->addNeighbor(*rightIt);
          }
          if(!(*rightIt)->existNeighbor(leftMemb)){
            (*rightIt)->addNeighbor(leftMemb);
          }
        }
        ++rightIt;
      }
      
      
      leftIt=leftMemb->getEpsNeighborhood(true);
      while(leftIt!=leftMemb->getEpsNeighborhood(false)){
        if(rightMemb->calcDistanz(*leftIt)<=eps){
          if(!rightMemb->existNeighbor(*leftIt)){
            rightMemb->addNeighbor(*leftIt);
          }
          if(!(*leftIt)->existNeighbor(rightMemb)){
            (*leftIt)->addNeighbor(rightMemb);
          }
        }
        ++leftIt;
      }
    }
    
  }
  
/*
* printAll
* print out the cluster information at the console

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::printAll(){
    cout << "--------------------cluster:--------------------" << endl;
    cout << "Min Points = " << minPts << "\nEPS = " << eps << "\n "<< endl;
    for( unsigned int i = 0; i<clusterArray.size(); i++){
      cout << "Cluster Number " << i << ":" << endl;
      printList(i);
      cout << endl;
    }
    typename list<MEMB_TYP_CLASS*>::iterator it = noiseList.begin();
    cout << "Noise List: ";
    for (;it!=noiseList.end();it++){
      (*it)->printPoint(); cout  << ", ";
    }
    
    cout << endl;
    cout << "-----" << endl;
    cout << "epsNeighborhood for eps= "<< eps << " and minPts = " 
    << minPts << ": \n " << endl;
    for(unsigned int i = 0; i<clusterArray.size(); i++){
      cout << "epsNeighborhood in Cluster Number " << i << ": " << endl;
      cout << "------------------------------------"<< endl;
      typename list<MEMB_TYP_CLASS*>::iterator it =clusterArray.at(i).begin();
      for (;it!=clusterArray.at(i).end();it++){
        cout << "Member: "; (*it)->printPoint();
        //cout << " isDensityReachable = " << (*it)->isDensityReachable();
        cout <<" Cnt neighbors :" << (*it)->getCountNeighbors() <<
        "  ClusterNo= " << (*it)->getClusterNo() << endl;
        cout <<" neighbors :";
        typename list<MEMB_TYP_CLASS*>::iterator nIt = 
        (*it)->getEpsNeighborhood(true);
        while (nIt!= (*it)->getEpsNeighborhood(false)){
          (*nIt)->printPoint(); cout << " DR: "
          << (*nIt)->isDensityReachable() <<
          " ClNo: " << (*nIt)->getClusterNo() << ", ";
          nIt++;
        }
        cout <<endl;
        
      }
      cout << "\n" <<endl;
    }
    
    cout << "epsNeighborhood in Noise List : " << endl;
    cout << "------------------------------------"<< endl;
    typename list<MEMB_TYP_CLASS*>::iterator NoisIt =noiseList.begin();
    for (;NoisIt!=noiseList.end();NoisIt++){
      cout << "Member: "; (*NoisIt)->printPoint();
      //cout << " isDensityReachable = " << (*NoisIt)->isDensityReachable();
      cout <<" Cnt neighbors :"<< (*NoisIt)->getCountNeighbors() <<
      "  ClusterNo= " << (*NoisIt)->getClusterNo() << endl;
      cout <<" neighbors :";
      typename list<MEMB_TYP_CLASS*>::iterator nIt = 
      (*NoisIt)->getEpsNeighborhood(true);
      while (nIt!= (*NoisIt)->getEpsNeighborhood(false)){
        (*nIt)->printPoint(); cout <<  " DR: "
        << (*nIt)->isDensityReachable() <<
        " ClNo: " << (*nIt)->getClusterNo() << ", ";
        nIt++;
      }
      cout <<endl;
      
    }
    
    
    cout << "length of clusters and noiseList" << endl;
    for( unsigned int i = 0; i<clusterArray.size(); i++){
      cout << "Cluster Number " << i << ": "<<
      clusterArray.at(i).size() << endl;
    }
    cout << endl;
     it = noiseList.begin();
    cout << "Noise List: " << noiseList.size() << endl;
    
    
    
    cout << "\n--------------------cluster END --------------------\n" << endl;
  }
  
/*
* printList
* print out the a choosen cluster on position pos

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::printList(int pos){
    for(typename list<MEMB_TYP_CLASS*>::iterator it = 
      clusterArray.at(pos).begin();
        it!=clusterArray.at(pos).end();it++){
      (*it)->printPoint();
    cout /*<< ": ClNo:"<< (*it)->getClusterNo() */<< ", ";
        }
        cout << "\n";
  }
  
/*
* printList
* print a committed list

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::printList(list<MEMB_TYP_CLASS*>& dlist){
    cout << "LIST: ";
    for(typename list<MEMB_TYP_CLASS*>::iterator it = dlist.begin();
        it!=dlist.end();it++){
      (*it)->printPoint();
    cout /*<< ": ClNo:"<< (*it)->getClusterNo()*/ << ", ";
        }
        cout << "\n";
  }
  
/*
 updateClusters
 this method is used at the end of dbDacScan
 it updates each cluster relating to minPts. 
 If the cluster points are density reachable they stay in the cluster
 otherwise they are moved to noise list.

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::updateClusters()
  { 
    typename list<MEMB_TYP_CLASS*>::iterator clusterIt;
    unsigned int pos = 0;
    int clusterNo;
    
    while (pos < clusterArray.size()){
      clusterIt = clusterArray.at(pos).begin();
      clusterNo = pos;
      
      //TODO NEW
      if(clusterArray.at(pos).size()< minPts){
        while(clusterIt!=clusterArray.at(pos).end()){
          (*clusterIt)->setClusterNo(-1);
          noiseList.push_back(*clusterIt);
          clusterIt = clusterArray.at(pos).erase(clusterIt);
        }
      }else{
        
        
        while(clusterIt!=clusterArray.at(pos).end()){
          if(!(*clusterIt)->updateInnerPnt(minPts)){
            if(!(*clusterIt)->updateDensityReachable(minPts)){
              // update ClusterNO to Noise
              (*clusterIt)->setClusterNo(-1); // clusterNo -1 is noise 
              //add point to noise and delete from cluster
              noiseList.push_back(*clusterIt);
              clusterIt = clusterArray.at(pos).erase(clusterIt);
              
            } else {
              (*clusterIt)->setClusterNo(clusterNo);
              clusterIt++;
            }
          } else {
            (*clusterIt)->setClusterNo(clusterNo);
            clusterIt++;
          }
        }
      }
      // delete clusterlist at position pos if it is empty
      if(!clusterArray.at(pos).size()){
        clusterArray.erase(clusterArray.begin()+pos);
      }else{
        pos++;
      }
    }
    // test Noise neighborhood if any point is densityReachable now
    typename list<MEMB_TYP_CLASS*>::iterator noiseIt = 
    this->getIterator(-1,true,NOISE);
    
    typename list<MEMB_TYP_CLASS*>::iterator neighborIt;
    typename list<MEMB_TYP_CLASS*>::iterator destListIt;
    bool isClusterMember = false,endOfNoiseList=false;
    while(!isClusterMember && noiseIt != this->getIterator(-1,false,NOISE)){
      neighborIt = (*noiseIt)->getEpsNeighborhood(true);
      
      while(!isClusterMember /*&& !endOfNoiseList*/
        && noiseIt != this->getIterator(-1,false,NOISE)
        && neighborIt != (*noiseIt)->getEpsNeighborhood(false)
      ){
        if((*neighborIt)->updateInnerPnt(minPts)){
          // find neighborIt.Point in cluster list pos
          for(unsigned int i=0; i<getClusterArraySize() &&
            !endOfNoiseList ;i++)
          {
            clusterNo = i;
            double pointXVal= (*neighborIt)->getXVal();
            double leftBorder = 
            (*(this->getIterator(i,true,CLUSTER)))->getXVal();
            double rightBorder =
            (*(--this->getIterator(i,false,CLUSTER)))->getXVal();
            if(pointXVal <= leftBorder && pointXVal >= rightBorder){
              //find position in destList
              destListIt = this->getIterator(i,true,CLUSTER);
              while(destListIt != this->getIterator(i,false,CLUSTER) &&
                (*destListIt)->getXVal() >= (*noiseIt)->getXVal())
              {
                destListIt++;
              }
              //add to cluster pos
              clusterArray.at(i).insert(destListIt,*noiseIt);
              //update clusterNo and density reachable
              (*noiseIt)->setClusterNo(clusterNo);
              (*noiseIt)->updateDensityReachable(minPts);
              //delete from noise
              noiseIt = this->eraseItem(-1,noiseIt,NOISE);
              if(noiseIt == this->getIterator(-1,false,NOISE)){
                endOfNoiseList = true;
              }
            }
          }
          isClusterMember=true;
        }else{
          neighborIt++;
        }
      }
      noiseIt++;
    }
  }
  
  
  template <class MEMB_TYP_CLASS, class TYPE>
  int
  Cluster<MEMB_TYP_CLASS,TYPE>::pushNoise(MEMB_TYP_CLASS* noisePoint, 
                                          list<MEMB_TYP_CLASS*>& destList, 
                                          list<MEMB_TYP_CLASS*>& noiseList,
                                          int clusterNo)
  {
    typename list<MEMB_TYP_CLASS*>::iterator destListIt = destList.begin();
    int cntNoisePnt =1;
    //input noisePoint in sorted List
    while(destListIt != destList.end() && 
      (*destListIt)->getXVal() >= noisePoint->getXVal()) {
      destListIt++;
    }
    destList.insert(destListIt,noisePoint);
    noisePoint->setClusterNo(clusterNo);
    
    //insert neighbors form noise point
    typename list<MEMB_TYP_CLASS*>::iterator 
    neighborIt = noisePoint->getEpsNeighborhood(true);
    while(neighborIt != noisePoint->getEpsNeighborhood(false)){
      if(!(*neighborIt)->isClusterMember()){
        
        //if end is reached - go one step back
        if(destListIt==destList.end()) 
          // if destListIt is on last position you can´t acces any point
          destListIt--;
        
        //first go back in list until listPoint<neighborPoint
        while(destListIt != destList.begin() && 
          (*destListIt)->getXVal() < (*neighborIt)->getXVal()){
          destListIt--;
        }
        //then serch position to insert
        while( destListIt !=destList.end() && 
          (*destListIt)->getXVal() >= (*neighborIt)->getXVal()){
          destListIt++;
        }
        
        destList.insert(destListIt,*neighborIt);
        (*neighborIt)->setClusterNo(clusterNo);
        cntNoisePnt++;
        deleteNeighborFromNoiseList(neighborIt,noiseList);
      }
      neighborIt++;
    }
    return cntNoisePnt;
  }
  
  
/*
* concatNoise

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::concatNoise(list<MEMB_TYP_CLASS*>& destList, 
                                            MEMB_TYP_CLASS* leftNoisePoint,
                                            MEMB_TYP_CLASS* rightNoisePoint, 
                                            Cluster* cluster, int clusterNo)
  {
    //     beginn with smallest item
    typename list<MEMB_TYP_CLASS*>::iterator 
    leftNoiseNeighborIt = leftNoisePoint->getEpsNeighborhood(true);
    typename list<MEMB_TYP_CLASS*>::iterator 
    rightNoiseNeighborIt = rightNoisePoint->getEpsNeighborhood(true);
    
    //beginn with left noise points
    destList.push_back(leftNoisePoint);
    leftNoisePoint->setClusterNo(clusterNo);
    
    typename list<MEMB_TYP_CLASS*>::iterator destListIt = destList.begin();
    while(leftNoiseNeighborIt != leftNoisePoint->getEpsNeighborhood(false)){
      if(!(*leftNoiseNeighborIt)->isClusterMember()){ //point is not in cluster 
        
        //first go back in list until listPoint<neighborPoint
        if(destListIt== destList.end()) 
          // if it is on last position you can´t acces any point
          destListIt--;
        while(destListIt != destList.begin() && 
          (*destListIt)->getXVal() < (*leftNoiseNeighborIt)->getXVal()){
          destListIt--;
        }
        //then serch position to insert
        while(destListIt != destList.end() && 
          (*destListIt)->getXVal() >= (*leftNoiseNeighborIt)->getXVal()){
          destListIt++;
        }
        destList.insert(destListIt,*leftNoiseNeighborIt);
        (*leftNoiseNeighborIt)->setClusterNo(clusterNo);
        //delete point from noiseList
        deleteNeighborFromNoiseList(leftNoiseNeighborIt,
                                    this->getList(-1,NOISE));
      }
      
      leftNoiseNeighborIt++;
    }
    //add right noise points
    //beginn with left noise points
    
    const typename list<MEMB_TYP_CLASS*>::iterator borderIt =--destList.end();
    //    const int borderPos = destList.size();
    
    destList.push_back(rightNoisePoint);
    rightNoisePoint->setClusterNo(clusterNo);
    destListIt = destList.end();
    
    while(rightNoiseNeighborIt != rightNoisePoint->getEpsNeighborhood(false)){
      if(!(*rightNoiseNeighborIt)->isClusterMember()){
        //point is not in cluster //TODO NEW
        
        if(destListIt== destList.end()) 
          // if it is on last position you can´t acces any point
          destListIt--;
        //first go back in list until listPoint<neighborPoint
        while( destListIt != destList.begin() &&
          (*destListIt)->getXVal() < (*rightNoiseNeighborIt)->getXVal()){
          destListIt--;
        }
        //then serch position to insert
        while(destListIt != destList.end() &&
          (*destListIt)->getXVal() >= (*rightNoiseNeighborIt)->getXVal()){
          destListIt++;
        }
        
        destList.insert(destListIt,*rightNoiseNeighborIt);
        (*rightNoiseNeighborIt)->setClusterNo(clusterNo);
        //delete point from noiseList
        deleteNeighborFromNoiseList(
          rightNoiseNeighborIt,cluster->getList(-1,NOISE));
      }
      rightNoiseNeighborIt++;
    }
    
    // control points descending from borderIt if there are new neighbors
    typename list<MEMB_TYP_CLASS*>::iterator rightBorderIt= borderIt;
    rightBorderIt++;
    updateBorderNeighbor(borderIt,destList,
                         (*borderIt)->getPoint(),
                         (*rightBorderIt)->getPoint());
  }
  
  
  template <class MEMB_TYP_CLASS, class TYPE>
  bool
  Cluster<MEMB_TYP_CLASS,TYPE>::deleteNeighborFromNoiseList(
    typename list<MEMB_TYP_CLASS*>::iterator neighborIt,
    list<MEMB_TYP_CLASS*>& noiseList)
  {
    typename list<MEMB_TYP_CLASS*>::iterator noiseListIt = noiseList.begin();
    bool deleted = false;
    while(!deleted && noiseListIt != noiseList.end()){
      if(*neighborIt == *noiseListIt){
        deleted = true;
        noiseListIt = noiseList.erase(noiseListIt);
      }else{
        noiseListIt++;
      }
    }
    return deleted;
  }
  
  

/*
* compareLeftNoiseWithRightCluster

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::
  compareLeftNoiseWithRightCluster(Cluster* rightCluster, 
                                   TYPE* rightOuterPoint,
                                   TYPE* leftOuterPoint)
  {
    typename list<MEMB_TYP_CLASS*>::iterator clusterListIt, noiseListIt;
    int pos = -1, meltedClusters=0,cntNoisePnt=0;//for noise lists
    
    
    //for each cluster element on right side
    for(int j=0; j<rightCluster->getClusterArraySize();j++){
      clusterListIt=rightCluster->getIterator(j,true,CLUSTER);
      typename list<MEMB_TYP_CLASS*>::iterator 
      borderIt = rightCluster->getIterator(j,true,CLUSTER);
      //check clusterListIt with rightOuterPoint
      if(clusterListIt != rightCluster->getIterator(j,false ,CLUSTER) 
        && (*clusterListIt)->calcXDistanz(rightOuterPoint) <=eps)
      {
        while(clusterListIt != rightCluster->getIterator(j,false,CLUSTER))
        {
          noiseListIt = this->getIterator(pos,false,NOISE);
          noiseListIt--;
          while(noiseListIt != --(this->getIterator(pos,true,NOISE))) 
          {
            cntNoisePnt=0;
            meltedClusters=0;
            if((*noiseListIt)->calcDistanz(*clusterListIt) <= eps)
            {
              cntNoisePnt=
              pushNoise(*noiseListIt,rightCluster->getList(j,CLUSTER),
                        this->getList(pos,NOISE),j);
              updateNeighbor(*noiseListIt,*clusterListIt);
              
              meltedClusters=
              meltRightClusterWithLeftNoise(j+1,rightCluster,
                                            noiseListIt, rightOuterPoint);
              
              noiseListIt = this->eraseItem(pos,noiseListIt,NOISE);
              
              if(meltedClusters){ //if clusters are melted make some changes
                borderIt = rightCluster->getIterator(j,true,CLUSTER);
                while(cntNoisePnt){
                  borderIt++;
                  cntNoisePnt--;
                }
                // set ClusterListIt new
                clusterListIt=rightCluster->getIterator(j,true,CLUSTER);
              }
              
              updateBorderNeighbor(borderIt,
                                   rightCluster->getList(j,CLUSTER),
                                   leftOuterPoint,rightOuterPoint);
            }
            noiseListIt--;
          }
          clusterListIt++;
        }
      }
    }
  }
  
  
/*
* meltRightClusterWithLeftNoise

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  int
  Cluster<MEMB_TYP_CLASS,TYPE>::
  meltRightClusterWithLeftNoise(int startPosList, 
                                Cluster* rightCluster,
                                typename list<MEMB_TYP_CLASS*>::iterator 
                                noiseListIt,
                                TYPE* rightOuterPoint)
  {
    typename list<MEMB_TYP_CLASS*>::iterator clusterListIt;
    int helpInd=startPosList, destList = startPosList - 1, retVal=0;
    bool listAppended = false;
    
    while(helpInd < rightCluster->getClusterArraySize())
    {
      clusterListIt=rightCluster->getIterator(helpInd,true,CLUSTER);
      listAppended=false;
      if(clusterListIt != rightCluster->getIterator(helpInd,false,CLUSTER) &&
        (*clusterListIt)->calcXDistanz(rightOuterPoint) <=eps)
      {
        while(!listAppended && 
          clusterListIt != rightCluster->getIterator(helpInd,false, CLUSTER))
        {
          if((*noiseListIt)->calcDistanz(*clusterListIt) <=eps){
            updateNeighbor(*noiseListIt,*clusterListIt);
            rightCluster->meltListsOfCluster(destList, helpInd);
            //TODO increment clusters
            //             TODO TEST
            rightCluster->eraseList(helpInd);
            retVal++; //TODO NEW TEST
            listAppended = true;
            
          }else{
            ++clusterListIt;
          }
        }
      }
      if(!listAppended)
        helpInd++;
    }
    
    return retVal; // so much clusters are appended 
  }
  
  
  //compareLeftClusterWithRightNoise
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::
  compareLeftClusterWithRightNoise(Cluster* rightCluster, TYPE* leftOuterPoint,
                                   TYPE* rightOuterPoint)
  {
    typename list<MEMB_TYP_CLASS*>::iterator clusterListIt, noiseListIt;
    int pos=-1,meltedClusters=0,cntNoisePnt=0;
    //     bool endOfList = false,noiseAdded=false;
    
    for(int j=0; j<this->getClusterArraySize();j++){
      clusterListIt=this->getIterator(j,false,CLUSTER);
      clusterListIt--;
      
      typename list<MEMB_TYP_CLASS*>::iterator borderIt
      = --this->getIterator(j,false,CLUSTER);
      
      if(clusterListIt != --this->getIterator(j,true,CLUSTER) &&
        (*clusterListIt)->calcXDistanz(leftOuterPoint) <=eps)
      { //if distance > eps -> cluster is out of eps range
        while(clusterListIt != --(this->getIterator(j,true,CLUSTER)))
        {
          noiseListIt = rightCluster->getIterator(pos,true,NOISE);
          while(noiseListIt != rightCluster->getIterator(pos,false,NOISE)) 
          {
            cntNoisePnt=0;
            meltedClusters=0;
            if((*noiseListIt)->calcDistanz(*clusterListIt) <= eps)
            {
              cntNoisePnt=
              pushNoise(*noiseListIt,this->getList(j,CLUSTER),
                        rightCluster->getList(pos,NOISE),j);
              updateNeighbor(*noiseListIt,*clusterListIt);
              
              meltedClusters=
              meltLeftClusterWithRightNoise(j+1,this,
                                            noiseListIt, leftOuterPoint);
              
              noiseListIt = rightCluster->eraseItem(pos,noiseListIt,NOISE);
              
              if(meltedClusters){//if clusters are melted make some changes
                borderIt = --this->getIterator(j,false,CLUSTER);
                while(cntNoisePnt){
                  borderIt--;
                  cntNoisePnt--;
                }
                // set ClusterListIt new
                clusterListIt=this->getIterator(j,false,CLUSTER);
                clusterListIt--;
              }
              updateBorderNeighbor(borderIt,
                                   this->getList(j,CLUSTER),
                                   leftOuterPoint,rightOuterPoint);
            }else{
              noiseListIt++;
            }
          }
          
          clusterListIt--;
        }
      }
      
    }
    
  }
  
/*
* meltLeftClusterWithRightNoise

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  int
  Cluster<MEMB_TYP_CLASS,TYPE>::
  meltLeftClusterWithRightNoise(int startPos, Cluster* leftCluster,
                                typename list<MEMB_TYP_CLASS*>::
                                iterator noiseListIt, 
                                TYPE* leftOuterPoint)
  {
    typename list<MEMB_TYP_CLASS*>::iterator clusterListIt;
    int helpInd=startPos,destList = startPos - 1, retVal=0;
    bool listAppended = false;
    
    while(helpInd < this->getClusterArraySize())
    {
      clusterListIt=this->getIterator(helpInd,false,CLUSTER);
      clusterListIt--;
      listAppended=false;
      if(clusterListIt != this->getIterator(helpInd,true,CLUSTER) &&
        (*clusterListIt)->calcXDistanz(leftOuterPoint) <=eps)
      {
        while(!listAppended &&
          clusterListIt != --(this->getIterator(helpInd,true, CLUSTER)))
        {
          if((*noiseListIt)->calcDistanz(*clusterListIt) <=eps){
            updateNeighbor(*noiseListIt,*clusterListIt);
            this->meltListsOfCluster(destList,helpInd);
            //             TODO TEST
            clusterArray.erase(clusterArray.begin()+(helpInd));
            retVal++; //TODO NEW TEST
            listAppended = true;
            helpInd=startPos;
          }else{
            --clusterListIt;
          }
        }
      }
      if(!listAppended)
        helpInd++;
    }
    return retVal; // so much clusters are appended 
  }
  
/*
* compareLeftOuterClusterWithRightInnerCluster

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::
  compareLeftOuterClusterWithRightInnerCluster(Cluster* rightCluster,
                                               TYPE* leftOuterPoint,
                                               TYPE* rightOuterPoint)
  {
    typename list<MEMB_TYP_CLASS*>::iterator 
    itLeft,itRight,itHelpLeft,appendIt,borderIt;
    
    vector<unsigned int> clusterToMeltRight[clusterArray.size()],
    clusterToAppend[clusterArray.size()], 
    clusterToMeltLeft[rightCluster->clusterArray.size()];
    
    bool newClusterFound=false,clusterIsDensR=false, memberAdded = false, 
    clusterMelted = false,newClusterFoundInL=false;
    
    
    //check each clusterlist on left side
    for(unsigned int i=0;i<clusterArray.size();i++){
      //beginn with the most right one
      itLeft =clusterArray.at(i).end();
      itLeft--;
      
      
      if(itLeft != --clusterArray.at(i).begin() &&
        (*itLeft)->calcXDistanz(leftOuterPoint) <=eps)
      { //check outer point distance - if bigger than eps - take next list
        while(itLeft != --clusterArray.at(i).begin() &&
          (*itLeft)->calcXDistanz(leftOuterPoint) <=eps)
        {
          //           cout << "in first while " << endl;
          //           cout << "i= " << i<< endl;
          
          //compare with each cluster on the right side
          for(unsigned int j=0;j<rightCluster->getClusterArraySize() ;j++)
          {
            bool jAlreadyAppend = false;
            vector<unsigned int>::iterator it = clusterToMeltRight[i].begin();
            while(!jAlreadyAppend && it != clusterToMeltRight[i].end()){
              if(j==*it){
                jAlreadyAppend = true;
              }
              it++;
            }
            if(!jAlreadyAppend){
              newClusterFoundInL = false;
              // get this most left point from right Cluster
              itRight=rightCluster->getIterator(j,true,CLUSTER);
              
              if(!newClusterFoundInL &&
                itRight !=rightCluster->getIterator(j,false,CLUSTER) &&
                (*itRight)->calcXDistanz(rightOuterPoint) <=eps)
              {//check outer point distance -if bigger than eps->take next list 
                while(!newClusterFoundInL && 
                  itRight != rightCluster->getIterator(j,false,CLUSTER) &&
                  (*itRight)->calcXDistanz(rightOuterPoint) <= eps)
                {//get distance from left Point to right Point
                  
                  if((*itLeft)->calcDistanz(*itRight) <= eps){
                    memberAdded=true;
                    newClusterFoundInL = true;
                    
                    //TODO test dens reachability
                    
                    //remember cluster listNo
                    clusterToMeltRight[i].push_back(j);
                    clusterToMeltLeft[j].push_back(i);
                  }
                  ++itRight;
                }
              }
            }
            
          }
          --itLeft;
        }
      }
    }
    
    //melt right cluster lists
    for (unsigned int i = 0; i< clusterArray.size(); i++){
      if(clusterToMeltRight[i].size()>1)
      {
        sort(clusterToMeltRight[i].begin(),clusterToMeltRight[i].end());
        unsigned int lowInd = clusterToMeltRight[i].at(0);
        
        for(unsigned int j = 1; j<clusterToMeltRight[i].size();j++)
        {
          unsigned int highInd = clusterToMeltRight[i].at(j);
          if(rightCluster->getListLength(highInd,CLUSTER)>0 &&
            clusterToMeltLeft[highInd].size()>0)
          {
            rightCluster->meltListsOfCluster(lowInd, highInd);
            meltIndexOfCluster(clusterToMeltLeft[lowInd],
                               clusterToMeltLeft[highInd]);
            clusterToMeltLeft[highInd].clear();
          }
          
        }
      }
    }
    
    //melt left cluster lists
    for (unsigned int i = 0; i< rightCluster->getClusterArraySize(); i++){
      if(clusterToMeltLeft[i].size()>1)
      {
        sort(clusterToMeltLeft[i].begin(),clusterToMeltLeft[i].end());
        unsigned int lowInd = clusterToMeltLeft[i].at(0);
        
        for(unsigned int j = 1; j<clusterToMeltLeft[i].size();j++)
        {
          unsigned int highInd = clusterToMeltLeft[i].at(j);
          if(getListLength(highInd,CLUSTER)>0 &&
            clusterToMeltRight[highInd].size()>0)
          {
            meltListsOfCluster(lowInd, highInd);
            meltIndexOfCluster(clusterToMeltRight[lowInd],
                               clusterToMeltRight[highInd]);
            clusterToMeltRight[highInd].clear();
          }
        }
      }
    }
    
    //append cluster list
    for (unsigned int i = 0; i< clusterArray.size(); i++){
      if(clusterToMeltRight[i].size()>=1) 
        //TODO hier eventuell nicht merh komplette lister wegen merge
      {
        //append right list to left list
        borderIt = --this->getIterator(i,false,CLUSTER); 
        
        appendIt = 
        rightCluster->getIterator(clusterToMeltRight[i].at(0),true,CLUSTER);
        while(appendIt != 
          rightCluster->getIterator(clusterToMeltRight[i].at(0),false,CLUSTER))
        {
          clusterArray.at(i).push_back((*appendIt));
        appendIt++;
          }
          rightCluster->clearList(clusterToMeltRight[i].at(0));
          updateBorderNeighbor(borderIt,this->getList(i,CLUSTER),
                               leftOuterPoint,rightOuterPoint);
      }
    }
  }
  
/*
* updateBorderNeighbor

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  void
  Cluster<MEMB_TYP_CLASS,TYPE>::
  updateBorderNeighbor(typename list<MEMB_TYP_CLASS*>::iterator borderIt,
                       list<MEMB_TYP_CLASS*>& destList,
                       TYPE* leftOuterPoint, TYPE* rightOuterPoint)
  {
    if(borderIt != destList.end())
    {
      
      // update neighborhood in new cluster
      typename list<MEMB_TYP_CLASS*>::iterator helpItLeft = borderIt;
      typename list<MEMB_TYP_CLASS*>::iterator helpItRight;
      while(helpItLeft != --destList.begin() &&
        (*helpItLeft)->calcXDistanz(leftOuterPoint) <= eps)
      {
        helpItRight=borderIt;
        helpItRight++;
        while(helpItRight != destList.end() &&
          (*helpItRight)->calcXDistanz(rightOuterPoint) <= eps)
        {
          if((*helpItLeft)->calcDistanz(*helpItRight) <= eps){
            updateNeighbor(*helpItLeft,*helpItRight);
          }
          ++helpItRight;
        }
        --helpItLeft;
      }
    }
  }
  
/*
* memberAllreadyExist

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  bool
  Cluster<MEMB_TYP_CLASS,TYPE>::
  memberAllreadyExist(typename list<MEMB_TYP_CLASS*>::iterator helpIt, 
                      typename list<MEMB_TYP_CLASS*>::iterator neighborIt,
                      list<MEMB_TYP_CLASS*>& destList)
  {
    bool exist=false, end = false;
    while(!exist && !end && helpIt!= destList.begin()){
      if(helpIt!= destList.end() && *helpIt == *neighborIt)
        exist=true;
      if(helpIt != destList.end() && 
        (*helpIt)->getXVal() > (*neighborIt)->getXVal())
        end = true;
      helpIt--;
    }
    return exist;
  }
  
/*
* meltListsOfCluster

*/
  template <class MEMB_TYP_CLASS, class TYPE> //TODO TEST
  void Cluster<MEMB_TYP_CLASS,TYPE>::
  meltListsOfCluster(int destInd, int sourceInd)
  {
    
    typename list<MEMB_TYP_CLASS*>::iterator destListIt, sourceListIt;
    typename vector<list<MEMB_TYP_CLASS*> >::iterator clusterIt;
    destListIt = clusterArray.at(destInd).begin();
    sourceListIt  = clusterArray.at(sourceInd).begin();
    bool destIt = true;
    //insert elements along the x coord
    list<MEMB_TYP_CLASS*> destList;
    while(destListIt != clusterArray.at(destInd).end()
      && sourceListIt  != clusterArray.at(sourceInd).end())
    {
      if((*destListIt)->getXVal() >= (*sourceListIt)->getXVal())
      {
        destList.push_back((*destListIt));
        (*destListIt)->setClusterNo(destInd);
        destListIt++;
      }
      else
      {
        destList.push_back((*sourceListIt));
        (*sourceListIt)->setClusterNo(destInd);
        sourceListIt++;
      }
    }
    
    while(destListIt != clusterArray.at(destInd).end()){
      destList.push_back((*destListIt));
      (*destListIt)->setClusterNo(destInd);
      destListIt++;
    }
    while(sourceListIt != clusterArray.at(sourceInd).end()){
      destList.push_back((*sourceListIt));
      (*sourceListIt)->setClusterNo(destInd);
      sourceListIt++;
    }
    
    clusterIt=clusterArray.begin()+(destInd);
    //TODO TEST - clusterUpdate löscht leere listen
    clusterArray.at(sourceInd).clear();
    
    //     if(destInd<sourceInd){
    //       clusterArray.erase(clusterArray.begin()+(sourceInd));
    clusterArray.erase(clusterArray.begin()+(destInd));
    //     } else{
    //       clusterArray.erase(clusterArray.begin()+(sourceInd));
    //       clusterArray.erase(clusterArray.begin()+(destInd));
    //     }
    clusterArray.insert(clusterIt,destList);
  }
  
  template <class MEMB_TYP_CLASS, class TYPE> 
  void Cluster<MEMB_TYP_CLASS,TYPE>::
  meltIndexOfCluster( vector<unsigned int> &destIndList ,  
                      vector<unsigned int> &sourceIndList)
  {
    vector<unsigned int>::iterator destIt,srcIt;
    destIt = destIndList.begin();
    srcIt = sourceIndList.begin();
    
    if (destIt == destIndList.end()){
      if(srcIt != sourceIndList.end()){
        while(srcIt != sourceIndList.end())
        {
          destIndList.push_back(*srcIt);
          srcIt++;
        }
      }
    }else{
      
      while(srcIt != sourceIndList.end())
      {
        if(destIt != destIndList.end() && 
          srcIt != sourceIndList.end() &&
          *srcIt==*destIt)
        {
          srcIt = sourceIndList.erase(srcIt);
        }else{
          
          if(destIt != destIndList.end() && 
            srcIt != sourceIndList.end() &&
            *srcIt<*destIt)
          {
            destIt = destIndList.insert(destIt,*srcIt);
            srcIt++;
          }else{
            if(destIt != --destIndList.end()){
              destIt++;
            }else{
              destIt++;
              destIt = destIndList.insert(destIt,*srcIt);
              destIt--;
            }
          }
        }
      }
    }
  }
  
}
#endif 

