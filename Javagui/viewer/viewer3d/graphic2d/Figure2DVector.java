//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

package viewer.viewer3d.graphic2d;

import gui.idmanager.*;


/*******************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
*
*********************************/


import java.util.* ;

/** this class provides a vector to store Figure2D-Objects */

class Figure2DVector {

/** the intern store */
private Vector V = new Vector();


/** add a new Figure2D */
public void append (Figure2D P) { 
    if(P!=null){
       V.add(P);
    }
}


/** returns the element at the specified position **/
public Figure2D get(int index){
   return (Figure2D) V.get(index);
}



/** sorts this vector by the value of the getSort() function **/
public void sort(){
  // sort using heapsort
  int n = V.size();
  for(int i=n/2;i>0;i--)
     reheap(i,n);
  for(int i=n;i>1;i--){
      Object tmp = V.get(0);
      V.set(0,V.get(i-1));
      V.set(i-1,tmp);
      reheap(1,i-1);
   }
}

/** function supporting the sort() function **/
private void reheap(int i, int k){
   int j,son;
   j=i;
   boolean done = false;
   while(2*j<=k && !done){
      if(2*j+1<=k){
         if(get(2*j-1).getSort()>get(2*j).getSort()){
            son = 2*j;
         } else {
            son = 2*j+1;
         }
      } else {
         son = 2*j;
      }
      if(get(j-1).getSort()<get(son-1).getSort()){
         // swap elements
         Object tmp = V.get(j-1);
         V.set(j-1, V.get(son-1));
         V.set(son-1, tmp);
         j = son;
      } else {
         done=true;
      }
   }
}


/** returns the number of containing figures */
public int getSize() { 
   return V.size(); 
}

/** get the figure on position i */
public Figure2D getFigure2DAt(int i) throws NoSuchElementException {
  return (Figure2D) V.get(i);
}

/** deletes all containing figures */
public void empty() { 
  V.clear(); 
}

/** check for emptyness */
public boolean isEmpty() { 
   return V.size()==0; 
}



/** deletes all figures with given ID */
public void deleteFiguresWithID(ID PID)  {

   // removes all Figures whith PID
  Vector nV = new Vector(V.size());
  int max = V.size();

  for(int i=0;i<max;i++){
    Figure2D fig = get(i);
    if(! fig.getID().equals(PID)){
      nV.add(fig);
    } 
  }  
  V = nV;
}


} // class
