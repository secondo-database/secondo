package fuzzyobjects.basic;

import fuzzyobjects.*;
import java.util.Vector;

/**
 * this class privides a BasicTriangle of the X-triangulation
 * @author Thomas Behr
 */
public class BasicTriangle implements BasicObject{
/**
 * creates a new basicTriangle from the given points
 * no check whether this is a valid triangle
 * @param CP_? a cornerpoint of the new Triangle
 */
public BasicTriangle(BasicPoint CP_1,
                     BasicPoint CP_2,
                     BasicPoint CP_3) {
  BasicPoint[] BPs = new BasicPoint[3];
  BPs[0] = CP_1;
  BPs[1] = CP_2;
  BPs[2] = CP_3;
  BasicPoint.sort(BPs);
  this.CP_1 = BPs[0];
  this.CP_2 = BPs[1];
  this.CP_3 = BPs[2];
}

/**
 * returns a copy of this
 * @return a new BasicTriangle on the same location as this
 */
public BasicTriangle copy(){
  return new BasicTriangle( CP_1.copy(), CP_2.copy(), CP_3.copy());
}

/**
 * check whether this is equal to BT
 * @param BT the another Triangle
 * @return true if BT is on the same location as this
 */
public boolean equals(BasicTriangle BT) {
 return
       ( CP_1.equals(BT.CP_1)) &&
       ( CP_2.equals(BT.CP_2)) &&
       ( CP_3.equals(BT.CP_3));
}

/**
 * returns a readable form of this triangle
 * @return a String representing this triangle
 */
public String toString(){
  return " BT : ( "+CP_1+","+CP_2+","+CP_3+")";
}

/**
 * check whether this is a Triangle of the X-Triangulation
 * @return true if all cornerpoints are valid and connected
 */
public  boolean isValid(){
  boolean ok = CP_1.isValid() && CP_2.isValid() && CP_3.isValid();
  if(ok) {
    ok =  CP_1.neightbooring(CP_2) &&
          CP_1.neightbooring(CP_3) &&
          CP_2.neightbooring(CP_3);
  }
  return ok;
}

/**
 * returns the 3 cornerpoints in a array
 * @return a array containing the 3 cornerpoints
 */
public BasicPoint[] getBasicPoints(){
 BasicPoint[] BPs = new BasicPoint[3];
 BPs[0] = CP_1.copy();
 BPs[1] = CP_2.copy();
 BPs[2] = CP_3.copy();
 return BPs;
}


/**
 * computes the number of common points
 * @param BO the another BasicObject
 * @return number of common BasicPoints
 */
private int commonPoints(BasicObject BO) {
int n = 0;
BasicPoint[] BPs = BO.getBasicPoints();
  for (int i=0;i<BPs.length;i++){
    if (CP_1.equals(BPs[i])) n++;
    if (CP_2.equals(BPs[i])) n++;
    if (CP_3.equals(BPs[i])) n++;
  }
return n;
}

/**
 * check whether this and BT have one common side
 * @params BT the another Triangle
 * @return true if BT and this have one common side
 */
public boolean connected(BasicTriangle BT) {
 return commonPoints(BT)==2;
}

/**
 * check whether this is connected whith BL
 * @param BL the BasicLine to checked
 * @return true if BL and this have one common BasicPoint
 */
public boolean connected(BasicSegment BL) {
 return commonPoints(BL)==1;
}

/**
 * check whether this and  BT are simple connected
 * @param BT the another BasicTriangle
 * @return true if BT and this have one common BasicPoint
 */
public boolean sconnected(BasicTriangle BT) {
  return commonPoints(BT) == 1;
}

/**
 * computes the euclidic distance to another BasicObject
 * @param BO the another BasicObject
 * @return the smallest euclidic distance from the 3 cornerpoints to
 * all BasicPoints from BO
 */
public double euclid_distance(BasicObject BO) {

  double min = Math.min( CP_1.q_euclid_distance(BO),
                         Math.min(  CP_2.q_euclid_distance(BO),
                                    CP_3.q_euclid_distance(BO)  )  );
  return Math.sqrt(min);

}

/**
 * computes the square of the euclidic distance to a BasicObject
 * @param BO the another BasicObject
 * @return the square of the smallest distance between the 3 cornerpoints
 * and the BasicPoints of BO
 */
public double q_euclid_distance(BasicObject BO) {
  return Math.min( CP_1.q_euclid_distance(BO) ,
                     Math.min(CP_2.q_euclid_distance(BO) ,
                              CP_3.q_euclid_distance(BO)));
}


/**
 * check whether BP is a Cornerpoint of this
 * @param BP the BasicPoint to checked
 * @return true if BP is a cornerpoint of this
 */
public boolean cornerPoint(BasicPoint BP) {
  return commonPoints(BP) == 1;
}

/**
 * check whether a BasicSegment is a Side of this
 * @param BL the BasicSegment to checked
 * @return true if BL is a Side from this
 */
public boolean borderSegment(BasicSegment BL) {
  return commonPoints(BL) == 2;
}

/**
 * computes the size from this Triangle
 * @return the standard-size of a Triangle in the X-Triangulation
 */
public double  surface() {
  return Params.a*Params.b/4;
}

/**
 * compares this whith another BasicObject
 * @param O the another BasicObject
 * @return <ul>
 *           <li> -1 if O not a BasicTriangle or this is smaller then O </li>
 *           <li>  0 id O equals to this                                </li>
 *           <li>  1 if 0 is a BasicTriangle and this is greater then O </li>
 *         </ul>
 */
public int compareTo(BasicObject O){
 if(!(O instanceof BasicTriangle))
     return -1;
 else {
   BasicTriangle BT = (BasicTriangle) O;
   int C = CP_1.compareTo(BT.CP_1);
   if (C==0)
      C = CP_2.compareTo(BT.CP_2);
      if (C==0)
         C = CP_3.compareTo(BT.CP_3);
   return C;
 } // else
} // compareTo


/**
 * returns a Cornerpoint of this
 * @return the smallest cornerpoint of this
 */
public BasicPoint getCP1(){
   return CP_1;
}

/**
 * returns a Cornerpoint of this
 * @returns the middle cornerpoint of this
 */
public BasicPoint getCP2(){
   return CP_2;
}

/**
 * returns a cornerpoint of this
 * @return the greatest cornerpoint of this
 */
public BasicPoint getCP3(){
   return CP_3;
}


/* returns the 3 BasicTriangles which have a
 * common Side with this
 */
public BasicTriangle[] getRealNeightboors(){

  BasicSegment[] Sides = new BasicSegment[3];
  Sides[0] = new BasicSegment(CP_1,CP_2);
  Sides[1] = new BasicSegment(CP_2,CP_3);
  Sides[2] = new BasicSegment(CP_1,CP_3);

  BasicTriangle[] result = new BasicTriangle[3];
  BasicTriangle[] BTs;

  for(int i=0;i<3;i++){
   BTs = Sides[i].getTriangles();
   if(BTs[0].equals(this))
      result[i] = BTs[1];
   else
      result[i] = BTs[0];
  }
  return result;
}


/**
 * computes all (simple) connected Triangles
 * @return all Triangles having 1 or 2 common BasicPoints whith this
 */
public BasicTriangle[] getNeightboors(){
  Vector Tmp = new Vector();
  BasicPoint[] NB1 = CP_1.getNeightboors();
  BasicPoint[] NB2 = CP_2.getNeightboors();
  BasicPoint[] NB3 = CP_3.getNeightboors();
  for(int i=0;i<NB1.length;i++){
    for(int j=0;j<NB2.length;j++)
      if(NB1[i].equals(NB2[j]) && !NB1[i].equals(CP_3) )   
         Tmp.add(new BasicTriangle(CP_1,CP_2,NB1[i]));
  }
  for(int i=0;i<NB1.length;i++){
    for(int j=0;j<NB3.length;j++)
      if(NB1[i].equals(NB3[j]) && !NB1[i].equals(CP_2) )   
         Tmp.add(new BasicTriangle(CP_1,CP_3,NB1[i]));
  }
  for(int i=0;i<NB2.length;i++){
    for(int j=0;j<NB3.length;j++)
      if(NB2[i].equals(NB3[j]) && !NB2[i].equals(CP_1) )   
         Tmp.add(new BasicTriangle(CP_2,CP_3,NB2[i]));
  }
  BasicTriangle[] result = new BasicTriangle[Tmp.size()];
  for(int i=0; i<Tmp.size();i++){
     result[i] = (BasicTriangle) Tmp.get(i);
  }
  return result;
}

/**
 * computes the common Segment whith another BasicTriangle
 * @param BT the another Triangle
 * @return the common Segment or null if not exists one
 */
public BasicSegment commonSegment(BasicTriangle BT){
if (commonPoints(BT)!=2) return null;

int N1=-1,N2=-1;
BasicPoint[] tmp = new BasicPoint[3];
tmp[0] = BT.CP_1;
tmp[1] = BT.CP_2;
tmp[2] = BT.CP_3;
for(int i=0;i<3;i++){
  if (CP_1.equals(tmp[i]) | CP_2.equals(tmp[i]) | CP_3.equals(tmp[i]) )
    if (N1<0)
       N1 = i;
    else
       N2 = i;
}
return new BasicSegment(tmp[N1],tmp[N2]);

}

/**
 * computes all Triangles having a given BasicPoint as corner
 * @param BP the given BasicPoint
 * @return a array containing all Triangles having BP as corner
 */
public static BasicTriangle[] getTriangles(BasicPoint BP){

  BasicPoint[] NB = BP.getNeightboors();
  Vector tmp = new Vector();
  for(int i=0;i<NB.length-1;i++)
     for(int j=i+1;j<NB.length;j++){
        if(NB[i].neightbooring(NB[j]))
          tmp.add(new BasicTriangle(BP,NB[i],NB[j]));
     }

  BasicTriangle[] result = new BasicTriangle[tmp.size()];
  for(int k=0;k<tmp.size();k++){
    result[k] = (BasicTriangle) tmp.get(k);
  }
  return result;
}

/**
 * computes all Triangles having a given point in their pointset
 * @param x,y the given point
 * @return a array containing all Triangles having (x,y) in their
 *  pointset
 */
public static BasicTriangle[] getTriangles(double x, double y){


 // case 1 : (x,y) is a cornerpoint
 if(BasicPoint.isBasicPoint(x,y))
   return getTriangles(new BasicPoint((int)x , (int)y));

 // case 2 : (x,y) is on a side

 BasicSegment[] Segs = BasicSegment.getSegments(x,y);

 Vector tmp=new Vector();
 BasicTriangle[] BTs;

 if(Segs!=null && Segs.length>0){
      for(int i=0;i<Segs.length;i++){
          BTs = Segs[i].getTriangles();
          for(int j=0;j<BTs.length;j++){
              tmp.add(BTs[j]);
          }
      }
      
 } 

 else { // case 3: (x,y) is in the interior

  int Xint = (int)x;
  int Yint = (int)y;
  int Aint = fuzzyobjects.Params.a;
  int Bint = fuzzyobjects.Params.b;
  double a = Aint;
  double b = Bint;
  int xC = (Xint/Aint)*Aint;
  int yC = (Yint/Bint)*Bint;
  if(x<0)
      xC=xC-Aint;
  if(y<0)
      yC=yC-Bint;

  x = x-xC;
  y = y-yC;


   /*
     cases :
            -------------
            |\         /|
            |  \  1  /  |
            | 2  \ /  4 |
            |    / \    |
            |  /  3  \  |
            |/         \|
            -------------
    */


  if( (y/x) > (b/a) )  {  // left-top    cases 1 and 2

    if( b/a > (b-y)/x ){ //  case 1
            tmp.add( new BasicTriangle(
                                  new BasicPoint(xC+Aint/2,yC+Bint/2),
                                  new BasicPoint(xC+Aint,yC+Bint),
                                  new BasicPoint(xC,yC+Bint)));
    }
    else { // case 2
       tmp.add( new BasicTriangle( new BasicPoint(xC,yC+Bint),
                                   new BasicPoint(xC+Aint/2,yC+Bint/2),
                                   new BasicPoint(xC,yC)));
   }
  }

  else{   //  right-bottom cases 3 and 4

     if( b/a > (b-y)/x  ) {   // case 4
        tmp.add( new BasicTriangle( new BasicPoint(xC+Aint/2,yC+Bint/2),
                                  new BasicPoint(xC+Aint,yC+Bint),
                                  new BasicPoint(xC+Aint,yC) ));
    }
    else{  // case 3
        tmp.add(new BasicTriangle( new BasicPoint(xC,yC),
                                 new BasicPoint(xC+Aint/2,yC+Bint/2),
                                 new BasicPoint(xC+Aint,yC)));
   }
  }

 }


 BasicTriangle[] result = new BasicTriangle[tmp.size()];
 for(int l=0;l<tmp.size();l++){
   result[l] = (BasicTriangle) tmp.get(l);
 }
 return result;
}

/** check whether this Triangle contains (x,y) */
public boolean contains(double x, double y){
  BasicTriangle[] BTs = getTriangles(x,y);
  boolean result = false;
  for(int i=0;i<BTs.length;i++){
    if(BTs[i].equals(this))
      result = true;
  }
  return result;
}



/** get a X-Coordinate */
public int getX1(){return CP_1.getX();}
/** get a X-Coordinate */
public int getX2(){return CP_2.getX();}
/** get a X-Coordinate */
public int getX3(){return CP_3.getX();}
/** get a Y-Coordinate */
public int getY1(){return CP_1.getY();}
/** get a Y-Coordinate */
public int getY2(){return CP_2.getY();}
/** get a Y-Coordinate */
public int getY3(){return CP_3.getY();}

/** returns the minimum x of the bounding box */
public int getMinX(){
  return Math.min(CP_1.getX(),Math.min(CP_2.getX(),CP_3.getX()));
}

/** returns the minimum y of the bounding box */
public int getMinY(){
  return Math.min(CP_1.getY(),Math.min(CP_2.getY(),CP_3.getY()));
}

/** returns the maximum x of the bounding box */
public int getMaxX(){
  return Math.max(CP_1.getX(),Math.max(CP_2.getX(),CP_3.getX()));
}

/** returns the maximum y of the bounding box */
public int getMaxY(){
  return Math.max(CP_1.getY(),Math.max(CP_2.getY(),CP_3.getY()));
}

/** returns the 3 Sides of this Triangle */
public BasicSegment[] getSides(){
BasicSegment[] result = new BasicSegment[3];
result[0] = new BasicSegment(CP_1,CP_2);
result[1] = new BasicSegment(CP_2,CP_3);
result[2] = new BasicSegment(CP_1,CP_3);
return result;
}


/** a Cornerpoint of this Triangle */
private BasicPoint CP_1, CP_2, CP_3;


} // class BasicTriangle
