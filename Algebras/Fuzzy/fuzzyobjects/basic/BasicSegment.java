package fuzzyobjects.basic;

import fuzzyobjects.*;

/**
 * in this class are BasicSegments of the X-triangulation
 * defined
 * @author Thomas Behr
 */
public class BasicSegment implements BasicObject{


/**
 * creates a new BasicSegment between P1 and P2
 *   no check whether this
 *   results a valid BasicSegment
 * @see isValid()
 */
public BasicSegment(BasicPoint P1, BasicPoint P2) {
// creates a BasicSegment from the parameters
// no checking of correctness
// the points are ordered 
 if (P1.compareTo(P2)<0){
     EP_1 = P1;
     EP_2 = P2;
 }
 else{
   EP_2 = P1;
   EP_1 = P2;
 }
}

/**
 * returns the BasicPoints of this
 * @return the both endpoints of this in a array
 */
public BasicPoint[] getBasicPoints(){
// returns the two endpoints of this segment in a array
 BasicPoint[] BPs = new BasicPoint[2];
 BPs[0] = EP_1.copy();
 BPs[1] = EP_2.copy();
 return BPs;
}

/**
 * provides a readable form of this
 * @return a String representing this
 */
public String toString(){
 return "BL : ("+EP_1+"->"+EP_2+")";
}

/**
 * returns a copy of this
 * @return a new BasicSegment on the same location as this
 */
public BasicSegment copy() {
   return new BasicSegment(EP_1.copy() , EP_2.copy());
}


/**
 * check equality with L
 * @return true if L is on the same location as this
 */
public boolean equals(BasicSegment L) {
// check for Equality whith L 
  return (EP_1.equals(L.EP_1) && EP_2.equals(L.EP_2) );
}

/**
 * check: is this a Segment of the X-triangulation ?
 * @return true if this is a valid segment  i.e.
 * both endpoints are valid and neightbooring
 */
public boolean isValid(){
 boolean ok = EP_1.isValid() & EP_2.isValid();
 if(ok){
    return EP_1.neightbooring(EP_2);
 }
 return false;
}

/**
 * compares this with another BasicObject
 * @param O the BasicObject to compare
 * @return
 * <ul>
 * <li>-1 if O not a BasicSegment or this is smaller then O </li>
 * <li> 0 if O equals to this                               </li>
 * <li> 1 if O is a BasicSegment and this is greater then O </li>
 * </ul>
 */
public int compareTo(BasicObject O) {
if (! (O instanceof BasicSegment))
   return -1;
BasicSegment L = (BasicSegment) O;
int c = EP_1.compareTo(L.EP_1);
if (c!=0)
   return c;
else
   return EP_2.compareTo(L.EP_2);
}

/** check whether P is a EndPoint of this
  * @param P the to checked BasicPoint
  * @return true if P is a endpoint of this
  */
public boolean isEndpoint(BasicPoint P) {
 return EP_1.equals(P) || EP_2.equals(P);
}

/**
 * check whether L and this have one common endpoint
 * @param L the to checked BasicSegment
 * @return true if L and this are connected
 */
public boolean connected(BasicSegment L) {
// **** check whether L is connected with this Segment ******
return !this.equals(L)  &&
       ( EP_1.equals(L.EP_1) || EP_1.equals(L.EP_2) ||
         EP_2.equals(L.EP_1) || EP_2.equals(L.EP_2)    );
}

/**
 * computes the length of this segment
 * @return the euclic distance between both endpoints
 */
public double length(){
 return EP_1.euclid_distance(EP_2);
}

/**
 * computes the euclidic distance between this an
 * another BasicObject
 * @param BO the another BasicObject
 * @return the smallest euclidic distance between
 * both endpoints to all BasicPoints of BO
 */
public double euclid_distance(BasicObject BO){
  double m1 = EP_1.q_euclid_distance(BO);
  double m2 = EP_2.q_euclid_distance(BO);
  return Math.sqrt( Math.min(m1,m2));
}

/**
 * computes the quadratic euclidic distance to BO
 * @params BO the another BasicObject
 * @return the smallest quadratic euclidic distance between both
 * endpoints of this to all BasicPoints from BO
 * @see euclidic_distance(BasicObject)
 */
public double q_euclid_distance(BasicObject BO){
  double m1 = EP_1.q_euclid_distance(BO);
  double m2 = EP_2.q_euclid_distance(BO);
  return ( Math.min(m1,m2));
}

/**
 * check whether (x,y) is a point on this segment
 * @param x the x-coordinate of the to checked point
 * @param y the y-coordinate of the to checked point
 * @return true if (x,y) contained in the pointset of this
 *  segment
 */
public boolean on(double x, double y){
// check : "(x,y) is a Point on this Segment ?"
 double x1 = EP_1.getX();
 double x2 = EP_2.getX();
 double y1 = EP_1.getY();
 double y2 = EP_2.getY();
 if(x2!=x1){
    double delta = (x-x1) / (x2-x1);
    if ((delta<0) | (delta>1))
       return false;
    else
       return y == y1 + delta*(y2-y1);
 }
 else{
   double minY = Math.min(y1,y2);
   double maxY = y1+y2-minY;
   return x==x1 & y>=minY & y<=maxY;
 }
}

/**
 * get a endpoint of this segment
 * @return the smallest endpoint of this
 */
public BasicPoint getEP1(){
    return EP_1;
}

/**
 * get a endpoint of this segment
 * @return the greates endpoint of this
 */
public BasicPoint getEP2(){
    return EP_2;
}

/**
 * computes all neigtboors of this segment
 * @return all segments which have one common Point with this
 */
public BasicSegment[] getNeightboors(){
  BasicPoint[] ToEP1 = EP_1.getNeightboors();
  BasicPoint[] ToEP2 = EP_2.getNeightboors();
  BasicSegment[] result;
  result = new BasicSegment[ ToEP1.length + ToEP2.length -2 ];
  // all neightbooring points without EP1 or EP2 respectively
  int j=0;  // current position in result
  for(int i=0;i<ToEP1.length;i++)
    if (!ToEP1[i].equals(EP_2)) {
       result[j]= new BasicSegment(ToEP1[i],EP_1);
       j++;
    }
  for(int i=0;i<ToEP2.length;i++)
     if(!ToEP2[i].equals(EP_1)){
       result[j]= new BasicSegment(ToEP2[i],EP_2);
       j++;
     }
 return result;
}

/**
  * computes the  2 BasicTriangles having this as a side
  * @return the triangles
  */
public BasicTriangle[] getTriangles(){
BasicPoint[] Ns1 = EP_1.getNeightboors();
BasicPoint[] Ns2 = EP_2.getNeightboors();

BasicPoint cN1 = null;  // common neightboors
BasicPoint cN2 = null;  
boolean firstfound = false;
boolean secondfound = false;

for(int i=0;i<Ns1.length;i++)
  for(int j=0;j<Ns2.length;j++)
     if(Ns1[i].equals(Ns2[j])) {
       if(!firstfound){
        firstfound=true;
        cN1 = Ns1[i];
       }
       else{
         secondfound=true;
         cN2 =Ns1[i];
       }
     }
BasicTriangle[] result=null;

if(secondfound){
  result=new BasicTriangle[2];
  result[0] = new BasicTriangle(EP_1.copy(),EP_2.copy(),cN1);
  result[1] = new BasicTriangle(EP_1.copy(),EP_2.copy(),cN2);
 }
else if(firstfound){
   result=new BasicTriangle[1];
   result[0] = new BasicTriangle(EP_1.copy(),EP_2.copy(),cN1);
  }

return result; 
     
}

/**
 * computes a common Point with BS
 * @param BS the another Segment
 * @return the first found common endpoint or null if
 * not exists one
 */
public BasicPoint commonPoint(BasicSegment BS){
 if(EP_1.equals(BS.EP_1) | EP_1.equals(BS.EP_2)) return EP_1;
 if(EP_2.equals(BS.EP_1) | EP_2.equals(BS.EP_2)) return EP_2;
 return null;
}


/**
 * check  whether this BasicSegment contains (x,y)
 * @param x the x coordinate of checked Point
 * @param y the y coordinate of checked Point
 * @return true if (x,y) on this
 */
public boolean contains(double x, double y){
BasicSegment[] Segs = getSegments(x,y);
if (Segs==null){
  return false;
}
else{
  boolean found=false;
  for(int i=0;i<Segs.length;i++){
    if(this.equals(Segs[i]))
      found = true;
  }
  return found;
}
}

/**
 * computes all BasicSegments having BP as endpoint
 * @param  BP the to checked point
 * @return all segments having BP as endpoint
 */
public static BasicSegment[] getSegments(BasicPoint BP){
// returns all Segments, which have BP as EndPoint
BasicPoint[] Neightboors = BP.getNeightboors();

BasicSegment[] result = new BasicSegment[Neightboors.length];

for(int i=0;i<Neightboors.length;i++)
     result[i] = new BasicSegment(BP,Neightboors[i]);
return result;

}

/**
 * computes all BasicSegments having (x,y) in their pointset
 * @param x the x coordinate of the to checked point
 * @param y the y coordinate of the to checked point
 * @return all Segemnts having (x,y) in their pointset
 */
public static BasicSegment[] getSegments(double x, double y){

// first check : (x,y) is a BasicPoint ?
 if (BasicPoint.isBasicPoint(x,y)){
    BasicPoint BP = new BasicPoint((int)x,(int)y);
    return getSegments(BP);
 }
 else{
   // max 1 Segments
   BasicSegment[] result;
   int a = fuzzyobjects.Params.a;
   int b = fuzzyobjects.Params.b;
   // compute "rectangle" of the x-triangulation which
   // contains (x,y)
   int ix = (int) x;
   int iy = (int) y;
   int ltx = (ix/a)*a;
   int lty = (iy/b)*b;

   if(ltx ==(int)x){  // a vertical Segment
      result = new BasicSegment[1];
      BasicPoint BP1 = new BasicPoint(ltx,lty);
      BasicPoint BP2;
      if(y>=0)
         BP2 = new BasicPoint(ltx,lty+b);
      else
         BP2 = new BasicPoint(ltx,lty-b);

      result[0] = new BasicSegment(BP1,BP2);
   }// if vertical segment
   else
      if(lty==y){  // the horizontal Segment
         result = new BasicSegment[1];
         BasicPoint BP1 = new BasicPoint(ltx,lty);
         BasicPoint BP2;
         if(x>=0)
            BP2 = new BasicPoint(ltx+a,lty);
         else
            BP2 = new BasicPoint(ltx-a,lty);
         result[0] = new BasicSegment(BP1,BP2);
      } // if horizontal segment
      else {

        // a diagonale or nothing
        if(x<0)
           ltx = ltx-a;
        if(y<0)
           lty = lty-b;

        x = x-ltx;
        y = y-lty;
        double m = (double)b/(double)a;
        double q  = Math.sqrt(m*m+1.0);
        double distance_up   = Math.abs((x*m-y) / q);
        double distance_down = Math.abs((-x*m-y+b)/q);
        boolean left,bottom;
        bottom = y<b/2.0;
        left   = x<a/2.0;

        if(distance_up<epsilon){
          result = new BasicSegment[1];
          BasicPoint BP1,BP2;
          BP1 = new BasicPoint(ltx+a/2,lty+b/2);
         if(bottom)
             BP2 = new BasicPoint(ltx,lty);
         else
             BP2 = new BasicPoint(ltx+a,lty+b);
         result[0] = new BasicSegment(BP1,BP2);
        }
        else{
          if(distance_down<epsilon){
             result = new BasicSegment[1];
             BasicPoint BP1,BP2;
             BP1 = new BasicPoint(ltx+a/2,lty+b/2);
             if(left)
                 BP2 = new BasicPoint(ltx,lty+b);
             else
                 BP2 = new BasicPoint(ltx+a,lty);
             result[0] = new BasicSegment(BP1,BP2);
           }
           else{
             result = null;
           }
        } 
      } // diagonale or nothing
   return result;
} // else : not a BasicPoint
} // getSegments

/** get the x-Coordinate of a endpoint */
public int getX1(){return EP_1.getX();}
/** get the x-Coordinate of a endpoint */
public int getX2(){return EP_2.getX();}
/** get the y-Coordinate of a endpoint */
public int getY1(){return EP_1.getY();}
/** get the y-Coordinate of a endpoint */
public int getY2(){return EP_2.getY();}


/** returns the minimum X of the bounding box */
public int getMinX(){
 return Math.min(EP_1.getX(),EP_2.getX());
}

/** returns the minimum Y of the bounding box */
public int getMinY(){
  return Math.min(EP_1.getY(),EP_2.getY());
}

/** returns the maximum x of the bounding box */
public int getMaxX(){
  return Math.max(EP_1.getX(),EP_2.getX());
}

/** returns the maximum Y of the bounding box */
public int getMaxY(){
  return Math.max(EP_1.getY(),EP_2.getY());
}

/**
 * a endpoint of this segment
 */
protected  BasicPoint EP_1,EP_2;

/** for approximate test "a point is containg in
    this segment ?" */
private static double epsilon = 0.3;

} // class
