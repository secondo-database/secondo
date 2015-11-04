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
 
 [1] Implementation of the Spatial Algebra
 
 Jun 2015, Daniel Fuchs 
 
 [TOC]
 
 1 Overview
 
 
 This file contains the implementation of the class dbDacScanAlgebra
 
 2 Includes
 
*/

#include "Cluster.h"

using namespace std;
namespace distributedClustering{
  

/*
3.0 ~Constructor~

*/
template <class MEMB_TYP_CLASS, class TYPE>
Cluster<MEMB_TYP_CLASS,TYPE>::
Cluster( MEMB_TYP_CLASS* leftMember, double _eps, int _minPts) :
eps(_eps), minPts(_minPts),leftOuterPoint(0),rightOuterPoint(0)
{
  clusterCandList.push_back(leftMember);
  leftMember->setClusterNo(getClusterNo(0,CLUSTERCAND));
  leftMember->setClusterType(CLUSTERCAND_CL_NO);
  firstElem = leftMember;
  emptyList.clear();
  pair <double,double> dummy = make_pair(MAX_DOUBLE,MIN_DOUBLE);
  emptyMinMaxY.clear();
  emptyMinMaxY.push_back(dummy);
  emptyVectorList.clear();
  emptyVectorList.push_back(emptyList);
  
}

/*
3.1 Constructor for merge Step

this constructor is used for distMerge
in members are stored calculated Clusters from dbdacscan
  

*/
template <class MEMB_TYP_CLASS, class TYPE>
Cluster<MEMB_TYP_CLASS,TYPE>::
Cluster(vector <MEMB_TYP_CLASS*>& members, double _eps, int _minPts):
eps(_eps), minPts(_minPts),
firstElem(members.front()),
leftOuterPoint(members.front()->getPoint()),
rightOuterPoint(members.back()->getPoint())
{
  for(unsigned int i =0; i< members.size(); i++)
  {
    
    int clusterNo = members.at(i)->getClusterNo();
    int clusterType = members.at(i)->getClusterType();
    Kind kind = getClusterKindFromType(clusterType);
    int listNo = getListNoOfClusterNo(clusterNo,kind);
    
    while(clusterType >= 0 && //else member is Noise or ClusterCand
      listNo >= getVectorSize(kind))
    {
      //push back empty list
      list<MEMB_TYP_CLASS*> newList;
      pushListToCluster(kind, newList);
      
      //push back minMax
      pair <double,double> newMinMax = make_pair(MAX_DOUBLE,MIN_DOUBLE);
      pushMinMaxToCluster(kind,newMinMax);
    }
    pushMemberToClusterList(false,members.at(i),listNo,kind);
    if(clusterType >= 0 ){
      updateMinMaxVal(kind,listNo,members.at(i));
    }
  }
  
//   //TODO vieleicht müssen listen untereinander geprüft werden
//   // z.B Cluster mit allen anderen listen bzw alle listen gegeneinander
//   // eventuell über YBorder kontrollieren
//   
//   initNeighborHood(CLUSTERCAND);
//   initNeighborHood(LEFT);
//   initNeighborHood(RIGHT);
//   initNeighborHood(BOTH);
  
}


/*
  meltClusters
  melt this Cluster with the right cluster
  medianPoint and rightMedPoint are the two outer Points which
  represent the splitline
 
 */
template <class MEMB_TYP_CLASS, class TYPE>
void
Cluster<MEMB_TYP_CLASS,TYPE>::
meltClusters(Cluster * rightCluster,
             TYPE* leftInnerPoint,
             TYPE* rightInnerPoint )
{
  
  // set leftOuterPoint and rightOuterPoint
  if(leftOuterPoint == 0){
    leftOuterPoint = leftInnerPoint;
  }else{
    leftOuterPoint =
    firstElem->getOuterLeftValue(leftOuterPoint,leftInnerPoint);
  }
  
  if(rightCluster-> getRightOuterPoint() == 0){
    if(rightOuterPoint == 0){
      rightOuterPoint = rightInnerPoint;
    }else{
      rightOuterPoint =
      firstElem->getOuterRightValue(rightOuterPoint,rightInnerPoint);
    }
  }else{
    rightOuterPoint =
    firstElem->getOuterRightValue(rightCluster->getRightOuterPoint(),
                                  rightOuterPoint,rightInnerPoint);
  }
  
  
  
  //1.) verlgeiche ClusterCands und ertelle Listen und MinMay Borders #
  //    vector<list<MEMB_TYP_CLASS*> > clusterCandClusters;
  //    vector<pair <double,double> > clusterCandClMinMax;
  
  /*
   *     compareLeftWithRightList
   *     compares all clusterCands and add new possible clusterCands to
   *     clusterCandClusters
   */
  
  
  compareLeftWithRightList(leftInnerPoint, rightInnerPoint,
                           true,
                           getList(-1,CLUSTERCAND),
                           rightCluster->getList(-1,CLUSTERCAND),
                           clusterCandClusters,
                           clusterCandClMinMax,true,true,rightCluster);
  
  
  
  testClusterCandListsOnClusters(rightCluster,
                                 leftInnerPoint,rightInnerPoint,
                                 getList(-1,CLUSTERCAND),
                                 rightCluster->getList(-1,CLUSTERCAND),
                                 clusterCandClusters,
                                 clusterCandClMinMax);
  
  
  
  // 2.) vergleiche alle clusterlistern von Min zu Max und füge auch neue 
  //listen hinzu
  
  int leftMin = 0,rightMin = 0,clusterCandMin = 0, leftCnt = 0,rightCnt = 0;
  Kind leftKind, rightKind;
  bool getNewMinIndex = false,getNewMinClCand = false;
  bool clCandOutOfRangeLeftCl = true, clCandOutOfRangeRightCl = true;
  
  double actMinLeft = MIN_DOUBLE, actMinRight = MIN_DOUBLE,
  actMinClCand = MIN_DOUBLE;
  double actMaxLeft = MAX_DOUBLE, actMaxRight = MAX_DOUBLE,
  actMaxClCand = MAX_DOUBLE;
  
  
  // *** LEFT indices ****************************************************
  //save indexes - for each list on left side save indexes for list on
  unsigned int leftIndexSize =  getVectorSize(RIGHT) +
  getVectorSize(BOTH);
  unsigned int bothDistLeft = getVectorSize(RIGHT);
  //right side who should melted together.
  vector<pair<unsigned int,Kind> >
  clusterToMeltOnRightForLeftSide[leftIndexSize];
  //save order of indexes from min to max
  vector<unsigned int > minToMaxIndexesLeft;
  //save new indices
  pair<unsigned int,Kind>
  newLeftIndices[leftIndexSize];
  //init newIndicies
  initIndicies(newLeftIndices,leftIndexSize,bothDistLeft,false);
  
  // ***RIGHT indices ****************************************************
  unsigned int rightIndexSize = rightCluster->getVectorSize(LEFT) +
  rightCluster->getVectorSize(BOTH);
  unsigned int bothDistRight = rightCluster->getVectorSize(LEFT);
  //left side who should melted toghether
  vector<pair<unsigned int,Kind> >
  clusterToMeltOnLeftForRightSide[rightIndexSize];
  //save order of indexes from min to max
  vector<unsigned int > minToMaxIndexesRight;
  //save new indices
  pair<unsigned int,Kind>
  newRightIndices[rightIndexSize];
  initIndicies(newRightIndices,rightIndexSize,bothDistRight,true);
  
  // ***CLUSTERCAND indices ****************************************************
  // save indexes for clusterCand
  vector< clusterCandMelt>
  clusterCandClustersToMeltWithClNo[clusterCandClusters.size()];
  //lists for single cluster cands -> index is ClusterNo 
  //  -> save Member Pointer and ClusterKind
  unsigned int leftClCSize = getListLength(0,CLUSTERCAND);
  vector< clusterCandMelt>
  leftClusterCandMemberToMeltWithRClNo[leftClCSize];
  unsigned int rightClCSize = rightCluster->getListLength(0,CLUSTERCAND);
  vector< clusterCandMelt>
  rightClusterCandMemberToMeltWithLClNo[rightClCSize];
  
  //***************************************************************************
  
  //find left and right y min of existing
  // clusterLists RIGHT and BOTH on left side
  //and LEFT and BOTH on right side
  if(leftIndexSize || rightIndexSize)
  {
    
    getNewMinIndex =
    findNextMinList(leftMin,leftKind,
                    actMinLeft,actMaxLeft,true,leftCnt,
                    rightMin,rightKind,
                    actMinRight,actMaxRight,true,rightCnt,
                    rightCluster);
    
  } else {
    getNewMinIndex = false;
  }
  // find clusterCand y min if there are clusterCandList in clusterCandClusters
  
  
  //check if clustercand or clusterlist reachable to other lists
  while(getNewMinIndex){
    //first Test all ClusterCands
    //test left clustercands with actual right list and save members
    if(rightMin > -1){
      compareClusterCandsWithOppositeList(
        leftInnerPoint,
        rightInnerPoint,clusterCandList,
        true,
        rightCluster->getList(rightMin,rightKind),
                                        rightKind,rightMin,bothDistRight,
                                        leftClusterCandMemberToMeltWithRClNo);
    }
    //test right clustercands with actual left list and save members
    
    if(leftMin > -1){
      compareClusterCandsWithOppositeList(
        leftInnerPoint,
        rightInnerPoint,
        rightCluster->getList(-1,CLUSTERCAND),
                                        false,
                                        getList(leftMin,leftKind),
                                        leftKind,leftMin,bothDistLeft,
                                        rightClusterCandMemberToMeltWithLClNo);
    }
    
    //init clusterCands
    actMinClCand = MIN_DOUBLE;
    actMaxClCand = MAX_DOUBLE;
    if(clusterCandClusters.size())
    {
      getNewMinClCand =
      findNextMinListOfClCand(clusterCandClMinMax,
                              clusterCandMin,
                              actMinClCand,actMaxClCand,clCandOutOfRangeLeftCl,
                              clCandOutOfRangeRightCl,actMaxLeft,actMaxRight);
    }else{
      getNewMinClCand=false;
    }
    
    //check clusterCandList
    while(getNewMinClCand &&
      (!clCandOutOfRangeLeftCl || !clCandOutOfRangeRightCl))
    {
      //test left list
      if(!clCandOutOfRangeLeftCl && leftMin > -1 &&
        compareLeftWithRightList(leftInnerPoint,rightInnerPoint,
                                 false,getList(leftMin, leftKind),
                                 clusterCandClusters.at(clusterCandMin),
                                 emptyVectorList,emptyMinMaxY,false,true)
      )
      {
        clusterCandMelt newItem = {(unsigned int)leftMin,leftKind, false,0};
        insertIndexToClusterCandToMelt(clusterCandMin,
                                       newItem,
                                       clusterCandClustersToMeltWithClNo);
      }
      //test right list
      if(!clCandOutOfRangeRightCl && rightMin > -1 &&
        compareLeftWithRightList(leftInnerPoint,rightInnerPoint,
                                 false,
                                 clusterCandClusters.at(clusterCandMin),
                                 rightCluster->getList(rightMin,rightKind),
                                 emptyVectorList,emptyMinMaxY,true,false)
      )
      {
        clusterCandMelt newItem = {(unsigned int)rightMin,rightKind, true,0};
        insertIndexToClusterCandToMelt(clusterCandMin,
                                       newItem,
                                       clusterCandClustersToMeltWithClNo);
      }
      
      //search next min of clusterCand
      getNewMinClCand = findNextMinListOfClCand(
        clusterCandClMinMax,clusterCandMin,
        actMinClCand,actMaxClCand,clCandOutOfRangeLeftCl,
        clCandOutOfRangeRightCl,actMaxLeft,actMaxRight);
      
    }
    
    
    //check cluster lists
    if(leftMin > -1 && rightMin > -1){
      if(listIsInYBordersOfList(
        this,leftMin,leftKind,
        rightCluster,rightMin,rightKind))
      {
        
        //Round n:
        if(compareLeftWithRightList(
          leftInnerPoint,rightInnerPoint,
          false,
          getList(leftMin,leftKind),
                                    rightCluster->getList(rightMin,rightKind),
                                    emptyVectorList,emptyMinMaxY,false,false))
        {
          
          
          
          //remember this two lists
          if(leftKind == RIGHT)
          {
            insertIndexToClusterToMelt(leftMin,rightMin,
                                       rightKind,
                                       clusterToMeltOnRightForLeftSide,
                                       minToMaxIndexesLeft);
          }
          if(leftKind == BOTH){
            insertIndexToClusterToMelt(leftMin + bothDistLeft,rightMin,
                                       rightKind,
                                       clusterToMeltOnRightForLeftSide,
                                       minToMaxIndexesLeft);
          }
          if( rightKind == LEFT)
          {
            insertIndexToClusterToMelt(rightMin,leftMin,
                                       leftKind,
                                       clusterToMeltOnLeftForRightSide,
                                       minToMaxIndexesRight);
          }
          if(rightKind == BOTH)
          {
            insertIndexToClusterToMelt(bothDistRight + rightMin,leftMin,
                                       leftKind,
                                       clusterToMeltOnLeftForRightSide,
                                       minToMaxIndexesRight);
          }
          
        }
      }
    }
    
    //find searching side:
    bool calcLeftMin = false, calcRightMin = false;
    if(leftMin > -1 )
    {
      calcLeftMin = (actMinLeft < actMinRight);
    }
    if(rightMin > -1 )
    {
      calcRightMin = !(actMinLeft < actMinRight);
    }
    
    if(!calcLeftMin && !calcRightMin){
      if(leftIndexSize > leftCnt && leftMin > -1){
        calcLeftMin = true;
      }
      if(rightIndexSize > rightCnt && rightMin > -1){
        calcRightMin = true;
      }
    }
    
    //Round n+1:
    //search next y min -> left or right
    getNewMinIndex =
    findNextMinList(leftMin,leftKind,
                    actMinLeft,actMaxLeft,
                    calcLeftMin,leftCnt,
                    rightMin,rightKind,
                    actMinRight,actMaxRight,
                    calcRightMin,rightCnt,
                    rightCluster);
  }
  
  
  
  // 2.1) Verschmelze cluster -> 
  // prüfe auch clusterCands -> setze minima und maxima neu
  
  // meltClusterlists on right side and update MinMax
  if(minToMaxIndexesLeft.size())
  {
    meltClusterLists(rightCluster,
                     clusterToMeltOnRightForLeftSide,
                     minToMaxIndexesLeft,
                     clusterToMeltOnLeftForRightSide,
                     newRightIndices
                     ,bothDistRight);
    
  }
  // melt Clsuterlists on left side and update MinMax
  if(minToMaxIndexesRight.size())
  {
    meltClusterLists(this,
                     clusterToMeltOnLeftForRightSide,
                     minToMaxIndexesRight,
                     clusterToMeltOnRightForLeftSide,
                     newLeftIndices,bothDistLeft);
    
  }
  //melt clustercandlist with cluster lists and update MinMax
  for(unsigned int i = 0; i< clusterCandClusters.size(); i++){
    
    if(clusterCandClustersToMeltWithClNo[i].size() > 0)
    {
      //melt list with first entry -> search entry
      findClListToMeltWithClustCandList(rightCluster,
                                        clusterCandClustersToMeltWithClNo[i],
                                        clusterCandClusters.at(i),
                                        clusterCandClMinMax.at(i),
                                        bothDistLeft,newLeftIndices,
                                        bothDistRight,newRightIndices);
      
      if(clusterCandClustersToMeltWithClNo[i].size() > 1)
      {
        
        // update MeltingIndexes - and melt cluster lists if it is necessary
        updateMeltedCluster(rightCluster,
                            clusterCandClustersToMeltWithClNo[i],
                            clusterToMeltOnRightForLeftSide,
                            bothDistLeft,
                            newLeftIndices,
                            clusterToMeltOnLeftForRightSide,
                            bothDistRight,
                            newRightIndices);
        
      }
    }else{ //size == 0
      
      
      pair<unsigned int,Kind> oldIndex = make_pair(i,CLUSTERCAND);
      // the list is a new Cluster
      addListToCorrectClusterType(
        clusterCandClusters.at(i),
                                  clusterCandClMinMax.at(i),
                                  oldIndex);
    }
    clusterCandClusters.at(i).clear();
  }
  //clear clusterCandClusters
  clusterCandClusters.clear();
  
  
  
  //melt clusterCands with list ##############################################
  //add leftClusterCandMember and update MinMaxY
  
  if(rightIndexSize){
    meltClsuterCandWithClusterList(rightCluster,
                                   leftClusterCandMemberToMeltWithRClNo,
                                   leftClCSize,
                                   clusterToMeltOnRightForLeftSide,
                                   newLeftIndices,
                                   bothDistLeft,
                                   clusterToMeltOnLeftForRightSide,
                                   newRightIndices,
                                   bothDistRight);
  }
  //add rightClsuterCandMember and update MinMaxY
  if(leftIndexSize){
    meltClsuterCandWithClusterList(rightCluster,
                                   rightClusterCandMemberToMeltWithLClNo,
                                   rightClCSize,
                                   clusterToMeltOnRightForLeftSide,
                                   newLeftIndices,
                                   bothDistLeft,
                                   clusterToMeltOnLeftForRightSide,
                                   newRightIndices,
                                   bothDistRight);
  }
  
  //3.) alle nicht verschmolzenen listen werden angehängt ##################
  
  
  
  //copy right noise to left noise
  pair<unsigned int,Kind> dummyIndex = make_pair(0,NOISE);
  pair<double,double> dummyMinMax = make_pair(MAX_DOUBLE,MIN_DOUBLE);
  if(noiseList.size() || rightCluster->getListLength(0,NOISE))
  {
    sortRightListToLeftList(this,
                            rightCluster->getList(-1,NOISE),
                            noiseList,
                            dummyMinMax,dummyMinMax,
                            dummyIndex,dummyIndex,false,false);
    
  }
  
  
  //copy right clusterCand to left clusterCand
  dummyIndex.second = CLUSTERCAND;
  if(clusterCandList.size() || rightCluster->getListLength(0,CLUSTERCAND)){
    sortRightListToLeftList(this,
                            rightCluster->getList(-1,CLUSTERCAND),
                            clusterCandList,
                            dummyMinMax,dummyMinMax,
                            dummyIndex,dummyIndex,false,false);
    
  }
  
  
  
  //copy right clusterLists to left side
  
  //first check clusterToMeltOnRightForLeftSide
  // then all RIGHT and BOTH on left cluster are checked and updated
  for(int i=0; i<leftIndexSize; i++)
  {
    //define left index
    pair<unsigned int,Kind> leftIndex;
    if(i >= bothDistLeft){
      leftIndex =
      make_pair( i- bothDistLeft,BOTH);
    }else {
      leftIndex =
      make_pair(i,RIGHT);
    }
    
    if(clusterToMeltOnRightForLeftSide[i].size() > 0)
    {
      //then there is a cluster
      //append first entry to list i
      
//TODO DELETE
//printClusterToMeltIndex(clusterToMeltOnRightForLeftSide,leftIndexSize);
//printClusterToMeltIndex(clusterToMeltOnLeftForRightSide,rightIndexSize);
// pair<unsigned int,Kind> rightIndex=clusterToMeltOnRightForLeftSide[i].at(0);
      if(getListLength(leftIndex)) //TODO workaround
      {//TODO DELETE
        
        //define new ClusterType
        pair<unsigned int,Kind> destIndex;
        defineDestIndexPair(getList(leftIndex),
                            rightCluster->getList( 
                                  clusterToMeltOnRightForLeftSide[i].at(0)),
                            leftIndex,
                            destIndex);
        
        
        
        
        sortRightListToLeftList(this,
                                rightCluster->
                                getList(
                                  clusterToMeltOnRightForLeftSide[i].at(0)),
                                getList(leftIndex),
                                rightCluster->
                                getMinMaxFromCluster(
                                  clusterToMeltOnRightForLeftSide[i].at(0)),
                                getMinMaxFromCluster(leftIndex),
                                leftIndex, destIndex, true,true);
      }
      
    }else{
      // index has no melting member
      //change Kind in left cluster
      //BOTH --> LEFT
      //RIGHT --> CLUSTER
      if(getListLength(leftIndex)){
        pair<unsigned int,Kind> destIndex;
        list<MEMB_TYP_CLASS*> emptylist;
        pair<double,double> initMinMax =
        make_pair(MAX_DOUBLE,MIN_DOUBLE);
        defineDestIndexPair(getList(leftIndex),
                            emptylist,
                            leftIndex,
                            destIndex);
        //rename each Point in destList
        sortRightListToLeftList(this,emptylist,
                                getList(leftIndex),
                                initMinMax,
                                getMinMaxFromCluster(leftIndex),
                                leftIndex, destIndex,
                                true,false);
      }
      
      
    }
  }//all RIGHT and BOTH on left cluster are checked and updated
  
  
  //nothing to do with LEFT and CLUSTER from leftSideCluster
  
  // move all right lists to left Cluster
  copyRightClusterListToLeftCluster(rightCluster,CLUSTER);
  copyRightClusterListToLeftCluster(rightCluster,RIGHT);
  copyRightClusterListToLeftCluster(rightCluster,LEFT);
  copyRightClusterListToLeftCluster(rightCluster,BOTH);
  
  
  //4.) delete all empty lists and pairs
  delete rightCluster;
  clusterCandClMinMax.clear();
  deleteEmptyLists(CLUSTER);
  deleteEmptyLists(RIGHT);
  deleteEmptyLists(LEFT);
  deleteEmptyLists(BOTH);
  
  //5.) bei update clsuter -> diese neu organisieren -> eventuell sind
  //Typen nicht richtig eingeordnet (BOTH LEFT RIGHT)
  
  //sort clsuterCands correct in List
  testReachabilityAndSetClusterNoAtEachPoint(leftOuterPoint,rightOuterPoint,
                                             clusterCandList,
                                             CLUSTERCAND_CL_NO,
                                             CLUSTERCAND_CL_NO,true);
  
}


/*
    compareLeftWithRightList
    return true if lists are density reachable
 */
template <class MEMB_TYP_CLASS, class TYPE>
bool
Cluster<MEMB_TYP_CLASS,TYPE>::
compareLeftWithRightList( TYPE* leftInnerPoint, TYPE* rightInnerPoint,
                          bool isNewClusterCand,/*  Kind leftKind,
                          Kind rightKind,*/
                          list<MEMB_TYP_CLASS*>& leftList ,
                          list<MEMB_TYP_CLASS*>& rightList,
                          vector<list<MEMB_TYP_CLASS*> >& retClusterCand,
                          vector<pair<double,double> >& retClusterCandMinMax,
                          bool leftListIsClusterCandList,
                          bool rightListIsClusterCandList,
                          Cluster* rightCluster)
{
  bool distGreaterEpsLeft = false, distGreaterEpsRight = false, retVal = false;
  typename list<MEMB_TYP_CLASS*>::iterator
  itLeft,itRight;
  
  itLeft = --leftList.end();
  while(!distGreaterEpsLeft && itLeft!= --leftList.begin() ){
    itRight = rightList.begin();
    distGreaterEpsRight = false;
    while(!distGreaterEpsRight &&
      itRight!= rightList.end()
      && itLeft!= --leftList.begin() )
    {
      if((*itLeft)->calcDistanz(*itRight) <= eps){
        retVal=true;
        //two members found
        if(isNewClusterCand)// then create a new empty list
        {
          //create a new cluster list
          list<MEMB_TYP_CLASS*> clusterList;
          //create a new minMax pair
          pair<double,double> clusterMinMax(MAX_DOUBLE,MIN_DOUBLE);
          
          MEMB_TYP_CLASS* leftMemb = *itLeft;
          MEMB_TYP_CLASS* rightMemb = *itRight;
          
          
          updateNeighborRightListToLeftList(leftList,rightList,
                                            false,false,
                                            leftInnerPoint,
                                            rightInnerPoint);
          
          concatClusterCand(rightCluster,
                            leftInnerPoint,
                            rightInnerPoint,
                            clusterList,
                            clusterMinMax,
                            *itLeft,
                            *itRight,
                            leftList,
                            rightList,
                            getClusterNo(
                              clusterCandClusters.size(),
                                         CLCANDCLUSTERS));
          
          
          bool deleteWorkedLeft = false, deleteWorkedRight = false;
          itLeft = searchAndDeletItemFromList(leftMemb,
                                              itLeft,
                                              leftList,
                                              deleteWorkedLeft);
          itRight = searchAndDeletItemFromList(rightMemb,
                                               itRight,rightList,
                                               deleteWorkedRight);
          retClusterCand.push_back(clusterList);
          retClusterCandMinMax.push_back(clusterMinMax);
          
          if(!deleteWorkedLeft || !deleteWorkedRight){
            cout << "FAIL delete didn't work "<< endl;
            cout << " in compareLeftWithRightList" << endl;
          }
          //start from beginning
          itLeft = --leftList.end();
          itRight = rightList.begin();
          
        }else{ //!isClusterCand
          if(leftListIsClusterCandList || rightListIsClusterCandList){
            updateNeighborRightListToLeftList(
              leftList,rightList,false,false,
              leftInnerPoint,rightInnerPoint);
          }else{
            updateNeighborRightListToLeftList(
              leftList,rightList,true,true,
              leftInnerPoint,rightInnerPoint);
          }
          return retVal;
        }
      }else{
        if(itRight!= rightList.end()){
          itRight++;
        }
        
        if (!isNewClusterCand &&
          !rightListIsClusterCandList &&
          itRight!= rightList.end())
        {
          distGreaterEpsRight =
          getGreaterEps(rightList,leftInnerPoint,itRight);
        }
      }
    }
    if(itLeft!= --leftList.begin()){
      itLeft--;
    }
    if(!isNewClusterCand &&
      !leftListIsClusterCandList &&
      itLeft!= --leftList.begin())
    {
      distGreaterEpsLeft = getGreaterEps(leftList,rightInnerPoint,itLeft);
    }
  }
  return retVal;
}

/*
    concatClusterCand
    
 */
template <class MEMB_TYP_CLASS, class TYPE>
void
Cluster<MEMB_TYP_CLASS,TYPE>::
concatClusterCand(Cluster* rightCluster, TYPE* leftInnerPoint,
                  TYPE* rightInnerPoint,
                  list<MEMB_TYP_CLASS*>& destList,
                  pair<double,double>& clusterPair,
                  MEMB_TYP_CLASS* leftMemb,
                  MEMB_TYP_CLASS* rightMemb,
                  list<MEMB_TYP_CLASS*>& leftList,
                  list<MEMB_TYP_CLASS*>& rightList,
                  int clusterNo)
{
  //beginn with left noise points
  destList.push_back(leftMemb);
  leftMemb->setClusterNo(clusterNo);
  leftMemb->setClusterType(CLCANDCL_CL_NO);
  updateMinMaxVal(clusterPair,leftMemb);
  //add right Member
  const typename list<MEMB_TYP_CLASS*>::iterator borderIt =--destList.end();
  rightMemb->setClusterNo(clusterNo);
  rightMemb->setClusterType(CLCANDCL_CL_NO);
  destList.push_back(rightMemb);
  updateMinMaxVal(clusterPair,rightMemb);
  
  moveNeighborsToDestList(rightCluster,
                          clusterPair,leftMemb, leftList,rightList,
                          destList,true,
                          clusterNo,CLCANDCL_CL_NO);
  
  moveNeighborsToDestList(rightCluster,
                          clusterPair,rightMemb, rightList,leftList,
                          destList,true,
                          clusterNo,CLCANDCL_CL_NO);
  
  //from borderIt look if other clusterCands found
  typename list<MEMB_TYP_CLASS*>::iterator helpIt;
  //from boderIt to left -> search right list for reachable neighbor
  while(borderIt !=
    (helpIt = compareSrcListFromItWithRightList(
      leftInnerPoint,rightInnerPoint,
      destList,borderIt,rightList,
      clusterNo,true,
      false,true)))
  {
    
    bool moveWorked = false;
    moveItemSorted(this,helpIt,rightList,destList,true,
                   clusterNo,CLCANDCL_CL_NO,false,
                   moveWorked);
    
    if(!moveWorked){ //TODO TEST
      cout << "FAIL move didn't work "<< endl;
      cout << " in concatClusterCand at "
      "compareSrcListFromItWithRightList" << endl;
    }
    
    updateMinMaxVal(clusterPair,*helpIt);
    moveNeighborsToDestList(rightCluster,
                            clusterPair, *helpIt, rightList,leftList,
                            destList, true,
                            clusterNo,CLCANDCL_CL_NO);
  }
  
  //from borderIt to right -> search left list for reachable neighbor
  while(borderIt !=
    (helpIt = compareSrcListFromItWithLEFTList(
      leftInnerPoint,rightInnerPoint,
      destList,borderIt,leftList,
      clusterNo,true,
      false,true)))
  {
    bool moveWorked = false;
    moveItemSorted(this, helpIt,leftList,destList,true,
                   clusterNo,CLCANDCL_CL_NO,false,moveWorked);
    
    if(!moveWorked){ //TODO TEST
      cout << "FAIL move didn't work "<< endl;
      cout << " in concatClusterCand at "
      "compareSrcListFromItWithLEFTList" << endl;
    }
    
    updateMinMaxVal(clusterPair,*helpIt);
    moveNeighborsToDestList(rightCluster,
                            clusterPair,  *helpIt, leftList,rightList,
                            destList, true,
                            clusterNo,CLCANDCL_CL_NO);
  }
  
}

/*
   addListToCorrectClusterType
    find out cluster type for given clusterlist and add
   it to existing clusterlists
   
 */
template <class MEMB_TYP_CLASS, class TYPE>
void
Cluster<MEMB_TYP_CLASS,TYPE>::
addListToCorrectClusterType(
  list<MEMB_TYP_CLASS*>& clusterList,
  pair<double,double>& clusterPair,
  pair<unsigned int,Kind>& listIndex
)
{
  // some Information regarding to left and right side of Lists
  // due to the descending sorting of the List elements corresponds the left
  // hand element to the right Cluster element
  // the same is with richt list element and left Cluster side
  if(clusterList.size()){
    pair<unsigned int,Kind> newIndex;
    list<MEMB_TYP_CLASS*> emptyList;
    
    defineDestIndexPair(clusterList,emptyList,listIndex ,newIndex);
    
    //test the list if each point is density reachable and
    //set ClusterNo and Type for each point
    
    bool allDensReachable =
    testReachabilityAndSetClusterNoAtEachPoint(
      leftOuterPoint,rightOuterPoint,
      clusterList,newIndex,false);
    
    if(clusterList.size()){
      //add list to correct cluster
      //        switch(clusterType)
      switch(newIndex.second)
      {
        //          case 1: //finished cluster
        case CLUSTER:
          if(clusterArray.size() == clusterMinMaxY.size())
          {
            clusterArray.push_back(clusterList);
            clusterMinMaxY.push_back(clusterPair);
          } else {
            cout << "clusterArray and clusterMinMaxY"
            " haven't the same size!" << endl;
            undefinedCluster.push_back(clusterList);
          }
          break;
        case LEFT:
          if(leftPartCluster.size() == leftPCMinMaxY.size())
          {
            leftPartCluster.push_back(clusterList);
            leftPCMinMaxY.push_back(clusterPair);
          } else {
            cout << "leftPartCluster and leftPCMinMaxY"
            " haven the same size!" << endl;
            undefinedCluster.push_back(clusterList);
          }
          break;
        case RIGHT:
          if(rightPartCluster.size() == rightPCMinMaxY.size())
          {
            rightPartCluster.push_back(clusterList);
            rightPCMinMaxY.push_back(clusterPair);
          } else {
            cout << "rightPartCluster and rightPCMinMaxY"
            " haven the same size!" << endl;
            undefinedCluster.push_back(clusterList);
          }
          break;
        case BOTH:
          if(bothSideCluster.size() == bothSCMinMaxY.size())
          {
            bothSideCluster.push_back(clusterList);
            bothSCMinMaxY.push_back(clusterPair);
          } else {
            cout << "bothSideCluster and bothSCMinMaxY"
            " haven the same size!" << endl;
            undefinedCluster.push_back(clusterList);
          }
          break;
          
      }
    }//else clusterList is empty -> all moved to Noise or ClusterCands
  }//else do nothing - list is empty
  
}


/*
 updateNeighbor
 update the eps neighborhood from left and right member
 */
template <class MEMB_TYP_CLASS, class TYPE>
void
Cluster<MEMB_TYP_CLASS,TYPE>::
updateNeighbor(MEMB_TYP_CLASS * leftMemb, MEMB_TYP_CLASS *rightMemb,
               list<MEMB_TYP_CLASS*>& neighborList)
{
  
  
  //     add left and right as neighbor
  if(leftMemb->calcDistanz(rightMemb)<=eps){
    //control if neighbor already existing
    if(!leftMemb->existNeighbor(rightMemb)){
      leftMemb->addNeighbor(rightMemb);
    }
    if(!rightMemb->existNeighbor(leftMemb)){
      rightMemb->addNeighbor(leftMemb);
    }
    neighborList.push_back(leftMemb);
    neighborList.push_back(rightMemb);
    
    typename list<MEMB_TYP_CLASS*>::iterator leftIt,rightIt;
    
    rightIt=rightMemb->getEpsNeighborhood(true);
    while(rightIt != rightMemb->getEpsNeighborhood(false)){
      
      if(leftMemb->calcDistanz(*rightIt)<=eps){
        typename list<MEMB_TYP_CLASS*>::iterator it = rightIt;
        if(!isMembInList((*it),neighborList)){
          updateNeighbor(leftMemb, *it,neighborList);
        }
      }
      ++rightIt;
    }
    leftIt=leftMemb->getEpsNeighborhood(true);
    while(leftIt!=leftMemb->getEpsNeighborhood(false)){
      if(rightMemb->calcDistanz(*leftIt)<=eps){
        typename list<MEMB_TYP_CLASS*>::iterator it = leftIt;
        if(!isMembInList((*it),neighborList)){
          updateNeighbor(rightMemb, *it,neighborList);
        }
      }
      ++leftIt;
    }
  }
  
}

/*
 isMembInList
 when point is in list -> return true
 */
template <class MEMB_TYP_CLASS, class TYPE>
bool
Cluster<MEMB_TYP_CLASS,TYPE>::
isMembInList(MEMB_TYP_CLASS *memb, list<MEMB_TYP_CLASS*>& list)
{
  bool contain = false;
  typename std::list<MEMB_TYP_CLASS*>::iterator it = list.begin();
  
  while(it != list.end()){
    if(memb==(*it))
      contain = true;
    it++;
  }
  return contain;
}


/*
 meltListsOfCluster

 */
template <class MEMB_TYP_CLASS, class TYPE>
void Cluster<MEMB_TYP_CLASS,TYPE>::
meltClusterCandListWithClusterList(pair<unsigned int,Kind>& destinationList,
                                   list<MEMB_TYP_CLASS*>& sourceIndList,
                                   pair <double,double>& minMaxList)
{
  unsigned int destInd = destinationList.first;
  Kind destKind = destinationList.second;
  Kind retKind = destKind;
  
  //insert elements along the x coord
  list<MEMB_TYP_CLASS*> retList;
  sortElemtsFromListsInNewList(sourceIndList,
                               getList(destInd,destKind),
                               retList,
                               destinationList);
  
  //get MinMax borders
  pair <double,double> retMM =
  getNewMinMaxForClusterList(minMaxList,
                             getMinMaxFromCluster(destKind,destInd) );
  //insert
  typename vector<list<MEMB_TYP_CLASS*> >::iterator
  clusterIt =getClusterVector(destKind).begin()+(destInd);
  typename vector<pair<double,double> >::iterator
  minMaxIt= getMinMaxVector(retKind).begin()+(destInd);
  eraseList(destKind,destInd);
  insertList(clusterIt,retList,destKind);
  eraseMinMax(destKind,destInd);
  insertMinMax(minMaxIt,retMM,retKind);
  //  }
}



/*
  meltListsOfCluster

 */
template <class MEMB_TYP_CLASS, class TYPE>
pair<unsigned int,Kind>
Cluster<MEMB_TYP_CLASS,TYPE>::
meltListsOfCluster(pair<unsigned int,Kind>& destinationList,
                   pair<unsigned int,Kind>& sourceList)
{
  unsigned int destInd = destinationList.first;
  Kind destKind = destinationList.second;
  
  unsigned int sourceInd = sourceList.first;
  Kind srcKind = sourceList.second;
  
  bool newRetInd = false;
  unsigned int retClNo = destInd;
  
  Kind retKind = destKind;
  if(destinationList.second == BOTH
    ||sourceList.second == BOTH )
  {
    retKind =BOTH;
    if(destinationList.second != BOTH){
      newRetInd = true;
      retClNo = getClusterNo(
        getVectorSize(retKind),
                             retKind);
    }
  }
  
  //insert elements along the x coord
  list<MEMB_TYP_CLASS*> retList;
  sortElemtsFromListsInNewList(getList(sourceInd,srcKind),
                               getList(destInd,destKind),
                               retList,
                               destinationList);
  //get MinMax borders
  pair <double,double> retMM =
  getNewMinMaxForClusterList(getMinMaxFromCluster(srcKind,sourceInd),
                             getMinMaxFromCluster(destKind,destInd));
  
  pair <double,double> initMM(MAX_DOUBLE,MIN_DOUBLE);
  
  if(newRetInd){
    //push back
    clearList(destKind,destInd);
    updateMinMaxVal(destKind,destInd,initMM);
    pushListToCluster(retKind,retList);
    pushMinMaxToCluster(retKind,retMM);
  }else{
    //insert
    typename vector<list<MEMB_TYP_CLASS*> >::iterator
    clusterIt =getClusterVector(destKind).begin()+(destInd);
    typename vector<pair<double,double> >::iterator
    minMaxIt= getMinMaxVector(retKind).begin()+(destInd);
    eraseList(destKind,destInd);
    insertList(clusterIt,retList,destKind);
    eraseMinMax(destKind,destInd);
    insertMinMax(minMaxIt,retMM,retKind);
  }
  return pair<unsigned int,Kind>(destInd,retKind);
}


/*
    meltIndexOfCluster
    melt the given clusterindexes
 */
template <class MEMB_TYP_CLASS, class TYPE>
void Cluster<MEMB_TYP_CLASS,TYPE>::
meltIndexOfCluster( vector<pair<unsigned int,Kind> > &destIndList ,
                    vector<pair<unsigned int,Kind> > &sourceIndList)
{
  typename  vector<pair<unsigned int,Kind> >::iterator destIt,srcIt;
  destIt = destIndList.end();
  destIt--;
  srcIt = sourceIndList.begin();
  
  unsigned int destLSize = destIndList.size();
  unsigned int srcLSize = sourceIndList.size();
  
  if (destIndList.empty() && !sourceIndList.empty()){
    while(srcIt != sourceIndList.end())
    {
      destIndList.push_back(*srcIt);
      srcIt++;
    }
  }else{
    if (destLSize > 0 && srcLSize > 0){
      while(srcIt != sourceIndList.end())
      {
        if(destIt != destIndList.end() &&
          //              (*srcIt).first !=(*destIt).first)
          (*srcIt) !=(*destIt))
        {
          destIndList.push_back(*srcIt);
        }
        srcIt++;
      }
    }
    
    vector<pair<unsigned int,Kind> > emptyV;
    sourceIndList.swap(emptyV);
    //      sourceIndList.clear();
  }
}

/*
findNextMinList

*/
template <class MEMB_TYP_CLASS, class TYPE>
bool Cluster<MEMB_TYP_CLASS,TYPE>::
findNextMinList(int& leftIndex, Kind& leftKind,
                       double& leftMinima,double& leftMaxima,
                       bool calcLeftMin, int& leftCnt,
                       int& rightIndex, Kind& rightKind,
                       double& rightMinima,double& rightMaxima,
                       bool calcRightMin, int& rightCnt,
                       Cluster* rightCluster)
  {

    bool retval = true;
    if(calcLeftMin || calcRightMin){

        if(calcLeftMin && leftIndex > -1){
            findNextMinList(leftIndex,leftKind,leftMinima,false);
            leftCnt ++;
        }
        if(calcRightMin && rightIndex > -1){
            rightCluster->findNextMinList(rightIndex,rightKind,
                                          rightMinima,true);
            rightCnt++;
        }

        if(leftIndex < 0 && rightIndex < 0)//no more min Value
          {
            retval = false;
          } else {
              if(leftIndex > -1){
                  leftMaxima =getYMaxfromCluster(leftKind,leftIndex);
                  leftMinima = getYMinfromCluster(leftKind,leftIndex);
              }
              if(rightIndex > -1){
                  rightMaxima =rightCluster->getYMaxfromCluster(
                      rightKind,rightIndex);
                  rightMinima = rightCluster->getYMinfromCluster(
                      rightKind,rightIndex);
              }
          }
    } else {
        retval = false;
    }

    return retval;
  }

/*
findNextMinList

find the next List in Y direction from a given minimum point
if no min found return -1
  
*/
template <class MEMB_TYP_CLASS, class TYPE>
void Cluster<MEMB_TYP_CLASS,TYPE>::
findNextMinList(int& retIndex, Kind& retKind,
                       double actualMinima, bool rightCluster)
  {
    //    double min = actualMinima;
    int  iSIDE = -1, iBOTH = -1;
    Kind sideKind;
    if(rightCluster){
        //serach in LEFT and BOTH
        iSIDE  = getIndexOfFindNextMinList(actualMinima,LEFT);
        sideKind = LEFT;
    } else {
        //search in RIGHT and BOTH
        iSIDE  = getIndexOfFindNextMinList(actualMinima,RIGHT);
        sideKind = RIGHT;
    }
    iBOTH = getIndexOfFindNextMinList(actualMinima,BOTH);

    if(iSIDE == -1 ||
        (iBOTH >=0 && getYMinfromCluster(BOTH,iBOTH) <
            getYMinfromCluster(sideKind,iSIDE) ))
      {
        retIndex = iBOTH;
        retKind = BOTH;
      }else{
          retIndex = iSIDE;
          retKind = sideKind;
      }
  }

  
/*
findNextMinListOfClCand

returns the next minimum clustercand
*/

template <class MEMB_TYP_CLASS, class TYPE>
bool Cluster<MEMB_TYP_CLASS,TYPE>::
findNextMinListOfClCand(vector<pair <double,double> >&
                               minMaxlist,int& index,
                               double& actualMinima,
                               double& actualMaxima,
                               bool& clCandOutOfRangeLeftCl,
                               bool& clCandOutOfRangeRightCl,
                               double actMaxLeftList,
                               double actMaxRightLsit)
  {
    int retVal = -1;
    double min = MAX_DOUBLE;
    for (int i =0; i < minMaxlist.size(); i++)
      {
        if(minMaxlist.at(i).first < min
            && minMaxlist.at(i).first > actualMinima){
            min= minMaxlist.at(i).first;
            retVal=i;
        }
      }
    index = retVal;
    if(retVal < 0){
        return false;
    }
    actualMinima= minMaxlist.at(retVal).first;
    actualMaxima= minMaxlist.at(retVal).second;


    clCandOutOfRangeLeftCl =
        (actMaxLeftList + eps) <= actualMinima;

    clCandOutOfRangeRightCl =
        (actMaxRightLsit + eps) <= actualMinima;

    return true;
  }
  
  
/*
pushMemberToClusterList

push a committed member at the end or beginning of a given clusterlist

*/ 
  template <class MEMB_TYP_CLASS, class TYPE>
  void Cluster<MEMB_TYP_CLASS,TYPE>:: 
  pushMemberToClusterList(bool front,MEMB_TYP_CLASS *member,
                          int list, Kind kind)
  {
    switch (kind){
      case NOISE:
        if(front)
        {
          noiseList.push_front(member);
        }else{
          noiseList.push_back(member);
        }
        break;
      case CLUSTERCAND:
        if(front)
        {
          clusterCandList.push_front(member);
        }else{
          clusterCandList.push_back(member);
        }
        break;
      case CLUSTER:
        if(front)
        {
          clusterArray.at(list).push_front(member);
        }else{
          clusterArray.at(list).push_back(member);
        }
        break;
      case LEFT:
        if(front)
        {
          leftPartCluster.at(list).push_front(member);
        }else{
          leftPartCluster.at(list).push_back(member);
        }
        break;
      case RIGHT:
        if(front)
        {
          rightPartCluster.at(list).push_front(member);
        }else{
          rightPartCluster.at(list).push_back(member);
        }
        break;
      case BOTH:
        if(front)
        {
          bothSideCluster.at(list).push_front(member);
        }else{
          bothSideCluster.at(list).push_back(member);
        }
        break;
      case CLCANDCLUSTERS:
        if(front)
        {
          clusterCandClusters.at(list).push_front(member);
        }else{
          clusterCandClusters.at(list).push_back(member);
        }
        break;
      default:
        break;
    }
  }
  
  
}