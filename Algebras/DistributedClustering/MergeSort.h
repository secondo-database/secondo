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

[1] Merge Sort

August-February 2015, Daniel Fuchs 

[TOC]

1 Overview


This is a implentation for merge sort. 

1.1 Includes

*/ 


 #include <vector>
 #include "Member.h"

 #ifndef Member_H
 #define Member_H
 namespace distributedClustering{
   /*
    1.16 ~mergeSort~
    
    sort an array with merge sort algorithm
    
    */
   template <class TYPE,class MEMB_TYP_CLASS>
   void mergeSort(std::vector<MEMB_TYP_CLASS*>& array,int left, int right){
     
     if(right == left+1)
       return ; //mergeSort finisch
       else{
         int median = (right - left)/2;
         int h_r = left + median; //position to the right subarray
         
         //divide 
         mergeSort<TYPE,MEMB_TYP_CLASS>(array, left, h_r);
         mergeSort<TYPE,MEMB_TYP_CLASS>(array, h_r, right);
         
         //merge - sort into subarray
         int h_l = left;
         MEMB_TYP_CLASS * auxArray[right-left+1];
         
         for(int i = 0; i < (right - left); i++){
           if(h_l < left+median && (h_r==right || 
             array[h_l]->getXVal() > array[h_r]->getXVal()))
           {
             auxArray[i]=array[h_l];
             h_l++;
           }
           else{
             auxArray[i]= array[h_r];
             h_r++;
           }
         }
         //copy subarray back
         for(int i=left; i < right; i++){
           array[i]=auxArray[i-left];
         }
       }
       
   }
   
 }
 #endif
