package fuzzyobjects.basic;

import java.util.Vector;

/**
 * this class manage a serie of connected BasicPoints
 * the Path has no selfcuts and can build a circle
 */

public class Path{

/** creates a new  empty Path */
public Path(){
  Points = new Vector();
}

/** check the emptyness of this path */
public boolean isEmpty(){
  return Points.size()==0;
}

/** deletes all points from this path */
public void makeEmpty(){
  if(Points.size()>0)
     Points = new Vector();
}

/** returns this path in a readable form */
public String toString(){
  if(Points.size()==0)
     return("empty Path");
  else{
    String result = "";
    int max = Points.size();
    for(int i=0;i<max;i++){
       result += Points.get(i);
       if(i<max-1)
         result += " -> ";
    }
    return result;
  }
}


/** invert this Path */
public void invert(){
 int s = Points.size();
 Vector NPs = new Vector(s);
 for(int i=1;i<=s;i++)
   NPs.add(Points.get(s-i));
 Points = NPs;
}

/** computes a path between 2 BasicPoints */
public static Path computePath(BasicPoint BP1, BasicPoint BP2){
 Path result = new Path();
 if(BP1.isValid() & BP2.isValid()){
   BasicPoint[] BPS = BP1.computePath(BP2);
   BasicPoint BP;
   int x,y;
   for(int i=0;i<BPS.length;i++){
      BP = BPS[i];
      x = BP.getX();
      y = BP.getY();
      if(i==0){
         result.minX = x;
         result.maxX = x;
         result.minY = y;
         result.maxY = y;
      }
      else{
        if(x<result.minX)
           result.minX = x;
        if(x>result.maxX)
           result.maxX = x;
        if(y<result.minY)
           result.minY = y;
        if(y>result.maxY)
           result.maxY = y;
      }
      result.Points.add(BP);
   }
 }
  return result;
}

/** returns the length(number of containing Points) of
  * this Path
  */
public int length(){
  int result = Points.size();
  // check of circle
  if(result>1)
    if (Points.get(0).equals(Points.get(result-1)))
      result--;
 return result;
}

/** returns the BasicPoint on position pos */
public BasicPoint getPoint(int pos){
  if( pos<0 | pos>= Points.size())
    return null;
  else
    return (BasicPoint) Points.get(pos);
}

/** returns the first Point in Path */
public BasicPoint getFirstPoint(){
  if(Points.size()==0)
    return null;
  else
    return (BasicPoint) Points.get(0);
}

/** returns the last Point in Path */
public BasicPoint getLastPoint(){
  if(Points.size()==0)
    return null;
  else
    return (BasicPoint) Points.get(Points.size()-1);
}


/** extend the Path with P
  * the first Point of P must be equals to the last Point
  * of this Path, this Path can not be a circle
  * this Path and P can not have cuts
  * @return true if sucessfull
  */

public boolean extend(Path P){

int length  = Points.size();
int Plength = P.Points.size();

if( Plength==0)
  return true;           // no change this Path

BasicPoint OldLast  = (BasicPoint) Points.get(length-1);
BasicPoint NewFirst = (BasicPoint) P.Points.get(0);

if(!OldLast.equals(NewFirst))
  return false;

boolean overlappingLine = false;

BasicSegment FromThis;
BasicSegment FromExtend;
for(int i=0;i<length-1;i++){
   FromThis = new BasicSegment(
                    (BasicPoint)Points.get(i),
                    (BasicPoint)Points.get(i+1));
   for(int j=0;j<Plength-1;j++) {
       FromExtend = new BasicSegment(
                           (BasicPoint)P.Points.get(j),
                           (BasicPoint)P.Points.get(j+1));
       if(FromThis.equals(FromExtend))
          overlappingLine = true;
   }
}

if(overlappingLine){
   return false;
}

// extend  the Path
for(int j=1;j<Plength;j++)
    Points.add(P.Points.get(j));

if(P.maxX>maxX)
  maxX = P.maxX;
if(P.minX<minX)
  minX=P.minX;
if(P.maxY>maxY)
  maxY=P.maxY;
if(P.minY<minY)
  minY=P.minY;
return true;
}


/** extend this path to BP
 * first is computed a path from the current last
 * point to BP
 * if possible (see extend(PATH)) is this extended
 *  with the new Path
 **/

public boolean extend(BasicPoint BP){

int size = Points.size();

if (BP==null)
  return false;

// the first Point ?

if(size==0){
  Points.add(BP);
  minX = BP.getX();
  maxX = minX;
  minY = BP.getY();
  maxY = minY;
  return true;
}

boolean overlappingLine = false;
BasicSegment BS;
BS = new BasicSegment((BasicPoint)Points.get(size-1),BP);
BasicSegment BS_old;
for(int i=0;i<size-1;i++){
  BS_old = new BasicSegment((BasicPoint)Points.get(i),
                            (BasicPoint)Points.get(i-1));
  if( BS.equals(BS_old))
    overlappingLine = true;
}

if(overlappingLine)
  return false;

if(!BP.neightbooring( (BasicPoint) Points.get(size-1)))
  return false;

Points.add(BP);
int x = BP.getX();
int y = BP.getY();

if(x<minX)
  minX = x;
if(x>maxX)
  maxX = x;
if(y<minY)
  minY=y;
if(y>maxY)
  maxY=y;

return true;

}


/** returns the number of containing segments */
public int getNumberOfSegments(){
 int NOP = Points.size();
 return NOP-1;
}


/** returns the segment on position no */
public BasicSegment getSegment(int no){
 if(no<0 | no>getNumberOfSegments())
   return null;
 else
  return new BasicSegment((BasicPoint)Points.get(no),
                          (BasicPoint)Points.get(no+1));
}


/**
  * returns the length of this path
  * (sum of lengths of the containing segments)
  */
public double getLength(){
  int max = getNumberOfSegments();
  double Length=0;
  for(int i=0;i<max;i++){
    Length += getSegment(i).length();
  }
  return Length;
}


/** returns the minimum x of the bounding box */
public int getMinX(){return minX;}
/* returns the minimum Y of the bounding box */
public int getMinY(){return minY;}
/** returns the maximum X of the bounding box */
public int getMaxX(){return maxX;}
/** returns the maximum Y of the bounding box */
public int getMaxY(){return maxY;}


/** returns the first Position from BP or
  * -1 if BP not contained
  */
public int getPos(BasicPoint BP){
int result = -1;
BasicPoint Current;
  for(int i=0;i<Points.size() & result<0; i++){
     Current = (BasicPoint) Points.get(i);
     if(Current.equals(BP))
        result = i;
  }
return result;
}


/** updates the bounding box */
protected void computeBoundingBox(){
 if(Points.size()==0){
    minX=0;
    maxX=0;
    minY=0;
    maxY=0;
 }
 else{
  BasicPoint BP = (BasicPoint) Points.get(0);
   minX = BP.getX();
   maxX = minX;
   minY = BP.getY();
   maxY = minY;
   int x,y;
   for(int i=1;i<Points.size();i++){
      BP = (BasicPoint) Points.get(i);
      x = BP.getX();
      y = BP.getY();
      if(x<minX)
        minX=x;
      if(x>maxX)
        maxX=x;
      if(y<minY)
        minY=y;
      if(y>maxY)
        maxY=y;
   }
 }
}


protected Vector Points;
/** the Bounding Box */
/** the minimum X of the bounding box */
protected int minX =0;
/** the minimum Y of the bounding box */
protected int minY =0;
/** the maximum X of the bounding box */
protected int maxX =0;
/** the maximum Y of the bounding box */
protected int maxY =0;

}
