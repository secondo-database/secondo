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

package fuzzyobjects.basic;

import java.util.Vector;
import java.io.Serializable;


/** provides a sorted container for basic objects */
public class SortedBasic implements Serializable{

/** the memory */
private Vector V;


/** creates a new empty container */
public SortedBasic(){
  V = new Vector(10);
}


/** deletes all objects in container */
public void makeEmpty(){
  V = new Vector(10);
}


/** returns a readable representaion of this container */
public String toString(){
 String result ="" + V.size()+" pts \n";
 for(int i=0;i<V.size();i++)
    result += V.get(i).toString()+"\n";
 return result;
}


/** returns the number of containing objects */
public int getSize(){ return V.size(); }


/** check for equality with SO */
public boolean equals(SortedBasic SB){
int max = SB.V.size();
if(V.size()!= max)
  return false;
else {
  boolean result=true;
  BasicObject E1,E2;
  for(int i=0;i<max & result;i++){
   E1 = (BasicObject) V.get(i);
   E2 = (BasicObject) SB.get(i);
   if(!E1.equals(E2))
     result = false;
  }
  return result;
}
}


/** returns a copy of this */
public SortedBasic copy(){
  SortedBasic result = new SortedBasic();
  result.V = new Vector(V.size());
  for(int i=0;i<V.size();i++)
    result.V.add(V.get(i));
  return result;
} // copy


/** this container is empty ? */
public boolean isEmpty() {
  return V.size()==0;
}



/**
  * insert a new Object in this container
  * <ul>
  *   <li> O(n) if memory of this container is full </li>
  *   <li> O(n) if insert before the last position </li>
  *   <li> O(1) else
  * </ul>
  */

public boolean insert(BasicObject BO){
// returns false if object whith same basic allready exists

boolean result = true;

if( V.size()==0)
    V.add(BO);                     // the first element
else
    // the first element
   if ( BO.compareTo( (BasicObject) V.get(0) ) <0)
      V.add(0,BO);
   else
      if( BO.compareTo( (BasicObject) V.get(V.size()-1) ) >0 )
         // last Element
         V.add(BO);
      else{
         // search insertPosition
        int min = 0;
        int max = V.size()-1;
        int mid;
 
        boolean found = false;
        BasicObject current;

        while (!found && (max-min)>1) {
           mid    = (max+min) / 2;
           current = (BasicObject) V.get(mid);
           if ( current.compareTo(BO)==0){
               found = true;
           }
           else
              if ( current.compareTo(BO) >0){
                 max = mid;
              }
              else {
                 min = mid;
              }
         } // while

         if (!found) {
            if( (BO.compareTo( (BasicObject)V.get(min))>0)   &&
                (BO.compareTo( (BasicObject)V.get(max))<0) )
                  V.add(max,BO);
            else
               result = false;
         }
         else
            result=false;
     } // else  search insertPosition

  return result;

} // insert



/**
  * returns the position of Object with basic BO
  * or -1 if not exists such one
  */
int getPos(BasicObject BO){
// returns -1 if BO not exsits

if(BO==null)
   return -1;

int result = -1;

 // in the cases : V is empty | BO<V.first or BO>V.Last is
 // nothing to do
 if (!(  (V.size()==0) ||
         (BO.compareTo( (BasicObject)V.get(V.size()-1) )>0 ) ||
         (BO.compareTo( (BasicObject)V.get(0))<0           ) )){
     int min = 0;
     int max = V.size()-1;
     int mid;
     boolean found = false;
     BasicObject current;
     while (!found && (max-min)>1) {
        mid = (max+min) / 2;
        current = (BasicObject) V.get(mid);
        if ( BO.compareTo(current)==0){
            found = true;
            min  = mid;
            max = mid;
        }
        else
          if (current.compareTo(BO) >0)
             max = mid;
          else
             min = mid;
      } // while

     if ( BO.compareTo( (BasicObject) V.get(min) ) == 0){
         result = min;
     }
     else
     if ( BO.compareTo( (BasicObject) V.get(max) ) ==0 ) {
        result =  max;
     }
     else{
        result = -1;
     }
 } // if possible in V
 return result;
} // getPos;




/** returns the object on position i */
public BasicObject get(int i){
  return (BasicObject)V.get(i);
}


/**
  * removes the object with basic BO;
  * returns false if not exists one
  */ 
public boolean delete(BasicObject BO){
 // search the Element
  int Pos = getPos(BO);
  if (Pos==-1)
    return false;
  else {
    V.remove(Pos);
    return true;
  }  
} // delete


public void delete(int i){
  if(i>=0 & i<V.size())
     V.remove(i);
}

/**
  * check whether this contains a object whith basic BO
  */
public boolean contains(BasicObject BO){
  return getPos(BO)>-1;
} // contains



}  // class 

