package fuzzyobjects.composite;

import fuzzyobjects.basic.*;
import fuzzyobjects.simple.*;
import java.util.Vector;
import java.io.Serializable;


/** provides a sorted container for simple objects */
public class SortedObjects implements Serializable{

/** the memory */
private Vector V;


/** creates a new empty container */
public SortedObjects(){
 V = new Vector(10);
}


/** deletes all objects in container */
public void makeEmpty(){
  V = new Vector(10);
}


/** returns a readable representaion of this container */
public String toString(){
 String result ="" + V.size()+" elems \n";
 for(int i=0;i<V.size();i++)
    result += V.get(i).toString()+"\n";
 return result;
}


/** returns the number of containing objects */
public int getSize(){ return V.size(); }



public int compareTo(SortedObjects SO){
   int max = Math.min(getSize(),SO.getSize());
   SimpleObject E1,E2;
   int comp;
   for(int i=0;i<max;i++){
      E1 = (SimpleObject) V.get(i);
      E2 = (SimpleObject) SO.V.get(i);
      comp= compare(E1,E2);
      if(comp!=0)
         return comp;
   }
   if(V.size()<SO.V.size())
      return -1;
   if(V.size()>SO.V.size())
      return 1;
   return 0;      
}


/** check for equality with SO */
public boolean equals(SortedObjects SO){
if(this==SO) return true;
if(SO.V==V) return true;
int max = SO.V.size();
if(V.size()!= max)
  return false;
else {
  boolean result=true;
  SimpleObject E1,E2;
  for(int i=0;i<max & result;i++){
   E1 = (SimpleObject) V.get(i);
   E2 = (SimpleObject) SO.get(i);
   if(!E1.equals(E2))
     result = false;
  }
  return result;
}
}


/** returns a copy of this */
public SortedObjects copy(){
  SortedObjects result = new SortedObjects();
  for(int i=0;i<V.size();i++)
    result.V.add(V.get(i));
  return result;
} // copy


/** this container is empty ? */
public boolean isEmpty() {
  return V.size()==0;
}


/** all containing objects are valid ? */
public boolean allValid(){
 int max = V.size();
 int h = 0;
 boolean ok = true;
 while(ok & h<max){
    ok = ok & ((SimpleObject)V.get(h)).isValid();
    h++;
 }//while
 return ok;
}

/** a compare function */
private static int compare(SimpleObject SO1,SimpleObject SO2){
 return SO1.basic().compareTo(SO2.basic());
}

/** a compare function */
private static int compare(BasicObject BO,SimpleObject SO){
 return BO.compareTo(SO.basic());
}

/** a compare function */
private static int compare(SimpleObject SO,BasicObject BO){
  return (SO.basic().compareTo(BO));
}



/**
  * insert a new Object in this container
  * <ul>
  *   <li> O(n) if memory of this container is full </li>
  *   <li> O(n) if insert before the last position </li>
  *   <li> O(1) else
  * </ul>
  */
public boolean insert(SimpleObject SO){
// returns false if object whith same basic allready exists

BasicObject BO = SO.basic();
boolean result = true;


if( V.size()==0)
    V.add(SO);                     // the first element
else
   if ( compare(BO,(SimpleObject)V.get(0))<0)  // the first element
      V.add(0,SO);
   else
      if( compare(BO,(SimpleObject)V.get(V.size()-1)) >0 )   // last Element
         V.add(SO);
      else{
         // search insertPosition
        int min = 0;
        int max = V.size()-1;
        int mid;
 
        boolean found = false;
        SimpleObject current;

        while (!found && (max-min)>1) {
           mid    = (max+min) / 2;
           current = (SimpleObject) V.get(mid);
           if ( compare(current,SO)==0){
               found = true;
           }
           else
              if ( compare(current,SO) >0){
                 max = mid;
              }
              else {
                 min = mid;
              }
         } // while
         if (!found) {
            if  (  compare(BO,(SimpleObject)V.get(min))>0    &&
                   compare(BO,(SimpleObject)V.get(max))<0      )
                  V.add(max,SO);
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
 if (!(  (V.size()==0) || (compare(BO,(SimpleObject)V.get(V.size()-1))>0)
      || (compare(BO, (SimpleObject)V.get(0))<0) )){


     int min = 0;
     int max = V.size()-1;
     int mid;
     boolean found = false;
     SimpleObject current;
     while (!found && (max-min)>1) {
        mid = (max+min) / 2;
        current = (SimpleObject) V.get(mid);
        if ( compare(current,BO)==0){
            found = true;
            min  = mid;
            max = mid;
      }
     else
        if (compare(current,BO) >0)
           max = mid;
        else
           min = mid;
    } // while

    if ( compare( (SimpleObject) V.get(min) , BO ) == 0){
       result = min;
    }
    else
      if ( compare( (SimpleObject) V.get(max) , BO) ==0 ) {
        result =  max;
     }
     else{
        result = -1;
     }
 } // if possible in V
 return result;


} // getPos;



/**
  * overwrite the object with the same basic as SO
  * returns false if not exists such object
  */
public boolean update(SimpleObject SO){

 int Pos = getPos(SO.basic());
 if (Pos==-1 )
   return false;
 else{
   V.set(Pos,SO);
   return true;
 }

} // update


/**
  * returns the object with basic BO or null if not exists one
  */
public SimpleObject search(BasicObject BO) {
  int Pos = getPos(BO);
  if (Pos==-1)
     return null;
  else
     return((SimpleObject)V.get(Pos));
} // search


/** returns the object on position i */
public SimpleObject get(int i){
  return (SimpleObject)V.get(i);
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


/**
  * check whether this contains a object whith basic BO
  */
public boolean contains(BasicObject BO){
  return getPos(BO)>-1;
} // contains


/**
 * check whether this contains SO
 */
public boolean contains(SimpleObject SO){
  int Pos = getPos(SO.basic());
  if (Pos==-1)
    return false;
  else{
    SimpleObject O = (SimpleObject) V.get(Pos);
    return SO.equals(O);
  }
} // contains

}  // class 

