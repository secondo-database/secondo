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
 
 
 #include "AlgebraTypes.h"
 #include "RelationAlgebra.h"
 #include "StandardTypes.h"
 #include "Stream.h"
 #include "TupleInfo.h"
 #include "Cluster.h"
 //  #include "TupleInfo.h"
 //  #include "Point.h"
 #include <utility>
 
 #ifndef DBDACSCANGEN_H
 #define DBDACSCANGEN_H
 using namespace std;
 
 namespace distributedClustering{
   
   template <class MEMB_TYP_CLASS, class TYPE>
   class DbDacScanGen{
   private:
/* 
 1.3 members

*/
     int minPts, attrPos,pos;
     double eps;
     TupleBuffer* buffer;
     GenericRelationIterator* resIt;  // iterator 
     TupleType* tt;   // the result tuple type
     vector <MEMB_TYP_CLASS*> membArrayUntouched;
     vector <MEMB_TYP_CLASS*> membArrayPtr;
     Cluster<MEMB_TYP_CLASS, TYPE>* cluster;
     
     
     
   public:
     
/*
 1.4 constructors

*/
     DbDacScanGen(Word _inStream, ListExpr _tupleResultType, double _eps, 
                  int _minPts, int _attrPos, size_t _maxMem): 
                  minPts(_minPts), 
                  attrPos(_attrPos), 
                  eps(_eps),buffer(0), 
                  resIt(0),tt(0),cluster(0)
    {
      tt = new TupleType(_tupleResultType);
      init(_maxMem,_inStream);
//       cluster = dbDacScan(membArray,0,membArray.size()-1,eps,minPts);
      
//       for( int i=0; i< membArrayPtr.size();i++){
//         cout << "membArray at Pos i= " << i;
//         membArrayUntouched[i]->printPoint();
//         cout << endl;
//         cout << "membArrayPtr at Pos i= " << i;
//         membArrayPtr[i]->printPoint();
//         cout << endl;
//       }
      
      cluster = dbDacScan(membArrayPtr,0,membArrayPtr.size()-1,eps,minPts);
      
//       cout << "Clustering finished!!-----------------------" << endl;
//       cluster->printAll();
//       cout << "Count of elements before clustering: " 
//       << membArrayUntouched.size() << endl;
//       cout << "Count of elements after clustering: " 
//       << cluster->getCntMembers() <<"\n" << endl;
//       cout << "Count of Clusters: " 
//       << cluster->getClusterArraySize() << endl;
//       cluster->findEqualElements(membArrayUntouched);
//       
      
      initOutput(); 
    }
    
/*
  Destructor
  
*/
    ~DbDacScanGen(){
      if(buffer)
        delete buffer;
      if(cluster)
        delete cluster;
      if(resIt) 
        delete resIt;
      if(tt) 
        tt->DeleteIfAllowed();
      //TODO delete membArray
    }
    
/*
 initOutput()
Starts the begin of returning tuples.

*/
    void initOutput(){
      if(resIt) delete resIt;
      resIt = buffer->MakeScan(); 
      pos=0;
    }
    
/*
* next()
* Returns the next output tuple.

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
        resTuple->PutAttribute(noAttr, new CcInt(true, 
                               membArrayUntouched[id]->getClusterNo()));
        resTuple->PutAttribute(noAttr+1, 
                               new CcBool(true,
                               membArrayUntouched[id]->isDensityReachable()));
        return resTuple;
      } else {
        return 0;
      }
    }
                  
                  
   private:
     
/*
 1.5 initialize
 
*/
     void init(size_t maxMem, Word _stream){
       Tuple* tuple;
       buffer = new TupleBuffer(maxMem);
       Stream<Tuple> inStream(_stream);
       inStream.open();
       while((tuple = inStream.request())){
          buffer->AppendTuple(tuple);
         TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
         if(obj->IsDefined()){
           MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
           membArrayUntouched.push_back(member);
           membArrayPtr.push_back(member);
         }
         tuple->DeleteIfAllowed();
       }
       inStream.close();
     }
     
     
/*
 dbDacScan
 
*/
     Cluster<MEMB_TYP_CLASS, TYPE>* 
        dbDacScan(vector<MEMB_TYP_CLASS*>& _membArray, 
                                                  int left , int right , 
                                                  double eps, int minPts)
     {
       if(right==left){//Array contains only one element
         return 0;
       }else{
         int globMedian = (right + left)/2;//position to the right subarray
         
         // sort the splitted array
         mergeSort(_membArray,left,right+1);
         
         //get left and right cluster
         Cluster<MEMB_TYP_CLASS, TYPE> *rightCluster, *leftCluster;
         leftCluster = dbDacScan(_membArray,left,globMedian,eps,minPts);
         rightCluster = dbDacScan(_membArray,globMedian+1,right,eps,minPts);
         
         Cluster<MEMB_TYP_CLASS, TYPE>* newCluster;
         
         if( rightCluster ==0 && leftCluster==0)
         {// right or left isn´t a cluster yet
           newCluster =
            new Cluster<MEMB_TYP_CLASS, TYPE>(_membArray[globMedian],
                                               _membArray[right], eps,minPts);
           return newCluster;
           
         } else
           if(leftCluster !=0){
             if(rightCluster !=0){ //there exists two clusters
               leftCluster->meltClusters(rightCluster,
                                         _membArray[globMedian]->getPoint(),
                                         _membArray[globMedian+1]->getPoint());
               return leftCluster;
             }else{ // right Cluster == 0
               leftCluster->addMember((_membArray[right]));
               return leftCluster;
             }
           }
       }
       return 0; //should never reached
     }
     
/*
* mergeSort
* sort an array in ascending order

*/
     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       MEMB_TYP_CLASS ** auxiliaryArray = new MEMB_TYP_CLASS*[right-left+1];
       if(auxiliaryArray!= 0){
         mergeSort(array,left,right,auxiliaryArray);
         
         delete [] auxiliaryArray;
       }
     }
     
/*
* mergeSort
* sort an array in ascending order

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
* leftIsMax()
* auxiliary fuction to compare the maximum Object with the left object

*/
     bool leftIsMax(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       bool retVal = false;
       
       double leftXVal = array[left]->getXVal();
       double rightXVal = array[right]->getXVal();
       
       leftXVal > rightXVal ? retVal = true : retVal = false;
       return retVal;
     }
     
   };
   
 }
 #endif