package fuzzyobjects.basic;

import fuzzyobjects.*;
import java.util.Vector;

/**
 * this class provides points for a fixed triangulation of a
 * subset of euclidian space.
 * This triangulation is called the X-triangulation.
 * @author Thomas Behr
 */
public class BasicPoint implements BasicObject{

/**
 * creates a new BasicPoint on (x,y)
 * here is no check wether (x,y) is a valid BasicPoint
 * @param x the x-coordinate of the new point
 * @param y the y-coordinate of the new point
 * @see #isValid()
 */

public BasicPoint(int x,int y) {
  this.x = x;
  this.y = y;
}


/**
 * computes the BasicPoints of this BasicPoint
 * @return a array of length 1 with a copy of this as content
 */
public BasicPoint[] getBasicPoints(){
// return a array with a copy from this as element
 BasicPoint[] Ps = new BasicPoint[1];
 Ps[0] = this.copy();
 return Ps;
}

/**
 * convert this point in a readable form
 * @return a String representing this point
 */
public String toString(){
// no comment
 return "BP : ("+x+","+y+")";
}

/**
 * computes a copy of this
 * @return a new BasicPoint on the same location as this
 */
public BasicPoint copy() {
  return new BasicPoint(x,y);
}

/**
 * check for equality
 * @param B the another BasicPoint
 * @return true if B is on the same location as this
 *  otherwise false
 */
public boolean equals(BasicPoint B) {
  return x==B.x & y==B.y;
}

/**
 * computes the distance to a another BasicObject
 * @param BO the another BasicObject
 * @return the smallest euclidic distance between this and all
 *         BasicPoints from BO
 * @see #q_euclid_distance(BasicObject)
 */
public double euclid_distance(BasicObject BO) {
// returns the smallest euclidic distance from this to
// all Basicpoints from BO
  return Math.sqrt(q_euclid_distance(BO));
}

/**
 * computes the quadratic euclidic distance to
 * another BasicObject
 * @param BO the another BasicObject
 * @return the square of the smallest euclidic distance to all
 *  BasicPoints from BO
 * @see #euclid_distance(BasicObject)
 */
public double q_euclid_distance(BasicObject BO) {
// get the quadratic euclidic distance
  BasicPoint[] BPs = BO.getBasicPoints();
  double min = ( (BPs[0].x-x)*(BPs[0].x-x)
                +(BPs[0].y-y)*(BPs[0].y-y) );
  double current;
  for(int i=1;i<BPs.length;i++) {
     current = ( (BPs[i].x-x)*(BPs[i].x-x) +
                 (BPs[i].y-y)*(BPs[i].y-y) );
      if (current<min)
          min = current;
  }
  return (min);
}

/**
 * check wether this is on a coordinate of the X-triangulation
 * @return true if this a valid BasicPoint
 */
public boolean isValid() {
  int a = fuzzyobjects.Params.a;
  int b = fuzzyobjects.Params.b;
  int dx = (x/a)*a;
  int dy = (y/b)*b;
  int x1 = Math.abs(x-dx);
  int y1 = Math.abs(y-dy);
  boolean ok =  (x1==0   & y1==0) | (x1==a/2 & y1==b/2);
  return ok;
} // isValid


/**
 * check if (x,y) on a valid BasicPoint
 * @param x the x-coordinate of the to checked point
 * @param y the y-coordinate of the to checked point
 * @return true if (x,y) on a valid Point of the X-Triangulation
 */
public static boolean isBasicPoint(double x, double y){
 int ix = (int) x;
 int iy = (int) y;
 if( ix==x && iy==y)  // x and y are integers ?
   return (new BasicPoint(ix,iy)).isValid();
 else
   return false;
}

/**
 * compares this with another BasicObject
 * @param O the another BasicObject
 * @return
 * <ul>
 * <li> -1 if O not a BasicPoint or this is smaller then O </li>
 * <li>  0 if this equals to O                             </li>
 * <li>  1 if O is a BasicPoint and this greater then O    </li>
 * </ul>
 */
public int compareTo(BasicObject O){
 if(!(O instanceof BasicPoint))
    return -1;
 BasicPoint BP = (BasicPoint) O;

 if ((x<BP.x) || ( (x==BP.x) & (y<BP.y)))
    return -1;
 else
    if ((x==BP.x) & (y==BP.y))
       return 0;
    else
       return 1;
} // compareTo


/**
 * computing the max. BasicPoint of a given array of BasicPoints
 * using the compareTo-method
 * @param BPs the array containing the BasicPoints
 * @return the maximum BasicPoint in Bps
 * @see #compareTo(BasicObject)
 * @see #getMin(BasicPoint[])
 */
public static BasicPoint getMax(BasicPoint[] BPs){
 if (BPs.length==0)
    return null;
 else {
    BasicPoint Max = BPs[0];
    for(int i=1;i<BPs.length;i++)
       if (BPs[i].compareTo(Max)==1)
           Max = BPs[i];
    return Max;
 }
} // getMax

/**
 * computes the minimum in a array of BasicPoints
 *  using the compareTo-Method
 * @param BPs the array of BasicPoint
 * @return the smallest BasicPoint containing in Bps
 * @see getMax(BasicPoint[])
 * @see compareTo(BasicObject)
 */
public static BasicPoint getMin(BasicPoint[] BPs){
 if (BPs.length==0)
    return null;
 else {
    BasicPoint Min = BPs[0];
    for(int i=1;i<BPs.length;i++)
       if (BPs[i].compareTo(Min)==-1)
           Min = BPs[i];
    return Min;
 }
} // getMax


/**
 * sort a array of BasicPoints
 * in O(Bps.length<sup>2</sup>)
 * @param Bps to sorted array of BasicPoints
 * @return the sorted array
 */
public static void sort( BasicPoint[] BPs) {
for (int i=0; i<BPs.length;i++)
  for (int j=i+1; j<BPs.length;j++)
     if ( BPs[i].compareTo(BPs[j])==1) {
       BasicPoint temp = BPs[i];
       BPs[i] = BPs[j];
       BPs[j] = temp;
     }
} // sort


/**
 * check wether B is a neightboor of this
 *  i.e. is this connected with B by a BasicSegment ?
 * @param B to checked BasicPoint
 * @return true if B connected with this by a BasicSegment
 */
public boolean neightbooring(BasicPoint B){
 // check, wether this and B are connected by a BasicSegment 

 int a = fuzzyobjects.Params.a;
 int b = fuzzyobjects.Params.b;

 boolean result;
 int dx = Math.abs(x-B.x);
 int dy = Math.abs(y-B.y);
 if ((x/a)*a ==x){
  // a corner of rectangle of the X triangulation
  result = (  dx==0   &  dy == b )   ||  // horizontal
           (  dx==a   &  dy == 0 )   ||  // vertical
           (  dx== a/2 & dy == b/2);     // diagonal
 }
 else { // middle of the X
   result= (  dx== a/2 & dy == b/2);     // diagonal
 }
 return result;
}


/**
 * computes all neightboors of this in the X-Triangulation
 * @return a array containing all neightboors of this
 * @see neightbooring(BasicPoint)
 */
public BasicPoint[] getNeightboors(){
  if (!isValid())
     return null;
  else {
      int a = Params.a;
      int b = Params.b;
      boolean isGridPoint = (x/a)*a==x;  //else middle of a X
      BasicPoint[] candidates;
      if (isGridPoint) {
         candidates = new BasicPoint[8];
         candidates[0] = new BasicPoint(x-a,y  );
         candidates[1] = new BasicPoint(x  ,y-b);
         candidates[2] = new BasicPoint(x+a,y  );
         candidates[3] = new BasicPoint(x  ,y+b);
         candidates[4] = new BasicPoint(x-(int)(0.5*(double)a),
	                                y-(int)(0.5*(double)b));
         candidates[5] = new BasicPoint(x+(int)(0.5*(double)a),
	                                y-(int)(0.5*(double)b));
         candidates[6] = new BasicPoint(x-(int)(0.5*(double)a),
	                                y+(int)(0.5*(double)b));
         candidates[7] = new BasicPoint(x+(int)(0.5*(double)a),
	                                y+(int)(0.5*(double)b));
      }
      else {  // a Point in the middle of a X
         candidates = new BasicPoint[4];
         candidates[0] = new BasicPoint(x-(int)(0.5*(double)a),
	                                y-(int)(0.5*(double)b));
         candidates[1] = new BasicPoint(x+(int)(0.5*(double)a),
	                                y-(int)(0.5*(double)b));
         candidates[2] = new BasicPoint(x-(int)(0.5*(double)a),
	                                y+(int)(0.5*(double)b));
         candidates[3] = new BasicPoint(x+(int)(0.5*(double)a),
 	                                y+(int)(0.5*(double)b));
      }
     return candidates;
   } // isValid
}

/**
 * get the x-coordinate of this
 * @return the x-coordinate of this
 */
public int getX() { return x;}

/**
 * get the y-coordinate of this
 * @return the y-coordinate of this
 */
public int getY() { return y;}

/** computes a path between 2 BasicPoints */
public BasicPoint[] computePath(BasicPoint BP){
 if( !isValid() | !BP.isValid() ) return null;
 else{
   int a = fuzzyobjects.Params.a;
   int b = fuzzyobjects.Params.b;
   Vector Path = new Vector();
   int CurrentX = x;
   int CurrentY = y;
   int GoalX = BP.x;
   int GoalY = BP.y;
   Path.add(new BasicPoint(CurrentX,CurrentY));
   int sx=1;
   int sy=1;

   while(!(CurrentX==GoalX & CurrentY==GoalY)){
     // compute the next Point
     if (CurrentX<GoalX)
        sx=1;
     else
        sx=-1;
     if (CurrentY<GoalY)
        sy=1;
     else
        sy=-1;

     if( (CurrentX/a)*a==CurrentX) {
     // current point on corner of rectangle
        if(CurrentX == GoalX)
           CurrentY = CurrentY + sy*b;
        else 
          if(CurrentY == GoalY)
            CurrentX = CurrentX + sx*a;
          else{ // x and y not ready
            // check diagonale
            int tx = CurrentX + sx*a/2;
            int ty = CurrentY + sy*b/2;
            if(tx==GoalX){
              if(ty==GoalY){
                 CurrentX=tx;
                 CurrentY=ty;
              }
              else
                CurrentY = CurrentY + sy*b;
            } // tx != GoalX
            else{ 
               if (ty==GoalY)
                  CurrentX = CurrentX+sx*a;
               else{
                 CurrentX = tx;
                 CurrentY = ty;
               }
            }
           } // x and y not ready
     }  // current Point on corner of rectangle
     else{ //current point on a diagonale
        CurrentX = CurrentX + sx*a/2;
        CurrentY = CurrentY + sy*b/2;
     }
     Path.add(new BasicPoint(CurrentX,CurrentY));
   }
   BasicPoint[] result = new BasicPoint[Path.size()];
   for(int i=0;i<Path.size();i++)
        result[i] = (BasicPoint) Path.get(i);

   return result;

 }
}


/** returns the mininum X of the "bounding box" */
public int getMinX(){ return x; }
/** return the minimum Y of the "bounding box" */
public int getMinY(){ return y; }
/** return the maximum X of the "bounding box" */
public int getMaxX(){ return x; }
/** returns the maximum Y of the "bounding box" */
public int getMaxY(){ return y; }

/** location of this BasicPoint */
private int x,y;
}
