package fuzzyobjects.composite;

import java.io.*;

public class BoundingBox implements Serializable{

/** returns the maximum X of the bounding box */
public int getMaxX(){ return maxX;}
/** returns the maximum Y of the bounding box */
public int getMaxY(){ return maxY;}
/** returns the minimum X of the bounding box */
public int getMinX(){ return minX;}
/** returns the maximum of the bounding box */
public int getMinY(){ return minY;}

/** returns a readable representaion of the bounding box */
public String toString(){
  return "("+minX+","+minY+") -> ("+maxX+","+maxY+")";
}


/** set the box */
boolean setBox(int minX, int minY, int maxX, int maxY){
 if( (minX>maxX) | (minY>maxY) )
   return false;
 else{
  this.minX = minX;
  this.minY = minY;
  this.maxX = maxX;
  this.maxY = maxY;
  return true;
 }
}

/** returns a copy of this bounding box */
public BoundingBox copy(){
 BoundingBox result = new BoundingBox();
 result.minX = minX;
 result.minY = minY;
 result.maxX = maxX;
 result.maxY = maxY;
 return result;
}

/** the min-X-Value */
private int minX=0;
/** the min-Y-Value */
private int minY=0;
/** the max-X-Value */
private int maxX=0;
/** the max-Y-Value */
private int maxY=0;

}
