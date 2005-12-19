
//This file is part of SECONDO.

//Copyright (C) 20055555, University in Hagen, Department of Computer Science,
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** This is the Java implementation of HeapSort
  *
  */


public class HeapSort{


private static void reheap_std(Comparable[] heap,int i, int k){
   int j,son;
   j=i;
   boolean done = false;
   Comparable o1,o2;
   int cmp;
   while(2*j<=k && !done){
      if(2*j+1<=k){
         o1 = heap[2*j-1];
         o2 =heap[2*j];
         cmp = o1.compareTo(o2);

         if(cmp>=0)
            son = 2*j;
         else
            son = 2*j+1;
      }
      else
         son = 2*j;
      o1 = heap[j-1];
      o2 = heap[son-1];
      cmp = o1.compareTo(o2);
      if(cmp<=0){
         Comparable tmp = heap[j-1];
         heap[j-1] = heap[son-1];
         heap[son-1] = tmp;
         j = son;
      }
      else
         done=true;
   }
}

public static  void heapSort(Comparable[] Set){
   int n = Set.length;
   for(int i=n/2;i>0;i--)
      reheap_std(Set,i,n);
   for(int i=n;i>1;i--){
      Comparable tmp = Set[0];
      Set[0] = Set[i-1];
      Set[i-1] = tmp;
      reheap_std(Set,1,i-1);
   }
}

/** returns true if the set is ordered , fals otherwise **/

public static boolean checkOrder(Comparable[] Set){
  for(int i=0;i<Set.length-1;i++){
     if(Set[i].compareTo(Set[i+1])>0){
        return false;
     }
  }
  return true;

}


}
