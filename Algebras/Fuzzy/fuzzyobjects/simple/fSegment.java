package fuzzyobjects.simple;

import fuzzyobjects.basic.*;
import sj.lang.ListExpr;

/**
 * provides fuzzy Segments
 * @author Thomas Behr
 */

public class fSegment implements SimpleObject{

/**
 * creates a new fuzzy segment
 * @param BP_1 a endpoint
 * @param Z1   the degree of fuzziness on BP_1
 * @param BP_2 a endpoint
 * @param Z2   the degre of fuzziness on BP_2
 */
public fSegment( BasicPoint BP_1, double Z1,
                 BasicPoint BP_2, double Z2) {

 if ( BP_1.compareTo(BP_2)<0) {
       this.EP_1 = BP_1;
       this.Z1   = Z1;
       this.EP_2 = BP_2;
       this.Z2   = Z2;
 }
 else {
       this.EP_1 = BP_2;
       this.Z1   = Z2;
       this.EP_2 = BP_1;
       this.Z2   = Z1;
 } 

}

/**
 * creates a new fSegment
 * @param x1,y1  the location of endpoint 1
 * @param Z1 degree of fuzziness of endpoint 1
 * @param x2,y2 the location of the second endpoint
 * @param Z2 degre of fuzziness of the second endpoint
 */
public fSegment(int x1, int y1, double Z1,
                int x2, int y2, double Z2) {
   this( (new BasicPoint(x1,y1)),Z1,
         (new BasicPoint(x2,y2)),Z2);
}

/**
 * creates a new fSegment
 * @param BS the location of the new Segment
 * @param Z1,Z2 the degree of fuzziness on both endpoints
 */
public fSegment(BasicSegment BS,double Z1,double Z2){
 this(BS.getEP1(),Z1,BS.getEP2(),Z2);
}

/**
 * returns the location of this segment
 * @return the underlying Basicsegment
 */
public BasicObject basic(){
  BasicSegment result = new BasicSegment(EP_1,EP_2);
  return result;
}


/**
 * creates a new fSegment
 * @param SP_1 the first fuzzy endpoint
 * @param SP_2 the second fuzzy endpoint
 */
public fSegment( fEPoint SP_1, fEPoint SP_2) {
  this.EP_1 = (BasicPoint) SP_1.basic();
  this.Z1   = SP_1.getZ();
  this.EP_2 = (BasicPoint) SP_2.basic();
  this.Z2   = SP_2.getZ();
  if(EP_1.compareTo(EP_2)>0) {
    BasicPoint BP = EP_1;
    EP_1 = EP_2;
    EP_2 = EP_1;
    double Z = Z1;
    Z1 = Z2;
    Z2 = Z1;
  }
}

/**
 * returns a copy of this
 * @return a new fSegment whith same
 * location and fuzziness as this
 */
public fSegment copy(){
  fSegment C = new fSegment(EP_1.copy(),Z1, EP_2.copy(),Z2);
  return C;
}

/**
 * check whether this is equal to fS
 * @return true if this and fS have same
 * location and fuzziness
 */
public boolean equals(fSegment fS) {
 return EP_1.equals(fS.EP_1) && EP_2.equals(fS.EP_2) &&
        (Z1 == fS.Z1) && (Z2 == fS.Z2);
}

/**
 * check whether this and fS have the same location
 * @return true if fS and this have the same underlying
 *  BasicSegment
 */
public boolean equalsBasic(fSegment fS) {
 return this.basic().equals(fS.basic());
}

/**
 * returns a readably form of this segment
 * @return a String representing this segment
 */
public String toString() {
 return "fS : ( (" + EP_1 + ","+Z1+") -> (" +
         EP_2 + ","+Z2+"))";
}

/**
 * check whether this segment is valid in the X-Triangulation
 * @return true if the underlying BasicSegment is valid and
 *         the degrees of fuzziness are in [0,1] and exist a
 *         Point whith a degree greater then 0 in this segment
 */
public boolean isValid() {
return  EP_1.neightbooring(EP_2) &&     // is over a BasicLine
        (Z1>=0) && (Z1<=1)  &&          // Zi is in [0,1]
        (Z2>=0) && (Z2<=1)  &&
        (Z1+Z2 >0);                     // not both Z are zero
}

/**
 * returns the degree of fuzziness 
 * @return degree of fuzziness of the smallest point
 */
public double getZ1(){
   return Z1;
}

/**
 * returns the degree of fuzziness
 * @return the degree of fuzziness of the greatest point
 */
public double getZ2(){
   return Z2;
}

/**
 * returns the smallest degree of fuzziness
 */
public double getMaxZ(){ return Math.max(Z1,Z2); }

/**
 * returns the greatest degree of fuzziness
 */
public double getMinZ(){ return Math.min(Z1,Z2); }


/**
 * returns the degree of fuzziness on a given point
 * @params the given point
 */
public double Zfkt(BasicPoint BP){
 double result=0.0;
 if (BP.equals(EP_1)) result=Z1;
 if (BP.equals(EP_2)) result=Z2;
 return result;
}

/**
 * returns the degree of fuzziness on a given point
 * the point must be on basic of the segment
 * @param x,y the geiven Point
 */
public double Zfkt(double x, double y){

// computes the membershipvalue of this Segment in (x,y)
// no check, wether (x,y) is on the Line !!!

double delta;
double x1 = EP_1.getX();
double x2 = EP_2.getX();
double y1 = EP_1.getY();
double y2 = EP_2.getY();

if (x1!=x2)
   delta =  (x-x1) /(x2-x1);
else
   delta = (y- y1) /(y2-y1);

return Z1+delta*(Z2-Z1);

}

/** computes the length of this segment as 3D-line*/
public double length(){
  double x1 = EP_1.getX();
  double y1 = EP_1.getY();
  double x2 = EP_2.getX();
  double y2 = EP_2.getY();
  return Math.sqrt( (x2-x1)*(x2-x1) +
                    (y2-y1)*(y2-y1) +
                    (Z2-Z1)*(Z2-Z1));
}

public int getMinX(){
 return Math.min(EP_1.getX(),EP_2.getX());
}

public int getMinY(){
  return Math.min(EP_1.getY(),EP_2.getY());
}

public int getMaxX(){
  return Math.max(EP_1.getX(),EP_2.getX());
}

public int getMaxY(){
  return Math.max(EP_1.getY(),EP_2.getY());
}


/** returns the ListExpr from this fSegment */
public ListExpr toListExpr(){
  fEPoint P1 = new fEPoint(EP_1,Z1);
  fEPoint P2 = new fEPoint(EP_2,Z2);
  return ListExpr.twoElemList(P1.toListExpr(),P2.toListExpr());
}


/** set the values of ths fSegment to LE,
  * if LE represent a valid FSegment
  * @return true if LE is a valid fSegment
  */
public boolean readFromListExpr(ListExpr LE){
  if(LE.listLength()!=2)
     return false;
  fEPoint P1 = new fEPoint(0,0,0);
  fEPoint P2 = new fEPoint(0,0,0);
  if(!( P1.readFromListExpr(LE.first()) &&
        P2.readFromListExpr(LE.second())))
     return false;
  BasicPoint BP1 = (BasicPoint) P1.basic();
  BasicPoint BP2 = (BasicPoint) P2.basic();
  if (! BP1.neightbooring(BP2))
     return false;
  
  this.EP_1 = BP1;
  this.EP_2 = BP2;
  this.Z1 = P1.getZ();
  this.Z2 = P2.getZ();  
  return true;
}

/* returns the first endpoint */
public fEPoint getP1(){
   return new fEPoint(EP_1,Z1);
}

/* returns the second endpoint */
public fEPoint getP2(){
   return new fEPoint(EP_2,Z2);
}


/** a BasicPoint */
private BasicPoint EP_1,
                   EP_2;
/**  degree of fuzziness */
private double     Z1,Z2;


}

